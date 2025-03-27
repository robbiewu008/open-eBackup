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
#ifndef DISK_SCANNER_HANDLER_H
#define DISK_SCANNER_HANDLER_H
#include <mutex>
#include <vector>
#include <string>
#include <sstream>
#include <set>
#include <map>
#include <memory>
#include <cstdlib>
#include <unistd.h>
#include <cstddef>
#include <scsi/sg.h>
#include <scsi/scsi_ioctl.h>
#include <scsi/scsi.h>
#include <sys/vfs.h>
#include <sys/sysinfo.h>
#include <sys/syslog.h>
#include <iconv.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <cctype>
#include <fcntl.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <functional>
#include <linux/raw.h>
#include <linux/major.h>
#include "volume_handlers/common/StorageDataType.h"
#include "common/Macros.h"
#include "common/openstorage_api_client/api/OpenStorageApiClient.h"
#include "volume_handlers/VolumeHandler.h"
#include "security/cmd/CmdParam.h"
#include "security/cmd/CmdExecutor.h"

namespace VirtPlugin {
class DiskScannerHandler {
public:
    static DiskScannerHandler *GetInstance();
    std::string GetHostUuid() const;
    int32_t GetIscsiInitor(std::string &iqnNumber);
    bool IsInstallIscsiInitiator();
    int32_t TestConnect(const std::string &targetIP);
    int32_t LoginIscsiTarget(const std::string &targetIP);
    int32_t GetFCInitor(std::vector<std::string> &vecWWPN);
    int32_t GetLoginedTargetIP(std::vector<std::string> &vecTargetIP);
    int32_t DoScanAfterAttach(const std::string &volId, const std::string &volWwn, std::string &objPath);
    int32_t DetachVolume(const std::string &volId, const std::string &volWwn);
    int32_t DetachVolumeInner(const std::string &diskPath);
    int32_t DeleteDiskFromPathSet(std::set<std::string>& diskPathSet, const std::string& volId,
        const std::string& volWwn, bool& isExist);
    int32_t GetDiskPathInner(std::map<std::string, std::set<std::string>>& diskPathMap,
        const std::string& volWwn, std::string& diskPath, const bool& ultraPathNotInstalled, bool& isLoop);
    int32_t GetDiskName(std::string& diskPath, std::string& strDiskName, const std::string& strDiskPath);
    void RescanDevice(const std::string& diskPath);
    int32_t DeleteAndScanHost(const std::string& strDiskName);
    std::string FormatBaseUrl(const std::string &baseUrl) const;
    std::string ScanDiskPreare(const std::string& volId, const std::string& volWwn);
    std::string GetDiskPathForWWN(const std::string& volWwn);
    std::string GetDiskPath(const std::string& volId, const std::string& volWwn);
    bool ChangeFilePriviledge(const std::string &file, const VolOpenMode &mode);
    int32_t GetDisk83Page(const std::string& strDevice, std::string& strLunWWN, std::string& strLunID);
    int32_t GetDiskPathMap(std::map<std::string, std::set<std::string>>& diskPathMap);
    int32_t GetDiskPage(int iFd, unsigned char ucCmd, unsigned char* aucBuffer);
    int32_t BinaryToAscii(char* pszHexBuffer, int iBufferLen, unsigned char* pucBuffer, int iStartBuf, int iLength);
    int32_t ConvertLunIdToAscii(unsigned char* puszAsciiLunID, int iBufferLen, unsigned char* puszLunLunID, int iLen);
    int32_t HextoDec(char* pDecStr, char* pHexStr, int iMaxLen);
    int32_t HexEncode(char cHex, int iStep, int& iDecNum);
    int32_t CalStep(int iStep) const;
    int32_t DiscoveryIscsiTarget(const std::string &targetIP, std::string &iqnAddress);
    int32_t LogOutTarget(const std::string &targetIp);
    int32_t GetIscsiSessionStatus(std::vector<IscsiSessionStatus> &sessionStatusList);
    int32_t RunShellCmd(const std::string& cmd, const std::vector<std::string> &paramList,
        std::vector<std::string> &cmdOut);
    int32_t RunShellCmd(const std::string& cmd, const std::vector<std::string> &paramList,
        std::vector<std::string> &cmdOut, std::vector<std::string> &errOut);

    // 适配内置，和分布式共用
    int32_t GetHostSN(std::string &hostSN);
    // 通过查找文件找到路径，在安全容器中不支持使用/dev/disk/相关的udev进程
    std::string GetDiskPathForWwnInSysDir(const std::string &volWwn);
    // 查找所有的盘符，拿到纯净路径
    int32_t GetAllDiskLetters(std::set<std::string> &diskLetters);
    // 在/sys/class/block/sdk/device/wwid 文件下找到WWN
    int32_t GetIscsiDiskWwid(const std::string &diskLetter, std::string &diskWwn);
    // 扫描session
    void RescanDevice();
    int32_t AddTypeIpRulePolicy(const std::string &targetIP, const std::string &targetType);
    int32_t AddIscsiLogicIpRoutePolicy(const std::string &targetIP);

private:
    DiskScannerHandler();
    virtual ~DiskScannerHandler();
    int32_t GetHostUuid(std::string &hostUuid);
    void DeleteScsiAndScanHost(const std::string &strHost, const std::string &strHostFirst);

private:
    std::mutex m_Mutext {};
    static std::string m_hostUuid;
    bool m_isInternalScence = false;
};
}
#endif