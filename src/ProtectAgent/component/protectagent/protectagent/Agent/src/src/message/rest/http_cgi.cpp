/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TCPClientHandler.cpp
 * @brief  The implemention about fast cgi
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "message/rest/http_cgi.h"
#include "common/Log.h"
#include "message/rest/interfaces.h"
using namespace std;

CHttpRequest::CHttpRequest(FCGX_Request& pFcgiReq) : m_pFcgRequest(&pFcgiReq)
{
    // 获取URL和查询参数
    const mp_char* uri = FCGX_GetParam(REQUEST_URI.c_str(), m_pFcgRequest->envp);
    if (NULL == uri) {
        m_strURL = UNKNOWN;
    } else {
        mp_string strCompleteURL = uri;
        mp_string::size_type pos = strCompleteURL.find("?");
        if (pos != mp_string::npos) {
            m_strURL = strCompleteURL.substr(0, pos);
            m_strQueryParam = strCompleteURL.substr(pos + 1);
        } else {
            m_strURL = std::move(strCompleteURL);
            m_strQueryParam = "";
        }
    }

    // 获取method
    const mp_char* method = FCGX_GetParam(REQUEST_METHOD.c_str(), m_pFcgRequest->envp);
    m_strMethod = ((method == NULL) ? UNKNOWN : method);
}

/* ------------------------------------------------------------
Function Name: getHead
Description  : 根据HTTP消息头名称获取其内容
Others       :------------------------------------------------------------- */
mp_string CHttpRequest::GetHead(const mp_string& name)
{
    const mp_char* head = FCGX_GetParam(name.c_str(), m_pFcgRequest->envp);
    return (head == NULL) ? UNKNOWN : mp_string(head);
}

/* ------------------------------------------------------------
Function Name: getHead
Description  : 根据HTTP消息头名称获取其内容
Others       :------------------------------------------------------------- */
mp_string CHttpRequest::GetHeadNoCheck(const mp_string& name)
{
    const char* head = FCGX_GetParam(name.c_str(), m_pFcgRequest->envp);
    if (head) {
        return mp_string(head);
    } else {
        return mp_string("");
    }
}

/* ------------------------------------------------------------
Function Name: getAllHead
Description  : 根据所有HTTP消息头
Others       :------------------------------------------------------------- */
mp_char** CHttpRequest::GetAllHead()
{
    return m_pFcgRequest->envp;
}

/* ------------------------------------------------------------
Function Name: getRemoteIP
Description  : 获取远端IP地址
Others       :------------------------------------------------------------- */
mp_string CHttpRequest::GetRemoteIP()
{
    const mp_char* remoteIP = FCGX_GetParam(REMOTE_ADDR.c_str(), m_pFcgRequest->envp);
    return (remoteIP == NULL) ? UNKNOWN : mp_string(remoteIP);
}

/* ------------------------------------------------------------
Function Name: getContentLen
Description  : 获取http请求中的内容的长度
Others       :------------------------------------------------------------- */
mp_uint32 CHttpRequest::GetContentLen()
{
    return (mp_uint32)atoi(GetHead(CONTENT_LENGTH).c_str());
}

/* ------------------------------------------------------------
Function Name: getContentType
Description  : 获取http请求中的类型
Others       :------------------------------------------------------------- */
mp_bool CHttpRequest::GetContentType(mp_string& type)
{
    type = GetHead(CONTENT_TYPE);
    return MP_TRUE;
}

/* ------------------------------------------------------------
Function Name: getFcgxReq
Description  : 获取FCGX_Request的成员
Others       :------------------------------------------------------------- */
FCGX_Request* CHttpRequest::GetFcgxReq()
{
    return m_pFcgRequest;
}

mp_int32 CHttpRequest::ReadChar()
{
    return FCGX_GetChar(m_pFcgRequest->in);
}

mp_int32 CHttpRequest::ReadStr(mp_char* b, mp_int32 l)
{
    return FCGX_GetStr(b, l, m_pFcgRequest->in);
}

mp_string CHttpRequest::Readline(mp_char* b, mp_int32 l)
{
    const mp_char* tmp = FCGX_GetLine(b, l, m_pFcgRequest->in);
    return (tmp == NULL) ? mp_string("") : mp_string(tmp);
}

CHttpResponse::CHttpResponse(FCGX_Request& pRequest) : m_pFcgRequest(&pRequest)
{
    SetHead(CACHE_CONTROL, "no-cache");
}

mp_string CHttpRequest::GetClientCertDN()
{
    const mp_char* clientCertDN = FCGX_GetParam(CLIENT_CERT_DN.c_str(), m_pFcgRequest->envp);
    return (NULL == clientCertDN) ? UNKNOWN : mp_string(clientCertDN);
}

/* ------------------------------------------------------------
Function Name: setContentType
Description  : 设置http请求中的Content-Type字段值
Others       :------------------------------------------------------------- */
void CHttpResponse::SetContentType(const mp_string& type)
{
    m_Heads[CONTENT_TYPE] = type;
}

/* ------------------------------------------------------------
Function Name: getHead
Description  : 根据名称获取相应消息头信息的值
Others       :------------------------------------------------------------- */
mp_string CHttpResponse::GetHead(const mp_string& name)
{
    map<mp_string, mp_string>::iterator it = m_Heads.find(name);
    if (it != m_Heads.end()) {
        return it->second;
    }
    return "";
}

/* ------------------------------------------------------------
Function Name: setHead
Description  : 设置响应消息头字段值
Others       :------------------------------------------------------------- */
void CHttpResponse::SetHead(const mp_string& name, const mp_string& value)
{
    m_Heads[name] = value;
}

/* ------------------------------------------------------------
Function Name: removeHead
Description  : 根据名称移除响应消息头中的对应字段
Others       :------------------------------------------------------------- */
void CHttpResponse::RemoveHead(const mp_string& name)
{
    m_Heads.erase(name);
}

/* ----------------------------------------------------------------------
 *
 * writeChar
 *
 *      Writes a byte to the output stream.
 *
 * Results: *	    The byte, or EOF (-1) if an error occurred.
            *
 ----------------------------------------------------------------------
 */
mp_int32 CHttpResponse::WriteChar(mp_int32 c)
{
    SendHeads();
    return FCGX_PutChar(c, m_pFcgRequest->out);
}

/* ----------------------------------------------------------------------
 *
 * writeStr
 *
 *      Writes n consecutive bytes from the character array str
 *      into the output stream.  Performs no interpretation
 *      of the output bytes.
 *
 * Results: *      Number of bytes written (n) for normal return,
            *      EOF (-1) if an error occurred.
            *
 ----------------------------------------------------------------------
 */
mp_int32 CHttpResponse::WriteStr(const mp_string& str, const mp_int32& n)
{
    SendHeads();
    return FCGX_PutStr(str.c_str(), n, m_pFcgRequest->out);
}

/* ----------------------------------------------------------------------
 *
 * writeS
 *
 *      Writes a null-terminated character string to the output stream.
 *
 * Results: *      number of bytes written for normal return,
            *      EOF (-1) if an error occurred.
            *
 ----------------------------------------------------------------------
 */
mp_int32 CHttpResponse::WriteS(const mp_string& str)
{
    SendHeads();
    return FCGX_PutS(str.c_str(), m_pFcgRequest->out);
}

/* ------------------------------------------------------------
Function Name: sendHeads
Description  : 将消息头写入FCGI响应流中。
Others       :------------------------------------------------------------- */
mp_void CHttpResponse::SendHeads()
{
    if (!m_Heads.empty()) {
        for (map<mp_string, mp_string>::iterator it = m_Heads.begin(); it != m_Heads.end(); ++it) {
            COMMLOG(OS_LOG_DEBUG, "[Http Head Sent]%s=%s.", it->first.c_str(), it->second.c_str());
            if (it->first != CONTENT_TYPE) {
                FCGX_FPrintF(m_pFcgRequest->out, "%s: %s\r\n", it->first.c_str(), it->second.c_str());
            }
        }

        m_Heads.clear();
        FCGX_FPrintF(m_pFcgRequest->out, "\r\n");
    }
}

/* ------------------------------------------------------------
Function Name: complete
Description  : 设置fcgi结束标志。
Others       :------------------------------------------------------------- */
mp_void CHttpResponse::Complete()
{
    FCGX_Finish_r(m_pFcgRequest);
}
