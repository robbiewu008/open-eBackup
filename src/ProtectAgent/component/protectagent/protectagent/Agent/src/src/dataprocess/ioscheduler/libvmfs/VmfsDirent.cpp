/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * @file
 * @brief
 * @version 1.0.0.0
 * @date 2023-11-22
 * @author
 */
/*
 * VMFS directory entries.
 */

#include <cstdlib>
#include <cstring>
#include <cerrno>
#include "dataprocess/ioscheduler/libvmfs/Vmfs.h"


namespace Vmfs5IO {
VmfsDirent *VmfsDirent::m_instance = nullptr;
std::mutex VmfsDirent::m_mutex;

VmfsDirent *VmfsDirent::Instance()
{
    if (m_instance == nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_instance == nullptr) {
            m_instance = new (std::nothrow) VmfsDirent();
        }
    }
    return m_instance;
}

/* Read a directory entry */
int VmfsDirent::VmfsDirentRead(VmfsDirentT *entry, const u_char *buf)
{
    entry->type = ReadLE32(buf, VMFS5_DIRENT_OFFSET_TYPE);
    entry->blockId = ReadLE32(buf, VMFS5_DIRENT_OFFSET_BLK_ID);
    entry->recordId = ReadLE32(buf, VMFS5_DIRENT_OFFSET_REC_ID);
    int ret = memcpy_s(entry->name, VMFS5_DIRENT_OFFSET_NAME_SIZE,
        buf + VMFS5_DIRENT_OFFSET_NAME, VMFS5_DIRENT_OFFSET_NAME_SIZE);
    if (ret != 0) {
        return (-1);
    }
    entry->name[VMFS5_DIRENT_OFFSET_NAME_SIZE] = 0;
    return (0);
}

/* Write a directory entry */
int VmfsDirent::VmfsDirentWrite(const VmfsDirentT *entry, u_char *buf)
{
    WriteLE32(buf, VMFS5_DIRENT_OFFSET_TYPE, entry->type);
    WriteLE32(buf, VMFS5_DIRENT_OFFSET_BLK_ID, entry->blockId);
    WriteLE32(buf, VMFS5_DIRENT_OFFSET_REC_ID, entry->recordId);
    int ret = memcpy_s(buf + VMFS5_DIRENT_OFFSET_NAME,
        VMFS5_DIRENT_OFFSET_NAME_SIZE, entry->name, VMFS5_DIRENT_OFFSET_NAME_SIZE);
    if (ret != 0) {
        return (-1);
    }
    return (0);
}

/* Search for an entry into a directory ; affects position of the next
entry VmfsDirRead will return */
const VmfsDirentT *VmfsDirent::VmfsDirLookup(VmfsDirT *d, const char *name)
{
    const VmfsDirentT *rec;
    VmfsDirSeek(d, 0);
    while ((rec = VmfsDirRead(d))) {
        if (!strcmp(rec->name, name)) {
            return (rec);
        }
    }
    return (NULL);
}

/* Read a symlink */
char *VmfsDirent::VmfsDirentReadSymlink(const VmfsFsT *fs, const VmfsDirentT *entry)
{
    VmfsFileT *f;
    size_t str_len;
    char *str = NULL;

    if (!(f = VmfsFile::Instance()->OpenFromBlkid(fs, entry->blockId))) {
        return NULL;
    }

    str_len = VmfsFile::Instance()->VmfsFileGetSize(f);
    if (!(str = (char *)malloc(str_len + 1))) {
        goto done;
    }

    if ((str_len = VmfsFile::Instance()->Read(f, (u_char *)str, str_len, 0)) == -1) {
        free(str);
        goto done;
    }

    str[str_len] = 0;

done:
    VmfsFile::Instance()->Close(f);
    return str;
}

/* Resolve a path name to a block id */
uint32_t VmfsDirent::VmfsDirResolvePath(VmfsDirT *base_dir, const char *path, int follow_symlink)
{
    VmfsDirT *cur_dir;
    VmfsDirT *sub_dir;
    const VmfsDirentT *rec;
    char *nam;
    char *str;
    char *sl;
    char *symlink;
    int close_dir = 0;
    const VmfsFsT *fs = VmfsDirGetFs(base_dir);
    uint32_t ret = 0;

    cur_dir = base_dir;

    if (*path == '/') {
        if (!(cur_dir = VmfsDirOpenFromBlkid(fs, VMFS5_BLK_FD_BUILD(0, 0, 0)))) {
            return (0);
        }
        path++;
        close_dir = 1;
    }

    if (!(rec = VmfsDirLookup(cur_dir, "."))) {
        return (0);
    }

    ret = rec->blockId;
    nam = str = strdup(path);
    while (*str != 0) {
        sl = strchr(str, '/');
        if (sl != NULL) {
            *sl = 0;
        }

        if (*str == 0) {
            str = sl + 1;
            continue;
        }

        if (!(rec = VmfsDirLookup(cur_dir, str))) {
            ret = 0;
            break;
        }

        ret = rec->blockId;
        if ((sl == NULL) && !follow_symlink) {
            break;
        }

        /* follow the symlink if we have an entry of this type */
        if (rec->type == VMFS5_FTYPE_SYMLINK) {
            if (!(symlink = VmfsDirentReadSymlink(fs, rec))) {
                ret = 0;
                break;
            }

            ret = VmfsDirResolvePath(cur_dir, symlink, 1);
            free(symlink);

            if (!ret) {
                break;
            }
        }

        if (sl == NULL)
            break;

        if (!(sub_dir = VmfsDirOpenFromBlkid(fs, ret))) {
            ret = 0;
            break;
        }

        if (close_dir)
            VmfsDirClose(cur_dir);

        cur_dir = sub_dir;
        close_dir = 1;
        str = sl + 1;
    }
    free(nam);

    if (close_dir)
        VmfsDirClose(cur_dir);

    return (ret);
}

/* Cache content of a directory */
int VmfsDirent::VmfsDirCacheEntries(VmfsDirT *d)
{
    off_t dir_size;

    if (d->buf != NULL) {
        free(d->buf);
    }

    dir_size = VmfsFile::Instance()->VmfsFileGetSize(d->dir);
    if (!(d->buf = (u_char *)calloc(1, dir_size))) {
        return (-1);
    }

    if (VmfsFile::Instance()->Read(d->dir, d->buf, dir_size, 0) != dir_size) {
        free(d->buf);
        return (-1);
    }

    return (0);
}

/* Open a directory file */
VmfsDirT *VmfsDirent::VmfsDirOpenFromFile(VmfsFileT *file)
{
    VmfsDirT *d;

    if (file == NULL) {
        return NULL;
    }

    if (!(d = (VmfsDirT *)calloc(1, sizeof(*d))) || (file->inode->type != VMFS5_FTYPE_DIR)) {
        VmfsFile::Instance()->Close(file);
        return NULL;
    }

    d->dir = file;
    VmfsDirCacheEntries(d);
    return d;
}

/* Open a directory based on a directory entry */
VmfsDirT *VmfsDirent::VmfsDirOpenFromBlkid(const VmfsFsT *fs, uint32_t blkId)
{
    return VmfsDirOpenFromFile(VmfsFile::Instance()->OpenFromBlkid(fs, blkId));
}

/* Return next entry in directory. Returned directory entry will be overwritten
by subsequent calls */
const VmfsDirentT *VmfsDirent::VmfsDirRead(VmfsDirT *d)
{
    u_char *buf;
    if (d == NULL) {
        return (NULL);
    }

    if (d->buf) {
        if (d->pos * VMFS5_DIRENT_SIZE >= VmfsFile::Instance()->VmfsFileGetSize(d->dir)) {
            return (NULL);
        }
        buf = &d->buf[d->pos * VMFS5_DIRENT_SIZE];
    } else {
        u_char _buf[VMFS5_DIRENT_SIZE];
        if ((VmfsFile::Instance()->Read(d->dir, _buf, sizeof(_buf), d->pos * sizeof(_buf)) != sizeof(_buf))) {
            return (NULL);
        }
        buf = _buf;
    }

    VmfsDirentRead(&d->dirent, buf);
    d->pos++;

    return &d->dirent;
}

/* Close a directory */
int VmfsDirent::VmfsDirClose(VmfsDirT *dir)
{
    if (dir == NULL) {
        return -1;
    }

    if (dir->buf) {
        free(dir->buf);
    }

    VmfsFile::Instance()->Close(dir->dir);
    free(dir);
    return 0;
}
}