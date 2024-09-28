#include "dataprocess/datareadwrite/SocketStream.h"
#include "dataprocess/datareadwrite/DataContext.h"
#include "dataprocess/datareadwrite/DataStream.h"
#include "message/tcp/CSocket.h"
#include "common/TimeOut.h"
#include "common/Utils.h"
#include "common/Types.h"
#include "common/Log.h"
#include "common/Defines.h"

SocketStream::SocketStream() {}

SocketStream::~SocketStream() {}

mp_int32 SocketStream::StreamWrite(mp_void *ctx, mp_char *buff, mp_int32 iBuffLen)
{
    mp_int32 uiSendLen = 0;
    mp_socket sock = 0;

    DataContext *ctxdata = static_cast<DataContext *>(ctx);
    if (ctxdata == nullptr) {
        COMMLOG(OS_LOG_ERROR, "data context is null.");
        return MP_FAILED;
    }
    sock = ctxdata->GetSockFd();

    uiSendLen = CSocket::SendBuffer(sock, buff, iBuffLen, TIMEOUT_INFINITE);
    if (iBuffLen == uiSendLen) {
        COMMLOG(OS_LOG_DEBUG, "Send messge to sock %d, len=%d.", sock, uiSendLen);
        return MP_SUCCESS;
    }
    COMMLOG(OS_LOG_ERROR, "Send msg failed, need send %u, send %u.", iBuffLen, uiSendLen);

    return MP_FAILED;
}

mp_int32 SocketStream::StreamRead(mp_void *ctx, mp_char *buff, mp_int32 iBuffLen, mp_uint32 &iRecvLen)
{
    // /1. check the data availability, if not available, return immediately
    // /2. receive a complete cmd not a partical cmd
    DataContext *ctxdata = static_cast<DataContext *>(ctx);
    if (ctxdata == nullptr) {
        COMMLOG(OS_LOG_ERROR, "data context is null.");
        return MP_FAILED;
    }
    mp_socket sock = 0;
    sock = ctxdata->GetSockFd();
    mp_int32 iRet = MP_SUCCESS;
    mp_uint32 uiRecvLen = 0;
    iRet = CSocket::RecvBuffer(sock, buff, iBuffLen, TIMEOUT_INFINITE, uiRecvLen);
    if (iRet == MP_FAILED) {
        COMMLOG(OS_LOG_ERROR, "Receive message failed, iRet %d", iRet);
        return (iRet);
    }

    return (iRet);
}
