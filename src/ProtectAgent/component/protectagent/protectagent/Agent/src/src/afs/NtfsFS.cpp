#include "afs/NtfsFS.h"
#include "afs/LogMsg.h"
#include "afs/AfsError.h"

using namespace std;

/**
 * @brief NTFS获得分区bitmap
 *
 * @param  &bitmap 输入bitmap用于返回
 * @return int32_t  0 成功
 * 负数   错误ID
 *
 */
int32_t ntfsHandler::getBitmap(vector<BitMap *> &bitmap_vect)
{
    AFS_TRACE_OUT_DBG("Enter get ntfs bitmap.");
    int32_t ret = 0;
    uint64_t bitmap_size = 0;
    ntfs_part_info ntfs_info; // NTFS 分区基本信息(扇区大小，簇大小等等)
    BitMap fsbitmap;          // 保存分区Bitmap
    uint8_t *bitmap_data_buffer = NULL;  // Bitmap 数据

    ntfsUtility ntfs_util_obj;
    // 分析分区信息
    ret = ntfs_util_obj.ntfs_initFSInfo(ntfs_info, getImgReader());
    if (0 != ret) {
        AFS_TRACE_OUT_ERROR("Failed to analyze file system information.");
        return ret;
    }

    // 获取MFT区域地址
    ret = ntfs_util_obj.ntfs_getMFTZoneRunlist();
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to read $MFT data zone.");
        goto out;
    }

    // 检查NTFS版本
    ret = ntfs_util_obj.ntfs_checkVersion();
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Unsupported NTFS file system. Ret=%d", ret);
        goto out;
    }

    // 读取卷的空闲块
    ret = ntfs_getVolumeBitmap(&ntfs_util_obj, &ntfs_info, &bitmap_data_buffer, bitmap_size);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to allocation MFT space.");
        goto out;
    }

    // 转换NTFS卷的Bitmap到镜像
    ret = ntfs_convertBitmap(bitmap_data_buffer, bitmap_size, ntfs_info.cluster_size, bitmap_vect);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to convert NTFS volume bitmap to interface.");
        goto out;
    }
    AFS_TRACE_OUT_DBG("Success to get NTFS free disk bitmap.");

out:
    // 释放内存
    if (NULL != bitmap_data_buffer) {
        free(bitmap_data_buffer);
        bitmap_data_buffer = NULL;
    }

    ntfs_util_obj.ntfs_freeMFTZoneSpace();
    return ret;
}

/**
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
int32_t ntfsHandler::ntfs_convertBitmap(
    const uint8_t *bitmap_data_buffer, uint64_t bitmap_size, uint32_t cluster_size, vector<BitMap *> &bitmap_vect)
{
    int32_t ret = 0;

    BitMap fsbitmap; // 保存分区Bitmap
    char *rbitmap = NULL;

    ret = fsbitmap.initBitMap(bitmap_size * NTFS_BIT_PER_BYTE);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to initialize bitmap.");
        return ret;
    }
    fsbitmap.bitmapSetBlocksize(cluster_size);
    rbitmap = fsbitmap.getbitmap();

    CHECK_MEMCPY_S_OK(rbitmap, fsbitmap.getsize(), bitmap_data_buffer, bitmap_size);

    unsigned char tc = 0;
    uint64_t bit_index = 0;
    // bitmap字节逆序
    for (bit_index = 0; bit_index < fsbitmap.getsize(); bit_index++) {
        tc = (unsigned char)rbitmap[bit_index];
        tc = (tc & 0xaa) >> 1 | (tc & 0x55) << 1;
        tc = (tc & 0xcc) >> 2 | (tc & 0x33) << 2;
        tc = (tc & 0xf0) >> 4 | (tc & 0x0f) << 4;
        rbitmap[bit_index] = (char)tc;
    }
    ///newadd 转换成一位代表512字节并复制到传入的bitmap进行返回
    ret = fsbitmap.bitmapConvert(m_reader, NTFS_BLOCK_SIZE, bitmap_vect);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Convert bitmap failed.");
        return AFS_ERR_INNER;
    }
    return AFS_SUCCESS;
}

/**
 * @brief 实现找出文件所占块的功能
 *
 * @param *file_path       文件/目录全路径
 * @param &bitmap          用于返回的bitmap
 *
 * @return   0： 成功
 * 负数： 错误ID
 */
int32_t ntfsHandler::getFile(const char *file_path, BitMap &bitmap)
{
    int32_t ret = 0;

    uint64_t mft_no_result = 0; // MFT查找结果
    uint32_t mft_attr = 0;      // MFT文件属性

    vector<string> file_path_vec;
    vector<string> sylink_path_vec;

    ntfs_part_info ntfs_info; // NTFS 分区基本信息(扇区大小，簇大小等等)
    ntfsUtility ntfs_util_obj;

    CHECK_MEMSET_S_OK(&ntfs_info, sizeof(ntfs_info), 0, sizeof(ntfs_info));

    // 解析文件路径
    ntfs_convertPath(file_path, file_path_vec);
    if (file_path_vec.size() == 0) {
        AFS_TRACE_OUT_ERROR("Failed to analyze path. path=%s", file_path);
        return AFS_ERR_PARA_PATH;
    }

    // 分析分区信息
    ret = ntfs_util_obj.ntfs_initFSInfo(ntfs_info, getImgReader());
    if (0 != ret) {
        AFS_TRACE_OUT_ERROR("Failed to analyze partition information. ret=%d", ret);
        return ret;
    }
    AFS_TRACE_OUT_DBG("Current partition cluster size = %d.", ntfs_info.cluster_size);

    // 申请MFT数据空间
    ret = ntfs_util_obj.ntfs_callocSearchSpace();
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to allocate MFT space.");
        goto out;
    }

    // 初始化NTFS分区参数
    ret = ntfs_initSearch(&ntfs_util_obj);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to initialize search condition. ret=%d", ret);
        goto out;
    }

    // 查找文件
    ret = ntfs_searchFileByPath(&ntfs_util_obj, file_path_vec, &mft_no_result, &mft_attr);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to search file. ret=%d", ret);
        goto out;
    }

    // 过滤目录或者文件
    ret = ntfs_util_obj.ntfs_getFileFilterBitmap(mft_no_result, mft_attr, bitmap);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to filter data. MFT=%llu, Attr=%d", (long long)mft_no_result, mft_attr);
        goto out;
    }

    AFS_TRACE_OUT_INFO("Success to filter file. %s", file_path);

out:
    ntfs_util_obj.ntfs_freeSearchSpace(); // 释放大小写表内存
    ntfs_util_obj.ntfs_freeMFTZoneSpace();

    return ret;
}

/**
 * @brief 转换路径为数组
 *
 * @param file_path     文件路径
 * @param &path_vector  路径转换到数组中
 *
 * @return void
 *
 */
void ntfsHandler::ntfs_convertPathToVector(string file_path, vector<string> &path_vector)
{
    size_t pos = 0;

    path_vector.clear();

    // "\\"转换为"/"
    pos = file_path.find("\\");
    while (pos != string::npos) {
        file_path.replace(pos, 1, "/");
        pos = file_path.find("\\");
    }

    // 将路径以数组方式返回
    size_t last = 0;
    size_t index = file_path.find_first_of("/", last);
    string tmp_str;
    // 将文件Path解析到数组中
    while (index != std::string::npos) {
        tmp_str = file_path.substr(last, index - last);
        if (!(tmp_str.empty())) {
            path_vector.push_back(tmp_str);
        }
        last = index + 1;
        index = file_path.find_first_of("/", last);
    }
    if (index - last > 0) {
        tmp_str = file_path.substr(last, index - last);
        if (!(tmp_str.empty())) {
            path_vector.push_back(tmp_str);
        }
    }
}

/**
 * @brief 转换文件路径，去掉盘符以及将"\\"转换为"/"
 *
 * @param *file_path   过滤的文件路径
 * @param &path_vector 转换到数组中
 *
 * @return void
 *
 */
void ntfsHandler::ntfs_convertPath(const char *file_path, vector<string> &path_vector)
{
    string file_path_str(file_path);

    // 去掉盘符(":"后开始)
    size_t pos = file_path_str.find(":");
    if (pos != string::npos) {
        m_driver = file_path_str.substr(0, pos);
        file_path_str = file_path_str.substr(pos + 1, file_path_str.length() - 1);
    }

    ntfs_convertPathToVector(file_path_str, path_vector);
}

/**
 * @brief 检查当前链接路径是否可以支持
 *
 * @param &file_path_str    符号链接目录
 * @param &is_absolute_path 是否是绝对路径
 *
 * @return AFS_SUCCESS  可支持的链接
 * 负数  不支持并返回错误ID
 *
 */
int32_t ntfsHandler::ntfs_isSupportedPath(string &file_path_str, bool &is_absolute_path)
{
    string tmp_driver;

    is_absolute_path = false;

    // 去掉盘符(":"后开始)
    size_t pos = file_path_str.find(":");
    if (pos != string::npos) {
        tmp_driver = file_path_str.substr(0, pos);
        if (strcasecmp(tmp_driver.c_str(), m_driver.c_str())) {
            AFS_TRACE_OUT_ERROR("Current driver is %s. link driver=%s", tmp_driver.c_str(), file_path_str.c_str());
            return AFS_ERR_FILE_SYMLINKS; // 非同分区符号链接
        }
        file_path_str = file_path_str.substr(pos + 1, file_path_str.length() - 1);
        is_absolute_path = true;
    }

    // 是否跨分区
    if (0 == file_path_str.compare(0, 7, NTFS_REPARSE_VOLUME, 0, 7)) {
        AFS_TRACE_OUT_ERROR("Current directory is reparsed to other volume. Path=%s", file_path_str.c_str());
        return AFS_ERR_FILE_SYMLINKS; // 非同分区符号链接
    }

    return AFS_SUCCESS;
}

/**
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
int32_t ntfsHandler::ntfs_getReparseVector(bool is_absolute_path, size_t vector_index, int32_t start_index,
    vector<string> sylink_tmp, vector<string> &vector_path, vector<string> &sylink_vec)
{
    // 记录符号链接之前的目录
    if (!is_absolute_path && (start_index > 0)) {
        for (int32_t tmp = 0; tmp < start_index; tmp++) {
            sylink_vec.push_back(vector_path[tmp]);
        }
    }

    // 处理相对目录
    for (size_t tmp = 0; tmp < sylink_tmp.size(); tmp++) {
        sylink_vec.push_back(sylink_tmp[tmp]);
    }

    // 记录符号链接后的文件
    for (size_t index = vector_index + 1; index < vector_path.size(); index++) {
        sylink_vec.push_back(vector_path[index]);
    }

    if (sylink_vec.size() == 0) {
        AFS_TRACE_OUT_ERROR("Symbol link path vector is 0.");
        return AFS_ERR_INNER;
    }

    return AFS_SUCCESS;
}

/**
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
int32_t ntfsHandler::ntfs_reparsePathToVector(const char *file_path, size_t vector_index, vector<string> &vector_path,
    vector<string> &sylink_vec)
{
    int32_t ret = 0;
    string file_path_str = file_path;
    vector<string> sylink_tmp;
    bool is_absolute_path = false;

    // 是否是可支持的符号链接
    ret = ntfs_isSupportedPath(file_path_str, is_absolute_path);
    if (ret != 0) {
        return ret;
    }

    vector<string> dispose_vec;
    int32_t start_index = vector_index;
    // 将Path转换到数组中
    ntfs_convertPathToVector(file_path_str, dispose_vec);

    for (size_t tmp = 0; tmp < dispose_vec.size(); tmp++) {
        if (!strcmp(dispose_vec[tmp].c_str(), "..")) {
            start_index--;
            continue;
        } else if (!strcmp(dispose_vec[tmp].c_str(), ".")) {
            continue;
        }
        sylink_tmp.push_back(dispose_vec[tmp]);
    }

    if (start_index < 0) {
        AFS_TRACE_OUT_ERROR("Invalid symlink path. Path=%s", file_path_str.c_str());
        return AFS_ERR_INNER;
    }

    ret = ntfs_getReparseVector(is_absolute_path, vector_index, start_index, sylink_tmp, vector_path, sylink_vec);

    return ret;
}

/**
 * @brief 初始化文件查找所依赖的数据
 *
 * @param *ntfs_util_obj 调用公共函数的对象指针
 *
 * @return AFS_SUCCESS 成功
 * 负数 对应错误ID
 *
 */
int32_t ntfsHandler::ntfs_initSearch(ntfsUtility *ntfs_util_obj)
{
    int32_t ret = 0;

    const char *locale = NULL; // 记录设定本地环境的返回值
    locale = setlocale(LC_ALL, "");
    if (!locale) {
        locale = setlocale(LC_ALL, NULL);
    }

    // 获取MFT区域地址
    ret = ntfs_util_obj->ntfs_getMFTZoneRunlist();
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to read $MFT data zone.");
        return ret;
    }

    // 检查NTFS版本(可支持的版本为3.1)
    ret = ntfs_util_obj->ntfs_checkVersion();
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Unsupported NTFS file system. ret=%d", ret);
        return ret;
    }

    // 从第10号MFT(FILE_UpCase)读取大小写信息表
    ret = ntfs_util_obj->ntfs_getVolumeUpcaseData();
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to read FILE_UpCase data.");
        return ret;
    }

    return AFS_SUCCESS;
}

/**
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
int32_t ntfsHandler::ntfs_getVolumeBitmap(ntfsUtility *ntfs_util_obj, ntfs_part_info *ntfs_info,
    uint8_t **bitmap_data_buffer, uint64_t &bitmap_size)
{
    int32_t ret = 0;

    uint8_t *mft_buffer = NULL; // MFT数据
    uint32_t attr_start = 0;    // 0x80属性头起始偏移
    ntfs_attr_record attr_head; // bitmap 属性头

    // 申请一个MFT记录的空间
    mft_buffer = (uint8_t *)calloc(1, ntfs_info->mft_size);
    if (NULL == mft_buffer) {
        AFS_TRACE_OUT_ERROR("Failed to allocation MFT space.");
        return AFS_ERR_API;
    }

    // 读取Bitmap的MFT记录数据
    ret = ntfs_util_obj->ntfs_getMFTData((uint64_t)(FILE_Bitmap), mft_buffer);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to read bitmap MFT data(FILE_Bitmap).");
        goto out;
    }

    // 读取 Bitmap MFT的80H属性
    CHECK_MEMSET_S_OK(&attr_head, sizeof(attr_head), 0, sizeof(attr_head));
    ret = ntfs_util_obj->ntfs_getMFTAttr(mft_buffer, (uint32_t)(AT_DATA), &attr_head, &attr_start);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("ntfsHandler::get_partition_bitmap() Failed to read 80H attribute by MFT.");
        goto out;
    }

    // 读取分区的Bitmap数据
    ret = ntfs_util_obj->ntfs_readAttributeData(mft_buffer, &attr_head, attr_start, bitmap_data_buffer, bitmap_size);

out:
    if (NULL != mft_buffer) {
        free(mft_buffer);
        mft_buffer = NULL;
    }

    return ret;
}

/**
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
int32_t ntfsHandler::ntfs_searchOneFile(ntfsUtility *ntfs_util_obj, ntfschar *file_name, uint64_t &mft_no_find,
    int32_t file_unicode_len, vector<string> &file_path_vec, size_t &path_index, uint64_t *mft_no_result,
    uint32_t *mft_attr)
{
    int32_t ret = 0;

    ntfschar *file_unicode = file_name;
    char reparse_file[NTFS_MAX_PATH] = {0};

    vector<string> sylink_path_vec;

    if ((0 == file_path_vec.size()) || (file_path_vec.size() <= path_index)) {
        AFS_TRACE_OUT_ERROR("Invalid file path vector size or path index.");
        return AFS_ERR_INNER;
    }

    // 查找MFT记录号
    ret = ntfs_util_obj->ntfs_searchMFTByName(mft_no_find, file_unicode, file_unicode_len, *mft_no_result, *mft_attr);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Search file \"%s\" failed. ret=%d", file_path_vec[path_index].c_str(), ret);
        return ret;
    }
    AFS_TRACE_OUT_DBG("Success to find %s. MFT=0x%llx", file_path_vec[path_index].c_str(), (long long)*mft_no_result);

    // 检查当前目录是否是符号链接
    // ret=1代表是符号链接，并将链接目录保存在reparse_file中
    // ret=0代表不是符号链接，可继续查找下一级目录/文件
    ret = ntfs_util_obj->ntfs_analyzeReparsePath(*mft_no_result, reparse_file);
    if (ret < 0) {
        AFS_TRACE_OUT_ERROR("Failed to check current file re-parse property. MFT=0x%llx", (long long)*mft_no_result);
        return ret;
    }
    // 正常文件/目录
    if (0 == ret) {
        path_index++;
        mft_no_find = *mft_no_result;
        return AFS_SUCCESS;
    }

    // 执行到此处，说明当前MFT是一个符号链接，处理符号链接
    AFS_TRACE_OUT_INFO("Current file is re-parse file. file=%s, path=%s", file_path_vec[path_index].c_str(),
        reparse_file);
    ret = ntfs_reparsePathToVector(reparse_file, path_index, file_path_vec, sylink_path_vec);
    // 跨分区，不支持
    if (ret < 0) {
        AFS_TRACE_OUT_ERROR("Re-parse file point to different partition. (Unsupported path=%s)", reparse_file);
        return ret;
    }

    file_path_vec.clear();
    file_path_vec.swap(sylink_path_vec); // 将sylink_path_vec数据赋值到file_path_vec

    // 符号链接时从根目录重新查找
    mft_no_find = (uint64_t)FILE_root; // ROOT的MFT记录号
    path_index = 0;

    return AFS_SUCCESS;
}

/**
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
int32_t ntfsHandler::ntfs_searchFileByPath(ntfsUtility *ntfs_util_obj, vector<string> &file_path_vec,
    uint64_t *mft_no_result, uint32_t *mft_attr)
{
    int32_t ret = 0;

    int32_t file_unicode_len = 0;

    size_t path_index = 0;

    uint32_t file_length = 0;
    uint64_t mft_no_find = 0; // 查找的MFT记录号

    vector<string> sylink_path_vec;

    ntfschar *file_unicode = NULL;

    file_length = (uint32_t)(sizeof(ntfschar) * NTFS_MAX_NAME_LEN);
    // 申请文件名转换为Unicode字符的最大空间
    file_unicode = (ntfschar *)calloc(1, file_length);
    if (NULL == file_unicode) {
        AFS_TRACE_OUT_ERROR("Failed to allocate file name memory.");
        return AFS_ERR_API;
    }

    mft_no_find = (uint64_t)FILE_root; // ROOT的MFT记录号（从根目录开始查找）
    // 开始从根目录的MFT(第5号记录)遍历指定的路径
    for (path_index = 0; path_index < file_path_vec.size();) {
        AFS_TRACE_OUT_DBG("Start to search file %s by MFT(0x%llx)", file_path_vec[path_index].c_str(),
            (long long)mft_no_find);

        // 将查找的文件名转换为Unicode字符
        CHECK_MEMSET_S_OK(file_unicode, file_length, 0, file_length);
        file_unicode_len =
            ntfs_util_obj->ntfs_mbstoucs(file_path_vec[path_index].c_str(), file_unicode, (int32_t)file_length);
        if (file_unicode_len <= 0) {
            AFS_TRACE_OUT_ERROR("Failed to convert multibyte char to unicode.");
            ret = AFS_ERR_INNER; // 转换为 Unicode 字符失败
            goto out;
        }

        *mft_no_result = 0;
        *mft_attr = 0;
        ret = ntfs_searchOneFile(ntfs_util_obj, file_unicode, mft_no_find, file_unicode_len, file_path_vec, path_index,
            mft_no_result, mft_attr);
        if (ret != AFS_SUCCESS) {
            goto out;
        }
    }

out:
    if (NULL != file_unicode) {
        free(file_unicode);
        file_unicode = NULL;
    }

    return ret;
}
