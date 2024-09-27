#ifndef __VMFS5_LVM_H__
#define __VMFS5_LVM_H__


#define VMFS5_LVM_MAX_EXTENTS 32

#define VMFS5_LVM_SEGMENT_SIZE (256 * 1024 * 1024)


namespace Vmfs5IO {
struct VmfsLvmInfoS {
    uuid_t uuid;
    uint32_t numExtents;
    uint64_t size;
    uint64_t blocks;
};
using VmfsLvmInfoT = struct VmfsLvmInfoS;

/* === LVM === */
struct VmfsLvmS {
    VmfsDeviceT dev;

    VmfsFlagsT flags;

    /* LVM information */
    VmfsLvmInfoT lvm_info;

    /* number of extents currently loaded in the lvm */
    int loadedExtents;

    /* extents */
    VmfsVolumeT *extents[VMFS5_LVM_MAX_EXTENTS];
};
using VmfsLvmT = struct VmfsLvmS;

class VmfsLvm {
public:
    static VmfsLvm *Instance();
    virtual ~VmfsLvm() = default;

    /* Create a volume structure */
    VmfsLvmT *Create(VmfsFlagsT flags);

    /* Add an extent to the LVM */
    int AddExtent(VmfsLvmT *lvm, VmfsVolumeT *vol);

    /* Open an LVM */
    int Open(VmfsLvmT *lvm);

private:
    VmfsVolumeT *GetExtentFromOffset(const VmfsLvmT *lvm, off_t pos);
    /* Get extent size */
    uint64_t ExtentSize(const VmfsVolumeT *extent);
    typedef ssize_t (*vmfs_vol_io_func)(const VmfsDeviceT *, off_t, u_char *, size_t);

    /* Read a raw block of data on logical volume */
    ssize_t VmfsLvmIO(const VmfsLvmT *lvm, off_t pos, u_char *buf, size_t len, vmfs_vol_io_func func);
    /* Read a raw block of data on logical volume */
    ssize_t VmfsLvmRead(const VmfsDeviceT *dev, off_t pos, u_char *buf, size_t len);
    /* Reserve the underlying volume given a LVM position */
    int Reserve(const VmfsDeviceT *dev, off_t pos);
    /* Release the underlying volume given a LVM position */
    int Release(const VmfsDeviceT *dev, off_t pos);
    /* Close an LVM */
    void Close(VmfsDeviceT *dev); // TODO - who calls this?

private:
    VmfsLvm() = default;
    static VmfsLvm *m_instance;
    static std::mutex m_mutex;
};
}

#endif
