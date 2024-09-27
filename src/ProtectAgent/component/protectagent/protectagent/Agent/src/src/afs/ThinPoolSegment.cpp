#include "afs/ThinPoolSegment.h"

/**
 * @brief 构造函数
 */
thinpoolSegment::thinpoolSegment() : segment()
{
    m_thinlv_device_id = 0;
}

/**
 * @brief 析构函数
 */
thinpoolSegment::~thinpoolSegment() {}

/**
 * @brief 根据名字寻找lv
 *
 * @param lv_name    lv名字
 *
 * @return NULL：未找到
 * lv:找到lv
 */
logicalVolume *thinpoolSegment::lookupLv(string lv_name)
{
    if (NULL == m_this_vg) {
        AFS_TRACE_OUT_ERROR("The space is NULL");
        return NULL;
    }

    vector<logicalVolume *>::iterator lv_iter;
    int32_t ret = 0;

    for (lv_iter = m_this_vg->m_lvolumes.begin(); lv_iter != m_this_vg->m_lvolumes.end(); lv_iter++) {
        // 2.1匹配meta卷
        ret = (*lv_iter)->m_volname.compare(lv_name);
        if (0 == ret) {
            return (*lv_iter);
        }
    }

    return NULL;
}

/**
 * @brief 寻找meta卷
 *
 * @return 返回meta卷
 */
logicalVolume *thinpoolSegment::findMetaLv()
{
    return lookupLv(m_sthin_metadata);
}

/**
 * @brief 寻找data卷
 *
 * @return：返回data卷
 */
logicalVolume *thinpoolSegment::findDataLv()
{
    return lookupLv(m_sthin_data);
}

/**
 * @brief 初始化segment
 *
 * @param info 传入参数
 *
 * @return
 * AFS_ERR_LVM_PART 错误lvm分区
 * AFS_SUCCESS 成功
 */
int32_t thinpoolSegment::initSegment(segment_init_info *info)
{
    AFS_TRACE_OUT_DBG("thinpoolSegment::initSegment");
    logicalVolume *lv = findMetaLv();
    if (NULL == lv) {
        AFS_TRACE_OUT_ERROR("Cann't find Meta Volume");
        return AFS_ERR_LVM_PART;
    }
    // 设置meta卷lv
    m_pb.setMetaLv(lv);

    lv = findDataLv();
    if (NULL == lv) {
        AFS_TRACE_OUT_ERROR("Cann't find data Volume");
        return AFS_ERR_LVM_PART;
    }
    // 设置data卷lv
    m_pb.setDataLv(lv);
    m_this_lv->m_chunk_size = m_pb.m_data_block_size;

    return AFS_SUCCESS;
}

/**
 * @brief 寻找块号
 *
 * @param reader 句柄
 * @param start_sectno 起始的扇区号
 * @param buf 缓存
 * @param count_sector 扇区统计
 *
 * @return 返回读到数据（扇区）
 */
int64_t thinpoolSegment::findBlock(imgReader *reader, uint64_t start_sectno, char *buf, uint64_t count_sector,
    int32_t is_annotated)
{
    int64_t read_size = 0;
    // 设置设备id
    m_pb.setDeviceId(m_thinlv_device_id);
    // 1.pool中data卷,利用phy_blk,找到数据
    read_size = m_pb.mappingDatablk(start_sectno, buf, count_sector);
    return read_size;
}

/**
 * @brief 虚地址到物理地址映射
 *
 * @param vaddr 虚地址
 *
 * @return 物理地址
 */
int64_t thinpoolSegment::mapVaddrToPaddr(int64_t vaddr, int32_t &disk_id)
{
    // 设置设备id
    m_pb.setDeviceId(m_thinlv_device_id);
    return m_pb.mappingVaddToPaddr(vaddr, disk_id);
}


/**
 * @brief 设置属性
 *
 * @param plvm
 *
 * @return
 * AFS_ERR_LVM_PART
 * AFS_SUCCESS
 */
int32_t thinpoolSegment::setSegProp(afsLVM *plvm)
{
    m_sthin_metadata = plvm->parseSegMetaData();
    if (m_sthin_metadata.empty()) {
        AFS_TRACE_OUT_ERROR("Can'nt parse meta data");
        return AFS_ERR_LVM_PART;
    }
    AFS_TRACE_OUT_DBG("find a thin pool xxx_tmeta [%s]", m_sthin_metadata.c_str());

    m_sthin_data = plvm->parseSegPool();
    if (m_sthin_data.empty()) {
        AFS_TRACE_OUT_ERROR("Can'nt parse data");
        return AFS_ERR_LVM_PART;
    }
    AFS_TRACE_OUT_DBG("find a thin pool xxx_tdata [%s]", m_sthin_data.c_str());

    m_pb.m_data_block_size = plvm->parseThinPoolSegChunkSize();
    if (0 == m_pb.m_data_block_size) {
        AFS_TRACE_OUT_ERROR("Can'nt parse the thin pool segment chunk size");
        return AFS_ERR_LVM_PART;
    }
    AFS_TRACE_OUT_DBG("the thin pool chunk size is %u", m_pb.m_data_block_size);

    return AFS_SUCCESS;
}
