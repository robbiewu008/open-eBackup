#ifndef _AGENT_MESSAGE_PROCESS_H_
#define _AGENT_MESSAGE_PROCESS_H_

#include <memory>
#include <map>
#include <functional>
#include "jsoncpp/include/json/value.h"
#include "jsoncpp/include/json/json.h"

#include "common/Types.h"
#include "common/MpString.h"
#include "common/JsonUtils.h"
#include "message/Message.h"
#include "message/rest/http_cgi.h"
#include "message/rest/http_status.h"

enum BODY_ACTION_TYPE { BODY_DECODE_JSON = 0, BODY_RAW_BUF = 1 };

static const mp_string REST_TAG = "/agent/";
static const mp_string REST_VERSION = "/v";
static const mp_int32 MSG_BLK_SIZE = 2048;
// 1 * 1024 * 1024
static const mp_int32 MAX_MSG_CACHE_SIZE = 3 * 1048576;   // 3M
static const mp_int32 ONCE_SEND_MSG_KB = 32;
// 2 * 1024 * 1024 * 1024
static const mp_uint64 MAX_ATTCHMENT_SIZE = 2147483648;
// 32 * 1024
static const mp_int32 ONCE_SEND_MSG_SIZE = 32768;

#define BOUNDARY_TAG "multipart/form-data; boundary="
static const mp_string CTN_DISP = "Content-Disposition";
static const mp_string CTN_TYPE = "Content-Type";
#define CTN_DISP_NAME "Content-Disposition: form-data; name=\""
#define CTN_TYPE_COMMA "Content-Type: "
#define FILE_NAME "; filename=\""

// 内部消息缓存块，功能类似于ACE_Message_Block
class CMessage_Block {
public:
    CMessage_Block();

    CMessage_Block(mp_uint32 n);

    ~CMessage_Block();

    CMessage_Block(CMessage_Block& msg_blk)
    {}

    CMessage_Block& operator=(CMessage_Block& msg_blk)
    {
        return *this;
    }

    /// Get the write I/O buffer pointer.
    mp_char* GetWritePtr();

    // Get the read I/O buffer pointer.
    mp_char* GetReadPtr();

    /// Set the I/O buffer ahead @a n bytes.  This is used to compute
    /// the <length> of a message.
    mp_void AddLength(mp_uint32 n);

    /// Get the length of the message
    mp_uint32 GetLength() const;

    /// Get the number of bytes in the top-level Message_Block (i.e.,
    /// does not consider the bytes in chained Message_Blocks).
    mp_uint32 GetSize() const;

    /**
     * Set the number of bytes in the top-level Message_Block,
     * reallocating space if necessary.  However, the @c rd_ptr_ and
     * @c wr_ptr_ remain at the original offsets into the buffer, even if
     * it is reallocated.  Returns 0 if successful, else -1.
     */
    mp_int32 Resize(mp_uint32 length);

private:
    mp_char* m_data_block;
    mp_char* m_wirte_ptr;
    mp_uint32 m_size;
    mp_uint32 m_length;

    static const mp_uchar MESSAGEPROC_NUM_2   = 2;
    static const mp_uchar MESSAGEPROC_NUM_4   = 4;
    static const mp_uchar MESSAGEPROC_NUM_10  = 10;
    static const mp_uchar MESSAGEPROC_NUM_16  = 16;
};

// HTTP请求中URL相关的封装
class CRequestURL {
public:
    CRequestURL() : m_version("")
    {}
    ~CRequestURL()
    {}
    CRequestURL(CRequestURL& reqUrl)
    {
        m_procURL = reqUrl.m_procURL;
        m_oriURL = reqUrl.m_oriURL;
        m_queryParam = reqUrl.m_queryParam;
        m_id = reqUrl.m_id;
        m_version = reqUrl.m_version;
    }
    CRequestURL& operator=(CRequestURL& reqUrl)
    {
        m_procURL = reqUrl.m_procURL;
        m_oriURL = reqUrl.m_oriURL;
        m_queryParam = reqUrl.m_queryParam;
        m_id = reqUrl.m_id;
        return *this;
    }
    mp_string GetProcURL()
    {
        return m_procURL;
    }
    mp_string GetOriURL()
    {
        return m_oriURL;
    }
    mp_string GetVersion()
    {
        return m_version;
    }
    mp_string GetServiceName();
    mp_string GetCutURL(mp_int32 index);
    mp_string GetID()
    {
        return m_id;
    }

    mp_void SetProcURL(const mp_string& strProcURL)
    {
        m_procURL = strProcURL;
    }
    mp_void SetOriURL(mp_string strOriURL)
    {
        m_oriURL = CMpString::ToLower(strOriURL);
        ParseURL();
    }
    mp_void SetQueryParam(const mp_string& strQueryParam, mp_bool bUTF8ToANSI = MP_TRUE);
    std::map<mp_string, mp_string>& GetQueryParam()
    {
        return m_queryParam;
    }
    mp_string GetSpecialQueryParam(const mp_string& strKey)
    {
        return (m_queryParam.find(strKey) != m_queryParam.end()) ? m_queryParam[strKey] : "";
    }

private:
    mp_string m_procURL;                     // 经过处理后的URL，和各app注册的消息函数对应
    mp_string m_oriURL;                      // 原始URL
    mp_string m_id;                          // 预留，如果原始URL中包含id时使用此成员
    mp_string m_version;                     // http版本号
    std::map<mp_string, mp_string> m_queryParam;  // URL中的查询参数
    mp_void ParseURL();
};

// HTTP请求中消息体的封装，支持json和文件传输
class CRequestMsgBody {
public:
    CRequestMsgBody() : m_msgLen(0), m_msgBodyType(BODY_DECODE_JSON)
    {}

    ~CRequestMsgBody()
    {}

    CRequestMsgBody(CRequestMsgBody& msgBody)
    {
        m_msgBodyType = msgBody.m_msgBodyType;
        m_msgLen = msgBody.m_msgLen;
        m_msgJsonData = msgBody.m_msgJsonData;
        m_raw_msg = msgBody.m_raw_msg;
    }

    CRequestMsgBody& operator=(CRequestMsgBody& msgBody)
    {
        m_msgBodyType = msgBody.m_msgBodyType;
        m_msgLen = msgBody.m_msgLen;
        m_msgJsonData = msgBody.m_msgJsonData;
        m_raw_msg = msgBody.m_raw_msg;
        return *this;
    }

    mp_int32 ReadWait(CHttpRequest& req);
    mp_int32 GetValue(const mp_string& name, mp_string& val);
    mp_int32 GetValueThrow(const mp_string& name, mp_string& val);
    mp_string JsonValueToString(const Json::Value& v);
    const Json::Value& GetJsonValueRef() const
    {
        return m_msgJsonData;
    }
    const Json::Value GetJsonValue() const
    {
        return m_msgJsonData;
    }
    mp_void SetJsonValue(const Json::Value& JsonValue)
    {
        m_msgJsonData = JsonValue;
    }
    mp_bool GetOriMsg(mp_char& buf, mp_uint32& len);
    mp_void SetMsgBodyType(BODY_ACTION_TYPE bodyType)
    {
        m_msgBodyType = bodyType;
    }

public:
    Json::Value m_msgJsonData;  // 存放解析好的json对象

private:
    BODY_ACTION_TYPE m_msgBodyType;  // 消息体类型
    mp_uint32 m_msgLen;              // 消息长度
    // static unsigned long m_totalCacheBodySize;  //内存中整体分配的长度
    std::auto_ptr<CMessage_Block> m_raw_msg;  // http请求消息中的原始类容

private:
    mp_int32 ReadData(CHttpRequest& req, CMessage_Block& msg);
    mp_int32 ParseJson(CMessage_Block& msg);
};

// 请求消息体的封装，用于各app得到http请求消息的载体
class CRequestMsg : public CBasicReqMsg {
public:
    CRequestMsg(FCGX_Request& pFcgiReq);
    CRequestMsg()
    {
        m_iType = REQMESSAGE_TYPE;
    }

    CRequestMsg(CRequestMsg& rsqMsg)
    {
        m_iType = REQMESSAGE_TYPE;
        m_url = rsqMsg.m_url;
        m_msgBody = rsqMsg.m_msgBody;
        m_httpReq = rsqMsg.m_httpReq;
    }

    CRequestMsg& operator=(CRequestMsg& rsqMsg)
    {
        m_iType = REQMESSAGE_TYPE;
        m_url = rsqMsg.m_url;
        m_msgBody = rsqMsg.m_msgBody;
        m_httpReq = rsqMsg.m_httpReq;
        return *this;
    }

    ~CRequestMsg()
    {}

    CRequestURL& GetURL()
    {
        return m_url;
    }

    CRequestMsgBody& GetMsgBody()
    {
        return m_msgBody;
    }

    CHttpRequest& GetHttpReq()
    {
        return m_httpReq;
    }

    mp_void SetProcURL(const mp_string& strRUL)
    {
        m_url.SetProcURL(strRUL);
    }

    mp_void SetJsonData(const Json::Value& jsonValue)
    {
        m_msgBody.SetJsonValue(jsonValue);
    }

    mp_int32 Parse();

public:
    CRequestURL m_url;          // URL对象
    CRequestMsgBody m_msgBody;  // 消息体对象
    CHttpRequest m_httpReq;     // http请求对象
};

// 响应消息体的封装，用于各app响应http请求消息的载体
class CResponseMsg : public CBasicRspMsg {
public:
    CResponseMsg(FCGX_Request& pFcgiReq) : m_httpRsp(pFcgiReq)
    {
        m_httpType = RSP_JSON_TYPE;
        m_iHttpStatus = SC_OK;
        m_lRetCode = MP_SUCCESS;
        m_bInternalMsg = MP_FALSE;
    }

    CResponseMsg()
    {
        m_httpType = RSP_JSON_TYPE;
        m_iHttpStatus = SC_OK;
        m_bInternalMsg = MP_TRUE;
        m_lRetCode = MP_SUCCESS;
    }

    ~CResponseMsg()
    {}

    enum enRspType {
        RSP_JSON_TYPE = 0,   // Json格式返回消息 ex: { "errorCode" : "string", "errorMessage" : json }
        RSP_JSON_TYPE2,         // Json格式返回消息 ex: { "errorCode" : "string", "errorMessage" : "string" }
        RSP_ATTACHMENT_TYPE  // 附件返回消息
    };

    mp_bool IsInternalMsg()
    {
        return m_bInternalMsg;
    }

    enRspType GetHttpType()
    {
        return m_httpType;
    }

    mp_void SetHttpType(enRspType httpType)
    {
        m_httpType = httpType;
    }

    mp_void SetHttpStatus(mp_int32 iStatus)
    {
        m_iHttpStatus = iStatus;
    }

    Json::Value& GetJsonValueRef()
    {
        return m_msgJsonData;
    }

    Json::Value GetJsonValue()
    {
        return m_msgJsonData;
    }

    mp_int32 Send();

public:
    Json::Value m_msgJsonData;  // 存放响应的json对象

private:
    enRspType m_httpType;  // http请求类型
    mp_int32 m_iHttpStatus;
    CHttpResponse m_httpRsp;  // http响应对象
    mp_bool m_bInternalMsg;   // 是否是内部处理消息，默认是false，在处理内部命令时将其设置为true
    static const mp_uchar MESSAGERES_NUM_10  = 10;

private:
    mp_void PackageReponse(Json::Value& root);
    mp_int32 SendJson();
    mp_int32 SendAttchment();
    mp_void SetHeader(const mp_string& strAttachFileName);
    mp_void SendFailedRsp();
};

// URL编解码相关的工具类
static const mp_int32 RFC_LENGTH = 256;
class CUrlUtils {
public:
    static mp_void Initrfc3986tb();
    static mp_int32 Urldecode2(mp_char indec[], const mp_char s[], mp_int32 len);
    static mp_string GetBetweenValue(const mp_char eqp[], const mp_char p[]);

private:
    static mp_int32 IsHex(mp_int32 x)
    {
        return (x >= '0' && x <= '9') || (x >= 'a' && x <= 'f') || (x >= 'A' && x <= 'F');
    }
    static mp_void Initrfc3986tb(mp_char rfc[], mp_int32 len)
    {
        for (mp_int32 i = 0; i < len; i++) {
            rfc[i] = (isalnum(i) || i == '~' || i == '-' || i == '.' || i == '_') ? i : 0;
        }
    }
    static mp_bool GetNum(const mp_char s[], mp_uint32 len, mp_uint32 b, mp_uint64& ret);

private:
    static const mp_uchar URLUTILS_NUM_2   = 2;
    static const mp_uchar URLUTILS_NUM_4   = 4;
    static const mp_uchar URLUTILS_NUM_16  = 16;
};

#endif  // _AGENT_MESSAGE_PROCESS_H_
