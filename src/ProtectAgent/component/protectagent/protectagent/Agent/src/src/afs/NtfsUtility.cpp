/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include "afs/NtfsUtility.h"
#include "afs/LogMsg.h"
#include "afs/AfsError.h"

/**
 * @brief 查找指定文件名的MFT记录号
 *
 * @param mft_no_search  当前目录的MFT记录号
 * @param *search_name   所要查找的文件名(Unicode字符)
 * @param search_len     文件名长度(Unicode字符)
 * @param &mft_result    查找结果的MFT记录号
 * @param &mft_attr      MFT记录的属性(文件属性)
 *
 * @return         AFS_SUCCESS： 成功
 * AFS_ERR_NOT_EXIST_PATH： 不能正常匹配文件名
 * 其他 ： 失败（错误ID）
 *
 */
int32_t ntfsUtility::ntfs_searchMFTByName(MFT_REF mft_no_search, ntfschar *search_name, int32_t search_len,
    MFT_REF &mft_result, uint32_t &mft_attr)
{
    int32_t ret = 0;

    uint32_t index_block_size = 0; // 索引节点块大小
    uint64_t vcn_id = 0;           // VCN号

    int32_t attr_list_flag = 0;       // MFT记录是否含有属性列表
    ntfs_search_condition search_obj; // 查询条件

    CHECK_MEMSET_S_OK(&search_obj, sizeof(search_obj), 0, sizeof(search_obj));

    m_reparse_flg = 0; // 初始化当前是非符号链接
    search_obj.search_name = search_name;
    search_obj.search_len = search_len;

    // 初始化 MFT Buffer 空间
    CHECK_MEMSET_S_OK(m_mft_buffer, m_ntfs_info.mft_size, 0, m_ntfs_info.mft_size);

    // 读取 MFT 记录数据(1KB)
    ret = ntfs_getMFTData(mft_no_search, m_mft_buffer);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to read MFT data. MFT=0x%llx", (long long)mft_no_search);
        return ret;
    }

    // 当MFT记录存在属性列表属性(20H)时，相关的属性可能不在当前MFT中，需要检查属性列表
    attr_list_flag = ntfs_checkMFTAttrList();
    if (attr_list_flag < 0) {
        ret = attr_list_flag;
        AFS_TRACE_OUT_ERROR("Failed to check attribute list. MFT=0x%llx", (long long)mft_no_search);
        return ret;
    }

    // 根据90H属性查找文件
    ret = ntfs_searchByRoot(mft_no_search, attr_list_flag, &search_obj, index_block_size, mft_result, mft_attr, vcn_id);
    // ret=1:需要继续查找子节点
    // ret=0:成功匹配到文件/目录 mft_result为该文件的MFT记录号
    // ret=负数：出错或指定目录不存在
    AFS_TRACE_OUT_DBG("Search root attribute finished. ret = %d, mft_no = %lld", ret, (long long)mft_no_search);
    if (1 != ret) {
        return ret;
    }
    // 90H属性遍历结束，继续根据AOH属性查找文件
    AFS_TRACE_OUT_DBG("Search root attribute vcn_id = %lld", (long long)vcn_id);

    // 根据A0H属性查找文件
    ret = ntfs_searchByAllocation(mft_no_search, attr_list_flag, &search_obj, vcn_id, index_block_size, mft_result,
        mft_attr);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to search mft_no = %lld", (long long)mft_no_search);
        return ret;
    }

    // 执行到此说明成功找到指定文件或目录的MFT记录号
    AFS_TRACE_OUT_DBG("Success to found the specified file. mft_no = %lld", (long long)mft_no_search);

    return AFS_SUCCESS;
}

/**
 * @brief 从MFT的90H属性(根节点)中查找文件
 *
 * @param mft_no_search      当前目录的MFT记录号
 * @param attr_list_flag     该MFT是否含有属性列表
 * @param *search_obj        要查找的文件信息
 * @param &index_block_size  索引块的大小
 * @param &mft_result        查找到的MFT记录号
 * @param &mft_attr          MFT文件属性
 * @param &vcn_id            子节点的VCN记录号
 *
 * @return 0 成功匹配到文件/目录 mft_result为该文件的MFT记录号
 * 1 需要继续查找子节点
 * 负数   错误ID
 *
 */
int32_t ntfsUtility::ntfs_searchByRoot(MFT_REF mft_no_search, int32_t attr_list_flag, ntfs_search_condition *search_obj,
    uint32_t &index_block_size, MFT_REF &mft_result, uint32_t &mft_attr, uint64_t &vcn_id)
{
    int32_t ret = 0;

    uint8_t *mft_buffer_pos = NULL;
    ntfs_index_root *index_root = NULL;   // 索引头结构
    ntfs_index_entry *index_entry = NULL; // 索引项结构

    ntfs_attr_record attr_head; // 属性头
    uint32_t attr_start = 0;    // 属性头起始偏移

    mft_buffer_pos = ntfs_getMFTDataByAttrList(mft_no_search, m_mft_buffer, attr_list_flag, (uint32_t)AT_INDEX_ROOT);
    if (NULL == mft_buffer_pos) {
        AFS_TRACE_OUT_ERROR("Failed to get AT_INDEX_ROOT attribute data.");
        return AFS_ERR_INNER;
    }

    CHECK_MEMSET_S_OK(&attr_head, sizeof(attr_head), 0, sizeof(attr_head));
    ret = ntfs_getMFTAttr(mft_buffer_pos, (uint32_t)AT_INDEX_ROOT, &attr_head, &attr_start);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Cannot found file in AT_INDEX_ROOT attribute by MFT = %lld", (long long)mft_no_search);
        return ret;
    }

    // 得到索引头结构
    index_root = (ntfs_index_root *)((u8 *)mft_buffer_pos + attr_start + attr_head.data.resident.value_offset);
    index_block_size = index_root->index_block_size;
    if (index_block_size == 0) {
        AFS_TRACE_OUT_ERROR("Invalid index block size in root node.");
        return AFS_ERR_INNER;
    }

    // 第一个索引项
    index_entry = (ntfs_index_entry *)((u8 *)&index_root->index + index_root->index.entries_offset);
    search_obj->index_entry = index_entry;
    search_obj->node_flag = (uint16_t)(index_entry->ie_flags);
    search_obj->start_pos = mft_buffer_pos;
    search_obj->end_pos = (u8 *)&index_root->index + index_root->index.index_length;

    // ret=1:需要继续查找子节点
    // ret=0:成功匹配到文件/目录 mft_result为该文件的MFT记录号
    // ret=负数：出错或指定目录不存在
    ret = ntfs_findEntry(search_obj, vcn_id, mft_result, mft_attr);

    return ret;
}

/**
 * @brief 根据MFT记录号，读取该记录号A0H属性对应的簇流
 *
 * @param mft_no_search         当前目录的MFT记录号
 * @param attr_list_flag        该MFT是否含有属性列表
 * @param **data_runlist        保存簇流项的列表
 * @param &runlist_element_num  簇流项数
 *
 * @return 0 成功匹配到文件/目录 mft_result为该文件的MFT记录号
 * 负数   错误ID
 *
 */
int32_t ntfsUtility::ntfs_getSearchFileRunlist(MFT_REF mft_no_search, int32_t attr_list_flag,
    struct ntfs_runlist_element **data_runlist, uint32_t &runlist_element_num)
{
    int32_t ret = 0;

    uint8_t *mft_buffer_pos = NULL;
    ntfs_attr_record attr_head; // 属性头
    uint32_t attr_start = 0;    // 属性头起始偏移

    mft_buffer_pos =
        ntfs_getMFTDataByAttrList(mft_no_search, m_mft_buffer, attr_list_flag, (uint32_t)AT_INDEX_ALLOCATION);
    if (NULL == mft_buffer_pos) {
        AFS_TRACE_OUT_ERROR("Failed to read AT_INDEX_ALLOCATION attribute.");
        return AFS_ERR_INNER;
    }
    CHECK_MEMSET_S_OK(&attr_head, sizeof(attr_head), 0, sizeof(attr_head));
    ret = ntfs_getMFTAttr(mft_buffer_pos, (uint32_t)AT_INDEX_ALLOCATION, &attr_head, &attr_start);
    if (ret != 0) {
        AFS_TRACE_OUT_DBG("Current MFT has not index allocation attribute.");
        return AFS_ERR_INNER;
    }

    // 读取AOH属性的数据流
    ret = ntfs_getRunlist(mft_buffer_pos, &attr_head, attr_start, data_runlist, runlist_element_num);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to read index allocation attribute(A0H) runlist.");
        return ret;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 从MFT的A0H属性中查找文件
 *
 * @param mft_no_search      当前目录的MFT记录号
 * @param attr_list_flag     该MFT是否含有属性列表
 * @param *search_obj        要查找的文件信息
 * @param vcn_id_root        根节点对应的VCN记录号
 * @param index_block_size   索引块的大小
 * @param &mft_result        查找到的MFT记录号
 * @param &mft_attr          MFT文件属性
 *
 * @return 0 成功匹配到文件/目录 mft_result为该文件的MFT记录号
 * 1 需要继续查找子节点
 * 负数   错误ID
 *
 */
int32_t ntfsUtility::ntfs_searchByAllocation(MFT_REF mft_no_search, int32_t attr_list_flag,
    ntfs_search_condition *search_obj, uint64_t vcn_id_root, uint32_t index_block_size, MFT_REF &mft_result,
    uint32_t &mft_attr)
{
    int32_t ret = 0;

    struct ntfs_runlist_element *data_runlist = NULL; // 簇流列表信息
    uint32_t runlist_element_num = 0;                 // 记录簇流项数
    uint32_t index_vcn_size = 0;

    data_runlist = (struct ntfs_runlist_element *)calloc(1, sizeof(*data_runlist) * NTFS_RUNLIST_LENGTH);
    if (NULL == data_runlist) {
        AFS_TRACE_OUT_ERROR("Failed to allocate runlist data memory.");
        return AFS_ERR_API;
    }

    // 读取Runlist数据
    ret = ntfs_getSearchFileRunlist(mft_no_search, attr_list_flag, &data_runlist, runlist_element_num);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_DBG("Current MFT has not index allocation attribute.");
        goto out;
    }

    // 需要计算索引块对应的VCN大小
    AFS_TRACE_OUT_DBG(" index_block_size = %d", index_block_size);
    if (m_ntfs_info.cluster_size <= index_block_size) {
        index_vcn_size = m_ntfs_info.cluster_size;
    } else {
        index_vcn_size = m_ntfs_info.sector_size;
    }
    ret = ntfs_searchByRunlist(data_runlist, runlist_element_num, index_vcn_size, search_obj, vcn_id_root,
        index_block_size, mft_result, mft_attr);
    // ret=1:需要继续查找子节点（此时已经查找完所有Runlist，因此对该值出内部错误ID）
    // ret=0:成功匹配到文件/目录 mft_result为该文件的MFT记录号
    // ret=负数：出错或指定目录不存在
    if (ret == 1) {
        AFS_TRACE_OUT_ERROR("Failed to read search file by mft(%llu). ret = %d", (long long)mft_no_search, ret);
    }

out:
    if (NULL != data_runlist) {
        free(data_runlist);
        data_runlist = NULL;
    }

    return ret;
}

/**
 * @brief 检查索引节点块数据的有效性
 *
 * @param *index_block_buffer  索引节点的开始位置
 * @param index_block_size     索引节点块大小
 * @param vcn_id               索引节点的VCN号
 *
 * @return 0 成功匹配到文件/目录 mft_result为该文件的MFT记录号
 * 负数   错误ID
 *
 */
int32_t ntfsUtility::ntfs_checkIndexBlock(uint8_t *index_block_buffer, uint32_t index_block_size, uint64_t vcn_id)
{
    int32_t ret = 0;

    ntfs_index_allocation *index_allcation = NULL; // 索引分配数据
    u8 *index_end = NULL;                          // 索引数据结束位置

    // 更新索引块(4KB)中每个数据块(512字节)最后两个字节
    ret = ntfs_updatePostReadFixup((ntfs_record *)index_block_buffer, index_block_size);
    if (ret < 0) {
        AFS_TRACE_OUT_ERROR("Failed to fix up last two characters in index block data.");
        return ret;
    }

    index_allcation = (ntfs_index_allocation *)index_block_buffer;

    AFS_TRACE_OUT_DBG(" index_allcation->index_block_vcn:%lld", (long long)index_allcation->index_block_vcn);
    AFS_TRACE_OUT_DBG(" VCN:%lld", (long long)vcn_id);

    // VCN ID检查
    if (index_allcation->index_block_vcn != (s64)vcn_id) {
        AFS_TRACE_OUT_ERROR("Invalid VCN ID. Index block VCN=%lld, Para VCN=%lld",
            (long long)index_allcation->index_block_vcn, (long long)vcn_id);
        return AFS_ERR_INNER;
    }
    // 数据有效性检查 (0x18字节是索引块头)
    if ((index_allcation->index.allocated_size + 0x18) != index_block_size) {
        AFS_TRACE_OUT_ERROR("Invalid index allocate size. %d", (index_allcation->index.allocated_size + 0x18));
        return AFS_ERR_INNER;
    }
    // 数据范围检查
    index_end = (uint8_t *)&index_allcation->index + index_allcation->index.index_length;
    if (index_end > ((u8 *)index_allcation + index_block_size)) {
        AFS_TRACE_OUT_ERROR("Size of index buffer (VCN 0x%llx) exceeds maximum size.",
            (long long)index_allcation->index_block_vcn);
        return AFS_ERR_INNER;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 根据从MFT中的A0H数据流对应的地址查找相应的文件
 *
 * @param *data_runlist       MFT中描述的A0H属性的数据流项
 * @param runlist_element_num 数据流项数
 * @param index_vcn_size      vcn大小
 * @param *search_obj         查找条件
 * @param vcn_id_root         根记录号的VCN ID
 * @param index_block_size    索引块的大小
 * @param &mft_result         查找到的MFT记录号
 * @param &mft_attr           MFT文件属性
 *
 * @return 0 成功匹配到文件/目录 mft_result为该文件的MFT记录号
 * 1 需要继续查找子节点
 * 负数   错误ID
 *
 */
int32_t ntfsUtility::ntfs_searchByRunlist(struct ntfs_runlist_element *data_runlist, uint32_t runlist_element_num,
    uint32_t index_vcn_size, ntfs_search_condition *search_obj, uint64_t vcn_id_root, uint32_t index_block_size,
    MFT_REF &mft_result, uint32_t &mft_attr)
{
    int32_t ret = 0;
    uint64_t vcn_id = vcn_id_root;

    uint8_t *index_block_buffer = NULL;
    uint8_t *index_end = NULL;
    ntfs_index_allocation *index_allcation = NULL;

    index_block_buffer = (u8 *)calloc(1, index_block_size);
    if (NULL == index_block_buffer) {
        AFS_TRACE_OUT_ERROR("Failed to allocate index buffer memory. size=%d", index_block_size);
        return AFS_ERR_API;
    }

    while (1) {
        // 根据VCN ID计算出VCN号，再根据数据流得到索引块的起始位置,从而读取索引块数据
        ret = ntfs_getIndexBlockByVCNPosition(data_runlist, runlist_element_num, index_block_size, index_vcn_size,
            vcn_id, index_block_buffer);
        if (ret != 0) {
            AFS_TRACE_OUT_ERROR("Failed to read index block data by VCNId = %lld", (long long)vcn_id);
            ret = AFS_ERR_INNER;
            goto out;
        }

        // 检查索引节点块的数据有效性
        ret = ntfs_checkIndexBlock(index_block_buffer, index_block_size, vcn_id);
        if (ret != 0) {
            AFS_TRACE_OUT_ERROR("Failed to check index block data. ret = %d", ret);
            goto out;
        }

        // 设置参数
        index_allcation = (ntfs_index_allocation *)index_block_buffer;
        index_end = (uint8_t *)&index_allcation->index + index_allcation->index.index_length;
        search_obj->index_entry =
            (ntfs_index_entry *)((u8 *)&index_allcation->index + index_allcation->index.entries_offset);
        search_obj->node_flag = (uint16_t)(index_allcation->index.ih_flags);
        search_obj->start_pos = index_block_buffer;
        search_obj->end_pos = index_end;

        // ret=1:需要继续查找子节点
        // ret=0:成功匹配到文件/目录 mft_result为该文件的MFT记录号
        // ret=负数：出错或指定目录不存在
        ret = ntfs_findEntry(search_obj, vcn_id, mft_result, mft_attr);
        if (ret == AFS_SUCCESS) { // 成功匹配到
            AFS_TRACE_OUT_DBG("Success to find file.");
            break;
        }
        if (1 != ret) {
            AFS_TRACE_OUT_ERROR("Search end, it cannot to find file.");
            goto out;
        }
    }

out:
    search_obj->index_entry = NULL;
    search_obj->node_flag = 0;
    search_obj->start_pos = NULL;
    search_obj->end_pos = NULL;

    if (NULL != index_block_buffer) {
        free(index_block_buffer);
        index_block_buffer = NULL;
    }

    return ret;
}

/**
 * @brief 读取符号链接所对应的目录已经转换该Unicode字符到多字符
 *
 * @param *reparse_data_buffer        重解析数据Buffer
 * @param *reparse_path               重解析路径
 *
 * @return 0 解析成功
 * 负数  分析失败
 *
 */
int32_t ntfsUtility::ntfs_getReparsePath(uint8_t *reparse_data_buffer, char *reparse_path)
{
    int32_t ret = 0;
    int32_t temp_length = 0;
    uint32_t file_offset = 0;
    uint32_t convert_name_len = 0;

    ntfschar file_unicode[NTFS_MAX_PATH] = {0};
    ntfs_reparse *reparse_data = NULL;
    char *temp_file_buffer = NULL; // Unicode名转换为多字符

    file_offset = sizeof(ntfs_reparse);
    // 开始解析数据
    reparse_data = (ntfs_reparse *)reparse_data_buffer;
    if (REPARSE_TAG_SYMLINKD == reparse_data->reparse_tag) {
        file_offset += TAG_SYMLINKD_OFFSET;
    } else if (REPARSE_TAG_JUNCTION == reparse_data->reparse_tag) {
        file_offset += TAG_JUNCTION_OFFSET;
    } else {
        AFS_TRACE_OUT_ERROR("Unknown re-parse type. reparse_tag=%d", reparse_data->reparse_tag);
        return AFS_ERR_INNER;
    }

    // 读取重解析路径
    CHECK_MEMCPY_S_OK(file_unicode, NTFS_MAX_PATH * sizeof(ntfschar), reparse_data_buffer + file_offset,
        reparse_data->file_name_length);

    convert_name_len = (reparse_data->file_name_length + 1) * MB_CUR_MAX;
    temp_file_buffer = (char *)calloc(1, convert_name_len);
    if (NULL == temp_file_buffer) {
        return AFS_ERR_API;
    }

    ret = AFS_ERR_INNER;
    temp_length = ntfs_ucstombs(file_unicode, reparse_data->file_name_length, &temp_file_buffer, convert_name_len);
    if (temp_length > 0) {
        ret = memcpy_s(reparse_path, temp_length, temp_file_buffer, temp_length);
    }

    if (NULL != temp_file_buffer) {
        free(temp_file_buffer);
        temp_file_buffer = NULL;
    }

    return ret;
}

/**
 * @brief 分析重解析目录
 *
 * @param mft_no         当前文件的MFT记录号
 * @param *reparse_path  重解析路径
 * @return 0 非重解析点
 * 1 重解析点，返回reparse_path
 * 负数  分析失败
 *
 */
int32_t ntfsUtility::ntfs_analyzeReparsePath(MFT_REF mft_no, char *reparse_path)
{
    int32_t ret = 0;

    int32_t attr_list_flag = 0;
    uint64_t reparse_data_size = 0;

    uint8_t *reparse_data_buffer = NULL; // 重解析点数据

    // 检查10H文件属性，确定当前MFT记录是否是符号链接
    if (1 != m_reparse_flg) {
        return 0;
    }

    // 内存初始化
    CHECK_MEMSET_S_OK(m_mft_buffer, m_ntfs_info.mft_size, 0, m_ntfs_info.mft_size);

    // 读取MFT记录数据(1KB)
    ret = ntfs_getMFTData(mft_no, m_mft_buffer);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to read MFT data. mft=0x%llx", (long long)mft_no);
        goto out;
    }

    // 当MFT记录存在属性列表属性(20H)时，相关的属性可能不在当前MFT中，需要检查属性列表
    attr_list_flag = ntfs_checkMFTAttrList();
    if (attr_list_flag < 0) {
        ret = attr_list_flag;
        AFS_TRACE_OUT_ERROR("Failed to check attribute list. MFT=0x%llx", (long long)mft_no);
        goto out;
    }

    // 读取重解析数据
    ret = ntfs_getAllocationData(mft_no, attr_list_flag, (uint32_t)AT_REPARSE_POINT, &reparse_data_buffer,
        reparse_data_size);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to read AT_REPARSE_POINT attribute. MFT=%lld", (long long)mft_no);
        goto out;
    }

    ret = ntfs_getReparsePath(reparse_data_buffer, reparse_path);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to analyze re-parse path. MFT=%lld", (long long)mft_no);
        goto out;
    }

    AFS_TRACE_OUT_DBG("Current file is re-parse dir(%s), MFT=%lld", reparse_path, (long long)mft_no);

    ret = 1; // 存在符号链接

out:
    if (NULL != reparse_data_buffer) {
        free(reparse_data_buffer);
        reparse_data_buffer = NULL;
    }

    return ret;
}

/**
 * @brief 过滤当前MFT记录对应的子目录以及文件
 *
 * @param mft_no        当前文件MFT记录号
 * @param mft_attr      当前文件属性
 * @param &bitmap       返回过滤文件的bitmap
 *
 * @return 0 成功
 * AFS_ERR_FILE_TYPE 不支持的文件类型
 * 负值 失败
 */
int32_t ntfsUtility::ntfs_getFileFilterBitmap(MFT_REF mft_no, uint32_t mft_attr, BitMap &bitmap)
{
    int32_t ret = 0;

    // 指定Path最后的文件为目录
    if (mft_attr & (uint32_t)FILE_ATTR_I30_INDEX_PRESENT) {
        // 符号链接目录直接跳过
        if (mft_attr & (uint32_t)FILE_ATTR_REPARSE_POINT) {
            AFS_TRACE_OUT_DBG("Current directory is re-parse point.");
            return 0;
        }
        // 将当前的MFT记录号入队列，进行目录过滤
        ret = ntfs_FilterDirectory(mft_no, bitmap);
        if (ret != 0) {
            AFS_TRACE_OUT_ERROR("Failed to filter the specified directory.");
            return ret;
        }
        AFS_TRACE_OUT_DBG("Success to filter the specified directory.");
    } else if ((mft_attr & (uint32_t)FILE_ATTR_ARCHIVE) || (mft_attr == 0)) {
        // 属性类型是0的时候也是正常文件需要过滤
        AFS_TRACE_OUT_DBG("Current filter file attribute. MFT=%lld, FileAttr=%d", (long long)mft_no, mft_attr);
        ret = ntfs_getFileBitmap(mft_no, bitmap);
        if (ret != 0) {
            AFS_TRACE_OUT_ERROR("Current file cannot be supported. MFT=%lld, FileAttr=%d", (long long)mft_no, mft_attr);
            return ret;
        }
        // 其他类型不支持
    } else {
        AFS_TRACE_OUT_ERROR("Current file cannot be supported. MFT=%lld, FileAttr=%d", (long long)mft_no, mft_attr);
        return AFS_ERR_FILE_TYPE;
    }

    return 0;
}

/**
 * @brief 过滤当前MFT记录对应的子目录以及文件
 *
 * @param mft_no_filter   当前文件MFT记录号
 * @param &bitmap         返回过滤文件的bitmap
 *
 * @return  0： 成功
 * 负数： 错误ID
 */
int32_t ntfsUtility::ntfs_FilterDirectory(MFT_REF mft_no_filter, BitMap &bitmap)
{
    int32_t ret = 0;

    int32_t attr_list_flag = 0;
    uint64_t mft_no_search = 0;
    queue<uint64_t> file_queue;
    uint8_t *index_block_buffer = NULL; // 索引块空间(4KB)

    // 将当前MFT记录号写入队列
    file_queue.push(mft_no_filter);

    // 申请一个索引块节点的数据空间，查找过程中会不断使用
    // 申请索引块空间
    index_block_buffer = (uint8_t *)calloc(1, m_ntfs_info.index_block_size);
    if (NULL == index_block_buffer) {
        AFS_TRACE_OUT_ERROR("Failed to allocation index block space.");
        return AFS_ERR_INNER;
    }

    // 通过队列遍历目录下所有子目录和文件
    while (!file_queue.empty()) {
        mft_no_search = file_queue.front(); // 取出第一个MFT记录号
        file_queue.pop();                   // 移除当前目录MFT记录

        // 读取MFT记录数据(1KB)
        ret = ntfs_getMFTData(mft_no_search, m_mft_buffer);
        if (ret != 0) {
            AFS_TRACE_OUT_ERROR("Failed to read MFT data. mft=%lld", (long long)mft_no_search);
            continue;
        }

        // 当MFT记录存在属性列表属性(20H)时，相关的属性可能不在当前MFT中，需要检查属性列表
        attr_list_flag = ntfs_checkMFTAttrList();
        if (attr_list_flag < 0) {
            AFS_TRACE_OUT_DBG("The AT_ATTRIBUTE_LIST attribute is not exist. ");
            continue;
        }

        // 开始过滤目录
        ret = ntfs_filterMFTDir(mft_no_search, attr_list_flag, file_queue, index_block_buffer, bitmap);
        if (ret != 0) {
            continue;
        }
    } // end while queue

    if (NULL != index_block_buffer) {
        free(index_block_buffer);
        index_block_buffer = NULL;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 过滤当前MFT记录对应的所有文件
 *
 * @param mft_no_search        当前文件MFT记录号
 * @param attr_list_flag       是否存在属性列表标识
 * @param &file_queue          目录对应的MFT记录号队列
 * @param *index_block_buffer  索引块数据开始位置
 * @param &bitmap              返回过滤文件的bitmap
 *
 * @return 0 成功
 * 负数  失败
 */
int32_t ntfsUtility::ntfs_filterMFTDir(uint64_t mft_no_search, int32_t attr_list_flag, queue<uint64_t> &file_queue,
    uint8_t *index_block_buffer, BitMap &bitmap)
{
    int32_t ret = 0;

    uint32_t index_block_size = 0;
    uint64_t ia_size = 0;             // A0H对应数据大小
    uint64_t ia_bitmap_size = 0;      // B0H对应数据大小
    uint8_t *ia_data_buffer = NULL;   // A0H对应数据
    uint8_t *ia_bitmap_buffer = NULL; // B0H对应数据
    uint8_t *tmp_index_block_buffer = index_block_buffer;

    // 过滤90H属性中的文件
    ret = ntfs_filterRootDir(mft_no_search, attr_list_flag, index_block_size, file_queue, bitmap);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to filter file in root attribute. mft=%lld", (long long)mft_no_search);
        return ret;
    }

    // 读取AOH属性对应的数据
    ret =
        ntfs_getAllocationData(mft_no_search, attr_list_flag, (uint32_t)AT_INDEX_ALLOCATION, &ia_data_buffer, ia_size);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_INFO("Failed to read index allocation data. mft=%lld", (long long)mft_no_search);
        goto out;
    }

    // 读取B0H对应的位图数据
    ret = ntfs_getAllocationData(mft_no_search, attr_list_flag, (uint32_t)AT_BITMAP, &ia_bitmap_buffer, ia_bitmap_size);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to read bitmap data. mft=%lld", (long long)mft_no_search);
        goto out;
    }

    // 根据索引节点块的Bitmap过滤对应的所有文件
    ret = ntfs_filterIndexBlock(ia_data_buffer, ia_size, ia_bitmap_buffer, ia_bitmap_size, index_block_size, file_queue,
        tmp_index_block_buffer, bitmap);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to filter index block by Bitmap.");
        goto out;
    }

out:
    // 释放内存
    if (NULL != ia_data_buffer) {
        free(ia_data_buffer);
        ia_data_buffer = NULL;
    }

    if (NULL != ia_bitmap_buffer) {
        free(ia_bitmap_buffer);
        ia_bitmap_buffer = NULL;
    }

    return ret;
}

/**
 * @brief 过滤根节点中的文件，并将目录存储到队列中
 *
 * @param mft_no_search          当前MFT记录号
 * @param attr_list_flag         是否存在属性分配列表标识
 * @param &index_block_size      返回根节点中描述的索引块大小
 * @param &file_queue            将目录的MFT记录号写入队列
 * @param &bitmap                文件的数据块位置
 *
 * @return 0  成功
 * 负数  失败
 */
int32_t ntfsUtility::ntfs_filterRootDir(uint64_t mft_no_search, int32_t attr_list_flag, uint32_t &index_block_size,
    queue<uint64_t> &file_queue, BitMap &bitmap)
{
    int32_t ret = 0;

    uint32_t attr_start = 0;    // 属性头起始偏移
    ntfs_attr_record attr_head; // 属性头

    uint8_t *mft_buffer_pos = NULL;       // MFT数据指针
    uint8_t *index_end = NULL;            // 索引数据结束位置
    ntfs_index_root *index_root = NULL;   // 索引头结构
    ntfs_index_entry *index_entry = NULL; // 索引项结构

    mft_map find_result_map;

    // 查找90H属性头
    mft_buffer_pos = ntfs_getMFTDataByAttrList(mft_no_search, m_mft_buffer, attr_list_flag, (uint32_t)AT_INDEX_ROOT);
    if (NULL == mft_buffer_pos) {
        AFS_TRACE_OUT_DBG("The  AT_INDEX_ROOT attribute is not exist. MFT=%lld", (long long)mft_no_search);
        return AFS_ERR_INNER;
    }

    // 查找90H属性
    ret = ntfs_getMFTAttr(mft_buffer_pos, (uint32_t)AT_INDEX_ROOT, &attr_head, &attr_start);
    if (ret != 0) {
        AFS_TRACE_OUT_DBG("The  AT_INDEX_ROOT attribute is not exist.");
        return ret;
    }

    // 得到索引头结构
    index_root = (ntfs_index_root *)((u8 *)mft_buffer_pos + attr_start + attr_head.data.resident.value_offset);
    index_block_size = index_root->index_block_size;
    if (index_block_size == 0) {
        AFS_TRACE_OUT_ERROR("Invalid index block size.");
        return AFS_ERR_INNER;
    }
    index_end = (u8 *)&index_root->index + index_root->index.index_length;

    // 第一个索引项
    index_entry = (ntfs_index_entry *)((u8 *)&index_root->index + index_root->index.entries_offset);
    ret = ntfs_findMFTByIndexBlock(index_entry, mft_buffer_pos, index_end, find_result_map);
    if (0 != ret) {
        AFS_TRACE_OUT_ERROR("Failed to find MFT record in root attribute block. MFT=%lld ", (long long)mft_no_search);
        return ret;
    }

    // 处理当前索引块中的所有文件或目录
    ntfs_updateDirectoryQueue(find_result_map, file_queue, bitmap);

    return AFS_SUCCESS;
}

/**
 * @brief 读取索引分配属性对应的数据
 *
 * @param mft_no_search          当前MFT记录号
 * @param attr_list_flag         属性列表存在标志（0:无 1:有）
 * @param attr_num               属性ID
 * @param **data_buffer          返回属性数据
 * @param &data_size             返回属性数据大小
 *
 * @return 0  成功
 * 负数  失败
 */
int32_t ntfsUtility::ntfs_getAllocationData(uint64_t mft_no_search, int32_t attr_list_flag, uint32_t attr_num,
    uint8_t **data_buffer, uint64_t &data_size)
{
    int32_t ret = 0;

    uint32_t attr_start = 0;    // 属性头起始偏移
    ntfs_attr_record attr_head; // 属性头

    uint8_t *mft_buffer_pos = NULL; // MFT数据指针

    // 读取属性数据
    mft_buffer_pos = ntfs_getMFTDataByAttrList(mft_no_search, m_mft_buffer, attr_list_flag, attr_num);
    if (NULL == mft_buffer_pos) {
        AFS_TRACE_OUT_ERROR("Failed to read attribute(%d) in MFT=%lld", attr_num, (long long)mft_no_search);
        return AFS_ERR_INNER;
    }

    // 读取MFT记录数据的属性头
    CHECK_MEMSET_S_OK(&attr_head, sizeof(attr_head), 0, sizeof(attr_head));
    ret = ntfs_getMFTAttr(mft_buffer_pos, attr_num, &attr_head, &attr_start);
    if (ret != 0) {
        AFS_TRACE_OUT_DBG("Current MFT has no attribute. ret=%d attr_num=%d mft=%lld", ret, attr_num,
            (long long)mft_no_search);
        return ret;
    }

    // 读取实际数据
    ret = ntfs_readAttributeData(mft_buffer_pos, &attr_head, attr_start, data_buffer, data_size);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to read data. MFT=%lld, attr_num=%d", (long long)mft_no_search, attr_num);
        return ret;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 根据索引节点块的Bitmap过滤每个索引节点对应的数据
 *
 * @param *ia_data_buffer        索引节点数据
 * @param ia_size                索引数据大小
 * @param *ia_bitmap_buffer      索引块对应的Bitmap数据
 * @param ia_bitmap_size         Bitmap数据大小
 * @param index_block_size       索引节点块大小（一般4KB）
 * @param &file_queue            将目录的MFT记录号写入队列
 * @param *index_block_buffer    索引块数据指针
 * @param &bitmap                文件的数据块位置
 *
 * @return 0 成功
 * -1 失败
 */
int32_t ntfsUtility::ntfs_filterIndexBlock(uint8_t *ia_data_buffer, uint64_t ia_size, uint8_t *ia_bitmap_buffer,
    uint64_t ia_bitmap_size, uint32_t index_block_size, queue<uint64_t> &file_queue, uint8_t *index_block_buffer,
    BitMap &bitmap)
{
    int32_t ret = 0;

    uint32_t curr_bit_pos = 0;

    uint32_t index_vcn_size = 0;
    uint64_t ia_pos = 0;
    uint8_t *index_end = NULL; // 索引数据结束位置
    uint8_t *ia_data_end = (uint8_t *)(ia_data_buffer + ia_size);

    ntfs_index_entry *index_entry = NULL;          // 索引项结构
    ntfs_index_allocation *index_allcation = NULL; // 索引分配数据

    uint8_t *tmp_ia_bitmap_buffer = ia_bitmap_buffer;

    mft_map find_result_map;

    // 需要计算索引块对应的VCN大小
    if (m_ntfs_info.cluster_size <= index_block_size) {
        index_vcn_size = m_ntfs_info.cluster_size;
    } else {
        index_vcn_size = m_ntfs_info.sector_size;
    }

    while (curr_bit_pos < (ia_bitmap_size << 3)) { // 每个字节8位
        // 索引位图中位为0时表示没有数据
        if (!(tmp_ia_bitmap_buffer[curr_bit_pos >> 3] & (1 << ((curr_bit_pos % 8) & 7)))) {
            curr_bit_pos++;
            continue;
        }
        if (((uint64_t)(curr_bit_pos >> 3) * m_ntfs_info.index_block_size) >= ia_size) {
            AFS_TRACE_OUT_ERROR("Invalid bitmap position is specified.");
            break;
        }

        if ((ia_data_buffer + curr_bit_pos * m_ntfs_info.index_block_size) >= ia_data_end) {
            AFS_TRACE_OUT_ERROR("Read index block data by bitmap over buffer.");
            break;
        }

        // 每次处理一个索引块(4KB)
        CHECK_MEMCPY_S_OK(index_block_buffer, m_ntfs_info.index_block_size,
            (ia_data_buffer + curr_bit_pos * m_ntfs_info.index_block_size),
            m_ntfs_info.index_block_size); // 每次拷贝一个索引块的数据
        curr_bit_pos++;

        // 检查数据有效性
        ia_pos = (uint64_t)(curr_bit_pos - 1) * index_block_size;
        ret = ntfs_checkIndexBlock(index_block_buffer, index_block_size, (ia_pos / index_vcn_size));
        if (ret != 0) {
            AFS_TRACE_OUT_ERROR("Failed to check index block data.");
            break;
        }

        index_allcation = (ntfs_index_allocation *)index_block_buffer;
        index_entry = (ntfs_index_entry *)((u8 *)&index_allcation->index + index_allcation->index.entries_offset);
        index_end = (u8 *)&index_allcation->index + index_allcation->index.index_length;

        ret = ntfs_findMFTByIndexBlock(index_entry, index_block_buffer, index_end, find_result_map);
        if (0 != ret) {
            ret = 0;
            continue;
        }
        // 处理当前索引块中的所有文件或目录
        ntfs_updateDirectoryQueue(find_result_map, file_queue, bitmap);
    }

    return ret;
}

/**
 * @brief  设置查找到的文件在Bitmap中的对应位
 *
 * @param   &find_result_map   查询的MFT数据
 * @param   &file_queue        存放目录的队列
 * @param   &bitmap            返回Bitmap数据
 *
 * @return 0：成功 1：失败
 *
 */
void ntfsUtility::ntfs_updateDirectoryQueue(mft_map &find_result_map, queue<uint64_t> &file_queue, BitMap &bitmap)
{
    int32_t ret = 0;

    mft_map::iterator map_it = find_result_map.begin();
    while (map_it != find_result_map.end()) {
        if ((uint32_t)(map_it->second) & (uint32_t)FILE_ATTR_I30_INDEX_PRESENT) {
            if (!(map_it->second & (uint32_t)FILE_ATTR_REPARSE_POINT)) {
                AFS_TRACE_OUT_DBG("Add filter directory MFT. MFT=%lld", (long long)map_it->first);
                file_queue.push(map_it->first);
            } else {
                AFS_TRACE_OUT_ERROR("Re-Parse point. MFT=%lld, Attr=%d", (long long)map_it->first, map_it->second);
            }
            // 文件
        } else if ((map_it->second & (uint32_t)FILE_ATTR_ARCHIVE) || (map_it->second == 0)) {
            // 设置文件Bitmap
            ret = ntfs_getFileBitmap(map_it->first, bitmap);
            if (ret != 0) {
                AFS_TRACE_OUT_ERROR("Set file bitmap position failed. MFT = %lld", (long long)map_it->first);
            }
        } else {
            // 其他类型不进行处理
            AFS_TRACE_OUT_ERROR("Unsupported file type. MFT=%lld, FileType=%d", (long long)map_it->first,
                map_it->second);
        }
        ++map_it;
    }
}

/**
 * @brief  设置查找到的文件在Bitmap中的对应位
 *
 * @param   mft_no     文件对应的MFT记录号
 * @param   &bitmap    返回Bitmap数据
 * @return 0：成功  负数：失败
 *
 */
int32_t ntfsUtility::ntfs_getFileBitmap(MFT_REF mft_no, BitMap &bitmap)
{
    int32_t ret = 0;
    int32_t attr_list_flag = 0; // 属性列表标识

    AFS_TRACE_OUT_DBG("ntfs_getFileBitmap: Start to filter file MFT = 0x%llx", (long long)mft_no);

    uint8_t *mft_buffer_filter = (uint8_t *)calloc(1, m_ntfs_info.mft_size);
    if (NULL == mft_buffer_filter) {
        AFS_TRACE_OUT_ERROR("Failed to allocate mft memory to filter file. MFT=0x%llx", (long long)mft_no);
        return AFS_ERR_API;
    }

    // 读取Bitmap的MFT记录数据
    ret = ntfs_getMFTData(mft_no, mft_buffer_filter);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to read bitmap MFT. MFT=0x%llx", (long long)mft_no);
        goto out;
    }

    // 当MFT记录存在属性列表属性(20H)时，相关的属性可能不在当前MFT中，需要检查属性列表
    attr_list_flag = ntfs_checkMFTAttrList();
    if (attr_list_flag < 0) {
        ret = attr_list_flag;
        AFS_TRACE_OUT_ERROR("Failed to check attribute list. MFT=0x%llx", (long long)mft_no);
        goto out;
    }

    ret = ntfs_getFileBitmap_1(mft_no, mft_buffer_filter, attr_list_flag, bitmap);

out:
    if (mft_buffer_filter != NULL) {
        free(mft_buffer_filter);
        mft_buffer_filter = NULL;
    }

    return ret;
}

/**
 * @brief  设置查找到的文件在Bitmap中的对应位
 *
 * @param mft_no             文件对应的MFT记录号
 * @param *mft_buffer_filter MFT数据指针
 * @param attr_list_flag     当前属性是否有属性列表属性
 * @param &bitmap            返回Bitmap数据
 *
 * @return 0：成功 负数：失败（错误ID）
 *
 */
int32_t ntfsUtility::ntfs_getFileBitmap_1(MFT_REF mft_no, uint8_t *mft_buffer_filter, int32_t attr_list_flag,
    BitMap &bitmap)
{
    int32_t ret = 0;
    uint8_t *mft_buffer_pos = NULL;
    uint32_t data_attr_start = 0; // 0x80属性头起始偏移

    ntfs_attr_record attr_head; // bitmap 属性头
    CHECK_MEMSET_S_OK(&attr_head, sizeof(attr_head), 0, sizeof(attr_head));

    mft_buffer_pos = ntfs_getMFTDataByAttrList(mft_no, mft_buffer_filter, attr_list_flag, AT_DATA);
    if (NULL == mft_buffer_pos) {
        AFS_TRACE_OUT_ERROR("Failed to read AT_DATA attribute. MFT=0x%llx", (long long)mft_no);
        return AFS_ERR_INNER;
    }

    // 读取 Bitmap MFT的80H属性
    CHECK_MEMSET_S_OK(&attr_head, sizeof(ntfs_attr_record), 0, sizeof(ntfs_attr_record));
    ret = ntfs_getMFTAttr(mft_buffer_pos, (uint32_t)AT_DATA, &attr_head, &data_attr_start);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to read 80H attribute by MFT.MFT=0x%llx", (long long)mft_no);
        return ret;
    }

    // 常驻时说明数据很小(小于1KB)，不在Buffer中标识
    ret = ntfs_getFileBitmap_2(mft_buffer_pos, &attr_head, data_attr_start, bitmap);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to read 80H attribute by MFT.MFT=0x%llx", (long long)mft_no);
        return ret;
    }

    return AFS_SUCCESS;
}

/**
 * @brief  设置查找到的文件在Bitmap中的对应位
 *
 * @param  *mft_buffer       当前MFT的数据
 * @param  *attr_head        属性头结构体
 * @param  attr_start        属性开始位置
 * @param  &bitmap           bitmap
 *
 * @return 0：成功
 * 负数：失败
 *
 */
int32_t ntfsUtility::ntfs_getFileBitmap_2(uint8_t *mft_buffer, ntfs_attr_record *attr_head, uint32_t attr_start,
    BitMap &bitmap)
{
    int32_t ret = 0;

    uint32_t runlist_element_num = 0; // 记录簇流项数
    uint64_t start_pos = 0;           // 数据对应于Bitmap的起始位置
    uint64_t data_len = 0;            // 数据长度

    struct ntfs_runlist_element *data_runlist = NULL; // 簇流列表信息

    // 常驻时说明数据很小(小于1KB)，不在Buffer中标识
    if (!attr_head->non_resident) {
        AFS_TRACE_OUT_INFO("Current MFT's data is resident attribute. ");
        return 0;
    }

    data_runlist = (struct ntfs_runlist_element *)calloc(1, sizeof(*data_runlist) * NTFS_RUNLIST_LENGTH);
    if (NULL == data_runlist) {
        AFS_TRACE_OUT_ERROR("Failed to allocate RUNLIST buffer memory.");
        return AFS_ERR_API;
    }

    // 分析数据区的簇流数据
    ret = ntfs_getRunlist(mft_buffer, attr_head, attr_start, &data_runlist, runlist_element_num);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to call ntfs_getRunlist() function.");
        goto out;
    }

    // 设置数据对应的Bitmap位
    for (uint32_t i = 0; i < runlist_element_num; i++) {
        start_pos = ((uint64_t)(data_runlist[i].lcn * m_ntfs_info.cluster_size) >> NTFS_BLOCK_SIZE_BITS);
        data_len = ((uint64_t)(data_runlist[i].length * m_ntfs_info.cluster_size) >> NTFS_BLOCK_SIZE_BITS);
        // ret = bitmap.bitmapSetRangeMapAddr(m_reader, start_pos, data_len, 1);  //多磁盘的原因 暂时去掉
        if (ret != 0) {
            AFS_TRACE_OUT_ERROR("Set bitmap failed. Offset=%lld, DataSzie=%lld", (long long)start_pos,
                (long long)data_len);
            continue;
        }
    }
    ret = AFS_SUCCESS;

out:

    // 释放申请的Runlist空间
    if (NULL != data_runlist) {
        free(data_runlist);
        data_runlist = NULL;
    }

    return ret;
}

/**
 * @brief 初始化NTFS文件系统的基本信息
 *
 * @param &part_info  将分区基本数据返回
 * @param *img_reader 读取镜像的Reader指针
 *
 * @return int32_t 0  成功
 * 负数  失败
 *
 */
int32_t ntfsUtility::ntfs_initFSInfo(ntfs_part_info &part_info, imgReader *img_reader)
{
    int32_t cluster_size = 0;
    int64_t read_len = 0;
    ntfs_boot_sector boot_sector;
    CHECK_MEMSET_S_OK(&boot_sector, sizeof(boot_sector), 0, sizeof(boot_sector));

    m_reader = img_reader;

    // 初始化数据
    CHECK_MEMSET_S_OK(&m_ntfs_info, sizeof(ntfs_part_info), 0, sizeof(ntfs_part_info));

    // 读取引导扇区数据，计算分区实际簇大小
    CHECK_MEMSET_S_OK(&boot_sector, sizeof(boot_sector), 0, sizeof(boot_sector));
    read_len = m_reader->read(&boot_sector, 0, (int64_t)NTFS_BLOCK_SIZE, 1);
    if ((int64_t)NTFS_BLOCK_SIZE != read_len) {
        AFS_TRACE_OUT_ERROR("Read boot sector failed.");
        return AFS_ERR_IMAGE_READ;
    }

    // 计算一个簇的大小,单位:字节
    cluster_size = boot_sector.bpb.bytes_per_sector * boot_sector.bpb.sectors_per_cluster;

    m_ntfs_info.sector_size = boot_sector.bpb.bytes_per_sector;
    m_ntfs_info.cluster_size = cluster_size;

    AFS_TRACE_OUT_DBG("Current file system SectorSize=%d, ClusterSize=%d", m_ntfs_info.sector_size,
        m_ntfs_info.cluster_size);
    // 判断每个扇区的字节数是否为2的整数幂,若不是,说明这是一个无效的NTFS分区
    if (0 != (m_ntfs_info.sector_size & (m_ntfs_info.sector_size - 1))) {
        AFS_TRACE_OUT_ERROR("Invalid cluster size read in current volume. SectorSize = %d", m_ntfs_info.sector_size);
        return AFS_ERR_INNER;
    }
    if (0 != (cluster_size & (cluster_size - 1))) {
        AFS_TRACE_OUT_ERROR("Invalid cluster size read in current volume. ClusterSize = %d", m_ntfs_info.cluster_size);
        return AFS_ERR_INNER;
    }

    // MFT 项的大小,单位:字节
    m_ntfs_info.mft_size =
        m_ntfscomm.calculateMFTRecordSize(boot_sector.clusters_per_mft_record, (uint32_t)cluster_size);
    // 计算索引项大小
    m_ntfs_info.index_block_size =
        m_ntfscomm.calculateIndexBlockSize(boot_sector.clusters_per_index_record, (uint32_t)cluster_size);
    if ((0 != (m_ntfs_info.mft_size & (m_ntfs_info.mft_size - 1))) ||
        (0 != (m_ntfs_info.index_block_size & (m_ntfs_info.index_block_size - 1)))) {
        AFS_TRACE_OUT_ERROR("Invalid mft size read in current volume. MFTSize = %d", m_ntfs_info.mft_size);
        AFS_TRACE_OUT_ERROR("Invalid index block size read in current volume. BlockSize = %d",
            m_ntfs_info.index_block_size);
        return AFS_ERR_INNER;
    }

    // MFT 起始簇号
    m_ntfs_info.mft_lcn = boot_sector.mft_lcn;
    m_ntfs_info.sector_count = boot_sector.number_of_sectors;
    if ((0 == m_ntfs_info.mft_lcn) || (0 == m_ntfs_info.sector_count)) {
        AFS_TRACE_OUT_ERROR(
            "Invalid start mft LCN or sector count in current volume. mft_lcn = %lld, sector_count = %lld",
            (long long)m_ntfs_info.mft_lcn, (long long)m_ntfs_info.sector_count);
        return AFS_ERR_INNER;
    }

    CHECK_MEMCPY_S_OK(&part_info, sizeof(part_info), &m_ntfs_info, sizeof(ntfs_part_info));

    return AFS_SUCCESS;
}

/**
 * @brief 根据MFT记录号，读取该MFT记录的数据(大小为1024字节)
 *
 * @param mft_no      MFT记录号
 * @param *mft_buffer 返回读取的MFT数据
 *
 * @return int32_t AFS_SUCCESS 成功
 * AFS_ERR_IMAGE_READ 读失败
 * AFS_ERR_INNER      内部错误
 *
 */
int32_t ntfsUtility::ntfs_getMFTData(uint64_t mft_no, uint8_t *mft_buffer)
{
    int32_t ret = 0;

    uint64_t mft_offset = 0; // /MFT 记录的偏移,单位:字节
    int64_t read_len = 0;
    int64_t vcn_tmp = 0;
    uint8_t valid_flag = 0;

    uint8_t *tmp_mft_buffer = mft_buffer;

    vcn_tmp = mft_no * m_ntfs_info.mft_size / m_ntfs_info.cluster_size;
    // 计算MFT开始位置
    for (uint32_t zone_tmp = 0; zone_tmp < m_mft_zone_count; zone_tmp++) {
        // 检查当前MFT记录号所在的簇位置
        if ((vcn_tmp >= m_mft_zone_runlist[zone_tmp].vcn) &&
            (vcn_tmp < (m_mft_zone_runlist[zone_tmp].length) + m_mft_zone_runlist[zone_tmp].vcn)) {
            // MFT区域的相对偏移
            mft_offset =
                (m_mft_zone_runlist[zone_tmp].lcn - m_mft_zone_runlist[zone_tmp].vcn) * m_ntfs_info.cluster_size;
            // MFT相对偏移
            mft_offset += mft_no * m_ntfs_info.mft_size;

            valid_flag = 1;
            break;
        }
    }

    // 检查是否成功找到MFT记录所在的区间
    if (!valid_flag) {
        AFS_TRACE_OUT_ERROR("Failed to calculate MFT zone offset.");
        return AFS_ERR_INNER;
    }

    // 读取MFT数据(1024字节)
    CHECK_MEMSET_S_OK(tmp_mft_buffer, m_ntfs_info.mft_size, 0, m_ntfs_info.mft_size);
    read_len = m_reader->read(tmp_mft_buffer, (int64_t)mft_offset, (int64_t)m_ntfs_info.mft_size, 0);
    if ((int64_t)(m_ntfs_info.mft_size) != read_len) {
        AFS_TRACE_OUT_ERROR("Failed to read MFT data.");
        return AFS_ERR_IMAGE_READ;
    }

    // 根据更新序列号替换扇区最后两个字节
    ret = ntfs_updatePostReadFixup((ntfs_record *)tmp_mft_buffer, m_ntfs_info.mft_size);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to update block sequence number.");
        return AFS_ERR_INNER;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 分析MFT记录号所对应的簇流位置（一般情况下MFT记录号使用一个簇流存储）
 * @return int32_t AFS_SUCCESS 成功
 * AFS_ERR_IMAGE_READ 读失败
 * AFS_ERR_INNER      内部错误
 *
 */
int32_t ntfsUtility::ntfs_getMFTZoneRunlist()
{
    int32_t ret = 0;

    int64_t read_len = 0;
    uint8_t *mft_buffer = NULL;

    // 计算MFT开始位置
    uint64_t mft_offset = m_ntfs_info.mft_lcn * m_ntfs_info.cluster_size;

    mft_offset += ((uint32_t)FILE_MFT * m_ntfs_info.mft_size);

    // 读取MFT数据(1024字节)
    mft_buffer = (uint8_t *)calloc(1, m_ntfs_info.mft_size);
    if (NULL == mft_buffer) {
        AFS_TRACE_OUT_ERROR("Failed to allocation $MFT space.");
        return AFS_ERR_API;
    }
    read_len = m_reader->read(mft_buffer, (int64_t)mft_offset, (int64_t)m_ntfs_info.mft_size, 0);
    if ((int64_t)(m_ntfs_info.mft_size) != read_len) {
        AFS_TRACE_OUT_ERROR("Failed to read $MFT data.");
        ret = AFS_ERR_IMAGE_READ;
        goto out;
    }

    // 根据更新序列号替换扇区最后两个字节
    ret = ntfs_updatePostReadFixup((ntfs_record *)mft_buffer, m_ntfs_info.mft_size);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to update block sequence number.");
        ret = AFS_ERR_INNER;
        goto out;
    }

    ret = ntfs_getMFTZoneRunlist_1(mft_buffer);
    AFS_TRACE_OUT_INFO("$MFT data zone analyze finished. ret=%d", ret);

out:
    if (mft_buffer != NULL) {
        free(mft_buffer);
        mft_buffer = NULL;
    }

    return ret;
}

/**
 * @brief 分析MFT记录号所对应的簇流位置（一般情况下MFT记录号使用一个簇流存储）
 * @param *mft_buffer  指向MFT数据的指针
 * @return int32_t AFS_SUCCESS 成功
 * AFS_ERR_IMAGE_READ 读失败
 * AFS_ERR_INNER      内部错误
 */
int32_t ntfsUtility::ntfs_getMFTZoneRunlist_1(uint8_t *mft_buffer)
{
    int32_t ret = 0;

    ntfs_attr_record attr_head; // 元文件MFT 80H属性头
    uint32_t attr_start = 0;    // 文件 80H属性头起始偏移

    ret = ntfs_getMFTAttr(mft_buffer, (uint32_t)AT_DATA, &attr_head, &attr_start);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to read $MFT 80H attribute.");
        ret = AFS_ERR_INNER;
        return ret;
    }

    // 元文件$MFT的80H属性应该总是非常驻
    if (!(attr_head.non_resident)) {
        AFS_TRACE_OUT_ERROR("Failed to read $MFT 80H attribute. it's a resident attribute.");
        return AFS_ERR_INNER;
    }

    m_mft_zone_runlist = (struct ntfs_runlist_element *)calloc(1, sizeof(*m_mft_zone_runlist) * NTFS_RUNLIST_LENGTH);
    if (NULL == m_mft_zone_runlist) {
        AFS_TRACE_OUT_ERROR("Failed to allocate $MFT data runlist memory.");
        return AFS_ERR_API;
    }

    AFS_TRACE_OUT_DBG("Start to read $MFT data runlist.");
    ret = ntfs_getRunlist(mft_buffer, &attr_head, attr_start, &m_mft_zone_runlist, m_mft_zone_count);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to read $MFT 80H RUNLIST data. ret=%d", ret);
        return ret;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 获取MFT记录中指定的属性
 *
 * @param *mft_buffer     MFT数据
 * @param attr_num        属性ID
 * @param *attr_record    属性内容
 * @param *attr_start     返回属性起始位置
 *
 * @return int32_t 0 成功
 * -1  失败
 *
 */
int32_t ntfsUtility::ntfs_getMFTAttr(uint8_t *mft_buffer, uint32_t attr_num, ntfs_attr_record *attr_record,
    uint32_t *attr_start)
{
    int32_t ret = AFS_ERR_INNER;

    uint32_t attr_start_tmp = 0; // 属性相对MFT项开始处的偏移

    ntfs_mft_record mft_record;      // 存储MFT属性头信息
    struct ntfs_attr_look attr_head; // 记录属性头信息

    uint8_t *mft_local = mft_buffer;
    uint8_t *data_end = (uint8_t *)(mft_buffer + m_ntfs_info.mft_size);

    // 为局部变量分配空间
    CHECK_MEMSET_S_OK(&attr_head, sizeof(attr_head), 0, sizeof(attr_head));
    CHECK_MEMSET_S_OK(attr_record, sizeof(ntfs_attr_record), 0, sizeof(ntfs_attr_record));
    CHECK_MEMSET_S_OK(&mft_record, sizeof(mft_record), 0, sizeof(mft_record));
    *attr_start = 0;

    // 获取MFT头信息
    CHECK_MEMCPY_S_OK(&mft_record, sizeof(ntfs_mft_record), (ntfs_mft_record *)mft_local, sizeof(ntfs_mft_record));

    // 跳过MFT头，到属性区
    attr_start_tmp = mft_record.attrs_offset; // attr_start为MFT属性列表起始位置偏移
    mft_local += attr_start_tmp;

    // 遍历属性列表，查找所需的属性的起始地址和长度
    do {
        // 获取每一个属性头信息
        CHECK_MEMCPY_S_OK(&attr_head, sizeof(attr_head), (struct ntfs_attr_look *)mft_local, sizeof(attr_head));

        // 找到指定的属性
        if (attr_num == attr_head.attr_num) {
            *attr_start = attr_start_tmp; // 给返回值AT_DATA属性头偏移赋值
            ret = AFS_SUCCESS;
            break;
        }

        // 属性查找位置达到最后,不再继续查找
        if ((m_ntfs_info.mft_size <= attr_start_tmp) || ((uint32_t)AT_END == attr_head.attr_num)) {
            AFS_TRACE_OUT_DBG("The specified attribute can not found. attribute id = %d", attr_num);
            return AFS_ERR_INNER;
        }
        // 没找到属性,且没有找到MFT项结尾,循环继续
        attr_start_tmp += attr_head.attr_len; // 每次处理一个属性
        mft_local += attr_head.attr_len;
    } while (mft_local < data_end);

    // /读属性头信息,读取的内容存到bitmap_attr_head中
    if (ret == AFS_SUCCESS) {
        CHECK_MEMCPY_S_OK(attr_record, sizeof(ntfs_attr_record), (ntfs_attr_record *)mft_local,
            sizeof(ntfs_attr_record));
    } else {
        AFS_TRACE_OUT_ERROR("Failed to search attribute in MFT buffer. attribute id = %d", attr_num);
    }

    return ret;
}

/**
 * @brief 非常驻属性的数据读取
 *
 * @param *mft_buffer        MFT记录数据
 * @param attr_head          数据(80H)属性头
 * @param data_attr_start    数据起始位置
 * @param *data_buffer       返回数据
 * @param buffer_size        实际数据大小
 *
 * @return int32_t 0 成功
 * 负数  错误ID
 *
 */
int32_t ntfsUtility::ntfs_getDataFromRunlist(uint8_t *mft_buffer, ntfs_attr_record *attr_head, uint32_t data_attr_start,
    uint8_t *data_buffer, uint64_t buffer_size)
{
    int32_t ret = 0;

    uint32_t runlist_element_num = 0;                 // 记录簇流项数
    uint32_t bitmap_cluster_count = 0;                // Bitmap 数据占用的簇个数
    struct ntfs_runlist_element *data_runlist = NULL; // 簇流列表信息
    uint8_t *bitmap_buffer_tmp = NULL;                // Bitmap数据存储

    data_runlist = (struct ntfs_runlist_element *)calloc(1, sizeof(*data_runlist) * NTFS_RUNLIST_LENGTH);
    if (NULL == data_runlist) {
        AFS_TRACE_OUT_ERROR("Failed to allocate runlist memory.");
        return AFS_ERR_API;
    }

    // 分析数据区的簇流数据
    ret = ntfs_getRunlist(mft_buffer, attr_head, data_attr_start, &data_runlist, runlist_element_num);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to read RUNLIST data. ret=%d", ret);
        goto out;
    }

    // 计算数据所占用的簇数
    for (uint32_t element_i = 0; element_i < runlist_element_num; element_i++) {
        bitmap_cluster_count += data_runlist[element_i].length;
    }

    // 以簇为单位申请数据存放的空间
    bitmap_buffer_tmp = (uint8_t *)calloc(1, ((uint64_t)(bitmap_cluster_count)*m_ntfs_info.cluster_size));
    if (NULL == bitmap_buffer_tmp) {
        AFS_TRACE_OUT_ERROR("Failed to allocate data memory.");
        ret = AFS_ERR_API;
        goto out;
    }

    // 读取实际数据到bitmap_buffer_tmp中
    ret = ntfs_readDataByRunlist(runlist_element_num, data_runlist, bitmap_buffer_tmp);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to read data by RUNLIST. ret=%d", ret);
        goto out;
    }

    ret = memcpy_s(data_buffer, buffer_size, bitmap_buffer_tmp, buffer_size); // 数据流的簇数并不一定是实际数据大小
    if (ret != EOK) {
        AFS_TRACE_OUT_ERROR("call memcpy_s failed. ret=%d", ret);
        goto out;
    }

out:
    // 内存释放
    if (NULL != data_runlist) {
        free(data_runlist);
        data_runlist = NULL;
    }

    if (NULL != bitmap_buffer_tmp) {
        free(bitmap_buffer_tmp);
        bitmap_buffer_tmp = NULL;
    }

    return ret;
}

/**
 * @brief 读取分区大小写表
 *
 * @param *unicode                 大小写表
 * @param upcase_len               大小写表长度
 * @param *upcase_mft              MFT记录数据
 * @param upcase_attr_head         属性头
 * @param upcase_data_attr_start   属性头起始位置
 *
 * @return int32_t 0 成功
 * 负数  错误ID
 *
 */
int32_t ntfsUtility::ntfs_getVolumeUpcase(ntfschar *unicode, uint64_t upcase_len, uint8_t *upcase_mft,
    ntfs_attr_record upcase_attr_head, uint32_t upcase_data_attr_start)
{
    int32_t ret = 0;

    uint32_t runlist_element_num = 0; // 记录簇流项数
    uint64_t clusterCount = 0;

    struct ntfs_runlist_element *upcase_runlist = NULL; // 簇流列表信息

    if ((upcase_attr_head.data.non_resident.data_size == 0) ||
        (upcase_len * 2 < upcase_attr_head.data.non_resident.data_size)) {
        AFS_TRACE_OUT_ERROR("Invalid size. upcase_len=%lld, upcase_attr_head.data.non_resident.data_size=%d",
            (long long)upcase_len, (long long)upcase_attr_head.data.non_resident.data_size);
        return AFS_ERR_INNER;
    }

    upcase_runlist = (struct ntfs_runlist_element *)calloc(1, sizeof(*upcase_runlist) * NTFS_RUNLIST_LENGTH);
    if (NULL == upcase_runlist) {
        AFS_TRACE_OUT_ERROR("Failed to allocate RUNLIST memory.");
        return AFS_ERR_API;
    }

    // 获取数据流
    ret = ntfs_getRunlist((u8 *)upcase_mft, &upcase_attr_head, upcase_data_attr_start, &upcase_runlist,
        runlist_element_num);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to get data runlist error. ret=%d", ret);
        goto out;
    }

    // 计算数据实际占用的簇数
    for (uint32_t element_i = 0; element_i < runlist_element_num; element_i++) {
        clusterCount += (uint64_t)(upcase_runlist[element_i].length);
    }

    // 获取实际数据
    ret = ntfs_getVolumeUpcase_1(unicode, upcase_len, clusterCount, upcase_runlist, runlist_element_num,
        upcase_attr_head);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to read volume UPCase data. ret=%d", ret);
        goto out;
    }

out:
    // 内存释放
    if (NULL != upcase_runlist) {
        free(upcase_runlist);
        upcase_runlist = NULL;
    }

    AFS_TRACE_OUT_DBG("ntfsHandler::ntfs_getVolumeUpcase() Success to get up case data.");

    return ret;
}

/**
 * @brief 读取分区大小写表函数2
 *
 * @param *unicode                 大小写表
 * @param upcase_len               大小写表长度
 * @param clusterCount             簇个数
 * @param *upcase_runlist          大小写数据所对应的簇流列表
 * @param runlist_element_num      簇流个数
 * @param upcase_attr_head         属性头
 *
 * @return int32_t 0  成功
 * 负数  错误ID
 *
 */
int32_t ntfsUtility::ntfs_getVolumeUpcase_1(ntfschar *unicode, uint64_t upcase_len, uint64_t clusterCount,
    struct ntfs_runlist_element *upcase_runlist, uint32_t runlist_element_num, ntfs_attr_record upcase_attr_head)
{
    int32_t ret = 0;
    uint8_t *upcase_data_cluster = NULL; // 临时数据存放
    uint32_t cluster_size = m_ntfs_info.cluster_size;
    uint64_t copyStartIndex = 0;
    int64_t read_len = 0;

    // 检查数据有效性
    if ((0 == clusterCount) || (0 == cluster_size)) {
        AFS_TRACE_OUT_ERROR("Invalid runlist data, Cluster count is zero.");
        return AFS_ERR_INNER;
    }

    upcase_data_cluster = (uint8_t *)calloc(1, clusterCount * cluster_size);
    if (NULL == upcase_data_cluster) {
        ret = AFS_ERR_API;
        AFS_TRACE_OUT_ERROR("Failed to allocate UpCase data memory.");
        goto out;
    }

    // 根据Runlist得到数据
    for (uint32_t element_i = 0; element_i < runlist_element_num; element_i++) {
        // 从第二项簇流开始，LCN是相对于前一项的偏移
        if (element_i > 0) {
            copyStartIndex += (uint64_t)(cluster_size * (upcase_runlist[element_i - 1].length));
        }
        // 从磁盘读取
        read_len = m_reader->read(upcase_data_cluster + copyStartIndex, upcase_runlist[element_i].lcn * cluster_size,
            cluster_size * upcase_runlist[element_i].length, 0);
        if ((s64)read_len != cluster_size * upcase_runlist[element_i].length) {
            AFS_TRACE_OUT_ERROR("Failed to read data.");
            ret = AFS_ERR_IMAGE_READ;
            goto out;
        }
    }

    ret = memcpy_s(unicode, upcase_attr_head.data.non_resident.data_size, upcase_data_cluster,
        upcase_attr_head.data.non_resident.data_size);
    if (ret != EOK) {
        AFS_TRACE_OUT_ERROR("call memcpy_s failed, ret = %d", ret);
        goto out;
    }

    ret = AFS_SUCCESS;

out:
    if (NULL != upcase_data_cluster) {
        free(upcase_data_cluster);
        upcase_data_cluster = NULL;
    }

    return ret;
}

/**
 * @brief 将多字符转换为Unicode字符
 *
 * @param *ins    多字符的Buffer
 * @param *ucs    返回转换后的Unicode字符Buffer
 * @param ucs_len Unicode字符长度
 *
 * @return int32_t -1 失败
 * 其他：返回字符长度
 *
 */
int32_t ntfsUtility::ntfs_doConvertMB2UC(const char *ins, ntfschar *ucs, int32_t ucs_len)
{
    int32_t out_len = 0;
    int32_t cnt = 0;
    uint32_t i = 0;
    uint32_t ins_size = (uint32_t)strlen(ins);
    wchar_t wc = L'0';

    for (; i < ins_size; i += cnt, out_len++) {
        /* Convert the multibyte character to a wide character. */
        cnt = mbtowc(&wc, ins + i, ins_size - i);
        if (0 == cnt) {
            break;
        }

        if ((cnt <= -1) || (out_len >= (ucs_len / 2))) {
            return -1;
        }
        ucs[out_len] = wc;
    }

    return out_len;
}

/**
 * @brief 将多字符转换为Unicode字符
 *
 * @param *ins       多字符的Buffer
 * @param *outs      返回转换后的Unicode字符Buffer
 * @param outs_len   Unicode字符长度
 *
 * @return int32_t 0  成功
 * -1 失败
 *
 */
int32_t ntfsUtility::ntfs_mbstoucs(const char *ins, ntfschar *outs, int32_t outs_len)
{
    int32_t ucs_len = outs_len;
    int32_t out_len = 0;

    ntfschar *ucs = outs;

    if ((NULL == ins) || (NULL == ucs) || (0 == outs_len)) {
        return -1;
    }

    out_len = ntfs_doConvertMB2UC(ins, ucs, ucs_len);
    if (out_len <= 0) {
        return -1;
    }

    /* Now write the NULL character. */
    ucs[out_len] = L'\0';

    return out_len;
}

/**
 * @brief 处理每个Unicode字符到多字符的转换
 *
 * @param *ins       需要转换的Unicode字符
 * @param ins_len    unicode字符长度
 * @param mbs_len    多字符长度
 * @param **mbs     返回转换后的多字符Buffer
 * @param **outs     参数传入时的地址
 *
 * @return int32_t -1  失败
 * 其他    返回字符长度
 *
 */
int32_t ntfsUtility::ntfs_doConvertUC2MBs(const ntfschar *ins, const int32_t ins_len, int32_t mbs_len, char **mbs,
    char **outs)
{
    wchar_t wc = L'0';
    int32_t i = 0;
    int32_t out_len = 0;
    int32_t cnt = 0;

    for (i = out_len = 0; i < ins_len; i++) {
        wc = (wchar_t)ins[i];
        if (!wc) {
            break;
        }
        cnt = wctomb(*mbs + out_len, wc);
        if (cnt <= 0) {
            return -1;
        }
        out_len += cnt;
    }

    if (out_len <= 0) {
        return -1;
    }

    return out_len;
}

/**
 * @brief 将Unicode字符转换为多字符
 *
 * @param *ins       需要转换的Unicode字符
 * @param ins_len    unicode字符长度
 * @param **outs     返回转换后的多字符Buffer
 * @param outs_len   转换后的字符长度
 *
 * @return int32_t -1  失败
 * 其他    返回字符长度
 *
 */
int32_t ntfsUtility::ntfs_ucstombs(const ntfschar *ins, const int32_t ins_len, char **outs, int32_t outs_len)
{
    char *mbs = NULL;
    int32_t mbs_len = 0;
    int32_t ret = 0;

    mbs = *outs;
    mbs_len = outs_len;

    if (NULL == ins || NULL == mbs || ins_len <= 0) {
        return -1;
    }

    ret = ntfs_doConvertUC2MBs(ins, ins_len, mbs_len, &mbs, outs);
    if (ret < 0) {
        return -1;
    }

    /* Now write the NULL character. */
    mbs[ret] = '\0';
    return ret;
}

//************************************************************************************
// * 描述:
// *   解析簇流信息，将解析的结果存入data_runlist结构体数组中
// *   比如：在磁盘中存储了如下数据：
// *        31 0C 9F 8F 08 22 80 00 12 A0 21 02 FF 00 00
// *        读取第一个字节 0x31,可知第一条簇流项的簇流长度占一个字节，簇流起始位置占3个字节。
// *        这样我们可以读取第一条簇流项 31 0C 9F 8F 08
// *        *data_runlist[0].vcn  = 0(vcn 从0开始编号)
// *        *data_runlist[0].lcn  = 561055(0x088F9F)
// *        *data_runlist[0].length= 12(0x0C)
// *        接着我们读取下一字节 0x22，分析同上，读取第二条簇流项 22 80 00 12 A0
// *        *data_runlist[1].vcn  = 12(*data_runlist[0].vcn+*data_runlist[0].length)
// *        *data_runlist[1].lcn  = 536497(0xA012=-24558,561055+(-24558)=536497)
// *        *data_runlist[1].length= 128(0x0080)
// *        接着我们读取下一字节 0x21，分析同上，读取第三条簇流项 21 02 FF 00
// *        *data_runlist[2].vcn  = 140(*data_runlist[1].vcn+*data_runlist[1].length)
// *        *data_runlist[2].lcn  = 536752(0x00FF=536497+255=536752)
// *        *data_runlist[2].length= 2(0x02)
// *        接着我们读取下一字节 0x00，说明为结束标志，簇流列表解析完成
// *************************************************************************************/

/**
 * @brief 分析非常驻属性的数据流
 *
 * @param *mft_buffer           MFT记录数据
 * @param *attr_head            属性头
 * @param data_attr_start      属性起始位置
 * @param **data_runlist         返回Runlist链表
 * @param runlist_element_num  Runlist记录个数
 *
 * @return int32_t 0 成功
 * -1 失败
 *
 */
int32_t ntfsUtility::ntfs_getRunlist(uint8_t *mft_buffer, ntfs_attr_record *attr_head, uint32_t data_attr_start,
    struct ntfs_runlist_element **data_runlist, uint32_t &runlist_element_num)
{
    // 局部变量定义区
    int32_t ret = 0;
    uint32_t runlist_length = NTFS_RUNLIST_LENGTH; // runlist_length 长度初始值
    uint32_t runlist_count = 0;                    // g_ntfs_extend.runlist的计数变量
    uint8_t runlist_one_length = 0;                // 一个簇流记录长度
    uint8_t mapping_first_byte = 0;                // 簇流项的第一个字节
    uint8_t *mft_local = mft_buffer;
    uint64_t mapping_start = 0; // 读取数据的起始地址

    uint8_t *mft_local_end = mft_buffer + m_ntfs_info.mft_size;

    // 簇流列表开始位置
    mapping_start = data_attr_start + attr_head->data.non_resident.mapping_pairs_offset;

    // 定位到簇流列表起始偏移
    mft_local += mapping_start;

    // 解析簇流列表
    do {
        if (mft_local >= mft_local_end) { // 指针越界
            AFS_TRACE_OUT_ERROR("Current position exceed MFT buffer size.");
            return AFS_ERR_INNER;
        }

        // 读取簇流项的第一字节
        CHECK_MEMCPY_S_OK(&mapping_first_byte, sizeof(mapping_first_byte), (u8 *)mft_local, sizeof(mapping_first_byte));
        AFS_TRACE_OUT_DBG("mapping_first_byte is %x", mapping_first_byte);

        // 结束标志为0x00
        if (0 == mapping_first_byte) {
            break; // Runlist 查找结束
        }

        ret = m_ntfscomm.analyzeRunList(mft_local, mapping_first_byte, runlist_count, runlist_one_length, data_runlist,
            runlist_length);
        if (ret != AFS_SUCCESS) {
            AFS_TRACE_OUT_ERROR("Failed to analyze runnlist.");
            return ret;
        }

        AFS_TRACE_OUT_DBG("ntfsHandler::ntfs_getRunlist() >>runlist[%d].lcn is %lld", runlist_count,
            (long long)(*data_runlist)[runlist_count].lcn);
        AFS_TRACE_OUT_DBG("ntfsHandler::ntfs_getRunlist() >>runlist[%d].vcn is %lld", runlist_count,
            (long long)(*data_runlist)[runlist_count].vcn);
        AFS_TRACE_OUT_DBG("ntfsHandler::ntfs_getRunlist() >>runlist[%d].length is %lld", runlist_count,
            (long long)(*data_runlist)[runlist_count].length);

        runlist_count++;                 // 准备读下一条记录
        mft_local += runlist_one_length; // 读取的起始位置
    } while (1);

    runlist_element_num = runlist_count; // MFT记录数据的簇流项个数

    return AFS_SUCCESS;
}

/**
 * @brief 读取NTFS文件系统的大小写表
 * @return  0 读取成功
 * 其他  失败
 */
int32_t ntfsUtility::ntfs_getVolumeUpcaseData()
{
    int32_t ret = 0;
    uint32_t upcase_data_attr_start = 0; // 0x80属性头起始偏移
    ntfs_attr_record attr_head;          // bitmap 属性头

    // 检查MFT数据空间是否有效
    if (NULL == m_mft_buffer) {
        AFS_TRACE_OUT_ERROR("MFT buffer have not created.");
        return AFS_ERR_INNER;
    }

    CHECK_MEMSET_S_OK(m_mft_buffer, m_ntfs_info.mft_size, 0, m_ntfs_info.mft_size);

    // 从第10号MFT(FILE_UpCase)读取大小写信息表
    ret = ntfs_getMFTData((uint64_t)FILE_UpCase, m_mft_buffer);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to read FILE_UpCase data by FILE_UpCase.");
        return ret;
    }

    // 读取 FILE_UpCase MFT的80H属性
    ret = ntfs_getMFTAttr(m_mft_buffer, (uint32_t)AT_DATA, &attr_head, &upcase_data_attr_start);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to read FILE_UpCase's AT_DATA attribute.");
        return ret;
    }

    // 检查数据长度
    if (attr_head.data.non_resident.data_size < 2) {
        AFS_TRACE_OUT_ERROR("Invalid data size.");
        return AFS_ERR_INNER;
    }

    // Unicode字符长度
    m_upcase_len = attr_head.data.non_resident.data_size >> 1;
    m_upcase_data = (ntfschar *)calloc(1, attr_head.data.non_resident.data_size);
    if (NULL == m_upcase_data) {
        AFS_TRACE_OUT_ERROR("Failed to allocate memory.");
        return AFS_ERR_API;
    }

    // 读取实际数据
    ret = ntfs_getVolumeUpcase(m_upcase_data, m_upcase_len, m_mft_buffer, attr_head, upcase_data_attr_start);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to read UpCase table data.");
        return -1;
    }

    return 0;
}

/**
 * @brief 检查NTFS文件系统版本，本项目支持的NTFS版本为(3.1)
 * @return            0 读取成功
 * AFS_ERR_INNER 内部错误
 * 其他负值   错误
 */
int32_t ntfsUtility::ntfs_checkVersion()
{
    int32_t ret = 0;

    // 申请MFT数据空间
    uint8_t *mft_buffer = (uint8_t *)calloc(1, m_ntfs_info.mft_size);
    if (NULL == mft_buffer) {
        AFS_TRACE_OUT_ERROR("MFT buffer have not been allocated.");
        return AFS_ERR_API;
    }

    // 从第3号MFT(FILE_Volume)读取卷信息表
    ret = ntfs_getMFTData((uint64_t)FILE_Volume, mft_buffer);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to read FILE_Volume MFT data.");
        free(mft_buffer);
        return ret;
    }

    uint32_t attr_start = 0;    // 0x70属性头起始偏移
    ntfs_attr_record attr_head; // 属性头
    CHECK_MEMSET_S_OK(&attr_head, sizeof(attr_head), 0, sizeof(attr_head));
    // 读取 FILE_Volume MFT的0x70H属性
    ret = ntfs_getMFTAttr(mft_buffer, (uint32_t)AT_VOLUME_INFORMATION, &attr_head, &attr_start);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to read FILE_Volume's AT_VOLUME_INFORMATION attribute.");
        free(mft_buffer);
        mft_buffer = NULL;
        return ret;
    }

    // NTFS版本信息
    ntfs_attr_volinfo volume_information; // 卷属性数据
    uint8_t *version_offset = mft_buffer + attr_start + attr_head.data.resident.value_offset;
    CHECK_MEMSET_S_OK(&volume_information, sizeof(volume_information), 0, sizeof(volume_information));
    ret = memcpy_s(&volume_information, sizeof(ntfs_attr_volinfo), version_offset, sizeof(ntfs_attr_volinfo));
    if (ret != EOK) {
        AFS_TRACE_OUT_ERROR("call memcpy_s() failed, ret = %d", ret);
        free(mft_buffer);
        mft_buffer = NULL;
        return ret;
    }

    free(mft_buffer);
    mft_buffer = NULL;

    // 未使用
    if (0 != volume_information.unused) {
        AFS_TRACE_OUT_ERROR("It is not a valid NTFS volume! Read Value = %lld", (long long)volume_information.unused);
        return AFS_ERR_FS_VERSION;
    }

    if (!((NTFS_MAJOR_VER == volume_information.major_vor) && (NTFS_MINOR_VER == volume_information.minor_vor))) {
        AFS_TRACE_OUT_ERROR("Not supported NTFS volume! major_vor=%d, minor_vor=%d", volume_information.major_vor,
            volume_information.minor_vor);
        return AFS_ERR_FS_VERSION;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 申请文件过滤时所需要的MFT数据空间
 * @return            0 成功
 * AFS_ERR_API 失败
 */
int32_t ntfsUtility::ntfs_callocSearchSpace()
{
    // 申请一个MFT记录的空间
    m_mft_buffer = (uint8_t *)calloc(1, m_ntfs_info.mft_size);
    if (NULL == m_mft_buffer) {
        AFS_TRACE_OUT_ERROR("Allocate MFT buffer memory failed.");
        return AFS_ERR_API;
    }

    // 申请MFT属性列表对应MFT数据的Buffer
    m_attr_list_mft_buffer = (uint8_t *)calloc(1, m_ntfs_info.mft_size);
    if (NULL == m_attr_list_mft_buffer) {
        AFS_TRACE_OUT_ERROR("Allocate MFT list buffer memory failed.");
        return AFS_ERR_API;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 释放文件过滤时所分配的数据空间
 * @return void
 *
 */
void ntfsUtility::ntfs_freeSearchSpace()
{
    // MFT数据Buffer
    if (NULL != m_mft_buffer) {
        free(m_mft_buffer);
        m_mft_buffer = NULL;
    }

    // MFT存在属性列表时对应的MFT数据Buffer
    if (NULL != m_attr_list_mft_buffer) {
        free(m_attr_list_mft_buffer);
        m_attr_list_mft_buffer = NULL;
    }

    if (NULL != m_atrr_list_data) {
        free(m_atrr_list_data);
        m_atrr_list_data = NULL;
    }

    // 释放大小写表空间
    if (NULL != m_upcase_data) {
        free(m_upcase_data);
        m_upcase_data = NULL;
    }
    return;
}

/**
 * @brief 释放元文件$MFT的80H对应的数据
 * @return void
 *
 */
void ntfsUtility::ntfs_freeMFTZoneSpace()
{
    if (m_mft_zone_runlist != NULL) {
        free(m_mft_zone_runlist);
        m_mft_zone_runlist = NULL;
    }
}

/**
 * @brief 检查MFT记录是否有20H(属性列表)属性
 * @return int32_t 0 无属性列表属性
 * 1 有属性列表
 * 负数   处理失败
 */
int32_t ntfsUtility::ntfs_checkMFTAttrList()
{
    int32_t ret = 0;

    uint32_t attr_start = 0;    // 属性头起始偏移
    ntfs_attr_record attr_head; // 属性头

    // 检查MFT buffer
    if (NULL == m_mft_buffer) {
        AFS_TRACE_OUT_ERROR("MFT data buffer have not allocated.");
        return AFS_ERR_INNER;
    }

    // 检查MFT记录中是否有属性列表(20H属性)
    CHECK_MEMSET_S_OK(&attr_head, sizeof(attr_head), 0, sizeof(attr_head));
    ret = ntfs_getMFTAttr(m_mft_buffer, (uint32_t)AT_ATTRIBUTE_LIST, &attr_head, &attr_start);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_DBG("AT_ATTRIBUTE_LIST attribute is not existed.");
        return 0; // 不存在属性列表返回0
    }

    // 申请一个MFT记录的空间(属性列表时用)
    if (NULL == m_attr_list_mft_buffer) {
        AFS_TRACE_OUT_ERROR("MFT attribute list's MFT data buffer have not allocated.");
        return AFS_ERR_INNER;
    }
    CHECK_MEMSET_S_OK(m_attr_list_mft_buffer, m_ntfs_info.mft_size, 0, m_ntfs_info.mft_size);

    // 每次检查属性列表时，数据大小不一致，所以重新分配
    if (NULL != m_atrr_list_data) {
        free(m_atrr_list_data);
        m_atrr_list_data = NULL;
    }

    ret = ntfs_readAttributeData(m_mft_buffer, &attr_head, attr_start, &m_atrr_list_data, m_attr_list_length);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to read AT_ATTRIBUTE_LIST attribute data.");
        return ret;
    }

    return 1; // 存在属性列表
}

/**
 * @brief 查找MFT记录中是否存在属性列表属性
 *
 * @param mft_no               当前MFT记录号
 * @param *mft_buffer_main     MFT记录的主数据Buffer
 * @param attr_list_flg        MFT记录中是否存在属性列表(0:无，1：有)
 * @param attr_num             要查找的属性ID
 *
 * @return NULL   解析失败
 * 非NULL  MFT buffer的起始地址
 *
 */
uint8_t *ntfsUtility::ntfs_getMFTDataByAttrList(MFT_REF mft_no, uint8_t *mft_buffer_main, int32_t attr_list_flg,
    uint32_t attr_num)
{
    int32_t ret = 0;

    uint8_t *mft_pos = mft_buffer_main;
    uint8_t *search_pos = m_atrr_list_data;
    ntfs_attr_list_entry ale;
    MFT_REF tmp_mft_no = 0;

    // attr_list_flg为0时设定数据的起始地址
    if (0 == attr_list_flg) {
        return mft_buffer_main;
    }

    // 检查属性列表中对应属性所在MFT的Buffer是否已经创建
    if ((NULL == m_attr_list_mft_buffer) || (NULL == search_pos) || (NULL == m_atrr_list_data)) {
        AFS_TRACE_OUT_ERROR("Attribute list MFT buffer have not allocated. MFT=0x%llx", (long long)mft_no);
        return NULL;
    }

    CHECK_MEMSET_S_OK_RETURN(m_attr_list_mft_buffer, m_ntfs_info.mft_size, 0, m_ntfs_info.mft_size, NULL);

    // 查找属性
    while (search_pos < (uint8_t *)(m_atrr_list_data + m_attr_list_length)) {
        // 拷贝一个属性数据
        CHECK_MEMCPY_S_OK_RETURN(&ale, sizeof(ale), (ntfs_attr_list_entry *)search_pos, sizeof(ale), NULL);
        if ((AT_UNUSED == ale.type) || (0 == ale.length)) {
            AFS_TRACE_OUT_ERROR("Invalid attribute.");
            return NULL;
        }

        if ((uint32_t)(ale.type) != attr_num) {
            search_pos += ale.length;
            continue;
        }

        // 查找到指定的属性
        tmp_mft_no = MREF(ale.mft_reference);
        if (mft_no == tmp_mft_no) {
            // 查找的属性在当前的MFT记录中
            mft_pos = mft_buffer_main;
            break;
        }

        // 读取MFT记录的数据, m_ntfs_info.mft_size参数在此是否正确
        CHECK_MEMSET_S_OK_RETURN(m_attr_list_mft_buffer, m_ntfs_info.mft_size, 0, m_ntfs_info.mft_size, NULL);
        ret = ntfs_getMFTData(tmp_mft_no, m_attr_list_mft_buffer);
        if (ret != 0) {
            AFS_TRACE_OUT_ERROR("Failed to read MFT data. mft=0x%llx, ret=%d", (long long)tmp_mft_no, ret);
            return NULL;
        }
        mft_pos = m_attr_list_mft_buffer;
        break;
    }

    return mft_pos;
}

/**
 * @brief 根据数据流拷贝实际的数据
 *
 * @param runlist_element_num      数据流个数
 * @param *data_runlist            数据流链表
 * @param *process_data_cluster    返回数据
 *
 * @return int32_t 0 成功
 * 负数   失败
 *
 */
int32_t ntfsUtility::ntfs_readDataByRunlist(uint32_t runlist_element_num,
    const struct ntfs_runlist_element *data_runlist, uint8_t *process_data_cluster)
{
    uint32_t runlist_element_num_i = 0;   // runlist循环变量
    int64_t runlist_element_num_i_in = 0; // runlist的单个簇流项循环变量
    uint64_t data_copy_pos = 0;
    uint64_t ready_to_read_byte = 0; // 当前打算读和写的字节数
    uint64_t read_offset = 0;        // 读和写的偏移
    uint64_t need_to_copy_byte = 0;  // 还需要拷贝的数据大小（字节）
    int64_t read_bytes = 0;          // 对应读取磁盘数据的返回值

    uint8_t *tmp_process_data = process_data_cluster;

    // 输入参数检查
    if ((0 == runlist_element_num) || (NULL == data_runlist) || (NULL == tmp_process_data) || (NULL == m_reader)) {
        AFS_TRACE_OUT_ERROR("Invalid parameter to read data by runlist!");
        return AFS_ERR_INNER;
    }

    // 处理整个簇流项
    for (runlist_element_num_i = 0; runlist_element_num_i < runlist_element_num; runlist_element_num_i++) {
        // 处理一个簇流项
        for (runlist_element_num_i_in = 0; runlist_element_num_i_in < data_runlist[runlist_element_num_i].length;) {
            // 还需要拷贝的数据，runlist_element_num_i_in 每次增加已经拷贝的簇数
            need_to_copy_byte =
                (data_runlist[runlist_element_num_i].length - runlist_element_num_i_in) * m_ntfs_info.cluster_size;
            // 判断此次读取的大小
            if (need_to_copy_byte >= NTFS_PROCESS_BYTE) {
                ready_to_read_byte = NTFS_PROCESS_BYTE; // 需要读取的大小不能一次读取
            } else {
                ready_to_read_byte = need_to_copy_byte; // 能够一次读取
            }

            // 读写偏移,把lcn转换成字节偏移
            // 读数据的偏移，lcn加上已经拷贝的簇数就是这次需要读取的簇号
            read_offset = (uint64_t)((data_runlist[runlist_element_num_i].lcn + runlist_element_num_i_in) *
                m_ntfs_info.cluster_size);

            // 从磁盘读取
            read_bytes =
                m_reader->read(tmp_process_data + data_copy_pos, (int64_t)read_offset, (int64_t)ready_to_read_byte, 0);
            if (read_bytes != (int64_t)ready_to_read_byte) {
                AFS_TRACE_OUT_ERROR("Read data error!");
                return AFS_ERR_IMAGE_READ;
            }

            data_copy_pos += ready_to_read_byte;
            // 下次读位置(簇)
            runlist_element_num_i_in += ready_to_read_byte / m_ntfs_info.cluster_size;
        } // 处理完一个簇流列表
    }

    return 0;
}

/**
 * @brief 当数据占多个块(512字节)时，根据更新序列号替换块的最后两个字节
 *
 * @param *record                 需要处理的数据
 * @param record_size             数据大小
 *
 * @return int32_t 0 成功
 * -1 失败
 *
 */
int32_t ntfsUtility::ntfs_updatePostReadFixup(ntfs_record *record, const uint32_t record_size)
{
    uint16_t usa_ofs = 0;
    uint16_t usa_count = 0;
    uint16_t usn = 0;
    uint16_t *usa_pos = NULL;
    uint16_t *data_pos = NULL;

    usa_ofs = record->usa_ofs;
    usa_count = record->usa_count - 1;
    // 检查大小以及字节对齐
    if ((record_size & (NTFS_BLOCK_SIZE - 1)) || (usa_ofs & 1) || (u32)(usa_ofs + (usa_count * 2)) > record_size ||
        (record_size >> NTFS_BLOCK_SIZE_BITS) != usa_count) {
        return -1;
    }

    usa_pos = (u16 *)record + usa_ofs / sizeof(u16); // 更新序列号的位置

    // The update sequence number which has to be equal to each of the
    // u16 values before they are fixed up. Note no need to care for
    // endianness since we are comparing and moving data for on disk
    // structures which means the data is consistent. - If it is
    // consistency the wrong endianness it doesn't make any difference.
    usn = *usa_pos;

    // Position in protected data of first u16 that needs fixing up.
    data_pos = (u16 *)record + NTFS_BLOCK_SIZE / sizeof(u16) - 1;

    // Check for incomplete multi sector transfer(s).
    while (usa_count--) {
        if (*data_pos != usn) {
            // Incomplete multi sector transfer detected! )-:
            // Set the magic to "BAAD" and return failure.
            // Note that magic_BAAD is already converted to le32.
            record->magic = magic_BAAD;
            return -1;
        }
        data_pos += NTFS_BLOCK_SIZE / sizeof(u16);
    }
    // Re-setup the variables.
    usa_count = record->usa_count - 1;
    data_pos = (u16 *)record + NTFS_BLOCK_SIZE / sizeof(u16) - 1;
    // 遍历所有块(每个512字节)
    while (usa_count--) {
        /*
         * Increment position in usa and restore original data from
         * the usa into the data buffer.
         */
        *data_pos = *(++usa_pos);
        /* Increment position in data as well. */
        data_pos += NTFS_BLOCK_SIZE / sizeof(u16);
    }

    return 0;
}

/**
 * @brief 比较两个Unicode字符串
 *
 * @param *name1                第一个要比较的Unicode字符串
 * @param name1_len             第一个要比较的Unicode字符串长度
 * @param *name2                第二个要比较的Unicode字符串
 * @param name2_len             第二个要比较的Unicode字符串长度
 * @param ic                    CASE_SENSITIVE 或者 IGNORE_CASE
 * @param *upcase               大小写更新表(来源于分区的Volume)
 * @param upcase_len            大小写更新表长度
 *
 * @return -1 第一个字符串值大
 * 0 两个字符串相同
 * 1 第二个字符串值大
 *
 */
int32_t ntfsUtility::ntfs_collateNames(const ntfschar *name1, const uint32_t name1_len, const ntfschar *name2,
    const uint32_t name2_len, const IGNORE_CASE_BOOL ic, const ntfschar *upcase, const uint32_t upcase_len)
{
    int32_t ret = 0;
    uint32_t cnt = 0;
    ntfschar c1 = 0;
    ntfschar c2 = 0;

    for (cnt = 0; cnt < min(name1_len, name2_len); ++cnt) {
        c1 = *name1;
        name1++;
        c2 = *name2;
        name2++;
        ret = ntfs_collateNames_1(ic, upcase, upcase_len, c1, c2);
        if (ret == 0) {
            continue;
        } else {
            return ret;
        }
    }
    if (name1_len < name2_len) {
        return -1;
    }
    if (name1_len == name2_len) {
        return 0;
    }

    /* name1_len > name2_len */
    return 1;
}

/**
 * @brief 比较两个Unicode字符
 *
 * @param ic                   CASE_SENSITIVE 或 IGNORE_CASE
 * @param *upcase              大小写更新表(来源于分区的Volume)
 * @param upcase_len           大小写更新表长度
 * @param c1                   第一个要比较的Unicode字符
 * @param c2                   第二个要比较的Unicode字符
 *
 * @return -1 第一个字符串值大
 * 0 两个字符串相同
 * 1 第二个字符串值大
 *
 */
int32_t ntfsUtility::ntfs_collateNames_1(const IGNORE_CASE_BOOL ic, const ntfschar *upcase, const uint32_t upcase_len,
    const ntfschar c1, const ntfschar c2)
{
    ntfschar c1_tmp = 0;
    ntfschar c2_tmp = 0;

    if (ic) {
        if (c1 < upcase_len) {
            c1_tmp = upcase[c1];
        }
        if (c2 < upcase_len) {
            c2_tmp = upcase[c2];
        }
    }
    if (c1_tmp < c2_tmp) {
        return -1;
    }
    if (c1_tmp > c2_tmp) {
        return 1;
    }

    return 0;
}

/**
 * @brief 比较两个Unicode字符串
 *
 * @param &search_obj     查找参数
 * @param *vcnID          VCN号
 * @param &mft_result     查找结果的MFT记录号
 * @param &mft_attr       返回MFT属性
 *
 * @return  1 未找到MFT记录号，需要根据VCN号查找子节点；
 * 0 找到指定文件名的MFT记录号
 * AFS_ERROR_INNER 查找异常
 * AFS_ERR_NOT_EXIST_PATH 未匹配到指定文件
 *
 */
int32_t ntfsUtility::ntfs_findEntry(ntfs_search_condition *search_obj, uint64_t &vcnID, MFT_REF &mft_result,
    uint32_t &mft_attr)
{
    int32_t ret = 0;
    ntfs_index_entry *index_entry = search_obj->index_entry;
    ntfschar *search_name = search_obj->search_name;
    int32_t search_len = search_obj->search_len;

    for (;; index_entry = (ntfs_index_entry *)((uint8_t *)index_entry + index_entry->length)) {
        // 检查数据有效
        if (((uint8_t *)index_entry < search_obj->start_pos) ||
            (((uint8_t *)index_entry + sizeof(ntfs_index_entry_header)) > search_obj->end_pos) ||
            (((uint8_t *)index_entry + index_entry->key_length) > search_obj->end_pos)) {
            AFS_TRACE_OUT_ERROR("Invalid index entry position.");
            return AFS_ERR_INNER;
        }

        ret = ntfs_findEntry_1(index_entry, search_name, search_len, mft_result, mft_attr);
        if (ret == 0) {
            return AFS_SUCCESS; // 匹配成功
        } else if (ret == 1) {
            continue;
        } else {
            break;
        }
    }

    // 检查是否是叶节点
    ret = ntfs_isLeafNode(index_entry, search_obj->node_flag, vcnID);
    return ret;
}

/**
 * @brief 分析一个索引项并比较是否与指定的文件名匹配
 *
 * @param *index_entry     当前索引项
 * @param *search_name     待匹配的文件名
 * @param search_len       待匹配的文件名长度
 * @param &mft_result      MFT查找结果
 * @param &mft_attr        MFT属性
 *
 * @return  0  成功匹配到指定的文件名
 * 1  继续遍历当前索引块
 * 2  需要从子节点继续查找
 *
 */
int32_t ntfsUtility::ntfs_findEntry_1(ntfs_index_entry *index_entry, ntfschar *search_name, int32_t search_len,
    MFT_REF &mft_result, uint32_t &mft_attr)
{
    // 叶节点，结束查找
    if ((uint16_t)(index_entry->ie_flags) & (uint16_t)(INDEX_ENTRY_END)) {
        AFS_TRACE_OUT_DBG("Leaf node.");
        return 2;
    }

    // 匹配文件名
    int32_t ignore_case_ret =
        ntfs_collateNames(search_name, search_len, (ntfschar *)(index_entry->key.file_name.file_name),
        index_entry->key.file_name.file_name_length, IGNORE_CASE, m_upcase_data, m_upcase_len);
    ntfs_printFileName(index_entry);

    if (-1 == ignore_case_ret) {
        return 2; // 需要查找子节点
    } else if (1 == ignore_case_ret) {
        return 1; // 继续查找
    }

    // 匹配到指定的文件/目录名
    // 检查是否是重解析点
    if ((uint32_t)(index_entry->key.file_name.file_attributes) & (uint32_t)(FILE_ATTR_REPARSE_POINT)) {
        m_reparse_flg = 1;
    } else {
        m_reparse_flg = 0;
    }

    mft_attr = (uint32_t)(index_entry->key.file_name.file_attributes);
    mft_result = MREF(index_entry->uname.indexed_file);
    return AFS_SUCCESS; // 匹配成功
}

/**
 * @brief 打印当前索引项对应的文件名
 *
 * @param *index_entry     当前索引项
 *
 * @return  无
 *
 */
void ntfsUtility::ntfs_printFileName(const ntfs_index_entry *index_entry)
{
    char *curr_name_buff = NULL;
    uint32_t convert_name_len = 0;
    uint64_t tmp_mft_no = 0;

    tmp_mft_no = MREF(index_entry->uname.indexed_file);
    if (0 == tmp_mft_no) {
        return;
    }

    convert_name_len = ((int32_t)(index_entry->key.file_name.file_name_length) + 1) * MB_CUR_MAX;
    curr_name_buff = (char *)calloc(1, convert_name_len);
    if (NULL == curr_name_buff) {
        return;
    }
    int32_t temp_length = ntfs_ucstombs((ntfschar *)index_entry->key.file_name.file_name,
        (int32_t)index_entry->key.file_name.file_name_length, &curr_name_buff, convert_name_len);
    if (temp_length > 0) {
        AFS_TRACE_OUT_DBG("NTFS Filter:: Current file name: %s, fileLen=%d", curr_name_buff, temp_length);
    }

    if (NULL != curr_name_buff) {
        free(curr_name_buff);
        curr_name_buff = NULL;
    }
}

/**
 * @brief 检查当前索引项是否已经到叶节点，叶节点时说明未找到指定文件名，非叶节点时返回子节点的VCN号
 *
 * @param *index_entry     当前索引项
 * @param node_flag        节点标识
 * @param &vcnID           非叶节点时返回VCN号
 *
 * @return  1  根据VCN ID继续查找子节点
 * AFS_ERR_NOT_EXIST_PATH  文件不存在
 *
 */
int32_t ntfsUtility::ntfs_isLeafNode(ntfs_index_entry *index_entry, uint16_t node_flag, uint64_t &vcnID)
{
    if ((uint16_t)(index_entry->ie_flags) & (uint16_t)(INDEX_ENTRY_NODE)) {
        if ((node_flag & (uint16_t)NODE_MASK) == (uint16_t)(LEAF_NODE)) {
            AFS_TRACE_OUT_DBG("Search to leaf node.");
            return AFS_ERR_NOT_EXIST_PATH; // 查找结束（未能匹配）
        }
        CHECK_MEMCPY_S_OK(&vcnID, sizeof(uint64_t), (uint64_t *)((uint8_t *)index_entry + index_entry->length - 8),
            sizeof(uint64_t));
        AFS_TRACE_OUT_DBG("Continue to search by VCN = %lld", (long long)vcnID);
        return 1; // 需要继续遍历子节点
    } else {
        AFS_TRACE_OUT_ERROR("Finished to search but can not found the specified file.");
        return AFS_ERR_NOT_EXIST_PATH; // 查找结束（未能匹配）
    }
}

/**
 * @brief 查找索引块数据中所有的文件、目录
 *
 * @param *index_entry    索引块第一个索引项的位置
 * @param *start_pos      索引块起始位置
 * @param *end_pos        索引块结束位置
 * @param &mft_result    返回查找到的MFT记录号已经对应属性
 *
 * @return  0：成功遍历结束
 * 负数： 出错（错误ID）
 *
 */
int32_t ntfsUtility::ntfs_findMFTByIndexBlock(ntfs_index_entry *index_entry, uint8_t *start_pos, uint8_t *end_pos,
    mft_map &mft_result)
{
    ntfs_index_entry *ie_tmp = index_entry;
    uint64_t tmp_mft_no = 0;

    mft_result.clear();
    for (;; ie_tmp = (ntfs_index_entry *)((u8 *)ie_tmp + ie_tmp->length)) {
        // 检查数据有效
        if (((uint8_t *)ie_tmp < start_pos) || (((uint8_t *)ie_tmp + sizeof(ntfs_index_entry_header)) > end_pos) ||
            (((uint8_t *)ie_tmp + ie_tmp->key_length) > end_pos)) {
            AFS_TRACE_OUT_ERROR("Current position is not valid.");
            return AFS_ERR_INNER;
        }
        tmp_mft_no = MREF(ie_tmp->uname.indexed_file);
        if (tmp_mft_no == 0) {
            break;
        }

        // Unicode转多字符
        ntfs_findMFTByIndexBlock_1(ie_tmp, mft_result, tmp_mft_no);

        // 叶节点，结束查找
        if ((uint16_t)(ie_tmp->ie_flags) & (uint64_t)INDEX_ENTRY_END) {
            AFS_TRACE_OUT_DBG("Leaf node end.");
            break;
        }
    }

    return 0;
}

/**
 * @brief 将文件对应的MFT记录号存储到Map中
 *
 * @param *index_entry    当前索引项的开始位置
 * @param &mft_result     记录MFT记录号的Map
 * @param tmp_mft_no      当前MFT记录号
 *
 * @return  0  成功遍历结束
 * 负数      出错
 *
 */
void ntfsUtility::ntfs_findMFTByIndexBlock_1(ntfs_index_entry *index_entry, mft_map &mft_result, uint64_t tmp_mft_no)
{
    int32_t temp_length = 0;
    uint32_t convert_name_len = 0;
    ntfs_index_entry *ie_tmp = index_entry;
    char *curr_name_buff = NULL;

    convert_name_len = ((int32_t)ie_tmp->key.file_name.file_name_length + 1) * MB_CUR_MAX;
    curr_name_buff = (char *)calloc(1, convert_name_len);
    if (NULL == curr_name_buff) {
        return;
    }

    // Unicode转多字符
    temp_length = ntfs_ucstombs((ntfschar *)ie_tmp->key.file_name.file_name,
        (int32_t)ie_tmp->key.file_name.file_name_length, &curr_name_buff, convert_name_len);
    if (temp_length > 0) {
        if ((temp_length > 0) && (strcmp(curr_name_buff, ".") != 0) && (tmp_mft_no >= (uint64_t)FILE_first_user)) {
            AFS_TRACE_OUT_DBG("Current file name: %s, MFT = 0x%llx", curr_name_buff, (long long)tmp_mft_no);
            (void)mft_result.insert(mft_map::value_type(tmp_mft_no, ie_tmp->key.file_name.file_attributes));
        }
    }

    if (NULL != curr_name_buff) {
        free(curr_name_buff);
        curr_name_buff = NULL;
    }
}

/**
 * @brief 根据VCN号计算数据起始地址
 *
 * @param *data_runlist           索引数据流链表
 * @param runlist_element_num     索引数据流个数
 * @param index_block_size        索引块大小
 * @param index_vcn_size          VCN大小
 * @param index_block_vcn_id      VCN号
 * @param *index_block_buffer     索引块数据的开始位置
 *
 * @return  大于0 VCN号对应数据的开始位置
 * 其他  失败
 *
 */
int64_t ntfsUtility::ntfs_getIndexBlockByVCNPosition(const struct ntfs_runlist_element *data_runlist,
    uint32_t runlist_element_num, uint32_t index_block_size, uint32_t index_vcn_size, uint64_t index_block_vcn_id,
    uint8_t *index_block_buffer)
{
    uint8_t find_flg = 0;

    uint32_t vcn_index = 0;
    uint32_t cluster_size = m_ntfs_info.cluster_size;
    uint64_t tmp_vcn_id = 0; // 虚拟簇号
    int64_t read_len = 0;

    uint8_t *tmp_index_block_buffer = index_block_buffer;

    if ((NULL == data_runlist) || (0 == cluster_size) || (NULL == m_reader)) {
        AFS_TRACE_OUT_ERROR("Invalid data runlist or cluster size is zero.");
        return AFS_ERR_INNER;
    }

    // 索引块对应的数据流中的VCN ID号
    tmp_vcn_id = index_block_vcn_id * index_vcn_size;
    tmp_vcn_id /= cluster_size;

    // 查找虚拟簇号所在的数据流
    for (uint32_t i = 0; i < runlist_element_num; i++) {
        if (tmp_vcn_id < (uint64_t)(data_runlist[i].vcn + data_runlist[i].length)) {
            vcn_index = i;
            find_flg = 1;
            break;
        }
    }

    // 未找到虚拟簇号所在的数据流项
    if (!find_flg) {
        AFS_TRACE_OUT_ERROR("Can not find vcn position by RUNLIST. Index_Block_VCN=%lld, VCN ID=%lld",
            (long long)index_block_vcn_id, (long long)tmp_vcn_id);
        return AFS_ERR_INNER;
    }

    // 计算起始地址
    int64_t pos = data_runlist[vcn_index].lcn * cluster_size;
    // 跳过索引块开始位置之前的VCN
    pos += (((int64_t)(tmp_vcn_id)-data_runlist[vcn_index].vcn) * cluster_size);
    // 索引块偏移地址
    pos += (((int64_t)(index_block_vcn_id)*index_vcn_size) % cluster_size);

    // 根据pos读取索引块数据
    // 从数据流中读一个索引块数据
    read_len = m_reader->read(tmp_index_block_buffer, pos, (int64_t)index_block_size, 0);
    if (read_len != (int64_t)index_block_size) {
        AFS_TRACE_OUT_ERROR("Failed to read index block data.");
        return AFS_ERR_IMAGE_READ;
    }

    return 0;
}

/**
 * @brief 读取指定属性的实际数据
 *
 * @param *mft_buffer            MFT记录的数据
 * @param *attr_head             属性头
 * @param attr_start             属性开始位置
 * @param **data_buffer          返回读取到的数据
 * @param &data_size             返回读取到的数据长度
 *
 * @return  大于0 正常读取成功
 * 负数     失败
 *
 */
int32_t ntfsUtility::ntfs_readAttributeData(uint8_t *mft_buffer, ntfs_attr_record *attr_head, uint32_t attr_start,
    uint8_t **data_buffer, uint64_t &data_size)
{
    int32_t ret = 0;

    // 常驻时说明数据很小(一般不存在这种可能性)
    if (!(attr_head->non_resident)) {
        AFS_TRACE_OUT_DBG("Current file system bitmap data is resident attribute.");
        data_size = attr_head->data.resident.value_length;
        if (data_size == 0) {
            AFS_TRACE_OUT_ERROR("Invalid attr_head->data.resident.value_length");
            return AFS_ERR_INNER;
        }

        // 根据80H属性描述的信息，读取实际数据
        *data_buffer = (uint8_t *)calloc(1, data_size);
        if (NULL == *data_buffer) {
            AFS_TRACE_OUT_ERROR("Failed to allocate bitmap space.");
            return AFS_ERR_API;
        }
        // *data_buffer = (uint8_t*)calloc(1, data_size);是否有释放？
        CHECK_MEMCPY_S_OK(*data_buffer, data_size,
            (uint8_t *)(mft_buffer + attr_start + attr_head->data.resident.value_offset), data_size);
    } else {
        // 非常驻情况
        data_size = attr_head->data.non_resident.initialized_size;
        if (data_size == 0) {
            AFS_TRACE_OUT_ERROR("Invalid attr_head->data.non_resident.initialized_size");
            return AFS_ERR_INNER;
        }

        // 根据80H属性描述的信息，读取实际数据
        *data_buffer = (uint8_t *)calloc(1, data_size);
        if (NULL == *data_buffer) {
            AFS_TRACE_OUT_ERROR("Failed to allocate bitmap space.");
            return AFS_ERR_API;
        }

        // 根据属性描述的Data Size申请 Bitmap 空间
        ret = ntfs_getDataFromRunlist(mft_buffer, attr_head, attr_start, *data_buffer, data_size);
        if (ret != 0) {
            AFS_TRACE_OUT_ERROR("Get bitmap data failed.");
            return ret;
        }
    }

    return AFS_SUCCESS;
}
