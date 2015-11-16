/************************************************************************

     Name:     PRJ main function

     Type:     C Header file

     Desc:     PRJ Framework 

     File:     nc_wd.c

     Sid:      nc_wd.c

     Prg:      frau / comate

************************************************************************/
/* header */
#include "nc_nl.h"


static void tsNone(void);
static void tsUnkDown(void);
static void tsUnkUp(void);
static void tsDeadUp(void);
static void tsRunDown(void);
static void tsUnkReg(void);
static void tsDeadReg(void);
static void tsRunReg(void);
static void tsRegDown(void);

/* ------------------------------------------------------------------- */
/* FSM */
/* ------------------------------------------------------------------- */
tskSmNlStateFnc tskSmNlFsmMt[ST_MAX][EV_MAX] = {
    { //ST_UNKNW
        tsUnkDown,  //EV_DOWN
        tsUnkUp,    //EV_UP
        tsUnkReg,    //EV_REG
    },
    { //ST_DEAD
        tsNone,     //EV_DOWN
        tsDeadUp,   //EV_UP
        tsDeadReg,    //EV_REG
    },
    { //ST_RUN
        tsRunDown,  //EV_DOWN
        tsNone,     //EV_UP
        tsRunReg,    //EV_REG
    },
    { //ST_REG
        tsRegDown,  //EV_DOWN
        tsNone,     //EV_UP
        tsNone,    //EV_REG
    },
};
static 
void tsNone(void) {}
static 
void tsUnkUp(void)
{
    pr_alert("[%s] mgwd INIT --> RUN \n", __FUNCTION__);
    SET_STATE(ST_RUN);
}

static 
void tsUnkDown(void)
{
    pr_alert("[%s] mgwd INIT --> DEAD \n", __FUNCTION__);
    SET_STATE(ST_DEAD);
}

static 
void tsDeadUp(void)
{
    pr_alert("[%s] mgwd DEAD --> RUN \n", __FUNCTION__);
    SET_STATE(ST_RUN);
}

static 
void tsRunDown(void)
{
    pr_alert("[%s] mgwd RUN --> DEAD \n", __FUNCTION__);
    SET_STATE(ST_DEAD);
}
static 
void tsUnkReg(void)
{
    pr_alert("[%s] mgwd INIT --> REGI \n", __FUNCTION__);
    SET_STATE(ST_REG);
}
static 
void tsDeadReg(void)
{
    pr_alert("[%s] mgwd DEAD --> REGI \n", __FUNCTION__);
    SET_STATE(ST_REG);
}
static 
void tsRunReg(void)
{
    pr_alert("[%s] mgwd RUN --> REGI \n", __FUNCTION__);
    SET_STATE(ST_REG);
}

static 
void tsRegDown(void)
{
    pr_alert("[%s] mgwd REGI --> DOWN \n", __FUNCTION__);
    SET_STATE(ST_DEAD);
}

/* -------------------------------------------------------------------------- */
/* Watchdog */
/* -------------------------------------------------------------------------- */
int thrWd(void *arg)
{
volatile int running = 0;
struct task_struct *proc = NULL;

    while(!kthread_should_stop())
    {
        set_current_state(TASK_INTERRUPTIBLE);
        //schedule_timeout(2*HZ);//sleep 1 sec
        //schedule_timeout(1*HZ);//sleep 1 sec
        //schedule_timeout(msecs_to_jiffies(500)); //500msec
        schedule_timeout(msecs_to_jiffies(1)); //1msec

        running = 0;

        for_each_process(proc)
        {
#if 0
            if( !strcmp(proc->comm, gSmNlCfg->wProcName) &&
                (proc->state == TASK_RUNNING ||
                proc->state & TASK_NORMAL ))
#else
            //if( !strcmp(proc->comm, gSmNlCfg->wProcName) &&
            //    (proc->state < 4))
            if( !strcmp(proc->comm, gSmNlCfg->wProcName) &&
                (proc->state != 4))
#endif
            {
                running++;
            }
        }

        if(!running)
            tskSmNlFsmMt[GET_STATE][EV_DOWN]();
        else
            tskSmNlFsmMt[GET_STATE][EV_UP]();

    } /* end of while */

    gSmNlTsk = NULL;
    return 0;
} /*end of function */


/************************************************************************
         End of file:   
************************************************************************/

/************************************************************************
     ver       pat    init                  description
------------ -------- ---- ----------------------------------------------
/main/1      ---      frau  1. Initial Release
************************************************************************/
