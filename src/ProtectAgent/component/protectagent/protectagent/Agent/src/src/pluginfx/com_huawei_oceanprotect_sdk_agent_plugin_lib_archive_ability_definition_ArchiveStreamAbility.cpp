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
#include "pluginfx/com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility.h"
#include "message/archivestream/ArchiveStreamService.h"
#include "common/Log.h"

enum class ArchiveStreamErrType {
    ARCHIVE_EXCEPTION_SUCCESS = 0,
    ARCHIVE_EXCEPTION_REQ_INVALID = 1,
    ARCHIVE_EXCEPTION_INNER_ERROR = 2,
    ARCHIVE_EXCEPTION_S3_READ_FAIL = 3,
    ARCHIVE_EXCEPTION_S3_CONNECT_FAIL = 4,
    ARCHIVE_EXCEPTION_S3_FILE_NOT_EXIST = 5,
    ARCHIVE_EXCEPTION_COPY_NOT_EXIST = 6,
    ARCHIVE_EXCEPTION_COPY_STATUS_INVALID = 7,
    ARCHIVE_EXCEPTION_QUERY_S3INFO_FAIL = 8,
    ARCHIVE_EXCEPTION_S3_OPEN_COPY_FAIL = 9,
    ARCHIVE_EXCEPTION_NOT_INIT = 10,
    ARCHIVE_EXCEPTION_SAME_ID_INITED = 11,
    ARCHIVE_EXCEPTION_SDK_INNER_ERR = 12,
    ARCHIVE_EXCEPTION_CONNECTION_TOO_MUCH = -14,
};

jstring charTojstring(JNIEnv* env, const char* pat)
{
    jclass strClass = (env)->FindClass("Ljava/lang/String;");
    jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    jbyteArray bytes = (env)->NewByteArray(strlen(pat));
    (env)->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte*)pat);
    jstring encoding = (env)->NewStringUTF("UTF-8");
    return (jstring)(env)->NewObject(strClass, ctorID, bytes, encoding);
}

std::string jstring2str(JNIEnv *env, jstring jStr)
{
    if (!jStr) {
        return "";
    }

    const jclass stringClass = env->GetObjectClass(jStr);
    const jmethodID getBytes = env->GetMethodID(stringClass, "getBytes", "(Ljava/lang/String;)[B");
    const jbyteArray stringJbytes = (jbyteArray)env->CallObjectMethod(jStr, getBytes, env->NewStringUTF("UTF-8"));

    size_t length = (size_t)env->GetArrayLength(stringJbytes);
    jbyte* pBytes = env->GetByteArrayElements(stringJbytes, NULL);

    std::string ret = std::string((char *)pBytes, length);
    env->ReleaseByteArrayElements(stringJbytes, pBytes, JNI_ABORT);

    env->DeleteLocalRef(stringJbytes);
    env->DeleteLocalRef(stringClass);
    return ret;
}

jobject BuildRspObject(JNIEnv *env, jclass& cls, const mp_string& taskID, mp_int32 errcode)
{
    jobject obj = env->AllocObject(cls);
    {
        jstring jStr = charTojstring(env, taskID.c_str());
        jfieldID fid = env->GetFieldID(cls, "taskID", "Ljava/lang/String;");
        env->SetObjectField(obj, fid, jStr);
    }
    {
        jint nValue = errcode;
        jfieldID fid = env->GetFieldID(cls, "code", "I");
        env->SetIntField(obj, fid, nValue);
    }
    return obj;
}

static mp_string classNamePrefix = "com/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/rsp/";
static std::map<mp_string, ArchiveStreamService*> g_mapASService;

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
 * Method:    init
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)
 * Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/ArchiveInitRsq;
 */
JNIEXPORT jobject JNICALL
Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility_init
(JNIEnv *env, jobject, jstring jTaskID, jstring jBackupId, jstring jDirList)
{
    mp_string className = classNamePrefix + "ArchiveInitRsp";
    jclass cls = env->FindClass(className.c_str());

    mp_string backupId = jstring2str(env, jBackupId);
    mp_string taskID = jstring2str(env, jTaskID);
    mp_string dirList = jstring2str(env, jDirList);
    if (g_mapASService.find(taskID) != g_mapASService.end()) {
        ERRLOG("taskID:%s", taskID.c_str());
        return BuildRspObject(env, cls, taskID, mp_int32(ArchiveStreamErrType::ARCHIVE_EXCEPTION_SAME_ID_INITED));
    }

    ArchiveStreamService* pService = new ArchiveStreamService();
    g_mapASService.insert(std::make_pair(taskID, pService));
    mp_int32 iRet = pService->Init(backupId, taskID, dirList);
    return BuildRspObject(env, cls, taskID, (iRet == MP_FAILED) ?
        mp_int32(ArchiveStreamErrType::ARCHIVE_EXCEPTION_SDK_INNER_ERR) : iRet);
}

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
 * Method:    connect
 * Signature: (Ljava/lang/String;Ljava/lang/String;IZ)
 * Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/rsp/ArchiveConnectRsp;
 */
JNIEXPORT jobject JNICALL
Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility_connect
(JNIEnv *env, jobject, jstring jTaskID, jstring jBusiIp, jint jBusiPort, jboolean jOpenSsl)
{
    mp_string className = classNamePrefix + "ArchiveConnectRsp";
    jclass cls = env->FindClass(className.c_str());

    mp_string taskID = jstring2str(env, jTaskID);
    if (g_mapASService.find(taskID) == g_mapASService.end()) {
        ERRLOG("index:   taskID:%s", taskID.c_str());
        return BuildRspObject(env, cls, taskID, mp_int32(ArchiveStreamErrType::ARCHIVE_EXCEPTION_NOT_INIT));
    }
    mp_string busiIp = jstring2str(env, jBusiIp);
    mp_int32 busiPort = jBusiPort;
    bool openSsl = jOpenSsl == JNI_TRUE;
    mp_int32 iRet = g_mapASService[taskID]->Connect(busiIp, busiPort, openSsl);
    return BuildRspObject(env, cls, taskID, (iRet == MP_FAILED) ?
        mp_int32(ArchiveStreamErrType::ARCHIVE_EXCEPTION_SDK_INNER_ERR) : iRet);
}

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
 * Method:    prepareRecovery
 * Signature: (Ljava/lang/String;)
 * Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/rsp/ArchivePrepareRsp;
 */
JNIEXPORT jobject JNICALL
Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility_prepareRecovery
(JNIEnv *env, jobject, jstring jTaskID)
{
    mp_string className = classNamePrefix + "ArchivePrepareRsp";
    jclass cls = env->FindClass(className.c_str());

    mp_string taskID = jstring2str(env, jTaskID);
    if (g_mapASService.find(taskID) == g_mapASService.end()) {
        ERRLOG("taskID:%s", taskID.c_str());

        return BuildRspObject(env, cls, taskID, mp_int32(ArchiveStreamErrType::ARCHIVE_EXCEPTION_NOT_INIT));
    }
    mp_string metaFileDirl;
    mp_int32 iRet = g_mapASService[taskID]->PrepareRecovery(metaFileDirl);
    jobject objRet = BuildRspObject(env, cls, taskID, (iRet == MP_FAILED) ?
        mp_int32(ArchiveStreamErrType::ARCHIVE_EXCEPTION_SDK_INNER_ERR) : iRet);
    {
        jstring jStr = charTojstring(env, metaFileDirl.c_str());
        jfieldID fid = env->GetFieldID(cls, "metaFileDir", "Ljava/lang/String;");
        env->SetObjectField(objRet, fid, jStr);
    }
    return objRet;
}

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
 * Method:    queryPrepareStatus
 * Signature: (Ljava/lang/String;)
 * Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/rsp/ArchivePrepareStatusRsp;
 */
JNIEXPORT jobject JNICALL
Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility_queryPrepareStatus
(JNIEnv *env, jobject, jstring jTaskID)
{
    mp_string className = classNamePrefix + "ArchivePrepareStatusRsp";
    jclass cls = env->FindClass(className.c_str());

    mp_string taskID = jstring2str(env, jTaskID);
    if (g_mapASService.find(taskID) == g_mapASService.end()) {
        ERRLOG("taskID:%s", taskID.c_str());
        return BuildRspObject(env, cls, taskID, mp_int32(ArchiveStreamErrType::ARCHIVE_EXCEPTION_NOT_INIT));
    }
    mp_int32 state;
    mp_int32 iRet = g_mapASService[taskID]->QueryPrepareStatus(state);
    jobject objRet = BuildRspObject(env, cls, taskID, (iRet == MP_FAILED) ?
        mp_int32(ArchiveStreamErrType::ARCHIVE_EXCEPTION_SDK_INNER_ERR) : iRet);
    {
        jint nValue = state;
        jfieldID fid = env->GetFieldID(cls, "status", "I");
        env->SetIntField(objRet, fid, nValue);
    }
    return objRet;
}

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
 * Method:    getBackupInfo
 * Signature: (Ljava/lang/String;)
 * Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/rsp/ArchiveStreamCopyInfoRsp;
 */
JNIEXPORT jobject JNICALL
Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility_getBackupInfo
(JNIEnv *env, jobject, jstring jTaskID)
{
    mp_string className = classNamePrefix + "ArchiveStreamCopyInfoRsp";
    jclass cls = env->FindClass(className.c_str());

    mp_string taskID = jstring2str(env, jTaskID);
    if (g_mapASService.find(taskID) == g_mapASService.end()) {
        ERRLOG("taskID:%s", taskID.c_str());
        return BuildRspObject(env, cls, taskID, mp_int32(ArchiveStreamErrType::ARCHIVE_EXCEPTION_NOT_INIT));
    }
    ArchiveStreamCopyInfo copyInfo;
    mp_int32 iRet = g_mapASService[taskID]->GetBackupInfo(copyInfo);
    jobject objRet = BuildRspObject(env, cls, taskID, (iRet == MP_FAILED) ?
        mp_int32(ArchiveStreamErrType::ARCHIVE_EXCEPTION_SDK_INNER_ERR) : iRet);
    {
        jlong lValue = copyInfo.dirCount;
        jfieldID fid = env->GetFieldID(cls, "dirCount", "J");
        env->SetLongField(objRet, fid, lValue);
    }
    {
        jlong lValue = copyInfo.fileCount;
        jfieldID fid = env->GetFieldID(cls, "fileCount", "J");
        env->SetLongField(objRet, fid, lValue);
    }
    {
        jlong lValue = copyInfo.backupSize;
        jfieldID fid = env->GetFieldID(cls, "backupSize", "J");
        env->SetLongField(objRet, fid, lValue);
    }
    return objRet;
}

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
 * Method:    getRecoverObjectList
 * Signature: (Ljava/lang/String;JLjava/lang/String;)
 * Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/rsp/ArchiveRecoverObjectRsp;
 */
JNIEXPORT jobject JNICALL
Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility_getRecoverObjectList
(JNIEnv *env, jobject, jstring jTaskID, jlong jReadCountLimit, jstring jCheckpoint)
{
    mp_string className = classNamePrefix + "ArchiveRecoverObjectRsp";
    jclass cls = env->FindClass(className.c_str());

    mp_string taskID = jstring2str(env, jTaskID);
    if (g_mapASService.find(taskID) == g_mapASService.end()) {
        ERRLOG("taskID:%s", taskID.c_str());
        return BuildRspObject(env, cls, taskID, mp_int32(ArchiveStreamErrType::ARCHIVE_EXCEPTION_NOT_INIT));
    }
    mp_long readCountLimit = jReadCountLimit;
    mp_string checkpoint = jstring2str(env, jCheckpoint);
    mp_string splitFile;
    mp_int64 objectNum = 0;
    mp_int32 status = 0;
    mp_int32 iRet = g_mapASService[taskID]->GetRecoverObjectList(
        readCountLimit, checkpoint, splitFile, objectNum, status);
    jobject objRet = BuildRspObject(env, cls, taskID, (iRet == MP_FAILED) ?
        mp_int32(ArchiveStreamErrType::ARCHIVE_EXCEPTION_SDK_INNER_ERR) : iRet);
    {
        jstring jStr = charTojstring(env, checkpoint.c_str());
        jfieldID fid = env->GetFieldID(cls, "checkpoint", "Ljava/lang/String;");
        env->SetObjectField(objRet, fid, jStr);
    }
    {
        jstring jStr = charTojstring(env, splitFile.c_str());
        jfieldID fid = env->GetFieldID(cls, "splitFile", "Ljava/lang/String;");
        env->SetObjectField(objRet, fid, jStr);
    }
    {
        jlong lValue = objectNum;
        jfieldID fid = env->GetFieldID(cls, "objectNum", "J");
        env->SetLongField(objRet, fid, lValue);
    }
    {
        jint nValue = status;
        jfieldID fid = env->GetFieldID(cls, "status", "I");
        env->SetIntField(objRet, fid, nValue);
    }
    return objRet;
}

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
 * Method:    getDirMetaData
 * Signature: (Ljava/lang/String;Ljava/lang/String;)
 * Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/rsp/ArchiveFileMetaDataRsp;
 */
JNIEXPORT jobject JNICALL
Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility_getDirMetaData
(JNIEnv *env, jobject, jstring jTaskID, jstring jObjectName, jstring jFsID)
{
    mp_string className = classNamePrefix + "ArchiveFileMetaDataRsp";
    jclass cls = env->FindClass(className.c_str());

    mp_string taskID = jstring2str(env, jTaskID);
    if (g_mapASService.find(taskID) == g_mapASService.end()) {
        ERRLOG("taskID:%s", taskID.c_str());
        return BuildRspObject(env, cls, taskID, mp_int32(ArchiveStreamErrType::ARCHIVE_EXCEPTION_NOT_INIT));
    }
    mp_string ObjectName = jstring2str(env, jObjectName);
    mp_string fsID = jstring2str(env, jFsID);
    mp_string metaData;
    mp_int32 iRet = g_mapASService[taskID]->GetDirMetaData(ObjectName, fsID, metaData);
    jobject objRet = BuildRspObject(env, cls, taskID, (iRet == MP_FAILED) ?
        mp_int32(ArchiveStreamErrType::ARCHIVE_EXCEPTION_SDK_INNER_ERR) : iRet);
    {
        jstring jStr = charTojstring(env, metaData.c_str());
        jfieldID fid = env->GetFieldID(cls, "metaData", "Ljava/lang/String;");
        env->SetObjectField(objRet, fid, jStr);
    }
    return objRet;
}

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
 * Method:    getFileMetaData
 * Signature: (Ljava/lang/String;Ljava/lang/String;)
 * Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/rsp/ArchiveFileMetaDataRsp;
 */
JNIEXPORT jobject JNICALL
Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility_getFileMetaData
(JNIEnv *env, jobject, jstring jTaskID, jstring jObjectName, jstring jFsID)
{
    mp_string className = classNamePrefix + "ArchiveFileMetaDataRsp";
    jclass cls = env->FindClass(className.c_str());

    mp_string taskID = jstring2str(env, jTaskID);
    if (g_mapASService.find(taskID) == g_mapASService.end()) {
        ERRLOG("taskID:%s", taskID.c_str());
        return BuildRspObject(env, cls, taskID, mp_int32(ArchiveStreamErrType::ARCHIVE_EXCEPTION_NOT_INIT));
    }
    mp_string ObjectName = jstring2str(env, jObjectName);
    mp_string fsID = jstring2str(env, jFsID);
    mp_string metaData;
    mp_int32 iRet = g_mapASService[taskID]->GetFileMetaData(ObjectName, fsID, metaData);
    jobject objRet = BuildRspObject(env, cls, taskID, (iRet == MP_FAILED) ?
        mp_int32(ArchiveStreamErrType::ARCHIVE_EXCEPTION_SDK_INNER_ERR) : iRet);
    {
        jstring jStr = charTojstring(env, metaData.c_str());
        jfieldID fid = env->GetFieldID(cls, "metaData", "Ljava/lang/String;");
        env->SetObjectField(objRet, fid, jStr);
    }
    return objRet;
}


void JObjectToStruct(JNIEnv *env, const jobject& obj, ArchiveStreamGetFileReq& st)
{
    jclass cls = env->GetObjectClass(obj);
    {
        jfieldID fid = env->GetFieldID(cls, "taskID", "Ljava/lang/String;");
        jstring strValue = (jstring)env->GetObjectField(obj, fid);
        st.taskID = jstring2str(env, strValue);
    }
    {
        jfieldID fid = env->GetFieldID(cls, "archiveBackupId", "Ljava/lang/String;");
        jstring strValue = (jstring)env->GetObjectField(obj, fid);
        st.archiveBackupId = jstring2str(env, strValue);
    }
    {
        jfieldID fid = env->GetFieldID(cls, "fsID", "Ljava/lang/String;");
        jstring strValue = (jstring)env->GetObjectField(obj, fid);
        st.fsID = jstring2str(env, strValue);
    }
    {
        jfieldID fid = env->GetFieldID(cls, "filePath", "Ljava/lang/String;");
        jstring strValue = (jstring)env->GetObjectField(obj, fid);
        st.filePath = jstring2str(env, strValue);
    }
    {
        jfieldID fid = env->GetFieldID(cls, "fileOffset", "J");
        jlong lValue = env->GetLongField(obj, fid);
        st.fileOffset = lValue;
    }
    {
        jfieldID fid = env->GetFieldID(cls, "readSize", "I");
        jint nValue = env->GetIntField(obj, fid);
        st.readSize = nValue;
    }
    {
        jfieldID fid = env->GetFieldID(cls, "maxResponseSize", "I");
        jint nValue = env->GetIntField(obj, fid);
        st.maxResponseSize = nValue;
    }
}

void StructToJObject(const ArchiveStreamGetFileRsq& st, JNIEnv *env, jobject& obj)
{
    mp_string className = classNamePrefix + "ArchiveStreamGetFileRsp";
    jclass cls = env->FindClass(className.c_str());
    {
        jstring jStr = charTojstring(env, st.archiveBackupId.c_str());
        jfieldID fid = env->GetFieldID(cls, "archiveBackupId", "Ljava/lang/String;");
        env->SetObjectField(obj, fid, jStr);
    }
    {
        jstring jStr = charTojstring(env, st.filePath.c_str());
        jfieldID fid = env->GetFieldID(cls, "filePath", "Ljava/lang/String;");
        env->SetObjectField(obj, fid, jStr);
    }
    {
        jstring jStr = charTojstring(env, st.fsID.c_str());
        jfieldID fid = env->GetFieldID(cls, "fsID", "Ljava/lang/String;");
        env->SetObjectField(obj, fid, jStr);
    }
    {
        jlong lValue = st.offset;
        jfieldID fid = env->GetFieldID(cls, "offset", "J");
        env->SetLongField(obj, fid, lValue);
    }
    {
        jint nValue = st.fileSize;
        jfieldID fid = env->GetFieldID(cls, "fileSize", "I");
        env->SetIntField(obj, fid, nValue);
    }
    {
        jint nValue = st.readEnd;
        jfieldID fid = env->GetFieldID(cls, "readEnd", "I");
        env->SetIntField(obj, fid, nValue);
    }
    {
        jbyteArray jArr = env->NewByteArray(st.fileSize);
        env->SetByteArrayRegion(jArr, 0, st.fileSize, (jbyte*)st.data);
        jfieldID fid = env->GetFieldID(cls, "data", "[B");
        env->SetObjectField(obj, fid, jArr);
    }
}

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
 * Method:    getFileData
 * Signature: (Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/ArchiveStreamGetFileReq;)
 * Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/ArchiveStreamGetFileRsq;
 */
JNIEXPORT jobject JNICALL
Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility_getFileData
(JNIEnv *env, jobject, jstring jTaskID, jobject jGetFileReq)
{
    mp_string className = classNamePrefix + "ArchiveStreamGetFileRsp";
    jclass cls = env->FindClass(className.c_str());

    mp_string taskID = jstring2str(env, jTaskID);
    if (g_mapASService.find(taskID) == g_mapASService.end()) {
        ERRLOG("taskID:%s", taskID.c_str());
        return BuildRspObject(env, cls, taskID, mp_int32(ArchiveStreamErrType::ARCHIVE_EXCEPTION_NOT_INIT));
    }
    ArchiveStreamGetFileReq req;
    JObjectToStruct(env, jGetFileReq, req);
    ArchiveStreamGetFileRsq rsp;
    mp_int32 iRet = g_mapASService[taskID]->GetFileData(req, rsp);
    jobject objRet = BuildRspObject(env, cls, taskID, (iRet == MP_FAILED) ?
        mp_int32(ArchiveStreamErrType::ARCHIVE_EXCEPTION_SDK_INNER_ERR) : iRet);
    StructToJObject(rsp, env, objRet);
    if (rsp.data != nullptr) {
        free(rsp.data);
        rsp.data = nullptr;
    }
    return objRet;
}

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
 * Method:    endRecover
 * Signature: (Ljava/lang/String;)
 * Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/rsp/ArchiveEndRecoverRsp;
 */
JNIEXPORT jobject JNICALL
Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility_endRecover
(JNIEnv *env, jobject, jstring jTaskID)
{
    mp_string className = classNamePrefix + "ArchiveEndRecoverRsp";
    jclass cls = env->FindClass(className.c_str());

    mp_string taskID = jstring2str(env, jTaskID);
    if (g_mapASService.find(taskID) == g_mapASService.end()) {
        ERRLOG("taskID:%s", taskID.c_str());
        return BuildRspObject(env, cls, taskID, mp_int32(ArchiveStreamErrType::ARCHIVE_EXCEPTION_NOT_INIT));
    }
    mp_int32 iRet = g_mapASService[taskID]->EndRecover();
    return BuildRspObject(env, cls, taskID, (iRet == MP_FAILED) ?
        mp_int32(ArchiveStreamErrType::ARCHIVE_EXCEPTION_SDK_INNER_ERR) : iRet);
}

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility
 * Method:    disconnect
 * Signature: (Ljava/lang/String;)
 * Lcom/huawei/oceanprotect/sdk/agent/plugin/lib/archive/ability/definition/bean/rsp/ArchiveDisConnectRsp;
 */
JNIEXPORT jobject JNICALL
Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility_disconnect
(JNIEnv *env, jobject, jstring jTaskID)
{
    mp_string className = classNamePrefix + "ArchiveDisConnectRsp";
    jclass cls = env->FindClass(className.c_str());

    mp_string taskID = jstring2str(env, jTaskID);
    if (g_mapASService.find(taskID) == g_mapASService.end()) {
        ERRLOG("taskID:%s", taskID.c_str());
        return BuildRspObject(env, cls, taskID, mp_int32(ArchiveStreamErrType::ARCHIVE_EXCEPTION_NOT_INIT));
    }
    mp_int32 iRet = g_mapASService[taskID]->Disconnect();
    g_mapASService.erase(taskID);

    return BuildRspObject(env, cls, taskID, (iRet == MP_FAILED) ?
        mp_int32(ArchiveStreamErrType::ARCHIVE_EXCEPTION_SDK_INNER_ERR) : iRet);
}