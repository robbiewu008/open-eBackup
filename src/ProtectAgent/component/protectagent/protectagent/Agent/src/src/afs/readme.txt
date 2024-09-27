AFS�ӿ�ʹ�÷�����

AFS�����ṩ���ֽӿڣ��ֱ����ڴ��̵���Ч����ʶ����ļ������ݿ�ʶ��

1.������Ч����ʶ��ӿڣ�
/**
 * @brief ��ö����̿��п��bitmap
 * @param handle[]              �ص���������(�����ļ�ָ������)
 * @param disk_num              ���̸���
 * @param read_callback_func    �ص�����
 * @param *bitmap_buffer[]      ���÷���bitmap��buffer����
 * @param buffer_size[]         bitmap��Buffer��С
 * @param bytes_per_bit         ����bitmap��һλ��ʾ�����ֽ�
 * @return   0: ���óɹ�
 *         ������ ����ID
 */
 int32 getMulipleDisksFreeBlockBitmap(AFS_HANDLE handles[],
                                     int32  disk_num,
                                     AFS_READ_CALLBACK_FUNC_t read_callback_func,
                                     char * bitmap_buffer[],
                                     int64 buffer_size[],
                                     int32 bytes_per_bit);

�������������̿��豸/dev/sdc��2G��,/dev/sdd��4G��,bytes_per_bit = 512���ֽڣ���������bytes_per_bit%512=0��
handles[0] ��fopen��Open��/dev/sdc���ָ��
handles[1] ��fopen��Open��/dev/sdd���ָ��
read_callback_func�ǿ��Զ�ȡ/dev/sdc�����ݵĻص�����
��������Ҫ��������bitmap_buffer,���ڴ��ÿ�����̵���Ч����λͼ��λͼ��1 bit ����bytes_per_bit�ֽڣ�1��ʾ�����ݿ�����Ч����
�ڱ����У�
bitmap_buffer[0]�ڴ��С = ��[2]*1024*1024*1024 + [512]*8 - 1��/��512 * 8�� = 512k �ֽ�
bitmap_buffer[1]�ڴ��С = ��[4]*1024*1024*1024 + [512]*8 - 1��/��512 * 8�� = 1M �ֽ�
disk_num = 2
buffer_size[0] = 2048(��λ��MB)��buffer_size[1] = 4096

2.�ļ������ݿ�ʶ��ӿڣ�
/**
 * @brief ������»���ļ��б���ռ���bitmap
 *
 * @param handles[]           �ص���������(�����ļ�ָ��)
 * @param disk_num            ���̸���
 * @param read_callback_func  �ص�����
 * @param *file_path[]        ����ҪѰ���ļ��б�ȫ·�������ô����ص�·����
 * @param files_num           �ļ�����
 * @param fs_uuid             �ļ��б������ļ�ϵͳ��UUID��������Ψһ�ķ�������Ѱָ���ļ�
 * @param *bitmap_buffer[]    �����ļ��б�����ݿ��ڶ������bitmap��Buffer
 * @param buffer_size[]       bitmap��Buffer��С
 * @param bytes_per_bit       ����bitmap��1λ��ʾ���ֽ���
 *
 * @return : 0�����óɹ�     ����������ʧ�ܣ�����ID��
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

������һ��LVM��LV����������������̿��豸/dev/sdc��2G��,/dev/sdd��2G��,LV�Ϲ��ص��ļ�ϵͳΪEXT4��XFS

handles[],disk_num,read_callback_func,bitmap_buffer[],buffer_size,bytes_per_bit�Ķ���ͬ�ӿ�1һ��
bitmap_buffer�� bit = 1����ʾ��λ��Ӧ�����ݿ���  file_path[] ���ļ�����Ч���ݿ�

����Ҫ��������LV��/mnt/data/dir0/test.txt(�ļ�)��/mnt/data/(Ŀ¼)����files_num = 2, 
*files_path[0] = "/mnt/data/dir0/test.txt" 
*files_path[1] = "/mnt/data/"
fs_uuid �������á�df -h����ù��ص�/mnt�ڻ��ԡ�Filesystem�����¶�Ӧ��xxx����ʹ�� blkid + xxx���ɵõ�UUID
����
[root@localhost ~]# df -h
Filesystem                                          Size  Used Avail Use% Mounted on
/dev/mapper/centos00-root                            28G   13G   15G  47% /
devtmpfs                                            1.9G     0  1.9G   0% /dev
/dev/sr0                                            4.2G  4.2G     0 100% /run/media/zhuyuanjie/CentOS 7 x86_64
tmpfs                                               380M     0  380M   0% /run/user/0
/dev/mapper/vg_3disks_thin_pools-thin_pool_client2  477M  444M     0 100% /mnt
Ȼ��
[root@localhost ~]# blkid /dev/mapper/vg_3disks_thin_pools-thin_pool_client2
/dev/mapper/vg_3disks_thin_pools-thin_pool_client2: UUID="fc87be8d-f07c-4695-aa1b-fff6ce2b29b9" TYPE="ext4"
���õ�*fs_uuid = "fc87be8d-f07c-4695-aa1b-fff6ce2b29b9"
