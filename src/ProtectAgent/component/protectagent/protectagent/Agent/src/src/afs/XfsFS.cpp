#include "afs/XfsFS.h"
#include <climits>
#include "afs/LogMsg.h"
#include "afs/AfsError.h"

using namespace std;

/**
 * @brief 16位字节逆序
 * @param i
 * @return
 */
uint16_t xfsHandler::xfs_byteDirInfo_16b(uint8_t *i)
{
    if (i == NULL) {
        AFS_TRACE_OUT_ERROR("Failed to get agbno.");
        return (uint16_t)(-1);
    }
    uint16_t *pval = (reinterpret_cast<uint16_t *>(i));
    uint16_t bit16 = *pval;

    return ((bit16 & 0x00ff) << 8) + ((bit16 & 0xff00) >> 8);
}

/**
 * @brief 32位字节逆序
 * @param i
 * @return
 */
uint32_t xfsHandler::xfs_byteDirInfo_32b(uint8_t *i)
{
    if (i == NULL) {
        AFS_TRACE_OUT_ERROR("the parameter of xfs_byteDirInfo_32b is NULL");
        return (uint32_t)(-1);
    }
    uint32_t *pval = (reinterpret_cast<uint32_t *>(i));
    uint32_t bit32 = *pval;

    bit32 = (((bit32 << 8) & 0xFF00FF00) | ((bit32 >> 8) & 0x00FF00FF));

    return ((bit32 << 16) | (bit32 >> 16));
}

/**
 * @brief 64位字节逆序
 * @param i i8对应Inode的字节数组
 * @return -1 参数错误
 * 有效值 正常转换
 */
uint64_t xfsHandler::xfs_byteDirInfo_64b(uint8_t *i)
{
    if (i == NULL) {
        AFS_TRACE_OUT_ERROR("Invalid i8 address.");
        return (uint64_t)(-1);
    }
    uint64_t *pval = (reinterpret_cast<uint64_t *>(i));
    uint64_t bit64 = *pval;

    return afs_bswap_64(bit64);
}

/**
 * @brief 根据Inode实际存储时占用4字节或者8字节计算相应的Inode值
 * @param inode_info    Inode结构数据
 * @return
 */
uint64_t xfsHandler::xfs_calcInodeValue(xfs_dir2_inou_t inode_info)
{
    uint64_t inode8_value = 0;
    uint32_t inode4_value = 0;

    if (m_inode_8bit_flag) {
        inode8_value = xfs_byteDirInfo_64b(inode_info.i8.i);
    } else {
        inode4_value = (uint64_t)(xfs_byteDirInfo_32b(inode_info.i4.i));
        if (inode4_value == (uint32_t)(-1)) {
            return (uint64_t)(-1);
        }
        inode8_value = (uint64_t)inode4_value;
    }

    return inode8_value;
}

/**
 * @brief 获得文件长度
 * @param i8count
 * @param i8count_p
 * @return
 */
uint64_t xfsHandler::xfs_getFilenameLen(uint64_t i8count, uint8_t i8count_p)
{
    if (i8count) {
        i8count_p = 0;
    } else {
        i8count_p = 4;
    }

    return i8count_p;
}

/**
 * @brief 获取偏移
 * @param ino              块号或者inode号
 * @param inode_or_block   INode与Block的分区标识
 * @return -1 参数错误
 * 有效值 正常的offset
 */
uint64_t xfsHandler::xfs_getOffset(xfs_ino_t ino, uint8_t inode_or_block)
{
    xfs_agnumber_t agno = 0;
    xfs_agino_t agino = 0;
    xfs_agblock_t agbno = 0;
    xfs_ino_t ino_num = 0;
    uint64_t offset = 0;

    if (inode_or_block == INODE_SET) {
        agno = (xfs_agnumber_t)(ino >> m_agino_log);
        agino = (xfs_agino_t)ino & (xfs_agino_t)((1ULL << m_agino_log) - 1);
        agbno = (xfs_agblock_t)(agino >> m_inopblog);
        offset = agino & (xfs_agino_t)((1ULL << m_inopblog) - 1);
        ino_num = ((xfs_ino_t)(agno << m_agino_log)) | agino;

        if (agno >= m_agcount || agbno >= m_agblocks || offset >= m_inopblock || ino_num != ino) {
            AFS_TRACE_OUT_ERROR("Failed to get "
                "offset.m_agino_log=%d,m_inopblog=%d,agno=%lld,m_agcount=%d,m_agblocks=%lld,m_inopblock=%d,ino=%lld",
                m_agino_log, m_inopblog, (long long)agno, m_agcount, (long long)m_agblocks, m_inopblock,
                (long long)ino);

            return (uint64_t)(-1);
        }
    } else if (inode_or_block == BLOCK_SET) {
        agno = (xfs_agnumber_t)(ino >> m_agblklog);
        agbno = (xfs_agblock_t)(ino & xfs_mask64lo(m_agblklog));
    }

    uint64_t off_tmp = (xfs_daddr_t)((agno * m_agblocks + agbno) << m_blkbb_log);
    offset = (uint64_t)(off_tmp << BBSHIFT) + (uint64_t)(offset << m_inodelog);

    return offset;
}

/**
 * @brief 释放Queue
 */
void xfsHandler::xfs_freeQueue()
{
    // 找到目标文件，清空队列
    while (!m_que_bmap_sta.empty()) {
        m_que_bmap_sta.pop();
    }
    while (!m_que_bmap_cnt.empty()) {
        m_que_bmap_cnt.pop();
    }
    while (!m_que_tree_blknum.empty()) {
        m_que_tree_blknum.pop();
    }
    while (!m_file_path_arr.empty()) {
        m_file_path_arr.pop();
    }
}

/**
 * @brief 根据目标文件的inode号分析目标文件所占的块并获取bitmap
 * @param img_reader
 * @param offset_read
 * @return
 */
int32_t xfsHandler::xfs_msgBitmap(imgReader *img_reader, uint64_t offset_read)
{
    // 找到目标文件后，根据inode分析目标文件的属性等
    xfs_dinode_t *root_inode = NULL;
    uint8_t target_inode[INODESIZE] = {0};
    int64_t read_len = 0;
    uint64_t target_pos = 0;
    uint64_t target_len = 0;
    AFS_TRACE_OUT_DBG("before read: offset_read:%llu,INODESIZE:%d.", offset_read, INODESIZE);
    read_len = img_reader->read(target_inode, offset_read, INODESIZE, 0);
    // 读取根文件inode信息
    if (INODESIZE != read_len) {
        AFS_TRACE_OUT_ERROR("Failed to read data.");
        return AFS_ERR_IMAGE_READ;
    }

    root_inode = reinterpret_cast<xfs_dinode_t *>(target_inode);
    if ((afs_bswap_16(root_inode->di_magic) != (__be16)XFS_DINODE_MAGIC)) {
        AFS_TRACE_OUT_ERROR("Failed to read data.");
        return AFS_ERR_FS_VERSION;
    }

    AFS_TRACE_OUT_DBG("xfs inode->di_format = %d", root_inode->di_format);

    // 根据format判断目标文件的类型
    if (XFS_DINODE_FMT_LOCAL == root_inode->di_format) {
        if (0xffa1 == root_inode->di_mode) {
            // 文件的大小小于最小的单位，因此不做出处理
            target_pos = 0;
            target_len = 0;

            m_que_ret_bm_msg_pos.push(target_pos);
            m_que_ret_bm_msg_len.push(target_len);
            return AFS_SUCCESS; // 小于512字节的文件不做处理，返回值认为OK
        }

        m_target_dir_flag = TARGET_FLAG;
        return xfs_readInodeInfo(img_reader, offset_read);
    } else if (XFS_DINODE_FMT_EXTENTS == root_inode->di_format) {
        return xfs_msgBitmapExtents(img_reader, root_inode, target_inode);
    } else if (XFS_DINODE_FMT_BTREE == root_inode->di_format) {
        // 存储格式为btree需要进一步分析
        m_target_dir_flag = TARGET_FLAG;
        return xfs_readInodeInfo(img_reader, offset_read);
    }

    AFS_TRACE_OUT_ERROR("can not support this type!");

    return AFS_ERR_INNER;
}

/**
 * @brief 根据inode分析目标文件类型为Extents的属性
 *
 * @param img_reader 镜像读取的reader
 * @param root_inode 存放inode元数据信息的地址空间起始地址
 * @param target_inode 存放根文件inode信息的起始地址
 * @return -1 内部错误
 * 正常返回值0
 */
int32_t xfsHandler::xfs_msgBitmapExtents(imgReader *img_reader, xfs_dinode_t *root_inode, uint8_t *target_inode)
{
    my_rec_use_read *recs_info = NULL;
    xfs_bmbt_irec_t ret_irc;
    xfs_bmbt_irec_t *pret_irc = &ret_irc;
    head_sdb_msg *sdb_msg = NULL;
    uint8_t block_msg[4] = {0};
    int64_t read_len = 0;
    int32_t ret = AFS_SUCCESS;
    m_target_dir_flag = TARGET_FLAG;
    xfs_dinode_t *root_inode_tmp = root_inode;
    CHECK_MEMSET_S_OK(&ret_irc, sizeof(ret_irc), 0, sizeof(ret_irc));

    for (__be32 i = 0; i < afs_bswap_32(root_inode_tmp->di_nextents); i++) {
        // 获取文件位置信息
        recs_info = reinterpret_cast<my_rec_use_read *>(target_inode + m_offset_hdr + FILE_OFFSET * i);
        xfs_analyzeData(recs_info, pret_irc); // /下行代码可视为返回值判断
        if ((0 == pret_irc->br_startblock) && (0 == pret_irc->br_blockcount)) {
            AFS_TRACE_OUT_ERROR("can not found the file.");
            return AFS_ERR_IMAGE_READ;
        }
        if (pret_irc->br_startoff >= XFS_DIR2_LEAF_OFFSET) {
            AFS_TRACE_OUT_DBG(
                "extents.startoff is %llu, thus it is a leaf block or free index block, not a directory block",
                pret_irc->br_startoff);
            continue;
        }
        AFS_TRACE_OUT_DBG("xfs_msgBitmapExtents(), i = %d, di_nextexts is %u", i,
            afs_bswap_32(root_inode->di_nextents));

        uint64_t offset_blk = 0;
        uint64_t data_size = 0;

        if ((pret_irc->br_startblock > pret_irc->br_startoff) && (0 != pret_irc->br_blockcount) &&
            (0 != pret_irc->br_startblock) && (0 == (uint32_t)pret_irc->br_state)) {
            offset_blk = xfs_getOffset(pret_irc->br_startblock, BLOCK_SET);
            if (-1 == static_cast<int64_t>(offset_blk)) {
                AFS_TRACE_OUT_ERROR("Failed to get offset.");
                return AFS_ERR_INNER;
            }

            data_size = offset_blk + pret_irc->br_blockcount * m_blocksize;
            if (data_size > (uint64_t)((img_reader->m_imageinfo.length) << 9)) {
                AFS_TRACE_OUT_ERROR(
                    "Invalid extent data offset. pret_irc->br_startblock = %lld, pret_irc->br_blockcount = %lld",
                    (long long)pret_irc->br_startblock, (long long)pret_irc->br_blockcount);
                continue;
            }

            // 读取block信息
            AFS_TRACE_OUT_DBG("before read: offset_blk:%llu,head_sdb_msg:%d.", offset_blk, sizeof(head_sdb_msg));
            read_len = img_reader->read(block_msg, offset_blk, static_cast<int64_t>(sizeof(head_sdb_msg)), 1);
            if (static_cast<int64_t>(sizeof(head_sdb_msg)) != read_len) {
                AFS_TRACE_OUT_ERROR("Failed to read data.");
                return AFS_ERR_IMAGE_READ;
            }
            sdb_msg = reinterpret_cast<head_sdb_msg *>(block_msg);
            ret = xfs_msgBitmapExtents_if(img_reader, offset_blk, pret_irc, sdb_msg);
            if (AFS_SUCCESS != ret) {
                AFS_TRACE_OUT_ERROR("xfs_msgBitmapExtents_if() return error.");
                return ret;
            }
        }
    }

    return xfs_msgBitmapExtentswhile(img_reader);
}

/**
 * @brief 对获取到的数据块进行判断，是否是需要的
 *
 * @param img_reader 镜像读取的reader
 * @param offset_blk 根据起始块号获取的偏移
 * @param ret_irc xfs_bmbt_irec_t类型的结构体变量
 * @param sdb_msg head_sdb_msg* 存放块信息的前4个字节
 * @return 0: 成功
 * 负数：失败
 *
 */
int32_t xfsHandler::xfs_msgBitmapExtents_if(imgReader *img_reader, uint64_t &offset_blk, xfs_bmbt_irec_t *ret_irc,
    head_sdb_msg *sdb_msg)
{
    uint64_t target_pos = 0;
    uint64_t target_len = 0;
    uint64_t data_size = 0;

    uint32_t uret = xfs_byteDirInfo_32b(sdb_msg->msg);
    if (static_cast<uint32_t>(-1) == uret) {
        return AFS_ERR_INNER;
    } else if (((uint32_t)XFS_DIR2_BLOCK_MAGIC == uret) || ((uint32_t)XFS_DIR2_DATA_MAGIC == uret) ||
        ((uint32_t)XFS_DIR3_BLOCK_MAGIC == uret) || ((uint32_t)XFS_DIR3_DATA_MAGIC == uret)) {
        m_que_block_sta.push(ret_irc->br_startblock);
        m_que_block_cnt.push(ret_irc->br_blockcount);
    } else {
        // 计算文件的起始位置
        target_pos = offset_blk / SERCTORSIZE;
        target_len = ret_irc->br_blockcount * m_blocksize / SERCTORSIZE;

        data_size = (offset_blk + ret_irc->br_blockcount * m_blocksize);
        if (data_size > (uint64_t)((img_reader->m_imageinfo.length) << 9)) { // 检查size有效性
            AFS_TRACE_OUT_ERROR(
                "Invalid extent data offset. ret_irc->br_startblock = %lld, ret_irc->br_blockcount = %lld",
                (long long)ret_irc->br_startblock, (long long)ret_irc->br_blockcount);
            return AFS_ERR_INNER;
        }

        // 对获取到的数据块进行判断，是否是需要的
        m_que_ret_bm_msg_pos.push(target_pos);
        m_que_ret_bm_msg_len.push(target_len);
    }

    return AFS_SUCCESS;
}

/**
 * @brief 判断队列m_que_block_sta是否为空，不空的话取出起始块号和块数量进一步分析
 *
 * @param img_reader 镜像读取的reader
 * @return 0: 成功
 * 负数：失败
 *
 */
int32_t xfsHandler::xfs_msgBitmapExtentswhile(imgReader *img_reader)
{
    int32_t ret = AFS_SUCCESS;
    AFS_TRACE_OUT_DBG("Enter xfs_msgBitmapExtentswhile");

    while (!m_que_block_sta.empty() && !m_que_block_cnt.empty()) {
        uint64_t startblock = m_que_block_sta.front();
        uint64_t blockcount = m_que_block_cnt.front();

        m_que_block_sta.pop();
        m_que_block_cnt.pop();
        if ((0 != blockcount) && (0 != startblock)) {
            ret = xfs_analyzeBlockDir(img_reader, startblock, blockcount);
            if (AFS_SUCCESS != ret) {
                AFS_TRACE_OUT_ERROR("The logical is error.");
                return ret;
            }
        }
    }
    return AFS_SUCCESS;
}

/**
 * @brief 分割路径
 * @param str
 * @return
 */
void xfsHandler::xfs_splitPath(string &str)
{
    queue<string> resVec;

    // 方便截取最后一段数据
    string strs = str + PATH_PATTERN;

    size_t pos = strs.find(PATH_PATTERN);
    size_t size = strs.size();

    while (pos != string::npos) {
        string sub_str = strs.substr(0, pos);
        if (!sub_str.empty()) {
            m_file_path_arr.push(sub_str);
        }

        strs = strs.substr(pos + 1, size);
        pos = strs.find(PATH_PATTERN);
    }
}

/**
 * @brief 在获取空闲块时，读取超级块信息
 *
 * @param *img_reader  从磁盘读取数据的句柄
 * @param &getB_info   保存读取到的超级块信息
 * @return 0：成功
 * 负数：错误ID
 */
int32_t xfsHandler::xfs_getBitmapByReadSBInfo(imgReader *img_reader, my_info_use_t &getB_info)
{
    AFS_TRACE_OUT_DBG("ENTER xfsHandler::xfs_getBitmapByReadSBInfo()");

    int32_t ret = 0;
    int64_t read_len = 0;
    xfs_dsb_t *getB_sb = NULL;

    // 申请超级块空间
    getB_sb = static_cast<xfs_dsb_t *>(calloc(1, sizeof(xfs_dsb_t)));
    if (NULL == getB_sb) {
        AFS_TRACE_OUT_ERROR("Failed to calloc getB_sb.");
        return AFS_ERR_API;
    }

    // 读取超级块
    int64_t stSize = sizeof(xfs_dsb_t);
    bool result = ReadBySectorsBuff(img_reader, reinterpret_cast<void *>(getB_sb), 0, stSize, 1);
    if (!result) {
        AFS_TRACE_OUT_ERROR("Failed to read xfs_dsb_t data.");
        free(getB_sb);
        getB_sb = NULL;
        return AFS_ERR_IMAGE_READ;
    }

    // 保存来自超级块，getbitmap所需的必要信息
    getB_info.sb_magicnum = afs_bswap_32(getB_sb->sb_magicnum);
    if (getB_info.sb_magicnum != (__uint32_t)XFS_SB_MAGIC) {
        AFS_TRACE_OUT_ERROR("sb_magicnum is error!, getB_sb->sb_magicnum:%llu, getB_info.sb_magicnum:%llu",
            getB_sb->sb_magicnum,
            getB_info.sb_magicnum);
        ret = AFS_ERR_INNER;
        goto Tail;
    }

    getB_info.sb_dblocks = afs_bswap_64(getB_sb->sb_dblocks);
    getB_info.sb_blocksize = afs_bswap_32(getB_sb->sb_blocksize);
    getB_info.sb_agblocks = afs_bswap_32(getB_sb->sb_agblocks);
    getB_info.sb_agcount = afs_bswap_32(getB_sb->sb_agcount);
    getB_info.crc_flag = afs_bswap_32(getB_sb->sb_crc);
    getB_info.sb_sectsize = afs_bswap_16(getB_sb->sb_sectsize);

    AFS_TRACE_OUT_DBG("Current XFS BlockSize=%d, inodesize is %d", getB_info.sb_blocksize,
        afs_bswap_16(getB_sb->sb_inodesize));

    if (0 == getB_info.sb_blocksize) {
        AFS_TRACE_OUT_ERROR("sb_magicnum is error! getB_info.sb_blocksize is zero.");
        ret = AFS_ERR_INNER;
        goto Tail;
    }

    AFS_TRACE_OUT_INFO("Success to analyze super block");

Tail:
    // 处理内存泄漏问题
    if (NULL != getB_sb) {
        free(getB_sb);
        getB_sb = NULL;
    }

    AFS_TRACE_OUT_DBG("EXIT xfsHandler::xfs_getBitmapByReadSBInfo()");
    return ret;
}

/**
 * @brief 在获取空闲块时，读取agf信息
 *
 * @param *img_reader  从磁盘读取数据的句柄
 * @param offset       agf信息的相对偏移
 * @param &getB_info   保存读取到的AGF信息
 * @return 0:成功
 * 负数:错误ID
 *
 */
int32_t xfsHandler::xfs_getBitmapByReadAGFInfo(imgReader *img_reader, uint64_t offset, my_info_use_t &getB_info)
{
    AFS_TRACE_OUT_DBG("ENTER xfsHandler::xfs_getBitmapByReadAGFInfo()");

    int32_t ret = 0;
    int64_t read_len = 0;
    xfs_agf_t *getB_agf = NULL;

    // 申请暂存agf信息的空间
    getB_agf = static_cast<xfs_agf_t *>(calloc(1, sizeof(xfs_agf_t)));
    if (NULL == getB_agf) {
        AFS_TRACE_OUT_ERROR("Failed to calloc getB_agf.");
        return AFS_ERR_API;
    }

    // 读取agf信息
    int64_t stSize = sizeof(xfs_agf_t);
    bool result = ReadBySectorsBuff(img_reader, reinterpret_cast<void *>(getB_agf), offset, stSize, 1);
    if (!result) {
        AFS_TRACE_OUT_ERROR("Failed to read getB_agf data.");
        free(getB_agf);
        getB_agf = NULL;
        return AFS_ERR_IMAGE_READ;
    }

    // 保存getbitmap所需的agf信息，有AG号、ABTB以及ABTC的根节点，记录ABTB的根节点为将要遍历的b+树的节点
    getB_info.agf_magicnum = afs_bswap_32(getB_agf->agf_magicnum);
    if (getB_info.agf_magicnum != XFS_AGF_MAGIC) {
        AFS_TRACE_OUT_ERROR("Agf_magicnum[%02x] is error!", getB_info.agf_magicnum);
        ret = AFS_ERR_INNER;
        goto Tail;
    }

    getB_info.agf_seqno = afs_bswap_32(getB_agf->agf_seqno);
    getB_info.abtb_root = afs_bswap_32(getB_agf->agf_roots[0]);
    getB_info.abtc_root = afs_bswap_32(getB_agf->agf_roots[1]);
    getB_info.rootnum = afs_bswap_32(getB_agf->agf_roots[0]);

    AFS_TRACE_OUT_DBG("Success to analyze AGF, getB_info.rootnum:%d.", getB_info.rootnum);
Tail:
    // 处理内存泄漏问题
    if (NULL != getB_agf) {
        free(getB_agf);
        getB_agf = NULL;
    }

    AFS_TRACE_OUT_DBG("EXIT xfsHandler::xfs_getBitmapByReadAGFInfo()");
    return ret;
}

/**
 * @brief 获得文件空闲块信息并得到bitmap
 *
 * @param &bitmap 用于标记空闲块所占的bitmap
 * @return 0：设置成功
 * -1：设置失败
 */
int xfsHandler::getBitmap(vector<BitMap *> &bitmap_vect)
{
    AFS_TRACE_OUT_INFO("ENTER xfsHandler::getBitmap()");
    uint32_t blocksize = 0;
    uint32_t agblocks = 0;
    uint32_t agfcount = 0;
    uint64_t offset = 0;

    int32_t ret = 0;
    my_info_use_t getB_info;

    CHECK_MEMSET_S_OK(&getB_info, sizeof(getB_info), 0, sizeof(getB_info));

    imgReader *reader = getImgReader();
    if (NULL == getImgReader()) {
        AFS_TRACE_OUT_ERROR("The reader is NULL!");
        return AFS_ERR_INNER;
    }

    // 读取superblock
    ret = xfs_getBitmapByReadSBInfo(reader, getB_info);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to get sb info to getB_info.");
        return ret;
    }

    blocksize = getB_info.sb_blocksize;
    agblocks = getB_info.sb_agblocks;
    agfcount = getB_info.sb_agcount;

    AFS_TRACE_OUT_DBG("Xfs agfcount = %u, agblocks = %u", agfcount, agblocks);

    BitMap fsbitmap;
    fsbitmap.bitmapSetBlocksize(blocksize);
    ret = fsbitmap.initBitMap(getB_info.sb_dblocks);
    if (ret < 0) {
        AFS_TRACE_OUT_ERROR("Cannt handle bitmap.");
        return ret;
    }

    // 置1  XFS文件系统是按照空闲块处理的
    CHECK_MEMSET_S_OK(fsbitmap.getbitmap(), fsbitmap.getsize(), 0xff, fsbitmap.getsize());

    for (uint32_t agf_index = 0; agf_index < agfcount; agf_index++) {
        offset = static_cast<uint64_t>(agf_index) * agblocks;
        offset = offset * blocksize + getB_info.sb_sectsize; // /找到分配组（AG）的AGF扇区

        AFS_TRACE_OUT_DBG("The new AG AGF offset is %llu.", offset);

        // 读取agf信息
        ret = xfs_getBitmapByReadAGFInfo(reader, offset, getB_info);
        if (ret < 0) {
            AFS_TRACE_OUT_ERROR("Failed to get the agf info to getB_info, the agf No is: %d.", agf_index);
            return ret;
        }
        AFS_TRACE_OUT_DBG("The current ag is: %d.", getB_info.agf_seqno);

        ret = xfs_getBitmapByReadBlkInfo(reader, getB_info, &fsbitmap);
        if (ret < 0) {
            AFS_TRACE_OUT_ERROR("Failed to get this ag' bitmap, the agf No is: %d.", agf_index);
            return ret;
        }
    }

    AFS_TRACE_OUT_INFO("EXIT xfsHandler::getBitmap()");
    // 位图bit表示的数据块大小由文件系统blocksize转换到SECTOR_SIZE单位，再由SECTOR_SIZE转换到用户指定大小
    return fsbitmap.bitmapConvert(reader, SECTOR_SIZE, bitmap_vect);
}

/**
 * @brief bitmap转换
 * @param bitmap
 * @param position 起始位置
 * @param len 长度
 * @return 0
 */
int32_t xfsHandler::xfs_getBitmapBitmapConvert(BitMap *bitmap, uint64_t position, uint64_t len)
{
    uint64_t end_addr = position + len;
    uint64_t align_addr = 0;
    uint64_t remain_addr = 0;

    for (align_addr = position; align_addr < end_addr; align_addr++) {
        remain_addr = align_addr % 8;
        // 相应的位清0
        (bitmap->getbitmap())[(align_addr / 8)] &= ~(1 << (7 - remain_addr));
    }

    return AFS_SUCCESS;
}

/**
 * @brief 读取空闲块信息并生成bitmap中的深度分情况讨论
 *
 * @param *img_reader        镜像读取的reader
 * @param &ptr_position      ptr指针的起始位置
 * @param *btree_blk_info    为b+tree的信息块分配的空间起始地址
 * @param *recs              指向叶子节点信息的起始地址
 * @param &getB_info         用于存放从超级块读取的信息
 * @param *bitmap            用于记录空闲块的bitmap
 * @return 负数:失败
 * 0：成功
 *
 */
int32_t xfsHandler::xfs_getBitmapByReadBlkInfo_depth(imgReader *img_reader, uint32_t &ptr_position,
    uint8_t *btree_blk_info, xfs_alloc_key_t *recs, my_info_use_t &getB_info, BitMap *bitmap)
{
    if (NULL == recs) {
        AFS_TRACE_OUT_ERROR("The space is NULL.");
        return AFS_ERR_INNER;
    }

    uint32_t *record = NULL;
    uint16_t depth = 0;
    uint16_t bb_numrecs_abtb = 0;
    uint32_t num = 0;
    uint32_t loop_count = 0;
    uint32_t startblock = 0;
    uint64_t blockcount = 0;
    int32_t ret = 0;

    depth = getB_info.abtb_depth;
    AFS_TRACE_OUT_DBG("getB_info.abtb_depth is %u", getB_info.abtb_depth);

    bb_numrecs_abtb = getB_info.abtb_numrecs;
    if (depth == 1) {
        btree_blk_info += ptr_position;
        // 在读取内容时读取了一整块，通过强制指针转换，把指针定位到中间节点信息的起始位置
        record = reinterpret_cast<uint32_t *>(btree_blk_info);

        // 遍历ptr指针，读取相关信息
        for (num = 0; num < bb_numrecs_abtb; num++) {
            getB_info.rootnum = afs_bswap_32(record[num]);
            ret = xfs_getBitmapByReadBlkInfo(img_reader, getB_info, bitmap);
            if (ret < 0) {
                AFS_TRACE_OUT_ERROR("Failed to read bitmap,the rootnum is %d.", getB_info.rootnum);
                return AFS_ERR_INNER;
            }
        }
    } else if (depth == 0) {
        // 遍历输出叶子节点记录的空闲块信息
        for (loop_count = 0; loop_count < bb_numrecs_abtb; loop_count++) {
            startblock = afs_bswap_32(recs[loop_count].ar_startblock);
            blockcount = afs_bswap_32(recs[loop_count].ar_blockcount);
            AFS_TRACE_OUT_DBG("The startblock %d, blockcount %d.", startblock, blockcount);
            ret = xfs_getBitmapBitmapConvert(bitmap,
                (static_cast<uint64_t>(getB_info.agf_seqno) * getB_info.sb_agblocks + startblock), blockcount);
            if (ret < 0) {
                AFS_TRACE_OUT_ERROR("Failed to set bitmap.");
                return ret;
            }
        }
    } else {
        AFS_TRACE_OUT_ERROR("Has unresolved ABTB level!");
        return AFS_ERR_INNER;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 读取空闲块信息并生成bitmap
 *
 * @param *img_reader    镜像读取的reader
 * @param &getB_info     用于存储有用信息
 * @param *bitmap        用于记录空闲块的bitmap
 * @return 0：设置成功
 * 负数：错误ID
 */
int32_t xfsHandler::xfs_getBitmapByReadBlkInfo(imgReader *img_reader, my_info_use_t &getB_info, BitMap *bitmap)
{
    AFS_TRACE_OUT_DBG("ENTER xfsHandler::xfs_getBitmapByReadBlkInfo()");

    uint32_t agnum = 0;
    uint32_t blocksize = 0;
    uint32_t rootnum = 0;
    uint32_t ptr_position = 0;
    int32_t ret = 0;

    uint64_t offset = 0;
    uint64_t agblocks = 0;
    int64_t read_ret = 0;

    uint8_t *btree_blk_info = NULL;
    uint8_t *temp_xfs_btree_block_info = NULL;
    xfs_btree_block *btree_head = NULL;

    // 指向叶子节点相关信息的指针
    xfs_alloc_key_t *recs = NULL;

    agnum = getB_info.agf_seqno;
    agblocks = getB_info.sb_agblocks;
    blocksize = getB_info.sb_blocksize;
    rootnum = getB_info.rootnum; // 此处应该是ABTB树相对本AG的block偏移

    // 为b+tree的信息块分配空间
    btree_blk_info = static_cast<uint8_t *>(calloc(1, blocksize));
    if (NULL == btree_blk_info) {
        AFS_TRACE_OUT_ERROR("Failed to calloc ABTB_B+tree space.");
        return AFS_ERR_API;
    }
    // 临时存储calloc内存的首地址，以便准确释放
    temp_xfs_btree_block_info = btree_blk_info;

    // 计算具体偏移
    offset = static_cast<uint64_t>(agnum) * agblocks * blocksize + static_cast<uint64_t>(rootnum) * blocksize;
    AFS_TRACE_OUT_DBG("before read: ABTB offset to file system is %llu, root number is %u, blocksize: %u",
        (unsigned long long)offset,
        rootnum,
        blocksize);

    read_ret = img_reader->read(btree_blk_info, offset, blocksize, 0);
    if (read_ret != static_cast<int64_t>(blocksize)) {
        AFS_TRACE_OUT_ERROR("Failed to calloc ABTB_B+tree space.");
        ret = AFS_ERR_IMAGE_READ;
        goto Tail;
    }

    // 指向将ABTB信息块的头部信息
    btree_head = reinterpret_cast<xfs_btree_block *>(btree_blk_info);

    getB_info.abtb_magic = afs_bswap_32(btree_head->bb_magic);
    getB_info.abtb_numrecs = afs_bswap_16(btree_head->bb_numrecs);
    getB_info.abtb_depth = afs_bswap_16(btree_head->bb_level);

    ret = xfs_getBitmapByReadBlkInfo_1(img_reader, getB_info, ptr_position, &recs, btree_blk_info);
    if (ret < 0) {
        AFS_TRACE_OUT_ERROR("Failed to get bitmap.");
        goto Tail;
    }

    // ABTB/ABTC/AB3B/AB3C的深度
    ret = xfs_getBitmapByReadBlkInfo_depth(img_reader, ptr_position, btree_blk_info, recs, getB_info, bitmap);
    if (ret < 0) {
        AFS_TRACE_OUT_ERROR("Failed to getbitmap by block info.");
        goto Tail;
    }

Tail:
    if (NULL != temp_xfs_btree_block_info) {
        free(temp_xfs_btree_block_info);
        temp_xfs_btree_block_info = NULL;
    }

    AFS_TRACE_OUT_DBG("EXIT xfsHandler::xfs_getBitmapByReadBlkInfo()");

    return ret;
}

/**
 * @brief 函数xfs_getBitmapByReadBlkInfo根据crc_flag定位ptr和rec的起始位置
 *
 * @param *img_reader         镜像读取的reader
 * @param &getB_info          用于存放从超级块读取的信息
 * @param &ptr_position       ptr指针的起始位置
 * @param **recs              存放叶子节点信息的起始位置
 * @param *btree_blk_info     为b+tree的信息块分配的空间起始地址
 * @return
 * 0： 正常返回值
 * 负数：错误ID
 */
int32_t xfsHandler::xfs_getBitmapByReadBlkInfo_1(imgReader *img_reader, my_info_use_t &getB_info,
    uint32_t &ptr_position, xfs_alloc_key_t **recs, uint8_t *btree_blk_info)
{ // 判断是否开启crc校验位，crc=1表示开启，并且ABTB/ABTC变为AB3B/AB3C
    uint32_t crc_flag = getB_info.crc_flag;

    AFS_TRACE_OUT_DBG("xfs_getBitmapByReadBlkInfo_1: crc_flag = %u, getB_info.abtb_magic = %d", crc_flag,
        getB_info.abtb_magic);
    if (!crc_flag) { /* ABTB||ABTC */
        AFS_TRACE_OUT_DBG("ABTB||ABTC");
        if (getB_info.abtb_magic != (__uint32_t)XFS_ABTB_MAGIC) {
            AFS_TRACE_OUT_ERROR("Failed to read ABTB_B+tree.The abtb_magic is ERROR. magic:%d", getB_info.abtb_magic);
            return AFS_ERR_INNER;
        } else {
            // 定位ptr指针的具体位置
            ptr_position = (getB_info.sb_blocksize - XFS_BTREE_SBLOCK_LEN) / (8 + 4) * 8 + XFS_BTREE_SBLOCK_LEN;
            // 在读取内容时读取了一整块，通过强制指针转换，定位到指向叶子节点信息的位置
            AFS_TRACE_OUT_DBG("Current postion is %d", ptr_position);
            (*recs) = reinterpret_cast<xfs_alloc_key_t *>(btree_blk_info + XFS_BTREE_SBLOCK_LEN);
        }
    } else { /* AB3B||AB3C */
        AFS_TRACE_OUT_DBG("AB3B||AB3C");
        if (getB_info.abtb_magic != (__uint32_t)XFS_ABTB_CRC_MAGIC) {
            AFS_TRACE_OUT_ERROR("Failed to read AB3B_B+tree.The abtb_magic is ERROR. magic:%d ,getB_info.rootnum:%d",
                getB_info.abtb_magic, getB_info.rootnum);
            return AFS_ERR_INNER;
        } else {
            // 定位ptr指针的具体位置
            ptr_position = (getB_info.sb_blocksize - XFS_BTREE_SBLOCK_CRC_LEN) / (8 + 4) * 8 + XFS_BTREE_SBLOCK_CRC_LEN;
            // 在读取内容时读取了一整块，通过强制指针转换，定位到指向叶子节点信息的位置
            (*recs) = reinterpret_cast<xfs_alloc_key_t *>(btree_blk_info + XFS_BTREE_SBLOCK_CRC_LEN);
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief 解析extent压缩数据
 *
 * @param  rec_read 起始块，块个数等，未解压的数据信息
 * @param  m_bmbt_irec 保存解压后的起始块，块个数等信息
 *
 */

int32_t xfsHandler::xfs_analyzeData(my_rec_use_read *rec_read, xfs_bmbt_irec_t *m_bmbt_irec)
{
    // 解析磁盘extents压缩数据
    // 获取起始块和所占的块个数
    my_rec irec;
    CHECK_MEMSET_S_OK(&irec, sizeof(irec), 0, sizeof(irec));

    irec.uname.startblock = 0; // 需要将改数据初始化为0
    // 进行字节序转换
    rec_read->my_union1.x1 = afs_bswap_64(rec_read->my_union1.x1);
    rec_read->my_union2.x2 = afs_bswap_64(rec_read->my_union2.x2);

    // 数据赋值
    irec.blockcount = rec_read->my_union2.u2.blockcount;
    irec.extentflag = rec_read->my_union1.u1.extentflag;
    irec.uname.u.startblock1 = rec_read->my_union1.u1.startblock1;
    irec.uname.u.startblock2 = rec_read->my_union2.u2.startblock2;
    irec.startoff = rec_read->my_union1.u1.startoff;

    // 数据类型转换，得到startblock和blockcount
    m_bmbt_irec->br_state = (xfs_exntst_t)((irec.extentflag));
    m_bmbt_irec->br_startoff = (xfs_fileoff_t)((irec.startoff));
    m_bmbt_irec->br_startblock = (xfs_fsblock_t)((irec.uname.startblock));
    m_bmbt_irec->br_blockcount = (xfs_fsblock_t)((irec.blockcount));

    return AFS_SUCCESS;
}

/**
 * @brief  分析目录块信息
 *
 * @param img_reader 镜像读取的reader
 * @param start_blk  起始块
 * @param count_blk  块个数
 * @return -1内部错误
 * 正常返回值0
 */

int32_t xfsHandler::xfs_analyzeBlockDir(imgReader *img_reader, uint32_t start_blk, uint64_t count_blk)
{
    uint64_t offset_blk = 0;
    uint8_t block_msg[BLOCKSIZE] = {0};
    uint32_t file_position = 0;
    uint16_t single_block_dir_offset = 0;
    int32_t ret = AFS_SUCCESS;
    m_tag_inode = 0;

    // 根据参数count_blk，读取块信息
    for (uint64_t i = 0; i < count_blk; i++) {
        AFS_TRACE_OUT_DBG("xfs analyze block directory, %llu times of total[%llu]", i + 1, count_blk);
        // 根据参数start_blk计算起始偏移位置
        offset_blk = xfs_getOffset(start_blk + i, BLOCK_SET);
        if (-1 == static_cast<int64_t>(offset_blk)) {
            AFS_TRACE_OUT_ERROR("Failed to get offset.");
            return AFS_ERR_INNER;
        }

        file_position = 0;

        // 读取block信息
        AFS_TRACE_OUT_DBG("before read: offset_blk:%llu,BLOCKSIZE:%d.", offset_blk, BLOCKSIZE);
        int64_t read_len = img_reader->read(block_msg, offset_blk, BLOCKSIZE, 0);
        if (static_cast<int64_t>(BLOCKSIZE) != read_len) {
            AFS_TRACE_OUT_ERROR("Failed to read data.");
            return AFS_ERR_IMAGE_READ;
        }

        ret = xfs_analyzeBlockDirbestfree(img_reader, block_msg, file_position, single_block_dir_offset);
        if (-2 == ret) {
            continue;
        } else if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("The logical is error.");
            return ret;
        }

        while (file_position < BLOCKSIZE - single_block_dir_offset) {
            ret = xfs_analyzeBlockDirWhile(img_reader, file_position, block_msg);
            if (AFS_SUCCESS != ret) {
                AFS_TRACE_OUT_ERROR("The logical is error.");
                return ret;
            }

            if (m_found_target_file == 1) {
                AFS_TRACE_OUT_DBG("extent directory : m_found_target_file == 1");
                break;
            }
        }

        if (m_found_target_file == 1) {
            break;
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief 根据第一个bestfree的信息来确定postion起始位置
 *
 * @param *img_reader               文件对象
 * @param *block_msg                存放块信息的起始地址
 * @param &file_position            目录项的起始位置
 * @param single_block_dir_offset   单block目录块相对于尾部的偏移
 * @return -1 幻数不匹配
 * 正常返回position起始位置
 */
int32_t xfsHandler::xfs_analyzeBlockDirbestfree(imgReader *img_reader, uint8_t *block_msg, uint32_t &file_position,
    uint16_t &single_block_dir_offset)
{
    xfs_dir2_data_hdr *xdd2_head_info = NULL;
    xfs_dir3_data_hdr *xdd3_head_info = NULL;
    xfs_dir2_block_tail_t *xdd2_tail_info = NULL;

    if (0 == m_crc) {
        xdd2_head_info = reinterpret_cast<xfs_dir2_data_hdr *>(block_msg);
        if (0 == (afs_bswap_32(xdd2_head_info->magic))) {
            AFS_TRACE_OUT_ERROR("Can not analyze block. So skip this.");
            return -2;
        }

        if ((afs_bswap_32(xdd2_head_info->magic) != (__be32)XFS_DIR2_BLOCK_MAGIC) &&
            (afs_bswap_32(xdd2_head_info->magic) != (__be32)XFS_DIR2_DATA_MAGIC)) {
            AFS_TRACE_OUT_ERROR("Failed to read magic num.");
            return AFS_ERR_INNER;
        }

        if (afs_bswap_32(xdd2_head_info->magic) == (__be32)XFS_DIR2_BLOCK_MAGIC) { // /single block dirs
            xdd2_tail_info =
                reinterpret_cast<xfs_dir2_block_tail_t *>(block_msg + BLOCKSIZE - sizeof(xfs_dir2_block_tail_t));
            single_block_dir_offset =
                sizeof(xfs_dir2_block_tail_t) + afs_bswap_32(xdd2_tail_info->count) * sizeof(xfs_dir2_leaf_entry_t);
        }

        file_position = sizeof(xfs_dir2_data_hdr);
    } else {
        xdd3_head_info = reinterpret_cast<xfs_dir3_data_hdr *>(block_msg);

        if (0 == (afs_bswap_32(xdd3_head_info->hdr.magic))) {
            AFS_TRACE_OUT_ERROR("Cannot analyze the block as a direction block, hdr.magic is 0x%08x. So skip this.",
                (afs_bswap_32(xdd3_head_info->hdr.magic)));
            return -2;
        }

        if ((afs_bswap_32(xdd3_head_info->hdr.magic) != (__be32)XFS_DIR3_BLOCK_MAGIC) &&
            (afs_bswap_32(xdd3_head_info->hdr.magic) != (__be32)XFS_DIR3_DATA_MAGIC)) {
            AFS_TRACE_OUT_ERROR("Failed to read magic num [%08x].", afs_bswap_32(xdd3_head_info->hdr.magic));
            return AFS_ERR_INNER;
        }

        if (afs_bswap_32(xdd3_head_info->hdr.magic) == (__be32)XFS_DIR3_BLOCK_MAGIC) { // /single block dirs
            xdd2_tail_info =
                reinterpret_cast<xfs_dir2_block_tail_t *>(block_msg + BLOCKSIZE - sizeof(xfs_dir2_block_tail_t));
            single_block_dir_offset =
                sizeof(xfs_dir2_block_tail_t) + afs_bswap_32(xdd2_tail_info->count) * sizeof(xfs_dir2_leaf_entry_t);
            AFS_TRACE_OUT_DBG(
                "single_block_dir_offset = %u, sizeof(xfs_dir2_block_tail_t) is %u, sizeof(xfs_dir2_leaf_entry_t)is %u",
                single_block_dir_offset, sizeof(xfs_dir2_block_tail_t), sizeof(xfs_dir2_leaf_entry_t));
        }
        file_position = sizeof(xfs_dir3_data_hdr);
    }

    AFS_TRACE_OUT_DBG("the first directory entry position is %u", file_position);
    return AFS_SUCCESS;
}

/**
 * @brief 分析目录块信息中的while循环,指定目标为目录时，将子文件inode保存
 *
 * @param *img_reader     文件对象
 * @param count_blk       块个数
 * @param &file_postion   postion的起始位置
 * @param *block_msg      存放块信息的起始地址
 * @return  0：处理成功
 * 负数：错误ID
 */
int32_t xfsHandler::xfs_analyzeBlockDirWhile(imgReader *img_reader, uint32_t &file_postion, uint8_t *block_msg)
{
    xfs_dir2_data_entry_t *xd2d_dir_info = NULL;
    xfs_dir2_data_unused_t *xd2d_dir_free_data = NULL;
    int32_t ret = AFS_SUCCESS;
    uint64_t inode_number = 0;
    uint32_t dir_entry_length = 0;
    uint32_t data_entry_length = 0;

    xd2d_dir_free_data = reinterpret_cast<xfs_dir2_data_unused_t *>(block_msg + file_postion);
    if (afs_bswap_16(xd2d_dir_free_data->freetag) == XFS_DIR2_DATA_FREE_TAG) {
        file_postion += afs_bswap_16(xd2d_dir_free_data->length);
        return AFS_SUCCESS;
    }

    xd2d_dir_info = reinterpret_cast<xfs_dir2_data_entry_t *>(block_msg + file_postion);
    data_entry_length = (sizeof(xfs_dir2_data_entry_t) == 16 ? sizeof(xfs_dir2_data_entry_t) - NAME_SEAM :
                                                               sizeof(xfs_dir2_data_entry_t));

    dir_entry_length = (data_entry_length + xd2d_dir_info->namelen + m_has_filetype + 2 + XFS_DIR2_DATA_ALIGN - 1) /
        XFS_DIR2_DATA_ALIGN;
    dir_entry_length *= XFS_DIR2_DATA_ALIGN;
    file_postion += dir_entry_length; // 8字节对齐后的下个目录项首地址

    // 根据文件名和inode号，确认目标文件是否被找到
    char str[NAME_LEN] = {0};
    char str_cp[NAME_LEN] = {0};
    const char *str_file;
    uint16_t length = 0;

    CHECK_MEMCPY_S_OK(str, NAME_LEN, xd2d_dir_info->name, xd2d_dir_info->namelen);
    str[xd2d_dir_info->namelen] = '\0';
    AFS_TRACE_OUT_DBG("find a temp direction is %s", str);

    if ((!m_file_path_arr.empty()) && ((size_t)ULLONG_MAX != m_file_path_arr.size())) {
        str_file = m_file_path_arr.front().c_str();
        length = strlen(str_file);
    } else {
        str_file = " ";
        length = 1;
    }

    CHECK_MEMCPY_S_OK(str_cp, NAME_LEN, const_cast<char *>(str_file), length);
    str_cp[length] = '\0';

    ret = xfs_analyzeBlockDirWhilestr(img_reader, str_cp, str, xd2d_dir_info);
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("The logical is error.");
        return ret;
    }

    if (TARGET_FLAG == m_target_dir_flag && ((strncmp(".", str, 1) != 0) && (strncmp("..", str, 2) != 0))) {
        inode_number = afs_bswap_64(xd2d_dir_info->inumber);
        // 指定目标为目录时，将子文件inode保存
        if ((uint64_t)(inode_number * m_inodesize) < (uint64_t)((img_reader->m_imageinfo.length) << 9)) {
            m_que_save_inode.push(inode_number);
            AFS_TRACE_OUT_DBG("m_que_save_inode push a inode [%llu]", inode_number);
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief 将文件名与目录块里存放的文件名逐个匹配，找到目标文件的话返回inod号
 *
 * @param *img_reader     文件对象
 * @param count_blk       块数量
 * @param &xd2d_tag       文件相对于本目录块的偏移
 * @param *str_cp         拆分后的文件路径名
 * @param *str            目录entry存放的文件名
 * @param *xd2d_dir_info  目录entry信息
 * @return 0:处理成功
 * 负数：错误ID
 */
int32_t xfsHandler::xfs_analyzeBlockDirWhilestr(imgReader *img_reader, char *str_cp, char *str,
    xfs_dir2_data_entry_t *xd2d_dir_info)
{
    int32_t ret = AFS_SUCCESS;
    xfs_dir2_data_entry_t *tmp_pos = xd2d_dir_info;

    if (0 != strcmp(str_cp, str)) {
        return AFS_SUCCESS;
    }

    m_tag_inode = afs_bswap_64(tmp_pos->inumber);
    m_file_path_arr.pop();
    AFS_TRACE_OUT_DBG("find a dependent path [ %s, inode = %llu ] of object file", str, m_tag_inode);

    if (((0 == m_file_path_arr.size()))) {
        // 找到目标文件，返回inode号
        xfs_freeQueue();
        m_found_target_file = 1;
        if (m_tag_inode != 0) {
            m_que_save_inode.push(m_tag_inode);
        }
        AFS_TRACE_OUT_INFO("find the object file [ %s ]", str);
    } else {
        uint64_t offset_read = xfs_getOffset(m_tag_inode, INODE_SET);
        if (-1 == static_cast<int64_t>(offset_read)) {
            AFS_TRACE_OUT_ERROR("Failed to get offset. m_tag_inode = %lld", (long long)m_tag_inode);
            return AFS_ERR_INNER;
        }

        ret = xfs_readInodeInfo(img_reader, offset_read);
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("Failed to call xfs_readInodeInfo function.");
            return ret;
        }

        if (m_target_dir_flag != TARGET_FLAG) {
            m_found_target_file = 1; // /避免循环遍历所有的文件路径
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief 读取中间节点的块信息
 *
 * @param *reader    文件对象
 * @param offset_ret 地址偏移
 * @param blk_num    块数量
 * @param *block_msg 存放块信息的起始地址
 * @return 0: 成功
 * 负数：失败
 *
 */
int32_t xfsHandler::xfs_readBlockInfoMid(imgReader *reader, uint64_t offset_ret, uint16_t blk_num, uint8_t *block_msg)
{
    int32_t ret_val = AFS_SUCCESS;

    xfs_bmdr_ptr_t *bmap_ptr = NULL; // ptr指针信息
    uint64_t bmp_ptr = 0;
    uint64_t tree_blknum = 0;
    uint64_t file_postion = m_prt_off;

    for (uint16_t i = 0; i < blk_num; i++) {
        bmap_ptr = reinterpret_cast<xfs_bmdr_ptr_t *>(block_msg + file_postion);

        // 获取跳转指针
        bmp_ptr = afs_bswap_64((bmap_ptr->br_ptr));
        file_postion += sizeof(xfs_bmdr_ptr_t);

        if ((uint64_t)(bmp_ptr * m_blocksize) > ((uint64_t)((reader->m_imageinfo.length) << 9) - m_blocksize)) {
            AFS_TRACE_OUT_ERROR("Invalid block number. blk=%lld", (long long)bmp_ptr);
            continue;
        }
        m_que_tree_blknum.push(bmp_ptr);
    }

    uint64_t offset_read = 0;
    while (!m_que_tree_blknum.empty()) {
        tree_blknum = m_que_tree_blknum.front();
        m_que_tree_blknum.pop();
        // 根据指针，计算出偏移量
        offset_read = xfs_getOffset(tree_blknum, BLOCK_SET);
        if (-1 == static_cast<int64_t>(offset_read)) {
            AFS_TRACE_OUT_ERROR("Failed to get offset.");
            return AFS_ERR_INNER;
        }

        ret_val = xfs_readBlockInfo(reader, offset_read);
        if (AFS_SUCCESS != ret_val) {
            AFS_TRACE_OUT_ERROR("The logical is error.");
            return ret_val;
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief 读取叶子节点的块信息
 *
 * @param img_reader 文件对象
 * @param offset_ret 地址偏移
 * @param blk_num 块数量
 * @param file_postion 偏移量
 * @param block_msg 存放块信息的起始地址
 * @return -1内部错误
 * 正常返回值0
 */
int32_t xfsHandler::xfs_readBlockInfoLeaf(imgReader *img_reader, uint64_t offset_ret, uint16_t blk_num,
    uint32_t file_postion, uint8_t *block_msg)
{
    my_rec_use_read *recs_info = NULL;
    xfs_bmbt_irec_t ret_irec;
    xfs_bmbt_irec_t *pret_irec = &ret_irec;
    uint64_t offset_blk = 0;
    uint64_t target_pos = 0;
    uint64_t target_len = 0;
    CHECK_MEMSET_S_OK(&ret_irec, sizeof(ret_irec), 0, sizeof(ret_irec));
    int32_t ret = 0;

    for (uint16_t i = 0; i < blk_num; i++) {
        // 获取块信息（起始块，块个数，块偏移等）
        recs_info = reinterpret_cast<my_rec_use_read *>(block_msg + file_postion);
        ret = xfs_analyzeData(recs_info, pret_irec);
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("xfs analyze data failed, ret = %d", ret);
            return ret;
        }

        file_postion += sizeof(my_rec_use_read);

        if (m_target_dir_flag != TARGET_FLAG) {
            if (pret_irec->br_startoff >= XFS_DIR2_LEAF_OFFSET) {
                AFS_TRACE_OUT_DBG("find a leaf, node or freeindex extents in Btree");
                continue;
            }
            m_que_bmap_sta.push(pret_irec->br_startblock);
            m_que_bmap_cnt.push(pret_irec->br_blockcount);
        } else {
            // 指定一个文件过滤
            offset_blk = xfs_getOffset(pret_irec->br_startblock, BLOCK_SET);
            if (-1 == static_cast<int64_t>(offset_blk)) {
                AFS_TRACE_OUT_ERROR("Failed to get offset.");
                return AFS_ERR_INNER;
            }

            target_pos = offset_blk / SERCTORSIZE;
            target_len = (pret_irec->br_blockcount * m_blocksize) / SERCTORSIZE;

            m_que_ret_bm_msg_pos.push(target_pos);
            m_que_ret_bm_msg_len.push(target_len);
        }
    }

    return AFS_SUCCESS;
}
/**
 * @brief 函数xfs_readBlockInfo中crc判断
 *
 * @param *img_reader     文件对象
 * @param *block_msg      存放块信息的起始地址
 * @param &blk_num        块数量
 * @param &file_postion   文件起始偏移
 * @param &tree_level     B+ tree深度
 * @return 0: 成功
 * 负数：失败
 *
 */
int32_t xfsHandler::xfs_readBlockInfo_crc(imgReader *img_reader, uint8_t *block_msg, uint16_t &blk_num,
    uint32_t &file_postion, uint16_t &tree_level)
{
    xfs_btree_block_info *block_info = NULL;
    xfs_btree_block *t_block_info = NULL;

    if (!m_crc) {
        block_info = reinterpret_cast<xfs_btree_block_info *>(block_msg);
        if (afs_bswap_32(block_info->bb_magic) != (__be32)XFS_BMAP_MAGIC) {
            AFS_TRACE_OUT_ERROR("Invalid magic. %d", afs_bswap_32(block_info->bb_magic));
            return AFS_ERR_INNER;
        }

        blk_num = afs_bswap_16(block_info->bb_numrecs);
        tree_level = afs_bswap_16(block_info->bb_level);
        file_postion += sizeof(xfs_btree_block_info);
    } else {
        t_block_info = reinterpret_cast<xfs_btree_block *>(block_msg);
        if (afs_bswap_32(t_block_info->bb_magic) != (__be32)XFS_BMAP_CRC_MAGIC) {
            AFS_TRACE_OUT_ERROR("Invalid magic. %d", afs_bswap_32(t_block_info->bb_magic));
            return AFS_ERR_INNER;
        }
        blk_num = afs_bswap_16(t_block_info->bb_numrecs);
        tree_level = afs_bswap_16(t_block_info->bb_level);
        file_postion += sizeof(xfs_btree_block);
    }

    return AFS_SUCCESS;
}
/**
 * @brief  读取块信息
 *
 * @param  img_reader 文件对象
 * @param  offset_ret 地址偏移
 * @return 0： 成功
 * 负数：失败
 *
 */
int32_t xfsHandler::xfs_readBlockInfo(imgReader *img_reader, uint64_t offset_ret)
{
    uint8_t block_msg[BLOCKSIZE] = {0};

    uint16_t blk_num = 0;
    uint16_t tree_level = 0;
    int32_t ret_val = AFS_SUCCESS;
    uint32_t file_postion = 0;
    uint64_t start_blk = 0;
    uint64_t count_blk = 0;
    int64_t read_len = 0;

    // 读取块信息
    AFS_TRACE_OUT_DBG("before read: offset_ret:%llu,BLOCKSIZE:%d.", offset_ret, BLOCKSIZE);
    read_len = img_reader->read(block_msg, offset_ret, BLOCKSIZE, 0);
    if (BLOCKSIZE != read_len) {
        AFS_TRACE_OUT_ERROR("Failed to read data.");
        return AFS_ERR_IMAGE_READ;
    }

    // 解析B+树，获取节点个数
    ret_val = xfs_readBlockInfo_crc(img_reader, block_msg, blk_num, file_postion, tree_level);
    if (AFS_SUCCESS != ret_val) {
        AFS_TRACE_OUT_ERROR("Failed to call xfs_readBlockInfo_crc function. ret = %d", ret_val);
        return ret_val;
    }

    if (0 == tree_level) {
        ret_val = xfs_readBlockInfoLeaf(img_reader, offset_ret, blk_num, file_postion, block_msg);
    } else {
        ret_val = xfs_readBlockInfoMid(img_reader, offset_ret, blk_num, block_msg);
    }

    if (AFS_SUCCESS != ret_val) {
        AFS_TRACE_OUT_ERROR("The logical is error.");
        return ret_val;
    }

    // 根据节点个数，获取块信息
    while ((!m_que_bmap_sta.empty()) && (!m_que_bmap_cnt.empty())) {
        // 获取到起始块和块个数
        start_blk = m_que_bmap_sta.front();
        count_blk = m_que_bmap_cnt.front();
        m_que_bmap_sta.pop();
        m_que_bmap_cnt.pop();

        ret_val = xfs_analyzeBlockDir(img_reader, start_blk, count_blk);
        if (AFS_SUCCESS != ret_val) {
            AFS_TRACE_OUT_ERROR("The logical is error.");
            return ret_val;
        }
        if (m_found_target_file == 1) {
            AFS_TRACE_OUT_DBG("btree m_found_target_file == 1");
            break;
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief 判断Local结构中的Entry是否包含文件类型域（占1个字节，如SUSE11操作系统中不存储该域）
 * @param img_reader   读取镜像的Reader指针
 * @param offset_ret   Inode偏移位置
 * @return
 * AFS_ERR_FS_VERSION
 * AFS_ERR_IMAGE_READ
 * AFS_SUCCESS
 */
int32_t xfsHandler::xfs_isFileType(imgReader *img_reader, uint64_t offset_ret)
{
    xfs_dinode_t *root_inode = NULL; // 文件inode信息
    uint8_t root[INODESIZE] = {0}; // 根文件
    int64_t read_len = 0;

    // 读取根文件inode信息
    AFS_TRACE_OUT_DBG("before read: offset_ret:%llu,BLOCKSIZE:%d.", offset_ret, INODESIZE);
    read_len = img_reader->read(root, offset_ret, INODESIZE, 0);
    if (INODESIZE != read_len) {
        AFS_TRACE_OUT_ERROR("Failed to read data.");
        return AFS_ERR_IMAGE_READ;
    }

    // 目录
    root_inode = reinterpret_cast<xfs_dinode_t *>(root);
    if ((afs_bswap_16(root_inode->di_magic) == (__be16)XFS_DINODE_MAGIC)) {
        AFS_TRACE_OUT_DBG("Success to check file type.");
        return AFS_SUCCESS;
    }

    return AFS_ERR_FS_VERSION;
}

/**
 * @brief 按照目录寻找,找到目标文件返回inode号；没找到的话继续寻找直到最后一层
 * @param file_path    目标文件上的中间路径或最终文件
 * @return 0：成功
 * 负数：失败
 */
int32_t xfsHandler::xfs_readInodeInfoLocalIfLoopDir(const char *file_path)
{
    int32_t ret = AFS_SUCCESS;
    imgReader *reader = getImgReader();

    // 按照目录寻找，匹配到一层就释放一层目录
    m_file_path_arr.pop();

    if ((0 == m_file_path_arr.size()) || ((size_t)ULLONG_MAX == m_file_path_arr.size())) {
        // 找到目标文件，返回inode号
        xfs_freeQueue();
        m_found_target_file = 1;
        if (m_tag_inode != 0) {
            m_que_save_inode.push(m_tag_inode);
        }
        AFS_TRACE_OUT_INFO("the object file [%s] has been found", file_path);
    } else {
        // 没有到最后一层，继续寻找
        uint64_t offset_ret = xfs_getOffset(m_tag_inode, INODE_SET);
        if (-1 == static_cast<int64_t>(offset_ret)) {
            AFS_TRACE_OUT_ERROR("Failed to get offset.");
            return AFS_ERR_INNER;
        }

        ret = xfs_readInodeInfo(reader, offset_ret);
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("The logical is error.");
            return ret;
        }

        if (m_target_dir_flag != TARGET_FLAG) {
            m_found_target_file = 1; // /避免循环遍历所有的文件路径
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief 文件名匹配的话返回目标文件的inode
 *
 * @param *str               目录entry存放的文件名
 * @param *str_cp            拆分后的文件路径名
 * @param *xfs_file_type     包含filetype的entry结构
 * @param *xfs_no_file_type  不包含filetype的entry结构
 * @return 0：成功
 * 负数：失败
 *
 */
int32_t xfsHandler::xfs_readInodeInfoLocalIf(char *str, char *str_cp, xfs_dir2_info_t *xfs_file_type,
    xfs_dir2_info_f_t *xfs_no_file_type)
{
    int32_t ret = AFS_SUCCESS;
    uint64_t byte_rest = 0;

    if (0 != strcmp(str_cp, str)) {
        return AFS_SUCCESS;
    }

    if (m_filetype_flag == FILE_TYPE_1) {
        byte_rest = xfs_calcInodeValue(xfs_file_type->xfs_inou);
        if (-1 == static_cast<int64_t>(byte_rest)) {
            AFS_TRACE_OUT_ERROR("Failed to get offset.");
            return AFS_ERR_INNER;
        }
    } else if (m_filetype_flag == FILE_TYPE_2) {
        byte_rest = xfs_calcInodeValue(xfs_no_file_type->xfs_inou);
        if (-1 == static_cast<int64_t>(byte_rest)) {
            AFS_TRACE_OUT_ERROR("Failed to get offset.");
            return AFS_ERR_INNER;
        }
    }

    m_tag_inode = byte_rest;
    AFS_TRACE_OUT_INFO("find a dependent path [ %s, inode=%llu ] of object file", str, m_tag_inode);

    ret = xfs_readInodeInfoLocalIfLoopDir(str_cp);
    return ret;
}

/**
 * @brief 指定目标为目录时，保存子文件inode
 *
 * @param *str              目录entry存放的文件名
 * @param *xfs_file_type    包含filetype的entry结构
 * @param *xfs_no_file_type 不包含filetype的entry结构
 *
 * @return 0：成功
 * 负数：失败
 */
int32_t xfsHandler::xfs_readInodeInfoLocalif_1(char *str, xfs_dir2_info_t *xfs_file_type,
    xfs_dir2_info_f_t *xfs_no_file_type)
{
    uint64_t inode_value = 0;

    if (TARGET_FLAG == m_target_dir_flag && ((strncmp(".", str, 1) != 0) || (strncmp("..", str, 2) != 0))) {
        // 指定目标为目录时，将子文件inode保存
        if (m_filetype_flag == FILE_TYPE_1) {
            inode_value = xfs_calcInodeValue(xfs_file_type->xfs_inou);
            if (-1 == static_cast<int64_t>(inode_value)) {
                AFS_TRACE_OUT_ERROR("Failed to call xfs_byteDirInfo function.");
                return AFS_ERR_INNER;
            }
        } else if (m_filetype_flag == FILE_TYPE_2) {
            inode_value = xfs_calcInodeValue(xfs_no_file_type->xfs_inou);
            if (-1 == static_cast<int64_t>(inode_value)) {
                AFS_TRACE_OUT_ERROR("Failed to call xfs_byteDirInfo function.");
                return AFS_ERR_INNER;
            }
        }

        m_que_save_inode.push(inode_value);
        AFS_TRACE_OUT_DBG("m_que_save_inode push a inode %llu", inode_value);
    }

    return AFS_SUCCESS;
}

/**
 * @brief 分析Local模式存储时，Entry中是否含有文件类型的域
 * @param *root          根偏移位置
 * @param &file_postion  文件开始偏移
 * @return
 * AFS_ERR_FS_VERSION
 * AFS_ERR_INNER
 * AFS_SUCCESS
 */
int32_t xfsHandler::xfs_readInodeInfoLocalDoFile0(uint8_t *root, uint64_t &file_postion)
{
    int32_t ret = AFS_ERR_FS_VERSION;
    xfs_dir2_info *xfs_file_type = NULL; // 文件信息（文件类型，文件inode）
    imgReader *reader = getImgReader();

    // 由于开始时不确定Entry中是否存储文件类型域(占1个字节)，所以先按照有该域计算，如分析失败则按没有该域分析
    // 大部分操作系统会存储文件类型的空间，目前分析到Suse11以及Centos6.x不存储该域
    for (uint8_t tmp_index = 0; tmp_index < 2; tmp_index++) {
        // 判断结构体中是否包含file_type
        xfs_file_type = reinterpret_cast<xfs_dir2_info *>(root + file_postion - tmp_index);
        uint64_t uret = xfs_calcInodeValue(xfs_file_type->xfs_inou);
        if (static_cast<uint64_t>(-1) == uret) {
            AFS_TRACE_OUT_ERROR("Failed to get INODE value.");
            ret = AFS_ERR_INNER;
            continue;
        }

        uint64_t offset_read = xfs_getOffset(uret, INODE_SET);
        if (-1 == static_cast<int64_t>(offset_read)) {
            AFS_TRACE_OUT_ERROR("Failed to get INODE offset.");
            ret = AFS_ERR_INNER;
            continue;
        }

        int32_t ret = xfs_isFileType(reader, offset_read);
        if (AFS_ERR_FS_VERSION == ret) {
            continue;
        }

        if (tmp_index == 0) {
            m_filetype_flag = FILE_TYPE_1; // Entry包含文件类型域
            return AFS_SUCCESS;
        } else {                           // if (tmp_index == 1)
            m_filetype_flag = FILE_TYPE_2; // Entry不包含文件类型域
            return AFS_SUCCESS;
        }
    }
    AFS_TRACE_OUT_ERROR("Failed to call xfs_isFileType function.");

    return ret;
}

/**
 * @brief 根据文件名和inode号，确认目标文件是否被找到
 * @param *xfs_file_info     entry结构体
 * @param *xfs_file_type     包含fstype域时的inode信息
 * @param *xfs_no_file_type  不包含fstype域时的inode信息
 * @return 0:成功
 * 负数：失败
 */
int32_t xfsHandler::xfs_riilDoFile(xfs_dir2_sf_entry_t *xfs_file_info, xfs_dir2_info_t *xfs_file_type,
    xfs_dir2_info_f_t *xfs_no_file_type)
{
    char str[NAME_LEN] = {0};
    char str_cp[NAME_LEN] = {0};
    const char *str_file;
    uint16_t length = 0;
    int32_t ret = AFS_SUCCESS;

    CHECK_MEMCPY_S_OK(str, NAME_LEN, xfs_file_info->name, xfs_file_info->namelen);
    str[xfs_file_info->namelen] = '\0';

    if ((!m_file_path_arr.empty()) && ((size_t)ULLONG_MAX != m_file_path_arr.size())) {
        str_file = m_file_path_arr.front().c_str();
        length = strlen(str_file);
    } else {
        str_file = " ";
        length = 1;
    }

    CHECK_MEMCPY_S_OK(str_cp, NAME_LEN, const_cast<char *>(str_file), length);
    str_cp[length] = '\0';
    AFS_TRACE_OUT_INFO("find a file name: %s [target file is %s]", str, str_cp);

    ret = xfs_readInodeInfoLocalIf(str, str_cp, xfs_file_type, xfs_no_file_type);
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Failed to call xfs_readInodeInfoLocalIf function.");
        return ret;
    }

    return xfs_readInodeInfoLocalif_1(str, xfs_file_type, xfs_no_file_type);
}

/**
 * @brief 遍历根目录存储格式为local
 *
 * @param *reader       文件对象
 * @param file_postion  地址偏移
 * @param i8count_p     函数xfs_readInodeInfo中获取的文件名的长度0或者4
 * @param *root         存取块信息的起始地址
 * @param *head_info    存取entry信息的起始地址
 * @return 负数：失败
 * 0：成功
 *
 */
int32_t xfsHandler::xfs_readInodeInfoLocal(imgReader *reader, uint64_t file_postion, uint8_t i8count_p, uint8_t *root,
    xfs_dir2_sf_hdr_t *head_info)
{
    // 获取entry信息（文件名长度，地址偏移，文件名等）
    xfs_dir2_sf_entry_t *xfs_file_info = NULL; // 文件信息（文件名，文件名长度）
    xfs_dir2_info_t *xfs_file_type = NULL;     // 文件信息（文件类型，文件inode）
    xfs_dir2_info_f_t *xfs_no_file_type = NULL;
    int32_t ret = AFS_SUCCESS;
    uint8_t file_namelen = 0;
    uint8_t loop_count = (0 == i8count_p) ? head_info->i8count : head_info->count;

    for (uint8_t i = 1; i <= loop_count; i++) {
        AFS_TRACE_OUT_DBG("xfs_readInodeInfoLocal loop_cnt is %u, now is %u", loop_count, i);
        xfs_file_info = reinterpret_cast<xfs_dir2_sf_entry_t *>(root + file_postion);
        file_namelen = xfs_file_info->namelen;
        file_postion += (file_namelen + sizeof(xfs_dir2_sf_entry_t));

        if (m_filetype_flag == FILE_TYPE_0) {
            // 设置当前文件系统的Inode是否占用8字节存储
            m_inode_8bit_flag = (0 == i8count_p) ? 1 : 0; // 1(8字节存储), 0(4字节存储)
            ret = xfs_readInodeInfoLocalDoFile0(root, file_postion);
            if (AFS_SUCCESS != ret) {
                AFS_TRACE_OUT_ERROR("Failed to calculate logical address.");
                return ret;
            }
        }

        // xfs_readInodeInfoLocalDoFile0函数中可能会更新m_filetype_flag的值
        if (m_filetype_flag == FILE_TYPE_1) {
            // 对filetype位做处理
            xfs_file_type = reinterpret_cast<xfs_dir2_info_t *>(root + file_postion);
            file_postion += sizeof(xfs_dir2_info_t) - i8count_p;
        } else if (m_filetype_flag == FILE_TYPE_2) {
            xfs_no_file_type = reinterpret_cast<xfs_dir2_info_f_t *>(root + file_postion);
            file_postion += sizeof(xfs_dir2_info_f_t) - i8count_p;
        }

        // 根据文件名和inode号，确认目标文件是否被找到
        ret = xfs_riilDoFile(xfs_file_info, xfs_file_type, xfs_no_file_type);
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("The logical is error.");
            return ret;
        }

        if (m_found_target_file == 1) {
            AFS_TRACE_OUT_DBG("local m_found_target_file == 1");
            break;
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief 读取存储格式为BTree的inode的中间节点信息
 *
 * @param *img_reader  文件对象
 * @param *root        存取块信息的起始地址
 * @param numrecs      数组元素个数
 *
 * @return 负数：失败
 * 0：成功
 *
 */
int32_t xfsHandler::xfs_readInodeInfoBtreelevel(imgReader *img_reader, uint8_t *root, uint16_t numrecs)
{
    uint64_t file_postion = 0;
    uint64_t bmp_ptr = 0;
    uint64_t offset_read = 0;
    int32_t ret = 0;
    uint64_t tree_blknum = 0;
    xfs_bmdr_ptr_t *bmap_ptr = NULL; // ptr指针信息
    // 新增部分di_forkoff=0和!=0两种情况
    if (root[82] == 0) { // root[82]:di_forkoff
        file_postion = m_offset_tree;
    } else {
        file_postion = (uint64_t)(root[82] * 8 - 4) / (sizeof(uint64_t) * 2) * sizeof(uint64_t) + m_offset_hdr + 4;
    }

    for (uint16_t i = 0; i < numrecs; i++) {
        bmap_ptr = reinterpret_cast<xfs_bmdr_ptr_t *>(root + file_postion);

        // 获取跳转指针
        bmp_ptr = afs_bswap_64((bmap_ptr->br_ptr));
        m_que_tree_blknum.push(bmp_ptr);

        file_postion += sizeof(xfs_bmdr_ptr_t);
    }

    while (!m_que_tree_blknum.empty()) {
        tree_blknum = m_que_tree_blknum.front();
        m_que_tree_blknum.pop();
        // 根据指针，计算出偏移量

        offset_read = xfs_getOffset(tree_blknum, BLOCK_SET);
        if (-1 == static_cast<int64_t>(offset_read)) {
            AFS_TRACE_OUT_ERROR("Failed to get offset. tree_blknum = %lld", (long long)tree_blknum);
            return AFS_ERR_INNER;
        }

        ret = xfs_readBlockInfo(img_reader, offset_read);
        if (AFS_SUCCESS != ret) {
            return ret;
        }

        if (m_found_target_file == 1) {
            AFS_TRACE_OUT_DBG("btree m_found_target_file == 1");
            break;
        }
    }
    return AFS_SUCCESS;
}

/**
 * @brief 读取存储格式为Btree的inode信息
 *
 * @param *img_reader  文件对象
 * @param offset_ret   地址偏移
 * @param *root        存取块信息的起始地址
 *
 * @return 0:成功
 * 负数:失败
 *
 */
int32_t xfsHandler::xfs_readInodeInfoBtree(imgReader *img_reader, uint64_t offset_ret, uint8_t *root)
{
    // 存储格式为btree
    uint64_t file_postion = 0;
    xfs_bmdr_block *tree_info = NULL; // 树结构信息
    my_rec_use_read *recs_info = NULL;
    xfs_bmbt_irec_t ret_irec;
    xfs_bmbt_irec_t *pret_irec = &ret_irec;
    uint16_t numrecs = 0;
    uint16_t level_tree = 0;
    int32_t ret = 0;
    CHECK_MEMSET_S_OK(&ret_irec, sizeof(ret_irec), 0, sizeof(ret_irec));

    file_postion = m_offset_hdr;
    tree_info = reinterpret_cast<xfs_bmdr_block *>(root + file_postion);
    level_tree = afs_bswap_16(tree_info->bb_level);
    numrecs = afs_bswap_16(tree_info->bb_numrecs);

    if (0 == level_tree) {
        // 获取块信息（起始块，块个数，块偏移等）
        recs_info = reinterpret_cast<my_rec_use_read *>(root + file_postion);
        ret = xfs_analyzeData(recs_info, pret_irec);
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("xfs analyze data failed, ret = %d", ret);
            return ret;
        }

        // 对获取到的数据块进行判断，是否是需要的
        ret = xfs_analyzeBlockDir(img_reader, pret_irec->br_startblock, pret_irec->br_blockcount);
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("xfs analyze block directorys failed, ret = %d", ret);
            return ret;
        }
        file_postion += sizeof(my_rec_use_read);
    } else {
        ret = xfs_readInodeInfoBtreelevel(img_reader, root, numrecs);
        if (AFS_SUCCESS != ret) {
            return ret;
        }
    }
    return AFS_SUCCESS;
}

/**
 * @brief 读取存储格式为extent模式的inode信息
 *
 * @param *root       存取块信息的起始地址
 * @param *root_inode 将root重定向为xfs_dinode_t结构
 * @return 0：成功
 * 负数:失败
 */
int32_t xfsHandler::xfs_readInodeInfoDoModeExt(uint8_t *root, xfs_dinode_t *root_inode)
{
    int32_t ret = AFS_SUCCESS;

    my_rec_use_read *recs_info = NULL;
    xfs_bmbt_irec_t ret_irc;
    xfs_bmbt_irec_t *pret_irc = &ret_irc;
    imgReader *reader = getImgReader();
    CHECK_MEMSET_S_OK(&ret_irc, sizeof(ret_irc), 0, sizeof(ret_irc));

    for (__be32 i = 0; i < afs_bswap_32(root_inode->di_nextents); i++) {
        // 获取起始块个块个数信息
        recs_info = reinterpret_cast<my_rec_use_read *>(root + m_offset_hdr + FILE_OFFSET * i);

        ret = xfs_analyzeData(recs_info, pret_irc);
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("xfs analyze data failed, ret = %d", ret);
            return ret;
        }

        if (pret_irc->br_startoff >= XFS_DIR2_LEAF_OFFSET) {
            AFS_TRACE_OUT_DBG(
                "extents.startoff is %llu, thus it is a leaf block or free index block, not a directory block",
                pret_irc->br_startoff);
            continue;
        }

        AFS_TRACE_OUT_DBG("xfs_readInodeInfoDoModeExt, i = %d, di_nextexts is %u", i,
            afs_bswap_32(root_inode->di_nextents));

        // 分析块目录，返回目标文件的inode
        ret = xfs_analyzeBlockDir(reader, pret_irc->br_startblock, pret_irc->br_blockcount);
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("Failed to call function xfs_analyzeBlockDir.");
            return ret;
        }

        if (m_found_target_file) {
            break;
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief 存储模式判断
 *
 * @param *root       存取块信息的起始地址
 * @param *root_inode 将root重定向为xfs_dinode_t结构
 * @param offset_ret  偏移
 * @return 0：成功
 * 负数：失败
 */
int32_t xfsHandler::xfs_readInodeInfoDoMode(uint8_t *root, xfs_dinode_t *root_inode, uint64_t offset_ret)
{
    uint8_t i8count_p = 0;
    uint64_t file_postion = 0;
    xfs_dir2_sf_hdr_t *head_info = NULL; // 文件头信息

    imgReader *reader = getImgReader();

    AFS_TRACE_OUT_DBG("Filter file, inode->di_format=%d", root_inode->di_format);
    // 根据读取的inode信息进行判断，确定存储格式
    if ((uint8_t)XFS_DINODE_FMT_LOCAL == root_inode->di_format) {
        // 存储格式为local
        file_postion = m_offset_hdr;
        // 读取entry信息（包括内部文件个数）
        head_info = reinterpret_cast<xfs_dir2_sf_hdr_t *>(root + file_postion);
        i8count_p = xfs_getFilenameLen(head_info->i8count, i8count_p);
        file_postion += (sizeof(xfs_dir2_sf_hdr_t) - i8count_p);

        // /根据文件个数遍历目录
        return xfs_readInodeInfoLocal(reader, file_postion, i8count_p, root, head_info); // root_inode->di_version
    } else if ((uint8_t)XFS_DINODE_FMT_EXTENTS == root_inode->di_format) {
        // /存储格式为extents
        return xfs_readInodeInfoDoModeExt(root, root_inode);
    } else if ((uint8_t)XFS_DINODE_FMT_BTREE == root_inode->di_format) {
        return xfs_readInodeInfoBtree(reader, offset_ret, root);
    }

    AFS_TRACE_OUT_ERROR("can not support this type!");
    return AFS_ERR_INNER;
}

/**
 * @brief 遍历根目录
 *
 * @param *img_reader  文件对象
 * @param offset_read  地址偏移
 * @return 0:成功
 * 负数：失败
 *
 */
int32_t xfsHandler::xfs_readInodeInfo(imgReader *img_reader, uint64_t offset_read)
{
    AFS_TRACE_OUT_DBG("ENTER xfsHandler::read_inode_inf()");

    uint8_t root[INODESIZE] = {0}; // 根文件
    xfs_dinode_t *root_inode = NULL; // 文件inode信息
    int64_t read_len = 0;

    m_tag_inode = 0;

    // 读取根文件inode信息
    AFS_TRACE_OUT_DBG("before read: offset_read:%llu,INODESIZE:%d.", offset_read, INODESIZE);
    read_len = img_reader->read(root, offset_read, INODESIZE, 0);
    if (static_cast<int64_t>(INODESIZE) != read_len) {
        AFS_TRACE_OUT_ERROR("Failed to read data.");
        return AFS_ERR_IMAGE_READ;
    }

    root_inode = reinterpret_cast<xfs_dinode_t *>(root);
    if (afs_bswap_16(root_inode->di_magic) != (__be16)XFS_DINODE_MAGIC) {
        AFS_TRACE_OUT_ERROR("magic_num and version number read error. (%d)", afs_bswap_16(root_inode->di_magic));
        return AFS_ERR_FS_VERSION;
    }

    return xfs_readInodeInfoDoMode(root, root_inode, offset_read);
}

/**
 * @brief 过滤文件时解析超级块数据
 * @return
 * AFS_ERR_INNER
 * AFS_ERR_IMAGE_READ
 * AFS_SUCCESS
 *
 */
int32_t xfsHandler::getFileParseSB()
{
    imgReader *reader = getImgReader();
    int64_t read_len = AFS_SUCCESS;
    xfs_dsb_t *m_dsb = (xfs_dsb_t *)calloc(1, sizeof(xfs_dsb_t));
    if (NULL == m_dsb) {
        AFS_TRACE_OUT_ERROR("Failed to calloc the m_dsb!");
        return AFS_ERR_API;
    }

    // 获取super block的信息
    int64_t stSize = sizeof(xfs_dsb_t);
    bool result = ReadBySectorsBuff(reader, reinterpret_cast<void *>(m_dsb), 0, stSize, 0);
    if (!result) {
        AFS_TRACE_OUT_ERROR("Failed to read super block data.");
        free(m_dsb);
        m_dsb = NULL;
        return AFS_ERR_IMAGE_READ;
    }

    // 将获取到的super block信息，进行数据转换
    m_crc = afs_bswap_32(m_dsb->sb_crc);
    m_rootino = afs_bswap_64(m_dsb->sb_rootino);
    m_inopblock = afs_bswap_16(m_dsb->sb_inopblock);
    m_blocksize = afs_bswap_32(m_dsb->sb_blocksize);
    m_inodesize = afs_bswap_16(m_dsb->sb_inodesize);

    m_inopblog = m_dsb->sb_inopblog;
    m_agblklog = m_dsb->sb_agblklog;
    m_inodelog = m_dsb->sb_inodelog;
    m_agcount = afs_bswap_32(m_dsb->sb_agcount);
    m_agblocks = afs_bswap_32(m_dsb->sb_agblocks);

    m_agino_log = m_dsb->sb_inopblog + m_dsb->sb_agblklog;
    m_blkbb_log = m_dsb->sb_blocklog - BBSHIFT;

    m_has_filetype = xfs_sb_version_hasftype(m_dsb);
    if (m_has_filetype) {
        AFS_TRACE_OUT_DBG("Current XFS filesystem version has \"file type\" field");
    } else {
        AFS_TRACE_OUT_INFO("Current XFS filesystem version has no \"file type\" field");
    }

    AFS_TRACE_OUT_DBG("sb_version is %04x", afs_bswap_16(m_dsb->sb_versionnum));
    AFS_TRACE_OUT_DBG("sb_features2 is %08x", afs_bswap_32(m_dsb->sb_features2));

    // 赋值完成后释放空间
    free(m_dsb);
    m_dsb = NULL;

    // 判断校验位
    AFS_TRACE_OUT_DBG("Current XFS filesystem CRC = %u", m_crc);
    AFS_TRACE_OUT_INFO("Current xfs filesystem blocksize=%u", m_blocksize);
    AFS_TRACE_OUT_DBG("Current XFS filesystem inodesize = %u", m_inodesize);

    if (0 == m_crc) {
        m_offset_hdr = XFS_OFFSET_HDR;
        m_tag = LIMIT_TAG;
    } else {
        m_offset_hdr = XFS_OFFSET_HDR_CRC;
        m_tag = LIMIT_TAG_CRC;
    }

    // (256-100-4)/(8+8)*8+100+4=176=0xB0
    m_offset_tree = (m_inodesize - m_offset_hdr - 4) / (sizeof(uint64_t) * 2) * sizeof(uint64_t) + m_offset_hdr + 4;
    m_prt_off =
        (uint64_t)(m_blocksize - (uint32_t)PTR_SIZE) / (sizeof(uint64_t) * 2) * sizeof(uint64_t) + (uint32_t)PTR_SIZE;

    // FT
    if (!m_crc) {
        m_prt_off -= BTREE_CRC_OFFSET;
    }
    // 计算首次偏移地址
    m_RootPos = (m_rootino / m_inopblock) * m_blocksize;

    return AFS_SUCCESS;
}

/**
 * @brief 过滤文件并设置文件的bitmap
 * @param bitmap
 * @return
 * AFS_ERR_NOT_EXIST_PATH
 * AFS_ERR_INNER
 * AFS_SUCCESS
 */
int32_t xfsHandler::getFileFiltFile(vector<BitMap *> &bitmap_vect)
{
    int32_t ret = AFS_SUCCESS;
    imgReader *reader = getImgReader();

    // 遍历目录，从根目录开始
    ret = xfs_readInodeInfo(reader, m_RootPos);
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("The logical is error.");
        return ret;
    }

    if ((0 != m_file_path_arr.size())) {
        AFS_TRACE_OUT_ERROR("The specified path cannot found.");
        xfs_freeQueue();
        return AFS_ERR_NOT_EXIST_PATH;
    }

    m_found_target_file = 0;

    ret = getFileFiltFile_1(reader);
    if (ret != AFS_SUCCESS) {
        return ret;
    }

    uint64_t target_pos = 0;
    uint64_t target_len = 0;
    uint64_t total_size = 0;
    BitMap temp_bitmap;
    temp_bitmap.bitmapSetBlocksize(SECTOR_SIZE);

    while ((!m_que_ret_bm_msg_pos.empty()) && (!m_que_ret_bm_msg_len.empty())) {
        // 获取设置bitmap需要的信息（坐标和偏移）
        target_pos = m_que_ret_bm_msg_pos.front();
        target_len = m_que_ret_bm_msg_len.front();

        m_que_ret_bm_msg_pos.pop();
        m_que_ret_bm_msg_len.pop();

        total_size += (target_len / 2);

        AFS_TRACE_OUT_DBG("set file filter bitmap from logical addreess: %llu(sectors), length is %llu(sectors)",
            target_pos, target_len);
        // 设置bitmap
        ret = temp_bitmap.bitmapSetRangeMapAddr(reader, target_pos, target_len, bitmap_vect, 1);
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("The logical is error.");
            return ret;
        }
    }
    AFS_TRACE_OUT_INFO("Success to filter file data. size=%lld(KB)", (long long)total_size);

    return AFS_SUCCESS;
}

/**
 * @brief 过滤文件根据目标文件的inode号计算bitmap相关信息
 * @param reader 文件对象
 * @return
 * AFS_ERR_NOT_EXIST_PATH
 * AFS_ERR_INNER
 * AFS_SUCCESS
 */
int32_t xfsHandler::getFileFiltFile_1(imgReader *reader)
{
    int32_t ret = 0;
    uint64_t offset_ret = 0; // /坐标偏移
    imgReader *tmp_reader = reader;

    AFS_TRACE_OUT_DBG("the number of inodes in m_que_save_inode is %lld.", (long long)m_que_save_inode.size());
    while (!m_que_save_inode.empty()) {
        // 通过获取到的文件inode，计算偏移
        uint64_t inode_ret = m_que_save_inode.front();
        m_que_save_inode.pop();
        AFS_TRACE_OUT_DBG("the inode is %lld.", (long long)inode_ret);

        offset_ret = xfs_getOffset(inode_ret, INODE_SET);
        if (-1 == static_cast<int64_t>(offset_ret)) {
            AFS_TRACE_OUT_ERROR("Failed to get offset.");
            return AFS_ERR_INNER;
        }

        // 计算bitmap相关信息
        ret = xfs_msgBitmap(tmp_reader, offset_ret);
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("The logical is error.");
            return ret;
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief 实现找出文件所占块并设置bitmap的功能
 * @param file_path     文件全路径
 * @param bitmap_vect   返回文件在多磁盘上的位图
 * @return 0设置成功 -1设置失败
 *
 */
int xfsHandler::getFile(const char *file_path, vector<BitMap *> &bitmap_vect)
{
    if (NULL == getImgReader()) {
        AFS_TRACE_OUT_ERROR("The reader is NULL!");
        return AFS_ERR_INNER;
    }

    // 初始化空间
    int32_t ret = AFS_SUCCESS;

    string target_file_path = file_path;
    if (strlen(file_path) == 0) {
        // 指定路径为空
        AFS_TRACE_OUT_ERROR("The specified path is empty.");
        return AFS_ERR_PARA_PATH;
    }

    // 解析文件路径，输出数组
    xfs_splitPath(target_file_path);
    if (m_file_path_arr.empty()) {
        AFS_TRACE_OUT_ERROR("Failed to parse path.");
        return AFS_ERR_PARA_PATH;
    }

    ret = getFileParseSB();
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Failed to parse super-block.");
        return ret;
    }

    ret = getFileFiltFile(bitmap_vect);
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Failed to filter file.");
        return ret;
    }

    AFS_TRACE_OUT_INFO("EXIT xfsHandler::getFile()");

    return AFS_SUCCESS;
}

/**
 * @brief 判断当前XFS文件系统的目录项中是否含有 file type
 * @param sbp  XFS的超级块
 * @return 1: 含有filetype  0:不含
 */
uint32_t xfsHandler::xfs_sb_version_hasftype(struct xfs_dsb *sbp)
{
    return (XFS_SB_VERSION_NUM(afs_bswap_16(sbp->sb_versionnum)) == XFS_SB_VERSION_5 &&
               XFS_SB_HAS_INCOMPAT_FEATURE(afs_bswap_32(sbp->sb_features_incompat), XFS_SB_FEAT_INCOMPAT_FTYPE) ||
           (XFS_SB_VERSION_HASMOREBITS(afs_bswap_16(sbp->sb_versionnum)) &&
               (afs_bswap_32(sbp->sb_features2) & XFS_SB_VERSION2_FTYPE)));
}

/**
 * @brief 16位字节逆序
 * @param x
 * @return
 */
uint16_t xfsHandler::afs_bswap_16(uint16_t x)
{
    return ((uint16_t)(x << 8)) | ((uint16_t)(x >> 8));
}

/**
 * @brief 32位字节逆序
 * @param x
 * @return
 */
uint32_t xfsHandler::afs_bswap_32(uint32_t x)
{
    x = (((uint32_t)(x << 8)) & 0xFF00FF00) | (((uint32_t)(x >> 8)) & 0x00FF00FF);

    return ((uint32_t)(x << 16)) | ((uint32_t)(x >> 16));
}

/**
 * @brief 64位字节逆序
 * @param x
 * @return
 */
uint64_t xfsHandler::afs_bswap_64(uint64_t x)
{
    x = (((uint64_t)(x << 8)) & 0xFF00FF00FF00FF00ULL) | (((uint64_t)(x >> 8)) & 0x00FF00FF00FF00FFULL);
    x = (((uint64_t)(x << 16)) & 0xFFFF0000FFFF0000ULL) | (((uint64_t)(x >> 16)) & 0x0000FFFF0000FFFFULL);

    return ((uint64_t)(x >> 32)) | ((uint64_t)(x << 32));
}
