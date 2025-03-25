#include <cstring>
#include <cstdlib>
#include "dataprocess/ioscheduler/libvmfs6/Vmfs.h"

namespace Vmfs6IO {
/* VMFS meta-files */
const std::string VMFS6_FBB_FILENAME = ".fbb.sf";
const std::string VMFS6_FDC_FILENAME = ".fdc.sf";
const std::string VMFS6_PBC_FILENAME = ".pbc.sf";
const std::string VMFS6_SBC_FILENAME = ".sbc.sf";
const std::string VMFS6_PB2_FILENAME = ".pb2.sf";

VmfsFs *VmfsFs::m_instance = nullptr;
std::mutex VmfsFs::m_mutex;

VmfsFs *VmfsFs::Instance()
{
    if (m_instance == nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_instance == nullptr) {
            m_instance = new (std::nothrow) VmfsFs();
        }
    }
    return m_instance;
}

/* Read a block from the filesystem */
ssize_t VmfsFs::ReadBlock(const VmfsFsT *fs, uint32_t blk, off_t offset, u_char *buf, size_t len)
{
    off_t pos;

    pos = (uint64_t)blk * GetBlockSize(fs);
    pos += offset;

    return (VmfsDeviceRead(fs->dev, pos, buf, len));
}

/* Open a filesystem */
VmfsFsT *VmfsFs::FSOpen(const std::set<std::string> &paths, VmfsFlagsT flags)
{
    VmfsDeviceT *dev;
    VmfsFsT *fs;

    dev = DeviceOpen(paths, flags);
    if (!dev || !(fs = (VmfsFsT *)calloc(1, sizeof(*fs))))
        return NULL;

    fs->inodeHashBuckets = VMFS6_INODE_HASH_BUCKETS;
    fs->inodes = (VmfsInodeT **)calloc(fs->inodeHashBuckets, sizeof(VmfsInodeT *));

    if (!fs->inodes) {
        free(fs);
        return NULL;
    }

    fs->dev = dev;
    /* Read FS info */
    if (FsInfoRead(fs) == -1) {
        COMMLOG(OS_LOG_ERROR, "vmfsio - Unable to read FS information");
        FSClose(fs);
        return NULL;
    }

    if (uuid_compare(fs->fs_info.lvmUuid, *fs->dev->uuid)) {
        COMMLOG(OS_LOG_ERROR, "vmfsio - FS doesn't belong to the underlying LVM");
        FSClose(fs);
        return NULL;
    }

    /* Read FDC base information */
    if (ReadFdcBase(fs) == -1) {
        COMMLOG(OS_LOG_ERROR, "vmfsio - Unable to read FDC information");
        FSClose(fs);
        return NULL;
    }

    COMMLOG(OS_LOG_DEBUG, "vmfsio - filesystem opened successfully");
    return fs;
}

/* Close a FS */
void VmfsFs::FSClose(VmfsFsT *fs)
{
    if (!fs) {
        return;
    }

    if (fs->hb_refcount > 0) {
        COMMLOG(OS_LOG_WARN, "vmfsio - Heartbeat still active in metadata (refCount=%u)", fs->hb_refcount);
    }

    VmfsHeartbeat::Instance()->Unlock(fs, &fs->hb);
    VmfsBitmap::Instance()->VmfsBitmapClose(fs->fbb);
    VmfsBitmap::Instance()->VmfsBitmapClose(fs->fdc);
    VmfsBitmap::Instance()->VmfsBitmapClose(fs->pbc);
    VmfsBitmap::Instance()->VmfsBitmapClose(fs->pb2);
    VmfsBitmap::Instance()->VmfsBitmapClose(fs->sbc);

    SyncInodes(fs);
    VmfsDeviceClose(fs->dev);
    free(fs->inodes);
    free(fs->fs_info.label);
    free(fs);
}

/* Read filesystem information */
int VmfsFs::FsInfoRead(VmfsFsT *fs)
{
    V6_DECL_ALIGNED_BUFFER(buf, NUM_512);
    VmfsFsInfoT *fsi = &fs->fs_info;

    if (VmfsDeviceRead(fs->dev, VMFS6_FSINFO_BASE, buf, buf_len) != buf_len) {
        return (-1);
    }

    fsi->magic = ReadLE32(buf, VMFS6_FSINFO_OFFSET_MAGIC);

    if (fsi->magic != VMFS6_FSINFO_MAGIC && fsi->magic != VMFSL_FSINFO_MAGIC) {
        COMMLOG(OS_LOG_ERROR, "vmfsio - FSInfo: invalid magic number 0x%8.8x\n", fsi->magic);
        return (-1);
    }

    fsi->volVersion = ReadLE32(buf, VMFS6_FSINFO_OFFSET_VOLVER);
    fsi->version = buf[VMFS6_FSINFO_OFFSET_VER];
    fsi->mode = ReadLE32(buf, VMFS6_FSINFO_OFFSET_MODE);
    fsi->blockSize = ReadLE64(buf, VMFS6_FSINFO_OFFSET_BLKSIZE);
    fsi->subblockSize = ReadLE32(buf, VMFS6_FSINFO_OFFSET_SBSIZE);
    fsi->fdcHeaderSize = ReadLE32(buf, VMFS6_FSINFO_OFFSET_FDC_HEADER_SIZE);
    fsi->fdcBitmapCount = ReadLE32(buf, VMFS6_FSINFO_OFFSET_FDC_BITMAP_COUNT);
    fsi->ctime = (time_t)ReadLE32(buf, VMFS6_FSINFO_OFFSET_CTIME);

    ReadUuid(buf, VMFS6_FSINFO_OFFSET_UUID, &fsi->uuid);
    fsi->label = strndup((char *)buf + VMFS6_FSINFO_OFFSET_LABEL, VMFS6_FSINFO_OFFSET_LABEL_SIZE);
    ReadUuid(buf, VMFS6_FSINFO_OFFSET_LVM_UUID, &fsi->lvmUuid);

    return (0);
}

VmfsBitmapT *VmfsFs::OpenMetaFile(VmfsDirT *root_dir, const char *name, uint32_t max_item, uint32_t max_entry,
    const char *desc)
{
    VmfsBitmapT *bitmap = VmfsBitmap::Instance()->VmfsBitmapOpenAt(root_dir, name);
    if (!bitmap) {
        COMMLOG(OS_LOG_ERROR, "vmfsio - unable to open %s.\n", desc);
        return NULL;
    }

    if (bitmap->bmh.itemsPerBitmapEntry > max_item) {
        COMMLOG(OS_LOG_ERROR,
            "vmfsio - unsupported number of items per entry in %s, items per bitmap entry: %u, max item: %u",
            desc, bitmap->bmh.itemsPerBitmapEntry, max_item);
        return NULL;
    }

    // if the bitmap is disabled then total_items will be 0. e.g. pbc.sf in vmfs6
    if (bitmap->bmh.totalItems == 0) {
        return bitmap;
    }

    if ((bitmap->bmh.totalItems + bitmap->bmh.itemsPerBitmapEntry - 1) / bitmap->bmh.itemsPerBitmapEntry > max_entry) {
        COMMLOG(OS_LOG_ERROR, "vmfsio - unsupported number of entries in %s.\n", desc);
        return NULL;
    }
    return bitmap;
}

/* Open all the VMFS meta files */
int VmfsFs::OpenAllMetaFiles(VmfsFsT *fs)
{
    VmfsBitmapT *fdc = fs->fdc;
    VmfsDirT *root_dir;

    /* Read the first inode */
    if (!(root_dir = VmfsDirent::Instance()->VmfsDirOpenFromBlkid(fs, VMFS6_BLK_FD_BUILD(0, 0, 0)))) {
        COMMLOG(OS_LOG_ERROR, "vmfsio - unable to open root directory");
        return (-1);
    }

    COMMLOG(OS_LOG_DEBUG, "vmfsio - Loading pbc...");
    fs->pbc = OpenMetaFile(root_dir, VMFS6_PBC_FILENAME.c_str(), VMFS6_BLK_PB_MAX_ITEM, VMFS6_BLK_PB_MAX_ENTRY,
        "Pointer Block Bitmap (PBC)");
    if (fs->pbc == NULL) {
        return (-1);
    }

    COMMLOG(OS_LOG_DEBUG, "vmfsio - Loading pb2...");
    fs->pb2 = OpenMetaFile(root_dir, VMFS6_PB2_FILENAME.c_str(), VMFS6_BLK_PB2_MAX_ITEM, VMFS6_BLK_PB2_MAX_ENTRY,
        "Pointer 2nd Block Bitmap (PB2)");
    if (!fs->pb2)
        return (-1);

    COMMLOG(OS_LOG_DEBUG, "vmfsio - Loading sbc...");
    fs->sbc = OpenMetaFile(root_dir, VMFS6_SBC_FILENAME.c_str(), VMFS6_BLK_SB_MAX_ITEM, VMFS6_BLK_SB_MAX_ENTRY,
        "Pointer Sub Bitmap (SBC)");
    if (fs->sbc == NULL) {
        return (-1);
    }

    COMMLOG(OS_LOG_DEBUG, "vmfsio - Open root dir done.");
    if (!(fs->fbb = VmfsBitmap::Instance()->VmfsBitmapOpenAt(root_dir, VMFS6_FBB_FILENAME.c_str()))) {
        COMMLOG(OS_LOG_ERROR,
            "vmfsio - Unable to open File-Block Bitmap (FBB). (0x%x 0x%lx)", fs->fbb->bmh.totalItems,
            VMFS6_BLK_FB_MAX_ITEM);
        return (-1);
    }
    if (fs->fbb->bmh.totalItems > VMFS6_BLK_FB_MAX_ITEM) {
        COMMLOG(OS_LOG_ERROR, "vmfsio - Unsupported number of items in File-Block Bitmap (FBB).");
        return (-1);
    }

    COMMLOG(OS_LOG_DEBUG, "vmfsio - Loading fdc...");
    fs->fdc = OpenMetaFile(root_dir, VMFS6_FDC_FILENAME.c_str(), VMFS6_BLK_FD_MAX_ITEM, VMFS6_BLK_FD_MAX_ENTRY,
        "File Descriptor Bitmap (FDC)");
    if (fs->fdc == NULL) {
        return (-1);
    }

    VmfsBitmap::Instance()->VmfsBitmap::Instance()->VmfsBitmapClose(fdc);
    VmfsDirent::Instance()->VmfsDirClose(root_dir);
    return (0);
}

/* Read FDC base information */
int VmfsFs::ReadFdcBase(VmfsFsT *fs)
{
    VmfsInodeT inode = {
        {
            0,
        },
    };
    uint64_t fdc_base;

    /*
     * Compute position of FDC base: it is located at the first
     * block after heartbeat information.
     * When blocksize = 8 Mb, there is free space between heartbeats
     * and FDC.
     */
    fdc_base = M_MAX(1, (VMFS6_HB_BASE + VMFS6_HB_NUM * VMFS6_HB_SIZE) / GetBlockSize(fs));

    COMMLOG(OS_LOG_DEBUG, "vmfsio - File Descriptor Bitmap base = block #%lu\n", fdc_base);
    inode.fs = fs;
    inode.mdh.magic = VMFS6_INODE_MAGIC;
    inode.size = fs->fs_info.blockSize;
    inode.type = VMFS6_FTYPE_META;
    inode.blkSize = fs->fs_info.blockSize;
    inode.blkCount = 1;
    inode.zla = VMFS6_BLK_TYPE_FB;
    inode.blocks[0] = VMFS6_BLK_FB_BUILD(fdc_base, 0);
    inode.refCount = 1;

    fs->fdc = VmfsBitmap::Instance()->VmfsBitmapOpenFromInode(&inode);

    /* Read the meta files */
    if (OpenAllMetaFiles(fs) == -1) {
        return (-1);
    }

    return (0);
}

VmfsDeviceT *VmfsFs::DeviceOpen(const std::set<std::string> &paths, VmfsFlagsT flags)
{
    VmfsLvmT *lvm;

    if (!(lvm = VmfsLvm::Instance()->Create(flags))) {
        COMMLOG(OS_LOG_ERROR, "vmfsio - Unable to create LVM structure");
        return NULL;
    }

    for (const auto &path : paths) {
        if (VmfsLvm::Instance()->AddExtent(lvm, VmfsVolume::Instance()->VolOpen(path.c_str(), flags)) == -1) {
            COMMLOG(OS_LOG_ERROR, "vmfsio - Unable to open device: %s.", path.c_str());
            return NULL;
        }
    }

    if (VmfsLvm::Instance()->Open(lvm)) {
        VmfsDeviceClose(&lvm->dev);
        return NULL;
    }

    return &lvm->dev;
}

/*
 * Check that all inodes have been released, and synchronize them if this
 * is not the case.
 */
void VmfsFs::SyncInodes(VmfsFsT *fs)
{
    VmfsInodeT *inode;
    int i;

    for (i = 0; i < VMFS6_INODE_HASH_BUCKETS; i++) {
        for (inode = fs->inodes[i]; inode; inode = inode->next) {
            COMMLOG(OS_LOG_DEBUG, "Inode 0x%8.8x: refCount=%u, update_flags=0x%x\n", inode->id, inode->refCount,
                inode->update_flags);
            if (inode->update_flags) {
                VmfsInode::Instance()->Update(inode, inode->update_flags & VMFS6_INODE_SYNC_BLK);
            }
        }
    }
}
}