#ifndef __VMFS5_HEARTBEAT_H__
#define __VMFS5_HEARTBEAT_H__

#include <stddef.h>
#include <stdbool.h>
#include <mutex>

#define VMFS5_HB_BASE 0x0300000

#define VMFS5_HB_SIZE 0x200

#define VMFS5_HB_NUM 2048

#define VMFS5_HB_MAGIC_OFF 0xabcdef01
#define VMFS5_HB_MAGIC_ON 0xabcdef02


namespace Vmfs5IO {
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

#define VMFS5_HB_OFFSET_MAGIC offsetof(struct VmfsHeartbeartRaw, magic)
#define VMFS5_HB_OFFSET_POS offsetof(struct VmfsHeartbeartRaw, pos)
#define VMFS5_HB_OFFSET_SEQ offsetof(struct VmfsHeartbeartRaw, seq)
#define VMFS5_HB_OFFSET_UPTIME offsetof(struct VmfsHeartbeartRaw, uptime)
#define VMFS5_HB_OFFSET_UUID offsetof(struct VmfsHeartbeartRaw, uuid)
#define VMFS5_HB_OFFSET_JOURNAL_BLK offsetof(struct VmfsHeartbeartRaw, journalBlock)

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
        return (hb->magic == VMFS5_HB_MAGIC_ON);
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
