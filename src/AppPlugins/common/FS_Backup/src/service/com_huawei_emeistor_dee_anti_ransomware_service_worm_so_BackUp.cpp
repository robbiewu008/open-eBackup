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
#include "com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp.h"
#include <map>
#include <cstring>
#include "log/Log.h"
#include "AntiRansomware.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
    const std::string MODULE = "com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp";
    constexpr int BACKUP_LOG_COUNT = 100;
    constexpr int BACKUP_LOG_MAX_SIZE = 30;
    const int BACKUP_PHASE_ANTI_ANSOMWARE = 5;
    const int BACKUP_COMPLETED = 1;
    const int BACKUP_INPROGRESS = 2;
    const int BACKUP_FAILED = 3;
    const int MAX_BUFFER_CNT = 10;
    const int MAX_BUFFER_SIZE = 10 * 1024;  // 10kb
    const int MAX_ERROR_FILES = 100;

    typedef struct BackupConf_S {
        std::string jobId;
        int reqID;
        std::string ip;                          /* nas share ip */
        std::string sharePath;                   /* nas share */
        std::string metaPath;                    /* Metadata path for control files of nas share */
        int phase;                               /* BACKUP_PHASE_ANTI_ANSOMWARE = 5 */
        int antiType;                            /* WORM =0 , ENTROPY =1 */
        long nfsAtime;                           /* 存储元数据中的atime */
        std::string nfsMode;                     /* 类似："a+w" , "ugo-w", “a=w” */
        bool isOnlyModifyAtime;                    /* 仅设置atime时，mode设为-1 */
    } BackupConf;
}

/* key: jobId, value: backup inst ptr */
static std::map<std::string, shared_ptr<Backup>> g_mapBackupMgr;

jstring CStringToJstring(JNIEnv* env, const char* pat)
{
    jclass strClass = (env)->FindClass("Ljava/lang/String;");
    jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    jbyteArray bytes = (env)->NewByteArray(strlen(pat));
    (env)->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte*)pat);
    jstring encoding = (env)->NewStringUTF("UTF-8");
    return (jstring)(env)->NewObject(strClass, ctorID, bytes, encoding);
}

std::string JstringToStr(JNIEnv *env, jstring jStr)
{
    if (!jStr) {
        return "";
    }

    const jclass stringClass = env->GetObjectClass(jStr);
    const jmethodID getBytes = env->GetMethodID(stringClass, "getBytes", "(Ljava/lang/String;)[B");
    const jbyteArray stringJbytes = (jbyteArray)env->CallObjectMethod(jStr, getBytes, env->NewStringUTF("UTF-8"));

    size_t length = (size_t)env->GetArrayLength(stringJbytes);
    jbyte* pBytes = env->GetByteArrayElements(stringJbytes, nullptr);

    std::string ret = std::string((char *)pBytes, length);
    env->ReleaseByteArrayElements(stringJbytes, pBytes, JNI_ABORT);

    env->DeleteLocalRef(stringJbytes);
    env->DeleteLocalRef(stringClass);
    return ret;
}

void JObjectToStruct(JNIEnv *env, const jobject& obj, BackupConf& conf)
{
    jclass cls = env->GetObjectClass(obj);
    {
        jfieldID fid = env->GetFieldID(cls, "jobId", "Ljava/lang/String;");
        jstring strValue = (jstring)env->GetObjectField(obj, fid);
        conf.jobId = JstringToStr(env, strValue);
    }
    {
        jfieldID fid = env->GetFieldID(cls, "reqID", "I");
        conf.reqID = env->GetIntField(obj, fid);
    }
    {
        jfieldID fid = env->GetFieldID(cls, "ip", "Ljava/lang/String;");
        jstring strValue = (jstring)env->GetObjectField(obj, fid);
        conf.ip = JstringToStr(env, strValue);
    }
    {
        jfieldID fid = env->GetFieldID(cls, "sharePath", "Ljava/lang/String;");
        jstring strValue = (jstring)env->GetObjectField(obj, fid);
        conf.sharePath = JstringToStr(env, strValue);
    }
    {
        jfieldID fid = env->GetFieldID(cls, "metaPath", "Ljava/lang/String;");
        jstring strValue = (jstring)env->GetObjectField(obj, fid);
        conf.metaPath = JstringToStr(env, strValue);
    }
    {
        jfieldID fid = env->GetFieldID(cls, "phase", "I");
        conf.phase = env->GetIntField(obj, fid);
    }
    {
        jfieldID fid = env->GetFieldID(cls, "antiType", "I");
        conf.antiType = env->GetIntField(obj, fid);
    }
    {
        jfieldID fid = env->GetFieldID(cls, "nfsAtime", "J");
        conf.nfsAtime = env->GetLongField(obj, fid);
    }
    {
        jfieldID fid = env->GetFieldID(cls, "nfsMode", "Ljava/lang/String;");
        jstring strValue = (jstring)env->GetObjectField(obj, fid);
        conf.nfsMode = JstringToStr(env, strValue);
    }
    {
        jfieldID fid = env->GetFieldID(cls, "isOnlyModifyAtime", "Z");
        conf.isOnlyModifyAtime= env->GetBooleanField(obj, fid);
    }
}

CommonParams FillCommonParas()
{
    CommonParams commonParams {};
    commonParams.maxBufferCnt = MAX_BUFFER_CNT;
    commonParams.maxBufferSize = MAX_BUFFER_SIZE;
    commonParams.maxErrorFiles = MAX_ERROR_FILES;
    commonParams.backupDataFormat = BackupDataFormat::NATIVE;
    commonParams.restoreReplacePolicy = RestoreReplacePolicy::OVERWRITE;
    commonParams.jobId = "jobId";
    commonParams.subJobId = "subJobId";
    commonParams.skipFailure = true;    // stop backup if any item backup failed.
    return commonParams;
}

/*
 * Class:     com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp
 * Method:    initLog
 * Signature: (Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp_initLog(
    JNIEnv *env, jobject, jstring jFullLogPath, jint jlogLevel)
{
    string fullLogPath = JstringToStr(env, jFullLogPath);
    unsigned int iLogLevel = static_cast<unsigned int>(jlogLevel);
    int iLogCount = BACKUP_LOG_COUNT;
    int iLogMaxSize = BACKUP_LOG_MAX_SIZE;
    std::string backupLogName = "backup.log";
    Module::CLogger::GetInstance().Init(backupLogName.c_str(), fullLogPath, iLogLevel, iLogCount, iLogMaxSize);
}

/*
 * Class:     com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp
 * Method:    createAntiBackupInst
 * Signature: (Lcom/huawei/emeistor/dee/anti/ransomware/service/worm/so/bean/BackUpConf;)V
 */
JNIEXPORT jboolean JNICALL Java_com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp_createAntiBackupInst(
    JNIEnv *env, jobject, jobject jBackupConf)
{
    INFOLOG("Enter CreateAntiBackupInst");
    BackupConf backupConf;
    JObjectToStruct(env, jBackupConf, backupConf);
    if (g_mapBackupMgr.count(backupConf.jobId) != 0) {
        return JNI_TRUE;
    }
    shared_ptr<Backup> backupHandle = nullptr;
    BackupParams backupParams {};
    backupParams.phase = BackupPhase(backupConf.phase);
    backupParams.srcAdvParams = make_shared<NfsAntiRansomwareAdvanceParams>();
    backupParams.dstAdvParams = make_shared<NfsAntiRansomwareAdvanceParams>();
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.srcAdvParams)->reqID = backupConf.reqID;
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.srcAdvParams)->backupAntiType =
        BackupAntiType(backupConf.antiType);
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.srcAdvParams)->atime = backupConf.nfsAtime;
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.srcAdvParams)->mode = backupConf.nfsMode;
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.srcAdvParams)->ip = backupConf.ip;
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.srcAdvParams)->sharePath = backupConf.sharePath;
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.srcAdvParams)->protocolVersion = NFS_V3;
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.srcAdvParams)->isOnlyModifyAtime =
        backupConf.isOnlyModifyAtime;
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.dstAdvParams)->reqID = backupConf.reqID;
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.dstAdvParams)->backupAntiType =
        BackupAntiType(backupConf.antiType);
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.dstAdvParams)->atime = backupConf.nfsAtime;
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.dstAdvParams)->mode = backupConf.nfsMode;
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.dstAdvParams)->ip = backupConf.ip;
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.dstAdvParams)->sharePath = backupConf.sharePath;
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.dstAdvParams)->protocolVersion = NFS_V3;
    
    backupParams.commonParams = FillCommonParas();

    ScanAdvanceParams scanAdvParams {};
    scanAdvParams.metaFilePath = backupConf.metaPath + "/latest";
    backupParams.scanAdvParams = scanAdvParams;

    backupParams.backupType = BackupType::BACKUP_FULL;

    if (backupConf.phase == BACKUP_PHASE_ANTI_ANSOMWARE) {
        backupParams.srcEngine = BackupIOEngine::NFS_ANTI_ANSOMWARE;
        backupParams.dstEngine = BackupIOEngine::NFS_ANTI_ANSOMWARE;
        backupHandle = make_shared<AntiRansomware>(backupParams);
    } else {
        ERRLOG("Not included Phase.");
        return JNI_FALSE;
    }
    g_mapBackupMgr.insert({backupConf.jobId, backupHandle});
    INFOLOG("Exit CreateAntiBackupInst");
    return JNI_TRUE;
}

/*
 * Class:     com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp
 * Method:    enqueue
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp_enqueue(
    JNIEnv *env, jobject, jstring jJobId, jstring jBackupControlFile)
{
    INFOLOG("Enter Enqueue");
    string jobId = JstringToStr(env, jJobId);
    string backupControlFile = JstringToStr(env, jBackupControlFile);

    INFOLOG("Enqueue jobId: %s", jobId.c_str());
    if (g_mapBackupMgr.count(jobId) == 0) {
        HCP_Log(ERR, MODULE) << "Backup mgr does not count this jobId: " << jobId << HCPENDLOG;
        return -1;
    }
    INFOLOG("find jobId in g_mapBackupMgr: %s", jobId.c_str());
    shared_ptr<Backup> backupHandle = g_mapBackupMgr[jobId];
    INFOLOG("Get backup instance: %s", jobId.c_str());
    if (backupHandle == nullptr) {
        return -1;
    }
    string controlFile = backupControlFile;
    BackupRetCode ret = backupHandle->Enqueue(controlFile);
    if (ret == BackupRetCode::SUCCESS) {
        return 0;
    }
    return -1;
}

/*
 * Class:     com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp
 * Method:    start
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp_start(
    JNIEnv *env, jobject, jstring jJobId)
{
    INFOLOG("Enter Start");
    string jobId = JstringToStr(env, jJobId);

    INFOLOG("Backup Start jobId: %s", jobId.c_str());
    if (g_mapBackupMgr.count(jobId) == 0) {
        HCP_Log(ERR, MODULE) << "Backup mgr does not count this jobId: " << jobId << HCPENDLOG;
        return -1;
    }
    INFOLOG("find jobId in g_mapBackupMgr: %s", jobId.c_str());
    shared_ptr<Backup> backupHandle = g_mapBackupMgr[jobId];
    INFOLOG("Get backup instance: %s", jobId.c_str());
    if (backupHandle == nullptr) {
        return -1;
    }
    BackupRetCode ret = backupHandle->Start();
    if (ret == BackupRetCode::SUCCESS) {
        return 0;
    }
    return -1;
}

/*
 * Class:     com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp
 * Method:    getStatus
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp_getStatus(
    JNIEnv *env, jobject, jstring jJobId)
{
    INFOLOG("Enter GetStatus");
    string jobId = JstringToStr(env, jJobId);

    INFOLOG("Backup GetStatus jobId: %s", jobId.c_str());
    if (g_mapBackupMgr.count(jobId) == 0) {
        HCP_Log(ERR, MODULE) << "Backup mgr does not count this jobId: " << jobId << HCPENDLOG;
        return BACKUP_FAILED;
    }
    INFOLOG("find jobId in g_mapBackupMgr: %s", jobId.c_str());
    shared_ptr<Backup> backupHandle = g_mapBackupMgr[jobId];
    INFOLOG("Get backup instance: %s", jobId.c_str());
    if (backupHandle == nullptr) {
        return BACKUP_FAILED;
    }
    BackupPhaseStatus status = backupHandle->GetStatus();
    if (status == BackupPhaseStatus::COMPLETED) {
        return BACKUP_COMPLETED;
    } else if (status == BackupPhaseStatus::INPROGRESS) {
        return BACKUP_INPROGRESS;
    }
    return BACKUP_FAILED;
}

/*
 * Class:     com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp
 * Method:    destroyBackupInst
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_huawei_emeistor_dee_anti_ransomware_service_worm_so_BackUp_destroyBackupInst(
    JNIEnv *env, jobject, jstring jJobId)
{
    INFOLOG("Enter DestroyBackupInst");
    string jobId = JstringToStr(env, jJobId);

    INFOLOG("Backup GetStats jobId: %s", jobId.c_str());
    if (g_mapBackupMgr.count(jobId) == 0) {
        HCP_Log(ERR, MODULE) << "Backup mgr does not count this jobId: " << jobId << HCPENDLOG;
        return;
    }
    shared_ptr<Backup> backupHandle = g_mapBackupMgr[jobId];
    INFOLOG("Get backup instance: %s", jobId.c_str());
    if (backupHandle != nullptr) {
        backupHandle->Destroy();
        g_mapBackupMgr.erase(jobId);
    }
    INFOLOG("Exit DestroyBackupInst Success");
}