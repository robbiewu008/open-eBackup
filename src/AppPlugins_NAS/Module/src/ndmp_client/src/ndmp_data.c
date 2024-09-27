#include "ndmpd.h"
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <errno.h>
#include <fcntl.h>
#include "comm.h"
#include <fstream>
#include "securec.h"
#include "log/Log.h"
#include <time.h>

NdmpDataConnection_t g_dataConnection;
extern pthread_t g_getDataPt;
extern bool g_isSendAbort;
extern void setSendAbortValue(bool value);

void ndmpClientDataSelectPort(unsigned short *port, int *tmpSd, struct sockaddr_in *sin, char *addrIp)
{
    int i;
    unsigned short tmpPort = 0;
    for (i = 0;  i < NDMPD_DATA_PORT_MAX_NUM; i++) {
        tmpPort = NDMPD_DEFAULT_MIN_PORT + i;    
        sin->sin_port = htons(tmpPort);
        int ret = inet_aton(addrIp, &sin->sin_addr);
        if (0 == ret) {
            ERRLOG("invalid listen addr:%s, port: %d.", addrIp, tmpPort);
            continue;
        }
        *tmpSd = socket(AF_INET, (int)SOCK_STREAM, 0);
        if (*tmpSd < 0) {
            ERRLOG("Socet err, ip:%s, error:%d.", addrIp, errno);
            continue;
        }
        int on = 1;
        (void)setsockopt(*tmpSd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
   
        if (bind(*tmpSd, (struct sockaddr *)sin, sizeof(struct sockaddr_in)) < 0) {
            ERRLOG("Bind err, ip:%s, error:%d.", addrIp, errno);
            (void)close(*tmpSd);
            continue;
        }
        int flags = fcntl(*tmpSd, F_GETFL, 0);

        if (fcntl(*tmpSd, F_SETFL, flags | O_NONBLOCK) < 0){
            ERRLOG("set noblock err, ip:%s, error:%d.", addrIp, errno);
            continue;
        }

        if (listen(*tmpSd, 5) < 0) { // 5 max thread
            ERRLOG("Listen err, ip:%s, error:%d.", addrIp, errno);
            (void)close(*tmpSd);
            continue;
        }

        *port = tmpPort;
        break;
    }   
}

int NdmpClientCreateDataIpv4Socket(char *addrIp, unsigned short *port, int *fd)
{
    int len = 0;
    int ret = -1;
    int tmpSd = -1;
    struct sockaddr_in sin;
    unsigned short tmpPort = 0;

    if (NULL == addrIp || NULL == port || NULL == fd) {
        ERRLOG("create data listen err, param is NULL.");
        return ret;
    }

    ret = memset_s(&sin, sizeof(struct sockaddr_in), 0, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    len = sizeof(sin);

    (void)ndmpClientDataSelectPort(&tmpPort, &tmpSd, &sin, addrIp);
    if (tmpPort == 0) {
        ERRLOG("no avaliable port 10200-10263, all used.");
        return -1;
    }
    *fd = tmpSd;
    *port = tmpPort;
    INFOLOG("Create data listen, port( %u), addr (%s), sock (%d).", *port, addrIp, tmpSd);
    return 0;
}

int NdmpClientCreateDataIpv6Socket(char *addrIp, int *fd, unsigned short *port, char *addr)
{
    struct sockaddr_in6 sin;
    int ret, tmpSd = -1;
    int i, len = 0;
    unsigned short tmpPort = 0;

    if (NULL == addrIp || NULL == port || NULL == fd) {
        ERRLOG("create data listen err, param is NULL.");
        return ret;
    }

    ret = memset_s(&sin, sizeof(struct sockaddr_in6), 0, sizeof(struct sockaddr_in6));
    sin.sin6_family = AF_INET6;
    for (i = 0;  i < NDMPD_DATA_PORT_MAX_NUM; i++) {
        tmpPort = NDMPD_DEFAULT_MIN_PORT + i;
        sin.sin6_port = htons(tmpPort);
        len = sizeof(sin);
        ret = inet_pton(AF_INET6, addrIp, (void *)&sin.sin6_addr);
        if (ret != 1) {
            ERRLOG("invalid listen addr:%s, port: %d.", addrIp, tmpPort);
            continue;;
        }

        tmpSd = socket(AF_INET6, (int)SOCK_STREAM, 0);
        if (tmpSd < 0) {
            ERRLOG("Socet err, ip:%s, error:%d.", addrIp, errno);
            continue;;
        }

        int on = 1;
        (void)setsockopt(tmpSd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));

        if (bind(tmpSd, (struct sockaddr *)&sin, len) < 0) {
            ERRLOG("Bind err, ip:%s, error:%d.", addrIp, errno);
            (void)close(tmpSd);
            continue;
        }

        if (listen(tmpSd, 5) < 0) { // 5 max thread
            ERRLOG("Listen err, ip:%s, error:%d.", addrIp, errno);
            (void)close(tmpSd);
            continue;;
        }
        break;      
    }

    if (tmpPort == 0) {
        ERRLOG("no avaliable port 10200-10263, all used.");
        return -1;
    }
    *fd = tmpSd;
    *port = tmpPort;
    (void)memcpy_s(addr, sizeof(struct in6_addr), (void *)&sin.sin6_addr, sizeof(struct in6_addr));
    INFOLOG("Create data listen, port( %u), addr (%s), sock (%d).", *port, addrIp, tmpSd);
    return 0;
}

int NdmpClientCreateDataIpv4Listener(char *srcIp, unsigned short *port, int *fd, unsigned long *addr)
{
    int ret;

    if (NULL == srcIp) {
        ERRLOG("data listen err, param is NULL");
        return -1;
    }

    ret = NdmpClientCreateDataIpv4Socket(srcIp, port, fd);
    if (ret != 0) {
        ERRLOG("ndmpClient create data sock failed, ret:%d.", ret);
        return ret;
    }

    *addr = inet_addr(srcIp);
    INFOLOG("listen data conn success, ip:%s, port:%d.", srcIp, *port);
    return 0;
}

int NdmpClientCreateDataIpv6Listener(char *srcIp, unsigned short *port, int *fd, char *addr)
{
    int ret;
    if (NULL == srcIp) {
        ERRLOG("data listen err, param is NULL");
        return -1;
    }

    ret = NdmpClientCreateDataIpv6Socket(srcIp, fd, port, addr);
    if (ret != 0) {
        ERRLOG("ndmpClient create data sock failed, ret:%d.", ret);
        return ret;
    }

    INFOLOG("listen data conn success, ip:%s, port:%d.", srcIp, *port);
    return ret;
}

void NdmpClientDataConnectSock(int sock, NdmpDataConnection_t *dataConn)
{
    struct sockaddr_in from;
    int fromLen = sizeof(from);
    int flag = 1;
    int keepIdle = 60;
    int keepInterval = 30;
    int keepCount = 6;
    unsigned int bufferSize = DEFAULT_DATA_SOCKET_BUFFER_SZIE;

    dataConn->conn_sock = accept(sock, (struct sockaddr *)&from, (socklen_t *)&fromLen);
    if (dataConn->conn_sock < 0) {
        ERRLOG("Accept fd failed, sock:%d, port:%d, error: %d.", sock, dataConn->port, errno);
        return;                                      
    }
    INFOLOG("Accept fd SUCCESS, sock:%d, port:%d, error: %d.", dataConn->conn_sock, dataConn->port, errno);

    (void)setsockopt(dataConn->conn_sock, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(flag));
    (void)setsockopt(dataConn->conn_sock, SOL_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(keepIdle));
    (void)setsockopt(dataConn->conn_sock, SOL_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(keepInterval));
    (void)setsockopt(dataConn->conn_sock, SOL_TCP, TCP_KEEPCNT, &keepCount, sizeof(keepCount));

    struct timeval timeOut;
    timeOut.tv_sec = DEFAULT_DATA_SOCKET_TIMEOUT_VALUE;
    timeOut.tv_usec = 0;
    (void)setsockopt(dataConn->conn_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeOut, sizeof(timeOut));
    (void)setsockopt(dataConn->conn_sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeOut, sizeof(timeOut));
    (void)setsockopt(dataConn->conn_sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
    setsockopt(dataConn->conn_sock, SOL_SOCKET, SO_SNDBUF, &bufferSize, sizeof(bufferSize));
    setsockopt(dataConn->conn_sock, SOL_SOCKET, SO_SNDBUF, &bufferSize, sizeof(bufferSize));

    INFOLOG("data connected, port:%d.", dataConn->port);
}

int NdmpCLientCreateDataConnect(unsigned short port, int fd)
{
    g_dataConnection.listen_sock = fd;
    g_dataConnection.port = port;

    NdmpClientDataConnectSock(fd, &g_dataConnection);

    return 0;
}

void NdmpGetBackupFileName(char *fileName)
{
    strcpy_s(fileName, NDMPD_DATA_MAX_FILE_PATH, g_dataConnection.backFilePath);
    strcat_s(fileName, NDMPD_DATA_MAX_FILE_PATH, "/");
    strcat_s(fileName, NDMPD_DATA_MAX_FILE_PATH, NDMPD_DEFAULT_FULL_BACKUP_FILE_NAME);
}

void clean()
{
    setSendAbortValue(true);
    close(g_dataConnection.conn_sock);
    close_thread(&g_getDataPt);
}

bool doRealReceive(FILE *f)
{
    int rc = 0;
    int recvLen = 0;
    char dataBuf[NDMPD_DATA_DEFAULT_BUFFER_SIZE] = { 0 };
    char aggBuf[NDMPD_DATA_AGGR_BUFFER_SIZE] = {0};
    int offset_index = 0;
    while (1) {
        if (g_isSendAbort) {
            return false;
        }
        bzero(dataBuf, NDMPD_DATA_DEFAULT_BUFFER_SIZE);            
        recvLen = read(g_dataConnection.conn_sock, dataBuf, NDMPD_DATA_DEFAULT_BUFFER_SIZE - 1);
        if (recvLen <= 0) {
            break;
        }
        if (offset_index + recvLen > NDMPD_DATA_AGGR_BUFFER_SIZE) {
            rc = fwrite(aggBuf, offset_index, 1, f);
            if (rc == 0) {
                ERRLOG("write data err, cannot write!");
                return false;
            }
            offset_index = 0;
            bzero(aggBuf, NDMPD_DATA_AGGR_BUFFER_SIZE);
        }
        memcpy_s(aggBuf + offset_index, recvLen, dataBuf, recvLen);
        offset_index += recvLen;
    }
    if (offset_index > 0) {
        rc = fwrite(aggBuf, offset_index, 1, f);
        if (rc == 0) {
            ERRLOG("write data err, cannot write!");
            return false;
        }
        offset_index = 0;
        bzero(aggBuf, NDMPD_DATA_AGGR_BUFFER_SIZE);
    }
    return true;
}

void* NdmpStartDataReceive(void* args)
{
    int rc;
    FILE *f = NULL;
    
    char fileName[NDMPD_DATA_MAX_FILE_PATH] = { 0 };
    // if file exist , clear it first
    (void)NdmpGetBackupFileName(fileName);
    f = fopen(fileName, "w");

    if (NULL == f) {
        ERRLOG("open file failed.");
        goto ERR;
    }

    if (NDMPD_DEFAULT_MIN_PORT > g_dataConnection.port || g_dataConnection.port > NDMPD_DEFALUT_MAX_PORT) {
        ERRLOG("write data err, data conn null.");
        fclose(f);
        goto ERR;
    }

    if (!doRealReceive(f)) {
        fclose(f);
        goto ERR;
    }
    fclose(f);
    close(g_dataConnection.conn_sock);
    close_thread(&g_getDataPt);
    return 0;
ERR:
    clean();
    return 0;
}

void* NdmpStartRestoreData(void* args)
{
    int rc;
    int readNum = 0;
    char dataBuf[NDMPD_DATA_DEFAULT_BUFFER_SIZE] = { 0 };
    unsigned long len = DEFAULT_DATA_SOCKET_BUFFER_SZIE;
    char fileName[NDMPD_DATA_MAX_FILE_PATH] = { 0 };
    (void)NdmpGetBackupFileName(fileName);

    FILE *f = fopen(fileName, "r+");
    if (NULL == f) {
        ERRLOG("open file failed.");
        goto ERR;
    }

    if (NDMPD_DEFAULT_MIN_PORT > g_dataConnection.port ||
        g_dataConnection.port > NDMPD_DEFALUT_MAX_PORT) {
        ERRLOG("write data err, data conn null.");
        fclose(f);
        goto ERR;
    }
    
    while (!feof(f)) {
        if (g_isSendAbort) {
            break;
        }
        bzero(dataBuf, NDMPD_DATA_DEFAULT_BUFFER_SIZE);
        readNum = fread(dataBuf, sizeof(char), sizeof(dataBuf) - 1, f);
        rc = write(g_dataConnection.conn_sock, dataBuf, readNum * sizeof(char));
        if (rc < 0) {
            if (errno == EAGAIN) {
                ERRLOG("write timeout errno(%d)", errno);
                goto ERR;
            }
        }
    }

    fclose(f);
    close(g_dataConnection.conn_sock);
    close_thread(&g_getDataPt);
    return 0;
ERR:
    setSendAbortValue(true);
    close(g_dataConnection.conn_sock);
    close_thread(&g_getDataPt);

    return 0;
}