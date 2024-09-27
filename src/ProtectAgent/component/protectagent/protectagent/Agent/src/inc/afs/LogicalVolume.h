#ifndef LV_LOGICALVOLUME_H_
#define LV_LOGICALVOLUME_H_

#include <queue>

#include "afs/AfsObject.h"
#include "afs/Segment.h"
#include "afs/ThinPoolBridge.h"
#include "afs/VolumeGroup.h"

#define NOT_SUPPORT_FORMAT (-65536)

class volumeGroup;

/**
 * @brief LogicalVolume：逻辑卷功能类,每个lv绑定一个分区reader
 */
class logicalVolume {
public:
    logicalVolume();
    logicalVolume(string &id, int32_t nsegs, string &vname, volumeGroup &pvg);

    virtual ~logicalVolume();

    void set_part_id(int32_t id)
    {
        m_part_id = id;
    }
    bool matchLvPartId(int32_t id);
    uint64_t lvmMapper(imgReader *reader, char *buf, uint64_t start_sectno, uint64_t count_sector,
        int32_t is_annotated);
    int32_t getBitMap(vector<BitMap *> &bitmap_vect);

    int64_t mapVaddrToPaddr(int64_t vaddr, int32_t &disk_id);
    int32_t verifyThinPoolSB();

    char *getFirstPVuuid();
    int32_t getFisrtPVindex();
    int32_t String2UUID(string uuid_string);

    string m_uuid;
    volumeGroup *m_this_group;

    string m_volname;
    int32_t m_segment_count;
    list<segment *> m_segments;

    vector<imgReader *> m_disk_readers_vect;

    int32_t m_part_id;
    bool m_is_visible;
    SEG_TYPE_ENU m_type;

    char m_first_pv_uuid[UUID_LEN + 1];
    uint32_t m_chunk_size;
    uint32_t m_stripe_size;

private:
    int32_t get_part_id()
    {
        return m_part_id;
    }
    int32_t segGetBitMap(vector<BitMap *> &bitmap_vect);
    int32_t lvGetBitMap(vector<BitMap *> &bitmap_vect);

    logicalVolume(logicalVolume &obj);
    logicalVolume &operator = (const logicalVolume &obj);
};
#endif /* LV_LOGICALVOLUME_H_ */
