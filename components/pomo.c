#include <string.h>
#include <signal.h>

#include "../util.h"

#define POMO_WORK_SECONDS 1500
#define POMO_BREAK_SECONDS 300
#define POMO_LONG_BREAK_SECONDS 900
#define POMO_WORK_SESSIONS 4

#if defined(__linux__)

static int pomo_seconds = POMO_WORK_SECONDS;
static int previous_time = POMO_WORK_SECONDS;
static int pomo_sessions = 0;
static int pomo_state = 0;

//0 = paused
//1 = ticking

const char *getSymbol() {
    return previous_time == POMO_WORK_SECONDS
        ? "w"
        : "b";
}

const char *getTime() {
    return bprintf("%d%s%.2d:%.2d",
            pomo_sessions,
            getSymbol(),
            pomo_seconds / 60,
            pomo_seconds % 60);
}

int breakSeconds() {
    pomo_sessions++;

    if (pomo_sessions % POMO_WORK_SESSIONS == 0) {
        previous_time = POMO_LONG_BREAK_SECONDS;
        return POMO_LONG_BREAK_SECONDS;
    }

    previous_time = POMO_BREAK_SECONDS;
    return POMO_BREAK_SECONDS;
}

int newSeconds() {
    if (previous_time != POMO_WORK_SECONDS) {
        previous_time = POMO_WORK_SECONDS;
        return POMO_WORK_SECONDS;
    }

    return breakSeconds();
}

const char *transition() {
    pomo_state = 1;
    pomo_seconds = newSeconds();

    return getTime();
}

const char *reduceTime(void) {
  pomo_seconds = pomo_seconds - 1;
  return pomo_seconds <= 0
             ? bprintf("%d%s-%.2d:%.2d", pomo_sessions, getSymbol(),(pomo_seconds / 60) * -1,
                       (pomo_seconds % 60) * -1)
             : bprintf("%d%s%.2d:%.2d", pomo_sessions, getSymbol(),pomo_seconds / 60, pomo_seconds % 60);
}

void signalChange() {
    if (pomo_state == 0)
        pomo_state = 1;
    else
        pomo_state = 0;
}

void reset() {
  pomo_seconds = POMO_WORK_SECONDS;
  previous_time = POMO_WORK_SECONDS;
  pomo_state = 0;
  pomo_sessions = 0;
}

const char *pomo(void) {
    char returnString[50];

    signal(SIGUSR2, signalChange);
    signal(SIGRTMIN + 1, reset);

    if (pomo_state == 1) 
        strncpy(returnString, reduceTime(), 50);
    else if(pomo_seconds < 0) 
        strncpy(returnString, transition(), 50);
    else 
        strncpy(returnString, getTime(), 50);
    
    return bprintf("%s", returnString);
}

#endif
