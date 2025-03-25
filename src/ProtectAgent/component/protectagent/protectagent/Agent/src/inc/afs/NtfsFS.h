/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file ntfsfs.h
 * @brief Analyze NTFS file system.
 *
 */
#ifndef __AFS_NTFS_H__
#define __AFS_NTFS_H__

#include <vector>
#include "afs/FileSystem.h"
#include "afs/NtfsUtility.h"

using namespace std;

/**
 * @brief 分析NTFS文件系统类
 */
class ntfsHandler : public filesystemHandler {
public:
    /* *
     * @brief 构造函数
     */
    ntfsHandler()
    {
        setObjType(OBJ_TYPE_FILESYSTEM);
        setMagic("ntfs");
        setType((int32_t)AFS_FILESYSTEM_NTFS);
    }
    /* *
     * @brief 析构函数
     */
    ~ntfsHandler() {}

    /* *
     * @brief NTFS获得分区bitmap
     *
     * @param  &bitmap 输入bitmap用于返回
     * @return int32_t  0 成功
     * 负数   错误ID
     *
     */
    int32_t getBitmap(vector<BitMap *> &bitmap_vect);

    /* *
     * @brief 实现找出文件所占块的功能
     *
     * @param *file_path       文件/目录全路径
     * @param &bitmap          用于返回的bitmap
     *
     * @return   0： 成功
     * 负数： 错误ID
     */
    int32_t getFile(const char *file_path, BitMap &bitmap);

    static afsObject *CreateObject()
    {
        return new ntfsHandler();
    }
#ifdef CPPUNIT_MAIN
protected:
#else
private:
#endif
    string m_driver;

    /* *
     * @brief 初始化文件查找所依赖的数据
     *
     * @param *ntfs_util_obj 调用公共函数的对象指针
     *
     * @return AFS_SUCCESS 成功
     * 负数 对应错误ID
     *
     */
    int32_t ntfs_initSearch(ntfsUtility *ntfs_util_obj);

    /* *
     * @brief 读取NTFS分区的空闲块Bitmap
     *
     * @param *ntfs_util_obj           用公共函数调用指针
     * @param *ntfs_info               分区基本信息结构体
     * @param **bitmap_data_buffer     分区的Bitmap
     * @param &bitmap_size             分区Bitmap大小
     *
     * @return AFS_SUCCESS 成功
     * 负数 对应错误ID
     *
     */
    int32_t ntfs_getVolumeBitmap(ntfsUtility *ntfs_util_obj, ntfs_part_info *ntfs_info, uint8_t **bitmap_data_buffer,
        uint64_t &bitmap_size);

    /* *
     * @brief 转换分区Bitmap的位，处理为小端表示
     *
     * @param  *bitmap_data_buffer  文件系统本身的空闲块Bitmap
     * @param  bitmap_size          Bitmap每位代表大小
     * @param  cluster_size         文件系统簇大小
     * @param  &bitmap              返回处理后的Bitmap
     * @return  0: 成功
     * 负值: 失败(错误ID)
     *
     */
    int32_t ntfs_convertBitmap(
        const uint8_t *bitmap_data_buffer, uint64_t bitmap_size, uint32_t cluster_size, vector<BitMap *> &bitmap_vect);
    /* *
     * @brief 根据指定的查找路径，逐级对文件系统进行分析
     *
     * @param *ntfs_util_obj    调用公共函数的对象指针
     * @param &file_path_vec    文件路径对应的数组
     * @param *mft_no_result    查找的MFT记录号
     * @param *mft_attr         MFT属性值
     *
     * @return AFS_SUCCESS 成功
     * 负数 对应错误ID
     *
     */
    int32_t ntfs_searchFileByPath(ntfsUtility *ntfs_util_obj, vector<string> &file_path_vec, uint64_t *mft_no_result,
        uint32_t *mft_attr);
    /* *
     * @brief 转换路径为数组
     *
     * @param file_path     文件路径
     * @param &path_vector  路径转换到数组中
     *
     * @return void
     *
     */
    void ntfs_convertPathToVector(string file_path, vector<string> &path_vector);
    /* *
     * @brief 检查当前链接路径是否可以支持
     *
     * @param &file_path_str    符号链接目录
     * @param &is_absolute_path 是否是绝对路径
     *
     * @return AFS_SUCCESS  可支持的链接
     * 负数  不支持并返回错误ID
     *
     */
    int32_t ntfs_isSupportedPath(string &file_path_str, bool &is_absolute_path);
    /* *
     * @brief 处理重解析目录，包括相对路径的处理，最终转换为数组
     *
     * @param is_absolute_path   当前的重解析目录是否是绝对路径
     * @param vector_index       当前路径层级
     * @param start_index        开始层级
     * @param sylink_tmp         符号链接路径数组
     * @param &vector_path       返回解析后的路径对应的数组
     * @param &sylink_vec        返回链接路径数组
     *
     * @return AFS_SUCCESS  可支持的链接
     * 负数  不支持并返回错误ID
     *
     */
    int32_t ntfs_getReparseVector(bool is_absolute_path, size_t vector_index, int32_t start_index,
        vector<string> sylink_tmp, vector<string> &vector_path, vector<string> &sylink_vec);
    /* *
     * @brief 查找指定Unicode文件名的MFT记录号
     *
     * @param *ntfs_util_obj    调用公共函数的对象指针
     * @param *file_name        文件名
     * @param &mft_no_find      当前位置的MFT记录号
     * @param file_unicode_len  文件名对应的unicode长度
     * @param &file_path_vec    指定的文件名Path数组
     * @param &path_index       当前文件名的数组下标
     * @param *mft_no_result    返回查找到的MFT记录号
     * @param *mft_attr         MFT文件属性
     *
     * @return AFS_SUCCESS 成功
     * 负数 对应错误ID
     *
     */
    int32_t ntfs_searchOneFile(ntfsUtility *ntfs_util_obj, ntfschar *file_name, uint64_t &mft_no_find,
        int32_t file_unicode_len, vector<string> &file_path_vec, size_t &path_index, uint64_t *mft_no_result,
        uint32_t *mft_attr);

    /* *
     * @brief 处理符号链接对应的相对或绝对目录
     *
     * @param *file_path     链接目录
     * @param vector_index   当前目录层级
     * @param &vector_path   当前整个目录的数组
     * @param &sylink_vec    符号链接对应的目录数组
     *
     * @return AFS_SUCCESS  解析成功
     * 负数  解析失败
     *
     */
    int32_t ntfs_reparsePathToVector(const char *file_path, size_t vector_index, vector<string> &vector_path,
        vector<string> &sylink_vec);

    /* *
     * @brief 转换文件路径，去掉盘符以及将"\\"转换为"/"
     *
     * @param *file_path   过滤的文件路径
     * @param &path_vector 转换到数组中
     *
     * @return void
     *
     */
    void ntfs_convertPath(const char *file_path, vector<string> &path_vector);
};
#endif