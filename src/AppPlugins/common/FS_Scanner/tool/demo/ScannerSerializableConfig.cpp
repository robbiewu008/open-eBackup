#include "ScannerSerializableConfig.h"

#include "ScanMgr.h"
#include <algorithm>
#include <cstdio>
#include "JsonHelper.h"

using namespace Module;
using namespace Module::JsonHelper;

std::string UpperCase(const std::string& str)
{
    std::string strLower = str;
    std::transform(strLower.begin(), strLower.end(), strLower.begin(),
       [](unsigned char ch) { return ::tolower(ch); }
    );
    return strLower;
}

bool UpperEquals(const std::string& str1, const std::string& str2)
{
    std::string strLower1 = str1;
    std::transform(strLower1.begin(), strLower1.end(), strLower1.begin(),
        [](unsigned char ch) { return ::tolower(ch); }
    );
    std::string strLower2 = str2;
    std::transform(strLower2.begin(), strLower2.end(), strLower2.begin(),
        [](unsigned char ch) { return ::tolower(ch); }
    );
    return strLower1 == strLower2;
}

void SerializableDirFilter::DeserializeLiteralFieldsToOrigin() {
    if (UpperEquals(typeLiteral, "include")) {
        type = FILTER_TYPE::INCLUDE;
    } else if (UpperEquals(typeLiteral, "exclude")) {
        type = FILTER_TYPE::EXCLUDE;
    } else if (UpperEquals(typeLiteral, "disabled")) {
        type = FILTER_TYPE::DISABLED;
    } else {
        throw "unknown literal type of SerializableDirFilter.typeLiteral";
    }
}

void SerializableFileFilter::DeserializeLiteralFieldsToOrigin() {
    if (UpperEquals(typeLiteral, "include")) {
        type = FILTER_TYPE::INCLUDE;
    } else if (UpperEquals(typeLiteral, "exclude")) {
        type = FILTER_TYPE::EXCLUDE;
    } else if (UpperEquals(typeLiteral, "disabled")) {
        type = FILTER_TYPE::DISABLED;
    } else {
        throw "unknown literal type of SerializableDirFilter.typeLiteral";
    }
}

#ifdef _NAS
void SerializableSmbContextArgs::DeserializeLiteralFieldsToOrigin() {
    if (UpperEquals(authTypeLiteral, "NTLMSSP")) {
        authType = SmbAuthType::NTLMSSP;
    } else if (UpperEquals(authTypeLiteral, "KRB5")) {
        authType = SmbAuthType::KRB5;
    } else {
        throw "unknown literal type of SerializableSmbContextArgs.authTypeLiteral";
    }

    if (UpperEquals(versionLiteral, "VERSION0311")) {
        version = SmbVersion::VERSION0311;
    } else if (UpperEquals(versionLiteral, "VERSION0302")) {
        version = SmbVersion::VERSION0302;
    } else if (UpperEquals(versionLiteral, "VERSION0300")) {
        version = SmbVersion::VERSION0300;
    } else if (UpperEquals(versionLiteral, "VERSION0210")) {
        version = SmbVersion::VERSION0210;
    } else if (UpperEquals(versionLiteral, "VERSION0202")) {
        version = SmbVersion::VERSION0202;
    } else {
        throw "unknown literal type of SerializableSmbContextArgs.versionLiteral";
    }
}
#endif

void ScannerSerializableConfig::DeserializeLiteralFieldsToOrigin() {
    serializableDirFilter.DeserializeLiteralFieldsToOrigin();
    serializableFileFilter.DeserializeLiteralFieldsToOrigin();
#ifdef _NAS
    serializableSmbContextArgs.DeserializeLiteralFieldsToOrigin();
#endif

    if (UpperEquals(scanTypeLiteral, "FULL")) {
        scanType = ScanJobType::FULL;
    } else if (UpperEquals(scanTypeLiteral, "INC")) {
        scanType = ScanJobType::INC;
    } else if (UpperEquals(scanTypeLiteral, "RESTORE")) {
        scanType = ScanJobType::RESTORE;
    } else if (UpperEquals(scanTypeLiteral, "INDEX")) {
        scanType = ScanJobType::INDEX;
    } else if (UpperEquals(scanTypeLiteral, "ARCHIVE")) {
        scanType = ScanJobType::ARCHIVE;
    } else if (UpperEquals(scanTypeLiteral, "CONTROL_GEN")) {
        scanType = ScanJobType::CONTROL_GEN;
    } else if (UpperEquals(scanTypeLiteral, "RFI_GEN")) {
        scanType = ScanJobType::RFI_GEN;
    } else if (UpperEquals(scanTypeLiteral, "SNAPDIFFNAS_GEN")) {
        scanType = ScanJobType::SNAPDIFFNAS_GEN;
    } else {
        throw "unknown literal type of scanTypeLiteral";
    }

    if (UpperEquals(scanIOLiteral, "DEFAULT")) {
        scanIO = IOEngine::DEFAULT;
    } else if (UpperEquals(scanIOLiteral, "LIBNFS")) {
        scanIO = IOEngine::LIBNFS;
    } else if (UpperEquals(scanIOLiteral, "LIBSMB2")) {
        scanIO = IOEngine::LIBSMB2;
    } else if (UpperEquals(scanIOLiteral, "POSIX")) {
        scanIO = IOEngine::POSIX;
    } else if (UpperEquals(scanIOLiteral, "WIN32_IO")) {
        scanIO = IOEngine::WIN32_IO;
    } else if (UpperEquals(scanIOLiteral, "SNAPDIFFNAS")) {
        scanIO = IOEngine::SNAPDIFFNAS;
    } else if (UpperEquals(scanIOLiteral, "OBJECTSTORAGE")) {
        scanIO = IOEngine::OBJECTSTORAGE;
    } else {
        throw "unknown literal type of scanIOLiteral";
    }

    lastBackupTime = lastBackupTimeIntValue;
    triggerTime = triggerTimeIntValue;

    if (UpperEquals(nasSnapdiffProtocolLiteral, "NFS")) {
        nasSnapdiffProtocol = NAS_PROTOCOL::NFS;
    } else if (UpperEquals(nasSnapdiffProtocolLiteral, "SMB")) {
        nasSnapdiffProtocol = NAS_PROTOCOL::SMB;
    } else {
        throw "unknown literal type of nasSnapdiffProtocolLiteral";
    }

    if (UpperEquals(metadataScanTypeLiteral, "METABACKUP_TYPE_ERR")) {
        metadataScanType = METABACKUP_TYPE_ERR;
    } else if (UpperEquals(metadataScanTypeLiteral, "METABACKUP_TYPE_FOLDER_ONLY")) {
        metadataScanType = METABACKUP_TYPE_FOLDER_ONLY;
    } else if (UpperEquals(metadataScanTypeLiteral, "METABACKUP_TYPE_FILE_AND_FOLDER")) {
        metadataScanType = METABACKUP_TYPE_FILE_AND_FOLDER;
    } else {
        throw "unknown literal type of metadataScanType";
    }

    dFilter = static_cast<ScanDirectoryFilter>(serializableDirFilter);
    fFilter = static_cast<ScanFileFilter>(serializableFileFilter);

#ifdef _NAS
    nfs = static_cast<LibnfsConfig>(serializableLibNfsConfig);
    smb = static_cast<SmbContextArgs>(serializableSmbContextArgs);
#endif
}

void ScannerSerializableConfig::PrintScanConfig()
{
    ::printf("==== Scanner Config ==== ");
    ::printf("jobId: %s\n", jobId.c_str());
    ::printf("subJobId: %s\n", subJobId.c_str());
    ::printf("copyId: %s\n", copyId.c_str());
    ::printf("reqId: %lu\n", reqID);
    ::printf("scanType: %u\n", scanType);
    ::printf("IOEngine: %u\n", scanIO);
    ::printf("metaPath: %s\n", metaPath.c_str());
    ::printf("metaPathForCtrlFiles: %s\n", metaPathForCtrlFiles.c_str());
    ::printf("curDcachePath: %s\n", curDcachePath.c_str());
    ::printf("prevDcachePath: %s\n", prevDcachePath.c_str());
    ::printf("indexPath: %s\n", indexPath.c_str());
    ::printf("lastBackupTime: %lu\n", lastBackupTime);
    ::printf("useLastBackupTime: %u\n", useLastBackupTime);
    ::printf("scanCheckPointEnable: %u\n", scanCheckPointEnable);
    ::printf("metadataScanType: %u\n", metadataScanType);
    ::printf("scanSparseFile: %u\n", scanSparseFile);
    ::printf("scanExtendAttribute: %u\n", scanExtendAttribute);
    ::printf("scanAcl: %u\n", scanAcl);
    ::printf("isGetDirExtAcl: %u\n", isGetDirExtAcl);
    ::printf("isGetFileExtAcl: %u\n", isGetFileExtAcl);
    ::printf("maxCommonServiceInstance: %u\n", maxCommonServiceInstance);
    ::printf("scanCopyCtrlFileSize: %d\n", scanCopyCtrlFileSize);
    ::printf("scanCtrlMaxDataSize: %s\n", scanCtrlMaxDataSize.c_str());
    ::printf("scanCtrlMinDataSize: %s\n", scanCtrlMinDataSize.c_str());
    ::printf("scanCtrlFileTimeSec: %u\n", scanCtrlFileTimeSec);
    ::printf("scanCtrlMaxEntriesFullBkup: %u\n", scanCtrlMaxEntriesFullBkup);
    ::printf("scanCtrlMinEntriesFullBkup: %u\n", scanCtrlMinEntriesFullBkup);
    ::printf("scanCtrlMaxEntriesIncBkup: %u\n", scanCtrlMaxEntriesIncBkup);
    ::printf("scanCtrlMinEntriesIncBkup: %u\n", scanCtrlMinEntriesIncBkup);
    ::printf("scanMtimeCtrlMaxEnties: %u\n", scanMtimeCtrlMaxEnties);
    ::printf("scanDeleteCtrlMaxEnties: %u\n", scanDeleteCtrlMaxEnties);
    ::printf("scanMetaFileSize: %lu\n", scanMetaFileSize);
    ::printf("maxWriteQueueSize: %lu\n", maxWriteQueueSize);
    ::printf("minScanQueueSize: %lu\n", minScanQueueSize);
    ::printf("maxScanQueueSize: %lu\n", maxScanQueueSize);
    ::printf("triggerTime: %lu\n", triggerTime);
    ::printf("generatorIsFull: %u\n", generatorIsFull);
    ::printf("maxServerRetryCnt: %u\n", maxServerRetryCnt);
    ::printf("writeQueueSize: %lu\n", writeQueueSize);
    ::printf("dirEntryReadCount: %u\n", dirEntryReadCount);
    ::printf("producerThreadCount: %u\n", producerThreadCount);

#ifdef _NAS
    if (scanIO == IOEngine::LIBNFS) {
        ::printf("nfs.serviceIp: %s\n", nfs.m_serverIp.c_str());
        ::printf("nfs.serverPath: %s\n", nfs.m_serverPath.c_str());
        ::printf("nfs.nasServerCheckSleepTime: %u\n", nfs.m_nasServerCheckSleepTime);
        ::printf("nfs.contextCount: %u\n", nfs.m_contextCount);
        ::printf("nfs.macOpendirReqCnt: %u\n", nfs.maxOpendirReqCount);
    }
#endif

    ::printf("dFilter.type: %d\n", dFilter.type);
    for (const std::string& filterPolicy: dFilter.dirList) {
        ::printf("dFilterPolicyItem: %s\n", filterPolicy.c_str());
    }
    ::printf("fFilter.type: %d\n", fFilter.type);
    for (const std::string& filterPolicy: fFilter.fileList) {
        ::printf("fFilterPolicyItem: %s\n", filterPolicy.c_str());
    }
}
