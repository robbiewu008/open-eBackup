#ifndef _AGENT_HTTP_CGI_H_
#define _AGENT_HTTP_CGI_H_

#include <map>
#include <stdlib.h>

#include "common/Types.h"
#include "common/Defines.h"  // macro DLLAPI is defined in Defines.h
#include "fcgi/include/fcgiapp.h"


// #pragma warning( disable: 4251 )
class CHttpRequest {
public:
    CHttpRequest(FCGX_Request& request);
    CHttpRequest()
    {
        m_pFcgRequest = NULL;
    }

    CHttpRequest(CHttpRequest& httpReq)
    {
        m_strURL = httpReq.m_strURL;
        m_strQueryParam = httpReq.m_strQueryParam;
        m_strMethod = httpReq.m_strMethod;
        m_pFcgRequest = httpReq.m_pFcgRequest;
    }

    ~CHttpRequest() {}
    CHttpRequest& operator=(CHttpRequest& httpReq)
    {
        m_strURL = httpReq.m_strURL;
        m_strQueryParam = httpReq.m_strQueryParam;
        m_strMethod = httpReq.m_strMethod;
        m_pFcgRequest = httpReq.m_pFcgRequest;
        return *this;
    }

    mp_string GetURL()
    {
        return m_strURL;
    }
    mp_string GetQueryParamStr()
    {
        return m_strQueryParam;
    }

    mp_string GetHead(const mp_string& name);
    mp_string GetHeadNoCheck(const mp_string& name);
    mp_char** GetAllHead();
    FCGX_Request* GetFcgxReq();
    mp_string GetRemoteIP();
    mp_string GetMethod()
    {
        return m_strMethod;
    }
    void SetMethod(const std::string& strMethod)
    {
        m_strMethod = strMethod;
    }
    void SetAllHead(char** envp)
    {
        m_pFcgRequest->envp = envp;
    }
    void SetFcgxReq(FCGX_Request& pFcgxReq)
    {
        m_pFcgRequest = &pFcgxReq;
    }
    mp_uint32 GetContentLen();
    mp_bool GetContentType(std::string& type);
    mp_int32 ReadChar();
    mp_int32 ReadStr(char* b, mp_int32 l);
    mp_string Readline(char* b, mp_int32 l);
    mp_string GetClientCertDN();

private:
    mp_string m_strURL;         // URL字符串
    mp_string m_strQueryParam;  // 查询字符串
    mp_string m_strMethod;
    FCGX_Request* m_pFcgRequest;  // 分配req对象，解决fastcgi
};

class CHttpResponse {
public:
    CHttpResponse(FCGX_Request& request);
    CHttpResponse()
    {
        m_pFcgRequest = NULL;
    }
    ~CHttpResponse() {}
    void SetContentType(const mp_string& type);
    mp_string GetHead(const mp_string& name);
    void SetHead(const mp_string& name, const mp_string& value);
    void RemoveHead(const mp_string& name);
    mp_int32 WriteChar(mp_int32 c);
    mp_int32 WriteStr(const mp_string& str, const mp_int32& n);
    mp_int32 WriteS(const mp_string& str);
    void SendError(mp_int32);
    void Complete();

private:
    FCGX_Request* m_pFcgRequest;        // fastcgi请求对象
    std::map<mp_string, mp_string> m_Heads;  // 消息头对象
    void SendHeads();
};

#endif  // _AGENT_HTTP_CGI_H_
