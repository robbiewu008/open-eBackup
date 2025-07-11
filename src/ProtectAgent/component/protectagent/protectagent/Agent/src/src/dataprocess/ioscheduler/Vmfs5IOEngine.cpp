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
#include "dataprocess/ioscheduler/Vmfs5IOEngine.h"
#include "dataprocess/vmwarenative/Define.h"
#include "common/Utils.h"

namespace NSVmfsIO {
const mp_string VMDK_FLAT_FILE_KEY = "-flat";
const mp_string VMDK_FILE_POSTFIX = ".vmdk";

mp_int32 Vmfs5IOEngine::Open()
{
    if (m_protectType != VMWARE_VM_BACKUP) {
        COMMLOG(OS_LOG_ERROR, "vmfsio support backup job only.");
        return MP_FAILED;
    }

    if (SetDiskPath() != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Disk file resolve failed.");
        return MP_FAILED;
    }

    if (m_filename.empty()) {
        COMMLOG(OS_LOG_ERROR, "vmfsio open failed: invalid parameter.");
        return MP_FAILED;
    }

    if (VmfsIOLookup(m_filename.c_str()) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "vmfs lookup failed: %s", m_filename.c_str());
        return MP_FAILED;
    }
    m_file = Vmfs5IO::VmfsFile::Instance()->OpenFromBlkid(m_vmfs, Vmfs5IO::ino2blkid(m_ino));
    if (m_file <= 0) {
        COMMLOG(OS_LOG_ERROR, "Open file failed.");
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_INFO, "Open vmfs file success, file: %s, ino: %ld", m_filename.c_str(), m_ino);
    return MP_SUCCESS;
}

mp_int32 Vmfs5IOEngine::Read(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes, unsigned char* buffer)
{
    mp_int32 sz = 0;

    if (!m_file) {
        COMMLOG(OS_LOG_ERROR, "Bad file descriptor.");
        return MP_FAILED;
    }

    sz = Vmfs5IO::VmfsFile::Instance()->Read(m_file, (u_char *)buffer, bufferSizeInBytes, offsetInBytes);
    if (sz < 0) {
        COMMLOG(OS_LOG_ERROR, "Read file failed.");
        return MP_FAILED;
    }
    bufferSizeInBytes = sz;
    return MP_SUCCESS;
}

mp_int32 Vmfs5IOEngine::Close()
{
    if (!m_file) {
        COMMLOG(OS_LOG_WARN, "Bad file descriptor.");
        return MP_SUCCESS;
    }
    Vmfs5IO::VmfsFile::Instance()->Close(m_file);
    return MP_SUCCESS;
}

void Vmfs5IOEngine::Split(const std::string &str, const char split, std::vector<std::string> &res)
{
    if (str.empty()) {
        return;
    }
    std::string strs = str + split;
    size_t pos = strs.find(split);

    while (pos != strs.npos) {
        std::string temp = strs.substr(0, pos);
        res.push_back(temp);
        strs = strs.substr(pos + 1, strs.size());
        pos = strs.find(split);
    }
}

mp_int32 Vmfs5IOEngine::VmfsIOLookup(const char *name, bool checkOnly)
{
    if (!name) {
        COMMLOG(OS_LOG_ERROR, "vmfs device name invalid");
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "vmfsio lookup, name=%s", name);
    std::vector<std::string> items;
    Split(name, '/', items);
    Vmfs5IO::VmfsInoT parent = VMFS5_ROOT_INO;

    for (const auto &item : items) {
        Vmfs5IO::VmfsDirT *d = Vmfs5IO::VmfsDirent::Instance()->VmfsDirOpenFromBlkid(
            m_vmfs, Vmfs5IO::ino2blkid(parent));
        const Vmfs5IO::VmfsDirentT *rec = nullptr;

        if (!d) {
            COMMLOG(OS_LOG_ERROR, "open dir from blkid failed");
            return MP_FAILED;
        }

        struct stat attr;
        rec = Vmfs5IO::VmfsDirent::Instance()->VmfsDirLookup(d, item.c_str());
        if (rec && !Vmfs5IO::VmfsInode::Instance()->StatFromBlkid(m_vmfs, rec->blockId, &attr)) {
            attr.st_ino = Vmfs5IO::blkid2ino(rec->blockId);
            if (!checkOnly) {
                m_ino = attr.st_ino;
            }
            parent = attr.st_ino;
        } else {
            COMMLOG(OS_LOG_ERROR, "inode stat from blkid failed");
            Vmfs5IO::VmfsDirent::Instance()->VmfsDirClose(d);
            return MP_FAILED;
        }
        Vmfs5IO::VmfsDirent::Instance()->VmfsDirClose(d);
        COMMLOG(OS_LOG_DEBUG, "lookup %s, inode: %ld\n", item.c_str(), attr.st_ino);
    }

    COMMLOG(OS_LOG_DEBUG, "End - VmfsIOLookup");
    return MP_SUCCESS;
}

mp_int32 Vmfs5IOEngine::Exist(const std::string &file)
{
    if (file.empty()) {
        return MP_FAILED;
    }

    if (VmfsIOLookup(file.c_str(), true) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "vmfs lookup failed: %s", file.c_str());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 Vmfs5IOEngine::SetDiskPath()
{
    mp_string diskName;
    mp_string diskNameOrg;
    mp_string diskFileFlat;

    size_t found = m_volInfo.strDiskRelativePath.find(VMDK_FILE_POSTFIX);
    if (found == std::string::npos) {
        m_filename = m_volInfo.strDiskRelativePath;
        COMMLOG(OS_LOG_ERROR, "Disk file name has no .vmdk postfix");
        return MP_FAILED;
    }

    diskName = m_volInfo.strDiskRelativePath.substr(0, found);
    diskNameOrg = diskName + VMDK_FILE_POSTFIX;
    diskFileFlat = diskName + VMDK_FLAT_FILE_KEY + VMDK_FILE_POSTFIX;

    if (Exist(diskFileFlat) == MP_SUCCESS) {
        m_filename = diskFileFlat;
    } else if (Exist(diskNameOrg) == MP_SUCCESS) {
        m_filename = diskNameOrg;
    } else {
        COMMLOG(OS_LOG_ERROR, "Both disk file '%s' and '%s' not exist.",
            diskFileFlat.c_str(), diskNameOrg.c_str());
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_INFO, "Disk: %s", m_filename.c_str());
    return MP_SUCCESS;
}
}