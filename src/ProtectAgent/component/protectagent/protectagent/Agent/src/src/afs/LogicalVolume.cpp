#include "afs/PartitionHandler.h"
#include "afs/LogicalVolume.h"
#include "afs/StripeSegment.h"
#include "afs/ThinPoolSegment.h"

/**
 * @brief 构造函数
 */
logicalVolume::logicalVolume()
{
    m_uuid = "";
    m_segment_count = 0;
    m_part_id = -1;
    m_volname = "";
    m_this_group = NULL;
    m_is_visible = false;
    m_type = SEG_THIN_POOL;
    m_segments.clear();
    m_chunk_size = 0;
    m_stripe_size = 0;
}

/* * @brief  带参构造函数
 *
 * @param id: uuid
 * @param nsegs: segment计数
 * @param vname: vloume名字
 * @param pvg: 卷空间
 *
 */
logicalVolume::logicalVolume(string &id, int32_t nsegs, string &vname, volumeGroup &pvg)
{
    m_uuid = id;
    m_segment_count = nsegs;
    m_this_group = &pvg;
    m_volname = vname;
    m_part_id = -1;
    m_is_visible = false;
    m_type = SEG_THIN_POOL;
    m_segments.clear();
    m_chunk_size = 0;
    m_stripe_size = 0;
}

/**
 * @brief 析构函数
 *
 */
logicalVolume::~logicalVolume()
{
    list<segment *>::iterator it_segments;

    for (it_segments = m_segments.begin(); it_segments != m_segments.end(); ++it_segments) {
        if (NULL != (*it_segments)) {
            delete (*it_segments);
            (*it_segments) = NULL;
        }
    }

    m_segments.clear();
    m_this_group = NULL;
}

/**
 * @brief 获取该LV的第一个PE所在的PV 的下标，后续更新时需要
 *
 * @return 返回PV下标， -1表示函数执行失败
 */

int32_t logicalVolume::getFisrtPVindex()
{
    int32_t pv_index;
    stripeSegment *pStripeSeg;
    stripe temp_stripe;

    if (m_type == SEG_LINEAR || m_type == SEG_STRIPED) {
        if (m_segments.size()) {
            pStripeSeg = dynamic_cast<stripeSegment *>(m_segments.front());
            temp_stripe = pStripeSeg->m_stripe_vector.at(0);
            pv_index = temp_stripe.stripe_pv;
            return pv_index;
        }
    }

    return -1; // /不支持格式
}

/**
 * @brief PV UUID string 转换成 UUID标准格式
 * string格式："\"o3OAi1-IUIz-0Svv-vum1-kknj-kHp3-sVT4qq\""
 * UUID格式  ：  o3OAi1IUIz0Svvvum1kknjkHp3sVT4qq
 *
 * @return 返回转换结果，0表示成功，-1 表示失败
 */

int32_t logicalVolume::String2UUID(string uuid_string)
{
    const char *pUUID_string = uuid_string.c_str();
    int32_t m_first_pv_uuid_len = UUID_LEN + 1;

    if (uuid_string.size() != 40 || sizeof(m_first_pv_uuid) != UUID_LEN + 1) {
        AFS_TRACE_OUT_ERROR("The size of String[%d] or UUID[%d] is not right", uuid_string.size(),
            sizeof(m_first_pv_uuid));
        return -1;
    }

    CHECK_MEMCPY_S_OK(m_first_pv_uuid, m_first_pv_uuid_len, pUUID_string + 1, 6);
    CHECK_MEMCPY_S_OK(m_first_pv_uuid + 6, m_first_pv_uuid_len - 6, pUUID_string + 8, 4);
    CHECK_MEMCPY_S_OK(m_first_pv_uuid + 10, m_first_pv_uuid_len - 10, pUUID_string + 13, 4);
    CHECK_MEMCPY_S_OK(m_first_pv_uuid + 14, m_first_pv_uuid_len - 14, pUUID_string + 18, 4);
    CHECK_MEMCPY_S_OK(m_first_pv_uuid + 18, m_first_pv_uuid_len - 18, pUUID_string + 23, 4);
    CHECK_MEMCPY_S_OK(m_first_pv_uuid + 22, m_first_pv_uuid_len - 22, pUUID_string + 28, 4);
    CHECK_MEMCPY_S_OK(m_first_pv_uuid + 26, m_first_pv_uuid_len - 26, pUUID_string + 33, 6);
    m_first_pv_uuid[UUID_LEN] = '\0';
    return 0;
}

/**
 * @brief 获取该LV的第一个PE所在的PV UUID，后续更新时需要
 *
 * @return 返回PV UUID的地址，NULL表示函数执行失败
 */
char *logicalVolume::getFirstPVuuid()
{
    int32_t pv_index;
    physicalVolume *pPhyVolume;
    if (m_type == SEG_LINEAR || m_type == SEG_STRIPED) {
        pv_index = getFisrtPVindex();
        if (pv_index != -1 && (uint32_t)pv_index < m_this_group->m_pvolumes.size()) {
            pPhyVolume = m_this_group->m_pvolumes[pv_index];
            String2UUID(pPhyVolume->uuid);
            return m_first_pv_uuid;
        }
    }
    return NULL;
}

/**
 * @brief 地址转换函数
 *
 * @param reader reader句柄
 * @param buf 传入的缓存
 * @param start_sectno 开始的扇区号
 * @param count_sector 要读多少扇区
 *
 * @return 返回读取的字节
 *
 */
uint64_t logicalVolume::lvmMapper(imgReader *reader, char *buf, uint64_t start_sectno, uint64_t count_sector,
    int32_t is_annotated)
{
    uint64_t read_size = 0;
    int64_t read_ret = 0;
    segment *seg = NULL;
    list<segment *>::iterator iterate;
    SEG_TYPE_ENU type = SEG_NULL;

    for (iterate = m_segments.begin(); iterate != m_segments.end(); ++iterate) {
        if (NULL == (*iterate)) {
            AFS_TRACE_OUT_ERROR("The Segment space is NULL.");
            return AFS_ERR_LVM_PART;
        }

        seg = (*iterate);
        type = seg->getType();
        if (type != SEG_LINEAR && type != SEG_STRIPED && type != SEG_THIN_POOL && type != SEG_THIN) {
            AFS_TRACE_OUT_ERROR("The Segment space is NULL. type is %d", type);
            return AFS_ERR_LVM_PART;
        }
        // segment模式(用户看到：stripe line Thin-lv 看不到的：thin-pool thin-meta thin-data)
        read_ret =
            seg->findBlock(reader, start_sectno, buf + read_size * SECTOR_SIZE, count_sector - read_size, is_annotated);
        if (read_ret < 0) {
            AFS_TRACE_OUT_ERROR("Failed to read disk. ret = %lld", (long long)read_ret);
            return 0; // 返回read size 0
        }

        read_size += read_ret;
        if ((read_ret == 0) || (read_size < count_sector)) {
            start_sectno = start_sectno + (uint64_t)read_size;
            continue;
        } else {
            // 读完结束
            break;
        }
    }

    return read_size;
}

/**
 * @brief 匹配lv的id
 *
 * @param id lv设备id号
 *
 * @return bool
 */
bool logicalVolume::matchLvPartId(int32_t id)
{
    return (m_part_id == id);
}

/**
 * @brief 获取segment BitMap
 *
 * @param bitmap 位图
 *
 * @return
 * AFS_ERR_LVM_PART
 * AFS_SUCCESS
 */
int32_t logicalVolume::segGetBitMap(vector<BitMap *> &bitmap_vect)
{
    bool bret = true;
    int32_t ret = AFS_SUCCESS;

    // 遍历每个segment, pool卷处理, 元数据lv
    list<segment *>::iterator iterate = m_segments.begin();
    for (; iterate != m_segments.end(); ++iterate) {
        if (NULL == (*iterate)) {
            AFS_TRACE_OUT_ERROR("The Segment space is NULL.");
            return AFS_ERR_LVM_PART;
        }

        bret = (*iterate)->isThinPoolMode();
        if (true == bret) {
            AFS_TRACE_OUT_DBG("handle the metadata of thin-pool LV[%s]", m_volname.c_str());
            thinpoolSegment *pSegPool = dynamic_cast<thinpoolSegment *>(*iterate);
            if (NULL == pSegPool) {
                AFS_TRACE_OUT_ERROR("The lowreader is NULL");
                return AFS_ERR_LVM_PART;
            }

            ret = pSegPool->m_pb.getBitMap(bitmap_vect);
            if (ret < 0) {
                AFS_TRACE_OUT_ERROR("Can'nt support format.");
                return AFS_ERR_LVM_PART;
            }
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief 获取保留lv BitMap
 *
 * @param bitmap 位图
 *
 * @return
 * AFS_ERR_LVM_PART
 * AFS_SUCCESS
 */
int32_t logicalVolume::lvGetBitMap(vector<BitMap *> &bitmap_vect)
{
    list<segment *>::iterator iterate;
    int32_t ret = AFS_SUCCESS;

    /* *
     * To create the pmspare ("pool metadata spare") LV, lvm first creates
       an LV with a default name, e.g. lvol0, and then converts this LV to a
       hidden LV with the _pmspare suffix, e.g. lvol0_pmspare.
       The "Metadata check and repair" section describes the use of the
       pmspare LV.
     */
    // pool-meta卷处理"xxx_pmspare"处理
    string::size_type sret = m_volname.find("_pmspare");
    if (sret != string::npos) {
        AFS_TRACE_OUT_DBG("handle thin pool xxx_pmspare LV [%s]", m_volname.c_str());
        stripeSegment *psegs = NULL;

        for (iterate = m_segments.begin(); iterate != m_segments.end(); ++iterate) {
            psegs = dynamic_cast<stripeSegment *>(*iterate);
            if (NULL == psegs) {
                AFS_TRACE_OUT_ERROR("The Segment space is NULL.");
                return AFS_ERR_LVM_PART;
            }

            ret = psegs->getBitMap(bitmap_vect);
            if (ret < 0) {
                AFS_TRACE_OUT_ERROR("Can'nt support format.");
                return AFS_ERR_LVM_PART;
            }
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief 验证thin-pool的_tmeta元数据卷组的super block是否正确
 *
 * @return
 * AFS_ERR_LVM_PART
 * AFS_SUCCESS
 */
int32_t logicalVolume::verifyThinPoolSB()
{
    int32_t ret = AFS_ERR_LVM_PART;
    if (m_segment_count != 1) { // thin-pool必须只有一个segment
        AFS_TRACE_OUT_ERROR("thin-pool LV's m_segment_count !=1 ");
        return AFS_ERR_LVM_PART;
    }

    thinpoolSegment *pThinPoolSeg = dynamic_cast<thinpoolSegment *>(m_segments.back());
    ret = pThinPoolSeg->m_pb.parseSb();
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Can not parse thin pool lv super block");
        return ret;
    }

    return ret;
}

/**
 * @brief thin-pool中元数据需要设置BitMap
 *
 * @param bitmap 位图
 *
 * @return
 * AFS_ERR_LVM_PART
 * AFS_SUCCESS
 */
int32_t logicalVolume::getBitMap(vector<BitMap *> &bitmap_vect)
{
    int32_t ret = AFS_SUCCESS;

    ret = segGetBitMap(bitmap_vect);
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Can'nt set segment bitmap");
        return ret;
    }

    ret = lvGetBitMap(bitmap_vect);
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Can'nt set lv bitmap");
        return ret;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 逻辑地址映射物理地址
 *
 * @param vaddr 虚拟地址
 *
 * @return
 * -1
 * offset
 */
int64_t logicalVolume::mapVaddrToPaddr(int64_t vaddr, int32_t &disk_id)
{
    list<segment *>::iterator iterate;
    int64_t offset = -1;
    // 字节转化为扇区
    int64_t vaddr_sect = vaddr >> 9;

    for (iterate = m_segments.begin(); iterate != m_segments.end(); ++iterate) {
        if (NULL == (*iterate)) {
            AFS_TRACE_OUT_ERROR("Failed to get segment space");
            return -1;
        }

        offset = (*iterate)->mapVaddrToPaddr(vaddr_sect, disk_id);
        if (-1 == offset) {
            continue; // 不在当前segment
        } else if (LVMNODATA == offset) {
            return LVMNODATA;
        } else if (offset < 0) {
            AFS_TRACE_OUT_ERROR("Failed to convert virtual to disk. ret = %lld", (long long)offset);
            return -1;
        }
        break;
    }
    return offset;
}
