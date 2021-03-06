/* -----------------------------------------------------------------------------
 *
 * (c) The GHC Team 2005
 *
 * Machine-independent interface to time measurement
 *
 * ---------------------------------------------------------------------------*/

#ifndef GETTIME_H
#define GETTIME_H

#include "BeginPrivate.h"

Time getProcessCPUTime     (void);
Time getThreadCPUTime      (void);
Time getProcessElapsedTime (void);
void getProcessTimes       (Time *user, Time *elapsed);

/* Get the current date and time.
   Uses seconds since the Unix epoch, plus nanoseconds
 */
void  getUnixEpochTime      (StgWord64 *sec, StgWord32 *nsec);

// Not strictly timing, but related
nat   getPageFaults         (void);

#include "EndPrivate.h"

#endif /* GETTIME_H */
