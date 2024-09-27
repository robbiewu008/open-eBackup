AFS接口使用方法：

AFS对外提供两种接口，分别用于磁盘的无效数据识别和文件的数据块识别

1.磁盘无效数据识别接口：
/**
 * @brief 获得多块磁盘空闲块的bitmap
 * @param handle[]              回调函数参数(镜像文件指针数组)
 * @param disk_num              磁盘个数
 * @param read_callback_func    回调函数
 * @param *bitmap_buffer[]      设置返回bitmap的buffer数组
 * @param buffer_size[]         bitmap的Buffer大小
 * @param bytes_per_bit         设置bitmap中一位表示多少字节
 * @return   0: 设置成功
 *         负数： 错误ID
 */
 int32 getMulipleDisksFreeBlockBitmap(AFS_HANDLE handles[],
                                     int32  disk_num,
                                     AFS_READ_CALLBACK_FUNC_t read_callback_func,
                                     char * bitmap_buffer[],
                                     int64 buffer_size[],
                                     int32 bytes_per_bit);

假设有两个磁盘块设备/dev/sdc（2G）,/dev/sdd（4G）,bytes_per_bit = 512（字节，必须满足bytes_per_bit%512=0）
handles[0] 是fopen或Open打开/dev/sdc后的指针
handles[1] 是fopen或Open打开/dev/sdd后的指针
read_callback_func是可以读取/dev/sdc中数据的回调函数
调用者需要申请两个bitmap_buffer,用于存放每个磁盘的无效数据位图，位图中1 bit 代表bytes_per_bit字节，1表示该数据块是有效数据
在本例中：
bitmap_buffer[0]内存大小 = （[2]*1024*1024*1024 + [512]*8 - 1）/（512 * 8） = 512k 字节
bitmap_buffer[1]内存大小 = （[4]*1024*1024*1024 + [512]*8 - 1）/（512 * 8） = 1M 字节
disk_num = 2
buffer_size[0] = 2048(单位：MB)，buffer_size[1] = 4096

2.文件级数据块识别接口：
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
int32 getMulipleDisksFileListBlockBitmap(AFS_HANDLE handles[],
                    int32_t disk_num,
                    AFS_READ_CALLBACK_FUNC_t read_callback_func,
                    char *file_path[],
                    int32_t files_num,
                    char *fs_uuid,
                    char *bitmap_buffer[],
                    int64 buffer_size[],
                    int32 bytes_per_bit);

假设有一个LVM的LV卷，它横跨了两个磁盘块设备/dev/sdc（2G）,/dev/sdd（2G）,LV上挂载的文件系统为EXT4或XFS

handles[],disk_num,read_callback_func,bitmap_buffer[],buffer_size,bytes_per_bit的定义同接口1一致
bitmap_buffer中 bit = 1，表示该位对应的数据块是  file_path[] 中文件的有效数据块

假设要分析的是LV下/mnt/data/dir0/test.txt(文件)、/mnt/data/(目录)，则files_num = 2, 
*files_path[0] = "/mnt/data/dir0/test.txt" 
*files_path[1] = "/mnt/data/"
fs_uuid 可以先用“df -h”获得挂载点/mnt在回显“Filesystem”列下对应的xxx，再使用 blkid + xxx即可得到UUID
例：
[root@localhost ~]# df -h
Filesystem                                          Size  Used Avail Use% Mounted on
/dev/mapper/centos00-root                            28G   13G   15G  47% /
devtmpfs                                            1.9G     0  1.9G   0% /dev
/dev/sr0                                            4.2G  4.2G     0 100% /run/media/zhuyuanjie/CentOS 7 x86_64
tmpfs                                               380M     0  380M   0% /run/user/0
/dev/mapper/vg_3disks_thin_pools-thin_pool_client2  477M  444M     0 100% /mnt
然后：
[root@localhost ~]# blkid /dev/mapper/vg_3disks_thin_pools-thin_pool_client2
/dev/mapper/vg_3disks_thin_pools-thin_pool_client2: UUID="fc87be8d-f07c-4695-aa1b-fff6ce2b29b9" TYPE="ext4"
最后得到*fs_uuid = "fc87be8d-f07c-4695-aa1b-fff6ce2b29b9"
