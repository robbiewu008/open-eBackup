#ifndef __VMFS5_VOLUME_H__
#define __VMFS5_VOLUME_H__

#include <stddef.h>

/* === Volume Info === */
#define VMFS5_VOLINFO_BASE 0x100000
#define VMFS5_VOLINFO_MAGIC 0xc001d00d


namespace Vmfs5IO {
    struct VmfsVolinfoRaw {
        uint32_t magic;
        uint32_t ver;
        u_char unknown0[6];
        u_char lun;
        u_char _unknown1[3];
        char name[28];
        u_char unknown2[49]; /* The beginning of this array looks like it is a LUN
                           * GUID for 3.31 * filesystems, and the LUN identifier
                           * string as given by ESX for 3.21 filesystems. */
        uint32_t size;        /* Size of the physical volume, divided by 256 */
        u_char unknown3[31];
        uuid_t uuid;
        uint64_t ctime; /* ctime? in usec */
        uint64_t mtime; /* mtime? in usec */
    } __attribute__((packed));

#define VMFS5_VOLINFO_OFFSET_MAGIC offsetof(struct VmfsVolinfoRaw, magic)
#define VMFS5_VOLINFO_OFFSET_VER offsetof(struct VmfsVolinfoRaw, ver)
#define VMFS5_VOLINFO_OFFSET_LUN offsetof(struct VmfsVolinfoRaw, lun)
#define VMFS5_VOLINFO_OFFSET_NAME offsetof(struct VmfsVolinfoRaw, name)
#define VMFS5_VOLINFO_OFFSET_SIZE offsetof(struct VmfsVolinfoRaw, size)
#define VMFS5_VOLINFO_OFFSET_UUID offsetof(struct VmfsVolinfoRaw, uuid)

#define VMFS5_VOLINFO_OFFSET_NAME_SIZE sizeof(((struct VmfsVolinfoRaw *)(0))->name)

/* === LVM Info === */
#define VMFS5_LVMINFO_OFFSET 0x0200

    struct VmfsLvminfoRaw {
        uint64_t size;
        uint64_t blocks; /* Seems to always be sum(numSegments for all extents) +
                      * num_extents */
        uint32_t unknown0;
        char uuidStr[35];
        u_char _unknown1[29];
        uuid_t uuid;
        uint32_t unknown2;
        uint64_t ctime; /* ctime? in usec */
        uint32_t unknown3;
        uint32_t numSegments;
        uint32_t firstSegment;
        uint32_t unknown4;
        uint32_t lastSegment;
        uint32_t unknown5;
        uint64_t mtime; /* mtime? in usec */
        uint32_t numExtents;
    } __attribute__((packed));

#define VMFS5_LVMINFO(field) (VMFS5_LVMINFO_OFFSET + offsetof(struct VmfsLvminfoRaw, field))

#define VMFS5_LVMINFO_OFFSET_SIZE VMFS5_LVMINFO(size)
#define VMFS5_LVMINFO_OFFSET_BLKS VMFS5_LVMINFO(blocks)
#define VMFS5_LVMINFO_OFFSET_UUID_STR VMFS5_LVMINFO(uuidStr)
#define VMFS5_LVMINFO_OFFSET_UUID VMFS5_LVMINFO(uuid)
#define VMFS5_LVMINFO_OFFSET_NUM_SEGMENTS VMFS5_LVMINFO(numSegments)
#define VMFS5_LVMINFO_OFFSET_FIRST_SEGMENT VMFS5_LVMINFO(firstSegment)
#define VMFS5_LVMINFO_OFFSET_LAST_SEGMENT VMFS5_LVMINFO(lastSegment)
#define VMFS5_LVMINFO_OFFSET_NUM_EXTENTS VMFS5_LVMINFO(numExtents)

/*
 * Segment bitmap is at 0x80200.
 * Segment information are at 0x80600 + i * 0x80 for i between 0 and
 * VMFS5_LVMINFO_OFFSET_NUM_SEGMENTS
 *
 * At 0x10 (64-bits) or 0x14 (32-bits) within a segment info, it seems like
 * something related to the absolute segment number in the logical volume
 * (looks like absolute segment number << 4 on 32-bits).
 * Other segment information seem relative to the extent (always the same
 * pattern on all extents)
 */

    struct VmfsVolinfoS {
        uint32_t magic;
        uint32_t version;
        char *name;
        uuid_t uuid;
        int lun;

        uint32_t size;
        uint64_t lvmSize;
        uint64_t blocks;
        uuid_t lvmUuid;
        uint32_t numSegments, firstSegment, lastSegment, numExtents;
    };
    using VmfsVolinfoT = struct VmfsVolinfoS;

/* === VMFS mounted-volume === */
    struct VmfsVolumeS {
        VmfsDeviceT dev;
        char *device;
        int fd;
        VmfsFlagsT flags;
        int isBlkdev;
        int scsiReservation;

        /* VMFS volume base */
        off_t vmfsBase;

        /* Volume and FS information */
        VmfsVolinfoT volInfo;
    };
    using VmfsVolumeT = struct VmfsVolumeS;

    class VmfsVolume {
    public:
        static VmfsVolume *Instance();
        virtual ~VmfsVolume() = default;

        /* Open a VMFS volume */
        VmfsVolumeT *VolOpen(const char *filename, VmfsFlagsT flags);

    private:
        /* Read a raw block of data on logical volume */
        ssize_t VolRead(const VmfsDeviceT *dev, off_t pos, u_char *buf, size_t len);

        /* Volume reservation */
        int VolReserve(const VmfsDeviceT *dev, off_t pos);

        /* Volume release */
        int VolRelease(const VmfsDeviceT *dev, off_t pos);

        /*
         * Check if physical volume support reservation.
         * TODO: We should probably check some capabilities info.
         */
        int CheckReservation(VmfsVolumeT *vol);
        /* Read volume information */
        int VolinfoRead(VmfsVolumeT *volume);
        /* Close a VMFS volume */
        void VolClose(VmfsDeviceT *dev); // TODO - who calls this?

    private:
        VmfsVolume() = default;
        static VmfsVolume *m_instance;
        static std::mutex m_mutex;
    };
}

#endif
