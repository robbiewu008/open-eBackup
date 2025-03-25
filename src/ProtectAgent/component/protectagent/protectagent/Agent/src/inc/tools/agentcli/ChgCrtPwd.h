#ifndef _AGENTCLI_CHGCRTPWD_H_
#define _AGENTCLI_CHGCRTPWD_H_

#include "common/Types.h"

static const mp_string CERT_NEW_NAME = "server_new.key";
static const mp_uchar  CHECK_PASSWORD_OVER_TIMES = 3;
#ifdef WIN32
static const mp_string OPENSSL_FILE_NAME = "openssl.exe";
static const mp_string OPENSSL_CONF_FILE_NAME = "openssl.cnf";
#else
static const mp_string OPENSSL_FILE_NAME = "openssl";
#endif

class ChgCrtPwd {
public:
    static mp_int32 Handle();

private:
    static void ReSetPwdMem(mp_string &strCrtOldPwd, mp_string &strCrtNewPwd, mp_string &strCrtNewEncryptPwd);
    static mp_int32 InputCrtOldPwd(mp_string& strCrtOldPwd, const mp_string& strUserName);
    static mp_int32 InputCrtNewPwd(mp_string& strCrtNewPwd, mp_string &strCrtOldPwd);
    static mp_int32 ChangeCrtPwd(mp_string &strCrtNewPwd, mp_string &strCrtOldPwd);
    static mp_int32 GetCertFileName(mp_string& strCrtFileName);
    static mp_int32 BuildCertFile(const mp_string& strNewCrtFileName);
};

#endif /* _AGENTCLI_CHGCRTPWD_H_ */
