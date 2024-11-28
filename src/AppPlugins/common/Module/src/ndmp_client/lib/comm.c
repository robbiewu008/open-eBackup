/*                               -*- Mode: C -*- 
 * comm.c
 * 
 * Description     : NDMP common communication functions.
 *
 * Copyright (c) 1999 Network Appliance, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, MODULE_FREE of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 * 
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include "comm.h"
#include "log/Log.h"
#include "securec.h"
#include "ndmp_common.h"

#define BUSINESS_SOCK (0)
#define MAKE_LIF_IP_CONN   (0)
#define CANCEL_LIF_IP_CONN (1)

#ifdef _ARM1620_
#define LVOS_setsockopt(s, level, optname, optvalue, optlen)  \
    setsockopt(s, level, optname, optvalue, (socklen_t)(optlen))

#include "ctrl/vrf_intf.h"
#define MAX_HOST_NAME_LEN (128)
typedef struct LifIpConn
{
    bool isConn;
    char ipAddr1[MAX_HOST_NAME_LEN + 1];
    char ipAddr2[MAX_HOST_NAME_LEN + 1];
}LifIpConn;

LifIpConn g_lif_ip_connect_array[2];
#endif

/* try to use select() since it is already used in src/dump.c */
#if !defined(USE_POLL) && defined(FD_SET) && defined(FD_ZERO)
#define USE_SELECT
#endif

typedef struct MsgInfo
{
	ndmp_header			hdr;
	NdmpMsgHandler*		handler;
	void*				body;
} MsgInfo;

/* Connection data structure. */
typedef struct Connection
{
	int					sock;
	XDR					xdrs;
	u_long				mySequence;
	bool_t				authorized;
	bool_t				eof;
	MsgInfo				msginfo;		/* received request or reply message */
	NdmpMsgHandler*		msgHandlerTbl;
	u_short				version;
	void*				clientData;
} Connection;

typedef struct
{
    char           ipadd[64];
}CMR_USERABLE_IP_replace;

#ifndef _ARM1620_
static int
ndmp_readit(void*		connectionHandle,
			void*		buf,
			int			len);

static int
ndmp_writeit(void*		connectionHandle,
			 void*	    buf,
			 int		len);
#else
static int
ndmp_readit(char*		connectionHandle,
			caddr_t		buf,
			int			len);

static int
ndmp_writeit(char*		connectionHandle,
			 caddr_t	buf,
			 int		len);
#endif
static int
ndmp_recv_msg(Connection*	connection);

static int
ndmp_process_messages(Connection*	connection,
					  bool_t		reply_expected);

static NdmpMsgHandler*
ndmp_get_handler(Connection*	connection,
				 uint32_t		procedure);

/*
 * ndmpCreateConnection
 *
 * Allocate and initialize a connection structure.
 *
 * Parameters:
 *   handler_tbl (input) - message handlers.
 *
 * Returns:
 *   0 - error
 *  connection pointer
 *
 * Notes:
 *   The returned connection should be destroyed using
 *   ndmpDestroyConnection().
 */
NdmpConnection
ndmpCreateConnection(NdmpMsgHandler*	msgHandlerTbl)
{
	Connection*		connection;
	
	connection = (Connection*)malloc(sizeof(Connection));
	if (connection == 0)
	{
		ERRLOG("system call (malloc): %s.", strerror(errno));
		return(0);
	}

	connection->sock          = -1;
	connection->mySequence    = 0;
	connection->authorized    = FALSE;
	connection->eof           = FALSE;
	connection->msginfo.body  = 0;
	connection->msgHandlerTbl = msgHandlerTbl;
	connection->version       = NDMPVER;
	connection->clientData    = 0;
	
	connection->xdrs.x_ops    = 0;
	
	xdrrec_create(&connection->xdrs, 0, 0,
				  (caddr_t)connection,
				  ndmp_readit, ndmp_writeit);

	if (connection->xdrs.x_ops == 0)
	{
		ERRLOG("Xdrrec_create failed.");
		close(connection->sock);
		return(0);
	}

	return((NdmpConnection)connection);
}

/*
 * ndmpDestroyConnection
 *
 * Shutdown a connection and release allocated resources.
 *
 * Parameters:
 *   connectionHandle (Input) - connection handle.
 *
 * Returns:
 *   void
 */
void
ndmpDestroyConnection(NdmpConnection	connectionHandle)
{
	Connection*	connection = (Connection*)connectionHandle;

	INFOLOG("NdmpDestroyConnection.");

	if (connection->sock >= 0)
		(void)close(connection->sock);

	xdr_destroy(&connection->xdrs);
	free((void*)connection);
}

/*
 * ndmpClose
 *   Close a connection.
 *
 * Parameters:
 *   connectionHandle (Input) - connection handle.
 *
 * Returns:
 *   void
 */
void
ndmpClose(NdmpConnection	connectionHandle)
{
	Connection*	connection = (Connection*)connectionHandle;

	INFOLOG("Ndmp close.");

	if (connection->sock != -1)
	{
		close(connection->sock);
		connection->sock = -1;
	}

	connection->eof = TRUE;
}

void loopForNdmpRun(int server_socket, bool_t dofork, NdmpMsgHandler* msgHandlerTbl,
	NdmpConHandlerFunc*	conHandlerFunc)
{
	for (;;) {
		struct sockaddr_in	from;
		socklen_t		    from_len = sizeof(from);
		int					ns;
		pid_t				pid;
		
		if ((ns = accept(server_socket, (struct sockaddr *)&from,
						 &from_len)) < 0)
		{
			INFOLOG("System call (accept) error: %s.", strerror(errno));
			continue;
		}

		INFOLOG("Received connection from %s.", inet_ntoa(from.sin_addr));

		if (dofork == FALSE) {
			NdmpConnection	connection;
			
			if ((connection = ndmpCreateConnection(msgHandlerTbl)) == 0)
			{
				close(ns);
				continue;
			}
			
			((Connection*)connection)->sock = ns;
			
			(*conHandlerFunc)(connection);

			ndmpDestroyConnection(connection);
			continue;
		}
		
		pid = fork();
		
		if (pid < 0) {
			INFOLOG("System call (fork) error: %s.", strerror(errno));
			(void)close(ns);
			continue;
		}

		if (pid == 0) {
			/* Child process. */

			NdmpConnection	connection;
			
			(void)close(server_socket);
			
			if ((connection = ndmpCreateConnection(msgHandlerTbl)) == 0) {
				(void)close(ns);
				exit(1);
			}
			
			((Connection *)connection)->sock = ns;
			(*conHandlerFunc)(connection);
			ndmpDestroyConnection(connection);
			exit(0);
		}

		(void)close(ns);
	}
}
/*
 * ndmpRun
 *
 * Creates a socket for listening and accepting connections
 * from NDMP clients.
 * Accepts connections and passes each connection to the connection
 * handler.
 * If the dofork parameter is TRUE, a child process is forked and the
 * connection handler is run in the context of the child process.
 *
 * Parameters:
 *   port (input)   -  NDMP server port.
 *                     If 0, the port number will be retrieved from
 *                     the network service database. If not found there,
 *                     the default NDMP port number (from ndmp.x)
 *                     will be used.
 *   dofork (input)  - if TRUE, connection handler is run in the context
 *                     of a child process.
 *   handler (input) - connection handler function.
 *
 * Returns:
 *   void
 *
 * Notes:
 *   This function does not return unless encountering an error
 *   related to the listen socket.
 */
void
ndmpRun(u_long				port,
		bool_t				dofork,
		NdmpMsgHandler*		msgHandlerTbl,
		NdmpConHandlerFunc*	conHandlerFunc)
{
    struct sockaddr_in	sin;
	struct servent		*sp;
	int					server_socket;
    int 				on = 1;
	
	INFOLOG("Ndmp run.");

	/*
	 * If no port number was specified, get
	 * the port number from the network services database.
	 */
	if (port == 0) {
		if ((sp = getservbyname("ndmp", "tcp")) == 0)
			port = NDMPPORT;
		else
			port = sp->s_port;
	}
	
	sin.sin_family 		= AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port 		= htons(port);

	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		ERRLOG("System call (socket) error: %s.", strerror(errno));
		return;
	}
		
	(void) setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR,
					  (char *)&on, sizeof(on));

	if (bind(server_socket, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		INFOLOG("System call (bind) error: %s.", strerror(errno));
		close(server_socket);
		return;
	}
		
	if (listen(server_socket, 5) < 0) {
		INFOLOG("System call (listen) error: %s.", strerror(errno));
		close(server_socket);
		return;
	}

	INFOLOG("Ndmp running.");
	(void)loopForNdmpRun(server_socket, dofork, msgHandlerTbl, conHandlerFunc);
} 

//对�??连接信息,区分IPV4 和IPV6
typedef struct dma_connect_info_tag
{
    struct sockaddr_in sin;
    struct sockaddr_in6 sin6;
}dma_connect_info;

int ndmp_connect_ip_nonblock_select(int socket, int timeout, int *error)
{

    fd_set rdset, wrset;
    struct timeval tval;
    int rc = -1;

    FD_ZERO(&rdset);
    FD_SET(socket, &rdset);
    wrset = rdset;
    tval.tv_sec = timeout;
    tval.tv_usec = 0;

    rc = select(socket + 1, &rdset, &wrset, NULL, &tval);
    if (0 > rc) {
		ERRLOG("While create connetction, select failed.");
        return -1;
    }

    if (0 == rc) {
		ERRLOG("while create connection, connect timeout!");
        return -1;
    }

    if (FD_ISSET(socket, &rdset) || FD_ISSET(socket, &wrset)) {
        unsigned int len = sizeof(*error);
        if (getsockopt(socket, SOL_SOCKET, SO_ERROR, error, &len) < 0) {
			ERRLOG("while create connection , get sockopt failed!");
            return -1; 
        }
    } else {
		ERRLOG("while create connection , no set if ready");
        return -1;
    }

	return 0;
}

int ndmpd_connect_ip_nonblock(int socket, dma_connect_info dma_info, int ip_type, int timeout)
{
    int error = 0;
    int flags = 0;
    int rc = -1;

    if ((flags = fcntl(socket, F_GETFL, 0)) < 0) {
        return -1;
    }

    if (fcntl(socket, F_SETFL, flags | O_NONBLOCK) < 0) {
        return -1;        
    }

    if (ip_type == NDMP_ADDR_TCP) {
        rc = connect(socket, (struct sockaddr *)&dma_info.sin, sizeof (dma_info.sin));
    } else if (ip_type == NDMP_ADDR_TCP_IPV6) { 
        rc = connect(socket, (struct sockaddr *)&dma_info.sin6, sizeof (dma_info.sin6));
    }

    if (rc < 0) {
		int curError = errno;
		ERRLOG("connect failed. errno: %d", curError);
        if (EINPROGRESS != curError) {
		   ERRLOG("connect failed. errno: %d", errno);
            return -1;
        }
    }
	INFOLOG("connected!");

    if (0 == rc)
    {
        goto done;
    }

	rc = ndmp_connect_ip_nonblock_select(socket, timeout, &error);

done:
    if (fcntl(socket, F_SETFL, flags) < 0) {
		ERRLOG("while create connection, reset socket flags failed!");
        return -1;  
    }

    if (error) {
		ERRLOG("While create connection , the error is %d", error);
        return -1;
    }

    return 0;
}


static int bind_ipv6_addr(int *sock, char*addr, dma_connect_info dma_info, int time_out, int socktype)
{
    int rv = -1;
    struct sockaddr_in6 conn_sin6;

    (void)memset_s(&conn_sin6, sizeof(struct sockaddr_in6), 0, sizeof(struct sockaddr_in6));
    conn_sin6.sin6_family = AF_INET6;
    (void)socktype;
	INFOLOG("bind addr %s begin", addr);
    rv = inet_pton(AF_INET6, addr, &conn_sin6.sin6_addr); 
    if(0 == rv) /* listen_addr invalid */
    {
		ERRLOG("invalid listen addr %s , pls check it", addr);
        return -1; 
    }

    int tmp_sock = socket(AF_INET6, (int)SOCK_STREAM, 0);
    if (tmp_sock < 0)
    {
		ERRLOG("Socket error!");
        return (-1);
    }

    rv = bind(tmp_sock, (struct sockaddr *)&conn_sin6, sizeof (conn_sin6));
    if (rv < 0)
    {
		int err_no = errno;
		ERRLOG("bind failed %d", err_no);
        close(tmp_sock);
        tmp_sock = -1;
        return rv;
    }

    if (ndmpd_connect_ip_nonblock(tmp_sock, dma_info, NDMP_ADDR_TCP_IPV6, time_out) < 0)
    {
		ERRLOG("Connect error:");
        close(tmp_sock);
        tmp_sock = -1;
        return (-1);
    }

    *sock = tmp_sock;
    return 0;
}


static int connect_to_ipv4_addr(int *sock, dma_connect_info dma_info, int time_out, int socktype)
{
    int rv = -1;

    int tmp_sock = socket(AF_INET, (int)SOCK_STREAM, 0);
    if (tmp_sock < 0)
    {
	   ERRLOG("Socket error: ");
        return (-1);
    }

    if (ndmpd_connect_ip_nonblock(tmp_sock, dma_info, NDMP_ADDR_TCP, time_out) < 0)
    {
        close(tmp_sock);
        tmp_sock = -1;
        return (-1);
    }

    *sock = tmp_sock;
    return 0;
}

void set_socket_keepalive(NdmpConnection connection)
{
    int keepAlive = 1;
    int keepIdle = 600;//开启后多久开始检测，单位秒
    int keepInterval = 60;//开启检测后的时间间隔，单位秒
    int keepCount = 3;//检查断开的次数

    (void)setsockopt(((Connection*)connection)->sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(keepAlive));  

    (void)setsockopt(((Connection*)connection)->sock, SOL_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(keepIdle)); 
    (void)setsockopt(((Connection*)connection)->sock, SOL_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(keepInterval)); 
    (void)setsockopt(((Connection*)connection)->sock, SOL_TCP, TCP_KEEPCNT, &keepCount, sizeof(keepCount));

	struct timeval timeout;      
    timeout.tv_sec = 300;
    timeout.tv_usec = 0;
    (void)setsockopt (((Connection*)connection)->sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
        sizeof(timeout)); 
    (void)setsockopt (((Connection*)connection)->sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
        sizeof(timeout));
}

void initIpv4NdmpConnectparam(struct sockaddr_in *sin, char* host, u_long port)
{
    (void)memset_s((void*)sin, sizeof(struct sockaddr_in), 0, sizeof(struct sockaddr_in));
    sin->sin_family      = AF_INET;
    sin->sin_addr.s_addr = inet_addr(host);
    sin->sin_port        = htons(port);
}

int initIpv6NdmpConnectparam(struct sockaddr_in6 *sin6, char* host, u_long port)
{
    struct in6_addr psin_addr;
	
    (void)memset_s((void *)sin6, sizeof(struct sockaddr_in6), 0, sizeof(struct sockaddr_in6));
    sin6->sin6_family = AF_INET6;
    int rv = inet_pton(AF_INET6, host, (void *)&psin_addr);
    if (rv != 1) {
		ERRLOG("Inet_pton ipv6 addr(%s) error.", host);
		return rv;
    }
    sin6->sin6_addr = psin_addr;
    sin6->sin6_port = htons(port);
}

/*
 * ndmpConnect
 *
 * Establish a connection to an NDMP server.
 *
 * Parameters:
 *   connection (input) - connection handle.
 *   host (input) - NDMP server host name.
 *   port (input) - NDMP server port number.
 *                  if 0, the port number is retrieved from the
 *                  network service database. If not found there,
 *                  the default NDMP port number (from ndmp.x)
 *                  will be used.
 *
 * Returns:
 *   0 - successful connection.
 *  -1 - error.
 */
int
ndmpConnect(NdmpConnection	connection,
			char*			host,
			u_long          port)
{
	struct sockaddr_in	sin;
    struct sockaddr_in6 sin6;
	int					sock;
    int rc = 0;
    CMR_USERABLE_IP_replace *ip_info = NULL;
    int ip_count = 0;
    int idx = 0;
    int rv = 0;
    int timeout = 3;
    dma_connect_info dma_info;
    int ip_type = NDMP_ADDR_TCP;
    char *p = NULL;
    int makeLifConnRet = -1;
    
	INFOLOG("NdmpConnect: host:%s port:%d.", host, port);

    //判断ipv4和ipv6
	p = strchr(host, ':');
    if (NULL == p) {
		ip_type = NDMP_ADDR_TCP;
		initIpv4NdmpConnectparam(&sin, host, port);
        dma_info.sin = sin;
    } else {
        ip_type = NDMP_ADDR_TCP_IPV6;
		rc = initIpv6NdmpConnectparam(&sin6, host, port);
		if (rc != 1) {
			return rc;
		}
        dma_info.sin6 = sin6;
    }

	INFOLOG("Ip type(%d).", ip_type);

    if (ip_type == NDMP_ADDR_TCP) {
		rv = connect_to_ipv4_addr(&sock, dma_info, timeout, BUSINESS_SOCK);
    } else if(ip_type == NDMP_ADDR_TCP_IPV6) {
        rv = bind_ipv6_addr(&sock, ip_info[0].ipadd, dma_info, timeout, BUSINESS_SOCK);
    }
    
    MODULE_FREE(ip_info);
    if (rv != 0) {
		ERRLOG("bind_ip_addr err: %d", rv);
        close(sock);
        return NDMPCOPY_CONNECT_ERR;
    }

    ((Connection*)connection)->sock = sock;
    set_socket_keepalive(connection);

    return(0);
}

/*
 * ndmpPoll
 *
 * Poll for message data on the connection socket and
 * process received messages.
 *
 * Parameters:
 *   connection (input) - connection handle
 *   block (input)      - If TRUE, wait for data.
 *
 * Returns:
 *   1 - messages received and processed.
 *   0 - block FALSE and no data available.
 *  -1 - Error; connection no longer established.
 */
int
ndmpPoll(NdmpConnection	connectionHandle,
		 bool_t			block)
{
	Connection		*connection = (Connection *)connectionHandle;
#ifdef USE_SELECT
   fd_set          readfds;
   struct timeval  *timeout;
#else
	struct pollfd	fds[1];
	int				timeout;
#endif
	int				ret;

#ifdef USE_SELECT
	if (block)
		timeout = NULL;
	else
	{
		timeout = (struct timeval *) malloc(sizeof(struct timeval));
		timeout->tv_sec = timeout->tv_usec = 0;
	}

	FD_ZERO(&readfds);
	FD_SET(connection->sock, &readfds);

	ret = select(connection->sock + 1, &readfds, NULL, NULL, timeout);

	if (timeout != NULL)
		MODULE_FREE(timeout);
#else

	if (block)
		timeout = -1;
	else
		timeout = 0;
	
	fds[0].fd      = connection->sock;
	fds[0].events  = POLLIN|POLLRDNORM|POLLRDBAND|POLLPRI;

	ret = poll(fds, 1, timeout);
#endif

	if (ret < 0) {
		connection->eof = TRUE;
		(void)close(connection->sock);
		connection->sock = -1;
		return(-1);
	}

#ifdef USE_SELECT
	if (FD_ISSET(connection->sock, &readfds))
		return(ndmpProcessRequests(connectionHandle));
	else
		return(0);
#else
	if (ret > 0)
		return(ndmpProcessRequests(connectionHandle));
	else
		return(0);
#endif
}

/*
 * ndmpProcessRequests
 *
 * Reads the next request message into the stream buffer.
 * Processes messages until the stream buffer is empty.
 *
 * Parameters:
 *   connectionHandle (input) - connection handle.
 *
 * Returns:
 *   0 - 1 or more messages successfully processed.
 *  -1 - error; connection no longer established.
 */
int
ndmpProcessRequests(NdmpConnection	connectionHandle)
{
	Connection		*connection = (Connection *)connectionHandle;

	if (ndmp_process_messages(connection, 0) < 0)
		return(-1);

	return(0);
}

/*
 * ndmpSendRequest
 *
 * Send an NDMP request message.
 *
 * Parameters:
 *   connectionHandle  (input)  - connection pointer.
 *   message           (input)  - message number.
 *   err               (input)  - error code to place in header.
 *   request_data      (input)  - message body.
 *   reply_msg         (output) - reply message. If 0, reply will be
 *                                discarded.
 *
 * Returns:
 *   0        - successful send.
 *  -1        - error.
 *  otherwise - error from reply header.
 *
 * Notes:
 *   - The reply body is only returned if the error code is NDMP_NO_ERR.
 */
int
ndmpSendRequest(NdmpConnection	connectionHandle,
				ndmp_message	message,
				ndmp_error		err,
				void*			request_data,
				void**			reply)
{
	Connection		*connection = (Connection *)connectionHandle;
	ndmp_header		header;
	NdmpMsgHandler	*handler;
	struct timeval	time;
	/* Lookup info necessary for processing this request. */
	if (!(handler = ndmp_get_handler(connection, message))) {
		ERRLOG("sending message 0x%x: not supported.", message);
		return -1;
	}
  	
	gettimeofday(&time, 0);
	
	header.sequence 		= ++(connection->mySequence);
	header.time_stamp		= time.tv_sec;
	header.message_type		= NDMP_MESSAGE_REQUEST;
	header.message		    = message;
	header.reply_sequence	= 0;
	header.error			= err;
	ndmp_conn_prepare_header prepareHeader;
	prepareHeader.sequence 		= connection->mySequence;
	prepareHeader.time_stamp		= time.tv_sec;
	prepareHeader.message_type		= NDMP_MESSAGE_REQUEST;
	prepareHeader.message		    = message;
	prepareHeader.reply_sequence	= 0;
	prepareHeader.error			= err;
	prepareHeader.length = 1;	
	connection->xdrs.x_op = XDR_ENCODE;
	bool_t rc = 0;
	if(message == NDMP_CAB_DATA_CONN_PREPARE) {
		rc = xdr_ndmp_conn_prepare_header(&connection->xdrs, &prepareHeader);
	} else {
		rc = xdr_ndmp_header(&connection->xdrs, &header);
	}
	if (!rc) {
		ERRLOG("sending message 0x%x: encoding request header error.", message);
		xdrrec_endofrecord(&connection->xdrs, 1);
		return -1;
	}

	if (err == NDMP_NO_ERR && handler->xdr_request && request_data) {
		INFOLOG("send ndmp request : %x", message);
		if (!(*handler->xdr_request)(&connection->xdrs, request_data)) {
			ERRLOG("sending message 0x%x: encoding request body error.", message);
			xdrrec_endofrecord(&connection->xdrs, 1);
			return -1;
		}
	}
	xdrrec_endofrecord(&connection->xdrs, 1);

	if (handler->xdr_reply == 0) {
		ERRLOG("handler xdr reply is 0");
		return(0);
	}

	/*
	 * Process messages until the reply to this request has been
	 * processed.
	 */
	for (;;) {
		int			r;

		r = ndmp_process_messages(connection, TRUE);
		/* connection error? */
		if (r < 0) {
			ERRLOG("connection error, %d", r);
			return(-1);
		}

		/* no reply received? */
		if (r == 0)
			continue;

		/* reply received? */
		if (r == 1) {
			INFOLOG("reply received!");
			if (message != connection->msginfo.hdr.message) {
				ERRLOG("Received unexpected reply 0x%x.", connection->msginfo.hdr.message);
				ndmpFreeMessage(connectionHandle);
				return -1;
			}

			if (reply != 0) {
				*reply = connection->msginfo.body;
			} else {
				INFOLOG("reply is 0 , where on earth set it!");
				ndmpFreeMessage(connectionHandle);
			}
			
			return(connection->msginfo.hdr.error);
		}

		/* error handling reply */
		ERRLOG("error handling reply");
		return -1;
	}
	
	return(0);
}

/*
 * ndmpSendReply
 *   Send an NDMP reply message.
 *
 * Parameters:
 *   connectionHandle  (input)  - connection pointer.
 *   err               (input)  - error code to place in header.
 *   reply             (input)  - reply message body.
 *
 * Returns:
 *   0 - successful send.
 *  -1 - error.
 *
 * Notes:
 *   - The body is only sent if the error code is NDMP_NO_ERR.
 */
int
ndmpSendReply(NdmpConnection	connectionHandle,
			  ndmp_error		err,
			  void*				reply)
{
	Connection		*connection = (Connection *)connectionHandle;
	ndmp_header		header;
	struct timeval	time;
	gettimeofday(&time, 0);

	DBGLOG("NdmpSendReply: message:0x%x.",  connection->msginfo.hdr.message);

	header.sequence 		= ++(connection->mySequence);
	header.time_stamp		= time.tv_sec;
	header.message_type		= NDMP_MESSAGE_REPLY;
	header.message  		= connection->msginfo.hdr.message;
	header.reply_sequence	= connection->msginfo.hdr.sequence;
	header.error			= err;

	connection->xdrs.x_op = XDR_ENCODE;
	if (!xdr_ndmp_header(&connection->xdrs, &header)) {
		ERRLOG("Sending message 0x%x: encoding reply header error.", header.message);
		xdrrec_endofrecord(&connection->xdrs, 1);
		return(-1);
	}

	if (err == NDMP_NO_ERR &&
		connection->msginfo.handler->xdr_reply &&
		reply) {
		if (!(*connection->msginfo.handler->xdr_reply)(&connection->xdrs,
													   reply)) {
			ERRLOG("Sending message 0x%x: encoding reply body error.", header.message);
			xdrrec_endofrecord(&connection->xdrs, 1);
			return(-1);
		}
	}
	
	xdrrec_endofrecord(&connection->xdrs, 1);
	return(0);
}

void
ndmpFreeMessage(NdmpConnection	connectionHandle)
{
	Connection*	connection = (Connection *)connectionHandle;
	

	if (connection->msginfo.handler == 0 ||
		connection->msginfo.body == 0)
		return;

	connection->xdrs.x_op = XDR_FREE;
	if (connection->msginfo.hdr.message_type == NDMP_MESSAGE_REQUEST)
	{
		if (connection->msginfo.handler->xdr_request)
			(*connection->msginfo.handler->xdr_request)(&connection->xdrs,
														connection->msginfo.body);
	}
	else
	{
		if (connection->msginfo.handler->xdr_reply)
			(*connection->msginfo.handler->xdr_reply)(&connection->xdrs,
													  connection->msginfo.body);
	}

	MODULE_FREE(connection->msginfo.body);
	connection->msginfo.body = 0;
}

/*
 * ndmpGetFd
 *
 * Returns the connection file descriptor.
 *
 * Parameters:
 *   connectionHandle (input) - connection handle
 *
 * Returns:
 *   >=0 - file descriptor.
 *   -1  - connection not open.
 */
int
ndmpGetFd(NdmpConnection	connectionHandle)
{
	return(((Connection *)connectionHandle)->sock);
}

/*
 * ndmpSetClientData
 *
 * This function provides a means for the library client to provide
 * a pointer to some user data structure that is retrievable by
 * each message handler via ndmpGetClientData.
 *
 * Parameters:
 *   connectionHandle  (input) - connection handle.
 *   clientData        (input) - user data pointer.
 *
 * Returns:
 *   void
 */
void
ndmpSetClientData(NdmpConnection	connectionHandle,
				  void*				clientData)
{
	((Connection *)connectionHandle)->clientData = clientData;
}

/*
 * ndmpGetClientData
 *
 * This function provides a means for the library client to provide
 * a pointer to some user data structure that is retrievable by
 * each message handler via ndmpGetClientData.
 *
 * Parameters:
 *   connectionHandle (input) - connection handle.
 *
 * Returns:
 *   client data pointer.
 */
void*
ndmpGetClientData(NdmpConnection	connectionHandle)
{
	return(((Connection *)connectionHandle)->clientData);
}

/*
 * ndmpSetVersion
 *   Sets the NDMP protocol version to be used on the connection.
 *
 * Parameters:
 *   connectionHandle  (input) - connection handle.
 *   version           (input) - protocol version.
 *
 * Returns:
 *   void
 */
void
ndmpSetVersion(NdmpConnection	connectionHandle,
			   u_short			version)
{
	((Connection *)connectionHandle)->version = version;
}

/*
 * ndmpGetVersion
 *   Gets the NDMP protocol version in use on the connection.
 *
 * Parameters:
 *   connectionHandle  (input) - connection handle.
 *   version           (input) - protocol version.
 *
 * Returns:
 *   void
 */
u_short
ndmpGetVersion(NdmpConnection	connectionHandle)
{
	return(((Connection *)connectionHandle)->version);
}

/*
 * ndmpSetAuthorized
 *   Mark the connection as either having been authorized or not.
 *
 * Parameters:
 *   connectionHandle  (input) - connection handle.
 *   authorized        (input) - TRUE or FALSE.
 *
 * Returns:
 *   void
 */
void
ndmpSetAuthorized(NdmpConnection	connectionHandle,
				  bool_t			authorized)
{
	((Connection *)connectionHandle)->authorized = authorized;
}

u_longlong_t
quadToLongLong(ndmp_u_quad	quad)
{
	u_longlong_t	ull;
	ull = ((u_longlong_t)quad.high << 32) +	quad.low;
	return(ull);
}

ndmp_u_quad
longLongToQuad(u_longlong_t	ull)
{
	ndmp_u_quad	quad;

	quad.high = (u_long)(ull >> 32);
	quad.low  = (u_long)ull;
	return(quad);
}

#ifndef _ARM1620_

/************** private functions ****************************************/

/*
 * ndmp_readit
 *
 * Low level read routine called by the xdrrec library.
 *
 * Parameters:
 *   connection (input) - connection pointer.
 *   buf        (input) - location to store received data.
 *   len        (input) - max number of bytes to read.
 *
 * Returns:
 *   >0 - number of bytes received.
 *   -1 - error.
 */
static int
ndmp_readit(void*		connectionHandle,
			void*		buf,
			int			len)
{
	Connection	*connection = (Connection *)connectionHandle;

	len = read(connection->sock, buf, len);
	if (len <= 0)
	{
		/* Connection has been closed. */
		connection->eof = TRUE;
		return(-1);
	}

	return(len);
}

/*
 * ndmp_writeit
 *
 * Low level write routine called by the xdrrec library.
 *
 * Parameters:
 *   connection (input) - connection pointer.
 *   buf        (input) - location to store received data.
 *   len        (input) - max number of bytes to read.
 *
 * Returns:
 *   >0 - number of bytes sent.
 *   -1 - error.
 */
static int
ndmp_writeit(void*		connectionHandle,
			 void*	    buf,
			 int		len)
{
	Connection		*connection = (Connection *)connectionHandle;
	register int	n;
	register int	cnt;

	for(cnt = len; cnt > 0; cnt -= n, buf = (void*)((long)buf + n))
	{
		if ((n = write(connection->sock, buf, cnt)) < 0)
		{
			ERRLOG("Write failed! %d", n);
			connection->eof = TRUE;
			return (-1);
		}
	}

	return(len);
}
#else
/************** private functions ****************************************/

/*
 * ndmp_readit
 *
 * Low level read routine called by the xdrrec library.
 *
 * Parameters:
 *   connection (input) - connection pointer.
 *   buf        (input) - location to store received data.
 *   len        (input) - max number of bytes to read.
 *
 * Returns:
 *   >0 - number of bytes received.
 *   -1 - error.
 */
static int
ndmp_readit(char*		connectionHandle,
			caddr_t		buf,
			int			len)
{
	Connection	*connection = (Connection *)connectionHandle;

	len = read(connection->sock, buf, len);
	if (len <= 0)
	{
		/* Connection has been closed. */
		connection->eof = TRUE;
		return(-1);
	}

	return(len);
}

/*
 * ndmp_writeit
 *
 * Low level write routine called by the xdrrec library.
 *
 * Parameters:
 *   connection (input) - connection pointer.
 *   buf        (input) - location to store received data.
 *   len        (input) - max number of bytes to read.
 *
 * Returns:
 *   >0 - number of bytes sent.
 *   -1 - error.
 */
static int
ndmp_writeit(char*		connectionHandle,
			 caddr_t	buf,
			 int		len)
{
	Connection		*connection = (Connection *)connectionHandle;
	register int	n;
	register int	cnt;

	DBGLOG("ndmp_writeit: len:%d.", len);

	for(cnt = len; cnt > 0; cnt -= n, buf += n)
	{
		if ((n = write(connection->sock, buf, cnt)) < 0)
		{
			connection->eof = TRUE;
			return (-1);
		}
	}

	return(len);
}
#endif


int ndmpRecvMsgRequestEncode(Connection *connection)
{
	bool_t	(*xdr_func)(XDR*, ...) = 0;
	if (connection->msginfo.handler->auth_required &&
		!connection->authorized) {
		ERRLOG("Processing request 0x%x: connection not authorized.", connection->msginfo.hdr.message);
		return(NDMP_NOT_AUTHORIZED_ERR);
	}
	
	if (connection->msginfo.handler->sizeof_request > 0) {
		xdr_func = connection->msginfo.handler->xdr_request;
		if (xdr_func == 0) {
			ERRLOG("Processing request 0x%x: no xdr function in handler table.", connection->msginfo.hdr.message);
			return(NDMP_NOT_SUPPORTED_ERR);
		}
		
		connection->msginfo.body =
			(void *)malloc(connection->msginfo.handler->sizeof_request);
		if (connection->msginfo.body == 0) {
			ERRLOG("Processing request 0x%x: system call (malloc): %s.",
					connection->msginfo.hdr.message, strerror(errno));
			return(NDMP_NO_MEM_ERR);
		}
		(void)memset_s(connection->msginfo.body, connection->msginfo.handler->sizeof_request, 0,
				connection->msginfo.handler->sizeof_request);
	}

	return NDMP_NO_ERR;
}

int ndmpRecvMsgReplyEncode(Connection *connection)
{
	bool_t	(*xdr_func)(XDR*, ...) = 0;
	if (connection->msginfo.handler->sizeof_reply > 0) {
		xdr_func = connection->msginfo.handler->xdr_reply;
		if (xdr_func == 0) {
			ERRLOG("Processing reply 0x%x: no xdr function in handler table.", connection->msginfo.hdr.message);
			return(NDMP_NOT_SUPPORTED_ERR);
		}

		connection->msginfo.body = (void *)malloc(connection->msginfo.handler->sizeof_reply);
		if (connection->msginfo.body == 0) {
			ERRLOG("Processing reply 0x%x: system call (malloc): %s.",
					connection->msginfo.hdr.message, strerror(errno));
			return(NDMP_NO_MEM_ERR);
		}
		(void)memset_s(connection->msginfo.body, connection->msginfo.handler->sizeof_reply, 0,
				connection->msginfo.handler->sizeof_reply);
	}

	return NDMP_NO_ERR;
}
/*
 * ndmp_recv_msg
 *
 * Read the next message.
 *
 * Parameters:
 *   connection (input)  - connection pointer.
 *   msg        (output) - received message.
 *
 * Returns:
 *   0 - Message successfully received.
 *   error number - Message related error.
 *  -1 - Error decoding the message header.
 */
static int
ndmp_recv_msg(Connection*	connection)
{
	bool_t	(*xdr_func)(XDR*, ...) = 0;

	/* Decode the header. */
	connection->xdrs.x_op = XDR_DECODE;
	xdrrec_skiprecord(&connection->xdrs);
	if (!xdr_ndmp_header(&connection->xdrs, &connection->msginfo.hdr))
	{
		ERRLOG("err decode ndmp header");
		return(-1);
	}

	/* Lookup info necessary for processing this message. */
	if ((connection->msginfo.handler =
		  ndmp_get_handler(connection,
						   connection->msginfo.hdr.message)) == 0)
	{
		ERRLOG("Message 0x%x not supported.", connection->msginfo.hdr.message);
		return(NDMP_NOT_SUPPORTED_ERR);
	}
  
	connection->msginfo.body		= 0;

	if (connection->msginfo.hdr.error != NDMP_NO_ERR) {
		ERRLOG("check1");
		return(0);
	}
	
	/* Determine body type */
	if (connection->msginfo.hdr.message_type == NDMP_MESSAGE_REQUEST)
	{
		if (connection->msginfo.handler->auth_required &&
			!connection->authorized)
		{
			ERRLOG("Processing request 0x%x: connection not authorized.", connection->msginfo.hdr.message);
			return(NDMP_NOT_AUTHORIZED_ERR);
		}
		
		if (connection->msginfo.handler->sizeof_request > 0)
		{
			xdr_func		= connection->msginfo.handler->xdr_request;
			if (xdr_func == 0)
			{
				ERRLOG("Processing request 0x%x: no xdr function in handler table.", connection->msginfo.hdr.message);
				return(NDMP_NOT_SUPPORTED_ERR);
			}
			
			connection->msginfo.body =
				(void *)malloc(connection->msginfo.handler->sizeof_request);
			if (connection->msginfo.body == 0)
			{
				ERRLOG("Processing request 0x%x: system call (malloc): %s.",
						connection->msginfo.hdr.message, strerror(errno));
				return(NDMP_NO_MEM_ERR);
			}
			(void)memset_s(connection->msginfo.body, connection->msginfo.handler->sizeof_request, 0,
				   connection->msginfo.handler->sizeof_request);
		}
	}
	else
	{
		if (connection->msginfo.handler->sizeof_reply > 0)
		{
			xdr_func = connection->msginfo.handler->xdr_reply;
			if (xdr_func == 0)
			{
				ERRLOG("Processing reply 0x%x: no xdr function in handler table.", connection->msginfo.hdr.message);
				return(NDMP_NOT_SUPPORTED_ERR);
			}

			connection->msginfo.body =
				(void *)malloc(connection->msginfo.handler->sizeof_reply);
			if (connection->msginfo.body == 0)
			{
				ERRLOG("Processing reply 0x%x: system call (malloc): %s.",
						connection->msginfo.hdr.message, strerror(errno));
				return(NDMP_NO_MEM_ERR);
			}
			(void)memset_s(connection->msginfo.body, connection->msginfo.handler->sizeof_reply, 0,
				   connection->msginfo.handler->sizeof_reply);
		}
	}

	/* Decode message arguments if needed */
	if (xdr_func)
	{
		if (!(*xdr_func)(&connection->xdrs,
						 connection->msginfo.body))
		{
			ERRLOG("Processing message 0x%x: error decoding arguments.", connection->msginfo.hdr.message);
			MODULE_FREE(connection->msginfo.body);
			connection->msginfo.body = 0;
			return(NDMP_XDR_DECODE_ERR);
		}
	}

	return(0);
}

int ndmpPostProcessMessages(Connection*	connection, bool_t reply_read, bool_t reply_error,
	MsgInfo reply_msginfo)
{

	if (connection->eof == TRUE) {    
		MODULE_FREE(reply_msginfo.body);
        return(-1);
    }
	
	if (reply_error) {
		MODULE_FREE(reply_msginfo.body);
        return(2);   
    }

	if (reply_read) {
		connection->msginfo = reply_msginfo;
		return(1);
	}
	
	return(0);
}

void ndmpSendErrReply(Connection *connection, int err)
{
	ndmpSendReply((NdmpConnection)connection, (ndmp_error)err, 0);
	ndmpFreeMessage((NdmpConnection)connection);
}

void ndmpProcessReplyMessage(Connection *connection, bool_t reply_expected,
	bool_t *reply_read, bool_t *reply_error)
{
	if (reply_expected == 0 || *reply_read == TRUE) {
		ERRLOG("unexpected reply message: 0x%x.", connection->msginfo.hdr.message);
	}      
	ndmpFreeMessage((NdmpConnection)connection);
	if (*reply_read == FALSE)
	{
		*reply_read = TRUE;
		*reply_error = TRUE;
	}
}

/*
 * ndmp_process_messages
 *
 * Reads the next message into the stream buffer.
 * Processes messages until the stream buffer is empty.
 *
 * This function processes all data in the stream buffer before returning.
 * This allows functions like poll() to be used to determine when new 
 * messages have arrived. If only some of the messages in the stream buffer
 * were processed and then poll was called, poll() could block waiting for
 * a message that had already been received and read into the stream buffer.
 *
 * This function processes both request and reply messages.
 * Request messages are dispatched using the appropriate function from the
 * message handling table.
 * Only one reply messages may be pending receipt at a time.
 * A reply message, if received, is placed in connection->msginfo
 * before returning to the caller.
 * Errors are reported if a reply is received but not expected or if
 * more than one reply message is received
 *
 * Parameters:
 *   connection     (input)  - connection pointer.
 *   reply_expected (output) - TRUE  - a reply message is expected.
 *                             FALSE - no reply message is expected and
 *                             an error will be reported if a reply
 *                             is received.
 *
 * Returns:
 *   2 - 1 or messages successfully processed, error processing reply message.
 *   1 - 1 or messages successfully processed, reply seen.
 *   0 - 1 or more messages successfully processed, no reply seen.
 *  -1 - error; connection no longer established.
 *
 * Notes:
 *   If the peer is generating a large number of requests, a caller
 *   looking for a reply will be blocked while the requests are handled.
 *   This is because this function does not return until the stream
 *   buffer is empty.
 *   Code needs to be added to allow a return if the stream buffer
 *   is not empty but there is data available on the socket. This will
 *   prevent poll() from blocking and prevent a caller looking for a reply
 *   from getting blocked by a bunch of requests.
 */
static int
ndmp_process_messages(Connection*	connection,
					  bool_t		reply_expected)
{
	MsgInfo		reply_msginfo;
	bool_t		reply_read = FALSE;
	bool_t		reply_error = FALSE;
	int			err;

	(void)memset_s(&reply_msginfo, sizeof(MsgInfo), 0, sizeof(MsgInfo));
	do {
		(void)memset_s((void *)&connection->msginfo, sizeof(MsgInfo), 0, sizeof(MsgInfo));
	
		if ((err = ndmp_recv_msg(connection)) != NDMP_NO_ERR) {
			if (connection->eof)
			{
			    ndmpFreeMessage((NdmpConnection)connection);
				ERRLOG("ndmp_process_messages: detected eof.");
				return(-1);
			}
			
			if (err < 1) {
				ERRLOG("Ndmp_process_messages: error decoding header.");

				/*
				 * Error occurred decoding the header.
				 * Don't send a reply since we don't know the message
				 * or if the message was even a request message.
				 *
				 * To be safe, assume that the message was a reply
				 * if a reply was expected. Need to do this to prevent
				 * hanging ndmpSendRequest() waiting for a reply. 
				 * Don't set reply_read so that the reply will be
				 * processed if it received later.
				 */
				if (reply_read == FALSE)
					reply_error = TRUE;
				continue;
			}
			
			if (connection->msginfo.hdr.message_type != NDMP_MESSAGE_REQUEST) {
				if (reply_expected == 0 || reply_read == TRUE) {
					ERRLOG("unexpected reply message: 0x%x.", connection->msginfo.hdr.message);
                }
                      
				ndmpFreeMessage((NdmpConnection)connection);

				if (reply_read == FALSE) {
					reply_read = TRUE;
					reply_error = TRUE;
				}
				
				continue;
			}

			ndmpSendReply((NdmpConnection)connection, (ndmp_error)err, 0);
			ndmpFreeMessage((NdmpConnection)connection);
			continue;
		}
			
		if (connection->msginfo.hdr.message_type != NDMP_MESSAGE_REQUEST) {
			if (reply_expected == 0 || reply_read == TRUE)
			{
				ERRLOG("unexpected reply message: 0x%x.", connection->msginfo.hdr.message);
				ndmpFreeMessage((NdmpConnection)connection);
				continue;
			}
			
			reply_read = TRUE;
			reply_msginfo = connection->msginfo;
			continue;
		}

		/*
		 * The following needed to catch an improperly constructed
		 * handler table or to deal with an NDMP client that is not
		 * conforming to the negotiated protocol version.
		 */
		if (connection->msginfo.handler->func == 0) {
			ERRLOG("No handler for message 0x%x.", connection->msginfo.hdr.message);

			ndmpSendReply((NdmpConnection)connection, NDMP_NOT_SUPPORTED_ERR, 0);
			ndmpFreeMessage((NdmpConnection)connection);
			continue;
		}

		/*
		 * Call the handler function.
		 * The handler will send any necessary reply.
		 */
		(*connection->msginfo.handler->func)(connection,
											 connection->msginfo.body);

		ndmpFreeMessage((NdmpConnection)connection);

	} while (xdrrec_eof(&connection->xdrs) == FALSE &&
			 connection->eof == FALSE);

	if (connection->eof == TRUE) {    
		MODULE_FREE(reply_msginfo.body);
        return(-1);
    }
	
	if (reply_error) {
		MODULE_FREE(reply_msginfo.body);
        return(2);   
    }

	if (reply_read) {
		connection->msginfo = reply_msginfo;
		return(1);
	}
	
	return(0);
}

/*
 * ndmp_get_handler_info
 *
 * Return the handler info for the specified NDMP procedure.
 *
 * Parameters:
 *   procedure (input) - procedure number.
 *
 * Returns:
 *   0 - procedure not found.
 *   pointer to handler info.
 */
static NdmpMsgHandler*
ndmp_get_handler(Connection*	connection,
				 uint32_t		procedure)
{
	NdmpMsgHandler		*handler;

	for (handler = connection->msgHandlerTbl;
		 handler->message != 0;
		 handler++)
	{
		if (handler->message == procedure &&
			handler->version == connection->version)
			return(handler);
    }

	return(0);
}

int check_connect_socket(NdmpConnection	connectionHandle)
{
    Connection	*connection = (Connection *)connectionHandle;
    
    if (NULL == connection)
    {
        return 0;
    }

    if (connection->sock == -1)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void set_connect_socket_valid(NdmpConnection connectionHandle)
{
    Connection	*connection = (Connection *)connectionHandle;
    if (NULL != connection)
    {
       connection->sock = -1;
    }
}

void
removeLifIpConn()
{
#ifdef _ARM1620_
    uint32_t i = 0;
    int rc = 0;

    for (i = 0; i < 2; i++)
    {
        if (g_lif_ip_connect_array[i].isConn)
        {
			DBGLOG("Cancle Lif Ip Connect, ip1(%s), ip2(%s).",
					g_lif_ip_connect_array[i].ipAddr1, g_lif_ip_connect_array[i].ipAddr2);
                        
            rc = cmr_kagent_net_lif_ip_conn(CANCEL_LIF_IP_CONN, g_lif_ip_connect_array[i].ipAddr1, g_lif_ip_connect_array[i].ipAddr2);

            if (0 != rc)
            {
				ERRLOG("Cancle Lif Ip Connect error, ip1(%s), ip2(%s).",
						g_lif_ip_connect_array[i].ipAddr1, g_lif_ip_connect_array[i].ipAddr2);
            }

            g_lif_ip_connect_array[i].isConn = false;
        }
    }

    return;
#else
    return;
#endif
}

void
intSaveLifIp()
{
#ifdef _ARM1620_
    uint32_t i = 0;

    for (i = 0; i < 2; i++)
    {
        g_lif_ip_connect_array[i].isConn = false;
        (void)memset_s(g_lif_ip_connect_array[i].ipAddr1, (MAX_HOST_NAME_LEN + 1), 0, (MAX_HOST_NAME_LEN + 1));
        (void)memset_s(g_lif_ip_connect_array[i].ipAddr2, (MAX_HOST_NAME_LEN + 1), 0, (MAX_HOST_NAME_LEN + 1));
    }
    return;
#else
    return;
#endif
}
