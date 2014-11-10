#ifndef _TIMER_
#define _TIMER_

#include <Windows.h>

#define WM_SETTIME  (WM_USER + 1)
#define WM_RESET    (WM_USER + 2)

#define TMR_ADDSEC  ((WPARAM)0)
#define TMR_SUBSEC  ((WPARAM)1)
#define TMR_RESET   ((WPARAM)2)
#define TMR_SET     ((WPARAM)3)

// A shortcut for memory allocation
#define NEW(s)      ((s*)calloc(1,sizeof(s)))

// Contains the current time
typedef struct _TIME {
	BYTE hours;
	BYTE minutes;
	BYTE seconds;
} TIME, * PTIME;

// Contains all the info about the timer
typedef struct _INFO {
	PTIME pTime; // The _TIME structure
	PTIME pBaseTime;
	WORD wTypedTimePlace;
	char *lpcTime;
	BOOL isOn;
	BOOL isSet;
	BOOL isGoingDown;
	HANDLE hTimer;
	HANDLE hTimerQueue;
} INFO, * PINFO;

#endif
