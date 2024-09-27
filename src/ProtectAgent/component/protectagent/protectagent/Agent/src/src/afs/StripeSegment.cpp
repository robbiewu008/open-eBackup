#include <string.h>
#include <climits>
#include <algorithm>
#include <cerrno>
#include "afs/StripeSegment.h"
#include "afs/LogMsg.h"
#include "afs/LogicalVolume.h"

/**
 * @brief 构造体
 */
stripeSegment::stripeSegment() : segment()
{
    m_stripe_count = 0;
    m_stripe_size = 0;
    m_stripe_vector.clear();
    m_map_pvolumes_vector.clear();
}

/**
 * @brief 析构函数
 */
stripeSegment::~stripeSegment()
{
    m_stripe_vector.clear();
    m_map_pvolumes_vector.clear();
}

/**
 * @brief stripe, 计算可读取的扇区个数
 *
 * @param count_sector 待读取的扇区数
 * @param cnt_delta 差量
 *
 * @return
 * read_cnt：本次可读取的扇区数
 */
int64_t stripeSegment::stripeCalcReadCnt(uint64_t &count_sector, uint32_t stripe_mod, int64_t &cnt_delta)
{
    cnt_delta = static_cast<int64_t>(m_stripe_size - stripe_mod - count_sector);
    int64_t read_cnt = 0;

    if (cnt_delta >= 0) {
        // stripe 够读
        read_cnt = count_sector;
        count_sector = 0;
    } else {
        // stripe 不够读
        read_cnt = m_stripe_size - stripe_mod;
        count_sector -= read_cnt;
    }

    return read_cnt;
}

/**
 * @brief 计算读的个数
 *
 * @param count_sector 扇区统计
 * @param cnt_delta 差量
 *
 * @return
 * read_cnt：扇区数
 */
int64_t stripeSegment::calcReadCnt(uint64_t &count_sector, int64_t &cnt_delta)
{
    cnt_delta = static_cast<int64_t>(m_this_vg->m_extent_size - count_sector);
    int64_t read_cnt = 0;

    if (cnt_delta >= 0) {
        // stripe 够读
        read_cnt = count_sector;
    } else {
        // stripe 不够读
        read_cnt = cnt_delta * (-1);
        count_sector -= cnt_delta;
    }

    return read_cnt;
}

/**
 * @brief striep类型计算物理地址
 *
 * @param data ：数据
 * @param index: 寻找地址所在PV的下标
 * @param index_stripe: 一个LE第几组stripe_size
 *
 * @return  diskId: 物理地址所在的磁盘ID
 *
 */

int32_t stripeSegment::stripeMapPaddr(seg_strip2_internal_t &data, uint32_t index, uint32_t index_stripe)
{
    int32_t key = 0;
    int32_t diskId = -1;
    key = m_stripe_vector[index].stripe_pv;

    data.start_mapped += (m_stripe_vector[index]).stripe_start_extent * m_this_vg->m_extent_size;
    data.start_mapped += (uint64_t)(m_this_vg->m_extent_size) * (data.start_extentno);

    data.start_mapped += index_stripe * (uint64_t)m_stripe_size;
    data.start_mapped += (m_map_pvolumes_vector[key])->pe_start + (m_map_pvolumes_vector[key])->offset;

    diskId = (m_map_pvolumes_vector[key])->disk_id; // /PV所属磁盘号
    return diskId;
}

/**
 * @brief striep类型的 segment loop处理
 *
 * @param data ：数据
 *
 * @return
 * read_size： 读到的字节数
 * AFS_ERR_LVM_PART
 * AFS_ERR_IMAGE_READ
 */

int32_t stripeSegment::findStripeBlockDoLoop(seg_strip2_internal_t &data, int32_t is_annotated)
{
    if (m_map_pvolumes_vector.empty() || (m_this_vg->m_extent_size % m_stripe_size != 0)) {
        AFS_TRACE_OUT_ERROR("The pv space is NULL or m_stripe_count % seg_extent_end = %u",
            m_this_vg->m_extent_size % m_stripe_size);
        return AFS_ERR_LVM_PART;
    }

    uint64_t seg_stripe_end;
    int64_t read_size = 0;
    int64_t temp_read_size = 0;
    uint32_t index_stripe = 0;
    uint32_t stripe_mod = data.start_mapped;
    int32_t diskId = -1;
    uint64_t segment_last_extend_rows_no = m_extent_count / m_stripe_count;

    for (index_stripe = 0; index_stripe < m_this_vg->m_extent_size / m_stripe_size; index_stripe++) {
        for (uint32_t index = 0; index < m_stripe_count; index++) {
            // 第二级映射：找到stripe_size号
            seg_stripe_end = static_cast<uint64_t>(index_stripe) * m_stripe_count + static_cast<uint64_t>(index + 1);

            if (data.start_stripeno != seg_stripe_end) {
                // 不在此stripe，找下一个stripe
                continue;
            }

            diskId = stripeMapPaddr(data, index, index_stripe);

            int64_t cnt_delta = 0;
            int64_t read_cnt = stripeCalcReadCnt(data.count_sector, stripe_mod, cnt_delta); // /该函数正常不会返回复数
            if (0 > read_cnt) {
                AFS_TRACE_OUT_ERROR("Can not read calculate read number");
                return AFS_ERR_LVM_PART;
            }

            temp_read_size =
                (data.reader_vect[diskId])->readSector(data.buf + read_size * SECTOR_SIZE, data.start_mapped, read_cnt);
            if (read_cnt != temp_read_size) {
                AFS_TRACE_OUT_ERROR("Cannot read call back addr(%lld)", data.start_mapped);
                return AFS_ERR_IMAGE_READ;
            }
            read_size += temp_read_size;

            if (cnt_delta >= 0) {
                // 够读，退出
                return read_size;
            } else {
                // 不够读，继续读
                stripe_mod = (data.start_mapped + read_cnt) % m_stripe_size;
                data.start_mapped = stripe_mod;
                if (data.start_stripeno < (m_stripe_count * (m_this_vg->m_extent_size) / m_stripe_size)) {
                    data.start_stripeno += 1;
                } else if (data.start_extentno < segment_last_extend_rows_no - 1) {
                    data.start_stripeno = 1;
                    data.start_extentno += 1;
                } else {
                    return read_size; // /该 segment的LE已经读完，需要在下一个segment中读取
                }
            }
        }
    }

    return read_size;
}

/**
 * @brief loop处理
 *
 * @param data ：数据
 *
 * @return
 * read_size： 读到的字节数
 * AFS_ERR_LVM_PART
 * AFS_ERR_IMAGE_READ
 */
int32_t stripeSegment::findBlockDoLoop(seg_strip_internal_t &data)
{
    if (m_map_pvolumes_vector.empty()) {
        AFS_TRACE_OUT_ERROR("The pv space is NULL");
        return AFS_ERR_LVM_PART;
    }

    uint64_t seg_extent_end = 0;
    int64_t read_size = 0;
    uint32_t loop_cnt = m_extent_count / m_stripe_count + 1;
    uint32_t loop = 0;
    int32_t key = 0;
    int32_t disk_id = -1;

    while (loop < loop_cnt) {
        loop++;
        for (uint32_t index = 0; index < m_stripe_count; index++) {
            // 一个extent的处理
            // 判断在第index个segment
            seg_extent_end =
                static_cast<uint64_t>(m_start_extent) + index + static_cast<uint64_t>(m_stripe_count) * (loop - 1);

            if (data.start_extentno != seg_extent_end) {
                // 不在此stripe，找下一个stripe
                continue;
            }

            key = m_stripe_vector[index].stripe_pv;

            // 第二级映射：找到PV所在的extent号
            data.start_mapped += (m_stripe_vector[index]).stripe_start_extent * m_this_vg->m_extent_size;
            data.start_mapped += (uint64_t)(m_this_vg->m_extent_size) * (data.start_extentno - m_start_extent);
            data.start_mapped += (m_map_pvolumes_vector[key])->pe_start + (m_map_pvolumes_vector[key])->offset;

            disk_id = -1;
            disk_id = (m_map_pvolumes_vector[key])->disk_id;
            if (disk_id == -1) {
                AFS_TRACE_OUT_ERROR("disk id [%d] error", disk_id);
            }

            int64_t cnt_delta = 0;
            int64_t read_cnt = calcReadCnt(data.count_sector, cnt_delta);
            if (0 > read_cnt) {
                AFS_TRACE_OUT_ERROR("Cannot read calculate read number");
                return AFS_ERR_LVM_PART;
            }

            read_size +=
                (data.reader_vect[disk_id])->readSector(data.buf, data.start_mapped, read_cnt);  // 返回的单位是扇区
            if (read_cnt != read_size) {
                AFS_TRACE_OUT_ERROR("Cannot read call back addr(%lld)", data.start_mapped);
                return AFS_ERR_IMAGE_READ;
            }

            if (cnt_delta >= 0) {
                // 够读，退出
                return read_size;
            } else {
                // 不够读，继续读
                data.start_mapped = 0;
            }
        }
    }

    return read_size;
}

/**
 * @brief 映射块
 *
 * @param reader 句柄
 * @param start_sectno 开始的扇区号
 * @param buf 缓冲
 * @param count_sector 扇区计数
 *
 * @return
 * AFS_ERR_LVM_PART
 * 读到的字节数
 * 0：不在当前Segment
 */
int64_t stripeSegment::findStripeBlock(vector<imgReader *> &reader_vect, uint64_t start_sectno, char *buf,
    uint64_t count_sector, int32_t is_annotated)
{
    if (NULL == m_this_vg) {
        AFS_TRACE_OUT_ERROR("The space is NULL");
        return AFS_ERR_LVM_PART;
    }

    int64_t start_mapped = 0;
    uint32_t start_extentno = 0;
    uint32_t extent_remain = 0;
    uint32_t start_stripeno = 0;
    uint32_t extent_size = m_this_vg->m_extent_size;

    // 第一级映射：找到VG管理extent

    start_extentno = start_sectno / extent_size;
    extent_remain = start_sectno % extent_size;

    if (m_start_extent > start_extentno || (m_start_extent + m_extent_count - 1) < start_extentno) {
        return 0; // 数据不在当前Segment
    }

    start_extentno = (start_sectno - m_start_extent * extent_size) / (extent_size * m_stripe_count);
    extent_remain = (start_sectno - m_start_extent * extent_size) % (extent_size * m_stripe_count);
    start_stripeno = extent_remain / (m_stripe_size) + 1;
    start_mapped = extent_remain % m_stripe_size; // /不足一个m_stripe_size(扇区)的大小

    if (is_annotated) {
        AFS_TRACE_OUT_DBG("findStripeBlock: start_sectno = %u, start_extentno =%u, extent_remain =%u, start_mapped "
                          "first = %lld, start_stripeno =%u",
            start_sectno,
            start_extentno,
            extent_remain,
            start_mapped,
            start_stripeno);
    }

    seg_strip2_internal_t data = { reader_vect, buf, count_sector, start_extentno, start_stripeno, start_mapped };
    return findStripeBlockDoLoop(data, is_annotated);
}

/**
 * @brief 映射块
 *
 * @param reader 句柄
 * @param start_sectno 开始的扇区号
 * @param buf 缓冲
 * @param count_sector 扇区计数
 *
 * @return
 * AFS_ERR_LVM_PART
 * 读到的字节数
 * 0：不在当前Segment
 */
int64_t stripeSegment::findBlock(imgReader *reader, uint64_t start_sectno, char *buf, uint64_t count_sector,
    int32_t is_annotated)
{
    if (m_stripe_count > 1) {
        int64_t read_size =
            findStripeBlock(m_this_lv->m_disk_readers_vect, start_sectno, buf, count_sector, is_annotated);
        return read_size;
    }

    if (NULL == m_this_vg) {
        AFS_TRACE_OUT_ERROR("The space is NULL");
        return AFS_ERR_LVM_PART;
    }

    int64_t start_mapped = 0;
    uint32_t start_extentno = 0;
    uint32_t extent_remain = 0;

    // 第一级映射：找到VG管理extent号
    start_extentno = start_sectno / m_this_vg->m_extent_size;
    extent_remain = start_sectno % m_this_vg->m_extent_size;
    start_mapped = extent_remain;

    if (m_start_extent > start_extentno || (m_start_extent + m_extent_count - 1) < start_extentno) {
        AFS_TRACE_OUT_INFO("The virtual block address is not in this segment,we'll find next segment.");
        return 0; // 数据不在当前Segment
    }

    seg_strip_internal_t data = { m_this_lv->m_disk_readers_vect, buf, count_sector, start_extentno, start_mapped };
    return findBlockDoLoop(data);
}

/**
 * @brief 虚拟地址到物理地址映射
 *
 * @param vaddr 虚地址
 *
 * @return
 * 映射地址
 * -1: 不在当前segment，需要上层继续查找
 * AFS_ERR_LVM_PART：分析错误
 */
int64_t stripeSegment::mapStripeVaddrToPaddr(int64_t vaddr, int32_t &disk_id)
{
    if (NULL == m_this_vg || m_map_pvolumes_vector.empty()) {
        AFS_TRACE_OUT_ERROR("The space is NULL");
        return AFS_ERR_LVM_PART;
    }

    uint32_t index_stripe = 0;
    int64_t start_mapped = -1;
    uint32_t start_extentno = 0;
    uint32_t extent_remain = 0;
    uint32_t start_stripeno = 0;
    uint64_t seg_stripe_end = 0;
    uint32_t extent_size = m_this_vg->m_extent_size;

    // 第一级映射：找到VG管理extent
    start_extentno = vaddr / extent_size;
    extent_remain = vaddr % extent_size;

    if (m_start_extent > start_extentno || (m_start_extent + m_extent_count - 1) < start_extentno) {
        return -1; // 数据不在当前Segment
    }

    start_extentno = (vaddr - m_start_extent * extent_size) / (extent_size * m_stripe_count);
    extent_remain = (vaddr - m_start_extent * extent_size) % (extent_size * m_stripe_count);
    start_stripeno = extent_remain / (m_stripe_size) + 1;
    start_mapped = extent_remain % m_stripe_size; // /不足一个m_stripe_size(扇区)的大小
    seg_strip2_internal_t data = {
        m_this_lv->m_disk_readers_vect, NULL, 0, start_extentno, start_stripeno, start_mapped
    };

    for (index_stripe = 0; index_stripe < extent_size / m_stripe_size; index_stripe++) {
        for (uint32_t index = 0; index < m_stripe_count; index++) {
            // 第二级映射：找到stripe_size号
            seg_stripe_end = static_cast<uint64_t>(index_stripe) * m_stripe_count + static_cast<uint64_t>(index + 1);

            if (data.start_stripeno != seg_stripe_end) {
                // 不在此stripe，找下一个stripe
                continue;
            }
            disk_id = stripeMapPaddr(data, index, index_stripe);
            return data.start_mapped;
        }
    }

    return AFS_ERR_LVM_PART;
}

/**
 * @brief 虚拟地址到物理地址映射
 *
 * @param vaddr 虚地址(单位：*扇区)
 *
 * @return
 * 映射地址
 * -1: 不在当前segment，需要上层继续查找
 * AFS_ERR_LVM_PART：分析错误
 */
int64_t stripeSegment::mapVaddrToPaddr(int64_t vaddr, int32_t &disk_id)
{
    int64_t start_mapped = AFS_ERR_LVM_PART;
    if (m_stripe_count > 1) {
        start_mapped = mapStripeVaddrToPaddr(vaddr, disk_id);
        return start_mapped;
    }

    if (NULL == m_this_vg || m_map_pvolumes_vector.empty()) {
        AFS_TRACE_OUT_ERROR("The space is NULL");
        return AFS_ERR_LVM_PART;
    }

    volumeGroup *pVG = m_this_vg;
    uint64_t start_extentno = 0;
    uint64_t extent_remain = 0;
    uint64_t seg_extent_end = 0;
    int32_t key = 0;

    // 扇区单位---转本地---segment单位
    start_extentno = (uint64_t)vaddr / pVG->m_extent_size;
    extent_remain = (uint64_t)vaddr % pVG->m_extent_size;

    if (m_start_extent > start_extentno || (m_stripe_count == 0) ||
        (m_start_extent + m_extent_count - 1) < start_extentno) {
        return -1;
    }

    uint32_t loop_cnt = m_extent_count / m_stripe_count + 1;
    uint32_t loop = 0;

    while (loop < loop_cnt) {
        loop++;
        for (uint32_t index = 0; index < m_stripe_count; index++) {
            // 1.判断是否在本segment里面
            seg_extent_end =
                static_cast<uint64_t>(m_start_extent) + index + static_cast<uint64_t>(m_stripe_count) * (loop - 1);
            if (start_extentno != seg_extent_end) {
                // 不在此stripe，找下一个stripe
                continue;
            }

            key = m_stripe_vector[index].stripe_pv;
            start_mapped = extent_remain;
            // 第二级映射：找到PV所在的extent号
            start_mapped += (start_extentno - m_start_extent) * m_this_vg->m_extent_size;
            start_mapped += ((m_stripe_vector[index]).stripe_start_extent) * pVG->m_extent_size;
            start_mapped += (m_map_pvolumes_vector[key])->pe_start + (m_map_pvolumes_vector[key])->offset;

            disk_id = (m_map_pvolumes_vector[key])->disk_id; // 执行到此处肯定是单磁盘场景
            return start_mapped;
        }
    }
    return AFS_ERR_LVM_PART;
}

/**
 * @brief 获取BitMap
 *
 * @param bitmap 位图
 *
 * @return
 * AFS_ERR_LVM_PART
 * AFS_SUCCESS
 */
int32_t stripeSegment::getBitMap(vector<BitMap *> &bitmap_vect)
{
    if (NULL == m_this_vg) {
        AFS_TRACE_OUT_ERROR("The space is NULL");
        return AFS_ERR_LVM_PART;
    }

    // 将整个Segment设置
    int64_t start_addr = 0;
    uint64_t seg_extent_size = 0;
    volumeGroup *pVG = m_this_vg;
    int32_t ret = AFS_SUCCESS;
    int32_t key = 0;
    int32_t disk_id = -1;
    BitMap *pBitmap = NULL;

    uint32_t index = 0;
    for (; index < m_stripe_count; index++) {
        // /真实striped模式下，每个PV在segment中分配的PE数量
        seg_extent_size = m_extent_count / m_stripe_count;

        disk_id = -1;
        key = m_stripe_vector[index].stripe_pv;
        disk_id = (m_map_pvolumes_vector[key])->disk_id;
        if (-1 == disk_id) {
            AFS_TRACE_OUT_ERROR("LV[%s] stripeSegment::getBitMap() failed, disk_id = %d", m_this_lv->m_volname.c_str(),
                disk_id);
            return AFS_ERR_LVM_PART;
        }

        start_addr = ((m_stripe_vector[index]).stripe_start_extent) * pVG->m_extent_size;
        start_addr += (m_map_pvolumes_vector[key])->pe_start + (m_map_pvolumes_vector[key])->offset;

        pBitmap = bitmap_vect[disk_id];
        ret = pBitmap->bitmapSetRange(start_addr, seg_extent_size * (pVG->m_extent_size), 1); // /单位：扇区
        if (ret < 0) {
            AFS_TRACE_OUT_ERROR("Can'nt support format");
            return ret;
        }
    }

    return ret;
}

/**
 * @brief 获取lv分区地址
 *
 * @return 地址
 */
int64_t stripeSegment::getPartitionFirstAddr()
{
    int32_t disk_id = -1;
    return mapVaddrToPaddr(0, disk_id);
}

/**
 * @brief 初始化处理
 *
 * @param info 数据
 *
 * @return
 * AFS_ERR_LVM_PART
 * AFS_SUCCESS
 */
int32_t stripeSegment::initSegment(segment_init_info *info)
{
    // 越界检查
    if ((static_cast<size_t>(m_stripe_count) > (m_stripe_vector.size() + 1)) ||
        (static_cast<size_t>(m_stripe_count) > (m_map_pvolumes_vector.size() + 1))) {
        AFS_TRACE_OUT_ERROR("The vector space is not enough.");
        return AFS_ERR_LVM_PART;
    }

    m_this_lv->m_chunk_size = m_this_vg->m_extent_size;

    return AFS_SUCCESS;
}

/**
 * @brief 设置属性
 *
 * @param plvm lvm类
 *
 * @return
 * AFS_ERR_LVM_PART
 * AFS_SUCCESS
 */
int32_t stripeSegment::setSegProp(afsLVM *plvm)
{
    // 设置stripe_count
    uint32_t istripe_count = plvm->parseSegStripeCnt();
    if (0 == istripe_count) {
        AFS_TRACE_OUT_ERROR("Can't parse stripe_count of segment");
        return AFS_ERR_LVM_PART;
    }

    m_stripe_count = istripe_count;

    if (m_stripe_count > 1) {
        uint32_t stripe_size = plvm->parseSegStripeSize();
        if (0 == stripe_size) {
            AFS_TRACE_OUT_ERROR("Can't parse stripe_size of segment");
            return AFS_ERR_LVM_PART;
        }
        m_stripe_size = stripe_size;
    }

    // 统计count数目
    int32_t ret = parseSegStripedPropCalc(plvm);
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Can't calculate stripe_count of segment");
        return ret;
    }

    return AFS_SUCCESS;
}

/**
 * @brief stripe count计算
 *
 * @param *plvm    parent LVM对象指针
 *
 * @return
 * AFS_SUCCESS: 成功
 * 负数：错误ID
 *
 */
int32_t stripeSegment::parseSegStripedPropCalc(afsLVM *plvm)
{
    int loop = m_stripe_count;

    while (loop--) {
        stripe temp_stripe;

        int32_t pv_index = plvm->parseSegPVIndex();
        if (0 > pv_index) {
            AFS_TRACE_OUT_INFO("Can'nt parse lvm metadata");
            return AFS_SUCCESS;
        }

        temp_stripe.stripe_pv = pv_index;

        // PV开始的extent
        temp_stripe.stripe_start_extent = plvm->parseSegStartExtent();
        if (0 > static_cast<int32_t>(temp_stripe.stripe_start_extent)) {
            AFS_TRACE_OUT_ERROR("Can'nt parse lvm metadata");
            return AFS_ERR_LVM_PART;
        }

        // 设置数组(二者一一对应)
        if (static_cast<size_t>(pv_index + 1) > plvm->m_vg->m_pvolumes.size()) {
            AFS_TRACE_OUT_ERROR("LVM pv_index is bigger than pvolumes_vector");
            return AFS_ERR_LVM_PART;
        }

        physicalVolume *ppv = (plvm->m_vg->m_pvolumes)[pv_index];

        m_map_pvolumes_vector.insert(pair<int32_t, physicalVolume *>(pv_index, ppv));
        m_stripe_vector.push_back(temp_stripe);
    }

    return AFS_SUCCESS;
}
