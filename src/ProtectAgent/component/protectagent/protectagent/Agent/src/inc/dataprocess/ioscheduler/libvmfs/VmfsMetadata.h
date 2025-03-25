/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * @file
 * @brief
 * @version 1.0.0.0
 * @date 2023-11-22
 * @author
 */
#ifndef __VMFS5_METADATA_H__
#define __VMFS5_METADATA_H__

#include <stddef.h>

#define VMFS5_METADATA_HDR_SIZE 512


namespace Vmfs5IO {
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
} __attribute__((packed));

#define VMFS5_MDH_OFFSET_MAGIC offsetof(struct VmfsMetadataHdrRaw, magic)
#define VMFS5_MDH_OFFSET_POS offsetof(struct VmfsMetadataHdrRaw, pos)
#define VMFS5_MDH_OFFSET_HB_POS offsetof(struct VmfsMetadataHdrRaw, hbPos)
#define VMFS5_MDH_OFFSET_HB_SEQ offsetof(struct VmfsMetadataHdrRaw, hbSeq)
#define VMFS5_MDH_OFFSET_OBJ_SEQ offsetof(struct VmfsMetadataHdrRaw, objSeq)
#define VMFS5_MDH_OFFSET_HB_LOCK offsetof(struct VmfsMetadataHdrRaw, hbLock)
#define VMFS5_MDH_OFFSET_HB_UUID offsetof(struct VmfsMetadataHdrRaw, hbUuid)
#define VMFS5_MDH_OFFSET_MTIME offsetof(struct VmfsMetadataHdrRaw, mtime)

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
