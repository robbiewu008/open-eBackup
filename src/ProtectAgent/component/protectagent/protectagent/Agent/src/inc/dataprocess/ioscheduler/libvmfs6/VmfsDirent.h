/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * @file
 * @brief
 * @version 1.0.0.0
 * @date 2023-11-22
 * @author
 */
#ifndef __VMFS6_DIRENT_H__
#define __VMFS6_DIRENT_H__

#include <stddef.h>


namespace Vmfs6IO {
#define VMFS6_DIRENT_SIZE (0x120)

struct VmfsDirentRaw {
    uint32_t type;
    uint32_t blockId;
    uint32_t recordId;
    uint32_t unknown;
    uint64_t off_in_file;
    char name[128];
} __attribute__((packed));

#define VMFS6_DIRENT_OFFSET_TYPE offsetof(struct VmfsDirentRaw, type)
#define VMFS6_DIRENT_OFFSET_BLK_ID offsetof(struct VmfsDirentRaw, blockId)
#define VMFS6_DIRENT_OFFSET_REC_ID offsetof(struct VmfsDirentRaw, recordId)
#define VMFS6_DIRENT_OFFSET_NAME offsetof(struct VmfsDirentRaw, name)

#define VMFS6_DIRENT_OFFSET_NAME_SIZE sizeof(((struct VmfsDirentRaw *)(0))->name)

struct VmfsDirentS {
    uint32_t type;
    uint32_t blockId;
    uint32_t recordId;
    char name[129];
};
using VmfsDirentT = struct VmfsDirentS;

struct VmfsDirS {
    VmfsFileT *dir;
    uint32_t pos;
    VmfsDirentT dirent;
    u_char *buf;
    u_char *ar_hb_exist;
};
using VmfsDirT = struct VmfsDirS;

class VmfsDirent {
public:
    static VmfsDirent *Instance();
    virtual ~VmfsDirent() = default;

    const VmfsFsT *VmfsDirGetFs(VmfsDirT *d)
    {
        return d ? VmfsFile::Instance()->VmfsFileGetFs(d->dir) : NULL;
    }

    /* Search for an entry into a directory ; affects position of the next
    entry VmfsDirRead will return */
    const VmfsDirentT *VmfsDirLookup(VmfsDirT *dir, const char *name);

    /* Resolve a path to a block id */
    uint32_t VmfsDirResolvePath(VmfsDirT *base_dir, const char *path, int follow_symlink);

    /* Open a directory based on a directory entry */
    VmfsDirT *VmfsDirOpenFromBlkid(const VmfsFsT *fs, uint64_t blkId);

    /* Close a directory */
    int VmfsDirClose(VmfsDirT *d);

private:
    /* Set position of the next entry that VmfsDirRead will return */
    void VmfsDirSeek(VmfsDirT *d, uint32_t pos)
    {
        if (d)
            d->pos = pos;
    }

    /* Return next entry in directory. Returned directory entry will be overwritten
    by subsequent calls */
    const VmfsDirentT *VmfsDirRead(VmfsDirT *d);

    /* Read a directory entry */
    int VmfsDirentRead(VmfsDirentT *entry, const u_char *buf);

    /* Write a directory entry */
    int VmfsDirentWrite(const VmfsDirentT *entry, u_char *buf);

    /* Read a symlink */
    char *VmfsDirentReadSymlink(const VmfsFsT *fs, const VmfsDirentT *entry);

    /* Cache content of a directory */
    int VmfsDirCacheEntries(VmfsDirT *d);

    /* Open a directory file */
    VmfsDirT *VmfsDirOpenFromFile(VmfsFileT *file);

private:
    VmfsDirent() = default;
    static VmfsDirent *m_instance;
    static std::mutex m_mutex;
};
}

#endif
