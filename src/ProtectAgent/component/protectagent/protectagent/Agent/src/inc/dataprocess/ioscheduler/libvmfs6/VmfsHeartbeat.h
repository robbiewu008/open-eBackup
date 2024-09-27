/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * @file
 * @brief
 * @version 1.0.0.0
 * @date 2023-11-22
 * @author
 */
#ifndef __VMFS6_HEARTBEAT_H__
#define __VMFS6_HEARTBEAT_H__

#include <stddef.h>
#include <stdbool.h>
#include <mutex>

#define VMFS6_HB_BASE 0x0300000

#define VMFS6_HB_SIZE 0x1000

#define VMFS6_HB_NUM 1024

#define VMFS6_HB_MAGIC_OFF 0xabcdef01
#define VMFS6_HB_MAGIC_ON 0xabcdef02


namespace Vmfs6IO {
struct VmfsHeartbeartRaw {
    uint32_t magic;
    uint64_t pos;
    uint64_t seq;
    uint64_t uptime;
    uuid_t uuid;
    uint32_t journalBlock;
    uint32_t volVersion; /* from fs_info (?) */
    uint32_t version;    /* from fs_info (?) */
} __attribute__((packed));

#define VMFS6_HB_OFFSET_MAGIC offsetof(struct VmfsHeartbeartRaw, magic)
#define VMFS6_HB_OFFSET_POS offsetof(struct VmfsHeartbeartRaw, pos)
#define VMFS6_HB_OFFSET_SEQ offsetof(struct VmfsHeartbeartRaw, seq)
#define VMFS6_HB_OFFSET_UPTIME offsetof(struct VmfsHeartbeartRaw, uptime)
#define VMFS6_HB_OFFSET_UUID offsetof(struct VmfsHeartbeartRaw, uuid)
#define VMFS6_HB_OFFSET_JOURNAL_BLK offsetof(struct VmfsHeartbeartRaw, journalBlock)

struct VmfsHeartbeatS {
    uint32_t magic;
    uint64_t pos;
    uint64_t seq;        /* Sequence number */
    uint64_t uptime;     /* Uptime (in usec) of the locker */
    uuid_t uuid;         /* UUID of the server */
    uint32_t journalBlk; /* Journal block */
};
using VmfsHeartbeatT = struct VmfsHeartbeatS;

class VmfsHeartbeat {
public:
    static VmfsHeartbeat *Instance();
    virtual ~VmfsHeartbeat() = default;

    /* Unlock an heartbeat */
    int Unlock(VmfsFsT *fs, VmfsHeartbeatT *hb);

private:
    inline bool Active(VmfsHeartbeatT *hb)
    {
        return (hb->magic == VMFS6_HB_MAGIC_ON);
    }

    /* Write a heartbeat info */
    int Write(const VmfsHeartbeatT *hb, u_char *buf);

private:
    VmfsHeartbeat() = default;
    static VmfsHeartbeat *m_instance;
    static std::mutex m_mutex;
};
}

#endif
