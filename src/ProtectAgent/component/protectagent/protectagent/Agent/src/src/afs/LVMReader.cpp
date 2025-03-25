#include "afs/LVMReader.h"
#include "afs/LogMsg.h"
#include "afs/PartitionHandler.h"
#include "securec.h"

/**
 * @brief 读取硬盘内容，按照文件系统的块大小为单位
 * @param *buf   读取缓存
 * @param offset 分区偏移
 * @param count  读取长度(字节)
 * @return 0 成功  负数 失败
 */
int64_t lvmReader::read(void *buf, int64_t offset, int64_t count, int32_t is_annotated)
{
    if (NULL == buf || NULL == m_lvol || NULL == m_lowreader) {
        AFS_TRACE_OUT_ERROR("LVM Reader can'nt work, because don't have a buff or a handler of lvm");
        return -1;
    }

    uint64_t ret_size = 0;
    uint64_t uoffset = static_cast<uint64_t>(offset);
    // 减去所在分区偏移
    uoffset -= m_part_info.offset * SECTOR_SIZE;

    if (is_annotated) {
        AFS_TRACE_OUT_DBG("lvmReader: uoffset is %llu(bytes), m_part_info.offset = %llu(sectors), count is %lld(bytes)",
            uoffset, m_part_info.offset, count);
    }

    uint64_t start_no = uoffset / SECTOR_SIZE;
    uint64_t remain_offset = uoffset % SECTOR_SIZE;

    uint64_t align_count = count % SECTOR_SIZE;
    uint64_t end_no = (count + SECTOR_SIZE - 1) / SECTOR_SIZE;
    if (0 == end_no) {
        AFS_TRACE_OUT_ERROR("The size is zero");
        return 0;
    }

    if (0 != align_count) {
        // count没对齐
        end_no += 1;
    }

    char *read_buf = new char[end_no * SECTOR_SIZE]();
    if (NULL == read_buf) {
        AFS_TRACE_OUT_ERROR("Cannt new space");
        return 0;
    }

    // 优化方案：找到连续空间属于的PV，进行更大的空间操作(进行分段处理)
    ret_size = m_lvol->lvmMapper(static_cast<imgReader *>(m_lowreader), read_buf, start_no, end_no, is_annotated);
    if (ret_size != end_no) {
        delete[] read_buf;
        AFS_TRACE_OUT_ERROR("Don't read image");
        return -1;
    }

    if (EOK != memcpy_s((char *)buf, count, (read_buf + remain_offset), count)) {
        delete[] read_buf;
        read_buf = NULL;
        return -1;
    }

    delete[] read_buf;
    read_buf = NULL;

    return count;
}

/**
 * @brief 初始化reader
 *
 * @param *buf   数据缓存
 * @param offset 偏移
 * @param count  长度
 * @return 大于等于0： 读取成功   负数：读取失败
 *
 */
int64_t lvmReader::readDisk(void *buf, int64_t offset, int64_t count)
{
    if (NULL == m_lowreader) {
        AFS_TRACE_OUT_ERROR("lvm must be a low reader");
        return 0;
    }

    // 偏移：分区偏移 + lv的偏移
    int64_t newoffset = offset + static_cast<int64_t>(m_part_info.lvm_info.lv_offset) * SECTOR_SIZE;
    return (static_cast<imgReader *>(m_lowreader))->read(buf, newoffset, count, 0);
}

/**
 * @brief 虚地址到物理地址转换
 * @param vaddr
 * @return (单位: 扇区)
 */
int64_t lvmReader::getVaddrToPaddr(int64_t vaddr, int32_t &disk_id)
{
    if (NULL == m_lvol) {
        AFS_TRACE_OUT_ERROR("lvm must be a lvm volume handler");
        return -1;
    }

    // 目前需要装换的只有lvm模式
    return m_lvol->mapVaddrToPaddr(vaddr - m_imageinfo.offset * SECTOR_SIZE, disk_id);
}

/**
 * @brief 初始化imageinfo信息
 * @return
 */
int32_t lvmReader::initImageInfo()
{
    if (NULL == m_lowreader) {
        AFS_TRACE_OUT_ERROR("lvm must be a lvm volume handler");
        return AFS_ERR_INNER;
    }

    imgReader *parentReader = dynamic_cast<imgReader *>(m_lowreader);
    if (NULL == parentReader) {
        AFS_TRACE_OUT_ERROR("The lowreader is NULL");
        return AFS_ERR_INNER;
    }

    m_imageinfo.formattype = parentReader->m_imageinfo.formattype;
    m_imageinfo.length = parentReader->m_imageinfo.length;
    m_imageinfo.offset = m_part_info.offset;

    return AFS_SUCCESS;
}
