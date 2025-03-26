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
#ifndef APPLICATION_SERVICE_DATA_TYPE_H
#define APPLICATION_SERVICE_DATA_TYPE_H
#include <string>
#include <locale>
#include <codecvt>
#include "Module/src/common/JsonHelper.h"
#include "common/CleanMemPwd.h"
namespace FilePlugin {
    const int PARTITION_TYPE_SYSTEM = 1;    // 操作系统分区
    const int PARTITION_TYPE_BOOT = 2;    // 启动分区
    const int PARTITION_TYPE_RECOVERY = 3;    // 恢复分区
    const int PARTITION_TYPE_DATA = 4;    // 数据分区

    struct NasShareExtendInfo {
        std::string ip;
        std::string kerberosId;
        std::string shareMode;
        std::string encryption;
        std::string authMode;
        std::string domainName;
        std::string isAutoScan;
        std::string passWord;
        std::string userName;
        std::string filters;
        ~NasShareExtendInfo()
        {
            Module::CleanMemoryPwd(passWord);
        }
        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(ip, ip)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(kerberosId, kerberosId)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(shareMode, shareMode)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(encryption, encryption)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(authMode, authMode)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(domainName, domainName)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(isAutoScan, isAutoScan)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(passWord, passWord)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(userName, userName)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(filters, filters)
        END_SERIAL_MEMEBER
    };

    struct NasShareResourceInfo {
        std::string path;
        std::string modifyTime;
        long size {0};
        std::string type;
        bool hasChildren {false};
        // only used for volume resource access, separated by ","
        std::string volumeMountPoints {};
        
        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(path, path)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(modifyTime, modifyTime)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(size, size)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(type, type)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(hasChildren, hasChildren)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeMountPoints, volumeMountPoints)
        END_SERIAL_MEMEBER
    };

    struct FileDiskResourceInfo {
        std::string diskId;
        std::string diskName;
        uint64_t diskSize;
        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(diskId, diskId)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(diskName, diskName)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(diskSize, diskSize)
        END_SERIAL_MEMEBER
    };

#ifdef WIN32
    
    struct WinVolumeInfo {
        bool isHealthy;
        bool isQuery;
        int partitionType;
        uint32_t partitionNumber;
        uint32_t volumeType;
        ULONGLONG totalSize;
        ULONGLONG freeSpace;
        ULONGLONG partitionOffset;
        ULONGLONG partitionLength;
        std::wstring volumeName;
        std::wstring label;
        std::wstring fileSystem;
        std::wstring volumeSerialNumber;
        std::wstring driveType;
        std::wstring driveLetter;
        std::wstring drivePath;
        std::wstring partitionName;
        std::wstring partitionGuid;
        std::wstring partitionNameType;
        std::wstring isBackupable;
    };
    
   
    struct StringVolumeInfo {
        bool isHealthy;
        int partitionType;
        uint32_t partitionNumber;
        uint32_t volumeType;
        ULONGLONG totalSize;
        ULONGLONG freeSpace;
        ULONGLONG partitionOffset;
        ULONGLONG partitionLength;
        std::string volumeName;
        std::string label;
        std::string fileSystem;
        std::string volumeSerialNumber;
        std::string driveType;
        std::string driveLetter;
        std::string displayName;
        std::string drivePath;
        std::string partitionName;
        std::string partitionGuid;
        std::string partitionNameType;
        std::string isBackupable;
        
        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeName, volumeName)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(label, label)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(fileSystem, fileSystem)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(totalSize, totalSize)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(freeSpace, freeSpace)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(isHealthy, isHealthy)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeSerialNumber, volumeSerialNumber)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(driveType, driveType)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(drivePath, drivePath)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(driveLetter, driveLetter)
		SERIAL_MEMBER_TO_SPECIFIED_NAME(displayName, displayName)
		SERIAL_MEMBER_TO_SPECIFIED_NAME(partitionName, partitionName)
		SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeType, volumeType)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(partitionType, partitionType)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(isBackupable, isBackupable)
        END_SERIAL_MEMEBER
    };
#endif

    struct FileResourceInfo {
        int totalCount {0};   // 总共资源数量
#ifdef __linux__
        std::vector<NasShareResourceInfo> volumeResourceDetailVec;
#elif defined(WIN32)
        std::vector<StringVolumeInfo> volumeResourceDetailVec;
#endif
        std::vector<NasShareResourceInfo> resourceDetailVec;
        std::vector<FileDiskResourceInfo> diskResourceDetailVec;
    };

    struct NasShareAuthInfo {
        // AuthType::type authType; //authType=0 无认证  authType=2 用户名和密码认证  authType=5 kerberos认证
        std::string authType;
        std::string authKey;
        std::string authPwd;
        ~NasShareAuthInfo()
        {
            Module::CleanMemoryPwd(authPwd);
        }
        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(authType, authType)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(authKey, authKey)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(authPwd, authPwd)
        END_SERIAL_MEMEBER
    };

    struct ResourceExendInfo {
        std::string protocol;
        std::string fileType; // value:native or aggregate
        std::string serviceIp;
        std::string domainName;
        std::string directory;
        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(protocol, protocol)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(fileType, fileType)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(serviceIp, ip)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(domainName, domainName)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(directory, directory)
        END_SERIAL_MEMEBER
    };

   
    struct NasAuthExtentInfo {
        std::string krb5Conf;
        std::string password;
        std::string keytab;
        std::string configMode;
        ~NasAuthExtentInfo()
        {
            Module::CleanMemoryPwd(password);
        }
        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(krb5Conf, krb5Conf)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(password, secret)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(keytab, keytab)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(configMode, configModel)
        END_SERIAL_MEMEBER
    };

    struct NfsAuthInfo {
        std::string kerberos;
        std::string mode;

        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(kerberos, kerberos)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(mode, mode)
        END_SERIAL_MEMEBER
    };

    struct CifsAuthInfo {
        std::string domainName;
        std::string mode;
        std::string username;
        std::string password;
        std::string kerberos;
        ~CifsAuthInfo()
        {
            Module::CleanMemoryPwd(password);
        }
        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(domainName, domainName)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(mode, mode)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(username, username)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(password, password)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(kerberos, kerberos)
        END_SERIAL_MEMEBER
    };

    struct ListApplicationShareInfoReq {
        std::string fileSystemName;
        std::string location;
        std::string shareIp;
        std::string name;
        std::string protocol;
        std::string targetType;
        NfsAuthInfo nfsAuth;
        CifsAuthInfo cifsAuth;
        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(fileSystemName, fileSystemName)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(location, location)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(shareIp, shareIp)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(name, name)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(protocol, protocol)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(targetType, targetType)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(nfsAuth, nfsAuth)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(cifsAuth, cifsAuth)
        END_SERIAL_MEMEBER
    };

    struct ListApplicationShareInfo {
        ListApplicationShareInfoReq shareInfoReq;
        int operation {0};
        std::string path;
        std::string resourceType;
        int pageNo {0};
        int pageSize {0};
        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(shareInfoReq, shareInfoReq)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(operation, operation)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(path, path)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(resourceType, resourceType)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(pageNo, pageNo)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(pageSize, pageSize)
        END_SERIAL_MEMEBER
    };

    struct ApplicationResourceExtendInfo {
        std::string nasAgentReq;

        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(nasAgentReq, nasAgentReq)
        END_SERIAL_MEMEBER
    };

    struct ListResourceParam {
        std::string applicationId;
        std::string sharePath;
        std::string path;
        int pageNo {0};
        int pageSize {0};
        ResourceExendInfo resourceExtendInfo {};
        NasShareAuthInfo nasShareAuthInfo {};
        NasAuthExtentInfo nasAuthExtentInfo {};
    };

    struct ListCifsResourceParams {
        std::string user;
        std::string password;
        std::string sharePath;
        std::string ip;
        std::string path;
        int pageNo {0};
        int pageSize {0};
        std::string mode;
        std::string applicationId;
        ~ListCifsResourceParams()
        {
            Module::CleanMemoryPwd(password);
        }
    };
}

#endif