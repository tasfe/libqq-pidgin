/**
 * @file qq.h
 *
 * purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#ifndef _QQ_QQ_H_
#define _QQ_QQ_H_

#include "internal.h"
#include "ft.h"
#include "circbuffer.h"
#include "dnsquery.h"
#include "dnssrv.h"
#include "proxy.h"
#include "roomlist.h"

#define QQ_KEY_LENGTH       16

/* steal from kazehakase :) */
#define qq_strlen(s) ((s)!=NULL?strlen(s):0)
#define qq_strcmp(s1,s2) ((s1)!=NULL && (s2)!=NULL?strcmp(s1,s2):0)

typedef struct _qq_data qq_data;
typedef struct _qq_buddy_data qq_buddy_data;
typedef struct _qq_interval qq_interval;
typedef struct _qq_net_stat qq_net_stat;
typedef struct _qq_login_data qq_login_data;
typedef struct _qq_captcha_data qq_captcha_data;

struct _qq_captcha_data {
	guint8 *token;
	guint16 token_len;
	guint8 next_index;
	guint8 *data;
	guint16 data_len;
};

struct _qq_login_data {
	guint8 random_key[QQ_KEY_LENGTH];			/* first encrypt key generated by client */
	guint8 *token_touch;				/* get from server */
	guint16 token_touch_len;
	guint8 *token_captcha;			/* get from server */
	guint16 token_captcha_len;

	guint8 pwd_md5[QQ_KEY_LENGTH];			/* password in md5 (or md5' md5) */
	guint8 pwd_twice_md5[QQ_KEY_LENGTH];

	guint8 **token_auth;
	guint16 token_auth_len[3];
	guint8 keys[4][QQ_KEY_LENGTH];	/* 0,Key to VerifyE5&VerifyE3&Login Request,
																			Sometimes to Login Response;
																		1,Key to VerifyE5 Response;
																		2,Key to Login Response;
																		3,Key to VerifyE3 Response */

	guint8 **token_verify;
	guint16 token_verify_len[3];
	guint32 login_fill;

	guint8 *token_login;
	guint16 token_login_len;
};

struct _qq_interval {
	gint resend;
	gint keep_alive;
	gint update;
};

struct _qq_net_stat {
	glong sent;
	glong resend;
	glong lost;
	glong rcved;
	glong rcved_dup;
};

struct _qq_buddy_data {
	guint32 uid;
	guint16 face;		/* index: 0 - 299 */
	guint8 age;
	guint8 gender;
	gchar *nickname;
	struct in_addr ip;
	guint16 port;
	guint8 status;
	guint8 ext_flag;
	guint8 comm_flag;	/* details in qq_buddy_list.c */
	guint16 client_tag;
	guint8 onlineTime;
	guint16 level;
	guint16 timeRemainder;
	time_t signon;
	time_t idle;
	time_t last_update;
	gint8  role;		/* role in group, used only in group->members list */
};

typedef struct _qq_connection qq_connection;
struct _qq_connection {
	int fd;				/* socket file handler */
	int input_handler;

	/* tcp related */
	int can_write_handler; 	/* use in tcp_send_out */
	PurpleCircBuffer *tcp_txbuf;
	guint8 *tcp_rxqueue;
	int tcp_rxlen;
};

struct _qq_data {
	PurpleConnection *gc;

	GSList *openconns;
	gboolean use_tcp;		/* network in tcp or udp */
	PurpleProxyConnectData *conn_data;
#ifndef purple_proxy_connect_udp
	PurpleDnsQueryData *udp_query_data;		/* udp related */
	gint udp_can_write_handler; 	/* socket can_write handle, use in udp connecting and tcp send out */
#endif
	gint fd;							/* socket file handler */
	qq_net_stat net_stat;

	GList *servers;
	gchar *curr_server;		/* point to servers->data, do not free*/

	guint16 client_tag;
	gint client_version;


	struct in_addr redirect_ip;
	guint16 redirect_port;
	guint8 *redirect;
	guint8 redirect_len;
	guint8 redirect_times;

	guint check_watcher;
	guint connect_watcher;
	gint connect_retry;

	qq_interval itv_config;
	qq_interval itv_count;
	guint network_watcher;
	gint resend_times;

	GList *transactions;	/* check ack packet and resend */

	guint32 uid;			/* QQ number */
	gchar * nickname;	/* QQ nickname */

	qq_login_data ld;
	qq_captcha_data captcha;

	guint8 session_key[QQ_KEY_LENGTH];		/* later use this as key in this session */
	guint8 session_md5[QQ_KEY_LENGTH];		/* concatenate my uid with session_key and md5 it */

	guint16 send_seq;		/* send sequence number */
	guint8 login_mode;		/* online of invisible */
	gboolean is_login;		/* used by qq_add_buddy */

	PurpleXfer *xfer;			/* file transfer handler */

	/* get from login reply packet */
	struct in_addr my_local_ip;			/* my local ip address detected by server */
	//guint16 my_local_port;		/* my lcoal port detected by server */
	time_t login_time;
	time_t last_login_time[3];
	struct in_addr last_login_ip;
	/* get from keep_alive packet */
	struct in_addr my_ip;			/* my ip address detected by server */
	guint16 my_port;		/* my port detected by server */
	guint16 my_icon;		/* my icon index */
	guint32 online_total;		/* the number of online QQ users */
	time_t online_last_update;		/* last time send get_friends_online packet */
	guint8 onlineTime;
	guint16 level;
	guint16 activeDays;

	GSList * buddy_list;
	GSList * group_list;

	PurpleRoomlist *roomlist;
	GSList *rooms;

	gboolean is_show_notice;
	gboolean is_show_news;
	gboolean is_show_chat;

	guint16 send_im_id;		/* send IM sequence number */
};

#endif
