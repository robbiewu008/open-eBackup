/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * @file libndmp.c
 * @date 5/16/2023
 * @author
 * @brief
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <openssl/md5.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "securec.h"
#include "ndmp.h"
#include "interface.h"
#include "ndmp_common.h"
#include "log/Log.h"
#include "common/Path.h"
#include "queue.h"
#include "comm.h"
#include "ndmpd.h"

#define MAX_HOST_NAME_LEN 128
#define MAX_DIR_LEN 4096
#define MAX_AUTH_USER_LEN 32
#define MAX_AUTH_USER_PASSWORD_LEN 512
#define MAX_EXCLUDE_LEN 1024
#define MAX_TENANT_NAME_LEN 512

#define LOG_VERBOSE_LEVEL	1
#define DEBUG_VERBOSE_LEVEL	3

#define DEF_SRC_HOST             ""
#define DEF_DEST_HOST            ""

#define DEF_SRC_AUTH_TYPE        NDMP_AUTH_MD5
#define DEF_SRC_AUTH_USER        ""
#define DEF_SRC_AUTH_PASSWORD    ""

#define DEF_DEST_AUTH_TYPE       NDMP_AUTH_MD5
#define DEF_DEST_AUTH_USER       ""
#define DEF_DEST_AUTH_PASSWORD   ""

#define DEF_DHOST                ""

#define DEF_SRC_DIR              "/"
#define DEF_DEST_DIR             "/"
#define DEF_LEVEL                "0"
#define DEF_EXTRACT				"y"

#define NDMPCOPY_TASK_EXCUTE_SUCCESS 	0x200F406E0028 /*NDMPCOPY执行成功事件*/
#define NDMPCOPY_TASK_EXCUTE_FAIL    	0x100F406E000E /*NDMPCOPY执行失败事件*/

#define MD5_CHALLENGE_SIZE 64
#define MD5_PASS_LIMIT 32

typedef enum VType
{
    VNON  = 0,
    VREG  = 1,
    VDIR  = 2,
    VBLK  = 3,
    VCHR  = 4,
    VLNK  = 5,
    VFIFO = 6,
    VDOOR = 7,
    VPROC = 8,
    VSOCK = 9,
    VPORT = 10,
    VROOT = 11,
    VBAD = 12
} VType;

typedef enum NdmpStartBackupStatus
{
    NDMP_START_BACKUP_SUCCESS = 0,
    NDMP_START_BACKUP_INNER_ERROR = -1,
    NDMP_AUTH_ERROR_SRC = -2,
    NDMP_AUTH_ERROR_DST = -3,
    NDMP_DATA_CONNECT_ERROR = -4,
    NDMP_DST_RECOVER_ERROR = -5,
    NDMP_SRC_BACKUP_ERROR = -6,
    NDMP_START_RESTORE_INNER_ERROR = -7
} NdmpStartBackupStatus;

typedef enum NdmpStorageType
{
    PRODUCT_DORADO = 0,
    PRODUCT_NETAPP = 1,
    PRODUCT_ISILON = 2,
    PRODUCT_UNITY = 3
} NdmpStorageType;

typedef enum DumpType
{
    DUMP_TYPE_DORADO = 0,
    DUMP_TYPE_NETAPP = 1
} DumpType;

typedef struct MsgInfo
{
	ndmp_header			hdr;
	NdmpMsgHandler*		handler;
	void*				body;
} MsgInfo;
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


struct AuthInfo {
    ndmp_auth_type type;
    char user[MAX_AUTH_USER_LEN + 1];
    char *password;
};

NdmpConnection		g_srcConnection;
extern NdmpMsgHandler		ndmp_msg_handler_tbl[];
static MsgQueue				g_backendQueue;
struct AuthInfo             g_srcAuth;
struct AuthInfo             g_dstAuth;
static int                  g_restoreState = 0;
static ndmp_pval			g_environment[10];
pthread_mutex_t             g_connectionMutex = PTHREAD_MUTEX_INITIALIZER;
static uint64_t             g_srcProcessBytes = 0;
static uint64_t             g_srcRemainingSize = 0;
char*                       g_srcIp;
char*                       g_backupFilePath;
char*                       g_dataPath;
static int                  g_ndmpStatus;
pthread_t                   g_getStatPt;
pthread_t                   g_runBackupPt;
pthread_t                   g_getDataPt;
bool                        g_isSendAbort = false;
uint64_t             g_backupFilesCnt = 0;
static ndmp_class_version g_setExt[3];

void setSendAbortValue(bool value)
{
    (void) pthread_mutex_lock(&g_connectionMutex);
    g_isSendAbort = value;
    (void) pthread_mutex_unlock(&g_connectionMutex);
}

void init_ndmp_connections()
{
    g_srcConnection = NULL;
}

void NdmpinitFilepath(char *dstPath, char *srcPath, char *name)
{
    int rc = strcpy_s(dstPath, NDMPD_DATA_MAX_FILE_PATH, srcPath);
    CHECK_MEMCPY(rc, 0);
    rc = strcat_s(dstPath, NDMPD_DATA_MAX_FILE_PATH, "/");
    CHECK_MEMCPY(rc, 0);
    rc = strcat_s(dstPath, NDMPD_DATA_MAX_FILE_PATH, name);
    CHECK_MEMCPY(rc, 0);
}

void set_ndmp_status(NdmpStatus status)
{
    INFOLOG("ndmp set status : %d", status);
    (void) pthread_mutex_lock(&g_dataMutex);
    g_ndmpStatus = status;
    (void) pthread_mutex_unlock(&g_dataMutex);
}

/* use to init ndmp client log */
void InitLog(const char* fullLogPath, int logLevel, const char* rootPath)
{
    if (rootPath != NULL) {
        Module::CPath::GetInstance().SetRootPath(rootPath);
    }

    unsigned int iLogLevel = static_cast<unsigned int>(logLevel);
    unsigned int iLogCount = 100;
    unsigned int iLogMaxSize = 30;
    const char* ndmpClientLogName = "ndmpclient.log";
    Module::CLogger::GetInstance().Init(ndmpClientLogName,
        fullLogPath,
        iLogLevel,
        iLogCount,
        iLogMaxSize);
}

void log_auth_params(NdmpClientInterface *interface)
{
    size_t srcIpLen = strlen(interface->srcIp);
    g_srcIp = (char*)malloc(srcIpLen + 1);
    strncpy_s(g_srcIp, srcIpLen + 1, interface->srcIp, srcIpLen + 1);
    INFOLOG("Log auth params: (srcip : %s), (dstip: %s), (srcuser: %s), (dstuser: %s), "
        "(srcpath: %s), (dstpath: %s), (backfilePath:%s)", interface->srcIp,
        interface->dstIp, interface->srcAuth, interface->dstAuth,
        interface->srcPath, interface->dstPath, interface->backFilePath);
}

int init_auth(NdmpClientInterface *interface)
{
    g_srcAuth.type = NDMP_AUTH_MD5;
    memset_s(g_srcAuth.user, (MAX_AUTH_USER_LEN + 1), 0, (MAX_AUTH_USER_LEN + 1));
    g_srcAuth.password = NULL;

    g_dstAuth.type = NDMP_AUTH_MD5;
    memset_s(g_dstAuth.user, (MAX_AUTH_USER_LEN + 1), 0, (MAX_AUTH_USER_LEN + 1));
    g_dstAuth.password = NULL;

    strncpy_s(g_srcAuth.user, MAX_AUTH_USER_LEN, interface->srcAuth, strlen(interface->srcAuth));
    g_srcAuth.password = (char*)malloc((strlen(interface->srcPwd) + 1));
    if (NULL == g_srcAuth.password)
    {
        ERRLOG("malloc error");
        return -1;
    }
    (void)memset_s(g_srcAuth.password, (strlen(interface->srcPwd) + 1), 0, (strlen(interface->srcPwd) + 1));
    (void)strncpy_s(g_srcAuth.password, (strlen(interface->srcPwd) + 1), interface->srcPwd, strlen(interface->srcPwd));

    strncpy_s(g_dstAuth.user, MAX_AUTH_USER_LEN, interface->dstAuth, strlen(interface->dstAuth));
    g_dstAuth.password = (char*)malloc((strlen(interface->dstPwd) + 1));
    if (NULL == g_dstAuth.password)
    {
        ERRLOG("malloc error");
        return -1;
    }
    (void)memset_s(g_dstAuth.password, (strlen(interface->dstPwd) + 1), 0, (strlen(interface->dstPwd) + 1));
    (void)strncpy_s(g_dstAuth.password, (strlen(interface->dstPwd) + 1), interface->dstPwd, strlen(interface->dstPwd));

    return 0;
}

int ndmp_open_connection(NdmpConnection *ndmp_connection, char *dest_host, int port)
{
    int	rc = 0;
    *ndmp_connection = ndmpCreateConnection(ndmp_msg_handler_tbl);
    if (0 == *ndmp_connection)
    {
        ERRLOG("Create connection error.");
        return NDMPCOPY_CONNECT_ERR;
    }
    ndmpSetClientData(*ndmp_connection, &g_backendQueue);
    INFOLOG("Connecting to %s.", dest_host);
    rc = ndmpConnect(*ndmp_connection, dest_host, port);
    if (rc != 0) {
        ERRLOG("Ndmp connection error.");
        return rc;
    }
    // send connect open cmd
    ndmp_connect_open_request_v4 open_request;
    ndmp_connect_open_reply_v4			    *open_reply = NULL;

    open_request.protocol_version = NDMPVER;
    rc = ndmpSendRequest(*ndmp_connection, NDMP_CONNECT_OPEN, NDMP_NO_ERR,
        (void*) &open_request, (void **) &open_reply);

    if (NDMP_ILLEGAL_ARGS_ERR == rc) {
        open_request.protocol_version = NDMPV3;  // ndmp v3
        rc = ndmpSendRequest(*ndmp_connection, NDMP_CONNECT_OPEN, NDMP_NO_ERR,
            (void*) &open_request, (void **) &open_reply);
    }

    if (rc != 0) {
        ERRLOG("Send connection open error");
        return NDMPCOPY_CONNECT_ERR;
    }

    if (NDMP_NO_ERR != open_reply->error)
    {
        ERRLOG("send connection reply error");
        rc = open_reply->error;
        ndmpFreeMessage(*ndmp_connection);
        return rc;
    }

    ndmpSetVersion(*ndmp_connection, open_request.protocol_version);
    ndmpFreeMessage(*ndmp_connection);

    return 0;
}

/*
 * create_md5_digest
 *
 * This function uses the MD5 message-digest algorithm described
 * in RFC1321 to authenticate the client using a shared secret (password).
 * The message used to compute the MD5 digest is a concatenation of password,
 * null padding, the 64 byte fixed length challenge and a repeat of the
 * password. The length of the null padding is chosen to result in a 128 byte
 * fixed length message. The lengh of the padding can be computed as
 * 64 - 2*(length of the password). The client digest is computed using the
 * server challenge from the NDMP_CONFIG_GET_AUTH_ATTR reply.
 *
 * Parameters:
 *   digest (output) - 16 bytes MD5 digest
 *   passwd (input) - user password
 *   challenge (input) - 64 bytes server challenge
 *
 * Returns:
 *   void
 */
void create_md5_digest(unsigned char *digest, char *passwd, const char *challenge)
{
    char buf[130];
    char *p = &buf[0];
    int len, i;
    MD5_CTX md;
    char *pwd;
    memset_s(buf, 130, 0, 130);

    *p  = 0;
    pwd = passwd;
    if ((len = strlen(pwd)) > MD5_PASS_LIMIT)
    {
        len = MD5_PASS_LIMIT;
    }

    (void) memcpy_s(p, sizeof(buf), pwd, len);
    p += len;

    for (i = 0; i < MD5_CHALLENGE_SIZE - 2 * len; ++i)
    {
        *p++ = 0;
    }

    (void) memcpy_s(p, sizeof(buf) - (p - buf), challenge, MD5_CHALLENGE_SIZE);
    p += MD5_CHALLENGE_SIZE;
    (void) strncpy_s(p, sizeof(buf) - (p - buf), pwd, len);

    (void)MD5_Init(&md);
    (void)MD5_Update(&md, buf, 128);
    (void)MD5_Final(digest, &md);

    /* for security, clear memory of password */
    (void)memset_s(buf, sizeof(buf), 0, sizeof(buf));
}

 void ndmpGetAuthAttrChallenge(char *challenge, ndmp_config_get_auth_attr_reply *auth_attr_reply)
 {
    char challenge_array[65] = {0};
    memcpy_s(challenge_array, 64, auth_attr_reply->server_attr.ndmp_auth_attr_u.challenge, 64);
    challenge_array[65] = '\0';
    memset_s(challenge, 65, 0, 65);
    (void)memcpy_s(challenge, 64, auth_attr_reply->server_attr.ndmp_auth_attr_u.challenge, 64);
    challenge[64] = '\0';
 }

int ndmp_auth_connection(NdmpConnection *ndmp_connection, char *dst_host, struct AuthInfo *ainfo)
{
    ndmp_config_get_auth_attr_request   auth_attr_request;
    ndmp_config_get_auth_attr_reply     *auth_attr_reply = NULL;
    ndmp_connect_client_auth_request	auth_request;
    ndmp_connect_client_auth_reply		*auth_reply = NULL;
    unsigned char digest[16] = {0};
    char pwd_decode[MAX_AUTH_USER_PASSWORD_LEN + 1] = {0};
    int rc = 0;

    //加NDMP_CONFIG_GET_AUTH_ATTR命令字，发送MD5
    auth_attr_request.auth_type = NDMP_AUTH_MD5;
    rc = ndmpSendRequest(*ndmp_connection, NDMP_CONFIG_GET_AUTH_ATTR, NDMP_NO_ERR,
                        (void*) &auth_attr_request, (void**) &auth_attr_reply);
    if (rc != 0) {
        ERRLOG("Send config get auth attr error.");
	    return rc;
    }

    if (NDMP_NO_ERR != auth_attr_reply->error) {
        ERRLOG("Send config get auth attr reply error.", auth_attr_reply->error);
        rc = auth_attr_reply->error;
        ndmpFreeMessage(*ndmp_connection);
	    return rc;
    }

    char *challenge = (char*)malloc(65);
    (void)ndmpGetAuthAttrChallenge(challenge, auth_attr_reply);

    ndmpFreeMessage(*ndmp_connection);

    // NDMP_CONNECT_CLIENT_AUTH中的属性需要重新赋值
    auth_request.auth_data.auth_type = NDMP_AUTH_MD5;
	auth_request.auth_data.ndmp_auth_data_u.auth_md5.user = ainfo->user;

    // If auth password is encrypted, decrypted here
    (void) strcpy_s(pwd_decode, sizeof(pwd_decode), ainfo->password);
    create_md5_digest(digest, pwd_decode, challenge);
    memcpy_s(auth_request.auth_data.ndmp_auth_data_u.auth_md5.auth_digest, 16, digest, 16);
    ndmpFreeMessage(*ndmp_connection);
    rc = ndmpSendRequest(*ndmp_connection, NDMP_CONNECT_CLIENT_AUTH, NDMP_NO_ERR,
				(void *)&auth_request, (void **) &auth_reply);
    if (rc != 0) {
        ERRLOG("Send connect client auth error.");
	    return rc;
    }

    if (NDMP_NO_ERR != auth_reply->error) {
        ERRLOG("Send connect client auth reply error %d", auth_reply->error);
        rc = auth_reply->error;
        ndmpFreeMessage(*ndmp_connection);
	    return rc;
    }

    ndmpSetAuthorized(*ndmp_connection, TRUE);
    ndmpFreeMessage(*ndmp_connection);
    return 0;
}

void NdmpSetExtInfo()
{
    ndmp_config_get_ext_list_reply_v4 *extReply = NULL;
    int rc = ndmpSendRequest(g_srcConnection, NDMP_CONFIG_GET_EXT_LIST, NDMP_NO_ERR,
                NULL, (void **)&extReply);
    if (rc != 0) {
        WARNLOG("NDMP_CONFIG_GET_EXT_LIST request send failed !");
    }

    ndmp_config_set_ext_list_request_v4 setExtReq;
    ndmp_config_set_ext_list_reply_v4 *setExtReply = NULL;
    g_setExt[0].ext_class_id = 0x00002050;
    g_setExt[0].ext_version = 0x00000003;
    g_setExt[1].ext_class_id = 0x00002051;
    g_setExt[1].ext_version = 0x00000001;
    g_setExt[2].ext_class_id = 0x00002052;
    g_setExt[2].ext_version = 0x00000001;
    setExtReq.ndmp_selected_ext.ndmp_selected_ext_len = 3;
    setExtReq.ndmp_selected_ext.ndmp_selected_ext_val = g_setExt;
    rc = ndmpSendRequest(g_srcConnection, NDMP_CONFIG_SET_EXT_LIST, NDMP_NO_ERR,
                (void *)&setExtReq, (void **)&setExtReply);
    if (rc != 0) {
        WARNLOG("NDMP_CONFIG_SET_EXT_LIST request send failed !");
    }
}

int NdmpGetProductType()
{
    // 获取生产端产品型号
    ndmp_config_get_host_info_reply_v4 *hostReply = NULL;
    int rc = ndmpSendRequest(g_srcConnection, NDMP_CONFIG_GET_HOST_INFO, NDMP_NO_ERR,
                NULL, (void **)&hostReply);
    if (rc == 0) {
        WARNLOG("NDMP_CONFIG_GET_HOST_INFO request send, osType:%s!", hostReply->os_type);
        if (strstr(hostReply->os_type, DORADO_OS) != NULL) {
            return PRODUCT_DORADO;
        }

        if (strstr(hostReply->os_type, NETAPP_OS) != NULL) {
            return PRODUCT_NETAPP;
        }

        if (strstr(hostReply->os_type, ISILON_OS) != NULL) {
            return PRODUCT_ISILON;
        }

        if (strstr(hostReply->os_type, UNITY_OS) != NULL) {
            return PRODUCT_UNITY;
        }
    }
    return PRODUCT_DORADO;
}

int NdmpClientAuth(NdmpClientInterface *interface)
{
    // log
    log_auth_params(interface);
    int rc = 0;
    init_ndmp_connections();
    rc = init_auth(interface);
    if (rc != 0) {
        ERRLOG("init auth failed!");
        return -1;
    }
    // tcp 连接
    rc = ndmp_open_connection(&g_srcConnection, interface->srcIp, interface->port);
    if (rc != 0) {
        ERRLOG("error connect to src ip : %s", interface->srcIp);
        return NDMP_AUTH_ERROR_SRC;
    }

    ndmp_config_get_server_info_reply_v4 *serverInfoReply = NULL;
    rc = ndmpSendRequest(g_srcConnection, NDMP_CONFIG_GET_SERVER_INFO, NDMP_NO_ERR,
        NULL, (void **) &serverInfoReply);
    // 发送auth
    rc = ndmp_auth_connection(&g_srcConnection, interface->srcIp, &g_srcAuth);
    if (rc != 0) {
        ERRLOG("error auth to src : %s", g_srcAuth.user);
        return NDMP_AUTH_ERROR_SRC;
    }
    int type = NdmpGetProductType();
    NdmpSetExtInfo();
    INFOLOG("auth success!");

    return type;
}

int NdmpGetFsInfo(NdmpFsInfo* reply)
{
    int rc = 0;
    NdmpSetExtInfo();
    ndmp_config_get_fs_info_reply_v4	*fs_reply = NULL;
    rc = ndmpSendRequest(g_srcConnection, NDMP_CONFIG_GET_FS_INFO, NDMP_NO_ERR,
                NULL, (void **)&fs_reply);
    if (rc != 0) {
        ERRLOG("send get fs info cmd error.");
        return -1;
    }

    if (NDMP_NO_ERR != fs_reply->error) {
        ERRLOG("Send get fs info reply error %d", fs_reply->error);
        rc = fs_reply->error;
        ndmpFreeMessage(g_srcConnection);
	    return fs_reply->error;
    }
    INFOLOG("get fs info success!");
    reply->fs_info_val = (FSInfo *)malloc(sizeof(struct FSInfo) * fs_reply->fs_info.fs_info_len);
    if (reply == NULL || reply->fs_info_val == NULL) {
        ERRLOG("malloc failed!");
        return -1;
    }

    reply->error = fs_reply->error;
    reply->fs_info_len = fs_reply->fs_info.fs_info_len;

    int i = 0;
    for (i; i < fs_reply->fs_info.fs_info_len; ++i) {
        reply->fs_info_val[i].invalid = fs_reply->fs_info.fs_info_val[i].invalid;
        reply->fs_info_val[i].fs_type = (char*)malloc(sizeof(char) *
            (strlen(fs_reply->fs_info.fs_info_val[i].fs_type) + 1));
        // fill type
        strncpy_s(reply->fs_info_val[i].fs_type,
            strlen(fs_reply->fs_info.fs_info_val[i].fs_type) + 1,
            fs_reply->fs_info.fs_info_val[i].fs_type,
            strlen(fs_reply->fs_info.fs_info_val[i].fs_type) + 1);
        // fill logic device
        reply->fs_info_val[i].fs_logical_device = (char*)malloc(sizeof(char) *
            (strlen(fs_reply->fs_info.fs_info_val[i].fs_logical_device) + 1));
        strncpy_s(reply->fs_info_val[i].fs_logical_device,
            strlen(fs_reply->fs_info.fs_info_val[i].fs_logical_device) + 1,
            fs_reply->fs_info.fs_info_val[i].fs_logical_device,
            strlen(fs_reply->fs_info.fs_info_val[i].fs_logical_device) + 1);
        // fill phsical device
        reply->fs_info_val[i].fs_physical_device = (char*)malloc(sizeof(char) *
            (strlen(fs_reply->fs_info.fs_info_val[i].fs_physical_device) + 1));
        strncpy_s(reply->fs_info_val[i].fs_physical_device,
            strlen(fs_reply->fs_info.fs_info_val[i].fs_physical_device) + 1,
            fs_reply->fs_info.fs_info_val[i].fs_physical_device,
            strlen(fs_reply->fs_info.fs_info_val[i].fs_physical_device) + 1);            
        // fill status
        reply->fs_info_val[i].fs_status = (char*)malloc(sizeof(char) *
            (strlen(fs_reply->fs_info.fs_info_val[i].fs_status) + 1));
        strncpy_s(reply->fs_info_val[i].fs_status,
            strlen(fs_reply->fs_info.fs_info_val[i].fs_status) + 1,
            fs_reply->fs_info.fs_info_val[i].fs_status,
            strlen(fs_reply->fs_info.fs_info_val[i].fs_status) + 1);
    }
    INFOLOG("set reply fs info error: %d, len : %u", reply->error, reply->fs_info_len);
    ndmpFreeMessage(g_srcConnection);
    return 0;
}

void log_backup_params(NdmpClientInterface *interface)
{
    INFOLOG("log backup params: (srcpath: %s), (dstpath: %s), (level: %s),"
        "(dumpType: %d[0:dorado, 1:netapp]), (port: %d), (exclude: %s)", 
    interface->srcPath, interface->dstPath, interface->level,
    interface->dumpType, interface->port, interface->exclude);
}

int check_backup_params(NdmpClientInterface *interface){
    if (strlen(interface->srcIp) == 0){
        ERRLOG("error no src ip");
        return -1;
    }

    if (strlen(interface->dstIp) == 0){
        ERRLOG("error no dst ip");
        return -1;
    }
    // log
    log_backup_params(interface);

    return 0;
}

void initSrcBackup(NdmpClientInterface *interface)
{ 
    g_environment[0].name = "UPDATE";
    g_environment[0].value = "Y";
    g_environment[1].name = "TYPE";
    g_environment[1].value = "dump";
    g_environment[2].name = "HIST";
    g_environment[2].value = "Y";
    g_environment[3].name = "FILESYSTEM";
    g_environment[3].value = interface->srcPath;
    g_environment[4].name = "LEVEL";
    g_environment[4].value = interface->level;
    g_environment[5].name = "DIRECT";
    g_environment[5].value = "N";
    g_environment[6].name = "RECURSIVE";
    g_environment[6].value = "Y";
    g_environment[7].name = "EXCLUDE";
    g_environment[7].value = interface->exclude;
    g_backupFilePath = (char *)malloc(NDMPD_DATA_MAX_FILE_PATH);
    g_dataPath = interface->backFilePath;
    if (g_backupFilePath == NULL) {
        ERRLOG("g_backupfilepath malloc failed!");
        return;
    }

    NdmpinitFilepath(g_backupFilePath, g_dataPath,  NDMPD_DEFAULT_BACKUP_PROCESS_MSG_FILE_NAME);
}

int ndmp_src_backup(NdmpClientInterface *interface)
{
    ndmp_data_start_backup_request_v4 dump_request;
    ndmp_data_start_backup_reply_v4 *dump_reply = NULL;

    int rc = 0;

    g_environment[0].name = "TYPE";
    g_environment[0].value = "dump";
    g_environment[1].name = "FILESYSTEM";
    g_environment[1].value = interface->srcPath;
    g_environment[2].name = "PREFIX";
    g_environment[2].value = interface->srcPath;
    g_environment[3].name = "LEVEL";
    g_environment[3].value = interface->level;
    g_environment[4].name = "HIST";
    g_environment[4].value = "Y";
    g_environment[5].name = "UPDATE";
    g_environment[5].value = "Y";
    g_environment[6].name = "EXCLUDE";
    g_environment[6].value = interface->exclude;
    dump_request.env.env_val = g_environment;
    dump_request.env.env_len = 7;
    dump_request.bu_type = "dump";

    rc = ndmpSendRequest(g_srcConnection, NDMP_DATA_START_BACKUP, NDMP_NO_ERR,
        (void*)&dump_request, (void**)&dump_reply);

    if (rc != 0) {
        ERRLOG("Send data start backup error.");
        return rc;
    }

    if (NDMP_NO_ERR != dump_reply->error) {
        ERRLOG("Send data start backup reply error\n.");
        rc = dump_reply->error;
        ndmpFreeMessage(g_srcConnection);
        return rc;
    }

    ndmpFreeMessage(g_srcConnection);
    return 0;
}

void initNdmpDstRestore(NdmpClientInterface *interface)
{
    g_environment[0].name = "UPDATE";
    g_environment[0].value = "Y";
    g_environment[1].name = "TYPE";
    g_environment[1].value = "dump";
    g_environment[2].name = "HIST";
    g_environment[2].value = "Y";
    g_environment[3].name = "LEVEL";
    g_environment[3].value = interface->level;
    g_environment[4].name = "EXTRACT";
    g_environment[4].value = "Y";

    g_backupFilePath = (char *)malloc(NDMPD_DATA_MAX_FILE_PATH);
    if (g_backupFilePath == NULL) {
        ERRLOG("g_backupfilepath malloc failed!");
        return;        
    }

    NdmpinitFilepath(g_backupFilePath, interface->backFilePath, NDMPD_DEFAULT_RESTORE_PROCESS_MSG_FILE_NAME);
}

void ndmpDstRestoreFreeNlist(ndmp_name_v3 *nlist, uint64_t nlistLen, int level)
{
    if (nlist == NULL) {
        return;
    }

    if (level == 0) {
        MODULE_FREE(nlist);
        return;
    }

    for (int i = 0; i < nlistLen; ++i) {
        MODULE_FREE(nlist[i].original_path);
        MODULE_FREE(nlist[i].destination_dir);
    }
    MODULE_FREE(nlist);
}

void NdmpGetRestoreTmpFileName(char *fileName, char *path)
{
    strcpy_s(fileName, NDMPD_DATA_MAX_FILE_PATH, path);
    strcat_s(fileName, NDMPD_DATA_MAX_FILE_PATH, "/");
    strcat_s(fileName, NDMPD_DATA_MAX_FILE_PATH, NDMPD_DEFAULT_RESTORE_FILE_TMP);
}

FILE *NdmpOpenFileWithRetry(char *fileName, char *mode) {
    int retryCount = 0;
    FILE *f = NULL;
    do {
        f = fopen(fileName, mode);
    } while (++retryCount <= MAX_RETRY_CNT && f == NULL);

    return f;
}

int ParseNdmpRestoreFilesTmpFile(ndmp_name_v3 *nList, uint64_t nlistSize, char *fileName)
{
    DBGLOG("fileName:%s", fileName);
    char *tmpOriginStr = NULL;
    FILE *f = NdmpOpenFileWithRetry(fileName, "r");
    if (f == NULL) {
        ERRLOG("open tmp file faild! errno: %d, error message: %s", errno, strerror(errno));
        return -1;
    }
 
    int outNum = 0;
    char fileLine[NDMPD_DATA_DEFAULT_BUFFER_SIZE] = {0};
    bzero(fileLine, NDMPD_DATA_DEFAULT_BUFFER_SIZE);
    fgets(fileLine, NDMPD_DATA_DEFAULT_BUFFER_SIZE, f);
    for (int line = 0; line < nlistSize; ++line) {
        fgets(fileLine, NDMPD_DATA_DEFAULT_BUFFER_SIZE, f);
        DBGLOG("fileLine:%s", fileLine);
        nList[line].new_name = "";
        nList[line].other_name = "";
        char *leftStr = NULL;
        char *originStr = strtok_r(fileLine, "|", &leftStr);
        tmpOriginStr = (char *)malloc(strlen(originStr) + 1);
        if (tmpOriginStr == NULL) {
            ndmpDstRestoreFreeNlist(nList, outNum, 1);
            fclose(f);
            return -1;
        }

        if (memcpy_s(tmpOriginStr, strlen(originStr) + 1, originStr, strlen(originStr) + 1) != 0) {
            ERRLOG("memcpy failed!");
            goto ERR;
        }
 
        char *dstStr = strtok_r(leftStr, "|", &leftStr);
        char *tmpDstStr = (char *)malloc(strlen(dstStr) + 1);
        CHECK_NULL_POINTER_GOTO(tmpDstStr, ERR);
        if (memcpy_s(tmpDstStr, strlen(dstStr) + 1, dstStr, strlen(dstStr) + 1)) {
            ERRLOG("memcpy failed!");
            MODULE_FREE(tmpDstStr);
            goto ERR;
        }
        nList[line].original_path = tmpOriginStr;
        nList[line].destination_dir = tmpDstStr;
        nList[line].node = longLongToQuad(atol(strtok_r(leftStr, "|", &leftStr)));
        nList[line].fh_info = longLongToQuad(atol(strtok_r(leftStr, "|", &leftStr)));
        DBGLOG("nList[%d]: %s, %s", line,
            nList[line].original_path, nList[line].destination_dir);
        ++outNum;
    }
 
    fclose(f);
    return 0;
 
ERR:
    fclose(f);
    MODULE_FREE(tmpOriginStr);
    ndmpDstRestoreFreeNlist(nList, outNum, 1);
    return -1;
}
 
int BuildNdmpRestoreFiles(ndmp_name_v3 *nList, NdmpClientInterface *interface, uint64_t nlistSize)
{
    int rc = 0;
    if (atoi(interface->level) == 0) {
        nList[0].new_name = "";
        nList[0].other_name = "";
        nList[0].node = longLongToQuad(0LL);
        nList[0].fh_info = longLongToQuad(0LL);
        nList[0].original_path = "/";
        nList[0].destination_dir = interface->dstPath;
    } else {
        char fileName[NDMPD_DATA_MAX_FILE_PATH] = { 0 };
        NdmpGetRestoreTmpFileName(fileName, interface->backFilePath);
        DBGLOG("fileName:%s", fileName);
        rc = ParseNdmpRestoreFilesTmpFile(nList, nlistSize, fileName);
    }
 
    return rc;
}
 
int NdmpeGetRestoreGetFileNum(char *restoreFile)
{
    char fileName[NDMPD_DATA_MAX_FILE_PATH] = { 0 };
    NdmpGetRestoreTmpFileName(fileName, restoreFile);
    FILE *f = NdmpOpenFileWithRetry(fileName, "r");
    char num[NDMPD_DEFAULT_RESTORE_FILE_NUM] = { 0 };
    if (f == NULL) {
        ERRLOG("open tmp file faild! errno: %d, error message: %s", errno, strerror(errno));
        return 1;
    }
 
    fgets(num, NDMPD_DEFAULT_RESTORE_FILE_NUM, f);
    fclose(f);
    return atol(num);    
}

int ndmp_dst_restore(NdmpClientInterface *interface)
{
    ndmp_data_start_recover_request_v4  restore_request;
    ndmp_data_start_recover_reply_v4   *restore_reply = NULL;

    int rc = 0;
    uint32_t path_type = VBAD;
    int level = atoi(interface->level);
    uint64_t nlistSize = level == 0 ? 1 : NdmpeGetRestoreGetFileNum(interface->backFilePath);
    INFOLOG("get nlistSize:%d", nlistSize);
    ndmp_name_v3 *nllist = (ndmp_name_v3 *)malloc(sizeof(ndmp_name_v3) * nlistSize);
    CHECK_NULL_POINTER_RETURN(nllist, -1);
    rc = BuildNdmpRestoreFiles(nllist, interface, nlistSize);
    if (rc != 0) {
        ERRLOG("parse file restore failed.");
        return rc;
    }

    char y[] = "y";
    char n[] = "n";
    g_environment[5].name = "FILESYSTEM";
    g_environment[5].value = interface->srcPath;
    g_environment[6].name = "PREFIX";
    g_environment[6].value = interface->srcPath;

    restore_request.env.env_val = g_environment;
    restore_request.env.env_len = 7;
    restore_request.bu_type = "dump";
    restore_request.nlist.nlist_val = nllist;
    restore_request.nlist.nlist_len = nlistSize;

    rc = ndmpSendRequest(g_srcConnection, NDMP_DATA_START_RECOVER, NDMP_NO_ERR,
        (void*)&restore_request, (void**)&restore_reply);
    if (rc != 0) {
        ERRLOG("Send data start recover error. %d", rc);
        ndmpDstRestoreFreeNlist(nllist, nlistSize, level);
        return rc;
    }

    if (NDMP_NO_ERR != restore_reply->error) {
        ERRLOG("Send data start recover reply error. %d", restore_reply->error);
        rc = restore_reply->error;
        ndmpFreeMessage(g_srcConnection);
        ndmpDstRestoreFreeNlist(nllist, nlistSize, level);
        return rc;
    }
    ndmpFreeMessage(g_srcConnection);
    ndmpDstRestoreFreeNlist(nllist, nlistSize, level);
    return 0;
}

void init_ndmp_data_ipv6_connect_param(ndmp_data_connect_request_v4 *dataReq, char *addr6, unsigned short port)
{
    dataReq->addr.tcp_ipv6_addr_v4 = NULL;
    dataReq->addr.addr_type = NDMP_ADDR_TCP_IPV6;
    dataReq->addr.tcp_ipv6_addr_v4 = (ndmp_tcp_ipv6_addr_v4*)malloc(sizeof(ndmp_tcp_ipv6_addr_v4));
    (void)memset_s(dataReq->addr.tcp_ipv6_addr_v4, sizeof(ndmp_tcp_ipv6_addr_v4), 0, sizeof(ndmp_tcp_ipv6_addr_v4));
    (void)memcpy_s(&dataReq->addr.tcp_ipv6_v4(0), sizeof(struct in6_addr), addr6, sizeof(struct in6_addr));
    dataReq->addr.tcp_ipv6_port_v4(0) = port;
    dataReq->addr.tcp_ipv6_len_v4 = 1;
    dataReq->addr.tcp_ipv6_env_v4(0).addr_env_len = 0;
    dataReq->addr.tcp_ipv6_env_v4(0).addr_env_val = NULL;
}

void init_ndmp_data_ipv4_connect_param(ndmp_data_connect_request_v4 *dataReq,
    unsigned long addr, unsigned short port)
{
    dataReq->addr.tcp_addr_v4 = NULL;
    dataReq->addr.addr_type = NDMP_ADDR_TCP;
    dataReq->addr.tcp_addr_v4 = (ndmp_tcp_addr_v4*)malloc(sizeof(ndmp_tcp_addr_v4));
    (void)memset_s(dataReq->addr.tcp_addr_v4, sizeof(ndmp_tcp_addr_v4), 0, 
        sizeof(ndmp_tcp_addr_v4));
    dataReq->addr.tcp_len_v4 = 1;
    dataReq->addr.tcp_ip_v4(0) = (unsigned int)htonl(addr);
    dataReq->addr.tcp_port_v4(0) = port;
    dataReq->addr.tcp_env_v4(0).addr_env_len = 0;
    dataReq->addr.tcp_env_v4(0).addr_env_val = NULL;
}

// data listen cmd to dst ,  data connect cmd to src
int ndmpDataConnectSendRequest(ndmp_data_connect_request_v4 *data_connect_request, ndmp_data_connect_reply_v4 *data_connect_reply,
    unsigned short port, int fd)
{
    int rc = 0;

    rc = ndmpSendRequest(g_srcConnection, NDMP_DATA_CONNECT, NDMP_NO_ERR,
        (void*)data_connect_request, (void**)&data_connect_reply);
    MODULE_FREE(data_connect_request->addr.tcp_addr_v4);
    MODULE_FREE(data_connect_request->addr.tcp_ipv6_addr_v4);
    if (rc != 0) {
        ERRLOG("Send data connect error.");
        return rc;
    }
    rc = NdmpCLientCreateDataConnect(port, fd);
    if (0 != rc) {
        ERRLOG("Send data connect error.");
        return rc;
    }

    if (NDMP_NO_ERR != data_connect_reply->error) {
        ERRLOG("Data connect reply error.");
        rc = data_connect_reply->error;
        ndmpFreeMessage(g_srcConnection);
        return rc;
    }

    ndmpFreeMessage(g_srcConnection);
    return 0;
}

int ndmp_data_connect(NdmpClientInterface *interface, uint32_t *isDataListen)
{
    ndmp_data_connect_request_v4 data_connect_request;
    ndmp_data_connect_reply_v4 *data_connect_reply;
    int rc = 0;
    int addrType = 0;
    unsigned short port;
    unsigned long addr;
    char addr6[16] = { 0 };
    int fd;

    if (NULL == strchr(interface->dstIp, ':')) {
        addrType = NDMP_ADDR_TCP;
    } else {
        addrType = NDMP_ADDR_TCP_IPV6;
    }
    *isDataListen = 1;

    if (addrType == NDMP_ADDR_TCP) {
        rc = NdmpClientCreateDataIpv4Listener(interface->dstIp, &port, &fd, &addr);
        if (rc != 0) {
            ERRLOG("ndmp client get listener failed, ret:%d.", rc);
            return -1;
        }

        init_ndmp_data_ipv4_connect_param(&data_connect_request, addr, port);
    } else if (addrType == NDMP_ADDR_TCP_IPV6) {
        rc = NdmpClientCreateDataIpv6Listener(interface->dstIp, &port, &fd, addr6);
        if (rc != 0) {
            ERRLOG("ndmp client get listener failed, ret:%d.", rc);
            return -1;
        }

        init_ndmp_data_ipv6_connect_param(&data_connect_request, addr6, port);
    } else {
        ERRLOG("Data addr type (%d) error.", addrType);
        return -1;
    }

    rc = ndmpDataConnectSendRequest(&data_connect_request, data_connect_reply, port, fd);

    return rc;
}

void ndmp_get_connect_stat(NdmpConnection *ndmp_connection, bool is_src)
{
    ndmp_data_get_state_reply_v4 *get_state_reply = NULL;
    int					 rc = 0;
    (void) pthread_mutex_lock(&g_connectionMutex);
    rc = ndmpSendRequest(*ndmp_connection, NDMP_DATA_GET_STATE, NDMP_NO_ERR,
				0, (void **) &get_state_reply);
    if (rc != 0) 
    {
        ERRLOG("Send data get state error, %d, %d, %d", rc, is_src, true);
        (void) pthread_mutex_unlock(&g_connectionMutex);
        return ;
    }
    if (is_src) {
        (void) pthread_mutex_lock(&g_dataMutex);
        g_srcProcessBytes = ((uint64_t)get_state_reply->bytes_processed.high << 32) | get_state_reply->bytes_processed.low;
        (void) pthread_mutex_unlock(&g_dataMutex);
        DBGLOG("ndmp get stat process bytes from src: %llu", g_srcProcessBytes);
        if (get_state_reply->est_bytes_remain.high > 0 ||
            get_state_reply->est_bytes_remain.low > 0) {
            uint64_t remain_bytes = (uint64_t)get_state_reply->est_bytes_remain.high;
            remain_bytes = remain_bytes << 32 + get_state_reply->est_bytes_remain.low;
            (void) pthread_mutex_lock(&g_dataMutex);
            g_srcRemainingSize = remain_bytes;
            (void) pthread_mutex_unlock(&g_dataMutex);
        }
    }

    ndmpFreeMessage(*ndmp_connection);
    (void) pthread_mutex_unlock(&g_connectionMutex);
    return;
}

int ndmp_check_connect_socket(NdmpConnection	connectionHandle)
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

void ndmp_abort_backup()
{
    set_ndmp_status(NDMP_STATUS_ABORTED);
    INFOLOG("send abort backup request");
    int rc = 0;
    ndmp_data_abort_reply *src_reply = NULL;
    ndmp_data_abort_reply *dst_reply = NULL;

    (void) pthread_mutex_lock(&g_connectionMutex);

    if (g_isSendAbort)
    {
        (void) pthread_mutex_unlock(&g_connectionMutex);
        return;
    }

    g_isSendAbort = true;

    if (NULL != g_srcConnection && ndmp_check_connect_socket(g_srcConnection))
    {
        rc = ndmpSendRequest(g_srcConnection, NDMP_DATA_ABORT, NDMP_NO_ERR,
                             0, (void**)&src_reply);

        if (rc != 0)
        {
            ERRLOG("Send src data abort error.");
        }
        else
        {
            ndmpFreeMessage(g_srcConnection);
        }
    }

    (void) pthread_mutex_unlock(&g_connectionMutex);

    return;    
}

int send_ndmp_stop(NdmpConnection* ndmp_connection)
{
    ndmp_data_stop_reply *stop_reply = NULL;
    int rc = 0;
    (void) pthread_mutex_lock(&g_connectionMutex);
    rc = ndmpSendRequest(*ndmp_connection, NDMP_DATA_STOP, NDMP_NO_ERR,
        0, (void**)&stop_reply);
    if (rc != 0) {
        ERRLOG("send data stop error;");
        (void) pthread_mutex_unlock(&g_connectionMutex);
        return rc;
    }

    ndmpFreeMessage(*ndmp_connection);
    (void) pthread_mutex_unlock(&g_connectionMutex);
    return rc;
}

void ndmp_stop_data_conn()
{
    INFOLOG("send stop data connection request");
    int rc = 0;
    rc = send_ndmp_stop(&g_srcConnection);
    if (rc != 0) {
        ERRLOG("send src data stop error %d", rc);
    }
}

int shutdown_connection()
{
    int rc = 0;
    // flush ndmp messages on both connections
    (void) pthread_mutex_lock(&g_connectionMutex);
    if (NULL != g_srcConnection) {
        if (check_connect_socket(g_srcConnection)) {
            DBGLOG("Backup:pass ndmpPoll.");
            rc = ndmpSendRequest(g_srcConnection, NDMP_CONNECT_CLOSE, NDMP_NO_ERR, NULL, NULL);
            if (rc != 0) {
                WARNLOG("Error closing src connection.");
            } else {
                ndmpFreeMessage(g_srcConnection);
            }
        }
    }
    (void) pthread_mutex_unlock(&g_connectionMutex);
    return 0;
}

void free_auth_pwd(NdmpClientInterface *interface)
{
    MODULE_FREE(interface->srcPwd);
    MODULE_FREE(interface->dstPwd);
}

void close_thread(pthread_t *pt)
{
    if (*pt != 0) {
        pthread_join(*pt, NULL);
        *pt = 0;
    }
    return;
}

void* get_backup_stat(void* arg)
{
    // 10s 查一次
    int time_intv = 10;
    int rc = 0;
    int st;

    while (true) {
        sleep(time_intv);
        (void) pthread_mutex_lock(&g_dataMutex);
        st = (int)g_ndmpStatus;
        (void) pthread_mutex_unlock(&g_dataMutex);
        if (st != NDMP_STATUS_PROCESSING) {
            break;
        }
        INFOLOG("get stat");
        if (g_srcConnection != NULL) {
            ndmp_get_connect_stat(&g_srcConnection, true);
        }
    }
    (void) pthread_mutex_lock(&g_connectionMutex);
    ndmpDestroyConnection(g_srcConnection);
    g_srcConnection = NULL;
    (void) pthread_mutex_unlock(&g_connectionMutex);
    INFOLOG("exit get stat cycle");
    return NULL;
}

void ndmp_end_backup()
{
    INFOLOG("enter ndmp_end_backup");
    MODULE_FREE(g_backupFilePath);
    ndmp_stop_data_conn();

    shutdown_connection();

    return;
}

void writeProcessMsg(char *msg)
{
    FILE *f = NdmpOpenFileWithRetry(g_backupFilePath, "a+");
    if (f == NULL) {
        ERRLOG("write log message file faild! errno: %d, error message: %s", errno, strerror(errno));
        return;
    }

    fprintf(f,"%s", msg);
    fclose(f);
    return;
}

void NdmpReleaseFhAddDirReq(NdmpGetFhAddDir *dirReq, unsigned int len)
{
    if (dirReq == NULL) {
        return;
    }
 
    for (int i = 0; i < len; i++) {
        MODULE_FREE(dirReq->dirs[i].path);
    }
    MODULE_FREE(dirReq->dirs);
    MODULE_FREE(dirReq);
}

int NdmpParseFhAddDirRequest(ndmp_fh_add_dir_request_v4 *req, NdmpGetFhAddDir *addDirs)
{
    unsigned int sucNum = 0;
    addDirs->dir_len = req->dirs.dirs_len;
    for (int i = 0; i < req->dirs.dirs_len; i++) {
        addDirs->dirs[i].node = quadToLongLong(req->dirs.dirs_val[i].node);
        addDirs->dirs[i].parent = quadToLongLong(req->dirs.dirs_val[i].parent);
        int pathLen = strlen(req->dirs.dirs_val[i].names.names_val[0].ndmp_file_name_v3_u.unix_name);
        addDirs->dirs[i].pathLen = pathLen;
        addDirs->dirs[i].path = (char *)malloc(pathLen + 1);
        if (addDirs->dirs[i].path == NULL) {
            ERRLOG("malloc faild!");
            goto ERR;
        }
        ++sucNum;
        if (memcpy_s(addDirs->dirs[i].path, pathLen + 1, req->dirs.dirs_val[i].names.names_val[0].ndmp_file_name_v3_u.unix_name,
            pathLen + 1) != 0) {
            MODULE_FREE(addDirs->dirs[i].path);
            ERRLOG("malloc faild!");
            return -1;
        }
    }
 
    return 0;

ERR:
    NdmpReleaseFhAddDirReq(addDirs, sucNum);
    return -1;
}
 
/*
    将得到文件和文件夹先写入临时文件中，文件名为FILE_CACHE_TMP,记录为:
    nodeid parentId pathLen path
      8 + 2 + 8 +  2 + 4 + 2 + 1024   = 1050 字节   
*/
int NdmpWriteTmpFhAddDirFile(NdmpGetFhAddDir *dirs)
{
    int rc = 0;
    char input[NDMP_CACHE_NODE_DEFAULE_INFO_LINE_SIZE] = { 0 };
    char aggr[NDMPD_DATA_AGGR_BUFFER_SIZE] = {0};
    int offset = 0;
    FILE *cacheFile = NULL;
    char fileName[NDMPD_DATA_MAX_FILE_PATH] = { 0 };
    NdmpinitFilepath(fileName, g_dataPath, "FILE_CACHE_TMP");
    cacheFile = NdmpOpenFileWithRetry(fileName, "a");
    if (cacheFile == NULL) {
        ERRLOG("Open file failed!");
        return -1;
    }
    char *escaped = (char *)malloc(NDMP_CACHE_NODE_DEFAULE_INFO_LINE_SIZE);
    if (!escaped) {
        ERRLOG("escaped malloc failed!");
        return -1;
    }
    for (int i = 0; i < dirs->dir_len; i++) {
        unsigned int newLen = dirs->dirs[i].pathLen;
        unsigned int k = 0;
        for (unsigned int j = 0; j < dirs->dirs[i].pathLen; j++) {
            char ch = dirs->dirs[i].path[j];
            if (ch == '"' || ch == '\\') {
                escaped[k++] = '\\';
                newLen++;
            }
            escaped[k++] = ch;
        }
        escaped[k] = '\0';  // 确保字符串结束
        bzero(input, NDMP_CACHE_NODE_DEFAULE_INFO_LINE_SIZE);
        rc = sprintf_s(input, NDMP_CACHE_NODE_DEFAULE_INFO_LINE_SIZE,
            "{\"nodeId\": \"%lld\", \"parentId\": \"%lld\", \"nameLen\": \"%u\", \"name\": \"%s\"}\n",
            dirs->dirs[i].node, dirs->dirs[i].parent, newLen, escaped);
        if (rc == -1) {
            ERRLOG("format input failed!");
            break;
        }
        memcpy_s(aggr + offset, strlen(input), input, strlen(input));
        offset += strlen(input);
    }
    free(escaped);
    escaped = NULL;
    if (fputs(aggr, cacheFile) < 0) {
        rc = -1;
        ERRLOG("input line failed!");
    }
    fclose(cacheFile);
    return rc < 0 ? rc : 0;
}
 
void NdmpReleaseFhAddNodeReq(NdmpGetFhAddNode *nodes)
{
    if (nodes == NULL) {
        return;
    }
 
    MODULE_FREE(nodes->files);
    MODULE_FREE(nodes);
}
 
int NdmpProcessFhAddDir(void *body)
{   
	int rc = 0;
    ndmp_fh_add_dir_request_v4 *req = (ndmp_fh_add_dir_request_v4 *)body;
    NdmpGetFhAddDir *addDirs = (NdmpGetFhAddDir *)malloc(sizeof(NdmpGetFhAddDir));
    if (addDirs == NULL) {
       ERRLOG("malloc failed");
        return -1;
    }

    addDirs->dirs = (NdmpDirs *)malloc(sizeof(NdmpDirs) * req->dirs.dirs_len);
    if (addDirs->dirs == NULL) {
        MODULE_FREE(addDirs);
        ERRLOG("malloc failed");
        return -1;
    }

    rc = NdmpParseFhAddDirRequest(req, addDirs);
    CHECK_RESULT_RETURN(rc, 0, "parse fh add dir request faild!");

    rc = NdmpWriteTmpFhAddDirFile(addDirs);   
    if (rc != 0) {
        ERRLOG("write faild");
        NdmpReleaseFhAddDirReq(addDirs, addDirs->dir_len);
        return rc;
    }

    // 释放内存
    NdmpReleaseFhAddDirReq(addDirs, addDirs->dir_len);
    return rc;
}
 
/*
    将得到文件和文件夹先写入临时文件中，文件名为node,记录为:
    node FhInfo ftype mtime size
*/
 
void NdmpFhAddNodeGenerateFileType(char *cType, int nType)
{
    switch (nType) {
        case NDMP_FILE_DIR:
			strcpy_s(cType, 2, "d");
            break;
        case NDMP_FILE_SLINK:
			strcpy_s(cType, 2, "l");
            break;
        default:
			strcpy_s(cType, 2, "f");
            break;
    }
}
int NdmpWriteTmpFhAddNodeFile(NdmpGetFhAddNode *nodes)
{
    int rc = 0;
    char input[NDMP_CACHE_NODE_DEFAULE_INFO_LINE_SIZE] = { 0 };
    char aggr[NDMPD_DATA_AGGR_BUFFER_SIZE] = {0};
    int offset = 0;
    FILE *cacheFile = NULL;
    char fileName[NDMPD_DATA_MAX_FILE_PATH] = { 0 };
    NdmpinitFilepath(fileName, g_dataPath, "NODE_CACHE_TMP");
    cacheFile = NdmpOpenFileWithRetry(fileName, "a");
    if (cacheFile == NULL) {
        ERRLOG("Open file failed!");
        return -1;
    }
 
    char cType[3] = { 0 };
    for (int i = 0; i < nodes->files_len; i++) {
        bzero(input, NDMP_CACHE_NODE_DEFAULE_INFO_LINE_SIZE);
        NdmpFhAddNodeGenerateFileType(cType, nodes->files[i].ftype);
        rc = sprintf_s(input, NDMP_CACHE_NODE_DEFAULE_INFO_LINE_SIZE,
            "{\"nodeId\": \"%lld\", \"fhInfo\": \"%lld\", \"type\": \"%s\", \"mtime\": \"%lld\", \"size\": \"%lld\"}\n",
            nodes->files[i].node, nodes->files[i].FhInfo, cType, nodes->files[i].mtime, nodes->files[i].size);
        if (rc == -1) {
            ERRLOG("format input failed!");
            break;
        }
        memcpy_s(aggr + offset, strlen(input), input, strlen(input));
        offset += strlen(input);
    }

    if (fputs(aggr, cacheFile) < 0) {
        rc = -1;
        ERRLOG("input line failed!");
    }
 
    fclose(cacheFile);
    return rc < 0 ? rc : 0;
}

int NdmpWriteTmpFhAddFiles(NdmpGetFhAddNode *nodes)
{
    int rc = 0;
    char input[NDMPD_DATA_DEFAULT_BUFFER_SIZE + 200] = { 0 };
    char tail[NDMPD_DATA_MAX_FILE_PATH] = { 0 };
    FILE *cacheFile = NULL;
    char fileName[NDMPD_DATA_MAX_FILE_PATH] = { 0 };
    NdmpinitFilepath(fileName, g_dataPath, "index.tmp");
    cacheFile = NdmpOpenFileWithRetry(fileName, "r");    // 先判断文件存不存在，存在append,不存在需要写第一行：
    if (cacheFile == NULL) {
        if (sprintf_s(tail, NDMPD_DATA_MAX_FILE_PATH,
           "{\"title\": \"Raw File-system Index Database\",\"version\": \"2.0\",\"time\": \"%ld\"}\n",
            (long)time(NULL)) == -1) {
            ERRLOG("format input failed!");
            return -1;
        }
    } else {
        fclose(cacheFile);
    }

    cacheFile = NdmpOpenFileWithRetry(fileName, "a");
    CHECK_NULL_POINTER_RETURN(cacheFile, -1);

    if (fputs(tail, cacheFile) < 0) {
        ERRLOG("input line failed!, errno:%d.", errno);
        fclose(cacheFile);
        return -1;
    }

    char cType[3] = { 0 };
    for (int i = 0; i < nodes->files_len; i++) {
        bzero(input, NDMPD_DATA_DEFAULT_BUFFER_SIZE + 200);
        if (strcmp("/", nodes->files[i].path) == 0) {
            continue;
        }
        NdmpFhAddNodeGenerateFileType(cType, nodes->files[i].ftype);
        rc = sprintf_s(input, NDMPD_DATA_DEFAULT_BUFFER_SIZE + 200,
            "{\"path\": \"%s\", \"mtime\": \"%lld\", \"size\": \"%lld\", \"inode\": \"%lld\", \"id\": \"%lld\", \"type\": \"%s\", \"status\": \"new\"}\n",
            nodes->files[i].path, nodes->files[i].mtime, nodes->files[i].size, nodes->files[i].node, nodes->files[i].FhInfo, cType);
        if (rc == -1) {
            ERRLOG("format input failed!");
            break;
        }
        if (fputs(input, cacheFile) < 0) {
            rc = -1;
            ERRLOG("input line failed!");
            break;
        }
    }
 
    fclose(cacheFile);
    return rc < 0 ? rc : 0;
}
 
void NdmpParseFhAddNodeRequest(ndmp_fh_add_node_request_v4 *req, NdmpGetFhAddNode *nodes)
{
    nodes->files_len = req->nodes.nodes_len;
    for (int i = 0; i < nodes->files_len; i++) {
        nodes->files[i].mtime = req->nodes.nodes_val[i].stats.stats_val[0].mtime;
        nodes->files[i].size = quadToLongLong(req->nodes.nodes_val[i].stats.stats_val[0].size);
        nodes->files[i].node = quadToLongLong(req->nodes.nodes_val[i].node);
        nodes->files[i].FhInfo = quadToLongLong(req->nodes.nodes_val[i].fh_info);
        nodes->files[i].ftype = req->nodes.nodes_val[i].stats.stats_val[0].ftype;
    }
}

void NdmpReleaseFhAddFilesReq(NdmpGetFhAddNode *nodeReq, unsigned int len)
{
    if (nodeReq == NULL) {
        return;
    }
 
    for (int i = 0; i < len; i++) {
        MODULE_FREE(nodeReq->files[i].path);
    }
    MODULE_FREE(nodeReq->files);
    MODULE_FREE(nodeReq);
}

int NdmpParseFhAddFileRequest(ndmp_fh_add_file_request_v4 *req, NdmpGetFhAddNode *nodes)
{
    unsigned int sucNum = 0;
    nodes->files_len = req->files.files_len;
    for (int i = 0; i < nodes->files_len; i++) {
        nodes->files[i].mtime = req->files.files_val[i].stats.stats_val[0].mtime;
        nodes->files[i].size = quadToLongLong(req->files.files_val[i].stats.stats_val[0].size);
        nodes->files[i].node = quadToLongLong(req->files.files_val[i].node);
        nodes->files[i].FhInfo = quadToLongLong(req->files.files_val[i].fh_info);
        nodes->files[i].ftype = req->files.files_val[i].stats.stats_val[0].ftype;
        int pathLen = strlen(req->files.files_val[i].names.names_val[0].ndmp_file_name_v3_u.unix_name);
        nodes->files[i].pathLen = pathLen;
        nodes->files[i].path = (char *)malloc(pathLen + 1);
        if (nodes->files[i].path == NULL) {
            ERRLOG("malloc faild!");
            NdmpReleaseFhAddFilesReq(nodes, sucNum);
            return -1;
        }
        ++sucNum;
        if (memcpy_s(nodes->files[i].path, pathLen + 1, req->files.files_val[i].names.names_val[0].ndmp_file_name_v3_u.unix_name,
            pathLen + 1) != 0) {
            ERRLOG("memcpy faild!");
            NdmpReleaseFhAddFilesReq(nodes, sucNum);
            return -1;
        }
    }

    return 0;
}
 
int NdmpProcessFhAddNode(void *body)
{
	int rc = 0;
    ndmp_fh_add_node_request_v4 *req = (ndmp_fh_add_node_request_v4 *)body;
    NdmpGetFhAddNode *nodes = (NdmpGetFhAddNode *)malloc(sizeof(NdmpGetFhAddNode));
    if (nodes == NULL) {
        ERRLOG("malloc failed");
        return -1;
    }

    nodes->files = (NdmpFileStats *)malloc(sizeof(NdmpFileStats) * req->nodes.nodes_len);
    if (nodes->files == NULL) {
        MODULE_FREE(nodes);
        ERRLOG("malloc failed");
        return -1;
    } 
    NdmpParseFhAddNodeRequest(req, nodes);

    rc = NdmpWriteTmpFhAddNodeFile(nodes);   

    NdmpReleaseFhAddNodeReq(nodes);
    return rc;
}

int NdmpProcessFhAddFile(void *body)
{
	int rc = 0;
    ndmp_fh_add_file_request_v4 *req = (ndmp_fh_add_file_request_v4 *)body;
    NdmpGetFhAddNode *nodes = (NdmpGetFhAddNode *)malloc(sizeof(NdmpGetFhAddNode));
    if (nodes == NULL) {
        ERRLOG("malloc failed");
        return -1;
    }

    nodes->files = (NdmpFileStats *)malloc(sizeof(NdmpFileStats) * req->files.files_len);
    if (nodes->files == NULL) {
        MODULE_FREE(nodes);
        ERRLOG("malloc failed");
        return -1;
    } 
    rc = NdmpParseFhAddFileRequest(req, nodes);
    CHECK_RESULT_RETURN(rc, 0, "Parse fh add file request failed!");

    rc = NdmpWriteTmpFhAddFiles(nodes);

    NdmpReleaseFhAddFilesReq(nodes, nodes->files_len);
    return rc;

}

int process_requests()
{
    fd_set conn_fds;
    int got_data_halted = 0;
    MsgData msg;
    struct timeval timeout;
    int rc = 0;

    while (1) {
        /* work-around NetApp bug id 7134 so we don't wait forever,
         * three minutes should be long enough in most cases... */
        (void) pthread_mutex_lock(&g_dataMutex);
        got_data_halted = (int)g_ndmpStatus;
        (void) pthread_mutex_unlock(&g_dataMutex);

        if (got_data_halted == NDMP_STATUS_COMPLETED) {
    		ndmp_get_connect_stat(&g_srcConnection, true);
            set_ndmp_status(NDMP_STATUS_FINISHED);
            INFOLOG("Mover and data server halted, return success.");
            return 0;
        }

        if (got_data_halted != NDMP_STATUS_PROCESSING) {
            ERRLOG("Data server never halted, closing connection.");
            return -1;
        }
        /* reinitialize timeout each time it is used because Linux modifies
         * timeout to reflect the amount of time not slept */
        timeout.tv_sec = 1L;
        timeout.tv_usec = 0L;

        (void) pthread_mutex_lock(&g_connectionMutex);

        FD_ZERO(&conn_fds);
        FD_SET(ndmpGetFd(g_srcConnection), &conn_fds);

        if (select(MAX(ndmpGetFd(g_srcConnection), ndmpGetFd(g_srcConnection)) + 1,
                   &conn_fds, NULL, NULL, &timeout) < 0) {
            ERRLOG("Process_requests: error selecting on connection sockets: %s.", strerror(errno));
			(void) pthread_mutex_unlock(&g_connectionMutex);
            set_ndmp_status(NDMP_STATUS_INTERNAL_ERROR);
            return -1;
        }

        if (FD_ISSET(ndmpGetFd(g_srcConnection), &conn_fds)) {
            if (ndmpPoll(g_srcConnection, FALSE) < 0) {
                ERRLOG("Error processing NDMP requests.");
                (void) pthread_mutex_unlock(&g_connectionMutex);
                set_ndmp_status(NDMP_STATUS_INTERNAL_ERROR);
                return -1;
            }
        }

        (void) pthread_mutex_unlock(&g_connectionMutex);
    }
}

void* ndmp_run_backup(void* arg)
{
    INFOLOG("enter ndmp_run_backup");
    pthread_t dequeThread;
    int rc = 0;
    rc = process_requests();
    
    INFOLOG("process_requests done, rc is %d\n", rc);

    if (rc != 0){
        ERRLOG("error during process_requests, send abort to servers");
        ndmp_abort_backup();
    }

    ndmp_end_backup();
    close_thread(&dequeThread);
    return NULL;
}

void initNdmpBackupDataParams(NdmpClientInterface *interface)
{
    g_dataConnection.backFilePath = interface->backFilePath;
    g_dataConnection.fspath = interface->srcPath;
    g_dataConnection.level = interface->level[0] - '0';
}

bool IsNdmpServerNeedPreparedSetting()
{
    // 获取生产端产品型号
    ndmp_config_get_host_info_reply_v4 *hostReply = NULL;
    int rc = ndmpSendRequest(g_srcConnection, NDMP_CONFIG_GET_HOST_INFO, NDMP_NO_ERR,
                NULL, (void **)&hostReply);
    if (rc == 0) {
        WARNLOG("NDMP_CONFIG_GET_HOST_INFO request send, osType:%s!", hostReply->os_type);
        if (strstr(hostReply->os_type, UNITY_OS) != NULL) { // 存储厂商为unity，不支持设置此扩展属性
            return false;
        }
    }

    return true;
}

void ndmpBackupPrepareRequestSend(NdmpClientInterface *interface)
{
    initSrcBackup(interface);
    if (!IsNdmpServerNeedPreparedSetting()) {
        return;
    }

    ndmp_data_start_backup_request_v4 prepareReq;
    ndmp_data_start_backup_reply_v4 *prepareReply = NULL;
    prepareReq.env.env_val = g_environment;
    prepareReq.env.env_len = 8;
    prepareReq.bu_type = "dump";
    ndmpSendRequest(g_srcConnection, NDMP_CAB_DATA_CONN_PREPARE, NDMP_NO_ERR,
                (void*)&prepareReq, (void**)&prepareReply);
}

int NdmpStartAsyncProcess()
{
    int rc = 0;

    rc = pthread_create(&g_getStatPt, NULL, get_backup_stat, NULL);
    if (rc != 0) {
        ERRLOG("Create get stat thread failed!");
        return NDMP_START_BACKUP_INNER_ERROR;
    }

    rc = pthread_create(&g_runBackupPt, NULL, ndmp_run_backup, NULL);
    if (rc != 0) {
        ERRLOG("Create run backup thread failed!");
        ndmp_end_backup();
        return NDMP_START_BACKUP_INNER_ERROR;
    }

    return rc;
}

int NdmpStartBackup(NdmpClientInterface *interface)
{
    set_ndmp_status(NDMP_STATUS_INIT);
    time_t	starttime, endtime, difftime;
    int rc = 0;
    uint32_t isDataListen = 0;
    rc = check_backup_params(interface);
    if (rc != 0){
        ERRLOG("error invalid backup params");
        return NDMP_START_BACKUP_INNER_ERROR;
    }

    // open ndmp connection & client auth
    rc = NdmpClientAuth(interface);
    if (rc < 0){
        ERRLOG("error auth before start backup");
        return rc;
    }
    INFOLOG("ndmp auth success!");

    ndmpBackupPrepareRequestSend(interface);

    rc = ndmp_data_connect(interface, &isDataListen);
    if (rc != 0) {
        ERRLOG("error connect data");
        return NDMP_DATA_CONNECT_ERROR;
    }

    rc = ndmp_src_backup(interface);
    if (rc != 0) {
        ERRLOG("error src backup");
        rc = NDMP_SRC_BACKUP_ERROR;
        goto ERR;
    }

    set_ndmp_status(NDMP_STATUS_PROCESSING);

    initNdmpBackupDataParams(interface);
    rc = pthread_create(&g_getStatPt, NULL, NdmpStartDataReceive, NULL);
    if (rc != 0) {
        ERRLOG("create backfile failed, rc(%d)!", rc);
        ndmp_abort_backup();
        rc = NDMP_START_BACKUP_INNER_ERROR;  
        goto ERR;
    }

    rc = NdmpStartAsyncProcess();
    if (rc != 0) {
        goto ERR;
    }

    return rc;
ERR:
    MODULE_FREE(g_backupFilePath);
    return rc;
}

void initNdmpRestoreDataParams(NdmpClientInterface *interface)
{
    g_dataConnection.backFilePath = interface->backFilePath;
    g_dataConnection.fspath = interface->srcPath;
}

void ndmpRestorePrepareRequestSend(NdmpClientInterface *interface)
{
    initNdmpDstRestore(interface);
    if (!IsNdmpServerNeedPreparedSetting()) {
        return;
    }
    ndmp_data_start_backup_request_v4 prepareReq;
    ndmp_data_start_backup_reply_v4 *prepareReply = NULL;
    g_environment[5].name = "FILESYSTEM";
    g_environment[5].value = interface->dstPath;
    g_environment[6].name = "PREFIX";
    g_environment[6].value = interface->dstPath;
    prepareReq.env.env_val = g_environment;
    prepareReq.env.env_len = 7;
    prepareReq.bu_type = "dump";
    if (!IsNdmpServerNeedPreparedSetting()) {
        return;
    }
    ndmpSendRequest(g_srcConnection, NDMP_CAB_DATA_CONN_PREPARE, NDMP_NO_ERR,
                (void*)&prepareReq, (void**)&prepareReply);
}

int NdmpStartRestore(NdmpClientInterface *interface)
{
    INFOLOG("srcip(%s), dstip(%s), level(%s), srcPath(%s), dstPath(%s), backFilePath(%s)",interface->srcIp, interface->dstIp, interface->level,
        interface->srcPath, interface->dstPath, interface->backFilePath);
    set_ndmp_status(NDMP_STATUS_INIT);
    time_t			starttime, endtime, difftime;
    int rc = 0;
    uint32_t isDataListen = 0;
    rc = check_backup_params(interface);
    if (rc != 0){
        ERRLOG("error invalid restore params");
        return NDMP_START_RESTORE_INNER_ERROR;
    }

    // open ndmp connection & client auth
    rc = NdmpClientAuth(interface);
    if (rc < 0){
        ERRLOG("error auth before start restore");
        return NDMP_AUTH_ERROR_DST;
    }
    INFOLOG("ndmp auth success!");

    ndmpRestorePrepareRequestSend(interface);

    rc = ndmp_data_connect(interface, &isDataListen);
    if (rc != 0) {
        ERRLOG("error connect data");
        rc = NDMP_DATA_CONNECT_ERROR;
        goto ERR;
    }

    rc = ndmp_dst_restore(interface);
    if (rc != 0) {
        ERRLOG("error src backup");
        rc = NDMP_SRC_BACKUP_ERROR;
        goto ERR;
    }

    set_ndmp_status(NDMP_STATUS_PROCESSING);

    initNdmpRestoreDataParams(interface);
    rc = pthread_create(&g_runBackupPt, NULL, NdmpStartRestoreData, NULL);
    if (rc != 0) {
        ERRLOG("Create get stat thread failed, rc(%d)!", rc);
        rc = NDMP_START_BACKUP_INNER_ERROR;
        goto ERR;
    }

    rc = NdmpStartAsyncProcess();
    if (rc != 0) {
        goto ERR;
    }

    return rc;
ERR:
    MODULE_FREE(g_backupFilePath);
    return rc;
}

NdmpStat NdmpGetStatFunc()
{
    NdmpStat stat;
    (void) pthread_mutex_lock(&g_dataMutex);
    stat.status = (int)g_ndmpStatus;
    stat.process_bytes = g_srcProcessBytes;
    stat.remain_bytes = g_srcRemainingSize;
    stat.files_cnt = g_backupFilesCnt;
    (void) pthread_mutex_unlock(&g_dataMutex);
    return stat;
}

int NdmpAbortBackup(NdmpClientInterface *interface)
{
    ndmp_abort_backup();

    ndmp_end_backup();

    return 0;
}

void NdmpProcessInterrupt()
{
    ndmp_abort_backup();

    ndmp_end_backup();    
}

void NdmpDestroy()
{
    g_isSendAbort = false;

    close_thread(&g_getStatPt);
    close_thread(&g_runBackupPt);

    return;
}
