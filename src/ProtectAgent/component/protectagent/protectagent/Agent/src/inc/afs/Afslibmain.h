#ifndef AFSLIBMAIN_H
#define AFSLIBMAIN_H

#include <vector>
#include <string>
#include "afs/LogMsg.h"
#include "afs/AfsType.h"
#include "afs/AfsError.h"
#include "afs/Afslibrary.h"

using namespace std;

#ifdef CPPUNIT_MAIN
#include "afs/RawReader.h"
#include "afs/Bitmap.h"
#include "afs/PartitionHandler.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CPPUNIT_MAIN
int32 setImgageInfo(AFS_HANDLE handle, AFS_READ_CALLBACK_FUNC_t read_callback_func, struct imgInfo *imginfo);
int32 afs_getFSType(imgReader *reader, int32 *fs_type);
int32 afs_initBitmap(struct imgInfo *imginfo, BitMap &bitmap);
int32 afs_getSinglePartBitMap(imgReader *img_reader, struct partition *ppart, vector<BitMap *> &bitmap_vec);

int32 afs_convertBitmap(char *data_buffer, int64 buffer_size, int64 img_size, BitMap &bitmap, int32 bytes_per_bit,
    int32 filt_buffer_flag);

int32 afs_getFilePosition(imgReader *img_reader, vector<BitMap *> &bitmap_vect, int32 fsType, const char *file_path);
int32 afs_parseSwapSpace(imgReader *img_reader, struct partition *ppart, vector<BitMap *> &bitmap_vect);
int32 afs_getPartBitmap(imgReader *reader, vector<BitMap *> &bitmap_vect, int32 fs_type);
int32 gflpInitReaderParametar(input_para_space_t &input_para, struct imgInfo &img_info_obj);
int32 afs_getFilePosition_1(imgReader *img_reader, struct imgInfo &img_info_obj, int32 part_index,
    partitionHandler &handler, imgReader &part_reader, int32 &fs_type);
int32 gpiGetRealPartNumber(partitionHandler &handler, int32 input_part_num);
int32 gpiDoAllOfPartInfo(imgReader *reader, input_para_space_t &input_para, partitionHandler &handler,
    int real_part_num);
int32 gfpGetPartInfoByIndex(partitionHandler &handler, int32 part_index, struct partition &part);
int32 gflpDoBitMap(input_para_space_t &input_para, imgReader &part_reader, int32 fs_type, struct imgInfo &img_info_obj);

int32_t dgfDoPartDefaultSet(imgReader *part_reader, struct partition *ppart, vector<BitMap *> &bitmap_vect);

int32 gpiGetEachDiskPartNum(partitionHandler &partsHandler, AFS_HANDLE imgHandle,
    AFS_READ_CALLBACK_FUNC_t read_callback_func, int32_t disk_num);

int32 gpiDoMultipleDiskAllPartInfo(input_para_space_t &input_para, partitionHandler &partsHandler,
    int32_t real_part_num);

int32 gdfbGetBitMap(input_para_space_t &input_para, partitionHandler &handler, BitMap &bitmap, int64 img_size);

int32 gdfbGetMulitpleDiskBitMap(input_para_space_2_t &input_para, partitionHandler &handler,
    vector<BitMap *> &bitmap_vect);

int32 commInitReaderParametar(input_para_space_t &input_para, struct imgInfo &img_info_obj);
int32_t initBitMap(uint64_t blocknum);

int32_t multipleDisksFilesBitmapToBuff(input_para_space_2_t &input_para, vector<BitMap *> &bitmap_vect);
int32_t gflpDoMutipleDisksFilesBitmap(partitionHandler &partsHandle, input_para_space_2_t input_para,
    vector<BitMap *> &bitmap_vect, int32_t part_num);
int32 afs_getFilesBitMap(imgReader *part_reader, struct partition *ppart, vector<BitMap *> &bitmap_vect,
    input_para_space_2_t input_para, string target_fs_uuid);
int32_t afs_getFSTypeUUID(imgReader *part_reader, int32_t *fs_type, string target_fs_uuid);
int32_t afs_convertFSUUID2String_1(char *fs_uuid, uint32_t uuid_len, char *fs_uuid_string, uint32_t uuid_string_len);
int32_t afs_convertFSUUID2String(char *fs_uuid, uint32_t uuid_len, char *fs_uuid_string, uint32_t uuid_string_len);

#endif

/**
 * @brief 获得多块磁盘空闲块的bitmap
 * @param handle[]              回调函数参数(镜像文件指针数组)
 * @param disk_num              磁盘个数
 * @param read_callback_func    回调函数
 * @param *bitmap_buffer[]      设置返回bitmap的buffer数组
 * @param buffer_size[]         bitmap的Buffer大小
 * @param bytes_per_bit         设置bitmap中一位表示多少字节
 * @return   0: 设置成功
 * 负数： 错误ID
 */
int32 getMulipleDisksFreeBlockBitmap(AFS_HANDLE handles[], int32 disk_num, AFS_READ_CALLBACK_FUNC_t read_callback_func,
    char *bitmap_buffer[], int64 buffer_size[], int32 bytes_per_bit, std::string &errMsg);

/**
 * @brief 获得多个磁盘的总分区数量
 * @param img_handles         多磁盘镜像文件指针
 * @param read_callback_func  回调函数
 * @return :  大于0 分区数量
 * 其他     错误ID
 */
int32 getMultipleDisksPartsNum(vector<AFS_HANDLE> img_handles, AFS_READ_CALLBACK_FUNC_t read_callback_func);

/**
 * @brief 获得磁盘分区数量
 * @param handle              回调函数参数(镜像文件指针)
 * @param read_callback_func  回调函数
 * @return:  大于0    分区数量
 * 其他     错误ID
 */
int32 getPartNum(AFS_HANDLE handle, AFS_READ_CALLBACK_FUNC_t read_callback_func);

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
    struct partition *part_info, int32 input_part_num);

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
    int32 input_part_num);

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
    int64 buffer_size, int32 bytes_per_bit);

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
    int32 part_index, char *bitmap_buffer, int64 buffer_size, int32 bytes_per_bit);

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
    int32 file_num, int32 part_index, char *bitmap_buffer, int64 buffer_size, int32 bytes_per_bit);

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
    char *bitmap_buffer[], int64 buffer_size[], int32 bytes_per_bit);

#ifdef __cplusplus
}
#endif

#endif // AFSLIBMAIN_H
