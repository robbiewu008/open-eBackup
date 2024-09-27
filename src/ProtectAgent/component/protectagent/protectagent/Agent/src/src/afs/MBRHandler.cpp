#include "afs/MBRHandler.h"
#include "afs/RawReader.h"
#include "afs/AfsLVM.h"
#include "afs/LogMsg.h"

/**
 * @brief 扩展分区标志设置
 *
 * @param extent_lba  扩展分区逻辑地址
 *
 * @return 返回扩展分区标志标志
 *
 */
void MBRHandler::setExtentLba(uint64_t extent_lba)
{
    this->m_extent_lba = extent_lba;
}

/**
 * @brief 扩展分区标志获得
 * @return 返回扩展分区标志标志
 *
 */
uint64_t MBRHandler::getExtentLba()
{
    return this->m_extent_lba;
}

/**
 * @brief 分析单分区
 * @param &part         分区结构体
 * @param real_partNum  实际分区数
 * @param partNum       当前分区ID
 * @return
 * AFS_SUCCESS：分析成功
 * 负数：错误ID
 */
int32_t MBRHandler::analyzeExtentSetSinglePart(struct partition &part, int32_t real_partNum, int32_t partNum)
{
    int32_t ret = AFS_SUCCESS;
    int32_t lv_cnt = 0;

    // LVM格式判断
    // 初始化分区信息
    ret = setPartition(partNum - 1, &part);
    if (ret != AFS_SUCCESS) {
        return AFS_ERR_LVM_PART;
    }

    // LVM模式
    // LVM处理,LVM会对分区进行管理,重新设置分区偏移
    void *part_tmp = getPartitionPointer(partNum - 1);
    if (NULL == part_tmp) {
        AFS_TRACE_OUT_ERROR("Failed to get partition information.");
        return AFS_ERR_LVM_PART;
    }

    vector<struct partition> &partVect = getPartSpaceVect();
    afsLVM lvm(this, partVect, partNum - 1);
    lv_cnt = lvm.parseLVMFormat(real_partNum);
    if (NOT_LVM_FORMAT == lv_cnt) {
        // 非lvm格式分区
        ret = setPartition(partNum - 1, &part);
        if (ret != AFS_SUCCESS) {
            AFS_TRACE_OUT_ERROR("Set partition failed. index=%d", partNum - 1);
            return ret;
        }
    } else if (0 <= lv_cnt) {
        // LVM格式，分区加lv个数最为最终的号，设置分区标识
        setPartnumValue(partNum + lv_cnt);
    } else if (-1 == lv_cnt || AFS_VOLUME_GROUP_EXIST == lv_cnt || AFS_PV_NO_VG_METADAT == lv_cnt) {
        ret = AFS_SUCCESS;
    } else {
        AFS_TRACE_OUT_ERROR("Can't parse lvm part");
        return lv_cnt;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 分析分区信息
 * @param *buf              缓存数据
 * @param &que_extent       扩展分区队列
 * @param extentBase        扩展分区起始位置
 * @param extentL1          逻辑分区地址
 * @param real_part_index   分区ID
 * @return
 * AFS_ERR_LVM_PART
 * AFS_SUCCESS
 */
int32_t MBRHandler::analyzeExtentParsePart(unsigned char *buf, queue<uint64_t> &que_extent, uint64_t extentBase,
    uint64_t extentL1, int32_t real_part_index)
{
    int32_t ret = 0;
    int32_t partNum = 0;
    struct partitionOpt part;

    // 遍历扩展分区分区表中的两项
    for (int32_t i = 0; i < 2; i++) {
        struct dos_partition *dos_par = mbr_get_partition(buf, i);
        unsigned char sys_ind = dos_par->sys_ind;

        if (MBR_UNUSE_PARTITION_ENTRY == sys_ind) {
            // 分区识别为0，不一定代表错误
            AFS_TRACE_OUT_DBG("Not set the value of sys_ind is (%d). The Partition index=%d", 0, i);
            continue;
        }
        // /扩展分区则扩展分区地址加入队列
        if (MBR_EXTEND_PARTITION_1 == sys_ind || MBR_EXTEND_PARTITION_2 == sys_ind) {
            extentL1 = dos_partition_get_start(dos_par) + extentBase;
            que_extent.push(extentL1);
            continue;
        }

        // /逻辑分区记录
        partNum = getPartnum();
        ret = setPartnum(++partNum);
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("MBR analyzeExtentParsePart() set partitions num failed, ret = %d", ret);
            return ret;
        }

        AFS_TRACE_OUT_DBG("The %d Partiton Type is %d", partNum, sys_ind);

        AFS_FSTYPE_t pfstype = (MBR_SWAP_PARTITION == sys_ind) ? AFS_FILESYSTEM_SWAP : AFS_FILESYSTEM_NULL;

        part.setPartInfo(PARTITION_MBR, getPartnum() - 1, false, dos_partition_get_start(dos_par) + extentL1,
            dos_partition_get_size(dos_par), pfstype);

        AFS_TRACE_OUT_INFO("the %d parttion's offset is %llu, length is % llu", getPartnum(),
            dos_partition_get_start(dos_par) + extentL1, dos_partition_get_size(dos_par));

        ret = analyzeExtentSetSinglePart(part, real_part_index, partNum);
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("Cannt analyze partition of info");
            return ret;
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief  分析扩展分区并标记扩展分区所占bitmap
 * @return int32_t 0：设置成功
 * 负数： 设置失败
 */
int32_t MBRHandler::analyzeExtent()
{
    uint64_t extentBase = 0;
    uint64_t extentL1 = 0;
    int32_t ret = 0;
    int64_t read_size = 0;
    int32_t real_part_index = 5;

    queue<uint64_t> que_extent;
    extentBase = getExtentLba();
    if (!extentBase) {
        AFS_TRACE_OUT_DBG("the disk or image has no extend partition");
        return AFS_SUCCESS;
    }

    unsigned char buf[SECTOR_SIZE] = {0};

    que_extent.push(extentBase);
    imgReader *reader_tmp = getImgReader();
    if (NULL == reader_tmp) {
        AFS_TRACE_OUT_ERROR("Can't read image");
        return AFS_ERR_PARTITION;
    }

    // 广度优先遍历extent二叉树
    while (!que_extent.empty()) {
        extentL1 = que_extent.front();
        m_extentSect.push_back(extentL1);
        que_extent.pop();

        // 读取扩展分区所占扇区
        read_size = reader_tmp->read(buf, (int64_t)(extentL1 * SECTOR_SIZE), static_cast<int64_t>(SECTOR_SIZE), 1);
        if (static_cast<int64_t>(SECTOR_SIZE) != read_size) {
            AFS_TRACE_OUT_ERROR("Can't read image");
            return AFS_ERR_IMAGE_READ;
        }

        if (!mbr_is_valid_magic(buf)) {
            AFS_TRACE_OUT_ERROR("Failed to check MBR partition magic. buf[510]=%d, buf[511]=%d", buf[510], buf[511]);
            continue;
        }

        ret = analyzeExtentParsePart(buf, que_extent, extentBase, extentL1, real_part_index);
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("Can't analyze part info");
            return ret;
        }

        real_part_index++;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 分区表项预处理
 *
 * @param *data_buf  缓存数据
 *
 * @return 0:成功  负数：错误
 *
 */
int32_t MBRHandler::partitionEntryHandle(dos_partition *pMbr_partition, const char entry_index)
{
    unsigned char sys_ind = pMbr_partition->sys_ind;
    // step1:分区表项未使用
    if (MBR_UNUSE_PARTITION_ENTRY == sys_ind) {
        AFS_TRACE_OUT_DBG("Not set the value of sys_ind is (%d). The Partition index=%d", 0, entry_index);
        return AFS_SUCCESS;
    }

    // setp2:逻辑分区
    if (MBR_EXTEND_PARTITION_1 == sys_ind || MBR_EXTEND_PARTITION_2 == sys_ind) {
        // 扩展分区标志位设置
        setExtentLba(dos_partition_get_start(pMbr_partition));
        AFS_TRACE_OUT_DBG("find a extend partition. index=%d", entry_index);
        return AFS_SUCCESS;
    }

    return AFS_ERROE;
}

/**
 * @brief 主分区处理
 *
 * @param *data_buf  缓存数据
 *
 * @return 0:成功  负数：错误ID
 *
 */
int32_t MBRHandler::primaryPartHandle(unsigned char *data_buf)
{
    int32_t ret = 0;
    int32_t partNum = 0;
    int32_t lv_cnt = 0;
    struct partitionOpt ppart;

    // /循环分析4个主分区
    for (char i = 0; i < 4; i++) {
        dos_partition *pMbr_partition = mbr_get_partition(data_buf, i); // 获取分区表项
        unsigned char sys_ind = pMbr_partition->sys_ind;                // 获取分区类型
        partNum = getPartnum();

        AFS_TRACE_OUT_DBG("The %d Partiton Type is %02x (primary parttion is %d)", partNum, sys_ind, i);
        // step1:分区表项未使用
        ret = partitionEntryHandle(pMbr_partition, i);
        if (AFS_SUCCESS == ret) {
            continue;
        }

        // setp3:处理主分区
        ret = setPartnum(++partNum);
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("MBR primaryPartHandle() set partitions num failed, ret = %d", ret);
            return ret;
        }

        AFS_TRACE_OUT_DBG("child m_parts_vect size is %u", m_parts_vect.size());

        AFS_FSTYPE_t pfstype = (0x82 == sys_ind ? AFS_FILESYSTEM_SWAP : AFS_FILESYSTEM_NULL);

        ppart.setPartInfo((unsigned char)(PARTITION_MBR), getPartnum() - 1, false,
            dos_partition_get_start(pMbr_partition), dos_partition_get_size(pMbr_partition), pfstype);

        AFS_TRACE_OUT_DBG("the %d parttion's offset is %llu, length is % llu",
            getPartnum(),
            dos_partition_get_start(pMbr_partition),
            dos_partition_get_size(pMbr_partition));

        // step4:初始化分区信息
        ret = setPartition(getPartnum() - 1, (struct partition *)(&ppart));
        if (ret != AFS_SUCCESS) {
            AFS_TRACE_OUT_ERROR("Failed to initialize partition. index=%d", partNum - 1);
            return AFS_ERR_PARTITION;
        }

        if (MBR_SWAP_PARTITION == sys_ind) {
            continue;
        }

        // step5:LVM处理,LVM会对分区进行管理,重新设置分区偏移
        void *part_tmp = getPartitionPointer(partNum - 1);
        if (NULL == part_tmp) {
            AFS_TRACE_OUT_ERROR("Failed to get partition information.");
            return AFS_ERR_PARTITION;
        }

        vector<struct partition> &partVect = getPartSpaceVect();
        afsLVM lvm(this, partVect, partNum - 1);

        // step6:分析是否是lvm格式
        lv_cnt = lvm.parseLVMFormat(i + 1);
        if (NOT_LVM_FORMAT == lv_cnt || AFS_VOLUME_GROUP_EXIST == lv_cnt || AFS_PV_NO_VG_METADAT == lv_cnt ||
            -1 == lv_cnt) {
            continue; // 非lvm格式分区 或 LVM卷组已经存在 或 PV 没有保存VG metadata
        } else if (0 <= lv_cnt) {
            setPartnumValue(partNum + lv_cnt); // LVM格式，分区加lv个数最为最终的号，设置分区标识
        } else {
            AFS_TRACE_OUT_ERROR("Can't parse lvm part");
            return lv_cnt;
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief 无分区表处理
 * @return AFS_SUCCESS 成功
 * 负值  失败
 */
int32_t MBRHandler::nonePartTableHandle()
{
    struct partitionOpt ppart;
    int32_t lv_cnt = 0;
    int32_t ret = 0;
    int32_t part_num = 0;

    if (NULL == m_reader) {
        AFS_TRACE_OUT_ERROR("Failed to get partition information.");
        return AFS_ERR_PARTITION;
    }

    ppart.setPartInfo(PARTITION_NO, 0, false, 0, m_reader->m_imageinfo.length, AFS_FILESYSTEM_NULL);

    // 一块大分区处理
    part_num = getPartnum();
    ret = setPartnum(++part_num);
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("MBR nonePartTableHandle() set partitions num failed, ret = %d", ret);
        return ret;
    }
    AFS_TRACE_OUT_DBG("the %d parttion's offset is %llu, length is % llu", part_num - 1, ppart.offset,
        m_reader->m_imageinfo.length);

    // 初始化分区信息
    ret = setPartition(part_num - 1, (struct partition *)(&ppart));
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to initialize partition. index=%d", part_num - 1);
        return AFS_ERR_PARTITION;
    }

    vector<struct partition> &partVect = getPartSpaceVect();
    afsLVM lvm(this, partVect, part_num - 1);
    lv_cnt = lvm.parseLVMFormat(0);
    if (0 <= lv_cnt) {
        // LVM格式
        setPartnumValue(part_num + lv_cnt);
    } else if (-1 == lv_cnt || AFS_VOLUME_GROUP_EXIST == lv_cnt || AFS_PV_NO_VG_METADAT == lv_cnt) {
        return AFS_SUCCESS;
    } else if (NOT_LVM_FORMAT != lv_cnt) {
        AFS_TRACE_OUT_ERROR("Parse LVM format is failed. lv_cnt=%d", lv_cnt);
        return lv_cnt;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 分析mbr主分区并且记录所占bitmap
 * @return 0 设置成功 负值  设置失败
 *
 */
int32_t MBRHandler::analyzePrimary()
{
    int32_t ret = 0;
    unsigned char buf[SECTOR_SIZE] = {0};
    int64_t read_size = 0;

    imgReader *reader_tmp = getImgReader();
    if (NULL == reader_tmp) {
        AFS_TRACE_OUT_ERROR("Can't read image");
        return AFS_ERR_PARTITION;
    }

    // 读取0扇区
    read_size = reader_tmp->read(buf, 0, static_cast<int64_t>(SECTOR_SIZE), 1);
    if (static_cast<int64_t>(SECTOR_SIZE) != read_size) {
        AFS_TRACE_OUT_ERROR("Can't parse Primary part");
        return AFS_ERR_IMAGE_READ;
    }

    if (m_partitionflags != PARTITION_NO) {
        ret = primaryPartHandle(buf);
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("Can't parse Primary part");
            return AFS_ERR_PARTITION;
        }
    } else {
        // 无分区处理（当做一个大分区处理）
        AFS_TRACE_OUT_INFO("find a no partition table disk or image");
        ret = nonePartTableHandle();
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("Failed to analyze no partition table image. ret=%d", ret);
            return AFS_ERR_PARTITION;
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief 分析MBR分区
 * @return int32_t 0 设置成功
 * 负数 设置失败
 */
int32_t MBRHandler::parseAllOfPart()
{
    int32_t ret = AFS_SUCCESS;

    // 分析主分区
    ret = analyzePrimary();
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Can't parse Primary part");
        return ret;
    }

    // 分析扩展分区
    return analyzeExtent();
}

/**
 * @brief 设置分区所占bitmap
 *
 * @param &bitmap  镜像Bitmap
 *
 * @return int32_t 0设置成功 -1设置失败
 *
 */
int32_t MBRHandler::getBitmap(BitMap &bitmap)
{
    int32_t ret = 0;

    ret = bitmap.bitmapSet(0);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to set bit value.");
        return AFS_ERR_INNER;
    }

    if (!this->m_extent_lba) {
        // 没有扩展分区直接返回成功
        return AFS_SUCCESS;
    }

    // 扩展分区
    for (vector<uint64_t>::iterator it = m_extentSect.begin(); it != m_extentSect.end(); ++it) {
        ret = bitmap.bitmapSet(*it);
        if (ret != AFS_SUCCESS) {
            AFS_TRACE_OUT_ERROR("Failed to set bit value.");
            return AFS_ERR_INNER;
        }
    }

    return AFS_SUCCESS;
}
