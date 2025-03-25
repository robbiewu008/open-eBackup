/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file afslibmain.cpp
 *
 * @brief Main program for analyze format of a disk image.
 *
 */
#include "afs/Afslibmain.h"
#include <stdio.h>
#include <cstdlib>
#include "afs/RawReader.h"
#include "afs/PartitionHandler.h"
#include "afs/FileSystem.h"
#include "afs/Ext4FS.h"
#include "afs/NtfsFS.h"
#include "afs/XfsFS.h"
#include "afs/FileSystemFactory.h"
#include "afs/AfsLVM.h"

#ifndef CPPUNIT_MAIN
#define CPPUNIT_STATIC static
#else
#define CPPUNIT_STATIC
#endif

/**
 * @brief 设置镜像信息
 *
 * @param handle             回调函数参数(镜像文件指针)
 * @param read_callback_func 回调函数
 * @param *img_info_obj      镜像信息
 *
 * @return :返回镜像句柄
 */
CPPUNIT_STATIC int32 setImgageInfo(AFS_HANDLE handle, AFS_READ_CALLBACK_FUNC_t read_callback_func,
    struct imgInfo *img_info_obj)
{
    int64 read_size = 0;
    CHECK_MEMSET_S_OK(img_info_obj, sizeof(struct imgInfo), 0, sizeof(struct imgInfo));
    img_info_obj->Handle = handle;
    img_info_obj->read_call_back_func = read_callback_func;

    // 通过回调函数获取整个镜像大小
    read_size = read_callback_func(handle, NULL, -1, 0);
    if (read_size <= 0) {
        AFS_TRACE_OUT_ERROR("Failed to read image size by CALL_BACK function. return value = %lld",
            (long long)read_size);
        return AFS_ERR_IMAGE_READ;
    }
    img_info_obj->imageSize = read_size;

    return AFS_SUCCESS;
}

/**
 * @brief 初始化bitmap函数
 *
 * @param *img_info_obj image文件的信息
 * @param &bitmap       需要初始化的bitmap
 * @return 0：设置成功    负数：设置失败
 *
 */
CPPUNIT_STATIC int32 afs_initBitmap(struct imgInfo *img_info_obj, BitMap &bitmap)
{
    int32 ret = AFS_SUCCESS;

    ret = bitmap.initBitMap(img_info_obj->imageSize / SECTOR_SIZE);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to initialize bitmap buffer.");
        return AFS_ERR_API;
    }

    bitmap.bitmapSetBlocksize(SECTOR_SIZE);
    bitmap.setImageSize(img_info_obj->imageSize);
    AFS_TRACE_OUT_DBG("image size of bitmap is %llu(bytes)", (unsigned long long)bitmap.getImageSize());

    return ret;
}

/**
 * @brief 获取文件类型
 *
 * @param *img_reader  镜像读取Reader
 * @param *fs_type     返回文件系统类型
 * @return    AFS_SUCCESS： 成功获取文件系统类型
 * AFS_ERR_API： API错误
 * AFS_ERR_IMAGE_READ: 镜像读取错误
 * AFS_ERR_FS_SUPPORT： 不支持的文件系统
 */
CPPUNIT_STATIC int32 afs_getFSType(imgReader *img_reader, int32 *fs_type)
{
    int32 ret = AFS_SUCCESS;

    filesystemHandler fshandler;
    fshandler.setImageReader(img_reader);
    ret = fshandler.getFSTypeUUID(fs_type, NULL, 0);

    return ret;
}

/**
 * @brief 解析swap分区
 *
 * @param *img_reader 镜像读取Reader指针
 * @param *ppart      分区信息
 * @param *bitmap     bitmap空间
 * @return 0 swap分区
 * 1 其他分区
 * 负值：错误
 *
 */
CPPUNIT_STATIC int32 afs_parseSwapSpace(imgReader *img_reader, struct partition *ppart, vector<BitMap *> &bitmap_vect)
{
    int32 ret = 0;
    uint64_t start = 0;
    BitMap *pbitmap = NULL;

    // 幻数判断
    if (AFS_FILESYSTEM_SWAP == ppart->fstype) {
        pbitmap = bitmap_vect[ppart->disk_id];  // 暂取ppart->disk_id 所属的磁盘位图
        ret = pbitmap->bitmapSetRangeSwapMapAddr(
            img_reader, start, 8, 1, bitmap_vect);  // 只有分区的前8个扇区才是有效数据
        if (ret != 0) {
            AFS_TRACE_OUT_ERROR("Failed to set swap bitmap.");
            return AFS_ERR_INNER;
        }
        AFS_TRACE_OUT_INFO("swap partition.");
        return AFS_SWAP_PART;
    }

    return AFS_NORMAL_PART; // 非交换分区
}

/**
 * @brief 获得磁盘分区空闲块的bitmap
 * @param *img_reader        设置imgReader
 * @param &bitmap_vect       所有磁盘bitmap
 * @param &disk_reader_vect  所有磁盘rawReader
 * @param fsType             文件系统类型
 * @return   0：成功
 * 负数：错误ID
 */
CPPUNIT_STATIC int32 afs_getPartBitmap(imgReader *img_reader, vector<BitMap *> &bitmap_vect, int32 fs_type)
{
    int32 ret = AFS_SUCCESS;
    filesystemFactory fsfactory;
    filesystemHandler *fsrealhandler = fsfactory.createObject(fs_type);
    if (NULL == fsrealhandler) {
        AFS_TRACE_OUT_ERROR("Can not supported file system type. Type = %d", fs_type);
        return AFS_ERR_API;
    }

    fsrealhandler->setImageReader(img_reader);
    ret = fsrealhandler->getBitmap(bitmap_vect);
    delete fsrealhandler;

    return ret;
}

/**
 * @brief 设置非LVM单个分区的BitMap
 *
 * @param *img_reader    镜像句柄
 * @param *ppart         分区信息
 * @param & bitmap_vect  所有磁盘的位图
 * @Parma & disk_reader_vect  所有磁盘的rawReader
 *
 * @return : 0：成功   负数：失败
 */
CPPUNIT_STATIC int32 afs_getSinglePartBitMap(imgReader *img_reader, struct partition *ppart,
    vector<BitMap *> &bitmap_vect)
{
    int32 ret = true;
    int32 fs_type;

    // 获得分区文件系统类型
    ret = afs_getFSType(img_reader, &fs_type);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to analyze file system type.");
        return ret;
    }
    ppart->fstype = (AFS_FSTYPE_t)fs_type;
    AFS_TRACE_OUT_DBG("partition filesystem is %d", fs_type);

    // 解析swap分区
    ret = afs_parseSwapSpace(img_reader, ppart, bitmap_vect);
    if (ret <= 0) {
        // 0：交换分区 负值:错误
        return ret;
    }

    // 根据文件系统类型分析空闲块
    ret = afs_getPartBitmap(img_reader, bitmap_vect, (int32)(ppart->fstype));
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to get partition bitmap. fsType = %d", ppart->fstype);
        return ret;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 获得文件的bitmap
 *
 * @param img_reader    设置imgReader
 * @param &bitmap       设置记录分区bitmap
 * @param fsType        设置文件系统类型
 * @param *file_path    过滤指定的路径
 *
 * @return : 0：成功  负数：失败
 *    */
CPPUNIT_STATIC int32 afs_getFilePosition(imgReader *img_reader, vector<BitMap *> &bitmap_vect, int32 fsType,
    const char *file_path)
{
    int32 ret = 0;
    filesystemFactory fsfactory;

    // 根据文件系统类型构造对象
    filesystemHandler *fshandler = fsfactory.createObject(fsType);
    if (NULL == fshandler) {
        AFS_TRACE_OUT_ERROR("Failed to create file system object by type = %d. ", fsType);
        return AFS_ERR_INNER;
    }

    fshandler->setImageReader(img_reader);
    // 过滤指定文件/目录
    ret = fshandler->getFile(file_path, bitmap_vect);

    // 内存释放
    delete fshandler;
    return ret;
}

/**
 * @brief 将代表512字节的Bitmap数据根据指定大小进行转换
 *
 * @param data_buffer       转换后的Bitmap数据
 * @param buffer_size       Bitmap的数据大小
 * @param img_size          镜像大小(字节)
 * @param &bitmap           位代表512字节的Bitmap
 * @param bytes_per_bit     转换后的位代表大小
 * @param filt_buffer_flag  转换类型(0:空闲块，1:文件过滤)
 *
 * @return :  0:成功    负数:错误码
 *    */
CPPUNIT_STATIC int32 afs_convertBitmap(char *data_buffer, int64 buffer_size, int64 img_size, BitMap &bitmap,
    int32 bytes_per_bit, int32 filt_buffer_flag)
{
    int32 ret = 0;
    int64 copy_size = 0;
    BitMap bitmap_tmp;

    ret = bitmap_tmp.initBitMap((img_size + bytes_per_bit - 1) / bytes_per_bit);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to initialize bitmap. BlockNumber=%lld",
            (long long)(img_size + bytes_per_bit - 1) / bytes_per_bit);
        return ret;
    }

    bitmap_tmp.bitmapSetBlocksize(bytes_per_bit);

    if (filt_buffer_flag) {
        ret = bitmap.bitmapConvertFile(bytes_per_bit, bitmap_tmp);
    } else {
        ret = bitmap.bitmapConvert(bytes_per_bit, bitmap_tmp);
    }

    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to convert bitmap");
        return AFS_ERR_INNER;
    }

    // 把bitmap中的值复制到buffer中
    copy_size = bitmap_tmp.getsize() > (uint64_t)buffer_size ? (uint64_t)buffer_size : bitmap_tmp.getsize();
    AFS_TRACE_OUT_DBG("copy_size is %lld(bytes), buffer_size is %llu(bytes)", (long long)copy_size,
        (unsigned long long)buffer_size);
    CHECK_MEMCPY_S_OK(data_buffer, copy_size, (const char *)bitmap_tmp.getbitmap(), copy_size);
    return AFS_SUCCESS;
}

/**
 * @brief 处理单独disk
 * @param partsHandler        全局 partitionHandler 变量
 * @param imgHandle           disk镜像文件指针
 * @param read_callback_func  回调函数
 * @return :  0     处理成功
 * 其他     错误ID
 */

CPPUNIT_STATIC int32 gpiGetEachDiskPartNum(partitionHandler &partsHandler, AFS_HANDLE imgHandle,
    AFS_READ_CALLBACK_FUNC_t read_callback_func, int32_t disk_num)
{
    int32_t ret = 0;
    if (imgHandle == NULL) {
        AFS_TRACE_OUT_ERROR("The AFS_HANDLE parameter of disk[%u] is not valid.", disk_num);
        return AFS_ERR_PARAMETER;
    }

    struct imgInfo img_info_obj;
    ret = setImgageInfo(imgHandle, read_callback_func, &img_info_obj);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to read image size by call back function.");
        return ret;
    }

    // 构造镜像Reader
    imgReader *reader_img = new rawReader();
    reader_img->initImgReader(&img_info_obj, 0, img_info_obj.imageSize / SECTOR_SIZE);
    partsHandler.setImgReader(reader_img);

    partsHandler.setDiskNumValue(disk_num);
    AFS_TRACE_OUT_DBG("partsHandler.m_disknum is %d, disk rawReader is %02x", partsHandler.getDiskNum(), reader_img);

    ret = partsHandler.analyzePartitions();
    if (AFS_SUCCESS != ret) {
        delete reader_img;
        AFS_TRACE_OUT_ERROR("Failed to analyze partition information.");
        return ret;
    }

    AFS_TRACE_OUT_INFO("ret = %d, finish parsing the [%u] disk", ret, disk_num);
    partsHandler.pushImgReader(reader_img);

    return ret;
}

/**
 * @brief 获得多个磁盘的总分区数量
 * @param img_handles         多磁盘镜像文件指针
 * @param read_callback_func  回调函数
 * @return :  大于0 分区数量
 * 其他     错误ID
 */
int32 getMultipleDisksPartsNum(vector<AFS_HANDLE> img_handles, AFS_READ_CALLBACK_FUNC_t read_callback_func)
{
    int32_t ret = 0;
    int32_t part_num = 0;
    int32_t disk_num = 0;
    AFS_HANDLE imgHandle = NULL;
    partitionHandler partsHandler;

    if (read_callback_func == NULL) {
        AFS_TRACE_OUT_ERROR("The read_callback_func parameter is not valid.");
        return AFS_ERR_PARAMETER;
    }

    for (uint32_t i = 0; i < img_handles.size(); i++) {
        imgHandle = img_handles.at(i);
        ret = gpiGetEachDiskPartNum(partsHandler, imgHandle, read_callback_func, disk_num);
        if (ret != 0) {
            AFS_TRACE_OUT_ERROR("gpiGetEachDiskPartNum failed");
            return ret;
        }
        disk_num++;
    }

    ret = partsHandler.updateAllPartitions();
    if (ret != AFS_SUCCESS) {
        return AFS_ERR_INNER;
    }

    part_num = partsHandler.getPartnum();
    if (part_num <= 0) {
        AFS_TRACE_OUT_ERROR("Failed to get partition count.");
        return AFS_ERR_INNER;
    }

    AFS_TRACE_OUT_INFO("finish geting all partitions");
    return part_num;
}

/**
 * @brief 获得磁盘分区数量
 * @param handle              回调函数参数(镜像文件指针)
 * @param read_callback_func  回调函数
 * @return :  大于0 分区数量
 * 其他     错误ID
 */
int32 getPartNum(AFS_HANDLE handle, AFS_READ_CALLBACK_FUNC_t read_callback_func)
{
    int32_t ret = 0;
    int32_t part_num = 0;

    // 参数检查
    if ((NULL == handle) || (NULL == read_callback_func)) {
        AFS_TRACE_OUT_ERROR("The specified parameter is not valid.");
        return AFS_ERR_PARAMETER;
    }

    struct imgInfo img_info_obj;
    ret = setImgageInfo(handle, read_callback_func, &img_info_obj);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to read image size by call back function.");
        return ret;
    }

    // 构造镜像Reader
    rawReader raw_reader;
    imgReader *reader_img = &raw_reader;
    reader_img->initImgReader(&img_info_obj, 0, img_info_obj.imageSize / SECTOR_SIZE);

    // 处理分区
    partitionHandler handler;
    handler.setImgReader(reader_img);
    ret = handler.analyzePartitions();
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Failed to analyze partition information.");
        return ret;
    }

    part_num = handler.getPartnum();
    if (part_num <= 0) {
        AFS_TRACE_OUT_ERROR("Failed to get partition count.");
        return AFS_ERR_INNER;
    }

    return part_num;
}

/**
 * @brief 解析reader参数
 *
 * @param &input_para    输入数据
 * @param &img_info_obj  传出数据
 * @return
 * AFS_ERR_PARAMETER
 * AFS_SUCCESS
 */
static int32 gpiInitReaderParameter(input_para_space_t &input_para, struct imgInfo &img_info_obj)
{
    int ret = 0;
    // 参数检查
    if ((NULL == input_para.handle) || (NULL == input_para.read_callback_func) || (NULL == input_para.part_info) ||
        (input_para.input_part_num <= 0)) {
        AFS_TRACE_OUT_ERROR("The specified parameter is not valid.");
        return AFS_ERR_PARAMETER;
    }

    ret = setImgageInfo(input_para.handle, input_para.read_callback_func, &img_info_obj);
    if (ret != AFS_SUCCESS) {
        // 读取镜像大小失败
        AFS_TRACE_OUT_ERROR("Failed to read image size by call back function.");
        return ret;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 获取真正数量分区数
 *
 * @param &handler 分区句柄
 * @param input_part_num 分区数量
 *
 * @return
 * AFS_ERR_INNER
 * real_part_num
 */
CPPUNIT_STATIC int32 gpiGetRealPartNumber(partitionHandler &handler, int32 input_part_num)
{
    int ret = 0;
    // 分析镜像分区
    ret = handler.analyzePartitions();
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Failed to analyze image partition.");
        return ret;
    }

    // /真实分区数量
    int part_num = 0;
    int real_part_num = 0;
    part_num = handler.getrealHandler()->getPartnum();
    if (part_num <= 0) {
        AFS_TRACE_OUT_ERROR("Failed to get partition count.");
        return AFS_ERR_INNER;
    }
    real_part_num = input_part_num > part_num ? part_num : input_part_num;

    return real_part_num;
}

/**
 * @brief 获取所有的分区信息
 * @param *reader       句柄
 * @param &input_para   输入参数
 * @param &handler      分区句柄
 * @param real_part_num 分区数量
 */
CPPUNIT_STATIC int32 gpiDoAllOfPartInfo(imgReader *reader, input_para_space_t &input_para, partitionHandler &handler,
    int real_part_num)
{
    struct partition *part_info = reinterpret_cast<struct partition *>(input_para.part_info);
    int32 fs_type = 0;
    int32 ret = 0;
    AFS_TRACE_OUT_DBG("real_part_num is %d", real_part_num);
    // 遍历取得每个分区的文件系统类型
    for (int i = 0; i < real_part_num; i++) {
        imgReader part_reader(reader);

        // 初始化分区类型
        CHECK_MEMSET_S_OK(&(part_info[i]), sizeof(struct partition), 0, sizeof(struct partition));
        part_info[i].fstype = AFS_FILESYSTEM_NULL;

        ret = handler.getrealHandler()->getPartition(i, &(part_info)[i]);
        if (0 != ret) {
            AFS_TRACE_OUT_ERROR("Failed to read partition information. Index=%d", i);
            continue;
        }

        AFS_TRACE_OUT_DBG("Parts[%d], free disk part chunk size = %d", i, handler.m_map_chunk_size[i]);
        ret = part_reader.initImgReader(handler.getrealHandler()->getRealPartReader(), &(part_info)[i],
            handler.m_map_chunk_size[i]);
        if (0 != ret) {
            AFS_TRACE_OUT_ERROR("Failed to initialize partition. Index=%d", i);
            continue;
        }

        fs_type = (int32)(part_info[i].fstype);
        if ((int32)AFS_FILESYSTEM_SWAP == fs_type) {
            continue;
        }

        ret = afs_getFSType(&part_reader, &fs_type);
        if (AFS_SUCCESS == ret) {
            part_info[i].fstype = (AFS_FSTYPE_t)fs_type;
        } else {
            return ret;
        }
    }
    return AFS_SUCCESS;
}

/**
 * @brief 获取多磁盘所有分区信息
 * @param *reader       句柄
 * @param &input_para   输入参数
 * @param &handler      分区句柄
 * @param real_part_num 分区数量
 */
CPPUNIT_STATIC int32 gpiDoMultipleDiskAllPartInfo(input_para_space_t &input_para, partitionHandler &partsHandler,
    int32_t real_part_num)
{
    struct partition *part_info = reinterpret_cast<struct partition *>(input_para.part_info);
    int32 fs_type = 0;
    int ret = 0;
    vector<imgReader *> &image_reader_vec = partsHandler.getImgReaderVector();
    int32_t part_diskId;

    // 遍历取得每个分区的文件系统类型
    for (int32_t i = 0; i < real_part_num; i++) {
        // 初始化分区类型
        CHECK_MEMSET_S_OK(&(part_info[i]), sizeof(struct partition), 0, sizeof(struct partition));
        part_info[i].fstype = AFS_FILESYSTEM_NULL;

        ret = partsHandler.getPartition(i, &(part_info)[i]);
        if (0 != ret) {
            AFS_TRACE_OUT_ERROR("Failed to read partition information. Index=%d", i);
            continue;
        }

        part_diskId = part_info[i].disk_id;
        AFS_TRACE_OUT_DBG("part_info[%d] disk_Id is %d", i, part_diskId);
        partsHandler.setImgReader(image_reader_vec[part_diskId]);

        ret = partsHandler.createPartitionReader(&part_info[i]);
        if (0 != ret) {
            AFS_TRACE_OUT_ERROR("Failed to create partition reader. Index=%d", i);
            continue;
        }

        imgReader part_reader(image_reader_vec[part_diskId]);
        AFS_TRACE_OUT_DBG("Parts[%d], free disk part chunk size = %d", i, partsHandler.m_map_chunk_size[i]);

        ret = part_reader.initImgReader(partsHandler.getRealPartReader(), &(part_info)[i],
            partsHandler.m_map_chunk_size[i]);
        if (0 != ret) {
            AFS_TRACE_OUT_ERROR("Failed to initialize partition[%d], ret = %d", i, ret);
            continue;
        }

        ret = afs_getFSType(&part_reader, &fs_type);
        if (AFS_SUCCESS == ret) {
            part_info[i].fstype = (AFS_FSTYPE_t)fs_type;
        } else {
            AFS_TRACE_OUT_ERROR("partition[%d] get file system failed, ret = %d", i, ret);
            continue;
        }
    }
    return AFS_SUCCESS;
}

/**
 * @brief 获得多磁盘分区信息及单个分区文件系统类型
 *
 * @param img_handles          多磁盘镜像文件指针
 * @param read_callback_func   回调函数
 * @param *part_info           设置返回分区信息的结构体指针
 * @param input_part_num       设置需要获得分区信息的数量
 *
 * @return 大于0： 实际返回分区数量
 * 负数： 错误ID
 *    */
int32 getMultipleDiskPartInfo(vector<AFS_HANDLE> img_handles, AFS_READ_CALLBACK_FUNC_t read_callback_func,
    struct partition *part_info, int32 input_part_num)
{
    int32_t ret = 0;
    int32_t part_num = 0;
    int32_t real_part_num = 0;
    int32_t disk_num = 0;
    partitionHandler partsHandler;

    input_para_space_t input_para = {
        NULL, read_callback_func, NULL, input_part_num, reinterpret_cast<char *>(part_info), 0, 0, NULL, 0
    };

    if (read_callback_func == NULL) {
        AFS_TRACE_OUT_ERROR("The read_callback_func parameter is not valid.");
        return AFS_ERR_PARAMETER;
    }

    for (uint32_t i = 0; i < img_handles.size(); i++) {
        input_para.handle = img_handles.at(i);
        AFS_TRACE_OUT_DBG("input_para.handle addr is %02x", input_para.handle);

        ret = gpiGetEachDiskPartNum(partsHandler, input_para.handle, read_callback_func, disk_num);
        if (ret != 0) {
            AFS_TRACE_OUT_ERROR("gpiGetEachDiskPartNum for parts info failed");
            return ret;
        }

        disk_num++;
    }

    ret = partsHandler.updateAllPartitions();
    if (ret != AFS_SUCCESS) {
        return ret;
    }

    part_num = partsHandler.getPartnum();
    real_part_num = input_part_num > part_num ? part_num : input_part_num;
    ret = gpiDoMultipleDiskAllPartInfo(input_para, partsHandler, real_part_num);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("failed to get all partitions info for mutiple disks");
        return ret;
    }

    AFS_TRACE_OUT_INFO("Total partitions number is %d", part_num);
    return real_part_num;
}

/**
 * @brief 获得分区信息及分区文件系统类型
 *
 * @param handle              回调函数参数(镜像文件指针)
 * @param read_callback_func  回调函数
 * @param *part_info          设置返回分区信息的结构体指针
 * @param input_part_num      设置需要获得分区信息的数量
 *
 * @return 大于0： 实际返回分区数量
 * 负数： 错误ID
 *    */
int32 getPartInfo(AFS_HANDLE handle, AFS_READ_CALLBACK_FUNC_t read_callback_func, struct partition *part_info,
    int32 input_part_num)
{
    int ret = 0;
    input_para_space_t input_para = {
        handle, read_callback_func, NULL, input_part_num, reinterpret_cast<char *>(part_info), 0, 0, NULL, 0
    };

    struct imgInfo img_info_obj;
    CHECK_MEMSET_S_OK(&img_info_obj, sizeof(struct imgInfo), 0, sizeof(img_info_obj));

    ret = gpiInitReaderParameter(input_para, img_info_obj);
    AFS_TRACE_OUT_DBG("img_info_obj.imageSize = %lld", (long long)img_info_obj.imageSize);

    if (ret != AFS_SUCCESS) {
        // 读取镜像大小失败
        AFS_TRACE_OUT_ERROR("Failed to read image size by call back function.");
        return ret;
    }

    // 构造镜像Reader
    rawReader raw_reader;
    imgReader *reader_tmp = &raw_reader;
    reader_tmp->initImgReader(&img_info_obj, 0, img_info_obj.imageSize / SECTOR_SIZE);

    // 处理分区
    partitionHandler part_handler;
    part_handler.setImgReader(reader_tmp);

    // 真实分区数量
    int real_part_num = gpiGetRealPartNumber(part_handler, input_part_num);
    if (real_part_num < 0) {
        AFS_TRACE_OUT_ERROR("Failed to get partition count. real_part_num = %d", real_part_num);
        return real_part_num;
    }
    AFS_TRACE_OUT_DBG("get partition count. real_part_num = %d", real_part_num);

    ret = gpiDoAllOfPartInfo(reader_tmp, input_para, part_handler, real_part_num);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to get all partitions info in single disk");
        return ret;
    }
    return real_part_num;
}

/**
 * @brief 根据参数获取reader的信息
 * @param &input_para    输入参数
 * @param &img_info_obj  传出参数
 * @return
 * AFS_SUCCESS
 * AFS_ERR_PARAMETER
 * AFS_ERR_PARA_BIT_SIZE
 * AFS_ERR_PARA_BUFFER_SIZE
 */
CPPUNIT_STATIC int32 commInitReaderParametar(input_para_space_t &input_para, struct imgInfo &img_info_obj)
{
    // 参数检查
    if ((NULL == input_para.handle) || (NULL == input_para.read_callback_func) || (NULL == input_para.bitmap_buffer) ||
        (input_para.buffer_size <= 0) || (input_para.bytes_per_bit <= 0)) {
        AFS_TRACE_OUT_ERROR("The specified parameter is not valid.");
        return AFS_ERR_PARAMETER;
    }

    // bytes_per_bit必须是512的整数倍
    uint32_t bytes_bit = (uint32_t)(input_para.bytes_per_bit);
    if ((0 != (bytes_bit & (uint32_t)(bytes_bit - 1))) || (bytes_bit < (uint32_t)AFS_DEFAULT_BLOCK_SIZE) ||
        bytes_bit > (uint32_t)(AFS_MX_BLOCK_SIZE)) {
        AFS_TRACE_OUT_ERROR("The input bytes_per_bit is not right.bytes_per_bit=%d", input_para.bytes_per_bit);
        return AFS_ERR_PARA_BIT_SIZE;
    }

    int32_t ret = setImgageInfo(input_para.handle, input_para.read_callback_func, &img_info_obj);
    if (ret != AFS_SUCCESS) {
        // 读取镜像大小失败
        AFS_TRACE_OUT_ERROR("Failed to read image size by call back function.");
        return ret;
    }

    // Buffer Size向上取整
    int64 tmp_buffer_size = (int64)(img_info_obj.imageSize + (int32)(input_para.bytes_per_bit * 8) - 1) /
        (int32)(input_para.bytes_per_bit * 8);
    if (tmp_buffer_size > input_para.buffer_size) {
        AFS_TRACE_OUT_ERROR("The input buffer size is little than real size. NeedSize = %lld, InputSize = %lld",
            (long long)tmp_buffer_size, (long long)(input_para.buffer_size));
        return AFS_ERR_PARA_BUFFER_SIZE;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 初始化reader参数
 *
 * @param &input_para   输入参数
 * @param &img_info_obj 镜像信息
 *
 * @return 0：成功  负数：错误ID
 *
 */
static int32 gdfbInitReaderParameter(input_para_space_t &input_para, struct imgInfo &img_info_obj)
{
    return commInitReaderParametar(input_para, img_info_obj);
}

/**
 * @brief 获取分区数量
 * @param handler
 * @return 大于0：分区个数
 * 负数：  错误ID
 */
static int32 gdfbGetPartNumber(partitionHandler &handler)
{
    int32 ret = 0;

    ret = handler.analyzePartitions();
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Failed to analyze image partition.");
        return ret;
    }

    // 根据分区表中的数目进行遍历
    ret = handler.getrealHandler()->getPartnum();
    if (ret <= 0) {
        AFS_TRACE_OUT_ERROR("Failed to get partition count.");
        return AFS_ERR_INNER;
    }

    return ret;
}

/**
 * @brief 获取多磁盘的bitmap
 * @param &input_para   输入参数
 * @param &handler      处理句柄
 * @param &bitmap       bitmap（内含image大小成员）
 *
 * @return 0:成功  负数:错误ID
 */
CPPUNIT_STATIC int32 gdfbGetMulitpleDiskBitMap(input_para_space_2_t &input_para, partitionHandler &handler,
    vector<BitMap *> &bitmap_vect)
{
    int32 ret = 0;
    int32_t disk_num = input_para.disk_num;
    BitMap *pBitmap = NULL;

    if (disk_num <= 0) {
        AFS_TRACE_OUT_ERROR("disk_num[%d] error", disk_num);
        return -1;
    }

    ret = handler.getDisksBitmap(bitmap_vect); // 分区表
    if (ret < 0) {
        AFS_TRACE_OUT_ERROR("Failed to get partition table bitmap.");
        return ret;
    }

    for (int32_t i = 0; i < disk_num; i++) {
        pBitmap = bitmap_vect[i];
        // /把bitmap中的值复制到buf中
        if (SECTOR_SIZE != input_para.bytes_per_bit) {
            // 根据指定的位代表大小转换Bitmap
            ret = afs_convertBitmap(input_para.bitmap_buffer[i], input_para.buffer_size[i], pBitmap->getImageSize(),
                *pBitmap, input_para.bytes_per_bit, 0);
            if (ret != AFS_SUCCESS) {
                AFS_TRACE_OUT_ERROR("Convert bitmap failed. bytes_per_bit = %d", input_para.bytes_per_bit);
                return ret;
            }
        } else {
            CHECK_MEMCPY_S_OK(input_para.bitmap_buffer[i], input_para.buffer_size[i],
                (const char *)pBitmap->getbitmap(), pBitmap->getsize());
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief 获取bitmap
 * @param &input_para   输入参数
 * @param &handler      处理句柄
 * @param &bitmap       bitmap
 * @param img_size      镜像大小
 *
 * @return 0:成功  负数:错误ID
 */
CPPUNIT_STATIC int32 gdfbGetBitMap(input_para_space_t &input_para, partitionHandler &handler, BitMap &bitmap,
    int64 img_size)
{
    int32 ret = 0;
    // /把bitmap中的值复制到buf中
    if (SECTOR_SIZE != input_para.bytes_per_bit) {
        // 根据指定的位代表大小转换Bitmap
        ret = afs_convertBitmap(input_para.bitmap_buffer, input_para.buffer_size, img_size, bitmap,
            input_para.bytes_per_bit, 0);
        if (ret != AFS_SUCCESS) {
            AFS_TRACE_OUT_ERROR("Convert bitmap failed. bytes_per_bit = %d", input_para.bytes_per_bit);
            return ret;
        }
    } else {
        CHECK_MEMCPY_S_OK(input_para.bitmap_buffer, input_para.buffer_size, (const char *)bitmap.getbitmap(),
            bitmap.getsize());
    }

    return AFS_SUCCESS;
}

/**
 * @brief 获取所有的分区bitmap
 * @param *reader   reader
 * @param &handler  处理句柄
 * @param &bitmap   Bitmap Buffer
 * @param part_num  分区数量
 *
 * @return 0: 成功  负数：失败
 */
static int32 gdfbDoAllOfPartBitMap(imgReader *reader, partitionHandler &handler, BitMap &bitmap, int32 part_num)
{
    int32 ret = 0;
    struct partition ppart;
    CHECK_MEMSET_S_OK(&ppart, sizeof(ppart), 0, sizeof(ppart));

    for (int32 i = 0; i < part_num; i++) {
        AFS_TRACE_OUT_DBG("Start to analyze partition index(%d).", i);
        imgReader part_reader(reader);

        ret = handler.getrealHandler()->getPartition(i, &ppart);
        if (ret != 0) {
            AFS_TRACE_OUT_ERROR("Failed to get partition information. part_current=%d", i);
            ret = AFS_ERR_INNER;
            return ret;
        }

        AFS_TRACE_OUT_DBG("Free disk part chunk size = %d", handler.m_map_chunk_size[i]);
        ret = part_reader.initImgReader(handler.getrealHandler()->getRealPartReader(), &ppart,
            handler.m_map_chunk_size[i]);
        // 分区偏移有问题
        if (ret != 0) {
            AFS_TRACE_OUT_ERROR("The input image information is NULL.");
            ret = AFS_ERR_INNER;
            return ret;
        }

        // 设置分区Bitmap
        if (ret != 0) {
            AFS_TRACE_OUT_INFO("Partition cannot to analyze. Index = %d", i);
            // 不支持文件系统类型和分析出错时，将该分区置全1
            ret = bitmap.bitmapSetRange(ppart.offset, ppart.length, 1);
            if (ret != 0) {
                AFS_TRACE_OUT_ERROR("Failed to set bitmap range.");
                ret = AFS_ERR_INNER;
                return ret;
            }
            continue;
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief 初始化信息
 * @param *reader         句柄
 * @param &input_para     输入参数
 * @param &img_info_obj   镜像信息
 * @param &bitmap         位图
 * @return 0:成功   负数:错误ID
 */
static int32 gdfbInitInfo(imgReader *reader, input_para_space_t &input_para, struct imgInfo &img_info_obj,
    BitMap &bitmap)
{
    int32 ret = 0;

    ret = gdfbInitReaderParameter(input_para, img_info_obj);
    if (AFS_SUCCESS != ret) {
        // 读取镜像大小失败
        AFS_TRACE_OUT_ERROR("Can'nt init parameter of input.");
        return ret;
    }

    // 构造镜像Reader
    reader->initImgReader(&img_info_obj, 0, img_info_obj.imageSize / SECTOR_SIZE);

    // 初始化bitmap
    ret = afs_initBitmap(&img_info_obj, bitmap);
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Failed to initialize bitmap.");
        return ret;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 处理单独disk
 * @param partsHandler        全局 partitionHandler 变量
 * @param imgHandle           disk镜像文件指针
 * @param read_callback_func  回调函数
 * @return :  0     处理成功
 * 其他     错误ID
 */
CPPUNIT_STATIC int32 gdfGetEachDiskPartNum(partitionHandler &partsHandler, input_para_space_t &input_para,
    BitMap &bitmap, int32_t disk_num)
{
    int32_t ret = 0;
    AFS_HANDLE imgHandle = input_para.handle;
    AFS_READ_CALLBACK_FUNC_t read_callback_func = input_para.read_callback_func;

    if (imgHandle == NULL) {
        AFS_TRACE_OUT_ERROR("The AFS_HANDLE parameter of disk[%u] is not valid.", disk_num);
        return AFS_ERR_PARAMETER;
    }

    struct imgInfo img_info_obj;
    ret = setImgageInfo(imgHandle, read_callback_func, &img_info_obj);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to read image size by call back function.");
        return ret;
    }

    // 构造镜像Reader
    imgReader *reader_img = new(std::nothrow) rawReader();
    if (reader_img == NULL) {
        AFS_TRACE_OUT_ERROR("New rawReader failed");
        return AFS_ERR_INNER;
    }
    reader_img->initImgReader(&img_info_obj, 0, img_info_obj.imageSize / SECTOR_SIZE);
    reader_img->setImgInfoDiskID(disk_num);

    partsHandler.setImgReader(reader_img);

    ret = afs_initBitmap(&img_info_obj, bitmap);
    if (AFS_SUCCESS != ret) {
        delete reader_img;
        AFS_TRACE_OUT_ERROR("Failed to initialize bitmap.");
        return ret;
    }

    partsHandler.setDiskNumValue(disk_num);
    AFS_TRACE_OUT_DBG("partsHandler.m_disknum is %d", partsHandler.getDiskNum());

    ret = partsHandler.analyzePartitions();
    if (AFS_SUCCESS != ret) {
        delete reader_img;
        AFS_TRACE_OUT_ERROR("Failed to analyze partition information.");
        return ret;
    }

    AFS_TRACE_OUT_DBG("ret = %d, finish parsing the [%u] disk", ret, disk_num);
    partsHandler.pushImgReader(reader_img);

    return ret;
}

/**
 * @brief 多磁盘场景下分区错误或者分区的文件系统错误，将分区的物理数据块置为有效数据标志
 * @param *reader       分区reader
 * @param *ppart        分区的相关信息
 * @param &bitmap_vect  所有磁盘的位图
 *
 * @return 0: 成功  负数：失败
 */
static int32_t dgfDoPartDefaultSet(imgReader *part_reader, struct partition *ppart, vector<BitMap *> &bitmap_vect)
{
    int64_t real_addr = -1;
    uint64_t index = 0;
    int32_t disk_id = -1;
    BitMap *pbitmap = NULL;
    int32_t ret = -1;
    uint32_t max_storage_zone = 0;
    uint64_t lv_length = ppart->lvm_info.lv_length;

    AFS_TRACE_OUT_DBG("Enter dgfDoPartDefaultSet");
    if (part_reader == NULL) {
        return AFS_ERR_PARAMETER;
    }

    // /判断是LVM格式的LV
    if (0 != part_reader->getChunkSize() && 1 == ppart->is_lvm) {
        max_storage_zone = part_reader->getChunkSize();
        AFS_TRACE_OUT_DBG("max_storage_zone is %u(sectors), lv_length is %llu", max_storage_zone, lv_length);
        for (index = 0; index < lv_length;) {
            disk_id = -1;
            // 需要转换地址(字节) ,返回值为物理地址（扇区）-----
            real_addr = part_reader->getVaddrToPaddr((int64_t)(index)*SECTOR_SIZE, disk_id);
            if (real_addr == -1) {
                AFS_TRACE_OUT_ERROR("Failed to get block number. block-num(%llu) block-size(%d sectors)", index,
                    max_storage_zone);
                return AFS_ERR_INNER;
            } else if (real_addr < 0) {
                AFS_TRACE_OUT_ERROR("A error[%d] happened when default set  lv ", real_addr);
                return real_addr;
            }

            if (-1 == disk_id) {
                AFS_TRACE_OUT_ERROR("Failed, real_addr is %llu, disk_id is %d", real_addr, disk_id);
                return AFS_ERR_INNER;
            }

            pbitmap = bitmap_vect[disk_id];
            ret = pbitmap->bitmapSetRange(real_addr, max_storage_zone, 1);
            if (ret != 0) {
                AFS_TRACE_OUT_ERROR("Failed to defaultly set lv bitmap");
                return AFS_ERR_INNER;
            }
            index += max_storage_zone;
        }
        AFS_TRACE_OUT_DBG("finish dgfDoPartDefaultSet bitmap");
        AFS_TRACE_OUT_DBG("last vaddr[%lld] to paddr is %lld", (int64_t)(index), real_addr);
    } else if (0 == ppart->is_pv_part && 0 == ppart->is_lvm) { // 物理分区
        disk_id = ppart->disk_id;
        pbitmap = bitmap_vect[disk_id];
        // /单纯物理分区不需要地址映射，直接按照扇区置为有效数据
        ret = pbitmap->bitmapSetRange(ppart->offset, ppart->length, 1);
        if (ret != 0) {
            AFS_TRACE_OUT_ERROR("Failed to defaultly set physical part bitmap.");
            return AFS_ERR_INNER;
        }
        AFS_TRACE_OUT_DBG("physical partition[%d] set valid data from disk offset(sectors): %llu, and length is %llu",
            ppart->part_id, (unsigned long long)ppart->offset, (unsigned long long)ppart->length);
    }

    if (1 == ppart->is_pv_part && 0 == ppart->is_lvm) {
        AFS_TRACE_OUT_INFO("the partition[%d] is a PV type", ppart->part_id);
    }
    return 0;
}

/**
 * @brief 获取多磁盘场景所有的分区bitmap
 * @param *reader   reader
 * @param &handler  处理句柄
 * @param &bitmap   Bitmap Buffer
 * @param part_num  分区数量
 *
 * @return 0: 成功  负数：失败
 */
static int32 gdfbDoMulitpleDisksBitMap(
    partitionHandler &handler, vector<BitMap *> &bitmap_vect, int32 part_num, std::string &errMsg)
{
    errMsg = "";
    int32 ret = 0;
    struct partition ppart;
    int32_t part_diskId;
    std::vector<imgReader *> &image_reader_vec = handler.getImgReaderVector();
    CHECK_MEMSET_S_OK(&ppart, sizeof(ppart), 0, sizeof(ppart));

    int32_t failPartNum = 0;
    for (int32 i = 0; i < part_num; i++) {
        AFS_TRACE_OUT_DBG("Start to analyze partition index(%d).", i);
        ret = handler.getPartition(i, &ppart);
        if (ret != 0) {
            AFS_TRACE_OUT_ERROR("Failed to get partition information. part_current=%d", i);
            ret = AFS_ERR_INNER;
            return ret;
        }

        part_diskId = ppart.disk_id;
        handler.setImgReader(image_reader_vec[part_diskId]); // 必须在handler.createPartitionReader(&ppart)之前执行
        ret = handler.createPartitionReader(&ppart); // 内部设置了 m_real_part_reader
        if (0 != ret) {
            AFS_TRACE_OUT_ERROR("Failed to create partition reader. Index=%d", i);
            continue;
        }

        imgReader part_reader(image_reader_vec[part_diskId]);
        ret = part_reader.initImgReader(handler.getRealPartReader(), &ppart, handler.m_map_chunk_size[i]);
        if (ret != 0) { // 分区偏移有问题
            AFS_TRACE_OUT_ERROR("The input image information is NULL.");
            ret = AFS_ERR_INNER;
            return ret;
        }

        // 设置分区Bitmap
        ret = afs_getSinglePartBitMap(&part_reader, &ppart, bitmap_vect);
        if (ret != 0) {
            failPartNum ++;
            AFS_TRACE_OUT_INFO(
                "Partition cannot to analyze. Index = %d, offset:%llu,length:%llu", i, ppart.offset, ppart.length);
            ret = bitmap_vect[part_diskId]->bitmapSetRange(ppart.offset, ppart.length, 1);
            if (ret != 0) {
                AFS_TRACE_OUT_ERROR("Failed to defaultly set part bitmap.");
                return AFS_ERR_INNER;
            }
            continue;
        }
    }

    if (failPartNum == part_num) {
        AFS_TRACE_OUT_ERROR("Failed to analyze all part file system type.");
        return AFS_ERR_FS_VERSION;
    }

    AFS_TRACE_OUT_INFO("gdfbDoMulitpleDisksBitMap() AFS_SUCCESS .");
    return AFS_SUCCESS;
}

/**
 * @brief 获得多块磁盘空闲块的bitmap
 * @param handle[]              回调函数参数(镜像文件指针数组)
 * @param read_callback_func    回调函数
 * @param *bitmap_buffer[]      设置返回bitmap的buffer数组
 * @param buffer_size[]         bitmap的Buffer大小
 * @param bytes_per_bit         设置bitmap中一位表示多少字节
 * @return   0: 设置成功
 * 负数： 错误ID
 */
int32 getMulipleDisksFreeBlockBitmap(AFS_HANDLE handles[], int32 disk_num, AFS_READ_CALLBACK_FUNC_t read_callback_func,
    char *bitmap_buffer[], int64 buffer_size[], int32 bytes_per_bit, std::string& errMsg)
{
    int32 ret = 0;
    int32_t disk_id = 0;
    int32_t part_num = 0;
    partitionHandler partsHandler;
    vector<BitMap *> bitmap_vect;

    input_para_space_t input_para = { NULL, read_callback_func, NULL, 0, NULL, 0, bytes_per_bit, NULL, 0 };

    input_para_space_2_t input_para2 = { handles,     read_callback_func, NULL, disk_num, bitmap_buffer,
                                         buffer_size, bytes_per_bit,      NULL, 0 };

    if (read_callback_func == NULL) {
        AFS_TRACE_OUT_ERROR("The read_callback_func parameter is not valid.");
        return AFS_ERR_PARAMETER;
    }
    AFS_TRACE_OUT_DBG("The disk_num is %d", disk_num);
    for (int32_t i = 0; i < disk_num; i++) {
        if (handles[i] == NULL || bitmap_buffer[i] == NULL || buffer_size[i] <= 0) {
            AFS_TRACE_OUT_ERROR("getMulipleDiskFreeBlkBitmap() parameters error");
            return AFS_ERR_PARAMETER;
        }
    }

    for (int32_t i = 0; i < disk_num; i++) {
        BitMap *pBitmap = new BitMap();
        input_para.handle = handles[i];
        input_para.buffer_size = buffer_size[i];
        input_para.bitmap_buffer = (char *)bitmap_buffer[i];

        ret = gdfGetEachDiskPartNum(partsHandler, input_para, *pBitmap, disk_id);
        if (ret != 0) {
            AFS_TRACE_OUT_WARN("Get the %d disk partition number failed.", i);
            break;
        }

        bitmap_vect.push_back(pBitmap);
        disk_id++;
    }

    if (bitmap_vect.size() != disk_num) {
        errMsg = "Can not get all disks partition";
        AFS_TRACE_OUT_ERROR("Can not get all disks partition number.");
        ret = AFS_ERR_PARTITION;
        goto get_free_block_bitmap_err;
    }

    part_num = partsHandler.getPartnum();
    AFS_TRACE_OUT_DBG("get parts num is %d, bitmap size():%d", part_num, bitmap_vect.size());

    ret = partsHandler.updateAllPartitions();
    if (ret != AFS_SUCCESS) {
        goto get_free_block_bitmap_err;
    }

    ret = gdfbDoMulitpleDisksBitMap(partsHandler, bitmap_vect, part_num, errMsg);
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Can'nt get BitMaps of all partitions.");
        goto get_free_block_bitmap_err;
    }
    AFS_TRACE_OUT_DBG("get parts num is %d, bitmap size():%d", part_num, bitmap_vect.size());
    ret = gdfbGetMulitpleDiskBitMap(input_para2, partsHandler, bitmap_vect);
    if (AFS_SUCCESS != ret) {
        // 读取镜像大小失败
        AFS_TRACE_OUT_ERROR("get all disk bitmaps failed");
        goto get_free_block_bitmap_err;
    }

get_free_block_bitmap_err:
    if ((ret != AFS_SUCCESS) && (NULL != bitmap_buffer)) {
        for (int32_t i = 0; i < disk_num; i++) {
            CHECK_MEMSET_S_OK(bitmap_buffer[i], buffer_size[i], 0xFF, buffer_size[i]);
        }
    }
    if (ret != AFS_SUCCESS) {
        for (size_t i = 0; i < bitmap_vect.size(); ++i) {
            if (bitmap_vect[i] != nullptr) {
                delete bitmap_vect[i];
                bitmap_vect[i] = nullptr;
            }
        }
    }
    AFS_TRACE_OUT_INFO("Finish invalid data recognizing of multiple disk");
    return ret;
}

/**
 * @brief 获得磁盘空闲块的bitmap
 * @param handle              回调函数参数(镜像文件指针)
 * @param read_callback_func  回调函数
 * @param *bitmap_buffer      设置返回bitmap的buffer
 * @param buffer_size         bitmap的Buffer大小
 * @param bytes_per_bit       设置bitmap中一位表示多少字节
 * @return   0: 设置成功
 * 负数： 错误ID
 */
int32 getDiskFreeBlkBitmap(AFS_HANDLE handle, AFS_READ_CALLBACK_FUNC_t read_callback_func, char *bitmap_buffer,
    int64 buffer_size, int32 bytes_per_bit)
{
    int32 ret = 0;

    input_para_space_t input_para = { handle,      read_callback_func, NULL, 0, bitmap_buffer,
                                      buffer_size, bytes_per_bit,      NULL, 0 };

    int32 part_num = 0;

    struct imgInfo img_info_obj;
    rawReader raw_reader;
    imgReader *img_reader = &raw_reader;
    CHECK_MEMSET_S_OK(&img_info_obj, sizeof(img_info_obj), 0, sizeof(img_info_obj));

    BitMap bitmap;
    partitionHandler handler;

    ret = gdfbInitInfo(img_reader, input_para, img_info_obj, bitmap);
    if (AFS_SUCCESS != ret) {
        // 读取镜像大小失败
        AFS_TRACE_OUT_ERROR("Can'nt init parameter of input.");
        goto out_label;
    }

    // 获得分区表bitmap
    handler.setImgReader(img_reader);

    // 根据分区表中的数目进行遍历
    part_num = gdfbGetPartNumber(handler);
    if (part_num <= 0) {
        AFS_TRACE_OUT_ERROR("Failed to get partition count.");
        ret = part_num;
        goto out_label;
    }

    ret = gdfbDoAllOfPartBitMap(img_reader, handler, bitmap, part_num);
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Can'nt Get BitMap of partition.");
        goto out_label;
    }

    // lvm
    // thin模式下，lv大小不是真正的大小（使用多少分配多少），文件系统bitmap初始化的时候会，冲刷掉不是正在空间BitMap /
    // 所以分区表的BitMap需要（之前设置的BitMap），后延防止数据被冲刷
    ret = gdfbGetBitMap(input_para, handler, bitmap, img_info_obj.imageSize);
    if (AFS_SUCCESS != ret) {
        // 读取镜像大小失败
        AFS_TRACE_OUT_ERROR("Can'nt Get BitMap.");
        goto out_label;
    }

    ret = AFS_SUCCESS;

out_label:
    if ((ret != AFS_SUCCESS) && (NULL != bitmap_buffer)) {
        CHECK_MEMSET_S_OK(bitmap_buffer, buffer_size, 0xFF, buffer_size);
    }

    return ret;
}

/**
 * @brief 根据参数获取reader的信息
 * @param &input_para    输入参数
 * @param &img_info_obj  传出参数
 * @return :
 * AFS_SUCCESS
 * AFS_ERR_PARAMETER
 * AFS_ERR_PARA_BIT_SIZE
 * AFS_ERR_PARA_BUFFER_SIZE
 */
CPPUNIT_STATIC int32 gflpInitReaderParametar(input_para_space_t &input_para, struct imgInfo &img_info_obj)
{
    // 参数检查
    if (0 > input_para.file_num || NULL == input_para.file_list || 0 > input_para.part_index) {
        AFS_TRACE_OUT_ERROR("The specified parameter is not valid.");
        return AFS_ERR_PARAMETER;
    }

    return commInitReaderParametar(input_para, img_info_obj);
}

/**
 * @brief 根据参数获取reader的信息
 * @param &input_para    输入参数
 * @param &img_info_obj  传出参数
 * @return :
 * AFS_SUCCESS
 * AFS_ERR_PARAMETER
 * AFS_ERR_PARA_BIT_SIZE
 * AFS_ERR_PARA_BUFFER_SIZE
 */
static int32 gfpInitReaderParametar(input_para_space_t &input_para, struct imgInfo &img_info_obj)
{
    // 参数检查
    if (NULL == input_para.file_path || 0 > input_para.part_index) {
        AFS_TRACE_OUT_ERROR("The specified parameter is not valid.");
        return AFS_ERR_PARAMETER;
    }

    return commInitReaderParametar(input_para, img_info_obj);
}

/**
 * @brief 通过index获取part信息
 * @param &handler     分区处理句柄
 * @param part_index   分区索引号
 * @param &part        返回分区信息
 * @return
 * AFS_SUCCESS
 * AFS_ERR_INNER
 * AFS_ERR_INNER
 * AFS_ERR_PART_INDEX_INVALID
 */
CPPUNIT_STATIC int32 gfpGetPartInfoByIndex(partitionHandler &handler, int32 part_index, struct partition &part)
{
    int32_t ret = handler.analyzePartitions();
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Failed to analyze image partition.");
        return ret;
    }

    // 检查分区ID是否有效
    int32 part_num = handler.getrealHandler()->getPartnum();
    if (part_num <= 0) {
        AFS_TRACE_OUT_ERROR("Failed to get partition count.");
        return AFS_ERR_INNER;
    }
    if (part_index >= part_num) {
        AFS_TRACE_OUT_ERROR("The specified partition index is not valid. part_index=%d, partition_count=%d", part_index,
            part_num);
        return AFS_ERR_PART_INDEX_INVALID;
    }

    ret = handler.getrealHandler()->getPartition(part_index, &part);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to read partition information. Index=%d", part_index);
        return ret;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 文件系统处理
 * @param &part_reader  分区reader
 * @param &fs_type      传出 fs_type
 * @return
 * AFS_SUCCESS
 * AFS_ERR_FS_SUPPORT
 */
static int32 gfpDoFilesSystem(imgReader &part_reader, int32 &fs_type)
{
    int32_t ret = 0;

    ret = afs_getFSType(&part_reader, &fs_type);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("This file system is unsupported.");
        return ret;
    }

    if (fs_type <= 0) {
        AFS_TRACE_OUT_ERROR("Invalid part index had been specified.");
        ret = AFS_ERR_FS_SUPPORT;
        return ret;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 处理bitmap
 * @param &input_para    输入参数
 * @param &part_reader   分区reader
 * @param fs_type        文件系统类型
 * @param &img_info_obj  镜像对象
 * @return
 * 0：成功   负数：失败
 */
static int32 gfpDoBitMap(input_para_space_t &input_para, imgReader &part_reader, int32 fs_type,
    struct imgInfo &img_info_obj)
{
    int32_t ret = 0;
    // /初始化整个镜像的bitmap buffer
    BitMap bitmap;
    ret = afs_initBitmap(&img_info_obj, bitmap);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to initialize image file bitmap.");
        return ret;
    }
    return AFS_SUCCESS;
}

/**
 * @brief 处理bitmap
 *
 * @param &input_para    输入参数
 * @param &part_reader   分区reader
 * @param fs_type        文件系统类型
 * @param &img_info_obj  镜像对象
 * @return
 * 0: 成功   负数：错误ID
 */
CPPUNIT_STATIC int32 gflpDoBitMap(input_para_space_t &input_para, imgReader &part_reader, int32 fs_type,
    struct imgInfo &img_info_obj)
{
    int32_t ret = 0;
    uint32_t filter_count = 0;
    BitMap bitmap;

    // 初始化整个镜像的Bitmap的buffer
    ret = afs_initBitmap(&img_info_obj, bitmap);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to initialize bitmap.");
        return ret;
    }

    // 遍历文件列表
    for (int i = 0; i < input_para.file_num; i++) {
        if (ret != 0) {
            AFS_TRACE_OUT_ERROR("Failed to filter current path. Path=%s, Result=%d", (char *)(input_para.file_list[i]),
                ret);
        } else {
            filter_count++;
        }
    }

    if (0 == filter_count) {
        return ret; // 返回最后一个文件失败时的错误ID
    }
    return AFS_SUCCESS;
}

/**
 * @brief 过滤指定文件处理
 *
 * @param *img_reader        读取镜像Reader指针
 * @param &img_info_obj      分区reader
 * @param part_index         分区ID
 * @param &handler           分区Handler
 * @param &part_reader       分区Reader
 * @param &fs_type           返回文件系统类型
 * @return
 * 0：成功
 * 负数：错误ID
 */
CPPUNIT_STATIC int32 afs_getFilePosition_1(imgReader *img_reader, struct imgInfo &img_info_obj, int32 part_index,
    partitionHandler &handler, imgReader &part_reader, int32 &fs_type)
{
    int32 ret = AFS_SUCCESS;

    img_reader->initImgReader(&img_info_obj, 0, img_info_obj.imageSize / SECTOR_SIZE);

    // 分区分析
    struct partition part;
    CHECK_MEMSET_S_OK(&part, sizeof(part), 0, sizeof(part));

    // 分区分析
    handler.setImgReader(img_reader);
    ret = gfpGetPartInfoByIndex(handler, part_index, part);
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Failed to analyze information of partition.");
        return ret;
    }

    AFS_TRACE_OUT_DBG("Filter file chunk size = %d", handler.m_map_chunk_size[part_index]);

    ret = part_reader.initImgReader(handler.getrealHandler()->getRealPartReader(), &part,
        handler.m_map_chunk_size[part_index]);
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Failed to set partition offset.");
        return ret;
    }

    if (AFS_FILESYSTEM_SWAP == part.fstype) {
        AFS_TRACE_OUT_ERROR("It's SWAP partition.");
        return AFS_ERR_FS_SUPPORT;
    }

    ret = gfpDoFilesSystem(part_reader, fs_type);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to check file system type. ret=%d", ret);
        return ret;
    }

    return ret;
}

/**
 * @brief 获得文件所占块的bitmap
 *
 * @param handle              回调函数参数(镜像文件指针)
 * @param read_callback_func  回调函数
 * @param *file_path          设置要寻找文件全路径
 * @param part_index          设置文件所在分区号
 * @param *bitmap_buffer      设置返回bitmap的Buffer
 * @param buffer_size         bitmap的Buffer大小
 * @param bytes_per_bit       设置bitmap中1位表示的字节数
 *
 * @return : 0：设置成功     负数：设置失败（错误ID）
 */
int32 getFilePosition(AFS_HANDLE handle, AFS_READ_CALLBACK_FUNC_t read_callback_func, const char *file_path,
    int32 part_index, char *bitmap_buffer, int64 buffer_size, int32 bytes_per_bit)
{
    int32_t ret = 0;
    input_para_space_t input_para = { handle,      read_callback_func, file_path, part_index, bitmap_buffer,
                                      buffer_size, bytes_per_bit,      NULL,      0 };

    struct imgInfo img_info_obj;
    CHECK_MEMSET_S_OK(&img_info_obj, sizeof(img_info_obj), 0, sizeof(img_info_obj));

    ret = gfpInitReaderParametar(input_para, img_info_obj);
    if (AFS_SUCCESS != ret) {
        // 读取镜像大小失败
        AFS_TRACE_OUT_ERROR("Failed to read image size by call back function.");
        return ret;
    }

    int32 fs_type = 0;
    // 初始化镜像信息
    rawReader raw_reader;
    imgReader *img_reader = &raw_reader;
    imgReader part_reader; // 分区Reader
    partitionHandler handler;

    ret = afs_getFilePosition_1(img_reader, img_info_obj, part_index, handler, part_reader, fs_type);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to check partition. part index:%d", part_index);
        return ret;
    }

    ret = gfpDoBitMap(input_para, part_reader, fs_type, img_info_obj);
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Cannot get bitmap information. ret = %d", ret);
        return ret;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 获得文件列表所占块的bitmap
 *
 * @param handle              回调函数参数(镜像文件指针)
 * @param read_callback_func  回调函数
 * @param **file_list         设置要寻找文件路径列表
 * @param file_num            过滤路径数量
 * @param part_index          设置文件所在分区号
 * @param *bitmap_buffer      设置返回bitmap的Buffer
 * @param buffer_size         bitmap的Buffer大小（字节）
 * @param bytes_per_bit       设置bitmap中1位表示的字节数
 *
 * @return : 0：设置成功        负数：设置失败（错误ID）
 */
int32 getFileListPosition(AFS_HANDLE handle, AFS_READ_CALLBACK_FUNC_t read_callback_func, const char **file_list,
    int32 file_num, int32 part_index, char *bitmap_buffer, int64 buffer_size, int32 bytes_per_bit)
{
    int32_t ret = 0;

    // 参数检查
    input_para_space_t input_para = { handle,        read_callback_func, "This is a filter of file of list.",
                                      part_index,    bitmap_buffer,      buffer_size,
                                      bytes_per_bit, file_list,          file_num };

    // 参数检查
    struct imgInfo img_info_obj;
    ret = gflpInitReaderParametar(input_para, img_info_obj);
    if (AFS_SUCCESS != ret) {
        // 读取镜像大小失败
        AFS_TRACE_OUT_ERROR("Failed to read image size by call back function.");
        return ret;
    }

    // 初始化镜像信息
    rawReader raw_reader;
    imgReader *img_reader = &raw_reader;

    int32 fs_type = 0;
    imgReader part_reader; // 分区Reader
    partitionHandler handler;

    ret = afs_getFilePosition_1(img_reader, img_info_obj, part_index, handler, part_reader, fs_type);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to check partition. part index:%d", part_index);
        return ret;
    }

    ret = gflpDoBitMap(input_para, part_reader, fs_type, img_info_obj);
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Can'nt get bitmap information.");
        return ret;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 多磁盘场景下，文件bitmap转换为buff
 * @param &input_para    输入参数
 * @param &img_info_obj  分区信息
 * @param &bitmap        Bitmap buffer
 * @return 0：成功  负数：错误ID
 */
CPPUNIT_STATIC int32_t multipleDisksFilesBitmapToBuff(input_para_space_2_t &input_para, vector<BitMap *> &bitmap_vect)
{
    int32_t ret = AFS_SUCCESS;
    BitMap *pBitmap = NULL;

    for (int32_t i = 0; i < input_para.disk_num; i++) {
        pBitmap = bitmap_vect[i];
        // /把bitmap中的值复制到buf中
        if (SECTOR_SIZE != input_para.bytes_per_bit) {
            // 根据指定的位代表大小转换Bitmap
            ret = afs_convertBitmap(input_para.bitmap_buffer[i], input_para.buffer_size[i], pBitmap->getImageSize(),
                *pBitmap, input_para.bytes_per_bit, 1);
            if (ret != AFS_SUCCESS) {
                AFS_TRACE_OUT_ERROR("Convert bitmap failed. bytes_per_bit = %d", input_para.bytes_per_bit);
                return ret;
            }
        } else {
            CHECK_MEMCPY_S_OK(input_para.bitmap_buffer[i], input_para.buffer_size[i],
                (const char *)pBitmap->getbitmap(), pBitmap->getsize());
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief 将char[]形式的文件系统UUID转换为 "97bbbf59-ea1e-4093-bfe3-22946b73f59d" 字符串格式
 *
 * @param *fs_uuid            文件系统的UUID
 * @param uuid_len            文件系统的UUID内存长度
 * @param *fs_uuid_string     最终转换后的UUID格式
 * @param uuid_string_len     转换后的UUID长度
 *
 * @return : 0: 文件系统UUID转换成功     其他：UUID转换失败
 * 备注：转化后的UUID字符串格式字母全是小写
 */
CPPUNIT_STATIC int32_t afs_convertFSUUID2String(char *fs_uuid, uint32_t uuid_len, char *fs_uuid_string,
    uint32_t uuid_string_len)
{
    int32_t uuid_byte_num = 0;
    int32_t string_byte_num = 0;

    if (uuid_string_len != AFS_FS_UUID_STRING_LEN + 1 || uuid_len != AFS_FS_UUID_MAX_LEN + 1) {
        AFS_TRACE_OUT_ERROR("The size of file system string uuid [%d] or UUID[%d] is not right", uuid_string_len,
            uuid_len);
        return -1;
    }

    for (; uuid_byte_num < 4; uuid_byte_num++) {
        CHECK_VSNPRINTF_S_OK(fs_uuid_string + string_byte_num, uuid_string_len - string_byte_num, 2, "%02x",
            (unsigned char)fs_uuid[uuid_byte_num]);
        string_byte_num += 2;
    }

    CHECK_VSNPRINTF_S_OK(fs_uuid_string + string_byte_num, uuid_string_len - string_byte_num, 1, "%c", '-');
    string_byte_num++;

    for (; uuid_byte_num < 6; uuid_byte_num++) {
        CHECK_VSNPRINTF_S_OK(fs_uuid_string + string_byte_num, uuid_string_len - string_byte_num, 2, "%02x",
            (unsigned char)fs_uuid[uuid_byte_num]);
        string_byte_num += 2;
    }

    CHECK_VSNPRINTF_S_OK(fs_uuid_string + string_byte_num, uuid_string_len - string_byte_num, 1, "%c", '-');
    string_byte_num++;

    for (; uuid_byte_num < 8; uuid_byte_num++) {
        CHECK_VSNPRINTF_S_OK(fs_uuid_string + string_byte_num, uuid_string_len - string_byte_num, 2, "%02x",
            (unsigned char)fs_uuid[uuid_byte_num]);
        string_byte_num += 2;
    }

    CHECK_VSNPRINTF_S_OK(fs_uuid_string + string_byte_num, uuid_string_len - string_byte_num, 1, "%c", '-');
    string_byte_num++;

    for (; uuid_byte_num < 10; uuid_byte_num++) {
        CHECK_VSNPRINTF_S_OK(fs_uuid_string + string_byte_num, uuid_string_len - string_byte_num, 2, "%02x",
            (unsigned char)fs_uuid[uuid_byte_num]);
        string_byte_num += 2;
    }

    CHECK_VSNPRINTF_S_OK(fs_uuid_string + string_byte_num, uuid_string_len - string_byte_num, 1, "%c", '-');
    string_byte_num++;

    for (; uuid_byte_num < 16; uuid_byte_num++) {
        CHECK_VSNPRINTF_S_OK(fs_uuid_string + string_byte_num, uuid_string_len - string_byte_num, 2, "%02x",
            (unsigned char)fs_uuid[uuid_byte_num]);
        string_byte_num += 2;
    }

    fs_uuid_string[AFS_FS_UUID_STRING_LEN] = '\0';
    AFS_TRACE_OUT_DBG("file system's string format uuid is %s", fs_uuid_string);
    return 0;
}

/**
 * @brief 将 char[]形式的NTFS UUID转换为 "D8B3-0120"(windows) 或 "07886DA827174E46"(linux) 字符串格式
 *
 * @param *fs_uuid            文件系统的UUID
 * @param uuid_len            文件系统的UUID内存长度
 * @param *fs_uuid_string     最终转换后的UUID格式
 * @param uuid_string_len     转换后的UUID长度
 *
 * @return : 0: 文件系统UUID转换成功     其他：UUID转换失败
 * 备注：转化后的UUID字符串格式字母全是大写
 */
CPPUNIT_STATIC int32_t afs_convertFSUUID2String_1(char *fs_uuid, uint32_t uuid_len, char *fs_uuid_string,
    uint32_t uuid_string_len)
{
    if (uuid_string_len != AFS_FS_UUID_STRING_LEN + 1 || uuid_len != AFS_FS_UUID_MAX_LEN + 1) {
        AFS_TRACE_OUT_ERROR("The size of file system string uuid [%d] or UUID[%d] is not right", uuid_string_len,
            uuid_len);
        return -1;
    }

#ifdef _WIN32
    CHECK_MEMCPY_S_OK(fs_uuid_string, uuid_string_len, fs_uuid + 2, 4);
    CHECK_VSNPRINTF_S_OK(fs_uuid_string + 4, uuid_string_len - 4, 1, "%c", '-');
    CHECK_MEMCPY_S_OK(fs_uuid_string + 5, uuid_string_len - 5, fs_uuid + 12, 4);
#else
    CHECK_MEMCPY_S_OK(fs_uuid_string, uuid_string_len, fs_uuid,
        AFS_FS_UUID_MAX_LEN + 1); // /linux 下NTFS的UUID格式不用改变
#endif

    return 0;
}

/**
 * @brief 解析分区的UUID, 并和目的分区UUID进行比较
 *
 * @param *part_reader      读分区内容函数
 * @param *fs_type          分区的文件类型
 * @param target_fs_uuid    目标分区的UUID
 *
 * @return : 0: 找到分区UUID   -1：解析文件系统的type和UUID失败  -2：该文件系统不是目标分区文件系统
 */
CPPUNIT_STATIC int32_t afs_getFSTypeUUID(imgReader *part_reader, int32_t *fs_type, string target_fs_uuid)
{
    int32 ret = AFS_SUCCESS;
    char fs_uuid[AFS_FS_UUID_MAX_LEN + 1];
    char fs_uuid_string[AFS_FS_UUID_STRING_LEN + 1];

    CHECK_MEMSET_S_OK(fs_uuid, AFS_FS_UUID_MAX_LEN + 1, 0, AFS_FS_UUID_MAX_LEN + 1);
    CHECK_MEMSET_S_OK(fs_uuid_string, AFS_FS_UUID_STRING_LEN + 1, 0, AFS_FS_UUID_STRING_LEN + 1);

    filesystemHandler fshandler;
    fshandler.setImageReader(part_reader);
    ret = fshandler.getFSTypeUUID(fs_type, fs_uuid, AFS_FS_UUID_MAX_LEN + 1);
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("Failed to analyze file system type and uuid, error code is %d", ret);
        return -1;
    }

    switch (*fs_type) {
        case AFS_FILESYSTEM_EXT4:
        case AFS_FILESYSTEM_XFS:
            ret = afs_convertFSUUID2String(fs_uuid, sizeof(fs_uuid), fs_uuid_string,
                sizeof(fs_uuid_string)); // /EXT4和XFS处理逻辑是相同的
            break;
        case AFS_FILESYSTEM_NTFS:
            ret = afs_convertFSUUID2String_1(fs_uuid, sizeof(fs_uuid), fs_uuid_string, sizeof(fs_uuid_string));
            break;
        default:
            AFS_TRACE_OUT_INFO("file system type is not EXT4/EXT3, XFS, and NTFS");
            return -1;
    }

    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("convert file system's uuid error");
        return -2;
    }

    string uuid_string = fs_uuid_string;
    if (target_fs_uuid.compare(uuid_string) == 0) {
        AFS_TRACE_OUT_INFO("find the targeted file system [uuid: %s, FS type is %d]", fs_uuid_string, *fs_type);
        return AFS_SUCCESS;
    }

    AFS_TRACE_OUT_ERROR("the file system is not the targeted one");
    return -2;
}

/**
 * @brief 获取分区文件系统UUID, 判断UUID是否为目的分区, 获取文件列表bitmap, 拷贝bitmap到buffer
 *
 * @param *img_reader    镜像句柄
 * @param *ppart         分区信息
 * @param & bitmap_vect  所有磁盘的位图
 * @Parma & disk_reader_vect  所有磁盘的rawReader
 *
 * @return : 0：成功   负数：失败
 */
CPPUNIT_STATIC int32 afs_getFilesBitMap(imgReader *part_reader, struct partition *ppart, vector<BitMap *> &bitmap_vect,
    input_para_space_2_t input_para, string target_fs_uuid)
{
    int32_t ret = AFS_SUCCESS;
    int32_t fs_type;

    // 获得分区文件系统类型
    ret = afs_getFSTypeUUID(part_reader, &fs_type, target_fs_uuid);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to analyze file system type or the file system is not the targeted FS");
        return ret;
    }

    ppart->fstype = (AFS_FSTYPE_t)fs_type;

    // 遍历文件列表
    for (int i = 0; i < input_para.file_num; i++) {
        ret = afs_getFilePosition(part_reader, bitmap_vect, fs_type, (char *)(input_para.file_list[i]));
        if (ret != AFS_SUCCESS) {
            AFS_TRACE_OUT_ERROR("Failed to filter current path. Path=%s, Result=%d", (char *)(input_para.file_list[i]),
                ret);
            return AFS_ERR_FS_ANALYZE;
        }
    }

    ret = multipleDisksFilesBitmapToBuff(input_para, bitmap_vect);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to convert bitmap to buffer, and error code is %d", ret);
        return AFS_ERR_FS_ANALYZE;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 遍历分区，找到文件所在目标分区进行bitmap设置
 *
 * @param partsHandle
 * @param input_para
 * @param bitmap_vect
 * @param part_num
 *
 * @return : 0：设置成功     负数：设置失败（错误ID）
 */
CPPUNIT_STATIC int32_t gflpDoMutipleDisksFilesBitmap(partitionHandler &partsHandle, input_para_space_2_t input_para,
    vector<BitMap *> &bitmap_vect, int32_t part_num)
{
    int32 ret = -1;
    struct partition ppart;
    int32_t part_diskId;
    vector<imgReader *> &image_reader_vec = partsHandle.getImgReaderVector();
    string target_uuid_string;
    CHECK_MEMSET_S_OK(&ppart, sizeof(ppart), 0, sizeof(ppart));

    target_uuid_string = input_para.fs_uuid;
    AFS_TRACE_OUT_DBG("the target file system's uuid is %s [bytes:%u]", target_uuid_string.c_str(),
        target_uuid_string.size());

    for (int32 i = 0; i < part_num; i++) {
        AFS_TRACE_OUT_DBG("Start to analyze partition index(%d).", i);

        ret = partsHandle.getPartition(i, &ppart);
        if (ret != AFS_SUCCESS) {
            AFS_TRACE_OUT_ERROR("Failed to get partition information. part_current=%d", i);
            ret = AFS_ERR_INNER;
            return ret;
        }

        part_diskId = ppart.disk_id;
        AFS_TRACE_OUT_DBG("part_info[%d] disk_Id is %d", i, part_diskId);
        partsHandle.setImgReader(image_reader_vec[part_diskId]); // 必须在handler.createPartitionReader(&ppart)之前执行

        ret = partsHandle.createPartitionReader(&ppart); // 内部设置了 m_real_part_reader
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("Failed to create partition reader. Index=%d", i);
            continue;
        }

        imgReader part_reader(image_reader_vec[part_diskId]);
        ret = part_reader.initImgReader(partsHandle.getRealPartReader(), &ppart, partsHandle.m_map_chunk_size[i]);
        if (ret != 0) { // 分区偏移有问题
            AFS_TRACE_OUT_ERROR("The input image information is NULL.");
            ret = AFS_ERR_INNER;
            return ret;
        }

        ret = afs_getFilesBitMap(&part_reader, &ppart, bitmap_vect, input_para, target_uuid_string);
        if (ret == AFS_SUCCESS) {
            AFS_TRACE_OUT_DBG("find the target partition, and success converting files bitmap to buffer");
            return AFS_SUCCESS;
        } else if (ret == AFS_ERR_FS_ANALYZE) {
            AFS_TRACE_OUT_ERROR("find the target partition, but analyze it error");
            return AFS_ERR_FS_ANALYZE;
        }
    }

    AFS_TRACE_OUT_ERROR("not find the targeted partition or get the files bitmap failed");
    return -1;
}

/**
 * @brief 多磁盘下获得文件列表所占块的bitmap
 *
 * @param handles[]           回调函数参数(镜像文件指针)
 * @param disk_num            磁盘个数
 * @param read_callback_func  回调函数
 * @param *file_path[]        设置要寻找文件列表（全路径，不用带挂载点路径）
 * @param files_num           文件个数
 * @param fs_uuid             文件列表所在文件系统的UUID，用于在唯一的分区中搜寻指定文件
 * @param *bitmap_buffer[]    返回文件列表的数据块在多磁盘上bitmap的Buffer
 * @param buffer_size[]       bitmap的Buffer大小
 * @param bytes_per_bit       设置bitmap中1位表示的字节数
 *
 * @return : 0：设置成功     负数：设置失败（错误ID）
 */
int32 getMulipleDisksFileListBlockBitmap(AFS_HANDLE handles[], int32_t disk_num,
    AFS_READ_CALLBACK_FUNC_t read_callback_func, char *file_path[], int32_t files_num, char *fs_uuid,
    char *bitmap_buffer[], int64 buffer_size[], int32 bytes_per_bit)
{
    int32 ret = 0;
    int32_t disk_id = 0;
    int32_t part_num = 0;
    partitionHandler partsHandler;
    vector<BitMap *> bitmap_vect;

    input_para_space_t input_para = { NULL, read_callback_func, NULL, 0, NULL, 0, bytes_per_bit, NULL, 0 };
    input_para_space_2_t input_para2 = { handles,     read_callback_func, fs_uuid,   disk_num, bitmap_buffer,
                                         buffer_size, bytes_per_bit,      file_path, files_num };

    if (read_callback_func == NULL) {
        AFS_TRACE_OUT_ERROR("The read_callback_func parameter is not valid.");
        return AFS_ERR_PARAMETER;
    }

    for (int32_t i = 0; i < disk_num; i++) {
        if (handles[i] == NULL || bitmap_buffer[i] == NULL || buffer_size[i] <= 0) {
            AFS_TRACE_OUT_ERROR("getMulipleDisksFileListBlockBitmap() parameters error");
            return AFS_ERR_PARAMETER;
        }
    }

    for (int32_t i = 0; i < disk_num; i++) {
        BitMap *pBitmap = new BitMap();
        input_para.handle = handles[i];
        input_para.buffer_size = buffer_size[i];
        input_para.bitmap_buffer = (char *)bitmap_buffer[i];

        ret = gdfGetEachDiskPartNum(partsHandler, input_para, *pBitmap, disk_id);
        if (ret != AFS_SUCCESS) {
            goto get_files_position_err;
        }

        bitmap_vect.push_back(pBitmap);
        disk_id++;
    }

    part_num = partsHandler.getPartnum();
    AFS_TRACE_OUT_DBG("total partitions number is %d", part_num);

    ret = partsHandler.updateAllPartitions();
    if (ret != AFS_SUCCESS) {
        goto get_files_position_err;
    }

    ret = gflpDoMutipleDisksFilesBitmap(partsHandler, input_para2, bitmap_vect, part_num);
    if (ret != AFS_SUCCESS) {
        goto get_files_position_err;
    }

get_files_position_err:
    if ((ret != AFS_SUCCESS) && (NULL != bitmap_buffer)) {
        for (int32_t i = 0; i < disk_num; i++) {
            // 0表示对应的块非指定文件数据块
            CHECK_MEMSET_S_OK(bitmap_buffer[i], buffer_size[i], 0, buffer_size[i]);
        }
    }

    AFS_TRACE_OUT_INFO("Finish invalid data recognizing of multiple disk");
    return ret;
}
