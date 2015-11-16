/************************************************************************

     Name:     PRJ main function

     Type:     C Header file

     Desc:     PRJ Framework 

     File:     nc_nl.c

     Sid:      nc_nl.c

     Prg:      frau / comate

************************************************************************/
/* header */
#include "nc_nl.h"


/* ------------------------------------------------------------------- */
/* Global Variables  */
/* -------------------------------------------------------------------------- */
ncCfg    gSmNlCfg[1];
ncCCB    gSmNlCcb[MAX_SM_CCB];

ncAtom 	 gSmNlState = ATOMIC_INIT(ST_UNKNW);
ncTsk   *gSmNlTsk = NULL;

/* ------------------------------------------------------------------- */
/* Ge Netlink  */
/* ------------------------------------------------------------------- */
/* family definition */
static
struct genl_family fm_sm_nl = {
    .id         = GENL_ID_GENERATE,
    .hdrsize    = 0,
    .name       = SM_NL_GENET_FMNAME,
    .version    = SM_NL_GENET_VERSION,
    .maxattr    = SM_ATTR_MAX,
};

/* attribute policy */
static
struct nla_policy  pl_sm_nl[SM_ATTR_MAX+1] =
{
    [SM_ATTR_INDEX]  = {.type = NLA_U32},
    [SM_ATTR_RESULT] = {.type = NLA_U32},
    [SM_ATTR_MSG]    = {.type = NLA_BINARY,
                        .len = sizeof(SMMsg)},
    [SM_ATTR_RTB]  = {.type = NLA_BINARY,
                        .len = sizeof(RTB)},
};

/* operation definition */
static
struct genl_ops   op_sm_nl[] = {
    {
        .cmd    = SM_REQ_REGIST,
        .doit   = sm_regist,
        .policy = pl_sm_nl,
    },
    {
        .cmd    = SM_REQ_LTE_RTB,
        .doit   = sm_lte_rtb,
        .policy = pl_sm_nl,
    },
};

struct mptcp_sm_ops mptcp_sm_nl = {
    .on_new_master_session = nl_on_new_mas_sess,
    .on_new_sub_session    = nl_on_new_sub_sess,
    .on_del_session        = nl_on_del_sess,
    .on_destory_session    = nl_on_destroy_sess,
    .name = "sm_nl",
    .owner = THIS_MODULE,
};

/* -------------------------------------------------------------------------- */
void nl_on_new_mas_sess(struct mptcp_cb *mpcb, struct sock *sk)
{
struct tcp_sock *tp = tcp_sk(sk);

	if(((struct inet_sock *)tp)->inet_sport == mptcp_gw_port)
	{
		if(isIPv4LTE(sk->sk_daddr)) {
			/* This is a user land application */
			mptcp_meta_tp(tp)->netProto = 2; /* LTE */
			/* It's for me */
			tp->netProto = 2; /* LTE */
		}
		else {
			mptcp_meta_tp(tp)->netProto = 1; /* WIFI */
			tp->netProto = 1; /* WIFI */
		} /* end of if */

		/* For Delete and Destroy */
		mpcb->isReportDestroy = 1; 

		smSndFstSess(mpcb, sk);
	}
	else
		mpcb->isReportDestroy = 0;

#if 1
    if(sk->sk_family == AF_INET)
    {
        SMLog("SM[%20s][%#x] [pi:%d/%d] src_addr:%pI4:%d dst_addr:%pI4:%d PROTO[%d/%d]\n",
            __FUNCTION__,
            mpcb->mptcp_loc_token, tp->mptcp->path_index, mpcb->cnt_subflows,
            &((struct inet_sock *)tp)->inet_saddr,
             ntohs(((struct inet_sock *)tp)->inet_sport),
            &((struct inet_sock *)tp)->inet_daddr,
             ntohs(((struct inet_sock *)tp)->inet_dport), 
			 mptcp_meta_tp(tp)->netProto, tp->netProto);
    }
#if IS_ENABLED(CONFIG_IPV6)
    else
    {
        SMLog("SM[%20s][%#x] [pi:%d/%d] src_addr:%pI6c:%d dst_addr:%pI6c:%d PROTO[%d/%d]\n",
            __FUNCTION__,
            mpcb->mptcp_loc_token, tp->mptcp->path_index, mpcb->cnt_subflows,
            &inet6_sk((const struct sock *)sk)->saddr,
            ntohs(((struct inet_sock *)tp)->inet_sport),
            &sk->sk_v6_daddr,
            ntohs(((struct inet_sock *)tp)->inet_dport),
			mptcp_meta_tp(tp)->netProto, tp->netProto);
    }
#endif
#endif

	return;
} /* end of function */

/* -------------------------------------------------------------------------- */
void nl_on_new_sub_sess(struct mptcp_cb *mpcb, struct sock *sk)
{
struct tcp_sock *tp = tcp_sk(sk);

	if(((struct inet_sock *)tp)->inet_sport == mptcp_gw_port)
	{
		if(isIPv4LTE(sk->sk_daddr)) {
			/* It's for me */
			tp->netProto = 2; /* LTE */
		}
		else {
			tp->netProto = 1; /* WIFI */
		}
		smSndSubSess(mpcb, sk);
	}

#if 1
	if(sk->sk_family == AF_INET)
	{
		SMLog("SM[%20s][%#x] [pi:%d/%d] src_addr:%pI4:%d dst_addr:%pI4:%d PROTO[%d/%d]\n",
			__FUNCTION__,
		   	mpcb->mptcp_loc_token, tp->mptcp->path_index, mpcb->cnt_subflows, 
            &((struct inet_sock *)tp)->inet_saddr, 
			ntohs(((struct inet_sock *)tp)->inet_sport),
            &((struct inet_sock *)tp)->inet_daddr, 
			ntohs(((struct inet_sock *)tp)->inet_dport),
			mptcp_meta_tp(tp)->netProto, tp->netProto);
	}
#if IS_ENABLED(CONFIG_IPV6)
	else
	{
		SMLog("SM[%20s][%#x] [pi:%d/%d] src_addr:%pI6c:%d dst_addr:%pI6c:%d PROTO[%d/%d]\n",
			__FUNCTION__,
		    mpcb->mptcp_loc_token, tp->mptcp->path_index, mpcb->cnt_subflows,
			&inet6_sk((const struct sock *)sk)->saddr, 
			ntohs(((struct inet_sock *)tp)->inet_sport),
            &sk->sk_v6_daddr, 
			ntohs(((struct inet_sock *)tp)->inet_dport),
			mptcp_meta_tp(tp)->netProto, tp->netProto);
	}
#endif
#endif
	return;
} /* end of funtion */

/* -------------------------------------------------------------------------- */
void nl_on_del_sess(struct mptcp_cb *mpcb, struct sock *sk)
{
  	if(mpcb->isReportDestroy) {
		smSndDelSubs(mpcb, sk);
	}

#if 0
struct tcp_sock *tp = tcp_sk(sk);

	if(sk->sk_family == AF_INET)
	{
		SMLog("SM[%20s][%#x] [pi:%d/%d] src_addr:%pI4:%d dst_addr:%pI4:%d\n",
			__FUNCTION__,
		    mpcb->mptcp_loc_token, tp->mptcp->path_index, mpcb->cnt_subflows,
            &((struct inet_sock *)tp)->inet_saddr, 
			ntohs(((struct inet_sock *)tp)->inet_sport),
            &((struct inet_sock *)tp)->inet_daddr, 
			ntohs(((struct inet_sock *)tp)->inet_dport));

	}
#if IS_ENABLED(CONFIG_IPV6)
	else
	{
		SMLog("SM[%20s][%#x] [pi:%d/%d] src_addr:%pI6:%d dst_addr:%pI6:%d\n",
			__FUNCTION__,
		    mpcb->mptcp_loc_token, tp->mptcp->path_index, mpcb->cnt_subflows,
			&inet6_sk(sk)->saddr, 
			ntohs(((struct inet_sock *)tp)->inet_sport),
            &sk->sk_v6_daddr, 
			ntohs(((struct inet_sock *)tp)->inet_dport));
	}
#endif

	SMLog("SM[%20s][%#x]  Rx[%llu, %llu bytes] Tx[%llu, %llu bytes]\n",
			__FUNCTION__,
		    mpcb->mptcp_loc_token, 
		    tp->mptcp->nc_stat->rxPkts, tp->mptcp->nc_stat->rxOctets,
			tp->mptcp->nc_stat->txPkts, tp->mptcp->nc_stat->txOctets);

	if(mpcb->cnt_subflows <=1)
		goto END;

struct sock     *sk2;
struct tcp_sock *tp2;
	mptcp_for_each_sk(mpcb, sk2) 
	{
		tp2 = tcp_sk(sk2);

		SMLog("SM[%20s][         ][pi:%d]  Rx[%llu, %llu bytes] Tx[%llu, %llu bytes]\n",
			__FUNCTION__,
		   	tp2->mptcp->path_index, 
			tp2->mptcp->nc_stat->rxPkts, tp2->mptcp->nc_stat->rxOctets,
			tp2->mptcp->nc_stat->txPkts, tp2->mptcp->nc_stat->txOctets);
	}

END:
#endif
	return;
} /* end of function */

/* -------------------------------------------------------------------------- */
void nl_on_destroy_sess(struct mptcp_cb *mpcb, struct sock *sk)
{
  	if(mpcb->isReportDestroy) {
		smSndDstSess(mpcb, sk);
	}

	return;
} /* end of function */



/* -------------------------------------------------------------------------- */
/* application register nelink */
/* -------------------------------------------------------------------------- */
int sm_regist(struct sk_buff *skb, struct genl_info *info)
{
struct sk_buff *msg;
struct nlattr  *na;
void *hdr;
int ret = 0;
int result = 0;
uint32_t index = 1;
ncCCB  *ccb;

    if(info == NULL) return -EINVAL;

    na = info->attrs[SM_ATTR_INDEX];
    if(na)
    {
        index = nla_get_u32(na);
        if(index >= MAX_SM_CCB)
        {
            pr_alert("[%s] ERROR ????? Invalid index:%d\n",
                    __func__, index );
            result=9; //error
        }
    }
    else
    {
        pr_alert("[%s] ERROR?? No info->attr..\n", __func__ );
        result=9; //error
    }

    msg = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
    if(msg == NULL)
    {
        pr_alert("[%s] Failed to genlmsg_new\n", __func__ );
        return -ENOMEM;
    } /* end of if */

    hdr = genlmsg_put(msg, info->snd_portid, info->snd_seq,
                        &fm_sm_nl, 0, SM_REQ_REGIST);
    if(hdr == NULL)
    {
        pr_alert("[%s] Failed to genlmsg_put \n", __func__);
        ret = -ENOBUFS;
        goto ERROR;
    } /* end of if */

    ret = nla_put_u32(msg, SM_ATTR_RESULT, result);
    if(ret < 0)
    {
        pr_alert("[%s] Failed to nla_put_u32 \n", __func__);
        goto ERROR;
    }
    genlmsg_end(msg, hdr);

    if(!result)
    {
        int tidx = 0;
#if 1
        pr_alert("[%s] Try Register APP Port Id [%u] \n", __func__, 
                        info->snd_portid);
#endif
        ccb = &gSmNlCcb[index];
        if(IS_CCB_UP(ccb))
        {
            tidx = (ccb->gidx+1)%2;
            memcpy(&ccb->gInfo[tidx], info, sizeof(struct genl_info));
            ccb->gidx = tidx;
        }
        else
        {
            memcpy(&ccb->gInfo[ccb->gidx], info, sizeof(struct genl_info));
        }
    }

    ret = genlmsg_unicast(genl_info_net(info), msg, info->snd_portid);
    if(ret < 0)
    {
        pr_alert("[%s] Failed to genlmsg_unicast: ret:%d \n",
                            __func__, ret);
        goto ERROR;
    }

    if(!result)
    {
        CCB_UP(ccb);
        tskSmNlFsmMt[GET_STATE][EV_REG]();
    }

    return 0;

ERROR:
    nlmsg_free(msg);
    return ret;
} /* end of function */

/* -------------------------------------------------------------------------- */
int sm_lte_rtb(struct sk_buff *skb, struct genl_info *info)
{
struct sk_buff *msg;
struct nlattr  *na;
void *hdr;
int ret = 0;
int result = 0;
RTB *rtb = NULL;

    if(info == NULL) return -EINVAL;

    na = info->attrs[SM_ATTR_RTB];
    if(na)
    {
        rtb = (RTB*)nla_data(na);

		if(rtb)
			lfRTBUpdate(rtb);
		else
        {
            pr_alert("[%s] ERROR ????? Invalid RTB:%p\n",
                    __func__, rtb);
            result=9; //error
        }
    }
    else
    {
        pr_alert("[%s] ERROR?? No info->attr..\n", __func__ );
        result=9; //error
    }

    msg = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
    if(msg == NULL)
    {
        pr_alert("[%s] Failed to genlmsg_new\n", __func__ );
        return -ENOMEM;
    } /* end of if */

    hdr = genlmsg_put(msg, info->snd_portid, info->snd_seq,
                        &fm_sm_nl, 0, SM_REQ_LTE_RTB);
    if(hdr == NULL)
    {
        pr_alert("[%s] Failed to genlmsg_put \n", __func__);
        ret = -ENOBUFS;
        goto ERROR;
    } /* end of if */

    ret = nla_put_u32(msg, SM_ATTR_RESULT, result);
    if(ret < 0)
    {
        pr_alert("[%s] Failed to nla_put_u32 \n", __func__);
        goto ERROR;
    }
    genlmsg_end(msg, hdr);

    ret = genlmsg_unicast(genl_info_net(info), msg, info->snd_portid);
    if(ret < 0)
    {
        pr_alert("[%s] Failed to genlmsg_unicast: ret:%d \n",
                            __func__, ret);
        goto ERROR;
    }

    return 0;

ERROR:
    nlmsg_free(msg);
    return ret;
} /* end of function */



/* -------------------------------------------------------------------------- */
inline
int smSndFstSess(struct mptcp_cb *mpcb, struct sock *sk)
{
struct tcp_sock *tp = tcp_sk(sk);

/* for Message */
ncGeInf *pInfo = NULL;
SMMsg   *sMsg  = NULL;
void    *hdr   = NULL;
struct sk_buff *skb;

	//SMLog("SM[%20s][%#x] \n", __func__, mpcb->mptcp_loc_token);

    if(!IS_REGI_STATE) return -EPERM;
	GET_CONNECTED_APP_INFO_RETRUN(mpcb->mptcp_loc_token, pInfo);

	GET_SKB_HDR_RETURN(skb, hdr, SM_NTF_NEW_FST_SESS);

    /* add structure */
	GET_SMMSG(skb, sMsg);
	SET_SMMSG_COMMON(sMsg, mpcb, tp);

	if(sk->sk_family == AF_INET) {
		SET_SMMSG_ADDR4(sMsg, sk, tp);
	}
	else {
		SET_SMMSG_ADDR6(sMsg, sk, tp);
	}

    /* finalize the message */
    genlmsg_end(skb, hdr);

    return genlmsg_unicast(genl_info_net(pInfo), skb, pInfo->snd_portid);
} /* end of function */

/* -------------------------------------------------------------------------- */
inline
int smSndSubSess(struct mptcp_cb *mpcb, struct sock *sk)
{
struct tcp_sock *tp = tcp_sk(sk);

/* for Message */
struct sk_buff *skb;
void    *hdr  = NULL;
SMMsg   *sMsg = NULL;
ncGeInf *pInfo;
struct sock     *sk2;
struct tcp_sock *tp2;

	//SMLog("SM[%20s][%#x] \n", __func__, mpcb->mptcp_loc_token);

    if(!IS_REGI_STATE) return -EPERM;
	GET_CONNECTED_APP_INFO_RETRUN(mpcb->mptcp_loc_token, pInfo);

	GET_SKB_HDR_RETURN(skb, hdr, NL_NEW_SUB_SESS);

    /* add structure */
	GET_SMMSG(skb, sMsg);
	SET_SMMSG_COMMON(sMsg, mpcb, tp);

	if(sk->sk_family == AF_INET) {
		SET_SMMSG_ADDR4(sMsg, sk, tp);
	}
	else {
		SET_SMMSG_ADDR6(sMsg, sk, tp);
	}

	// SET_STAT
	sMsg->stCnt = 0;
	mptcp_for_each_sk(mpcb, sk2) {
		tp2 = tcp_sk(sk2);
		SET_SMMSG_STAT(sMsg, tp2);
#if 0
	SMLog("SM[%20s][         ][pi:%d]  Rx[%llu, %llu bytes] Tx[%llu, %llu bytes]\n",
			__FUNCTION__,
			tp2->mptcp->path_index, 
			tp2->mptcp->nc_stat->rxPkts, tp2->mptcp->nc_stat->rxOctets,
			tp2->mptcp->nc_stat->txPkts, tp2->mptcp->nc_stat->txOctets);

	SMLog("SM[%20s][ SMMsg   ][pi:%d]  Rx[%llu, %llu bytes] Tx[%llu, %llu bytes]\n",
			__FUNCTION__,
			sMsg->stat[sMsg->stCnt].pi, 
			sMsg->stat[sMsg->stCnt].rxPkt, 
			sMsg->stat[sMsg->stCnt].rxOct, 
			sMsg->stat[sMsg->stCnt].txPkt, 
			sMsg->stat[sMsg->stCnt].txOct);
#endif
		sMsg->stCnt++;
		if(sMsg->stCnt >= 8) break;
	} /* for each */

    /* finalize the message */
    genlmsg_end(skb, hdr);

    return genlmsg_unicast(genl_info_net(pInfo), skb, pInfo->snd_portid);
} /* end of function */

/* -------------------------------------------------------------------------- */
inline
int smSndDelSubs(struct mptcp_cb *mpcb, struct sock *sk)
{
struct tcp_sock *tp = tcp_sk(sk);
/* for Message */
struct sk_buff *skb;
void    *hdr  = NULL;
SMMsg   *sMsg = NULL;
ncGeInf *pInfo;
struct sock     *sk2;
struct tcp_sock *tp2;

	SMLog("SM[%20s][%#x]PI[%d] PROTO[%d/%d] \n", __func__, 
			mpcb->mptcp_loc_token, tp->mptcp->path_index, 
			mptcp_meta_tp(tp)->netProto, tp->netProto);

    if(!IS_REGI_STATE) return -EPERM;
	GET_CONNECTED_APP_INFO_RETRUN(mpcb->mptcp_loc_token, pInfo);

	GET_SKB_HDR_RETURN(skb, hdr, NL_DEL_SUB_SESS);

    /* add structure */
	GET_SMMSG(skb, sMsg);
	SET_SMMSG_COMMON(sMsg, mpcb, tp);

	if(sk->sk_family == AF_INET) {
		SET_SMMSG_ADDR4(sMsg, sk, tp);
	}
	else {
		SET_SMMSG_ADDR6(sMsg, sk, tp);
	}

	// SET_STAT
	sMsg->stCnt = 0;
	mptcp_for_each_sk(mpcb, sk2) {
		tp2 = tcp_sk(sk2);
		SET_SMMSG_STAT(sMsg, tp2);
#if 0
	SMLog("SM[%20s][         ][pi:%d]  Rx[%llu, %llu bytes] Tx[%llu, %llu bytes]\n",
			__FUNCTION__,
			tp2->mptcp->path_index, 
			tp2->mptcp->nc_stat->rxPkts, tp2->mptcp->nc_stat->rxOctets,
			tp2->mptcp->nc_stat->txPkts, tp2->mptcp->nc_stat->txOctets);

	SMLog("SM[%20s][ SMMsg%2d][pi:%d]  Rx[%llu, %llu bytes] Tx[%llu, %llu bytes]\n",
			__FUNCTION__,
			sMsg->stCnt,
			sMsg->stat[sMsg->stCnt].pi, 
			sMsg->stat[sMsg->stCnt].rxPkt, 
			sMsg->stat[sMsg->stCnt].rxOct, 
			sMsg->stat[sMsg->stCnt].txPkt, 
			sMsg->stat[sMsg->stCnt].txOct
		 );
#endif
		sMsg->stCnt++;
		if(sMsg->stCnt >= 8) break;
	} /* for each */

    /* finalize the message */
    genlmsg_end(skb, hdr);

    return genlmsg_unicast(genl_info_net(pInfo), skb, pInfo->snd_portid);
} /* end of function */


/* -------------------------------------------------------------------------- */
inline
int smSndDstSess(struct mptcp_cb *mpcb, struct sock *sk)
{
/* for Message */
struct sk_buff *skb;
void    *hdr = NULL;
SMMsg   *sMsg;
ncGeInf *pInfo;

	SMLog("SM[%20s][%#x]\n", __func__, mpcb->mptcp_loc_token);

    if(!IS_REGI_STATE) return -EPERM;
	GET_CONNECTED_APP_INFO_RETRUN(mpcb->mptcp_loc_token, pInfo);

	GET_SKB_HDR_RETURN(skb, hdr, NL_DSTRY_SESS);

    /* add structure */
	GET_SMMSG(skb, sMsg);
	sMsg->mTok = mpcb->mptcp_loc_token;
	sMsg->mSc  = mpcb->cnt_subflows;
	/* Do not read anything except mptcp_loc_token */

    /* finalize the message */
    genlmsg_end(skb, hdr);

    return genlmsg_unicast(genl_info_net(pInfo), skb, pInfo->snd_portid);
} /* end of function */


/* -------------------------------------------------------------------------- */
static 
int nl_init(void) 
{
int ret = -EINVAL, i;

    memset(gSmNlCfg, 0x00, sizeof(gSmNlCfg));
    snprintf(gSmNlCfg->wProcName, 16, "%s", MAGW_PROC_NAME_DEF);

#ifdef _INTERNAL_TEST_
    gSmNlCfg->wPort = htons(22);
   	pr_alert("[%9s] SM_NL Internal Test : 22 ports permitted.\n", 
				__FUNCTION__);
#else
    gSmNlCfg->wPort = sysctl_mptcp_gw_port;
   	pr_alert("[%9s] SM_NL : [%d] port permitted.\n", 
				__FUNCTION__, sysctl_mptcp_gw_port);
#endif
    gSmNlCfg->nlCnt = MAX_SM_CCB;

    memset(gSmNlCcb, 0x00, sizeof(gSmNlCcb));
    for(i=0; i<MAX_SM_CCB; i++)
    {
        gSmNlCcb[i].idx = i;
        gSmNlCcb[i].neighbor = (i+1)%MAX_SM_CCB;
        CCB_DOWN(&gSmNlCcb[i]);
		/* for Generic Netlink Net info. */
        gSmNlCcb[i].gidx = 0;
    }


    /* Watchdog */
    gSmNlTsk = kthread_run(thrWd, NULL, "thrProcWd_%d", 0);
    if(IS_ERR(gSmNlTsk))
    {
        gSmNlTsk = NULL;
        pr_alert("[%9s] SM_NL Failed to kthread_run\n", __FUNCTION__);
        goto ERROR_THREAD;
    }
   	pr_alert("[%9s] SM_NL Successfully launched watch-dog:%s\n", 
				__FUNCTION__, MAGW_PROC_NAME_DEF);


	/* GeNetlink */
    ret = genl_register_family_with_ops(&fm_sm_nl, op_sm_nl);
    if(ret < 0)
    {
        pr_alert("[%s] SM_NL Failed to genl_register_family_with_ops [%s]:%d \n",
                            __FUNCTION__, "SM_NL", ret);
        goto ERROR_REGI_FM;
    }
   	pr_alert("[%9s] SM_NL Successfully registerd at GeNetlink family[%s]\n", 
				__FUNCTION__, "SM_NL");
    pr_alert("[%9s] SM_NL PACKET Size[%ld] Cnt[%d]\n", 
				__FUNCTION__, sizeof(SMMsg), gSmNlCfg->nlCnt);

	/* MPTCP Sesson Monitor */
	if(mptcp_register_session_monitor(&mptcp_sm_nl))
	{
    	pr_alert("[%s] SM_NL Failed to mptcp_register_session_monitor\n",
				 __FUNCTION__);
        goto ERROR_REGI_SM;
	}
   	pr_alert("[%9s] SM_NL Successfully registerd at MPTCP\n", __FUNCTION__);
   	pr_alert("[%9s] SM_NL Initialized...\n", __FUNCTION__);

    return 0;

ERROR_REGI_SM:
    genl_unregister_family(&fm_sm_nl);

ERROR_REGI_FM:
    if(gSmNlTsk) kthread_stop(gSmNlTsk);

ERROR_THREAD:
    return -1;
}

/* -------------------------------------------------------------------------- */
static 
void nl_exit(void) 
{
	mptcp_unregister_session_monitor(&mptcp_sm_nl);
   	pr_info("[%9s] SM_NL mptcp_unregister_session_monitor.\n", __FUNCTION__);

    if(gSmNlTsk) {
		kthread_stop(gSmNlTsk);
   		pr_info("[%9s] SM_NL kthread_stop.\n", __FUNCTION__);
	}

    genl_unregister_family(&fm_sm_nl);
   	pr_info("[%9s] SM_NL genl_unregister_family.\n", __FUNCTION__);

   	pr_alert("[%9s] SM_NL Exited...\n", __FUNCTION__);

	return;
}

/* -------------------------------------------------------------------------- */
module_init(nl_init);
module_exit(nl_exit);


MODULE_AUTHOR("Do you WANNA know me? WHY?");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sesson Monitor By GeNelink");
MODULE_VERSION("1.4.2");

/************************************************************************
         End of file:   
************************************************************************/

/************************************************************************
     ver       pat    init                  description
------------ -------- ---- ----------------------------------------------
/main/1      ---      frau  1. Initial Release
************************************************************************/
