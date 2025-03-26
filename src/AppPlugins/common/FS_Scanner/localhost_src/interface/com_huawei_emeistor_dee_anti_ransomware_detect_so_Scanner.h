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
#ifndef FS_SCANNER_LOCALHOST_SRC_INTERFACE_COM_HUAWEI_EMEISTOR_DEE_ANTI_RANSOMWARE_SERVICE_WORM_SO_SCANNER_H
#define FS_SCANNER_LOCALHOST_SRC_INTERFACE_COM_HUAWEI_EMEISTOR_DEE_ANTI_RANSOMWARE_SERVICE_WORM_SO_SCANNER_H
#include <jni.h>
/* Header for class com_huawei_emeistor_dee_anti_ransomware_service_worm_so_Scanner */

#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_huawei_emeistor_dee_anti_ransomware_service_worm_so_Scanner
 * Method:    InitLog
 * Signature: (Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_huawei_emeistor_dee_anti_ransomware_detect_so_Scanner_initLog
  (JNIEnv *, jobject, jstring, jint);

/*
 * Class:     com_huawei_emeistor_dee_anti_ransomware_service_worm_so_Scanner
 * Method:    CreateScannerInst
 * Signature: (Lcom/huawei/emeistor/dee/anti/ransomware/service/worm/so/bean/ScanConf;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_huawei_emeistor_dee_anti_ransomware_detect_so_Scanner_createScannerInst
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_huawei_emeistor_dee_anti_ransomware_service_worm_so_Scanner
 * Method:    DestroyScannerInst
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_huawei_emeistor_dee_anti_ransomware_detect_so_Scanner_destroyScannerInst
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_huawei_emeistor_dee_anti_ransomware_service_worm_so_Scanner
 * Method:    StartScanner
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_huawei_emeistor_dee_anti_ransomware_detect_so_Scanner_startScanner
  (JNIEnv *, jobject, jstring, jstring);

/*
 * Class:     com_huawei_emeistor_dee_anti_ransomware_service_worm_so_Scanner
 * Method:    MonitorScanner
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_huawei_emeistor_dee_anti_ransomware_detect_so_Scanner_monitorScanner
  (JNIEnv *, jobject, jstring);

#ifdef __cplusplus
}
#endif
#endif  // FS_SCANNER_LOCALHOST_SRC_INTERFACE_COM_HUAWEI_EMEISTOR_DEE_ANTI_RANSOMWARE_SERVICE_WORM_SO_SCANNER_H