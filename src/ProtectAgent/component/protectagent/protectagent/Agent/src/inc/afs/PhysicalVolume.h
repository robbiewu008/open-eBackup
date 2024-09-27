#ifndef PV_PHYSICALVOLUME_H_
#define PV_PHYSICALVOLUME_H_

#include "afs/AfsLVMType.h"

#include "afs/Afslibrary.h"

// On disk
typedef struct disk_locn {
    uint64_t size;   /* Bytes */
    uint64_t offset; /* Offset in bytes to start sector */
} __attribute__((packed)) disk_locn_t;

// Structure to hold Physical Volumes (PV) label
typedef struct pv_label_header {
    char pv_name[LVM_SIGLEN];        // Physical volume signature
    uint64_t pv_sector_xl;           // sector number of this label
    uint32_t pv_crc;                 // CRC value
    uint32_t pv_offset_xl;           // Offset from start of struct to contents
    char pv_vermagic[LVM_MAGIC_LEN]; // Physical Volume version "LVM2 001"
    char pv_uuid[UUID_LEN];
    //        uint64_t    pv_unknown1[5];             // documentation lacks for lvm

    /* NULL-terminated list of data areas followed by */
    /* NULL-terminated list of metadata area headers */
    struct disk_locn disk_areas_xl[2]; /* Two lists */
    uint64_t pv_unknown1;              // documentation lacks for lvm

    uint64_t pv_labeloffset; // location of the label
} __attribute__((__packed__)) PV_LABEL_HEADER;

typedef struct pv_label {
    uint32_t pv_magic;
    char pv_sig[4]; // signature
    uint64_t unknown1[2];
    uint64_t pv_offset_low;
    uint64_t unknown2;
    uint64_t pv_offset_high;
    uint64_t pv_length;
} __attribute__((__packed__)) PV_LABEL;

struct stripe {
    int32_t stripe_pv;
    uint32_t stripe_start_extent;
};

/**
 * @brief 物理卷描述类
 */
class physicalVolume {
public:
    physicalVolume();
    physicalVolume(string &sdev, int32_t part_num, string &id, uint64_t devsize, uint32_t start, uint32_t count,
        int32_t diskId, uint64_t dsk_offset);
    int32_t setPVUUID(char *temp_uuid);
    ~physicalVolume();

public:
    int32_t pv_num;
    int32_t part_id;
    int32_t disk_id; // 标注该PV所属的image or disk
    uint64_t dev_size;
    uint32_t pe_start, pe_count;
    uint64_t offset; // offset from the start of disk to lvm volume:单位扇区
    string uuid;
    string device;
    char pv_uuid2[UUID_LEN + 1];

private:
    physicalVolume(physicalVolume &obj);
    physicalVolume &operator = (const physicalVolume &obj);
};

#endif /* PV_PHYSICALVOLUME_H_ */
