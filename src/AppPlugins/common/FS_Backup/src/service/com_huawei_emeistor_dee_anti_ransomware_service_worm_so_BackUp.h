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
#ifndef FS_BACKUP_SRC_SERVICE_COM_HUAWEI_EMEISTOR_DEE_ANTI_RANSOMWARE_SERVICE_WORM_SO_BACKUP_H
#define FS_BACKUP_SRC_SERVICE_COM_HUAWEI_EMEISTOR_DEE_ANTI_RANSOMWARE_SERVICE_WORM_SO_BACKUP_H
#include "Backup.h"
#include "jni.h"

/* Header for class com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp */
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp
 * Method:    initLog
 * Signature: (Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp_initLog
  (JNIEnv *, jobject, jstring, jint);

/*
 * Class:     com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp
 * Method:    createAntiBackupInst
 * Signature: (Lcom/huawei/emeistor/dee/anti/ransomware/service/worm/so/bean/BackUpConf;)V
 */
JNIEXPORT jboolean JNICALL Java_com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp_createAntiBackupInst
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp
 * Method:    enqueue
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp_enqueue
  (JNIEnv *, jobject, jstring, jstring);

/*
 * Class:     com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp
 * Method:    start
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp_start
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp
 * Method:    getStatus
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp_getStatus
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp
 * Method:    getStats
 * Signature: (Ljava/lang/String;)Lcom/huawei/emeistor/dee/anti/ransomware/service/worm/so/bean/BackupStatistics;
 */
JNIEXPORT jobject JNICALL Java_com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp_getStats
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp
 * Method:    destroyBackupInst
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp_destroyBackupInst
  (JNIEnv *, jobject, jstring);

#ifdef __cplusplus
}
#endif
#endif  // FS_BACKUP_SRC_SERVICE_COM_HUAWEI_EMEISTOR_DEE_ANTI_RANSOMWARE_SERVICE_WORM_SO_BACKUP_H
