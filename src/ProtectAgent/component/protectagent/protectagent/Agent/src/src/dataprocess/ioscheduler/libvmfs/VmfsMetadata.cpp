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
#include <cstring>
#include <sys/stat.h>
#include "dataprocess/ioscheduler/libvmfs/Vmfs.h"


namespace Vmfs5IO {
VmfsMetadata *VmfsMetadata::m_instance = nullptr;
std::mutex VmfsMetadata::m_mutex;

VmfsMetadata *VmfsMetadata::Instance()
{
    if (m_instance == nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_instance == nullptr) {
            m_instance = new (std::nothrow) VmfsMetadata();
        }
    }
    return m_instance;
}

/* Read a metadata header */
int VmfsMetadata::HdrRead(VmfsMetadataHdrT *mdh, const u_char *buf)
{
    mdh->magic = ReadLE32(buf, VMFS5_MDH_OFFSET_MAGIC);
    mdh->pos = ReadLE64(buf, VMFS5_MDH_OFFSET_POS);
    mdh->hbPos = ReadLE64(buf, VMFS5_MDH_OFFSET_HB_POS);
    mdh->hbSeq = ReadLE64(buf, VMFS5_MDH_OFFSET_HB_SEQ);
    mdh->objSeq = ReadLE64(buf, VMFS5_MDH_OFFSET_OBJ_SEQ);
    mdh->hbLock = ReadLE32(buf, VMFS5_MDH_OFFSET_HB_LOCK);
    mdh->mtime = ReadLE64(buf, VMFS5_MDH_OFFSET_MTIME);
    ReadUuid(buf, VMFS5_MDH_OFFSET_HB_UUID, &mdh->hbUuid);
    return (0);
}

/* Write a metadata header */
int VmfsMetadata::HdrWrite(const VmfsMetadataHdrT *mdh, u_char *buf)
{
    int ret = memset_s(buf, VMFS5_METADATA_HDR_SIZE, 0, VMFS5_METADATA_HDR_SIZE);
    if (ret != 0) {
        return (-1);
    }
    WriteLE32(buf, VMFS5_MDH_OFFSET_MAGIC, mdh->magic);
    WriteLE64(buf, VMFS5_MDH_OFFSET_POS, mdh->pos);
    WriteLE64(buf, VMFS5_MDH_OFFSET_HB_POS, mdh->hbPos);
    WriteLE64(buf, VMFS5_MDH_OFFSET_HB_SEQ, mdh->hbSeq);
    WriteLE64(buf, VMFS5_MDH_OFFSET_OBJ_SEQ, mdh->objSeq);
    WriteLE32(buf, VMFS5_MDH_OFFSET_HB_LOCK, mdh->hbLock);
    WriteLE64(buf, VMFS5_MDH_OFFSET_MTIME, mdh->mtime);
    WriteUuid(buf, VMFS5_MDH_OFFSET_HB_UUID, &mdh->hbUuid);
    return (0);
}
}