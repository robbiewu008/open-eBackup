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
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <sys/types.h>
#include "dataprocess/ioscheduler/libvmfs/Vmfs.h"


namespace Vmfs5IO {
VmfsHeartbeat *VmfsHeartbeat::m_instance = nullptr;
std::mutex VmfsHeartbeat::m_mutex;

VmfsHeartbeat *VmfsHeartbeat::Instance()
{
    if (m_instance == nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_instance == nullptr) {
            m_instance = new (std::nothrow) VmfsHeartbeat();
        }
    }
    return m_instance;
}

/* Write a heartbeat info */
int VmfsHeartbeat::Write(const VmfsHeartbeatT *hb, u_char *buf)
{
    WriteLE32(buf, VMFS5_HB_OFFSET_MAGIC, hb->magic);
    WriteLE64(buf, VMFS5_HB_OFFSET_POS, hb->pos);
    WriteLE64(buf, VMFS5_HB_OFFSET_SEQ, hb->seq);
    WriteLE64(buf, VMFS5_HB_OFFSET_UPTIME, hb->uptime);
    WriteLE32(buf, VMFS5_HB_OFFSET_JOURNAL_BLK, hb->journalBlk);
    WriteUuid(buf, VMFS5_HB_OFFSET_UUID, &hb->uuid);
    return (0);
}

/* Unlock an heartbeat */
int VmfsHeartbeat::Unlock(VmfsFsT *fs, VmfsHeartbeatT *hb)
{
    DECL_ALIGNED_BUFFER(buf, VMFS5_HB_SIZE);

    if (!Active(hb)) {
        return (-1);
    }

    hb->magic = VMFS5_HB_MAGIC_OFF;
    hb->seq++;
    uuid_clear(hb->uuid);
    Write(hb, buf);

    return ((VmfsDeviceWrite(fs->dev, hb->pos, buf, buf_len) == buf_len) ? 0 : -1);
}
}