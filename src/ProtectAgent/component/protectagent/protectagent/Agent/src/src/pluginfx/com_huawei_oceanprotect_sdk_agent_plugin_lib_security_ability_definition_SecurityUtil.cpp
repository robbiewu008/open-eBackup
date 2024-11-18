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
#include "pluginfx/com_huawei_oceanprotect_sdk_agent_plugin_lib_security_ability_definition_SecurityUtil.h"
#include "pluginfx/ExternalPluginSDK.h"
#include "pluginfx/jnitostd.h"

JNIEXPORT jboolean JNICALL
Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_security_ability_definition_SecurityUtil_opaInitialize
(JNIEnv *env, jobject, jstring path)
{
    auto stdPath = jstring2string(env, path);
    auto ret = OpaInitialize(stdPath);
    if (ret != OPA_SUCCESS) {
        return false;
    }
    return true;
}

JNIEXPORT jstring JNICALL
Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_security_ability_definition_SecurityUtil_getSecurityItemValue
(JNIEnv *env, jobject obj, jint item)
{
    std::string value;
    GetSecurityItemValue(item, value);
    return stringTojstring(env, value);
}

JNIEXPORT jstring JNICALL
Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_security_ability_definition_SecurityUtil_decrypt
(JNIEnv *env, jobject, jstring str)
{
    auto stdStr = jstring2string(env, str);
    std::string value;
    Decrypt(stdStr, value);
    auto jstr = stringTojstring(env, value);
    if (value.empty()) {
        return jstr;
    }
    memset_s(&value[0], value.length(), 0, value.length());
    return jstr;
}