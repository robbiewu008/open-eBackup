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
#ifndef __VMFS5_FS_H__
#define __VMFS5_FS_H__

#include <stddef.h>
#include <string>
#include <set>

/* === FS Info === */
#define VMFS5_FSINFO_BASE 0x0200000
#define VMFS5_FSINFO_MAGIC 0x2fabf15e


namespace Vmfs5IO {
struct VmfsFSInfoPlain {
    unsigned int magic;
    unsigned int volver;
    unsigned char ver;
    uuid_t uuid;
    unsigned int mode;
    char label[128];
    uint32_t devBlocksize;
    uint64_t blocksize;
    uint32_t ctime;
    uint32_t unknown3;
    uuid_t lvmUuid;
    unsigned char unknown4[16];
    uint32_t fdcHeaderSize;
    uint32_t fdcBitmapCount;
    uint32_t subblockSize;
} __attribute__((packed));

#define VMFS5_FSINFO_OFFSET_MAGIC offsetof(struct VmfsFSInfoPlain, magic)
#define VMFS5_FSINFO_OFFSET_VOLVER offsetof(struct VmfsFSInfoPlain, volver)
#define VMFS5_FSINFO_OFFSET_VER offsetof(struct VmfsFSInfoPlain, ver)
#define VMFS5_FSINFO_OFFSET_UUID offsetof(struct VmfsFSInfoPlain, uuid)
#define VMFS5_FSINFO_OFFSET_MODE offsetof(struct VmfsFSInfoPlain, mode)
#define VMFS5_FSINFO_OFFSET_LABEL offsetof(struct VmfsFSInfoPlain, label)
#define VMFS5_FSINFO_OFFSET_BLKSIZE offsetof(struct VmfsFSInfoPlain, blocksize)
#define VMFS5_FSINFO_OFFSET_CTIME offsetof(struct VmfsFSInfoPlain, ctime)
#define VMFS5_FSINFO_OFFSET_LVM_UUID offsetof(struct VmfsFSInfoPlain, lvmUuid)
#define VMFS5_FSINFO_OFFSET_SBSIZE offsetof(struct VmfsFSInfoPlain, subblockSize)

#define VMFS5_FSINFO_OFFSET_FDC_HEADER_SIZE offsetof(struct VmfsFSInfoPlain, fdcHeaderSize)

#define VMFS5_FSINFO_OFFSET_FDC_BITMAP_COUNT offsetof(struct VmfsFSInfoPlain, fdcBitmapCount)

#define VMFS5_FSINFO_OFFSET_LABEL_SIZE sizeof(((struct VmfsFSInfoPlain *)(0))->label)

struct VmfsFsInfoS {
    uint32_t magic;
    uint32_t volVersion;
    uint32_t version;
    uint32_t mode;
    uuid_t uuid;
    char *label;
    time_t ctime;

    uint64_t blockSize;
    uint32_t subblockSize;

    uint32_t fdcHeaderSize;
    uint32_t fdcBitmapCount;

    uuid_t lvmUuid;
};
using VmfsFsInfoT = struct VmfsFsInfoS;

/* === VMFS filesystem === */
#define VMFS5_INODE_HASH_BUCKETS 256

struct VmfsFsS {
    int debugLevel;

    /* FS information */
    VmfsFsInfoT fs_info;

    /* Associated VMFS Device */
    VmfsDeviceT *dev;

    /* Meta-files containing file system structures */
    VmfsBitmapT *fbb, *sbc, *pbc, *fdc;

    /* Heartbeat used to lock meta-data */
    VmfsHeartbeatT hb;
    u_int hb_id;
    uint64_t hbSeq;
    u_int hb_refcount;
    uint64_t hbExpire;

    /* Counter for "gen" field in inodes */
    uint32_t inodeGen;

    /* In-core inodes hash table */
    u_int inodeHashBuckets;
    VmfsInodeT **inodes;
};
using VmfsFsT = struct VmfsFsS;


class VmfsFs {
public:
    static VmfsFs *Instance();
    virtual ~VmfsFs() = default;

    /* Get block size of a volume */
    inline uint64_t GetBlockSize(const VmfsFsT *fs)
    {
        return (fs->fs_info.blockSize);
    }

    /* Read a block from the filesystem */
    ssize_t ReadBlock(const VmfsFsT *fs, uint32_t blk, off_t offset, u_char *buf, size_t len);

    /* Open a FS */
    VmfsFsT *FSOpen(const std::set<std::string> &paths, VmfsFlagsT flags);

    /* Close a FS */
    void FSClose(VmfsFsT *fs);

private:
    /* Read filesystem information */
    int FsInfoRead(VmfsFsT *fs);
    VmfsBitmapT *OpenMetaFile(VmfsDirT *root_dir, const char *name, uint32_t max_item, uint32_t max_entry,
        const char *desc);

    /* Open all the VMFS meta files */
    int OpenAllMetaFiles(VmfsFsT *fs);

    /* Read FDC base information */
    int ReadFdcBase(VmfsFsT *fs);

    VmfsDeviceT *DeviceOpen(const std::set<std::string> &paths, VmfsFlagsT flags);

    /*
     * Check that all inodes have been released, and synchronize them if this
     * is not the case.
     */
    void SyncInodes(VmfsFsT *fs);

private:
    VmfsFs() = default;
    static VmfsFs *m_instance;
    static std::mutex m_mutex;
};
}

#endif
