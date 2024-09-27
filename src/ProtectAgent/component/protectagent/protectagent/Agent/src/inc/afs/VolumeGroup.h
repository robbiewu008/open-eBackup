#ifndef VG_VOLUMEGROUP_H_
#define VG_VOLUMEGROUP_H_

#include <cerrno>
#include "afs/Bitmap.h"
#include "afs/PhysicalVolume.h"

class logicalVolume;

/**
 * @brief 卷组
 */
class volumeGroup {
public:
    /* *
     * @brief 构造函数
     */
    volumeGroup(string &id, string &name, int32_t seq, int32_t size);
    volumeGroup();
    /* *
     * @brief 析构函数
     */
    ~volumeGroup();

    physicalVolume *findPhysicalVolume(string &id);
    physicalVolume *addPhysicalVolume(string &sdev, string &id, uint64_t devsize, uint32_t start, uint32_t count,
        uint64_t disk_offset, int32_t disk_id, int32_t part_index, char *uuid2);
    logicalVolume *findLogicalVolume(string &id);
    logicalVolume *findThinLogicalVolume(string &uuid_type);
    logicalVolume *addLogicalVolume(string &id, int32_t count, string &vname);
    void updatePVAddr(int32_t part_index, uint64_t m_pv_offset);
    int32_t getBitmap(vector<BitMap *> &bitmap_vect);

public:
    string m_volname;
    string m_uuid;
    string m_format;
    // 单位为扇区
    uint32_t m_extent_size;
    int32_t m_seqno;
    int32_t m_max_lv;
    int32_t m_max_pv;

    // 分区设备：/dev/sda /dev/vda
    string m_dev_name;

    // 跨分区标志位
    bool m_span_part_flag;

    vector<int32_t> m_pv_part_id;

    vector<physicalVolume *> m_pvolumes;
    vector<logicalVolume *> m_lvolumes;

private:
    /* *
     * @brief 拷贝构造函数
     */
    volumeGroup(volumeGroup &obj);
    /* *
     * @brief 赋值构造函数
     */
    volumeGroup &operator = (const volumeGroup &obj);

    void splitStrAndNum(string src, string &dest, int32_t &num);
};

#endif /* VG_VOLUMEGROUP_H_ */
