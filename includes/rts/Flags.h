/* -----------------------------------------------------------------------------
 *
 * (c) The GHC Team, 1998-2009
 *
 * Datatypes that holds the command-line flag settings.
 *
 * Do not #include this file directly: #include "Rts.h" instead.
 *
 * To understand the structure of the RTS headers, see the wiki:
 *   http://hackage.haskell.org/trac/ghc/wiki/Commentary/SourceTree/Includes
 *
 * ---------------------------------------------------------------------------*/

#ifndef RTS_FLAGS_H
#define RTS_FLAGS_H

#include <stdio.h>

/* For defaults, see the @initRtsFlagsDefaults@ routine. */

struct GC_FLAGS {
    FILE   *statsFile;
    nat	    giveStats;
#define NO_GC_STATS	 0
#define COLLECT_GC_STATS 1
#define ONELINE_GC_STATS 2
#define SUMMARY_GC_STATS 3
#define VERBOSE_GC_STATS 4

    nat     maxStkSize;         /* in *words* */
    nat     initialStkSize;     /* in *words* */

    nat	    maxHeapSize;        /* in *blocks* */
    nat     minAllocAreaSize;   /* in *blocks* */
    nat     minOldGenSize;      /* in *blocks* */
    nat     heapSizeSuggestion; /* in *blocks* */
    double  oldGenFactor;
    double  pcFreeHeap;

    nat     generations;
    nat     steps;
    rtsBool squeezeUpdFrames;

    rtsBool compact;		/* True <=> "compact all the time" */
    double  compactThreshold;

    rtsBool sweep;		/* use "mostly mark-sweep" instead of copying
                                 * for the oldest generation */
    rtsBool ringBell;
    rtsBool frontpanel;

    int idleGCDelayTime;	/* in milliseconds */

    StgWord heapBase;           /* address to ask the OS for memory */
};

struct DEBUG_FLAGS {  
    /* flags to control debugging output & extra checking in various subsystems */
    rtsBool scheduler;      /* 's' */
    rtsBool interpreter;    /* 'i' */
    rtsBool weak;           /* 'w' */
    rtsBool gccafs;         /* 'G' */
    rtsBool gc;             /* 'g' */
    rtsBool block_alloc;    /* 'b' */
    rtsBool sanity;         /* 'S'   warning: might be expensive! */
    rtsBool stable;         /* 't' */
    rtsBool prof;           /* 'p' */
    rtsBool eventlog;       /* 'e' */
    rtsBool linker;         /* 'l'   the object linker */
    rtsBool apply;          /* 'a' */
    rtsBool stm;            /* 'm' */
    rtsBool squeeze;        /* 'z'  stack squeezing & lazy blackholing */
    rtsBool hpc; 	    /* 'c' coverage */
    rtsBool timestamp;          /* add timestamps to traces */
};

struct COST_CENTRE_FLAGS {
    unsigned int	    doCostCentres;
# define COST_CENTRES_SUMMARY	1
# define COST_CENTRES_VERBOSE	2 /* incl. serial time profile */
# define COST_CENTRES_ALL	3
# define COST_CENTRES_XML       4

    int	    profilerTicks;   /* derived */
    int	    msecsPerTick;    /* derived */
};

struct PROFILING_FLAGS {
    unsigned int	doHeapProfile;
# define NO_HEAP_PROFILING	0	/* N.B. Used as indexes into arrays */
# define HEAP_BY_CCS		1
# define HEAP_BY_MOD		2
# define HEAP_BY_DESCR		4
# define HEAP_BY_TYPE		5
# define HEAP_BY_RETAINER       6
# define HEAP_BY_LDV            7

# define HEAP_BY_CLOSURE_TYPE   8

    nat                 profileInterval;      /* delta between samples (in ms) */
    nat                 profileIntervalTicks; /* delta between samples (in 'ticks') */
    rtsBool             includeTSOs;


    rtsBool		showCCSOnException;

    nat                 maxRetainerSetSize;

    nat                 ccsLength;

    char*               modSelector;
    char*               descrSelector;
    char*               typeSelector;
    char*               ccSelector;
    char*               ccsSelector;
    char*               retainerSelector;
    char*               bioSelector;

};

#ifdef EVENTLOG
struct EVENTLOG_FLAGS {
  rtsBool doEventLogging;
};
#endif

struct CONCURRENT_FLAGS {
    int ctxtSwitchTime;		/* in milliseconds */
    int ctxtSwitchTicks;	/* derived */
};

struct MISC_FLAGS {
    int tickInterval;     /* in milliseconds */
    rtsBool install_signal_handlers;
    rtsBool machineReadable;
    StgWord linkerMemBase;       /* address to ask the OS for memory
                                  * for the linker, NULL ==> off */
};

#ifdef THREADED_RTS
struct PAR_FLAGS {
  nat            nNodes;         /* number of threads to run simultaneously */
  rtsBool        migrate;        /* migrate threads between capabilities */
  rtsBool        wakeupMigrate;  /* migrate a thread on wakeup */
  unsigned int	 maxLocalSparks;
  rtsBool        parGcEnabled;   /* enable parallel GC */
  rtsBool        parGcGen;       /* do parallel GC in this generation
                                  * and higher only */
  rtsBool        parGcLoadBalancing; /* do load-balancing in parallel GC */
  rtsBool        setAffinity;    /* force thread affinity with CPUs */
};
#endif /* THREADED_RTS */

struct TICKY_FLAGS {
    rtsBool showTickyStats;
    FILE   *tickyFile;
};

#ifdef USE_PAPI
#define MAX_PAPI_USER_EVENTS 8

struct PAPI_FLAGS {
    nat     eventType;          /* The type of events to count */
    nat     numUserEvents;
    char *  userEvents[MAX_PAPI_USER_EVENTS];
};

#define PAPI_FLAG_CACHE_L1 1
#define PAPI_FLAG_CACHE_L2 2
#define PAPI_FLAG_BRANCH 3
#define PAPI_FLAG_STALLS 4
#define PAPI_FLAG_CB_EVENTS 5
#define PAPI_USER_EVENTS 6

#endif

/* Put them together: */

typedef struct _RTS_FLAGS {
    /* The first portion of RTS_FLAGS is invariant. */
    struct GC_FLAGS	     GcFlags;
    struct CONCURRENT_FLAGS  ConcFlags;
    struct MISC_FLAGS        MiscFlags;
    struct DEBUG_FLAGS	     DebugFlags;
    struct COST_CENTRE_FLAGS CcFlags;
    struct PROFILING_FLAGS   ProfFlags;
#ifdef EVENTLOG
    struct EVENTLOG_FLAGS    EventLogFlags;
#endif
    struct TICKY_FLAGS	     TickyFlags;

#if defined(THREADED_RTS)
    struct PAR_FLAGS	ParFlags;
#endif
#ifdef USE_PAPI
    struct PAPI_FLAGS   PapiFlags;
#endif
} RTS_FLAGS;

#ifdef COMPILING_RTS_MAIN
extern DLLIMPORT RTS_FLAGS RtsFlags;
#elif IN_STG_CODE
/* Hack because the C code generator can't generate '&label'. */
extern RTS_FLAGS RtsFlags[];
#else
extern RTS_FLAGS RtsFlags;
#endif

/* Routines that operate-on/to-do-with RTS flags: */

void initRtsFlagsDefaults(void);
void setupRtsFlags(int *argc, char *argv[], int *rts_argc, char *rts_argv[]);
void setProgName(char *argv[]);


/*
 * The printf formats are here, so we are less likely to make
 * overly-long filenames (with disastrous results).  No more than 128
 * chars, please!  
 */

#define STATS_FILENAME_MAXLEN	128

#define GR_FILENAME_FMT		"%0.124s.gr"
#define GR_FILENAME_FMT_GUM	"%0.120s.%03d.%s"
#define HP_FILENAME_FMT		"%0.124s.hp"
#define LIFE_FILENAME_FMT	"%0.122s.life"
#define PROF_FILENAME_FMT	"%0.122s.prof"
#define PROF_FILENAME_FMT_GUM	"%0.118s.%03d.prof"
#define QP_FILENAME_FMT		"%0.124s.qp"
#define STAT_FILENAME_FMT	"%0.122s.stat"
#define TICKY_FILENAME_FMT	"%0.121s.ticky"
#define TIME_FILENAME_FMT	"%0.122s.time"
#define TIME_FILENAME_FMT_GUM	"%0.118s.%03d.time"

/* an "int" so as to match normal "argc" */
/* Now defined in Stg.h (lib/std/cbits need these too.)
extern int     prog_argc;
extern char  **prog_argv;
*/
extern int     rts_argc;  /* ditto */
extern char   *rts_argv[];

#endif	/* RTS_FLAGS_H */