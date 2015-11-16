/************************************************************************

************************************************************************/
#ifndef __NC_NL_H__
#define __NC_NL_H__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>  /* for kthread */
#include <linux/kernel.h> 
#include <linux/proc_fs.h> //for /proc entiries
#include <linux/sched.h>
#include <linux/workqueue.h> //for workqueue
#include <linux/interrupt.h>
#include <net/net_namespace.h>
#include <linux/genetlink.h>    //for netlink 
#include <net/genetlink.h>  //for netlink
#include <net/mptcp.h>

#define SMLog			mptcp_sm_debug

#define IS_REGI_STATE   (atomic_read(&gSmNlState)==ST_REG)
#define GET_STATE       atomic_read(&gSmNlState)
#define SET_STATE(_s)   atomic_set(&gSmNlState, _s)

#define IS_CCB_UP(_c)   (atomic_read(&((_c)->up))==1)
#define CCB_DOWN(_c)    atomic_set(&((_c)->up), 0)
#define CCB_UP(_c)      atomic_set(&((_c)->up), 1)

#define SM_NL_GENET_FMNAME  	"SM_NL" 
#define SM_NL_GENET_VERSION 	0x01

#define MAGW_PROC_NAME_DEF  	"mgwd" 
#define MAGW_PROC_PORT_DEF  	7683
#define MAGW_PROC_NAME_MAX  	16 

//#define MAX_SM_CCB          	12
#define MAX_SM_CCB          	20

#define GET_CONNECTED_APP_INFO_RETRUN(_tok, _pInfo)   do {\
ncCCB  *_ccb;                                      \
u8		_rtry = 0;                                 \
  _ccb = &(gSmNlCcb[_tok%gSmNlCfg->nlCnt]);        \
__RETRY_TO_GET_IDLE:                               \
  if(IS_CCB_UP(_ccb)) {                            \
    _pInfo = &_ccb->gInfo[_ccb->gidx];             \
  } else {                                         \
    if(++_rtry > gSmNlCfg->nlCnt) {                \
		return -EPERM;                             \
	}                                              \
    _ccb = &(gSmNlCcb[_ccb->neighbor]);            \
	goto __RETRY_TO_GET_IDLE;                      \
  }                                                \
} while(0)

#define GET_SKB_HDR_RETURN(_skb, _hdr, _type)     do {\
  _skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);   \
  if(_skb == NULL) {                                  \
    pr_alert("[%s] Failed to nlmsg_new\n", __FUNCTION__);\
	return -ENOMEM;                                   \
  }                                                   \
  _hdr = genlmsg_put(_skb, 0, 0, &fm_sm_nl, 0, _type);\
  if(_hdr == NULL) {                                  \
    pr_alert("[%s] Failed to genlmsg_put\n", __FUNCTION__);\
    nlmsg_free(_skb);                                 \
	return -ENOMEM;                                   \
  }                                                   \
} while(0)

#define GET_SMMSG(_skb, _sMsg)                     do {\
struct nlattr  *_pa;                                   \
  _pa = nla_reserve(_skb, SM_ATTR_MSG, sizeof(SMMsg)); \
  _sMsg  = (SMMsg*)nla_data(_pa);                      \
} while(0)

#define SET_SMMSG_COMMON(_sM, _mpcb, _tp)      { \
  (_sM)->mTok = (_mpcb)->mptcp_loc_token;        \
  (_sM)->mSc  = (_mpcb)->cnt_subflows;           \
  (_sM)->mPi  = (_tp)->mptcp->path_index;        \
}

#define SET_SMMSG_ADDR4(_sM, _sk, _tp)            do { \
  (_sM)->sAddr.ver = AF_INET;                          \
  (_sM)->sAddr.port =                                  \
	          ((struct inet_sock *)(_tp))->inet_sport; \
  (_sM)->sAddr.u.v4.s_addr =                           \
	          ((struct inet_sock *)(_tp))->inet_saddr; \
  (_sM)->dAddr.ver = AF_INET;                          \
  (_sM)->dAddr.port =                                  \
	           ((struct inet_sock *)(_tp))->inet_dport;\
  (_sM)->dAddr.u.v4.s_addr =                           \
	          ((struct inet_sock *)(_tp))->inet_daddr; \
} while(0)

#define SET_SMMSG_ADDR6(_sM, _sk, _tp)            do  { \
  (_sM)->sAddr.ver = AF_INET6;                          \
  (_sM)->sAddr.port = ((struct inet_sock *)(_tp))->inet_sport; \
  memcpy(&((_sM)->sAddr.u.v6),                          \
		 &(inet6_sk((const struct sock *)(_sk))->saddr),\
		  sizeof(struct in6_addr));                     \
  (_sM)->dAddr.ver = AF_INET6;                          \
  (_sM)->dAddr.port = ((struct inet_sock *)(_tp))->inet_dport; \
  memcpy(&((_sM)->dAddr.u.v6),                          \
		 &((_sk)->sk_v6_daddr),                         \
		  sizeof(struct in6_addr));                     \
} while(0)

#define SET_SMMSG_STAT(_sM, _tp)                                do {\
  (_sM)->stat[(_sM)->stCnt].pi =    (_tp)->mptcp->path_index;       \
  (_sM)->stat[(_sM)->stCnt].rxPkt = (_tp)->mptcp->nc_stat->rxPkts;  \
  (_sM)->stat[(_sM)->stCnt].rxOct = (_tp)->mptcp->nc_stat->rxOctets;\
  (_sM)->stat[(_sM)->stCnt].txPkt = (_tp)->mptcp->nc_stat->txPkts;  \
  (_sM)->stat[(_sM)->stCnt].txOct = (_tp)->mptcp->nc_stat->txOctets;\
}while (0)

/* ------------------------------------------------------------------- */
/* Typedefs             */
/* ------------------------------------------------------------------- */
typedef struct task_struct ncTsk;
typedef struct genl_info   ncGeInf;
typedef atomic_t           ncAtom;
    
/* ------------------------------------------------------------------- */
/* For General  */
/* ------------------------------------------------------------------- */
/* For Configuration  */
typedef struct _nc_config {
    u16     nlCnt;    /* netlink count == MAX_SM_CCB */
    /* WatchDog Process Name */
    u16     wPort;
    char    wProcName[16 /* MAGW_PROC_NAME_MAX */]; 
} ncCfg;

/* ------------------------------------------------------------------- */
/* For App Connection Management  */
typedef struct _nc_ccb {
    u32     idx;
    u32     neighbor; /* For Circular operation */

    atomic_t up;

    u32      gidx;
    ncGeInf  gInfo[2];
} ncCCB;

/* ------------------------------------------------------------------- */
/* For FSM  */
/* ------------------------------------------------------------------- */
typedef enum {
    ST_UNKNW,
    ST_DEAD,
    ST_RUN,
    ST_REG,
    ST_MAX,
} TSK_STATE;

typedef enum {
    EV_DOWN,
    EV_UP,
    EV_REG,
    EV_MAX,
} TSK_EVENT;
typedef void (*tskSmNlStateFnc) (void);
extern tskSmNlStateFnc tskSmNlFsmMt[ST_MAX][EV_MAX];

/* ------------------------------------------------------------------- */
/* For Generic Netlink  */
/* ------------------------------------------------------------------- */
enum sm_nl_msg_types {
   SM_REQ_REGIST = 0,
   SM_NTF_NEW_FST_SESS,	/* First Subflow */
   SM_NTF_NEW_SUB_SESS, /* Any Other Subflow after first one */
   SM_NTF_DEL_SUB_SESS, /* Close a subflow including first(master) */
   SM_NTF_DSTRY_SESS,	/* Close a Mptcp session */
   SM_REQ_LTE_RTB,  /* LTE ADDRESS RANGE TABLE  */
   SM_CMD_MAX
};

enum sm_nl_attr {
   SM_ATTR_UNSPEC,
   SM_ATTR_INDEX,
   SM_ATTR_RESULT,
   SM_ATTR_MSG,
   SM_ATTR_RTB,
   __SM_ATTR_MAX,
};
#define SM_ATTR_MAX (__SM_ATTR_MAX - 1)

/* ------------------------------------------------------------------- */
/* For Communction  */
/* ------------------------------------------------------------------- */
/* Base Structure */

typedef struct _addr {
    u8     ver;  //AF_INET(2), AF_INET6(10)
    u8     padd[1];
    u16    port;
    union {
        struct in_addr  v4;
        struct in6_addr v6;
    }u;
}ADDR;
typedef struct _nc_mptcp_stat {
    u8          pi; /* path index */
    u8          padd[3];
    u64         rxPkt;
    u64         rxOct;
    u64         txPkt;
    u64         txOct;
} STAT;
typedef struct _nc_mptcp_conn_info {
    u32         tok;
    u8          pi;  /* path index */
    u8          sc;  /* subflow count */
    u8          padd[2];
} CONN;

#define NL_NEW_FST_SESS     0x01
#define NL_NEW_SUB_SESS     0x02
#define NL_DEL_SUB_SESS     0x03
#define NL_DSTRY_SESS       0x04
typedef struct _nc_sm_nl_info {
	/* MPTCP Connection Info */
    CONN        conn;
#define mTok    conn.tok
#define mPi     conn.pi
#define mSc     conn.sc

	/* Address Info */
    ADDR        sAddr;
    ADDR        dAddr;

	/* MPTCP Statistic  */
    u8          stCnt;
    u8          stPad[3];
    STAT        stat[8];

} SMMsg;


#if 0
typedef struct _range_4_lte {
    uint32_t    used;
    uint32_t    s;
    uint32_t    e;
} R4L;

typedef struct _range_tb {
    int         cnt;
#define MAX_RTB_ROWS    200
    R4L         r[MAX_RTB_ROWS];
} RTB;
#endif

/* ------------------------------------------------------------------- */
/* Function Prototypes */
/* ------------------------------------------------------------------- */
extern ncCfg    gSmNlCfg[];
extern ncTsk   *gSmNlTsk;
extern ncCCB    gSmNlCcb[];
extern ncAtom 	gSmNlState;

/* ------------------------------------------------------------------- */
/* Function Prototypes */
/* ------------------------------------------------------------------- */
//static int sm_nl_send_mew_sess(void);
int sm_regist          (struct sk_buff *skb, struct genl_info *info);
int sm_lte_rtb         (struct sk_buff *skb, struct genl_info *info);
inline 
int smSndFstSess       (struct mptcp_cb *mpcb, struct sock *sk);
inline 
int smSndSubSess       (struct mptcp_cb *mpcb, struct sock *sk);
inline 
int smSndDelSubs       (struct mptcp_cb *mpcb, struct sock *sk);
inline 
int smSndDstSess       (struct mptcp_cb *mpcb, struct sock *sk);

int thrWd(void *arg);

void nl_on_destroy_sess(struct mptcp_cb *mpcb, struct sock *sk);
void nl_on_del_sess    (struct mptcp_cb *mpcb, struct sock *sk);
void nl_on_new_sub_sess(struct mptcp_cb *mpcb, struct sock *sk);
void nl_on_new_mas_sess(struct mptcp_cb *mpcb, struct sock *sk);

#endif /* __NC_NL_H__ */
/************************************************************************
         End of file:   
************************************************************************/

/************************************************************************
     ver       pat    init                  description
------------ -------- ---- ----------------------------------------------
/main/1      ---      frau  1. Initial Release
************************************************************************/

