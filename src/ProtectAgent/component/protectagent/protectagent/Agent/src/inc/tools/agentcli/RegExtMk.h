#ifndef AGENTCLI_REG_EXT_MK
#define AGENTCLI_REG_EXT_MK

#include "common/Types.h"

#define INPUT_PLAIN_TEXT "Input a plaintext which consists of 32 to 127 characters:"
#define INPUT_MK_LIFE_DAYS "Input a key validity period whose range is [30, 3650] (days):"
#define HINT_REGISTER_MK_SUCC() \
    printf("%s\n", "Registering the external key is successful. The key will take effect immediately.")
#define HINT_REGISTER_MK_FAILED() printf("%s\n", "Registering the external key failed.")

class RegExtMk {
public:
    static mp_int32 Handle();

private:
    static mp_void SafeClearPasswdInner(mp_string& str);
    static mp_void SafeClearPasswd();
    static mp_int32 VerifyUserPasswd();
    static mp_int32 CheckPlainText(mp_string& plainText);
    static mp_int32 CheckMkLifeDays(mp_uint32& keyLifeDays);
    static mp_int32 DecryptStoredPasswd();
    static mp_void RollbackCiphertext();
    static mp_int32 EncryptPasswdWithExtMk();
    static mp_int32 SignAllScriptsWithNewMK();
    static mp_uint32 StringToUint32(const mp_string& originStr);

private:
    static mp_string nginxKeyPasswd;
    static mp_string snmpPrivatePasswd;
    static mp_string snmpAuthPasswd;
    static mp_string nginxCiphertext;
    static mp_string snmpPrivateCiphertext;
    static mp_string snmpAuthCiphertext;
    static const mp_uchar REGEXTMK_NUM_4 = 4;
};

#endif
