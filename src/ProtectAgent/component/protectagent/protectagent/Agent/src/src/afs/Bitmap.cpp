/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file bitmap.cpp
 * @brief AFS - Bitmap dispose class.
 *
 */

#include "afs/Bitmap.h"
#include <fstream>
#include "afs/LogMsg.h"
#include "afs/FSCommon.h"
#include "afs/AfsError.h"

using namespace std;

/**
 * @brief Bitmap类构造函数
 *
 */
BitMap::BitMap()
{
    m_bitmap = NULL;
    m_bufsize = 0;
    m_blocksize = 512;
    m_blocknum = 0;
}

/**
 * @brief Bitmap类析构函数
 *
 */
BitMap::~BitMap()
{
    if (m_bitmap != NULL) {
        delete[] m_bitmap;
        m_bitmap = NULL;
    }
}

/**
 * @brief 初始化bitmap
 *
 * @param blocknum  块数量
 *
 * @return  0：成功
 * 负数：失败
 *
 */
int32_t BitMap::initBitMap(uint64_t blocknum)
{
    this->m_blocknum = blocknum;
    uint64_t need_byte = (blocknum + 8 - 1) / 8;
    if (0 == need_byte) {
        AFS_TRACE_OUT_ERROR("The new space of specify is zero.");
        return AFS_ERR_INNER;
    }

    m_bitmap = new char[need_byte]();
    if (NULL == m_bitmap) {
        AFS_TRACE_OUT_ERROR("Bitmap buffer have not allocated.");
        return AFS_ERR_API;
    } else {
        CHECK_MEMSET_S_OK(m_bitmap, need_byte, 0, need_byte);
        m_bufsize = need_byte;
        return AFS_SUCCESS;
    }
}

/**
 * @brief 设置Bitmap中1位代表的的大小(字节)
 *
 * @param blocksize 块大小
 *
 * @return 无
 *
 */
void BitMap::bitmapSetBlocksize(uint32_t blocksize)
{
    this->m_blocksize = blocksize;
}

/**
 * @brief 设置bitmap中index位的值
 *
 * @param index Bitmap位置
 * @param value 该位对应的值，0或1
 *
 * @return  0 设置成功
 * -1 设置不成功
 *
 */
int32_t BitMap::bitmapSetMapAddr(uint64_t index, uint8_t value)
{
    if (NULL == m_bitmap) {
        return AFS_ERR_INNER;
    }

    uint64_t addr = index / 8;
    uint8_t addroffset = index % 8;

    unsigned char temp = 0x1 << (7 - addroffset);

    if ((addr + 1) > m_bufsize) {
        AFS_TRACE_OUT_ERROR("Current bitmap position exceed bitmap buffer.");
        return AFS_ERR_INNER;
    }

    if (value) {
        m_bitmap[addr] |= (int8_t)temp;
    } else {
        m_bitmap[addr] &= (int8_t)(~temp);
    }

    return 0;
}

/* * @brief swap分区设置bitmap从start开始的length长度的值
 *
 * @param reader 读取镜像的Reader指针
 * @param start  设置Bitmap的开始位置
 * @param length  设置Bitmap的长度
 * @param flag  该位对应的值，0或1
 * @param bitmap_vect 所有磁盘的位图
 *
 * @return  0 设置成功
 * 负数  设置不成功
 *
 */
int32_t BitMap::bitmapSetRangeSwapMapAddr(imgReader *reader, uint64_t start, uint64_t length, uint8_t flag,
    vector<BitMap *> &bitmap_vect)
{
    int32_t ret = 0;
    int32_t disk_id = -1;
    BitMap *pbitmap;

    if (NULL != reader && 0 != reader->getChunkSize()) { // 满足条件表明是LVM的LV做swap分区
        uint32_t index = 0;
        uint64_t convert_pos = 0;
        uint64_t remain_length = length;
        AFS_TRACE_OUT_INFO("the swap partition is a LVM type");

        // 需要转换地址(字节) ,返回值为物理地址（扇区）-----
        while (index < remain_length) {
            disk_id = -1;
            convert_pos = reader->getVaddrToPaddr((int64_t)(index * m_blocksize), disk_id);
            if (convert_pos == static_cast<uint64_t>(-1)) {
                AFS_TRACE_OUT_ERROR("Failed to get physical address. ret=%llu", (unsigned long long)convert_pos);
                return AFS_ERR_INNER;
            } else if (convert_pos == (uint64_t)LVMNODATA || -1 == disk_id) {
                index++;
                AFS_TRACE_OUT_ERROR("failed: convert_pos = %llu, disk_id = %d", convert_pos, disk_id);
                continue;
            }

            pbitmap = bitmap_vect[disk_id];
            ret = pbitmap->bitmapSetRange(convert_pos, 1, flag);
            if (ret != 0) {
                return ret;
            }
            index++;
        }
    } else {
        AFS_TRACE_OUT_INFO("the swap partition is a physical type");
        ret = bitmapSetRange(start, length, flag); // 物理分区直接做swap分区
    }

    return ret;
}

/* * @brief 设置bitmap从start开始的length长度的值
 *
 * @param reader       读取镜像的Reader指针
 * @param start        设置Bitmap的开始位置(单位：扇区)
 * @param length       设置Bitmap的长度(单位：扇区)
 * @param bitmap_vect  多磁盘的位图
 * @param flag         该位对应的值，0或1
 *
 * @return  0 设置成功
 * 负数  设置不成功
 *
 */
int32_t BitMap::bitmapSetRangeMapAddr(imgReader *reader, uint64_t start, uint64_t length, vector<BitMap *> &bitmap_vect,
    uint8_t flag)
{
    int32_t ret = 0;
    int32_t disk_id = -1;

    if (NULL != reader) {
        uint32_t index = 0;
        uint64_t convert_pos = 0;
        uint64_t pos = start;
        uint64_t tmp_pos = 0;
        uint64_t set_len = 0;
        uint64_t remain_length = length;
        uint32_t searial_len = getBitCount(reader->getMaxStorageZone(), 0);
        BitMap *pBitmap = NULL;

        searial_len <<= 3; // 2^3 = (4096/SECTOR_SIZE); 单次可处理的最大扇区个数
        AFS_TRACE_OUT_INFO("One time can set bitmap bits=%d, and bitmap block size is %u",
            (unsigned long long)searial_len, m_blocksize);

        // 需要转换地址(字节) ,返回值为物理地址（扇区）-----
        tmp_pos = pos % searial_len;
        while (remain_length > 0) {
            index++;
            if (index == 1) {
                set_len = remain_length > (searial_len - tmp_pos) ? (searial_len - tmp_pos) : remain_length;
            } else {
                set_len = remain_length > searial_len ? searial_len : remain_length;
            }

            disk_id = -1;
            convert_pos = reader->getVaddrToPaddr((int64_t)(pos * m_blocksize), disk_id);
            if (convert_pos == static_cast<uint64_t>(-1)) {
                AFS_TRACE_OUT_ERROR("Failed to get physical address. ret=%llu", (unsigned long long)convert_pos);
                return AFS_ERR_INNER;
            } else if ((convert_pos == (uint64_t)LVMNODATA)) {
                remain_length -= set_len;
                pos += set_len;
                AFS_TRACE_OUT_ERROR("No data node get block number. sector-num(%llu) sector-size(%u)", pos,
                    m_blocksize);
                continue;
            }
            if (disk_id == -1) {
                AFS_TRACE_OUT_ERROR("cat not find the right disk id [disk_id = %d]", disk_id);
                return AFS_ERR_INNER;
            }

            pBitmap = bitmap_vect[disk_id];
            ret = pBitmap->bitmapSetRange(convert_pos, set_len, flag);
            if (ret != 0) {
                return ret;
            }

            remain_length -= set_len;
            pos += set_len;
            AFS_TRACE_OUT_DBG("convert_pos is %llu(sectors), disk id is %d, set_len is %llu, remain_len is %llu",
                convert_pos,
                disk_id,
                set_len,
                remain_length);
        }

        ret = AFS_SUCCESS;
    } else {
        AFS_TRACE_OUT_ERROR("reader is NULL when set bitmap within a range of map address");
        return AFS_ERR_PARAMETER;
    }

    return ret;
}

/**
 * @brief 设置bitmap指定位的值
 *
 * @param index  Bitmap Buffer对应的位
 * @param value  设定位的值0或1
 * @return  0 设置成功
 * 负数  失败
 *
 */
int32_t BitMap::bitmapSet(uint64_t index, uint8_t value)
{
    return bitmapSetMapAddr(index, value);
}

/**
 * @brief 设置bitmap从start开始的length长度的值为flag
 *
 * @param start  开始位置
 * @param length 长度
 * @param flag   bitmap的值，0或1
 * @return  0 设置成功
 * 负数  失败
 *
 */
int32_t BitMap::bitmapSetRangeMapAddr(uint64_t start, uint64_t length, uint8_t flag)
{
    int32_t ret = 0;

    uint64_t end = start + length;
    uint64_t pos = 0;
    if ((start + length) > m_bufsize * 8) {
        end = m_bufsize * 8;
    }

    for (pos = start; pos < end; pos++) {
        ret = bitmapSetMapAddr(pos, flag);
        if (ret != AFS_SUCCESS) {
            AFS_TRACE_OUT_ERROR("Failed to set bitmap bit.");
            return AFS_ERR_INNER;
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief 设置范围bitmap
 *
 * @param start 起始
 * @param length 长度
 * @param flag 标志
 *
 * @return
 */
int32_t BitMap::bitmapSetRange(uint64_t start, uint64_t length, uint8_t flag)
{
    return bitmapSetRangeMapAddr(start, length, flag);
}

/**
 * @brief 设置index位的值为1
 *
 * @param *reader  镜像读取指针
 * @param index    要设置的位
 *
 * @return  0 设置成功
 * -1 设置失败
 *
 */
int32_t BitMap::bitmapSetMapAddr(imgReader *reader, uint64_t index)
{
    int32_t disk_id = -1;

    if (NULL != reader) {
        // 需要转换地址(字节) ,返回值为物理地址（扇区）-----
        index = reader->getVaddrToPaddr((int64_t)(index), disk_id);
        if (index == static_cast<uint64_t>(-1)) {
            AFS_TRACE_OUT_ERROR("Failed to get physical address. ret=%llu, disk_id is %d", (unsigned long long)index,
                disk_id);
            return AFS_ERR_INNER;
        }
    }

    return bitmapSetMapAddr(index, 1);
}

/**
 * @brief 设置bitmap
 *
 * @param index 索引地址
 *
 * @return
 */
int32_t BitMap::bitmapSet(uint64_t index)
{
    return bitmapSetMapAddr(index, 1);
}

/**
 * @brief 设置index位的值为0
 *
 * @param *reader 读取镜像的reader指针
 * @param index 要设置的位
 *
 * @return int 0设置不成功 1设置成功
 *
 */
int32_t BitMap::bitmapUnsetMapAddr(imgReader *reader, uint64_t index)
{
    return bitmapSetMapAddr(index, 0);
}

/**
 * @brief 获取bitmap的index位的值
 *
 * @param index 要获取的位数
 *
 * @return 对应位bitmap的值，0或1
 *
 */
int32_t BitMap::bitmapGet(uint64_t index)
{
    uint64_t addr = index / 8;
    uint8_t addroffset = index % 8;
    if (NULL == m_bitmap) {
        AFS_TRACE_OUT_ERROR("Bitmap buffer have not allocated.");
        return AFS_ERR_INNER;
    }

    unsigned char temp = 0x1 << (7 - addroffset);
    if ((addr + 1) > m_bufsize) {
        return 0;
    }

    return (m_bitmap[addr] & temp) != 0 ? 1 : 0;
}

/**
 * @brief 把bitmap保存到文件中
 *
 * @param *filename 文件名
 *
 * @return int 保存的bitmap大小，-1设置失败
 *
 */
int64_t BitMap::bitmapSave(const char *filename)
{
    if (NULL == m_bitmap) {
        AFS_TRACE_OUT_ERROR("Bitmap buffer have not allocated.");
        return -1;
    }

    ofstream out;
    out.open(filename, ios::out | ios::binary | ios::trunc);
    if (out.is_open()) {
        out.write(m_bitmap, m_bufsize);
        out.close();
    }

    return m_bufsize;
}

/**
 * @brief 从文件中加载bitmap到内存
 *
 * @param *filename 文件名
 *
 * @return bitmap大小
 *
 */
int64_t BitMap::bitmapLoad(const char *filename)
{
    ifstream in;
    in.open(filename, ios::in | ios::binary);
    if (in.is_open()) {
        int32_t ret = initBitMap(m_bufsize * 8);
        if (ret != AFS_SUCCESS) {
            in.close();
            return -1;
        }

        in.read(m_bitmap, m_bufsize);
        in.close();

        return m_bufsize;
    }

    return -1;
}

/**
 * @brief bitmap转换成1位代表bytes_per_bit字节(空闲块使用)
 *
 * @param bytes_per_bit 1位代表的字节数
 * @param &newbitmap 转换之后的bitmap
 *
 * @return int 0设置成功 -1:失败
 *
 */
int32_t BitMap::bitmapConvert(uint32_t bytes_per_bit, BitMap &bitmap)
{
    vector<BitMap *> bitmap_vect;
    bitmap_vect.push_back(&bitmap);
    return bitmapConvert(NULL, bytes_per_bit, bitmap_vect);
}

/**
 * @brief bitmap转换成1位代表bytes_per_bit字节(文件位置使用)
 *
 * @param bytes_per_bit 1位代表的字节数
 * @param &newbitmap 转换之后的bitmap
 *
 * @return int 0设置成功 -1:失败
 *
 */
int32_t BitMap::bitmapConvertFile(uint32_t bytes_per_bit, BitMap &newbitmap)
{
    int32_t ret = 0;

    // 如果块大小和输入的大小一致，则不用转换直接Copy数据
    if (m_blocksize == bytes_per_bit) {
        CHECK_MEMCPY_S_OK(newbitmap.getbitmap(), newbitmap.getsize(), this->m_bitmap, getsize());
        return 0;
    }
    // 拆分
    if (m_blocksize > bytes_per_bit) {
        uint32_t temp = m_blocksize / bytes_per_bit; // 整数倍
        for (uint64_t index = 0; index < getsize() * 8; index++) {
            if (!bitmapGet(index)) {
                continue;
            }

            ret = newbitmap.bitmapSetRange(index * temp, temp, 1);
            if (ret != 0) {
                AFS_TRACE_OUT_ERROR("Failed to set bitmap bit.");
                return -1;
            }
        }
        // 合并
    } else {
        ret = bitmapConvertFile_1(bytes_per_bit, newbitmap);
        return ret;
    }

    return AFS_SUCCESS;
}

/**
 * @brief bitmap 文件转换
 *
 * @param bytes_per_bit
 * @param newbitmap
 *
 * @return
 */
int32_t BitMap::bitmapConvertFile_1(uint32_t bytes_per_bit, BitMap &newbitmap)
{
    int32_t ret = 0;
    uint32_t merge_flag = 1; // 等于1时代表需要合并
    uint32_t temp = bytes_per_bit / m_blocksize;

    if (0 == temp) {
        return AFS_ERR_INNER;
    }

    for (uint64_t item_i = 0; item_i < getsize() * 8; item_i += temp) {
        merge_flag = 0; // 初始化
        for (uint32_t item_j = 0; item_j < temp; item_j++) {
            // 当待合并的位中有0时，则不合并
            if (bitmapGet(item_i + item_j)) { // /原位图只要有1就要进行合并
                merge_flag = 1;
                break;
            }
        }
        // merge_flag = 1表示待合并的所有位为全1
        if (!merge_flag) {
            continue;
        }

        ret = newbitmap.bitmapSet(item_i / temp);
        if (ret != 0) {
            AFS_TRACE_OUT_ERROR("Failed to set bitmap bit.");
            return AFS_ERR_INNER;
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief bitmap转换成1位代表bytes_per_bit字节(文件位置使用)
 *
 * @param *img_reader  镜像读取Reader
 * @param bytes_per_bit 1位代表的字节数
 * @param &newbitmap 转换之后的bitmap
 *
 * @return int 0设置成功 -1:失败
 *
 */
int32_t BitMap::bitmapConvert(imgReader *img_reader, uint32_t bytes_per_bit, vector<BitMap *> &bitmap_vect)
{
    int32_t ret = 0;
    BitMap *pbitmap = NULL;

    // 如果块大小和输入的大小一致(512)，则不用转换直接Copy数据(处理物理扇区头部等)
    if (m_blocksize == bytes_per_bit && 1 == bitmap_vect.size()) {
        pbitmap = bitmap_vect[0];
        char *pbuf = pbitmap->getbitmap();
        if (NULL == pbuf) {
            return AFS_ERR_INNER;
        }

        if (NULL == img_reader) {
            CHECK_MEMCPY_S_OK(pbuf, pbitmap->getsize(), this->m_bitmap, getsize());
            return 0;
        } else {
            // 需要根据LVM计算物理磁盘实际位置
        }
    }
    // 处理Bitmap拆分或者合并
    ret = bitmapConvert_1(img_reader, bytes_per_bit, bitmap_vect);

    return ret;
}

/**
 * @brief bitmap转换
 *
 * @param img_reader
 * @param bytes_per_bit
 * @param newbitmap
 *
 * @return 0 成功
 * 负值  失败
 */
int32_t BitMap::bitmapConvert_1(imgReader *img_reader, uint32_t bytes_per_bit, vector<BitMap *> &bitmap_vect)
{
    int32_t ret = -1;
    int32_t disk_id = -1;
    uint64_t index = 0;

    AFS_TRACE_OUT_DBG("m_blocksize is %u, bytes_per_bit is %u", m_blocksize, bytes_per_bit);
    // 拆分(根据LVM PV管理时块大小，每次按最大8个字节处理)
    if (m_blocksize >= bytes_per_bit) {
        // 计算每次处理的Bitmap位数
        uint32_t bits_count_per = 0;
        uint32_t temp = m_blocksize / bytes_per_bit; // 整数倍

        int64_t real_addr = -1;
        uint64_t bitmap_size = getsize();

        if (NULL != img_reader) {
            bits_count_per = getBitCount(img_reader->getMaxStorageZone(), 1);
        } else {
            bits_count_per = 64;
        }

        bitmap_size <<= 3;
        uint32_t bits_dispose = bits_count_per;
        // mod_size主要是处理最后剩余字节
        uint32_t mod_size = (uint8_t)(bitmap_size % bits_count_per);

        AFS_TRACE_OUT_INFO("Start to split bitmap by max storage zone. bits_count_per=%d(bits), mod=%d(bits)",
            bits_count_per, mod_size);

        for (index = 0; index < bitmap_size; index += bits_dispose) { // index += bits_dispose
            bits_dispose = (index + bits_count_per) <= bitmap_size ? bits_count_per : mod_size;
            if (isZeroBits(index, bits_dispose)) {
                continue;
            }

            real_addr = index;
            if (NULL != img_reader) {
                disk_id = -1;
                // 需要转换地址(字节) ,返回值为物理地址（扇区）-----
                real_addr = img_reader->getVaddrToPaddr((int64_t)(index)*m_blocksize, disk_id);
                if (LVMNODATA == real_addr) {
                    AFS_TRACE_OUT_ERROR("No data node get block number. block-num(%llu) block-size(%d)", index,
                        m_blocksize);
                    continue;
                } else if (real_addr < 0) {
                    AFS_TRACE_OUT_ERROR("Failed to get block number. block-num(%llu) block-size(%d)", index,
                        m_blocksize);
                    return AFS_ERR_INNER;
                }
                if (-1 == disk_id) {
                    AFS_TRACE_OUT_ERROR("Failed, real_addr is %llu, disk_id is %d", real_addr, disk_id);
                    return AFS_ERR_INNER;
                }
            }

            ret = bitmapSetRangeByChunk(index, (uint64_t)real_addr, temp, bits_dispose, bitmap_vect, disk_id);
            if (ret != 0) {
                AFS_TRACE_OUT_ERROR("Failed to set bitmap bit.");
                return AFS_ERR_INNER;
            }
        }
        AFS_TRACE_OUT_DBG("finish spilting bitmap");
        AFS_TRACE_OUT_DBG("vaddr[%lld bytes] to paddr is %lld, disk_id is %d", (int64_t)(index)*m_blocksize, real_addr,
            disk_id);
        // 合并
    } else {
        ret = bitmapConvert_2(img_reader, bytes_per_bit, bitmap_vect);
        return ret;
    }

    return AFS_SUCCESS;
}

/**
 * @brief bitmap转换(每次最大处理bits_dispose个字节)
 *
 * @param bitmap_curr_pos  相对于分区bitmap的位置
 * @param real_start_pos   整个镜像的实际bitmap位置
 * @param split_bit        每位需要差分的位数
 * @param bits_dispose     每次处理的位个数
 * @param bitmap_vect      所有磁盘的位图
 *
 * @return
 */
int32_t BitMap::bitmapSetRangeByChunk(uint64_t bitmap_curr_pos, uint64_t real_start_pos, uint32_t split_bit,
    uint8_t bits_dispose, vector<BitMap *> &bitmap_vect, int32_t disk_id)
{
    int32_t ret = 0;
    BitMap *pbitmap = NULL;
    disk_id = bitmap_vect.size() == 1 ? 0 : disk_id;
    pbitmap = bitmap_vect[disk_id];

    for (uint32_t tmp_index = 0; tmp_index < bits_dispose; ++tmp_index) {
        if (!bitmapGet(bitmap_curr_pos + tmp_index)) { // 如果bit为0，跳过
            continue;
        }
        // 将差量位图指定位设置为1
        ret = pbitmap->bitmapSetRangeMapAddr(real_start_pos + (uint64_t)split_bit * tmp_index, split_bit, 1);
        if (ret != 0) {
            AFS_TRACE_OUT_ERROR("Failed to set bitmap bit.");
            return AFS_ERR_INNER;
        }
    }

    return 0;
}

/**
 * @brief 转换bitmap
 *
 * @param *img_reader   镜像读取的指针
 * @param bytes_per_bit 每位代表大小
 * @param &newbitmap    转换后的Bitmap
 *
 * @return
 */
int32_t BitMap::bitmapConvert_2(imgReader *img_reader, uint32_t bytes_per_bit, vector<BitMap *> &bitmap_vect)
{
    int32_t ret = 0;
    uint32_t temp = bytes_per_bit / m_blocksize;
    int32_t disk_id = -1;
    uint64_t physical_addr = 0;
    BitMap *pbitmap = NULL;

    if (0 == temp) {
        return AFS_ERR_INNER;
    }

    for (uint64_t item_i = 0; item_i < getsize() * 8; item_i += temp) {
        for (uint32_t item_j = 0; item_j < temp; item_j++) {
            if (!bitmapGet(item_i + item_j)) {
                continue;
            }

            disk_id = -1;

            if (NULL != img_reader) {
                // 需要装换地址(字节) ,返回值为物理地址（扇区）-----
                physical_addr = img_reader->getVaddrToPaddr((int64_t)(item_i / temp), disk_id);
                if (physical_addr == static_cast<uint64_t>(-1) || -1 == disk_id) {
                    AFS_TRACE_OUT_ERROR("Failed to get physical address. physical_addr=%llu, disk_id =%d",
                        (unsigned long long)physical_addr, disk_id);
                    return AFS_ERR_INNER;
                }
                pbitmap = bitmap_vect[disk_id];
                ret = pbitmap->bitmapSetMapAddr(physical_addr, 1);
            } else if (1 == bitmap_vect.size()) {
                pbitmap = bitmap_vect[0];
                ret = pbitmap->bitmapSetMapAddr(item_i / temp, 1);
            }

            if (ret != 0) {
                AFS_TRACE_OUT_ERROR("Failed to set bitmap bit.");
                return AFS_ERR_INNER;
            }
            break;
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief 计算Bitmap位进行差分时，每次可以处理的位数（根据块大小计算，过滤文件时按4KB计算）
 *
 * @param chunk_size     连续存储单元（扇区数）
 * @param is_free_disk   区分空闲块(1)和文件过滤(0)
 *
 * @return uint32_t 返回一次可处理的位数（bit）
 *
 */
uint32_t BitMap::getBitCount(uint32_t max_storage_size, uint8_t is_free_disk)
{
    uint32_t block_size = 0;
    uint32_t data_block_size = max_storage_size; // 单位：扇区数

    AFS_TRACE_OUT_DBG("Max continuously storage size = %d (sectors)", max_storage_size);

    block_size = is_free_disk > 0 ? m_blocksize : 4096;

    if (0 == data_block_size) {
        return 64; // 非LVM管理时，每次处理64位(8字节)
    }

    data_block_size >>= 1; // chunk_size原单位为扇区，现转换成 K bytes单位
    uint32_t bit_count = (uint32_t)((data_block_size << 10) / block_size); // 一次最大可处理的位数
    uint32_t tmp_char_count = (uint32_t)(bit_count >> 3);

    // 检查是否超过8个字节（最大值）
    if ((tmp_char_count >> 3) > 0) {
        // 每次处理字节数(最大8字节)
        return 64;
    }

    // 小于8个字节时按位处理
    return bit_count;
}

/**
 * @brief 检查指定范围内的Bitmap是否为全0
 *
 * @param current_index    起始位的位置
 * @param bits_count_per   位数
 *
 * @return uint8_t 0：非全0,1：全0
 *
 */
uint8_t BitMap::isZeroBits(uint64_t current_index, uint8_t bits_count_per)
{
    uint8_t tmp_pos = 0;

    // 按位比较
    while (tmp_pos < bits_count_per) {
        if (!bitmapGet(current_index + tmp_pos)) {
            tmp_pos++;
            continue;
        } else {
            return 0;
        }
    }

    return 1;
}
