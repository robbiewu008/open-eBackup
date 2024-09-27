/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file message_process.cpp
 * @brief  Contains function declarations message process
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "message/rest/message_process.h"

#include <sstream>
#include <fstream>

#include "common/Defines.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "message/rest/interfaces.h"

using namespace std;
namespace {
    constexpr int32_t MAX_LOG_LEN = 1024 * 1024;
}

/* ------------------------------------------------------------
Function Name: CMessage_Block
Description  : CMessage_Block构造函数
Others       :------------------------------------------------------------- */
CMessage_Block::CMessage_Block()
{
    m_wirte_ptr = NULL;
    m_data_block = NULL;
    m_length = 0;
    m_size = 0;
}

/* ------------------------------------------------------------
Function Name: CMessage_Block
Description  : CMessage_Block构造函数，n表示要分配的空间大小
Others       :------------------------------------------------------------- */
CMessage_Block::CMessage_Block(mp_uint32 n)
{
    // CodeDex误报，ZERO_LENGTH_ALLOCATIONS
    NEW_ARRAY_CATCH(m_data_block, mp_char, n);
    if (m_data_block == NULL) {
        m_size = 0;
    } else {
        m_size = n;
    }
    m_length = 0;
    m_wirte_ptr = m_data_block;
}

/* ------------------------------------------------------------
Function Name: getWritePtr
Description  : 获取写空间的地址
Others       :------------------------------------------------------------- */
mp_char* CMessage_Block::GetWritePtr()
{
    return m_wirte_ptr;
}

/* ------------------------------------------------------------
Function Name: getReadPtr
Description  : 获取读空间的地址
Others       :------------------------------------------------------------- */
mp_char* CMessage_Block::GetReadPtr()
{
    return m_data_block;
}

/* ------------------------------------------------------------
Function Name: addLength
Description  : 用于计算写空间的地址
Others       :------------------------------------------------------------- */
mp_void CMessage_Block::AddLength(mp_uint32 n)
{
    m_length += n;
    if (m_wirte_ptr) {
        m_wirte_ptr += n;
    }
}

/* ------------------------------------------------------------
Function Name: getLength
Description  : 获取已写入空间的长度
Others       :------------------------------------------------------------- */
mp_uint32 CMessage_Block::GetLength() const
{
    return m_length;
}

/* ------------------------------------------------------------
Function Name: getSize
Description  : 获取已分配空间的长度
Others       :------------------------------------------------------------- */
mp_uint32 CMessage_Block::GetSize() const
{
    return m_size;
}

/* ------------------------------------------------------------
Function Name: resize
Description  : 动态调整空间的大小
Others       :------------------------------------------------------------- */
mp_int32 CMessage_Block::Resize(mp_uint32 length)
{
    if (length <= m_size) {
        COMMLOG(OS_LOG_ERROR, "input length %d is smaller than size %d", length, m_size);
        return MP_FAILED;
    }

    mp_char* data_block = NULL;
    // CodeDex误报，Memory Leak
    NEW_ARRAY_CATCH(data_block, mp_char, length);
    if (!data_block) {
        COMMLOG(OS_LOG_ERROR, "data_block is 0.");
        return MP_FAILED;
    }
    mp_int32 iRet = memset_s(data_block, length, 0, length);
    if (iRet != EOK) {
        COMMLOG(OS_LOG_ERROR, "Call memset_s failed, ret %d.", iRet);
        delete[] data_block;
        return MP_FAILED;
    }

    if (!m_data_block) {
        COMMLOG(OS_LOG_ERROR, "m_data_block is 0.");
        delete[] data_block;
        return MP_FAILED;
    }
    iRet = memcpy_s(data_block, length, m_data_block, m_size);
    if (iRet != EOK) {
        COMMLOG(OS_LOG_ERROR, "Call memcpy_s failed, ret %d.", iRet);
        delete[] data_block;
        return MP_FAILED;
    }

    delete[] m_data_block;
    m_data_block = data_block;
    m_wirte_ptr = m_data_block + m_length;
    m_size = length;

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: ~CMessage_Block
Description  : 析构函数，用于释放分配的内存
Others       :------------------------------------------------------------- */
CMessage_Block::~CMessage_Block()
{
    delete[] m_data_block;
    m_data_block = NULL;
    m_wirte_ptr = NULL;
    m_size = 0;
    m_length = 0;
}

/* ------------------------------------------------------------
Function Name: ParseURL
Description  : 通过原始字符串生成各app能处理的字符串, support url format like /v1/agent and /agent
Others       :------------------------------------------------------------- */
mp_void CRequestURL::ParseURL()
{
    // 截取出各处理模块需要的URL字符串
    mp_string::size_type pos = m_oriURL.find(REST_TAG);
    if (pos == mp_string::npos) {
        COMMLOG(OS_LOG_INFO, "can not find \"%s\" in url \"%s\".", REST_TAG.c_str(), m_oriURL.c_str());
        m_procURL = "";
    } else {
        if (m_oriURL.find(REST_VERSION) == 0) {
            m_procURL = m_oriURL.substr(0, pos) + "/" + m_oriURL.substr(pos + strlen(REST_TAG.c_str()));
        } else {
            m_procURL = "/" + m_oriURL.substr(pos + strlen(REST_TAG.c_str()));
        }
    }
}

/* ------------------------------------------------------------
Function Name: SetQueryParam
Description  : 解析请求字符串为map,bUTF8ToANSI默认为false，不进行utf8到ansi转换
Others       :------------------------------------------------------------- */
mp_void CRequestURL::SetQueryParam(const mp_string& strQueryParam, mp_bool bUTF8ToANSI)
{
    const mp_char* nm = strQueryParam.c_str();
    const mp_char* eqp = 0;
    const mp_char* p = strQueryParam.c_str();
    for (; *p != 0; ++p) {
        if (*p == '=') {
            eqp = p;
        } else if (*p == '&') {
            if (eqp != NULL) {
                mp_string name(nm, eqp - nm);
                m_queryParam[name] = CUrlUtils::GetBetweenValue(eqp, p);
            }
            nm = p + 1;
            eqp = NULL;
        }
    }

    if (eqp != NULL) {
        mp_string name(nm, eqp - nm);
        m_queryParam[name] = CUrlUtils::GetBetweenValue(eqp, p);
    }
    for (map<mp_string, mp_string>::iterator it = m_queryParam.begin(); it != m_queryParam.end(); ++it) {
#ifdef WIN32
        if (bUTF8ToANSI) {
            it->second = CMpString::UTF8ToANSI(it->second);
        }
#else
        (mp_void)bUTF8ToANSI;
#endif
        COMMLOG(OS_LOG_DEBUG, "Write query param, %s = %s.", it->first.c_str(), it->second.c_str());
    }
}

/* ------------------------------------------------------------
Function Name: GetServiceName
Description  : 通过原始字符串获取服务模块名称，如DB2, Oracle
Others       :------------------------------------------------------------- */
mp_string CRequestURL::GetServiceName()
{
    // 截取出各处理模块需要的URL字符串
    if (m_procURL.empty()) {
        COMMLOG(OS_LOG_INFO, "m_procURL is empty");
        return "";
    }

    mp_string strTmp = m_procURL;
    // need support url like this /v1/servicename and /servicename
    if (strTmp.find(REST_VERSION) == 0) {
        mp_string::size_type nextPos = strTmp.find("/", REST_VERSION.length());
        m_version = strTmp.substr(1, nextPos - 1);
        strTmp = strTmp.substr(nextPos);
    }

    strTmp = strTmp.substr(1);
    mp_string::size_type pos = strTmp.find("/");

    return (pos != mp_string::npos) ? strTmp.substr(0, pos) : strTmp;
}

/* ------------------------------------------------------------
Function Name: GetCutURL
Description  : 从agent/开始，根据指定的idx返回截断后的URL
Others       :------------------------------------------------------------- */
mp_string CRequestURL::GetCutURL(mp_int32 indexValue)
{
    // 截取出各处理模块需要的URL字符串
    if (m_procURL.empty()) {
        return "";
    }

    mp_int32 iNum = 0;  // url中"/"出现的个数
    mp_string strTmp = m_procURL;
    mp_string::size_type pos = strTmp.find("/");
    while (pos != mp_string::npos) {
        iNum++;
        if (iNum == indexValue) {
            break;
        }
        strTmp = strTmp.substr(pos);
        pos = strTmp.find("/");
    }

    if (pos != mp_string::npos) {
        return "";
    }

    pos = strTmp.substr(1).find("/");

    return strTmp.substr(1, pos);
}

/* ------------------------------------------------------------
Function Name: readWait
Description  : 从http请求中读取内容，并按照app要求将数据对象格式处理好
Others       :------------------------------------------------------------- */
mp_int32 CRequestMsgBody::ReadWait(CHttpRequest& req)
{
    LOGGUARD("");
    auto_ptr<CMessage_Block> msg_blk(new (std::nothrow) CMessage_Block(MSG_BLK_SIZE));
    if (ReadData(req, *msg_blk.get()) != 0) {
        COMMLOG(OS_LOG_ERROR, "Read data failed.");
        return MP_FAILED;
    }
    mp_int32 diff = msg_blk->GetLength() - req.GetContentLen();
    if (diff != 1) {
        COMMLOG(OS_LOG_ERROR, "Length of data recieved is not equal to the value descripted in head.");
        return MP_FAILED;
    }

    return ParseJson(*msg_blk.get());
}

/* ------------------------------------------------------------
Function Name: readData
Description  : 从http流中读取内容
Others       :------------------------------------------------------------- */
mp_int32 CRequestMsgBody::ReadData(CHttpRequest& req, CMessage_Block& msg_blk)
{
    LOGGUARD("");

    for (;;) {
        mp_uint32 len = msg_blk.GetSize() - msg_blk.GetLength();
        if (len <= 1) {
            if (msg_blk.Resize(msg_blk.GetSize() + MSG_BLK_SIZE)) {
                COMMLOG(OS_LOG_ERROR, "Resize failed.");
                return MP_FAILED;
            }
            len = msg_blk.GetSize() - msg_blk.GetLength();
        }

        len -= 1;  // 预留末尾填0

        mp_int32 ret = req.ReadStr(msg_blk.GetWritePtr(), (mp_int32)len);
        if (ret < 0) {
            COMMLOG(OS_LOG_ERROR, "Fcgi read stream failed, iRet = %d", ret);
            return MP_FAILED;
        } else if (ret == 0) {
            break;
        }

        msg_blk.AddLength(static_cast<mp_uint32>(ret));

        if ((mp_uint32)ret < len) {
            break;
        }

        // 内存增长控制
        if ((mp_int32)msg_blk.GetLength() > MAX_MSG_CACHE_SIZE) {
            COMMLOG(OS_LOG_ERROR, "Size of msg block is overload");
            return ERROR_COMMON_MSG_BLOCK_OVERLOAD;
        }
    }

    // 如果是文件，末尾不加0
    if (m_msgBodyType == BODY_DECODE_JSON) {
        *msg_blk.GetWritePtr() = 0;
        msg_blk.AddLength(1);
    }

    m_msgLen = msg_blk.GetLength();
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: parseJson
Description  : 将读取到的字符串解析成json对象
Others       :------------------------------------------------------------- */
mp_int32 CRequestMsgBody::ParseJson(CMessage_Block& msg_block)
{
    LOGGUARD("");
    if ((msg_block.GetLength() == 0) || (msg_block.GetLength() == 1)) {
        COMMLOG(OS_LOG_DEBUG, "Length of message body is 0.");
        return MP_SUCCESS;
    }

    Json::Reader r;
    mp_bool bRet = r.parse(msg_block.GetReadPtr(), m_msgJsonData);
    if (bRet != MP_TRUE) {
        COMMLOG(OS_LOG_ERROR, "Parse json data failed");
        return ERROR_COMMON_INVALID_PARAM;
    }

    // 只处理对象和数组
    if (!m_msgJsonData.isObject() && !m_msgJsonData.isArray()) {
        COMMLOG(OS_LOG_ERROR, "Json type is error, neither object nor array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: jsonValueToString
Description  : json对象到字符串的转换
Others       :------------------------------------------------------------- */
mp_string CRequestMsgBody::JsonValueToString(const Json::Value& v)
{
    if (v.isString()) {
        return v.asString();
    } else {
        mp_string val;
        Json::FastWriter writer;
        val = writer.write(v);
        if (!val.empty()) {
            val = val.substr(0, val.length() - 1);
        }
        return val;
    }
}

/* ------------------------------------------------------------
Function Name: getValue
Description  : 根据名称获取json对象中对应的值
Others       :------------------------------------------------------------- */
mp_int32 CRequestMsgBody::GetValue(const mp_string& name, mp_string& value)
{
    if (m_msgJsonData.isMember(name)) {
        value = JsonValueToString(m_msgJsonData[name]);
        return MP_SUCCESS;
    }

    return MP_FAILED;
}

/* ------------------------------------------------------------
Function Name: getOriMsg
Description  : 获取消息体中的原始内容，用于文件传输处理
Others       :------------------------------------------------------------- */
mp_bool CRequestMsgBody::GetOriMsg(mp_char& buf, mp_uint32& len)
{
    if (m_raw_msg.get()) {
        buf = *m_raw_msg->GetReadPtr();
        len = (mp_uint32)m_raw_msg->GetLength();

        return MP_TRUE;
    } else {
        len = 0;
        return MP_FALSE;
    }
}

/* ------------------------------------------------------------
Function Name: CRequestMsg
Description  : CRequestMsg构造函数，fcgiReq为fastcgi请求对象
Others       :-------------------------------------------------------- */
CRequestMsg::CRequestMsg(FCGX_Request& pFcgiReq) : m_httpReq(pFcgiReq)
{
    m_iType = REQMESSAGE_TYPE;
    std::string strurl = m_httpReq.GetURL();
    std::string reqMethod = m_httpReq.GetMethod();

    COMMLOG(OS_LOG_DEBUG, "##### URL:%s, METHOD:%s", strurl.c_str(), reqMethod.c_str());

    m_url.SetOriURL(m_httpReq.GetURL());
    m_url.SetQueryParam(m_httpReq.GetQueryParamStr());
}

/* ------------------------------------------------------------
Function Name: parse
Description  : 解析http请求内容
Others       :-------------------------------------------------------- */
mp_int32 CRequestMsg::Parse()
{
    // 解析URL
    if (m_msgBody.ReadWait(m_httpReq)) {
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: send
Description  : 将响应消息写入http流中
Others       :-------------------------------------------------------- */
mp_int32 CResponseMsg::Send()
{
    mp_int32 iRet = MP_FAILED;
    if (m_httpType == RSP_JSON_TYPE || m_httpType == RSP_JSON_TYPE2) {
        iRet = SendJson();
    } else if (m_httpType == RSP_ATTACHMENT_TYPE) {
        iRet = SendAttchment();
    } else {
        COMMLOG(OS_LOG_ERROR, "m_httpType %d is wrong", m_httpType);
        m_httpRsp.Complete();
        iRet = MP_FAILED;
    }
    return iRet;
}

/* ------------------------------------------------------------
Function Name: SendJson
Description  : 将json响应消息写入http流中
Others       :-------------------------------------------------------- */
mp_int32 CResponseMsg::SendJson()
{
    LOGGUARD("");
    Json::Value jasonValue;
    PackageReponse(jasonValue);

    Json::FastWriter w;
    mp_string out = w.write(jasonValue);
    ostringstream ossLength;
    ossLength << out.size();

    ostringstream ossStatus;
    ossStatus << m_iHttpStatus;

    m_httpRsp.SetHead(STATUS, ossStatus.str().c_str());
    m_httpRsp.SetHead(HTTP_CONTENT_TYPE, "application/json; charset=utf-8");
    m_httpRsp.SetHead(CONTENT_ENCODING, "utf-8");
    m_httpRsp.SetHead(CONTENT_LENGTH, ossLength.str().c_str());

    // 写消息内容
    // windows平台返回消息统一由ansi转换成utf8
#ifdef WIN32
    mp_int32 iWriteLen = m_httpRsp.WriteS(CMpString::ANSIToUTF8(out));
#else
    mp_int32 iWriteLen = m_httpRsp.WriteS(out);
#endif
    if (iWriteLen < 0) {
        COMMLOG(OS_LOG_ERROR, "Write message into fcgi stream failed");
        return MP_FAILED;
    }
    m_httpRsp.Complete();
    if (out.size() > MAX_LOG_LEN) {
        COMMLOG(OS_LOG_DEBUG, "[Response Message]: strbuffer too long");
    }
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: SendAttchment
Description  : 将附件响应消息写入http流中
Others       :-------------------------------------------------------- */
mp_int32 CResponseMsg::SendAttchment()
{
    LOGGUARD("");

    if (m_lRetCode != MP_SUCCESS) {
        SendFailedRsp();
        m_httpRsp.Complete();
        return MP_FAILED;
    }

    mp_string strAttachFileName, strAttachFilePath;
    GET_JSON_STRING_OPTION(m_msgJsonData, REST_PARAM_ATTACHMENT_NAME, strAttachFileName);
    GET_JSON_STRING_OPTION(m_msgJsonData, REST_PARAM_ATTACHMENT_PATH, strAttachFilePath);

    ifstream ifs(strAttachFilePath.c_str(), ios_base::in | ios_base::binary);
    if (!ifs) {
        COMMLOG(OS_LOG_ERROR, "ifs error, strAttachFilePath = %s", strAttachFilePath.c_str());
        SendFailedRsp();
        m_httpRsp.Complete();
        return MP_FAILED;
    }

    ifs.seekg(0, ios_base::end);
    mp_uint64 fileLen = static_cast<mp_uint64>(ifs.tellg());
    if (fileLen >= MAX_ATTCHMENT_SIZE) {
        COMMLOG(OS_LOG_ERROR, "length[%d] of attachment is too big", fileLen);
        SendFailedRsp();
        m_httpRsp.Complete();
        return MP_FAILED;
    }

    // 设置http的header
    SetHeader(strAttachFileName);

    // 写附件内容
    ostringstream ossLen;
    ossLen << fileLen;
    m_httpRsp.SetHead("Content-Length", ossLen.str().c_str());
    ifs.clear();
    ifs.seekg(0);
    char tmp;
    mp_uint64 u64OneceMsg = 0;
    while (ifs.get(tmp)) {
        m_httpRsp.WriteStr(mp_string(&tmp), 1);
        u64OneceMsg++;
        // 解决在hp环境下附件大小超过32K时nginx出现readv错误问题
        if (u64OneceMsg >= ONCE_SEND_MSG_SIZE) {
            u64OneceMsg = 0;
            DoSleep(MESSAGERES_NUM_10);
        }
    }
    ifs.close();
    m_httpRsp.Complete();
    COMMLOG(OS_LOG_INFO, "Send attachment success, attachment size:%s", ossLen.str().c_str());
    return MP_SUCCESS;
}

mp_void CResponseMsg::SetHeader(const mp_string& strAttachFileName)
{
    ostringstream ossStatus;
    ossStatus << m_iHttpStatus;
    m_httpRsp.SetHead(STATUS, ossStatus.str().c_str());
    m_httpRsp.SetHead("Content-type", "application/octet-stream");
    mp_string strHeadFileName = "attachment;filename=" + strAttachFileName;
    m_httpRsp.SetHead("Content-Disposition", strHeadFileName.c_str());
    m_httpRsp.SetHead("Pragma", "public");
}

/* ------------------------------------------------------------
Function Name: packageReponse
Description  : 封装http响应格式
Others       :-------------------------------------------------------- */
mp_void CResponseMsg::PackageReponse(Json::Value& root)
{
    LOGGUARD("RetCode is %d, HttpStatus is %d", m_lRetCode, m_iHttpStatus);
    if (m_lRetCode == MP_SUCCESS) {
        root = m_msgJsonData;
    } else {
        ostringstream oss;
        oss << m_lRetCode;
        root["errorCode"] = oss.str();
        if (m_httpType == RSP_JSON_TYPE) {
            root["errorMessage"] = m_msgJsonData;
        } else if (m_httpType == RSP_JSON_TYPE2) {
            root["errorMessage"] = m_msgJsonData.isNull() ? "" : m_msgJsonData.toStyledString();
        }
        COMMLOG(OS_LOG_DEBUG, "Response error code %d.", m_lRetCode);
        if (m_iHttpStatus == SC_OK) {
            // 如果各逻辑模块没有设置http status,默认设置为SC_NOT_ACCEPTABLE
            m_iHttpStatus = SC_NOT_ACCEPTABLE;
        }
    }
}

/* ------------------------------------------------------------
Function Name: SendFailedRsp
Description  : 发送失败返回消息
Others       :-------------------------------------------------------- */
mp_void CResponseMsg::SendFailedRsp()
{
    ostringstream oss;
    ostringstream ossStatus;
    oss << m_lRetCode;
    Json::Value jasonValue;
    jasonValue["errorCode"] = oss.str();
    COMMLOG(OS_LOG_DEBUG, "Response error code %d.", m_lRetCode);
    ossStatus << SC_NOT_ACCEPTABLE;
    m_httpRsp.SetHead(STATUS, ossStatus.str().c_str());
    Json::FastWriter w;
    mp_string out = w.write(jasonValue);
    mp_int32 iWriteLen = m_httpRsp.WriteS(out);
    if (iWriteLen < 0) {
        COMMLOG(OS_LOG_ERROR, "Write message into fcgi stream failed");
    }
}

/* ------------------------------------------------------------
Description  :将字符串转化成数值
Output       : ret -- 转化结果
Return       : MP_TRUE -- 成功
               非MP_TRUE -- 失败
------------------------------------------------------------- */
mp_bool CUrlUtils::GetNum(const mp_char s[], mp_uint32 len, mp_uint32 b, mp_uint64& ret)
{
    mp_uint32 cv = 0;

    ret = 0;

    mp_bool cd = ((len > URLUTILS_NUM_16) || (b > URLUTILS_NUM_16) || (s == NULL) || (len == 0));
    if (cd) {
        return MP_FALSE;
    }

    struct {
        mp_char c;
        mp_int32 v;
    } m[] = {
        {'0', 0},
        {'1', 1},
        {'2', 2},
        {'3', 3},
        {'4', 4},
        {'5', 5},
        {'6', 6},
        {'7', 7},
        {'8', 8},
        {'9', 9},
        {'a', 10},
        {'b', 11},
        {'c', 12},
        {'d', 13},
        {'e', 14},
        {'f', 15},
    };

    for (mp_uint32 i = 0; i < len; i++) {
        mp_uint32 j = 0;
        for (; j < sizeof(m) / sizeof(m[j]) && j < b; ++j) {
            mp_char c = (mp_char)tolower(s[i]);
            if (c == m[j].c) {
                cv = static_cast<mp_uint32>(m[j].v);
                break;
            }
        }

        if (j >= b) {
            return MP_FALSE;
        }

        ret *= b;
        ret += cv;
    }
    return MP_TRUE;
}
/* ------------------------------------------------------------
Description  :url解码
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败
------------------------------------------------------------- */
mp_int32 CUrlUtils::Urldecode2(mp_char indec[], const mp_char s[], mp_int32 len)
{
    if (!indec || !s) {
        return MP_SUCCESS;
    }
    mp_char* o;
    const mp_char* end = s + len;
    unsigned long long c;
    for (o = indec; s < end; o++) {
        c = static_cast<unsigned long long>(*s);
        s++;
        if (c == '+') {
            c = ' ';
        } else if (c == '%') {
            mp_char s1 = *s;
            s++;
            mp_char s2 = *s;
            s++;
            if (!CUrlUtils::IsHex(s1) || !CUrlUtils::IsHex(s2) ||
                !CUrlUtils::GetNum(s - URLUTILS_NUM_2, URLUTILS_NUM_2, URLUTILS_NUM_16, c)) {
                *o++ = 0;
                return ERROR_COMMON_INVALID_PARAM;
            }
        }

        *o = (mp_char)c;
    }
    *o++ = 0;
    return (mp_int32)(o - indec);
}

/* ------------------------------------------------------------
Description  :获取等号右边的值
------------------------------------------------------------- */
mp_string CUrlUtils::GetBetweenValue(const mp_char eqp[], const mp_char p[])
{
    // CodeDex误报，ZERO_LENGTH_ALLOCATIONS
    if (p - eqp <= 1) {
        return "";
    } else {
        mp_char* buf = NULL;
        // CodeDex误报，Memory Leak
        NEW_ARRAY_CATCH(buf, mp_char, p + 1 - eqp);
        if (!buf) {
            return "";
        }
        buf[0] = 0;
        (mp_void) CUrlUtils::Urldecode2(buf, eqp + 1, (mp_int32)(p - (eqp + 1)));
        mp_string ret(buf);
        delete[] buf;
        return ret;
    }
}

