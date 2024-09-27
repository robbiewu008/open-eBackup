/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file ext4fs.cpp
 * @brief Afs - Analyze EXT file system.
 *
 */

#include "afs/Ext4FS.h"
#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <math.h>
#include "afs/LogMsg.h"
#include "afs/Afslibrary.h"
#include "afs/AfsError.h"

/**
 * @brief 实现strncpy功能
 *
 * @param *dest   目标字符串
 * @param *src    源字符串
 * @param str_len 拷贝的个数
 *
 * @return char* 目标字符串
 *
 */
char *ext4Handler::ext_strnCopy(char *dest, size_t destMax, const char *src, size_t str_len)
{
    char *tmp_dest = dest;

    // buffer长度是否有效
    if (str_len > (size_t)EXT4_NAME_LEN) {
        return NULL;
    }

    int32_t ret = memcpy_s(tmp_dest, destMax, src, str_len);
    if (EOK != ret) {
        AFS_TRACE_OUT_ERROR("call memcpy_s() failed, ret = %d", ret);
        return NULL;
    }

    tmp_dest[str_len] = '\0';

    return tmp_dest;
}

/**
 * @brief 实现字符串分割功能
 *
 * @param &str        待分割字符串
 * @param &pattern    分割字符
 * @param &resVec     分割结束之后的字符串向量
 *
 */
void ext4Handler::ext_split(string &str, const string &pattern, vector<string> &resVec)
{
    if ("" == str) {
        return;
    }
    // 方便截取最后一段数据
    string strs = str + pattern;

    size_t pos = strs.find(pattern);
    size_t size = strs.size();

    while (pos != string::npos) {
        string x = strs.substr(0, pos);
        if ("" != x) {
            resVec.push_back(x);
        }
        strs = strs.substr(pos + 1, size);
        pos = strs.find(pattern);
    }
}

/**
 * @brief ext4获得bitmap
 *
 * @param bitmap BitMap& 输入bitmap用于返回
 * @return int 0设置成功 -1设置失败
 *
 */
int32_t ext4Handler::getBitmap(vector<BitMap *> &bitmap_vect)
{
    AFS_TRACE_OUT_DBG("ENTER ext4Handler::getBitmap()");

    int64_t this_return_value = 0; // 定义返回值，默认返回成功，即0
    int64_t return_value = 0;
    int64_t read_ret = 0;
    struct ext4_brief_info brief_info; // 从超级块中读出的简要信息
    unsigned char *group = NULL;       // 组描述符地址
    unsigned char *temp_group = NULL;  // 为了争取释放内存，临时保存group地址
    BitMap fsbitmap;                   // 声明一个bitmap用于记录块大小为blocksize时的bitmap

    imgReader *img_reader = getImgReader();

    // 分析超级块数据
    return_value = ext_analyzeSuperBlock(img_reader, brief_info);
    if (return_value < 0) {
        AFS_TRACE_OUT_ERROR("Fail to ext_analyzeSuperBlock!");
        this_return_value = return_value;
        goto Tail;
    }
    // 申请gdt内存空间
    group = static_cast<unsigned char *>(calloc(1, brief_info.group_desc_size * brief_info.group_total_num));
    if (NULL == group) {
        AFS_TRACE_OUT_ERROR("Fail to calloc the group!");
        this_return_value = AFS_ERR_API;
        goto Tail;
    }
    // 临时存储group地址，以便正确的内存释放
    temp_group = group;
    // 读取gdt内容
    AFS_TRACE_OUT_DBG("Before read, offset:%llu,count:%llu.",
        brief_info.group_offset,
        brief_info.group_desc_size * brief_info.group_total_num);

    read_ret = img_reader->read(group, brief_info.group_offset,
        static_cast<int64_t>(brief_info.group_desc_size * brief_info.group_total_num), 0);
    if (read_ret != static_cast<int64_t>(brief_info.group_desc_size * brief_info.group_total_num)) {
        AFS_TRACE_OUT_ERROR("Fail to read the group!");
        this_return_value = AFS_ERR_IMAGE_READ;
        goto Tail;
    }

    return_value = ext_getBitmap_realGet(bitmap_vect, fsbitmap, brief_info, group);
    if (return_value < 0) {
        this_return_value = return_value;
        goto Tail;
    }

Tail:

    if (NULL != temp_group) {
        free(temp_group);
        temp_group = NULL;
        group = NULL;
    }

    AFS_TRACE_OUT_INFO("EXIT ext4Handler::getBitmap()");
    return this_return_value;
}

/**
 * @brief 真正实施获取bitmap的函数
 *
 * @param bitmap BitMap& 外部接口
 * @param fsbitmap BitMap& 内部处理
 * @param brief_info struct ext4_brief_info& 简短信息
 * @param group unsigned char* 组描述符信息
 * @return int32_t 成功返回0，失败返回错误码
 *
 */
int32_t ext4Handler::ext_getBitmap_realGet(vector<BitMap *> &bitmap_vect, BitMap &fsbitmap,
    struct ext4_brief_info &brief_info, unsigned char *group)
{
    AFS_TRACE_OUT_DBG("ENTER ext4Handler::ext_getBitmap_realGet()");

    int32_t return_value = 0;
    uint64_t total_num = 0; // 读取的组总数(最大值：ext4_brief_info定义为64位)
    uint64_t one_group_size = 0; // 一次读取的组描述符大小(单位：字节)(最大值：可能乘以灵活块组大小，而灵活块组可能64位)
    uint64_t one_bitmap_size = 0; // 一次读取的bitmap大小(单位：字节)(最大值：可能乘以灵活块组大小，而灵活块组可能64位)
    uint64_t index = 0;
    struct ext4_group_desc *ext4_group_des = NULL;
    uint64_t bitmap_addr = 0; // bitmap开始的块号(最大值：ext4中块地址为48位)
    uint64_t block_left = 0;  // 遗留的块数量(最大值：每组的块数(灵活块组可能很大))
    int64_t read_ret = 0;
    char *rbitmap = NULL; // 指示读入的bitmap存放的位置

    // bitmap初始化处理
    return_value = fsbitmap.initBitMap(brief_info.block_num);
    if (return_value < 0) {
        AFS_TRACE_OUT_ERROR("Fail to initBitMap!");
        return AFS_ERR_INNER;
    }
    fsbitmap.bitmapSetBlocksize(brief_info.block_size);
    rbitmap = fsbitmap.getbitmap();
    if (NULL == rbitmap) {
        AFS_TRACE_OUT_ERROR("Fail to get BitMap!");
        return AFS_ERR_INNER;
    }

    // 循环寻找所有bitmap时所需参数的原始值
    total_num = brief_info.group_total_num;
    one_group_size = brief_info.group_desc_size;
    one_bitmap_size = brief_info.need_bitmap_bytes;
    AFS_TRACE_OUT_DBG("one_bitmap_size is %llu, total ext4 Group number is %llu", one_bitmap_size, total_num);

    for (index = 0; index < total_num; index++) {
        // 读取组描述符，解析bitmap首块号
        // 由字符类型(为了便于移动到指定组描述符处)强制转换为组描述符结构体
        ext4_group_des = reinterpret_cast<struct ext4_group_desc *>(group);
        bitmap_addr = ext_getLongValue(brief_info.feature_incompat, brief_info.group_desc_size,
            ext4_group_des->bg_block_bitmap_hi, ext4_group_des->bg_block_bitmap_lo);
        AFS_TRACE_OUT_DBG("EXT4: the %llu th data block bitmap's addr is %llu (blocks id)", index, bitmap_addr);

        // 如果是最后一组，可能块数不满，单独处理
        if ((total_num - 1) == index) {
            block_left = brief_info.block_num - index * one_bitmap_size * BITS_PER_BYTE;

            AFS_TRACE_OUT_DBG("Before read, offset:%llu,count:%llu.",
                (int64_t)(bitmap_addr * brief_info.block_size),
                static_cast<int64_t>((block_left - 1) / BITS_PER_BYTE + 1));

            read_ret = m_reader->read(rbitmap, (int64_t)(bitmap_addr * brief_info.block_size),
                static_cast<int64_t>((block_left - 1) / BITS_PER_BYTE + 1), 0);
            if (read_ret != static_cast<int64_t>(((block_left - 1) / BITS_PER_BYTE + 1))) {
                AFS_TRACE_OUT_ERROR("Fail to read the bitmap!");
                return AFS_ERR_IMAGE_READ;
            }
        } else {
            AFS_TRACE_OUT_DBG("Before read, offset:%llu,count:%llu.",
                (int64_t)(bitmap_addr * brief_info.block_size),
                one_bitmap_size);

            read_ret = m_reader->read(rbitmap, (int64_t)(bitmap_addr * brief_info.block_size),
                static_cast<int64_t>(one_bitmap_size), 0);
            if (read_ret != static_cast<int64_t>(one_bitmap_size)) {
                AFS_TRACE_OUT_ERROR("Fail to read the bitmap!");
                return AFS_ERR_IMAGE_READ;
            }
            group += one_group_size;
            rbitmap += one_bitmap_size;
        }
    }

    return_value = ext_getBitmap_alterBitmap(bitmap_vect, fsbitmap, brief_info);
    if (return_value < 0) {
        return return_value;
    }

    AFS_TRACE_OUT_INFO("EXIT ext4Handler::ext_getBitmap_realGet()");
    return AFS_SUCCESS;
}

/**
 * @brief 调整获取的bitmap
 *
 * @param bitmap BitMap& 外部接口
 * @param fsbitmap BitMap& 内部处理
 * @param brief_info struct ext4_brief_info& 简短信息
 * @return int32_t 成功返回0，失败返回错误码
 *
 */
int32_t ext4Handler::ext_getBitmap_alterBitmap(vector<BitMap *> &bitmap_vect, BitMap &fsbitmap,
    struct ext4_brief_info &brief_info)
{
    AFS_TRACE_OUT_DBG("ENTER ext4Handler::ext_getBitmap_alterBitmap()");

    uint64_t bit_index = 0;
    unsigned char tc = 0;
    char *rbitmap = fsbitmap.getbitmap(); // 指示读入的bitmap存放的位置
    int64_t return_value = 0;

    if (NULL == rbitmap) {
        AFS_TRACE_OUT_ERROR("Fail to get BitMap.");
        return AFS_ERR_INNER;
    }
    // bitmap字节逆序
    for (bit_index = 0; bit_index < fsbitmap.getsize(); bit_index++) {
        tc = (unsigned char)(rbitmap[bit_index]);
        tc = (unsigned char)((tc & 0xaa) >> 1) | (unsigned char)((tc & 0x55) << 1);
        tc = (unsigned char)((tc & 0xcc) >> 2) | (unsigned char)((tc & 0x33) << 2);
        tc = (unsigned char)((tc & 0xf0) >> 4) | (unsigned char)((tc & 0x0f) << 4);
        rbitmap[bit_index] = (char)tc;
    }

    // 当块大小为1024时的特殊处理
    if (EXT4_MIN_BLOCK == brief_info.block_size) {
        // 整体右移一位并补齐每组首位
        for (bit_index = (fsbitmap.getsize() - 1); bit_index > 0; bit_index--) {
            rbitmap[bit_index] = (rbitmap[bit_index]) >> 1;
            if ((0 == (bit_index % EXT4_MIN_BLOCK)) || ((rbitmap[bit_index - 1]) & 0x01)) {
                rbitmap[bit_index] |= 0x80;
            }
        }
        rbitmap[bit_index] = (rbitmap[bit_index]) >> 1;
        rbitmap[bit_index] = (rbitmap[bit_index]) | 0x80;
    }

    return_value = fsbitmap.bitmapConvert(m_reader, SECTOR_SIZE, bitmap_vect);
    if (return_value < 0) {
        AFS_TRACE_OUT_ERROR("Fail to Convert bitmap!");
        return AFS_ERR_INNER;
    }

    AFS_TRACE_OUT_DBG("EXIT ext4Handler::ext_getBitmap_alterBitmap()");
    return AFS_SUCCESS;
}

/**
 * @brief 通过哈希树寻找索引节点号
 *
 * @param img_reader     读取数据的句柄
 * @param brief_info     分析文件系统必要的参数
 * @param bitmap_vect    文件数据在多磁盘上的位图
 * @param group          组描述符首地址
 * @param blocks         区段树所有数据块
 * @param current_dir    目标过滤目录
 * @param find_all       是否递归查找
 * @return int32_t       分析成功返回索引节点号(大于0)，分析失败返回-1
 *
 */
int32_t ext4Handler::ext_findInodeByDX(imgReader *img_reader, struct ext4_brief_info &brief_info,
    vector<BitMap *> &bitmap_vect, unsigned char *group, vector<uint64_t> &blocks, char *current_dir, bool find_all)
{
    AFS_TRACE_OUT_DBG("ENTER ext4Handler::ext_findInodeByDX()");

    if (blocks.empty()) {
        AFS_TRACE_OUT_ERROR("The blocks is empty");
        return AFS_ERR_INNER;
    }

    uint32_t block_size = brief_info.block_size;
    int32_t this_return_value = 0;
    int32_t read_ret = 0;
    uint32_t level = 0; // hash树层数
    vector<uint64_t> leaf_blocks;
    uint64_t real_leaf_block = 0;
    struct dx_root_2 *dxroot = NULL; // 根节点信息
    struct dx_node_2 *dnode = NULL;  // 中间节点信息
    struct dx_hash_info hinfo;       // 声明hinfo结构体用于处理目录名的hash信息

    // 获得根节点位置（块号）
    uint64_t dx_root_addr = blocks[0];

    // 读取hash树信息
    dxroot = static_cast<struct dx_root_2 *>(calloc(1, block_size));
    if (NULL == dxroot) {
        this_return_value = AFS_ERR_API;
        AFS_TRACE_OUT_ERROR("Fail to calloc the root of hash tree!");
        goto Tail;
    }

    AFS_TRACE_OUT_DBG("Before read, offset:%llu,count:%llu.", (int64_t)(dx_root_addr * block_size), block_size);
    read_ret = img_reader->read(dxroot, (int64_t)(dx_root_addr * block_size), static_cast<int64_t>(block_size), 0);
    if (read_ret != static_cast<int64_t>(block_size)) {
        this_return_value = AFS_ERR_IMAGE_READ;
        AFS_TRACE_OUT_ERROR("Fail to read the root of hash tree!");
        goto Tail;
    }

    this_return_value = ext_findInodeByDX_prepare(brief_info, blocks, current_dir, find_all, dxroot, hinfo, leaf_blocks,
        real_leaf_block);
    if (this_return_value < 0) {
        goto Tail;
    }

    level = dxroot->info.indirect_levels; // hash树层数
    // 根的结构体(包括后面接的目录项)与中间节点都占一块，可通过强制转换借用空间
    dnode = reinterpret_cast<struct dx_node_2 *>(dxroot);

    if (!find_all) {
        this_return_value =
            ext_findInodeByDX_realFind(block_size, blocks, hinfo, leaf_blocks, real_leaf_block, dnode, level);
    } else {
        this_return_value = ext_findInodeByDX_realFindAll(block_size, blocks, leaf_blocks, dnode, level);
    }

Tail:
    if (dxroot != NULL) {
        free(dxroot);
        dxroot = NULL;
    }
    if (this_return_value >= 0) {
        // 最后块内的目录项还得靠线性方式查找
        this_return_value =
            ext_findInodeByLinear(img_reader, brief_info, bitmap_vect, group, leaf_blocks, current_dir, find_all);
    }

    AFS_TRACE_OUT_DBG("EXIT ext4Handler::ext_findInodeByDX()");
    return this_return_value;
}

/**
 * @brief 通过哈希树寻找单个文件索引节点号
 *
 * @param block_size uint32_t& 块大小
 * @param blocks vector<int64_t>& 区段树所有数据块
 * @param hinfo struct struct dx_hash_info& 目标文件的哈希信息
 * @param leaf_blocks vector<uint64_t>& 保存数据的叶子数据块
 * @param real_leaf_block uint64_t& 哈希值最小的块号
 * @param dnode struct dx_node_2* 中间节点头结构体
 * @param level uint32_t& 哈希树深度
 * @return int32_t 分析成功返回0，分析失败返回错误码
 *
 */
int32_t ext4Handler::ext_findInodeByDX_realFind(uint32_t &block_size, vector<uint64_t> &blocks,
    struct dx_hash_info &hinfo, vector<uint64_t> &leaf_blocks, uint64_t &real_leaf_block, struct dx_node_2 *dnode,
    uint32_t &level)
{
    AFS_TRACE_OUT_DBG("ENTER ext4Handler::ext_findInodeByDX_realFind()");

    int32_t read_ret = 0;
    struct dx_entry *dentry = NULL; // 中间节点实体
    int64_t leaf_block = 0;
    le16 tmp_index = 0;

    // 已知要找的文件名，哈希寻找
    // 遍历htree中间节点
    while (level--) {
        AFS_TRACE_OUT_DBG("Before read, offset:%llu,count:%llu.", (int64_t)(real_leaf_block * block_size), block_size);

        read_ret = m_reader->read(dnode, (int64_t)(real_leaf_block * block_size), static_cast<int64_t>(block_size), 0);
        if (read_ret != static_cast<int64_t>(block_size)) {
            AFS_TRACE_OUT_ERROR("Fail to read the node of hash tree!");
            return AFS_ERR_IMAGE_READ;
        }
        // 跳过头结构体
        // 头结构体后全是哈希映射项，指针移动到相应位置可直接强制转换
        dentry = reinterpret_cast<struct dx_entry *>(dnode + 1);
        leaf_block = dnode->block; // hash值最小的块

        for (tmp_index = 0; tmp_index < (dnode->count - 1); tmp_index++) {
            if (hinfo.hash < dentry[tmp_index].hash) {
                break;
            }
            leaf_block = dentry[tmp_index].block;
        }
        real_leaf_block = blocks[leaf_block];
    }

    leaf_blocks.clear();
    leaf_blocks.push_back(real_leaf_block);

    AFS_TRACE_OUT_DBG("EXIT ext4Handler::ext_findInodeByDX_realFind()");
    return AFS_SUCCESS;
}

/**
 * @brief 通过哈希树递归寻找文件夹下所有数据块
 *
 * @param block_size uint32_t& 块大小
 * @param blocks vector<int64_t>& 区段树所有数据块
 * @param leaf_blocks vector<uint64_t>& 保存数据的叶子数据块
 * @param dnode struct dx_node_2* 中间节点头结构体
 * @param level uint32_t& 哈希树深度
 * @return int32_t 分析成功返回0，分析失败返回错误码
 *
 */
int32_t ext4Handler::ext_findInodeByDX_realFindAll(uint32_t &block_size, vector<uint64_t> &blocks,
    vector<uint64_t> &leaf_blocks, struct dx_node_2 *dnode, uint32_t &level)
{
    AFS_TRACE_OUT_DBG("ENTER ext4Handler::ext_findInodeByDX_realFindAll()");

    int64_t read_ret = 0;
    vector<uint64_t> temp_blocks;
    struct dx_entry *dentry = NULL; // 中间节点实体
    le16 tmp_index = 0;

    // 循环遍历哈希树，找到所有叶子块
    while (level--) {
        // temp接过这一层所有块，leaf准备记录下一层所有块
        temp_blocks.assign(leaf_blocks.begin(), leaf_blocks.end());
        leaf_blocks.clear();

        // 循环遍历这一层所有块
        while (temp_blocks.size() > 0) {
            AFS_TRACE_OUT_DBG("Before read, offset:%llu,count:%llu.", (int64_t)(blocks[temp_blocks[0]] * block_size), block_size);

            read_ret = m_reader->read(dnode, (int64_t)(blocks[temp_blocks[0]] * block_size),
                static_cast<int64_t>(block_size), 0);
            if (read_ret != static_cast<int64_t>(block_size)) {
                AFS_TRACE_OUT_ERROR("Fail to read the node of hash tree!");
                return AFS_ERR_IMAGE_READ;
            }

            // hash值最小的块并不在哈希映射项中，应提前单独入栈
            leaf_blocks.push_back(blocks[dnode->block]);
            // 跳过头结构体
            // 头结构体后全是哈希映射项，指针移动到相应位置可直接强制转换
            dentry = reinterpret_cast<struct dx_entry *>(dnode + 1);

            // 记录这一块指向的下一层所有块
            for (tmp_index = 0; tmp_index < (dnode->count - 1); tmp_index++) {
                leaf_blocks.push_back(blocks[dentry[tmp_index].block]);
            }
            temp_blocks.erase(temp_blocks.begin());
        }
    }

    AFS_TRACE_OUT_DBG("EXIT ext4Handler::ext_findInodeByDX_realFindAll()");
    return AFS_SUCCESS;
}

/**
 * @brief 为哈希树分析做准备
 *
 * @param brief_info struct ext4_brief_info* 分析文件系统必要的参数
 * @param blocks vector<int64_t>& 区段树所有数据块
 * @param current_dir char* 目标过滤目录
 * @param find_all bool 是否递归查找
 * @param dxroot struct dx_root_2* 哈希树根节点头结构体
 * @param hinfo struct struct dx_hash_info& 目标文件的哈希信息
 * @param leaf_blocks vector<uint64_t>& 保存数据的叶子数据块
 * @param real_leaf_block uint64_t& 哈希值最小的块号
 * @return int32_t 分析成功返回0，分析失败返回错误码
 *
 */
int32_t ext4Handler::ext_findInodeByDX_prepare(struct ext4_brief_info &brief_info, vector<uint64_t> &blocks,
    char *current_dir, bool find_all, struct dx_root_2 *dxroot, struct dx_hash_info &hinfo,
    vector<uint64_t> &leaf_blocks, uint64_t &real_leaf_block)
{
    AFS_TRACE_OUT_DBG("ENTER ext4Handler::ext_findInodeByDX_prepare()");

    le16 tmp_index = 0;
    struct dx_entry *dxentry = NULL; // 根节点实体
    size_t leaf_block = 0;
    int32_t return_value = 0;

    dxentry = reinterpret_cast<struct dx_entry *>(dxroot + 1);

    // 递归过滤目录时，底下的循环并不会遍历到hash值为0的块，故hash值为0的块必须先入栈
    leaf_block = dxroot->block; // 取出hash值为0的块号
    if (leaf_block > blocks.size()) {
        AFS_TRACE_OUT_ERROR("Fail to get blocks space!");
        return AFS_ERR_API;
    }

    leaf_blocks.push_back(blocks[leaf_block]); // 将其对应的真正块号入栈

    // 如果不是递归寻找所有文件，才需要计算目标文件的哈希值以备哈希查找
    if (!find_all) {
        hinfo.hash_version = dxroot->info.hash_version; // hash版本信息
        hinfo.seed = brief_info.hash_seed;              // hash种子值，来自superblock
        // 计算目录的hash值，函数来自内核
        return_value = ext_calcDirHashValue(current_dir, strlen(current_dir), &hinfo);
        if (return_value < 0) {
            AFS_TRACE_OUT_ERROR("Fail to calc Dir Hash Value!");
            return AFS_ERR_API;
        }
    }

    // 根节点指向的块
    // count计算了头结构体，故该减一才是哈希映射项的数目
    for (tmp_index = 0; tmp_index < (dxroot->count - 1); tmp_index++) {
        // 如果不是递归寻找所有文件，找到文件哈希值处就可以停止了
        if (!find_all && hinfo.hash < dxentry[tmp_index].hash) {
            break;
        }
        leaf_block = dxentry[tmp_index].block;
        leaf_blocks.push_back(blocks[leaf_block]);
    }

    real_leaf_block = blocks[leaf_block];

    AFS_TRACE_OUT_DBG("EXIT ext4Handler::ext_findInodeByDX_prepare()");
    return AFS_SUCCESS;
}

/**
 * @brief ext3间接块映射的递归函数
 *
 * @param img_reader imgReader* 读取数据的句柄
 * @param temp_block_size uint32_t 块大小
 * @param blocks vector<int64_t>& 间接块映射所有数据块
 * @param block_no uint64_t 当前需要分析的块号
 * @param depth int32_t 当前块处于几级映射
 * @return 继续递归则返回成功，否则返回停止标志
 *
 */
int32_t ext4Handler::ext_blockMap(imgReader *img_reader, uint32_t temp_block_size, vector<uint64_t> &blocks,
    uint64_t block_no, int32_t depth)
{
    AFS_TRACE_OUT_DBG("ENTER ext4Handler::ext_blockMap()");

    if (0 == block_no) {
        return THE_BLOCK_MAP_END_FLAG;
    }
    if (0 == depth) {
        blocks.push_back(block_no);
        return AFS_SUCCESS;
    }

    le32 *block_map = NULL;
    uint32_t tmp_index = 0;
    int32_t read_ret = 0;
    int32_t return_value = 0;
    uint64_t block_size = temp_block_size;
    uint32_t block_num = block_size / 4;

    block_map = static_cast<le32 *>(calloc(1, block_size));
    if (NULL == block_map) {
        AFS_TRACE_OUT_ERROR("Fail to calloc the block_map!");
        return_value = AFS_ERR_API;
        goto Tail;
    }
    AFS_TRACE_OUT_DBG("Before read, offset:%llu,count:%llu.", block_no * block_size, block_size);
    read_ret = img_reader->read(block_map, (int64_t)(block_no * block_size), static_cast<int64_t>(block_size), 0);
    if (read_ret != static_cast<int64_t>(block_size)) {
        AFS_TRACE_OUT_ERROR("Fail to read block_map,the depth is %d", depth);
        return_value = AFS_ERR_IMAGE_READ;
        goto Tail;
    }
    for (tmp_index = 0; tmp_index < block_num; tmp_index++) {
        return_value = ext_blockMap(img_reader, temp_block_size, blocks, block_map[tmp_index], depth - 1);
        if (return_value < 0) {
            goto Tail;
        }
    }

Tail:
    if (block_map != NULL) {
        free(block_map);
        block_map = NULL;
    }
    AFS_TRACE_OUT_INFO("EXIT ext4Handler::ext_blockMap()");
    return return_value;
}

/**
 * @brief 以ext3间接块映射方式获取所有数据块
 *
 * @param img_reader imgReader* 读取数据的句柄
 * @param inode struct ext4_inode * 索引节点信息
 * @param temp_block_size uint32_t 块大小
 * @param blocks vector<int64_t>& 间接块映射所有数据块
 * @return 成功返回0，失败返回负数
 *
 */
int32_t ext4Handler::ext_getLeafInode(imgReader *img_reader, struct ext4_inode *inode, uint32_t temp_block_size,
    vector<uint64_t> &blocks)
{
    AFS_TRACE_OUT_DBG("ENTER ext4Handler::ext_getLeafInode()");

    le32 block_map[I_BLOCK_SIZE];
    uint8_t tmp_index = 0;
    int32_t return_value = 0;
    uint32_t index_map_depth[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3};
    // i_block为60字节，可能存储着区段树根节点、符号链接等
    CHECK_MEMCPY_S_OK(block_map, I_BLOCK_SIZE, inode->i_block, I_BLOCK_SIZE);

    for (tmp_index = 0; tmp_index < 15; tmp_index++) {
        return_value =
            ext_blockMap(img_reader, temp_block_size, blocks, block_map[tmp_index], index_map_depth[tmp_index]);
        if (THE_BLOCK_MAP_END_FLAG == return_value) {
            return AFS_SUCCESS;
        }
        if (return_value < 0) {
            return return_value;
        }
    }

    AFS_TRACE_OUT_DBG("EXIT ext4Handler::ext_getLeafInode()");
    return return_value;
}

/**
 * @brief 获取区段树所有叶子节点
 *
 * @param img_reader imgReader* 读取数据的句柄
 * @param inode struct ext4_inode* 保存区段树根节点的索引节点信息
 * @param block_size int 块大小
 * @param &blocks vector<int64_t> 存储区段树所有叶子块
 * @return int32_t 获取成功返回0，获取失败返回-1
 *
 */
int32_t ext4Handler::ext_getExtentLeafInode(imgReader *img_reader, struct ext4_inode *inode, uint32_t block_size,
    vector<uint64_t> &blocks)
{
    AFS_TRACE_OUT_DBG("ENTER ext4Handler::ext_getExtentLeafInode()");

    uint64_t extent_no = 0;
    size_t read_ret = 0;
    int32_t this_return_value = 0;
    struct ext4_extent_header *extent_header = NULL; // 头结构体

    if (0 == block_size) {
        AFS_TRACE_OUT_ERROR("Invalid block size. size is zero.");
        return AFS_ERR_INNER;
    }

    extent_header = static_cast<struct ext4_extent_header *>(calloc(1, block_size));
    if (NULL == extent_header) {
        this_return_value = AFS_ERR_API;
        AFS_TRACE_OUT_ERROR("Fail to calloc the buf to store extent_header!");
        goto Tail;
    }
    // i_block为60字节，可能存储着区段树根节点、符号链接等
    CHECK_MEMCPY_S_OK(extent_header, block_size, inode->i_block, I_BLOCK_SIZE);
    ext_getExtentInode_MidOrLeaf(extent_header, blocks);
    while (!extent_tree.empty()) {
        extent_no = extent_tree.front();
        extent_tree.pop();
        // 直接重复使用extent_header这块空间，避免多次申请与逻辑上的内存泄露
        AFS_TRACE_OUT_DBG("Before read, offset:%llu,count:%llu.", extent_no * block_size, block_size);

        read_ret = img_reader->read(extent_header, static_cast<int64_t>(extent_no * block_size),
            static_cast<int64_t>(block_size), 0);
        if (read_ret != static_cast<int64_t>(block_size)) {
            // 一错全错，不能因为递归查找子目录就不return
            this_return_value = AFS_ERR_IMAGE_READ;
            AFS_TRACE_OUT_ERROR("Fail to read Extent block!");
            goto Tail;
        }
        ext_getExtentInode_MidOrLeaf(extent_header, blocks);
    }

Tail:
    if (NULL != extent_header) {
        free(extent_header);
        extent_header = NULL;
    }
    AFS_TRACE_OUT_DBG("EXIT ext4Handler::ext_getExtentLeafInode()");
    return this_return_value;
}

/**
 * @brief 对中间节点或叶子节点操作，获取相应信息
 *
 * @param extent_header struct ext4_extent_header* 用于临时保存一个数据块的信息
 * @param blocks vector<uint64_t>& 保存区段树指向的所有数据块号
 * @return void 无需返回值，获取的信息已保存在传入参数与全局变量中
 *
 */
void ext4Handler::ext_getExtentInode_MidOrLeaf(struct ext4_extent_header *extent_header, vector<uint64_t> &blocks)
{
    AFS_TRACE_OUT_DBG("ENTER ext4Handler::ext_getExtentInode_MidOrLeaf()");

    uint64_t block_no = 0;
    int tmp_index = 0;
    int ee_index = 0;
    struct ext4_extent_idx *extent_idx = NULL;
    struct ext4_extent *extent_le = NULL;
    le16 ee_len = 0;

    for (tmp_index = 0; tmp_index < extent_header->eh_entries; tmp_index++) {
        // 中间节点
        if (extent_header->eh_depth != 0) {
            // 依次取得 保存中间节点信息的区段结构体 3种结构体大小相等
            // 将头结构体强制转换成中间节点结构体
            extent_idx = reinterpret_cast<struct ext4_extent_idx *>(extent_header + tmp_index + 1);
            // 下一节点的块号
            block_no =
                (static_cast<uint64_t>(extent_idx->ei_leaf_hi) << 32) | (static_cast<uint64_t>(extent_idx->ei_leaf_lo));
            // 现在保存节点块号来继续bfs，而不再每次申请空间
            extent_tree.push(block_no);
        }
        // 叶子节点
        // 既然只保存了块号，这块空间的信息在函数返回后就会被覆盖，因此其中信息应全部分析出来，保存到blocks
        else {
            // 依次取得 保存叶子节点信息的区段结构体 3种结构体大小相等
            // 将头结构体强制转换成叶子节点结构体
            extent_le = reinterpret_cast<struct ext4_extent *>(extent_header + tmp_index + 1);

            // 计算该叶子节点指向的初始数据块的块号
            block_no = (static_cast<int64_t>(extent_le->ee_start_hi) << 32) | extent_le->ee_start_lo;
            // 该区段覆盖的块数
            ee_len = extent_le->ee_len;
            if (extent_le->ee_len > static_cast<le16>(MAX_EE_LEN)) {
                ee_len = extent_le->ee_len - MAX_EE_LEN;
            }
            // else无需处理，否则即覆盖的块数为初值

            // 保存该叶子节点指向的所有数据块的块号
            for (ee_index = 0; ee_index < ee_len; ee_index++) {
                blocks.push_back(block_no + ee_index);
            }
        }
    }

    AFS_TRACE_OUT_DBG("EXIT ext4Handler::ext_getExtentInode_MidOrLeaf()");
    return;
}

/**
 * @brief 解析线性目录项时，获取所有目录项信息
 *
 * @param *img_reader         读取数据的句柄
 * @param &brief_info         分析文件系统必要的参数
 * @param *group              组描述符首地址
 * @param *inode              保存区段树根节点的索引节点信息
 * @param *dir_entry          目录项信息
 * @param *valid_dir          目标字符串
 * @param &bitmap_vect        文件的数据在多磁盘上的位图
 * @return int32_t 中途获取失败报错误log，无返回值（因为递归过滤，错误也得继续）
 *
 */
void ext4Handler::ext_find_all(imgReader *img_reader, struct ext4_brief_info &brief_info, unsigned char *group,
    struct ext4_inode *inode, struct ext4_dir_entry_2 *dir_entry, char *valid_dir, vector<BitMap *> &bitmap_vect)
{
    int32_t return_value = 0;

    if ((strlen(valid_dir) != 0) && strcmp(valid_dir, ".") && strcmp(valid_dir, "..")) {
        if (dir_entry->inode <= 0) {
            AFS_TRACE_OUT_ERROR("This File may be delete : %d,the name is \"%s\"", dir_entry->inode, valid_dir);
            return;
        }
        // 通过索引节点号取出对应的数据
        return_value = ext_getInode(img_reader, brief_info, group, inode, dir_entry->inode);
        if (return_value < 0) {
            AFS_TRACE_OUT_ERROR("Fail to get inode info by inode no:%d", dir_entry->inode);
            return;
        }
        // 通过索引节点读取文件数据
        return_value = ext_findFileByInode(img_reader, brief_info, bitmap_vect, group, inode);
        if (return_value < 0) {
            AFS_TRACE_OUT_ERROR("Fail to get file by inode no : %d,the name is \"%s\"", dir_entry->inode, valid_dir);
            return;
        }
    }
}

/**
 * @brief 通过线性结构遍历目录项寻找索引节点号
 *
 * @param img_reader     读取数据的句柄
 * @param brief_info     分析文件系统必要的参数
 * @param bitmap_vect    文件数据在多磁盘上的位图
 * @param group          组描述符首地址
 * @param blocks         有目录项的所有数据块
 * @param current_dir    目标过滤目录
 * @param find_all       是否递归查找
 * @return int32_t 分析成功返回索引节点号(大于0)，分析失败返回-1
 *
 */
int32_t ext4Handler::ext_findInodeByLinear(imgReader *img_reader, struct ext4_brief_info &brief_info,
    vector<BitMap *> &bitmap_vect, unsigned char *group, vector<uint64_t> &blocks, char *current_dir, bool find_all)
{
    AFS_TRACE_OUT_DBG("ENTER ext4Handler::ext_findInodeByLinear()");

    int32_t this_return_value = 0;
    struct ext4_inode *inode = NULL;
    unsigned char *block_buffer = NULL;
    char *valid_dir = NULL;

    block_buffer = static_cast<unsigned char *>(calloc(1, brief_info.block_size));
    if (NULL == block_buffer) {
        this_return_value = AFS_ERR_API;
        AFS_TRACE_OUT_ERROR("Fail to calloc the buf to store Extent block!");
        goto Tail;
    }

    valid_dir = static_cast<char *>(calloc(1, EXT4_NAME_LEN));
    if (NULL == valid_dir) {
        this_return_value = AFS_ERR_API;
        AFS_TRACE_OUT_ERROR("Fail to calloc the valid_dir to store dir name!");
        goto Tail;
    }

    inode = static_cast<struct ext4_inode *>(calloc(1, sizeof(struct ext4_inode)));
    if (NULL == inode) {
        this_return_value = AFS_ERR_API;
        AFS_TRACE_OUT_ERROR("Fail to calloc the inode!");
        goto Tail;
    }

    this_return_value = ext_findInodeByLinear_realGet(img_reader, brief_info, bitmap_vect, group, blocks, current_dir,
        find_all, inode, block_buffer, valid_dir, EXT4_NAME_LEN);

Tail:
    if (block_buffer != NULL) {
        free(block_buffer);
        block_buffer = NULL;
    }
    if (valid_dir != NULL) {
        free(valid_dir);
        valid_dir = NULL;
    }
    if (inode != NULL) {
        free(inode);
        inode = NULL;
    }
    AFS_TRACE_OUT_DBG("EXIT ext4Handler::ext_findInodeByLinear()");
    return this_return_value;
}

/**
 * @brief 通过线性结构遍历目录项的真正实现部分
 *
 * @param img_reader    imgReader*                读取数据的句柄
 * @param brief_info    struct ext4_brief_info*   分析文件系统必要的参数
 * @param bitmap_vect   vector<BitMap *> &        文件数据在多磁盘上的位图
 * @param group         unsigned char*            组描述符首地址
 * @param blocks        vector<int64_t>&          有目录项的所有数据块
 * @param current_dir   char*                     目标过滤目录
 * @param find_all      bool                      是否递归查找
 * @param inode         struct ext4_inode*        存储索引节点信息
 * @param buf           unsigned char *           存储目录项遍历指针
 * @param valid_dir char * 目标字符串
 *
 * @return int32_t 分析成功返回索引节点号(大于0)，分析失败返回-1
 *
 */
int32_t ext4Handler::ext_findInodeByLinear_realGet(imgReader *img_reader, struct ext4_brief_info &brief_info,
    vector<BitMap *> &bitmap_vect, unsigned char *group, vector<uint64_t> &blocks, char *current_dir, bool find_all,
    struct ext4_inode *inode, unsigned char *buf, char *valid_dir, size_t valid_dir_len)
{
    AFS_TRACE_OUT_DBG("ENTER ext4Handler::ext_findInodeByLinear_realGet()");

    uint32_t tmp_index = 0;
    uint32_t blocks_len = 0;
    uint32_t sum_len = 0;
    int32_t read_ret = 0;
    struct ext4_dir_entry_2 *dir_entry = NULL;
    char *return_char = NULL;
    unsigned char *temp_buf = NULL; // 不新建临时指针，移动了buf的话，Linux下编译运行会崩溃
    struct ext4_inode *tmp_inode = inode;
    char *tmp_valid_dir = valid_dir;
    unsigned char *buf_read = buf;

    blocks_len = blocks.size();
    // 循环遍历每一块
    for (tmp_index = 0; tmp_index < blocks_len; tmp_index++) {
        AFS_TRACE_OUT_DBG("Before read, offset:%llu,count:%llu.",
        (int64_t)((uint64_t)(blocks[tmp_index]) * (brief_info.block_size)), brief_info.block_size);

        read_ret = img_reader->read(buf_read, (int64_t)((uint64_t)(blocks[tmp_index]) * (brief_info.block_size)),
            (int64_t)(brief_info.block_size), 0);
        if (read_ret != static_cast<int64_t>(brief_info.block_size)) {
            AFS_TRACE_OUT_ERROR("Fail to read linear block!");
            return AFS_ERR_IMAGE_READ;
        }

        temp_buf = buf_read;

        // 循环遍历某块中的每一个目录项（目录项不定长，长度保存在dir_entry->rec_len中，单位：字节）
        // sum_len < block_size，必须是小于号，不能等于，最后一个目录项的长度会填满block_size
        for (sum_len = 0; sum_len < brief_info.block_size; (sum_len += dir_entry->rec_len)) {
            // 本应判断is_filetype（目录项是否存储了文件类型），再决定使用哪种结构体，但不影响遍历，故全部采用新结构体
            // 临时指针移动到确定位置后，直接强制转换成目录项结构体，原指针未移动
            dir_entry = reinterpret_cast<struct ext4_dir_entry_2 *>(temp_buf);
            return_char = ext_strnCopy(tmp_valid_dir, valid_dir_len, dir_entry->name, dir_entry->name_len);
            if (NULL == return_char) {
                AFS_TRACE_OUT_ERROR("Fail to strnCopy the dir_entry->name!");
                return AFS_ERR_API;
            }

            if (find_all) {
                // 递归查找所有文件，分析某一文件出错时不报错
                ext_find_all(img_reader, brief_info, group, tmp_inode, dir_entry, tmp_valid_dir, bitmap_vect);
            }
            // 如果与文件名匹配，则找到索引节点号，退出循环
            else if (0 == strcmp(tmp_valid_dir, current_dir)) {
                return dir_entry->inode;
            }
            // else无需处理，文件名不匹配 或者 希望遍历目录项 则继续遍历下一个目录项

            temp_buf += dir_entry->rec_len;
        }
    }

    AFS_TRACE_OUT_DBG("EXIT ext4Handler::ext_findInodeByLinear_realGet()");
    return AFS_SUCCESS;
}

/**
 * @brief 通过目录找到索引节点号
 *
 * @param img_reader      读取数据的句柄
 * @param brief_info      分析文件系统必要的参数
 * @param bitmap_vect     文件数据在多磁盘上的位图
 * @param group           组描述符首地址
 * @param inode           上一层目录的索引节点信息
 * @param current_dir     目标过滤目录
 * @param find_all        是否递归查找
 *
 * @return int32_t 分析成功返回索引节点号(大于0)，分析失败返回-1
 *
 */
int32_t ext4Handler::ext_findInodeByDir(imgReader *img_reader, struct ext4_brief_info &brief_info,
    vector<BitMap *> &bitmap_vect, unsigned char *group, struct ext4_inode *inode, char *current_dir, bool find_all)
{
    AFS_TRACE_OUT_DBG("ENTER ext4Handler::ext_findInodeByDir()");

    int32_t return_value = 0;
    vector<uint64_t> blocks;

    if (inode->i_flags & static_cast<le32>(EXT4_EXTENTS_FL)) {
        // 区段树存储
        return_value = ext_getExtentLeafInode(img_reader, inode, brief_info.block_size, blocks);
    } else {
        // 间接映射块存储
        return_value = ext_getLeafInode(img_reader, inode, brief_info.block_size, blocks);
    }

    if (return_value < 0) {
        AFS_TRACE_OUT_ERROR("Fail to get leaf inode!");
        return return_value;
    }

    if (blocks.size() == 0) {
        AFS_TRACE_OUT_ERROR("Fail to get leaf inode!The sum of blocks is zero!");
        return AFS_ERR_INNER;
    }

    // 如果目录以哈希树结构存储，则调用函数先找到目录项所在块
    if (inode->i_flags & static_cast<le32>(EXT4_INDEX_FL)) {
        return_value = ext_findInodeByDX(img_reader, brief_info, bitmap_vect, group, blocks, current_dir, find_all);
        if (return_value < 0) {
            AFS_TRACE_OUT_ERROR("Fail to find inode by hash tree!");
            return return_value;
        }
    } else {
        // 否则目录即线性存储，则调用函数遍历每个块
        return_value = ext_findInodeByLinear(img_reader, brief_info, bitmap_vect, group, blocks, current_dir, find_all);
        if (return_value < 0) {
            AFS_TRACE_OUT_ERROR("Fail to find inode by linear!");
            return return_value;
        }
    }

    AFS_TRACE_OUT_DBG("EXIT ext4Handler::ext_findInodeByDir()");
    return return_value;
}

/**
 * @brief 通过哈希树寻找索引节点号
 *
 * @param img_reader   imgReader*               读取数据的句柄
 * @param brief_info   struct ext4_brief_info*  分析文件系统必要的参数
 * @param bitmap_vect  vector<BitMap *>&        文件数据在多磁盘上的位图
 * @param group        unsigned char*           组描述符首地址
 * @param inode        struct ext4_inode*       文件索引节点信息
 *
 * @return int32_t 分析成功返回0，分析失败返回-1
 *
 */
int32_t ext4Handler::ext_findFileByInode(imgReader *img_reader, struct ext4_brief_info &brief_info,
    vector<BitMap *> &bitmap_vect, unsigned char *group, struct ext4_inode *inode)
{
    AFS_TRACE_OUT_DBG("ENTER ext4Handler::ext_findFileByInode()");

    int32_t return_value = 0;

    if (0 == brief_info.block_size) {
        AFS_TRACE_OUT_ERROR("Invalid block size. brief_info.block_size is zero.");
        return AFS_ERR_INNER;
    }

    if ((inode->i_mode & static_cast<le16>(0xf000)) == static_cast<le16>(S_IFDIR_AFS)) {
        return_value = ext_findInodeByDir(img_reader, brief_info, bitmap_vect, group, inode, NULL, true);
        if (return_value < 0) {
            AFS_TRACE_OUT_ERROR("Fail to find inode by dir!");
            return return_value;
        }
    } else if ((inode->i_mode & static_cast<le16>(0xf000)) == static_cast<le16>(S_IFLINK_AFS)) {
        AFS_TRACE_OUT_ERROR(
            "The file is symbolic link.The link target is : %s", reinterpret_cast<char *>(inode->i_block));
        return AFS_ERR_FILE_SYMLINKS;
    } else {  // 获取所有的叶子节点
        return ext_findFileByInode_writeBitmap(img_reader, brief_info, bitmap_vect, group, inode);
    }

    AFS_TRACE_OUT_DBG("EXIT ext4Handler::ext_findFileByInode()");
    return AFS_SUCCESS;
}

/**
 * @brief 通过哈希树寻找索引节点号，真正写bitmap的部分
 *
 * @param img_reader   imgReader*                读取数据的句柄
 * @param brief_info   struct ext4_brief_info*   分析文件系统必要的参数
 * @param bitmap_vect  vector<BitMap *> &        文件数据在多磁盘上的位图
 * @param group        unsigned char*            组描述符首地址
 * @param inode        struct ext4_inode*        文件索引节点信息
 *
 * @return int32_t 写成功返回0，写失败返回-1
 *
 */
int32_t ext4Handler::ext_findFileByInode_writeBitmap(imgReader *img_reader, struct ext4_brief_info &brief_info,
    vector<BitMap *> &bitmap_vect, unsigned char *group, struct ext4_inode *inode)
{
    AFS_TRACE_OUT_DBG("ENTER ext4Handler::ext_findFileByInode_writeBitmap");

    int32_t return_value = 0;
    vector<uint64_t> blocks;
    uint32_t blocks_len = 0;
    uint32_t block_size = brief_info.block_size;

    if (inode->i_flags & static_cast<le32>(EXT4_EXTENTS_FL)) {
        // 区段树处理
        return_value = ext_getExtentLeafInode(img_reader, inode, brief_info.block_size, blocks);
    } else {
        // 间接映射块处理
        return_value = ext_getLeafInode(img_reader, inode, brief_info.block_size, blocks);
    }

    if (return_value < 0) {
        AFS_TRACE_OUT_ERROR("Fail to get leaf inode!");
        return return_value;
    }

    blocks_len = blocks.size();
    if (0 == blocks_len) {
        AFS_TRACE_OUT_ERROR("Fail to get leaf inode!The sum of blocks is zero!");
        return AFS_ERR_INNER;
    }

    return_value = ext_setFilterBitmap(img_reader, block_size, blocks, bitmap_vect);
    if (return_value != AFS_SUCCESS) {
        return return_value;
    }

    AFS_TRACE_OUT_INFO("EXIT ext4Handler::ext_findFileByInode_writeBitmap");
    return AFS_SUCCESS;
}

/**
 * @brief 根据文件的数据块绝对号（blocks保存）标注Bitmap位置
 *
 * @param img_reader    读取数据的句柄
 * @param block_size    文件系统块大小
 * @param blocks        需要过滤的文件块数组
 * @param bitmap_vect   文件在每个磁盘上的bitmap
 *
 * @return int32_t 成功返回0，失败返回-1
 *
 */
int32_t ext4Handler::ext_setFilterBitmap(imgReader *img_reader, uint32_t block_size, vector<uint64_t> &blocks,
    vector<BitMap *> &bitmap_vect)
{
    int32_t ret = 0;
    uint32_t tmp_index = 0;
    uint32_t blocks_len = blocks.size();
    uint64_t start_block = 0;
    uint64_t cmp_block = 0;
    uint64_t tmp_length = 0;
    uint64_t total_size = 0;
    BitMap tempBitmap; // /临时bitmap,主要为了调用bitmapSetRangeMapAddr(),需设置m_blocksize为512字节
    tempBitmap.bitmapSetBlocksize(SECTOR_SIZE);

    AFS_TRACE_OUT_DBG("the file of xxx data blocks length is %d", blocks_len);
    // 如果块号连续则单词处理完整个offset和length以减少LVM映射次数
    for (tmp_index = 0; tmp_index < blocks_len; tmp_index++) {
        cmp_block = blocks[tmp_index];
        AFS_TRACE_OUT_DBG("the file of xxx %uth data blocks address is %llu(blocks: %u bytes)", tmp_index, cmp_block,
            block_size);

        // 块号不连续时重新组排
        if (start_block == 0) {
            start_block = cmp_block;
        }

        // 将连续的块号统一计算（LVM时的性能问题）
        if ((tmp_index + 1) == blocks_len) {
            tmp_length += 1; // 最后一个块停止continue
        } else {
            tmp_length += 1;
            if (cmp_block == (blocks[tmp_index + 1] - 1)) {
                continue;
            }
        }

        // 开始设置连续块的Bitmap
        AFS_TRACE_OUT_DBG("start block id is %llu, length is %llu", start_block, tmp_length);
        ret = tempBitmap.bitmapSetRangeMapAddr(img_reader, (uint64_t)((start_block * block_size) >> SECTOR_SIZE_BITS),
            (uint64_t)((block_size >> SECTOR_SIZE_BITS) * tmp_length), bitmap_vect, 1);
        if (ret != AFS_SUCCESS) {
            AFS_TRACE_OUT_ERROR("Fail to set bitmap. ret = %d", ret);
            return AFS_ERR_INNER;
        }

        total_size += (((block_size >> SECTOR_SIZE_BITS) * tmp_length) / 2);

        // 初始化起始信息
        start_block = 0;
        tmp_length = 0;
    }
    AFS_TRACE_OUT_DBG("Success to filter file data. size=%lld(KB)", (long long)total_size);
    return ret;
}

/**
 * @brief 分析超级块的数据，简要存储有效数据
 *
 * @param img_reader imgReader* 用于读取镜像文件的reader
 * @param brief_info struct ext4_brief_info* 超级块中需要使用的数据
 *
 * @return int 0分析成功 -1分析失败
 *
 */
int32_t ext4Handler::ext_analyzeSuperBlock(imgReader *img_reader, struct ext4_brief_info &brief_info)
{
    AFS_TRACE_OUT_DBG("ENTER ext4Handler::ext_analyzeSuperBlock()");

    int32_t this_return_value = 0;
    int64_t read_ret = 0;

    // 申请superblock内存空间
    struct ext4_super_block *ext4_super_bk =
        static_cast<struct ext4_super_block *>(calloc(1, sizeof(struct ext4_super_block)));
    if (NULL == ext4_super_bk) {
        AFS_TRACE_OUT_ERROR("Fail to calloc ext4_super_bk!");
        return AFS_ERR_API;
    }
    // 读取superblock
    AFS_TRACE_OUT_DBG("Before read, offset:%llu,count:%llu.", SUPER_BLOCK_PADDING, sizeof(struct ext4_super_block));
    int64_t stSize = sizeof(ext4_super_block);
    bool result = ReadBySectorsBuff(
        img_reader, reinterpret_cast<void *>(ext4_super_bk), SUPER_BLOCK_PADDING, stSize, 1);
    if (!result) {
        AFS_TRACE_OUT_ERROR("Failed to read ext4_super_bk data.");
        free(ext4_super_bk);
        return AFS_ERR_IMAGE_READ;
    }

    // 处理superblock信息
    CHECK_MEMCPY_S_OK(brief_info.last_mounted, sizeof(ext4_super_bk->s_last_mounted), ext4_super_bk->s_last_mounted,
        sizeof(ext4_super_bk->s_last_mounted));
    brief_info.is_filetype = ext4_super_bk->s_feature_incompat & EXT4_FEATURE_INCOMPAT_FILETYPE;
    brief_info.inode_size = ext4_super_bk->s_inode_size;
    brief_info.inodes_per_group = ext4_super_bk->s_inodes_per_group;
    brief_info.feature_incompat = ext4_super_bk->s_feature_incompat;
    CHECK_MEMCPY_S_OK(brief_info.hash_seed, sizeof(ext4_super_bk->s_hash_seed), ext4_super_bk->s_hash_seed,
        sizeof(ext4_super_bk->s_hash_seed));

    brief_info.group_desc_size = ext4_super_bk->s_desc_size > 32 ? 64 : 32;

    brief_info.block_num = ext_getLongValue(brief_info.feature_incompat, brief_info.group_desc_size,
        ext4_super_bk->s_blocks_count_hi, ext4_super_bk->s_blocks_count_lo);
    brief_info.need_bitmap_bytes = ext4_super_bk->s_blocks_per_group / BITS_PER_BYTE;

    brief_info.block_size = 1 << (BASE_BLOCK_SIZE_BITS + ext4_super_bk->s_log_block_size);
    brief_info.group_total_num = (brief_info.block_num - 1) / ext4_super_bk->s_blocks_per_group + 1;

    // 有效性检查
    if ((0 == brief_info.block_num) || (0 == brief_info.block_size) || (0 == brief_info.group_desc_size) ||
        (0 == brief_info.group_total_num)) {
        AFS_TRACE_OUT_ERROR("Fail to read ext4_super_bk!  Invalid size.");
        this_return_value = AFS_ERR_INNER;
        goto Tail;
    }

    AFS_TRACE_OUT_DBG("Current ext4 filesystem: total blocks = %lld, block size = %d bytes, group num = %llu",
        (long long)brief_info.block_num, brief_info.block_size, brief_info.group_total_num);
    AFS_TRACE_OUT_DBG("per group: GDT blocks=%d, RGDT blocks=%d, Inode table blocks=%d, inode table size = %d bytes",
        (ext4_super_bk->s_desc_size * brief_info.group_total_num + brief_info.block_size - 1) / brief_info.block_size,
        ext4_super_bk->s_reserved_gdt_blocks, ext4_super_bk->s_inodes_per_group, ext4_super_bk->s_inode_size);

    // 计算组0前的偏移
    if (brief_info.block_size == EXT4_MIN_BLOCK) {
        // 如果块大小为ext4规定的最小大小，则组0前多1024字节的填充偏移
        brief_info.group_offset = SUPER_BLOCK_PADDING + brief_info.block_size;
    } else {
        brief_info.group_offset = brief_info.block_size;
    }

Tail:
    if (NULL != ext4_super_bk) {
        free(ext4_super_bk);
        ext4_super_bk = NULL;
    }

    AFS_TRACE_OUT_INFO("EXIT ext4Handler::ext_analyzeSuperBlock()");
    return this_return_value;
}

/**
 * @brief 通过索引节点号获取索引节点信息
 *
 * @param img_reader imgReader* 读取数据的句柄
 * @param brief_info struct ext4_brief_info* 分析文件系统所需的参数
 * @param group unsigned char* 组描述符首地址
 * @param inode struct ext4_inode* 存储获取的索引节点信息
 * @param current_inode_num int32_t 要获取的索引节点的索引节点号
 *
 * @return int32_t 获取成功返回0，获取失败返回1
 *
 */
int32_t ext4Handler::ext_getInode(imgReader *img_reader, struct ext4_brief_info &brief_info, unsigned char *group,
    struct ext4_inode *inode, int32_t current_inode_num)
{
    AFS_TRACE_OUT_DBG("ENTER ext4Handler::ext_getInode()");

    int32_t this_return_value = 0;
    uint64_t inode_table_addr = 0; // 索引节点表的地址（单位：块）

    // 先找到gdt,再从gdt中找到inode_table的位置
    // 组号
    uint32_t group_num = (uint32_t)(current_inode_num - 1) / brief_info.inodes_per_group;
    // 索引节点组内偏移（单位：索引节点）
    uint32_t inode_offset = (uint32_t)(current_inode_num - 1) % brief_info.inodes_per_group;

    // 某一组的组描述符首地址指针
    // 由组描述符首地址偏移后，指针按字节移动到要找的组描述符处，强制转换为组描述符结构体
    struct ext4_group_desc *ext4_group_des =
        reinterpret_cast<struct ext4_group_desc *>(group + group_num * brief_info.group_desc_size);
    inode_table_addr = ext_getLongValue(brief_info.feature_incompat, brief_info.group_desc_size,
        ext4_group_des->bg_inode_table_hi, ext4_group_des->bg_inode_table_lo);

    size_t read_ret = img_reader->read(inode,
        (int64_t)(inode_table_addr * (brief_info.block_size) + (int64_t)(inode_offset) * (brief_info.inode_size)),
        static_cast<int64_t>(sizeof(struct ext4_inode)), 0);
    if (read_ret != static_cast<int64_t>(sizeof(struct ext4_inode))) {
        AFS_TRACE_OUT_ERROR("Fail to read inode info!");
        this_return_value = AFS_ERR_IMAGE_READ;
    }

    AFS_TRACE_OUT_DBG("EXIT ext4Handler::ext_getInode()");
    return this_return_value;
}

/**
 * @brief 分析文件目录
 *
 * @param imgReader      读取数据的句柄
 * @param brief_info     分析文件系统必要的参数
 * @param bitmap_vect    文件在多个磁盘上的位图
 * @param group          组描述符首地址
 * @param file_path      过滤文件的路径
 *
 * @return int32_t 分析成功返回0，分析失败返回-1
 *
 */
int32_t ext4Handler::ext_analyzeFileDir(imgReader *img_reader, struct ext4_brief_info &brief_info,
    vector<BitMap *> &bitmap_vect, unsigned char *group, const char *file_path)
{
    AFS_TRACE_OUT_DBG("ENTER ext4Handler::ext_analyzeFileDir()");

    int32_t return_value = 0;
    string str_last_mounted = brief_info.last_mounted;
    AFS_TRACE_OUT_DBG("The last mount is : %s", str_last_mounted.c_str());
    string str_file_path = file_path;
    vector<string> filePath;
    int32_t current_inode_num = ROOT_INODE_NUM; // 根节点索引节点号为2

    struct ext4_inode inode; // inode空间
    CHECK_MEMSET_S_OK(&inode, sizeof(inode), 0, sizeof(inode));

    ext_split(str_file_path, "/", filePath);
    // 处理挂载目录
    if (filePath.empty()) { // rootDir.empty()
        AFS_TRACE_OUT_ERROR("The file is not in the file system.");
        return_value = AFS_ERR_PARA_PATH;
        goto Tail;
    }

    // 循环处理文件目录
    while (filePath.size()) {
        if (memset_s(&inode, sizeof(struct ext4_inode), 0, sizeof(struct ext4_inode)) != 0) {
            goto Tail;
        }
        return_value = ext_analyzeFileDir_realAnalyze(img_reader, brief_info, bitmap_vect, &inode, group, filePath,
            current_inode_num);
        if (return_value < 0) {
            goto Tail;
        }
    }

    return_value = current_inode_num;

Tail:

    AFS_TRACE_OUT_INFO("EXIT ext4Handler::ext_analyzeFileDir()");
    return return_value;
}

/**
 * @brief 分析文件目录的主体执行部分
 *
 * @param *img_reader          读取数据的句柄
 * @param *brief_info struct   分析文件系统必要的参数
 * @param &bitmap_vect         文件数据在多磁盘上的位图
 * @param *inode               索引节点信息
 * @param *group               组描述符首地址
 * @param &filePath            过滤文件的路径
 * @param &current_inode_num   当前索引节点号
 *
 * @return int32_t 分析成功返回当前索引节点号，分析失败返回负数
 *
 */
int32_t ext4Handler::ext_analyzeFileDir_realAnalyze(imgReader *img_reader, struct ext4_brief_info &brief_info,
    vector<BitMap *> &bitmap_vect, struct ext4_inode *inode, unsigned char *group, vector<string> &filePath,
    int32_t &current_inode_num)
{
    AFS_TRACE_OUT_DBG("ENTER ext4Handler::ext_analyzeFileDir_realAnalyze()");

    if (filePath.empty()) {
        AFS_TRACE_OUT_ERROR("File Path is empty!");
        return AFS_ERR_INNER;
    }

    int32_t return_value = 0;
    char *tempchar = NULL;

    // 通过索引节点号获取索引节点信息
    return_value = ext_getInode(img_reader, brief_info, group, inode, current_inode_num);
    if (return_value < 0) {
        AFS_TRACE_OUT_ERROR("Fail to get inode info!");
        return return_value;
    }

    // 编码规范中仅为建议，取vector中的其中一节路径名(string)为临时变量(char*)，进行分析
    tempchar = const_cast<char *>(filePath[0].c_str());
    AFS_TRACE_OUT_DBG("The filter file name: %s", tempchar);

    // 逐级寻找inode
    current_inode_num = ext_findInodeByDir(img_reader, brief_info, bitmap_vect, group, inode, tempchar, false);
    AFS_TRACE_OUT_DBG("The current ext4 directory's inode number is %d.", current_inode_num);

    // 如果返回的inode为0说明文件不在该文件系统中
    if (0 == current_inode_num) {
        AFS_TRACE_OUT_ERROR("The file is not in the file system.");
        return AFS_ERR_NOT_EXIST_PATH;
    } else if (current_inode_num < 0) {
        AFS_TRACE_OUT_ERROR("Fail to find inode by dir!");
        return current_inode_num;
    }
    // else无需处理，若文件在文件系统，则继续循环处理下一层目录

    filePath.erase(filePath.begin());

    AFS_TRACE_OUT_INFO("EXIT ext4Handler::ext_analyzeFileDir_realAnalyze()");
    return AFS_SUCCESS;
}

/**
 * @brief 实现找出文件所占块的功能
 *
 * @param file_path const char* 文件全路径
 * @param bitmap_vect   用于返回文件在多磁盘上的bitmap
 *
 * @return int 0设置成功 -1设置失败
 *
 */
int32_t ext4Handler::getFile(const char *file_path, vector<BitMap *> &bitmap_vect)
{
    AFS_TRACE_OUT_DBG("ENTER ext4Handler::getFile()");

    int32_t return_value = 0;
    int32_t read_ret = 0;
    imgReader *img_reader = NULL;
    struct ext4_brief_info brief_info;
    unsigned char *group = NULL;
    struct ext4_inode *inode = NULL;

    img_reader = getImgReader();
    if (NULL == img_reader) {
        AFS_TRACE_OUT_ERROR("Invalid image reader object(NULL).");
        return AFS_ERR_INNER;
    }

    // 分析超级块数据
    return_value = ext_analyzeSuperBlock(img_reader, brief_info);
    if (return_value != 0) {
        AFS_TRACE_OUT_ERROR("Fail to ext_analyzeSuperBlock!");
        goto Tail;
    }
    // 申请gdt内存空间
    group = static_cast<unsigned char *>(calloc(1, brief_info.group_desc_size * brief_info.group_total_num));
    if (NULL == group) {
        return_value = AFS_ERR_API;
        AFS_TRACE_OUT_ERROR("Fail to calloc group!");
        goto Tail;
    }
    // 读取gdt内容
    read_ret = img_reader->read(group, brief_info.group_offset,
        static_cast<int64_t>(brief_info.group_desc_size * brief_info.group_total_num), 1);
    if (read_ret != static_cast<int64_t>((brief_info.group_desc_size * brief_info.group_total_num))) {
        return_value = AFS_ERR_IMAGE_READ;
        AFS_TRACE_OUT_ERROR("Fail to read group!");
        goto Tail;
    }
    // 申请inode内存空间
    inode = static_cast<struct ext4_inode *>(calloc(1, sizeof(struct ext4_inode)));
    if (NULL == inode) {
        return_value = AFS_ERR_API;
        AFS_TRACE_OUT_ERROR("Fail to calloc inode!");
        goto Tail;
    }

    return_value = ext_getFile_realGet(file_path, bitmap_vect, img_reader, brief_info, group, inode);

Tail:
    if (group != NULL) {
        free(group);
        group = NULL;
    }
    if (inode != NULL) {
        free(inode);
        inode = NULL;
    }
    AFS_TRACE_OUT_INFO("EXIT ext4Handler::getFile()");
    return return_value;
}

/**
 * @brief 实现找出文件所占块的功能
 *
 * @param file_path const char* 文件全路径
 * @param bitmap_vect  用于返回文件在多磁盘上的bitmap
 * @param img_reader imgReader* 读取数据的句柄
 * @param brief_info struct ext4_brief_info* 分析文件系统必要的参数
 * @param group unsigned char* 组描述符首地址
 * @param inode ext4_inode* 索引节点信息
 *
 * @return int 0设置成功 -1设置失败
 *
 */
int32_t ext4Handler::ext_getFile_realGet(const char *file_path, vector<BitMap *> &bitmap_vect, imgReader *img_reader,
    struct ext4_brief_info &brief_info, unsigned char *group, struct ext4_inode *inode)
{
    AFS_TRACE_OUT_DBG("ENTER ext4Handler::ext_getFile_realGet()");

    int32_t inode_num = 0;
    int32_t return_value = 0;

    struct ext4_inode *tmp_inode = inode;

    // 分析文件目录，获取索引节点号
    inode_num = ext_analyzeFileDir(img_reader, brief_info, bitmap_vect, group, file_path);
    if (inode_num <= 0) {
        AFS_TRACE_OUT_ERROR("Fail to ext_analyzeFileDir!");
        return inode_num;
    }
    if (ROOT_INODE_NUM == inode_num) {
        AFS_TRACE_OUT_ERROR("The User try to delete the all partition!");
        return AFS_ERR_PARA_PATH;
    }

    // 通过索引节点号获取索引节点信息
    return_value = ext_getInode(img_reader, brief_info, group, tmp_inode, inode_num);
    if (return_value < 0) {
        AFS_TRACE_OUT_ERROR("Fail to get inode info!");
        return return_value;
    }

    // 通过索引节点信息读取文件数据
    return_value = ext_findFileByInode(img_reader, brief_info, bitmap_vect, group, tmp_inode);
    if (return_value < 0) {
        AFS_TRACE_OUT_ERROR("Fail to ext_findFileByInode!");
        return return_value;
    }

    AFS_TRACE_OUT_INFO("EXIT ext4Handler::ext_getFile_realGet()");
    return AFS_SUCCESS;
}
