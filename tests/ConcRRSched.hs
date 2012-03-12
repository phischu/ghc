-----------------------------------------------------------------------------
-- |
-- Module      :  LwConc.Schedulers.ConcRRSched
-- Copyright   :  (c) The University of Glasgow 2001
-- License     :  BSD-style (see the file libraries/base/LICENSE)
-- 
-- Maintainer  :  libraries@haskell.org
-- Stability   :  experimental
-- Portability :  non-portable (concurrency)
--
-- A concurrent round-robin scheduler.
--
-----------------------------------------------------------------------------


module ConcRRSched
(ConcRRSched
, newConcRRSched      -- IO (ConcRRSched)
, forkIO              -- ConcRRSched -> IO () -> IO ()
, yield               -- ConcRRSched -> IO ()
, getSchedActionPair  -- ConcRRSched -> SCont -> IO (PTM (), PTM ())

, createThread        -- ConcRRSched -> IO () -> IO SCont | Creates a thread but
                      -- does not add it to the scheduler
) where

import LwConc.Substrate
import qualified Data.Sequence as Seq
import Control.OldException

newtype ConcRRSched = ConcRRSched (PVar (Seq.Seq SCont))

newConcRRSched :: IO (ConcRRSched)
newConcRRSched = do
  ref <- newPVarIO Seq.empty
  return $ ConcRRSched (ref)

switchToNextAndFinish :: ConcRRSched -> IO ()
switchToNextAndFinish (ConcRRSched ref) = atomically $ do
  contents <- readPVar ref
  if Seq.length contents == 0
      then undefined
      else do
        let x = Seq.index contents 0
        let tail = Seq.drop 1 contents
        writePVar ref $ tail
        switchTo x Completed

createThread :: ConcRRSched -> IO () -> IO SCont
createThread (ConcRRSched ref) task =
  let yieldingTask = do {
    try task;
    switchToNextAndFinish (ConcRRSched ref);
    print "ConcRRSched.forkIO: Should not see this!"
  }
  in do {
    s <- newSCont yieldingTask;
    (_,u) <- getSchedActionPair (ConcRRSched ref) s;
    setResumeThreadClosure s $ atomically u;
    return s
    }

forkIO :: ConcRRSched -> IO () -> IO ()
forkIO (ConcRRSched ref) task = do
  thread <- createThread (ConcRRSched ref) task
  atomically $ do
    contents <- readPVar ref
    writePVar ref $ contents Seq.|> thread

switchToNextWith :: ConcRRSched -> (Seq.Seq SCont -> Seq.Seq SCont) -> ThreadStatus -> PTM ()
switchToNextWith (ConcRRSched ref) f status = do
  oldContents <- readPVar ref
  let contents = f oldContents
  if Seq.length contents == 0
     then undefined
     else do
       let x = Seq.index contents 0
       let tail = Seq.drop 1 contents
       writePVar ref $ tail
       switchTo x status

enque :: ConcRRSched -> SCont -> PTM ()
enque (ConcRRSched ref) s = do
  contents <- readPVar ref
  let newSeq = contents Seq.|> s
  writePVar ref $ newSeq

yield :: ConcRRSched -> IO ()
yield sched = atomically $ do
  s <- getSCont
  switchToNextWith sched (\tail -> tail Seq.|> s) BlockedOnSched

-- blockAction must be called by the thread t whose SCont corresponds to the
-- Scont passed to the getSchedActionPair function.  unblockAction must be
-- called by a thread other than t, which adds t back to its scheduler.

getSchedActionPair :: ConcRRSched -> SCont -> IO (PTM (), PTM ())
getSchedActionPair sched s = do
  let blockAction   = switchToNextWith sched (\tail -> tail) BlockedOnConcDS
  let unblockAction = do
      unsafeIOToPTM $ print "unblocking SCont"
      enque sched s
  return (blockAction, unblockAction)
