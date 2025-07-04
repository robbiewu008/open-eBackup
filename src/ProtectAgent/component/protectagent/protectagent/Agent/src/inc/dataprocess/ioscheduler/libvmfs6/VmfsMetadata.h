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
#ifndef __VMFS6_METADATA_H__
#define __VMFS6_METADATA_H__

#include <stddef.h>

#define VMFS6_METADATA_HDR_SIZE 4096


namespace Vmfs6IO {
struct VmfsMetadataHdrRaw {
    uint32_t magic;  /* Magic number */
    uint64_t pos;    /* Position in the volume */
    uint64_t hbPos;  /* Heartbeat position */
    uint64_t hbSeq;  /* Heartbeat sequence */
    uint64_t objSeq; /* Object sequence */
    uint32_t hbLock; /* Heartbeat lock flag */
    uuid_t hbUuid;   /* UUID of locking server */
    uint64_t mtime;
    u_char pad1[0x1c0]; /* Padding/unknown */
    u_char pad2[4096 - 512];
} __attribute__((packed));

#define VMFS6_MDH_OFFSET_MAGIC offsetof(struct VmfsMetadataHdrRaw, magic)
#define VMFS6_MDH_OFFSET_POS offsetof(struct VmfsMetadataHdrRaw, pos)
#define VMFS6_MDH_OFFSET_HB_POS offsetof(struct VmfsMetadataHdrRaw, hbPos)
#define VMFS6_MDH_OFFSET_HB_SEQ offsetof(struct VmfsMetadataHdrRaw, hbSeq)
#define VMFS6_MDH_OFFSET_OBJ_SEQ offsetof(struct VmfsMetadataHdrRaw, objSeq)
#define VMFS6_MDH_OFFSET_HB_LOCK offsetof(struct VmfsMetadataHdrRaw, hbLock)
#define VMFS6_MDH_OFFSET_HB_UUID offsetof(struct VmfsMetadataHdrRaw, hbUuid)
#define VMFS6_MDH_OFFSET_MTIME offsetof(struct VmfsMetadataHdrRaw, mtime)

struct VmfsMetadataHdrS {
    uint32_t magic;
    uint64_t pos;
    uint64_t hbPos;
    uint64_t hbSeq;
    uint64_t objSeq;
    uint32_t hbLock;
    uuid_t hbUuid;
    uint64_t mtime;
};
using VmfsMetadataHdrT = struct VmfsMetadataHdrS;

class VmfsMetadata {
public:
    static VmfsMetadata *Instance();
    virtual ~VmfsMetadata() = default;

    /* Read a metadata header */
    int HdrRead(VmfsMetadataHdrT *mdh, const u_char *buf);

    /* Write a metadata header */
    int HdrWrite(const VmfsMetadataHdrT *mdh, u_char *buf);

private:
    VmfsMetadata() = default;
    static VmfsMetadata *m_instance;
    static std::mutex m_mutex;
};
}

#endif
