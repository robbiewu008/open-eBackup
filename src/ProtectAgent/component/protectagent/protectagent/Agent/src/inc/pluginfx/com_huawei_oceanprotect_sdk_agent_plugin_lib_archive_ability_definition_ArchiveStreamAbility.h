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
#include <jni.h>
/* Header for class com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility */

#ifndef Included_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
#define Included_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
 * Method:    connect
 * Signature: (Ljava/lang/String;Ljava/lang/String;IZ)
  Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/rsp/ArchiveConnectRsp;
 */
JNIEXPORT jobject JNICALL
 Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility_connect
  (JNIEnv *, jobject, jstring, jstring, jint, jboolean);

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
 * Method:    init
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)
  Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/rsp/ArchiveInitRsp;
 */
JNIEXPORT jobject JNICALL
 Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility_init
  (JNIEnv *, jobject, jstring, jstring, jstring);

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
 * Method:    prepareRecovery
 * Signature: (Ljava/lang/String;)
  Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/rsp/ArchivePrepareRsp;
 */
JNIEXPORT jobject JNICALL
 Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility_prepareRecovery
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
 * Method:    queryPrepareStatus
 * Signature: (Ljava/lang/String;)
  Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/rsp/ArchivePrepareStatusRsp;
 */
JNIEXPORT jobject JNICALL
 Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility_queryPrepareStatus
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
 * Method:    getBackupInfo
 * Signature: (Ljava/lang/String;)
 Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/rsp/ArchiveStreamCopyInfoRsp;
 */
JNIEXPORT jobject JNICALL
 Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility_getBackupInfo
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
 * Method:    getRecoverObjectList
 * Signature: (Ljava/lang/String;JLjava/lang/String;)
  Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/rsp/ArchiveRecoverObjectRsp;
 */
JNIEXPORT jobject JNICALL
 Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility_getRecoverObjectList
  (JNIEnv *, jobject, jstring, jlong, jstring);

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
 * Method:    getDirMetaData
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)
  Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/rsp/ArchiveFileMetaDataRsp;
 */
JNIEXPORT jobject JNICALL
 Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility_getDirMetaData
  (JNIEnv *, jobject, jstring, jstring, jstring);

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
 * Method:    getFileMetaData
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)
  Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/rsp/ArchiveFileMetaDataRsp;
 */
JNIEXPORT jobject JNICALL
 Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility_getFileMetaData
  (JNIEnv *, jobject, jstring, jstring, jstring);

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
 * Method:    getFileData
 * Signature: (Ljava/lang/String;
  Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/req/ArchiveStreamGetFileReq;)
  Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/rsp/ArchiveStreamGetFileRsp;
 */
JNIEXPORT jobject JNICALL
 Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility_getFileData
  (JNIEnv *, jobject, jstring, jobject);

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
 * Method:    endRecover
 * Signature: (Ljava/lang/String;)
  Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/rsp/ArchiveEndRecoverRsp;
 */
JNIEXPORT jobject JNICALL
 Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility_endRecover
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
 * Method:    disconnect
 * Signature: (Ljava/lang/String;)
  Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/rsp/ArchiveDisConnectRsp;
 */
JNIEXPORT jobject JNICALL
 Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility_disconnect
  (JNIEnv *, jobject, jstring);

#ifdef __cplusplus
}
#endif
#endif
