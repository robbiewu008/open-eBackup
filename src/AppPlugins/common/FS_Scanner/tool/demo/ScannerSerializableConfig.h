/*
* Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
* Author: w30029850
* Create: 2023-01-21
*/

#ifndef WIN32_SCANNER_DEMO_CONFIG_H
#define WIN32_SCANNER_DEMO_CONFIG_H

#include "ScanConfig.h"
#include "JsonHelper.h"

/*
 * {
 *     "type": "include" | "exclude" | "disabled",
 *     "fileList": ["/dir/file1", "/dir2/file2", ...],
 * }
 */
struct SerializableDirFilter: public ScanDirectoryFilter {
    std::string typeLiteral; /* include | exclude | disabled */

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(typeLiteral, type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(dirList, dirList)
    END_SERIAL_MEMEBER

    void DeserializeLiteralFieldsToOrigin();
};

/*
 * {
 *     "type": "include" | "exclude" | "disabled",
 *     "dirList": ["/dir/file1", "/dir2/file2", ...],
 * }
 */
struct SerializableFileFilter: public ScanFileFilter {
    std::string typeLiteral; /* include | exclude | disabled */

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(typeLiteral, type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(fileList, fileList)
    END_SERIAL_MEMEBER

    void DeserializeLiteralFieldsToOrigin();
};

#ifdef _NAS
/*
 * {
 *     "serverIp": "192.168.129.170",
 *     "serverPath": "/nfsshare1/"
 *     "nasServerCheckSleepTime": 3,
 *     "contextCount": 8,
 *     "maxOpendirReqCount": 4000,
 * }
 */
struct SerializableLibNfsConfig: public LibnfsConfig {
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_serverIp, serverIp)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_serverPath, serverPath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_nasServerCheckSleepTime, nasServerCheckSleepTime)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_contextCount, contextCount)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(maxOpendirReqCount, maxOpendirReqCount)
    END_SERIAL_MEMEBER
};

struct SerializableSmbContextArgs: public SmbContextArgs {
    std::string authTypeLiteral; /* NTLMSSP | KRB5 */
    std::string versionLiteral; /* VERSION0311 | VERSION0302 | VERSION0300 | VERSION0210 | VERSION0202 */

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(domain, domain)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(server, server)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(share, share)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(user, user)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(password, password)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(krb5CcacheFile, krb5CcacheFile)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(krb5ConfigFile, krb5ConfigFile)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(encryption, encryption)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(sign, sign)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(timeout, timeout)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(authTypeLiteral, authType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(versionLiteral, version)
    END_SERIAL_MEMEBER

    void DeserializeLiteralFieldsToOrigin();
};
#endif

struct ScannerSerializableConfig: public ScanConfig {

    /* Literal fields or serialable wrapper */
    std::string scanTypeLiteral {}; /* FULL | INC | RESTORE | INDEX | ARCHIVE | CONTROL_GEN | RFI_GEN | SNAPDIFFNAS_GEN */
    std::string scanIOLiteral {}; /* DEFAULT | LIBNFS | LIBSMB2 | POSIX | WIN32_IO | SNAPDIFFNAS */
    uint64_t lastBackupTimeIntValue {};
    std::string metadataScanTypeLiteral {}; /* METABACKUP_TYPE_ERR | METABACKUP_TYPE_FOLDER_ONLY | METABACKUP_TYPE_FILE_AND_FOLDER */
    std::string nasSnapdiffProtocolLiteral {}; /* NFS | SMB */
    uint64_t triggerTimeIntValue{};

    /* wrapper fields that used for filter */
    SerializableDirFilter serializableDirFilter;
    SerializableFileFilter serializableFileFilter;

#ifdef _NAS
    /* wrapper fields that used for NAS */
    SerializableLibNfsConfig serializableLibNfsConfig;
    SerializableSmbContextArgs serializableSmbContextArgs;
#endif

    void DeserializeLiteralFieldsToOrigin();
    void PrintScanConfig();

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(enableV2, enableV2)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(jobId, jobId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(subJobId, subJobId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(copyId, copyId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(reqID, reqID)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(metaPath, metaPath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(failureRecordRootPath, failureRecordRootPath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(metaPathForCtrlFiles, metaPathForCtrlFiles)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(curDcachePath, curDcachePath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(prevDcachePath, prevDcachePath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(indexPath, indexPath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(useLastBackupTime, useLastBackupTime)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanCheckPointEnable, scanCheckPointEnable)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(isLastScan, isLastScan)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(isPre, isPre)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanSparseFile, scanSparseFile)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanExtendAttribute, scanExtendAttribute)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanAcl, scanAcl)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(disableSmbAcl, disableSmbAcl)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(disableSmbNlink, disableSmbNlink)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(genDirOnly, genDirOnly)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanByRoot, scanByRoot)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(enableProduce, enableProduce)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(isGetDirExtAcl, isGetDirExtAcl)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(isGetFileExtAcl, isGetFileExtAcl)

    SERIAL_MEMBER_TO_SPECIFIED_NAME(maxCommonServiceInstance, maxCommonServiceInstance)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanCtrlMaxDataSize, scanCtrlMaxDataSize)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanCtrlMinDataSize, scanCtrlMinDataSize)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanCtrlFileTimeSec, scanCtrlFileTimeSec)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanCopyCtrlFileSize, scanCopyCtrlFileSize)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanCtrlMaxEntriesFullBkup, scanCtrlMaxEntriesFullBkup)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanCtrlMinEntriesFullBkup, scanCtrlMinEntriesFullBkup)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanCtrlMaxEntriesIncBkup, scanCtrlMaxEntriesIncBkup)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanCtrlMinEntriesIncBkup, scanCtrlMinEntriesIncBkup)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanMtimeCtrlMaxEnties, scanMtimeCtrlMaxEnties)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanDeleteCtrlMaxEnties, scanDeleteCtrlMaxEnties)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanMetaFileSize, scanMetaFileSize)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(maxWriteQueueSize, isGetFileExtAcl)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(maxScanQueueSize, maxScanQueueSize)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(minScanQueueSize, minScanQueueSize)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(writeQueueSize, writeQueueSize)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(dirEntryReadCount, dirEntryReadCount)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(generatorIsFull, generatorIsFull)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(maxServerRetryCnt, maxServerRetryCnt)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(diffThreadCount, diffThreadCount)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(producerThreadCount, producerThreadCount)

    /* fields that used for filter */
    SERIAL_MEMBER_TO_SPECIFIED_NAME(serializableDirFilter, dFilter)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(serializableFileFilter, fFilter)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(dCtrlFltr, dCtrlFltr)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(fCtrlFltr, fCtrlFltr)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(crossVolumeSkipSet, crossVolumeSkipSet)

    /* fields that used for V5 snapdiff */
    SERIAL_MEMBER_TO_SPECIFIED_NAME(deviceResourceName, deviceResourceName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(deviceUrl, deviceUrl)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(devicePort, devicePort)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(deviceUsername, deviceUsername)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(devicePassword, devicePassword)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(deviceCert, deviceCert)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(devicePoolID, devicePoolID)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(baseSnapshotName, baseSnapshotName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(incSnapshotName, incSnapshotName)

    SERIAL_MEMBER_TO_SPECIFIED_NAME(maxOpendirReqCount, maxOpendirReqCount)

    /* literal or wrapper fields */
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanTypeLiteral, scanType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanIOLiteral, scanIO)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(lastBackupTimeIntValue, lastBackupTime)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(metadataScanTypeLiteral, metadataScanType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(nasSnapdiffProtocolLiteral, nasSnapdiffProtocol)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(triggerTimeIntValue, triggerTime)
#ifdef _NAS
    SERIAL_MEMBER_TO_SPECIFIED_NAME(serializableLibNfsConfig, nfs)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(serializableSmbContextArgs, smb)
#endif
    END_SERIAL_MEMEBER

};

#endif