/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
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
