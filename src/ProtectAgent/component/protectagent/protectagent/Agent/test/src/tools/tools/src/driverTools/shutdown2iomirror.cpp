#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include "driver/linux/ctl_cmd.h"
#include "securec.h"
#include <sstream>
#include <iostream>
#include <vector>
#include <linux/fiemap.h>

using namespace std;

const int SUCCESS = 0;
const int FAILED  = -1;
const int FS_BLOCK_SIZE = 4096;
// 单独的工具需要单独定义
/*
/etc/huawei/im.conf
保存结构体struct im_config_pg，是保护时对应的信息，放到/etc目录下，系统启动时可以通过文件系统方式加载，
不能放到/boot目录，dirver启动时/boot目录未加载
/boot/hwsnap_bitmap
保存关机时卷的bitmap信息，关机时agent创建bitmap文件，把偏移发给driver，driver在关机前会把内存中的bitmap
通过磁盘偏移的方式写入文件中，只支持简单卷，只能放到/boot目录
/boot/huawei/im.conf
保存结构体struct im_config_variable，关机时agent创建配置文件，把偏移发给driver，driver在关机前会把内存中
结构体通过磁盘偏移的方式写入文件中，只支持简单卷，只能放到/boot目录
ps.因为/boot/hwsnap_bitmap需要和windows保持一致，/boot/huawei/im.conf的内容放到/boot/hwsnap_bitmap
不合适
*/
const std::string IM_CTL_DEVICE_PATH          = "/dev/im_ctldev";
const std::string IM_CTL_CONF_PATH            = "/etc/huawei/im.conf";
const std::string IM_BITMAP_FILE_PATH         = "/boot/hwsnap_bitmap";
const std::string IM_IOMIRROR_CONF_NAME       = "/boot/huawei/im.conf";
const std::string IM_IOMIRROR_CONF_PATH       = "/boot/huawei";
const std::string IM_BOOT_DIR                 = "/boot";

#define LOG_INFO  {cout << "[" << __FILE__ << ", " << __LINE__ << "] "
#define LOG_ENDL  endl;}

int ExecSystemCmd(const string& strCommand, vector<string>& strEcho);

enum {
    SINGLE_PV_LVM = 0,
    MUTIPLE_PV_LVM
};

enum {
    PHY_PARTTION_TYPE = 0,
    LVM_DEVICE_TYPE,
    OTHER_TYPE
};

class LVPosition {
public:
    LVPosition() {
        m_mountPoint = "";
        m_pvName = "";
        m_vgName = "";
        m_lvFullName = "";
        m_major = 0;
        m_minor = 0;
        m_peStart = 0;
        m_lvStartExtent = 0;
        m_vgExtentSize = 0;
        m_lvPhyOffsetToPv = 0;
    }

    ~LVPosition() {};

    unsigned long long GetLvPhysicalOffset();
    unsigned int GetMajor() {return m_major;}
    unsigned int GetMinor() {return m_minor;}
    void SetMountPoint(const string& mountPointStr) {m_mountPoint = mountPointStr;}

private:
    int GetVgName();
    int GetPeStart();
    int GetPVDeviceNo();
    int GetLVStartExtentAndPVName();
    int GetVGExtentSize();
    int ExecuteLvmCmd(const string& lvmCmdStr, vector<string>& echoInfoVector);
    int ParseEchoInfo(unsigned long long& var, const vector<string>& echoInfoVector);

    string m_mountPoint;
    string m_vgName;
    string m_lvFullName;
    string m_pvName;
    unsigned long long m_peStart;         // unit : bytes
    unsigned long long m_lvStartExtent;   // unit : extents
    unsigned long long m_vgExtentSize;    // unit : bytes
    unsigned long long m_lvPhyOffsetToPv; // unit : bytes
    unsigned int m_major;
    unsigned int m_minor;
};

/*------------------------------------------------------------
Description  : 获取LV卷相对于PV的偏移（此处LV卷只能是单PV）
Return       : 0：表示执行失败，其他：偏移值（单位：字节）
Modification : z00455045 2020/02/03
Others       :--------------------------------------------------------*/
unsigned long long LVPosition::GetLvPhysicalOffset()
{
    m_lvPhyOffsetToPv = 0;
    if (GetVgName() != SUCCESS) {
        LOG_INFO << "GetVgName() execute failed." << LOG_ENDL;
        return 0;
    }

    if (GetLVStartExtentAndPVName() != SUCCESS) {
        LOG_INFO << "GetLVStartExtentAndPVName() execute failed." << LOG_ENDL;
        return 0;
    }

    if (GetPeStart() != SUCCESS) {
        LOG_INFO << "GetPeStart() execute failed." << LOG_ENDL;
        return 0;
    }

    if (GetVGExtentSize() != SUCCESS) {
        LOG_INFO << "GetVGExtentSize() execute failed." << LOG_ENDL;
        return 0;
    }

    if (GetPVDeviceNo() != SUCCESS) {
        LOG_INFO << "GetPVDeviceNo() execute failed." << LOG_ENDL;
        return 0;
    }

    m_lvPhyOffsetToPv = m_peStart + m_lvStartExtent * m_vgExtentSize;
    LOG_INFO << "lv: " << m_lvFullName << " offset to dev: " << m_pvName << " is " << m_lvPhyOffsetToPv
        << " bytes." << LOG_ENDL;
    return m_lvPhyOffsetToPv;
}

// 获取boot分区所在PV的设备号
int LVPosition::GetPVDeviceNo()
{
    if (m_pvName == "") {
        LOG_INFO << "pv name is null, error." << LOG_ENDL;
        return FAILED;
    }

    const string LS_PV_DEV_NO = "ls -l " + m_pvName + " | awk '{print $5, $6}' | sed 's/ //g'";
    vector<string> echoInfoVector;
    if (ExecuteLvmCmd(LS_PV_DEV_NO, echoInfoVector) != SUCCESS) {
        return FAILED;
    }

    std::size_t found = echoInfoVector[0].find_last_of(",");
    if (found == string::npos) {
        LOG_INFO << "\"" << LS_PV_DEV_NO << "\" return info: [" << echoInfoVector[0] << "] format error." << LOG_ENDL;
        return FAILED;
    }

    string majorStr = echoInfoVector[0].substr(0, found);
    string minorStr = echoInfoVector[0].substr(found + 1);
    stringstream ss;
    ss << majorStr;
    ss >> m_major;
    ss.clear();
    ss << minorStr;
    ss >> m_minor;
    LOG_INFO << "device: " << m_pvName << " major no is " << m_major << ", minor no is " << m_minor << LOG_ENDL;
    return SUCCESS;
}

int LVPosition::ExecuteLvmCmd(const string& lvmCmdStr, vector<string>& echoInfoVector)
{
    if (lvmCmdStr == "") {
        LOG_INFO << "Input cmd : [" << lvmCmdStr << "] error." << LOG_ENDL;
        return FAILED;
    }

    if (ExecSystemCmd(lvmCmdStr, echoInfoVector) != SUCCESS) {
        LOG_INFO << "execute system cmd: [" << lvmCmdStr << "] failed." << LOG_ENDL;
        return FAILED;
    }

    if (echoInfoVector.size() == 0) {
        LOG_INFO << "system cmd: [" << lvmCmdStr << "] not return info." << LOG_ENDL;
        return FAILED;
    }

    return SUCCESS;
}

int LVPosition::GetVgName()
{
    if (m_mountPoint == "") {
        LOG_INFO << "not input mount point." << LOG_ENDL;
        return FAILED;
    }
    // df /boot | grep /boot | awk '{print $1}'
    const string DF_MOUNT_POINT = "df 2>/dev/null " + m_mountPoint + " | grep /dev/ | awk '{print $1}'";
    vector<string> echoInfoVector;
    if (ExecuteLvmCmd(DF_MOUNT_POINT, echoInfoVector) != SUCCESS) {
        return FAILED;
    }
    
    std::size_t found1 = echoInfoVector[0].find_last_of("/");
    std::size_t found2 = echoInfoVector[0].find_last_of("-");
    if (found1 == string::npos || found2 ==  string::npos || found2 <= found1 ) {
        LOG_INFO << "echo info:[" << echoInfoVector[0] << "] is not right." << LOG_ENDL;
        return FAILED;
    }
    m_lvFullName = echoInfoVector[0];
    m_vgName = echoInfoVector[0].substr(found1 + 1, found2 - found1 - 1);
    LOG_INFO << "The Vg name of mount point: " << m_mountPoint << " is \"" << m_vgName << "\"." << LOG_ENDL;;
    LOG_INFO << "The LV name od monut point: " << m_mountPoint << " is \"" << m_lvFullName << "\"." << LOG_ENDL;
    return SUCCESS;
}

int LVPosition::ParseEchoInfo(unsigned long long& var, const vector<string>& echoInfoVector)
{
    const string charB = "B";
    std::size_t found = echoInfoVector[0].find_last_of(charB);
    if (found == string::npos) {
        LOG_INFO << "Echo info: [" << echoInfoVector[0] << "] is not expect format."<< LOG_ENDL;
        return FAILED;
    }

    string sizeStr =  echoInfoVector[0].substr(0, found);
    stringstream ss(sizeStr);
    ss >> var;
    
    const unsigned int SECTORS_SIZE = 512;
    if (var % SECTORS_SIZE != 0) {
        LOG_INFO << "Return value is " << var << " bytes, not aligned with 512, error." << LOG_ENDL; 
        return FAILED;
    }

    return SUCCESS;
}

int LVPosition::GetPeStart()
{
    m_peStart = 0;
    if (m_pvName == "") {
        LOG_INFO << "pv name is null, error." << LOG_ENDL;
        return FAILED;
    }

    const string PVS_PE_START = "pvs 2>/dev/null -o pe_start --unit B " + m_pvName + " | grep B | sed 's/ //g'";
    vector<string> echoInfoVector;
    if (ExecuteLvmCmd(PVS_PE_START, echoInfoVector) != SUCCESS) {
        return FAILED;
    }

    if (ParseEchoInfo(m_peStart, echoInfoVector) != SUCCESS) {
        LOG_INFO << "parse echo info failed." << LOG_ENDL;
        return FAILED;
    }

    LOG_INFO << "pe start is " << m_peStart << LOG_ENDL;
    return SUCCESS;
}

int LVPosition::GetVGExtentSize()
{
    m_vgExtentSize = 0;
    if (m_vgName == "") {
        LOG_INFO << "vg name is null." << LOG_ENDL;
        return FAILED;
    }

    // vgs -o vg_extent_size --units B centos
    const string VGS_EXTENT_SIZE = "vgs 2>/dev/null -o vg_extent_size --unit B " + m_vgName +
        " | grep B | sed 's/ //g'";
    vector<string> echoInfoVector;
    if (ExecuteLvmCmd(VGS_EXTENT_SIZE, echoInfoVector) != SUCCESS) {
        return FAILED;
    }

    if (ParseEchoInfo(m_vgExtentSize, echoInfoVector) != SUCCESS) {
        LOG_INFO << "parse echo info failed." << LOG_ENDL;
        return FAILED;
    }

    LOG_INFO << "VG extent size is " << m_vgExtentSize << " bytes." << LOG_ENDL;
    return SUCCESS;
}

int LVPosition::GetLVStartExtentAndPVName()
{
    if (m_lvFullName == "") {
        LOG_INFO << "lv full name is null, error." << LOG_ENDL;
        return FAILED;
    }

    // lvs -o devices /dev/mapper/VolGroup-lv_swap | grep /dev/
    const string LVS_DEVICES = "lvs 2>/dev/null -o devices " + m_lvFullName + " | grep /dev/ | sed 's/ //g'";
    vector<string> echoInfoVector;
    if (ExecuteLvmCmd(LVS_DEVICES, echoInfoVector) != SUCCESS) {
        return FAILED;
    }

    std::size_t found1 = echoInfoVector[0].find_first_of("(");
    if (found1 == string::npos) {
        LOG_INFO << "lvs -o devices return info: [" << echoInfoVector[0] << "] format error." << LOG_ENDL;
        return FAILED;
    }

    m_pvName = echoInfoVector[0].substr(0, found1);
    LOG_INFO << "pv name is " << m_pvName << LOG_ENDL;

    std::size_t found2 = echoInfoVector[0].find_last_of(")");
    if (found2 == string::npos || found2 <= found1) {
        LOG_INFO << "lvs -o devices return info: [" << echoInfoVector[0] << "] format error." << LOG_ENDL;
        return FAILED;
    }

    string startExtentStr = echoInfoVector[0].substr(found1 + 1, found2 - found1 - 1);
    stringstream ss(startExtentStr);
    ss >> m_lvStartExtent;
    LOG_INFO << "lv start extent is " << m_lvStartExtent << LOG_ENDL;
    return SUCCESS;
}

int ExecSystemCmd(const string& strCommand, vector<string>& strEcho)
{
    FILE* pStream = popen(strCommand.c_str(), "r");
    if (NULL == pStream) {
        LOG_INFO << "popen failed." << LOG_ENDL;
        return FAILED;
    }

    while (!feof(pStream)) {
        char tmpBuf[1000] = {0};
        char* cRet = fgets(tmpBuf, sizeof(tmpBuf), pStream);
        if (NULL == cRet) {
        }
        if (strlen(tmpBuf) >= 0) {
            tmpBuf[strlen(tmpBuf) - 1] = 0;  // 去掉获取出来的字符串末尾的'\n'
        }

        bool bFlag = (tmpBuf[0] == 0) || (tmpBuf[0] == '\n');
        if (bFlag) {
            continue;
        }

        strEcho.push_back(string(tmpBuf));
    }

    (void)pclose(pStream);
    return SUCCESS;
}

unsigned int GetFileExtentsNum(int fd)
{
    struct fiemap extentBitmap;
    if (memset_s(&extentBitmap, sizeof(extentBitmap), 0, sizeof(extentBitmap)) != EOK) {
        LOG_INFO << "memset_s failed." << LOG_ENDL;
        return 0;
    }

    extentBitmap.fm_flags = FIEMAP_FLAG_SYNC;
    extentBitmap.fm_start = 0;
    extentBitmap.fm_length = FIEMAP_MAX_OFFSET;

    if (ioctl(fd, FS_IOC_FIEMAP, &extentBitmap) < 0) {
        LOG_INFO << "FS_IOC_FIEMAP ioctl failed, error: " << errno << strerror(errno) << LOG_ENDL;
        return 0;
    }

    LOG_INFO << "There is " << extentBitmap.fm_mapped_extents << " extents." << LOG_ENDL;
    return extentBitmap.fm_mapped_extents;
}

unsigned int GetBlocksOffsetToPv(struct fiemap pFiemap[], unsigned long long lvPhyOffset, 
    struct im_ctl_bitmap_extent extent[], long long fileSize)
{
    bool hasLastExtents = false;
    long long tempFileSize = 0;
    unsigned int extentNum = 0;
    struct fiemap_extent * pExtent = &pFiemap->fm_extents[0];
    for (unsigned int i = 0; i < pFiemap->fm_mapped_extents; i++) {
        if (pExtent[i].fe_flags != 0 && pExtent[i].fe_flags != FIEMAP_EXTENT_LAST) {
            LOG_INFO << "The " << "th extent flag is " << pExtent[i].fe_flags << ", error!." << LOG_ENDL;
            return 0;
        }
        LOG_INFO << "The " << i << "th extent logical offset is " << pExtent[i].fe_logical << LOG_ENDL;
        LOG_INFO << "The " << i << "th extent physical offset is " << pExtent[i].fe_physical << ", length is " 
            << pExtent[i].fe_length << " bytes." << LOG_ENDL;
        if (pExtent[i].fe_length % FS_BLOCK_SIZE != 0) {
            LOG_INFO << "The " << i << "th extent is not Aligned with " << FS_BLOCK_SIZE << LOG_ENDL
            return 0;
        }

        extent[i].offset = pExtent[i].fe_physical + lvPhyOffset;
        extent[i].length = pExtent[i].fe_length;
        if (pExtent[i].fe_flags == FIEMAP_EXTENT_LAST) {
            hasLastExtents = true;
            LOG_INFO << "The last extents has been found, i = " << i << LOG_ENDL;
            extent[i].length = fileSize - tempFileSize;
            extentNum = i + 1;
            break;
        }

        tempFileSize += pExtent[i].fe_length;
        if (tempFileSize >= fileSize) {
            LOG_INFO << "count file size is " << tempFileSize << ", exceed the real file size " << fileSize << LOG_ENDL;
            return 0;
        }
    }

    if (!hasLastExtents) {
        LOG_INFO << "The last extents has not been found, error." << LOG_ENDL;
        return 0;
    }

    return extentNum;
}

struct fiemap* InitFiemap(int fd, unsigned int extentsNum)
{
    unsigned int size = sizeof(struct fiemap_extent) * extentsNum + sizeof(struct fiemap);
    struct fiemap* pFiemap = (struct fiemap *)malloc(size);
    if (pFiemap == NULL) {
        LOG_INFO << "malloc for struct fiemap failed." << LOG_ENDL;
        return NULL;
    }

    if (memset_s(pFiemap, size, 0, size) != EOK) {
        LOG_INFO << "memset_s failed." << LOG_ENDL;
        return NULL;
    }
    pFiemap->fm_flags = FIEMAP_FLAG_SYNC;
    pFiemap->fm_start = 0;
    pFiemap->fm_length = FIEMAP_MAX_OFFSET;
    pFiemap->fm_extent_count = extentsNum;

    if (ioctl(fd, FS_IOC_FIEMAP, pFiemap) < 0) {
        LOG_INFO << "FS_IOC_FIEMAP ioctl failed, error:" << errno << strerror(errno) << LOG_ENDL;
        free(pFiemap); // 此处会释放已分配的资源
        return NULL;
    }
    
    return pFiemap;
}

int GetExtentByFIEMAP(int fd, struct im_ctl_bitmap_extent_setting extentSetting[], long long fileSize)
{
    unsigned int extentsNum = GetFileExtentsNum(fd);
    if (extentsNum == 0) {
        LOG_INFO << "Get file extents num failed." << LOG_ENDL;
        return FAILED;
    }

    struct fiemap *pFiemap = InitFiemap(fd, extentsNum);
    if (pFiemap == NULL) {
        LOG_INFO << "Init fiemap failed." << LOG_ENDL;
        return FAILED;
    }

    LOG_INFO << "Get total " << pFiemap->fm_mapped_extents << " extents." << LOG_ENDL;

    unsigned int extentSize = sizeof(struct im_ctl_bitmap_extent) * pFiemap->fm_mapped_extents;
    struct im_ctl_bitmap_extent* extent = (struct im_ctl_bitmap_extent *)malloc(extentSize);
    if (extent == NULL) {
        LOG_INFO << "malloc failed." << LOG_ENDL;
        free(pFiemap);
        return FAILED;
    }

    if (memset_s(extent, extentSize, 0, extentSize) != EOK) {
        LOG_INFO << "memset_s failed." << LOG_ENDL;
        return FAILED;
    }
    extentSetting->data = extent; // extent在函数外进行释放，释放前需要判断非空

    LVPosition lvPos;
    lvPos.SetMountPoint(IM_BOOT_DIR);
    unsigned long long lvPhyOffset = lvPos.GetLvPhysicalOffset();

    if (lvPhyOffset == 0) {
        LOG_INFO << "GetLvPhysicalOffset() failed." << LOG_ENDL;
        free(pFiemap);
        return FAILED;
    }

    extentSetting->extent_num = GetBlocksOffsetToPv(pFiemap, lvPhyOffset, extent, fileSize);
    if (extentSetting->extent_num != extentsNum) {
        LOG_INFO << "GetBlocksOffsetToPv() failed." << LOG_ENDL;
        free(pFiemap);
        return FAILED;
    }

    extentSetting->dev_major = lvPos.GetMajor();
    extentSetting->dev_minor = lvPos.GetMinor();
    free(pFiemap);
    LOG_INFO << "Get extent by file extent map success." << LOG_ENDL;
    return SUCCESS;
}

/*------------------------------------------------------------
Function Name: dir_exist
Description  : 判断指定路径文件夹是否存在
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
int DirExist(const char pszDirPath[])
{
    struct stat fileStat;
    int iRet = memset_s(&fileStat, sizeof(fileStat), 0, sizeof(fileStat));
    if (iRet != 0) {
        printf("memset_s failed, iRet=%d", iRet);
        return FAILED;
    }
    
    if (0 != stat(pszDirPath, &fileStat)) {
        return FAILED;
    }

    return SUCCESS;
}

/*------------------------------------------------------------
Function Name: CreateDir
Description  : 创建文件夹,暂未实现循环创建文件目录
                    CreateDir only supports the creation of directory in the existing directory.
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
int CreateDir(const char pszDirPath[])
{
    if (pszDirPath == NULL) {
        printf("dir path is null.");
        return FAILED;
    }

    if (mkdir(pszDirPath, S_IRWXU) != 0) {
        printf("Failed to create folder:%s, errno[%d].", pszDirPath, errno);
        return FAILED;
    }

    return SUCCESS;
}

/*------------------------------------------------------------
Function Name: file_exists
Description  : 判断指定路径文件是否存在
Return       :  0 exist, no zero: not exits
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
int FileExists(const char pszFilePath[])
{
    struct stat fileStat;
    int iRet = memset_s(&fileStat, sizeof(fileStat), 0, sizeof(fileStat));
    if (iRet != 0) {
        printf("memset_s failed, iRet=%d", iRet);
        return -1;
    }
    if (0 != stat(pszFilePath, &fileStat)) {
        return -1;
    }

    return 0;
}

/*------------------------------------------------------------
Function Name: delete_file
Description  : 删除文件
Return       :  0 success, no zero failed
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
int DeleteFile(const char pszFilePath[])
{
    if (pszFilePath == NULL) {
        return SUCCESS;
    }
    if (FileExists(pszFilePath) != 0) {
        return SUCCESS;
    }

    int iCheckNotOkRet = remove(pszFilePath);
    if (iCheckNotOkRet != 0) {
        printf("remove file failed, errno %d.", errno);
        return FAILED;
    }

    return SUCCESS;
}


/*------------------------------------------------------------
Description  : 通过ioctl下发命令
Input        :
Output       :
Return       : 0 -- 成功
               非0-- 失败
Create By    :
Modification :
-------------------------------------------------------------*/
int DoIoctl(unsigned int cmd, void* data)
{
    int fd = open(IM_CTL_DEVICE_PATH.c_str(), O_RDWR);
    if (fd < 0) {
        printf("open im_ctldev failed, error:%d.\n", errno);
        return FAILED;
    }
    if (ioctl(fd, cmd, data) < 0) {
        printf("ioctl failed, error:%d.\n", errno);
        close(fd);
        return FAILED;
    }
    close(fd);
    return SUCCESS;
}

// -1: 判断失败 0：物理分区 1：LVM分区 2： 其他
int BootMountType(const string dirStr, string& lvName)
{
    const string DF_MOUNT_POINT = "df 2>/dev/null " + dirStr + " | grep /dev/" + " | awk '{print $1}'";
    vector<string> echoInfoVector;
    if (ExecSystemCmd(DF_MOUNT_POINT, echoInfoVector) != SUCCESS) {
        LOG_INFO << "execute system cmd: [" << DF_MOUNT_POINT << "] failed." << LOG_ENDL;
        return FAILED;
    }

    if (echoInfoVector.size() == 0) {
        LOG_INFO << "system cmd: [" << DF_MOUNT_POINT << "] not return info." << LOG_ENDL;
        return FAILED;
    }

    LOG_INFO << "df " + dirStr << " - device name is " << echoInfoVector[0] << LOG_ENDL;
    // /dev/vda1, /dev/xvda1, /dev/sda1
    const int minPhyValidLen = 8;  // 至少是/dev/vda这种长度
    int len = echoInfoVector[0].size();
    if (len < minPhyValidLen) {
        LOG_INFO << dirStr << " mounted device type is " << OTHER_TYPE << LOG_ENDL;
        return OTHER_TYPE;
    }

    const string lvmDevPrefix = "/dev/mapper/";
    const int minLvmLen = lvmDevPrefix.size();
    // /dev/mapper/centos-root
    std::size_t found = echoInfoVector[0].find_last_of("-");
    bool isIncludeLetter = (found != std::string::npos);
    if (len > minLvmLen && echoInfoVector[0].substr(0, minLvmLen) == lvmDevPrefix && isIncludeLetter) {
        LOG_INFO << dirStr << " mounted device type is " << LVM_DEVICE_TYPE << LOG_ENDL;
        lvName = echoInfoVector[0]; // 找到LV name
        return LVM_DEVICE_TYPE;
    }

    LOG_INFO << dirStr << " mounted device type is " << PHY_PARTTION_TYPE << LOG_ENDL;
    return PHY_PARTTION_TYPE;
}

// 判断lv所在的VG是只有一个PV
int isSinglePvLVM(const string& lvName)
{
    if (lvName == "") {
        LOG_INFO << "lv name is empty." << LOG_ENDL;
        return FAILED;
    }

    // count the number underlying devices used by lvName
    const string LVS_DEVICES = "lvs 2>/dev/null -o devices " + lvName + " | grep /dev/ | wc -l"; 
    vector<string> echoInfoVector;
    if (ExecSystemCmd(LVS_DEVICES, echoInfoVector) != SUCCESS) {
        LOG_INFO << "execute system cmd: [" << LVS_DEVICES << "] failed." << LOG_ENDL;
        return FAILED;
    }

    if (echoInfoVector.size() == 0) {
        LOG_INFO << "system cmd: [" << LVS_DEVICES << "] not return info." << LOG_ENDL;
        return FAILED;
    }

    LOG_INFO << "system cmd: [" << LVS_DEVICES << "] return devices number is " << echoInfoVector[0] << LOG_ENDL;

    stringstream ss;
    int pvCount = 0;
    ss << echoInfoVector[0];
    ss >> pvCount;
    const int singlePvCount = 1;
    if (pvCount != singlePvCount) {
        return MUTIPLE_PV_LVM;
    }

    return SINGLE_PV_LVM;
}

int GetPhyParttionExtent(int blkcnt, int fd, struct im_ctl_bitmap_extent_setting extentSetting[], int blocksize,
    struct stat bootStat)
{
    int pos = 0;
    unsigned int extentSize = sizeof(struct im_ctl_bitmap_extent) * blkcnt;
    struct im_ctl_bitmap_extent* extent = (struct im_ctl_bitmap_extent *)malloc(extentSize);
    if (extent == NULL) {
        printf("malloc failed.\n");
        return FAILED;
    }
    memset_s(extent, extentSize, 0, extentSize);

    for (int i = 0; i < blkcnt; i++) {
        int block = i;
        // 获取文件的第i个block的数据块位置
        if (ioctl(fd, FIBMAP, &block) < 0) {
            printf("FIBMAP ioctl failed, i:%d, error:%d.\n", i, errno);
            free(extent);
            return FAILED;
        }

        // 需要分析每个数据块的位置，如果不是连续的数据块
        // 就需要新记录一个im_ctl_bitmap_extent
        // 每个im_ctl_bitmap_extent都记录文件的数据块的位置以及长度
        if (i == 0) {
            extent[0].offset = static_cast<uint64_t>(block * blocksize);
            extent[0].length = static_cast<uint32_t>(blocksize);
        } else if((extent[pos].offset + extent[pos].length) != static_cast<uint64_t>(block * blocksize)) {
            pos += 1;
            extent[pos].offset = static_cast<uint64_t>(block * blocksize);
            extent[pos].length = static_cast<uint32_t>(blocksize);
        } else {
            extent[pos].length += static_cast<uint32_t>(blocksize);
        }
    }

    pos += 1;
    for (int i = 0; i < pos; ++i) {
        printf("%d, %llu, %u.\n", i, extent[i].offset, extent[i].length);
    }

    printf("extent num %d.\n", pos);

    extentSetting->extent_num = pos;
    extentSetting->data = extent;
    extentSetting->dev_major = major(bootStat.st_dev);
    extentSetting->dev_minor = minor(bootStat.st_dev);

    return SUCCESS;
}

int GetExtent(int blkcnt, int fd, struct im_ctl_bitmap_extent_setting extentSetting[], int blocksize,
    struct stat bootStat)
{
    string lvName;
    int iRet = BootMountType(IM_BOOT_DIR, lvName);
    if (iRet == OTHER_TYPE || iRet == FAILED) {
        LOG_INFO << "do not support device type." << LOG_ENDL;
        return FAILED;
    }

    if (iRet == PHY_PARTTION_TYPE) {
        return GetPhyParttionExtent(blkcnt, fd, extentSetting, blocksize, bootStat);
    }else if (isSinglePvLVM(lvName) == SINGLE_PV_LVM) {
        LOG_INFO << lvName << " is a " << iRet << " type." << LOG_ENDL;
        return GetExtentByFIEMAP(fd, extentSetting, bootStat.st_size);
    } else {
        LOG_INFO << IM_BOOT_DIR << "mounted device is multiple PV." << LOG_ENDL;
        return FAILED;
    }
}

/*------------------------------------------------------------
Description  : 根据文件名称获取文件所在的磁盘位置
Input        : file_size 创建文件的初始大小，以4K为一个单位(选择的一个大小)
Output       :
Return       : 0 -- 成功
               非0-- 失败
Create By    :
Modification :
-------------------------------------------------------------*/
int GetDiskOffset(const char fileName[], struct im_ctl_bitmap_extent_setting extentSetting[], 
    unsigned int fileSize)
{
    int blocksize;
    int ret = FAILED;
    int blkcnt;
    struct stat bootStat;
    char buf[FS_BLOCK_SIZE];

    if (!extentSetting || !fileName || fileSize == 0) {
        printf("extent_setting or fd is invalid, file_size %u.\n", fileSize);
        return FAILED;
    }

    int fd = open(fileName, O_RDWR | O_CREAT | O_TRUNC);
    if (fd < 0) {
        printf("open %s failed, error:%d.\n", fileName, errno);
        return FAILED;
    }

    memset_s(buf, sizeof(buf), 0, sizeof(buf));
    memset_s(&bootStat, sizeof(bootStat), 0, sizeof(bootStat));
    for (unsigned int i = 0; i < fileSize; i++) {
        if (write(fd, buf, sizeof(buf)) != sizeof(buf)) {
            goto end;
        }
    }

    // Get the block size of the host file-system for the image file by calling
    if (ioctl(fd, FIGETBSZ, &blocksize) < 0) {
        printf("FIBMAP ioctl failed, error:%d.\n", errno);
        goto end;
    }

    if (fstat(fd, &bootStat) != 0) { // get file size
        printf("fstat failed, error:%d.\n", errno);
        goto end;
    }

    // 计算Block count number
    blkcnt = (bootStat.st_size + blocksize - 1) / blocksize;
    printf("File %s: block num is %d, blocksize is %d, stat blk=%ld.\n", fileName, blkcnt, blocksize,
        static_cast<long>(bootStat.st_blocks));

    if (GetExtent(blkcnt, fd, extentSetting, blocksize, bootStat) == FAILED) {
        printf("get extent failed.\n");
        goto end;
    }

    printf("boot partition: major %u, minor %u.\n", extentSetting->dev_major, extentSetting->dev_minor);

    ret = SUCCESS;
end:
    close(fd);
    return ret;
}

/*------------------------------------------------------------
Description  : 更新保护的配置文件内容，用于driver启动时获取保护信息
Input        :
Output       :
Return       : 0 -- 成功
               非0-- 失败
Create By    :
Modification :
-------------------------------------------------------------*/
int UpdateConfigFile(unsigned int devMajor, unsigned int devMinor, unsigned long long offset)
{
    int ret = FAILED;
    struct im_config_pg config;

    int fd = open(IM_CTL_CONF_PATH.c_str(), O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("open %s failed.\n", IM_CTL_CONF_PATH.c_str());
        return FAILED;
    }
    if (read(fd, &config, sizeof(config)) != sizeof(config)) {
        printf("read %s failed.\n", IM_CTL_CONF_PATH.c_str());
        goto end;
    }

    if (lseek(fd, 0, SEEK_SET) < 0) {
        printf("lseek %s failed.\n", IM_CTL_CONF_PATH.c_str());
        goto end;
    }

    config.bitmap_dev_major = devMajor;
    config.bitmap_dev_minor = devMinor;
    config.bitmap_dev_offset = offset;
    if (write(fd, &config, sizeof(config)) != sizeof(config)) {
        printf("write %s failed.\n", IM_CTL_CONF_PATH.c_str());
        goto end;
    }

    ret = SUCCESS;
end:
    close(fd);
    return ret;
}

/*------------------------------------------------------------
Description  : 生成一个bitmap文件，并统计block的偏移量和长度，
                通过ioctl通知driver，driver可以写bitmap到文件中
                driver不能访问文件系统，只能访问磁盘的数据块
                理解可以参考http://blog.osdba.net/536.html
Input        :
Output       :
Return       : 0 -- 成功
               非0-- 失败
Create By    :
Modification :
-------------------------------------------------------------*/
int CreateBitmapFile()
{
    int ret = FAILED;
    unsigned int cmd = IM_CTL_SETBITMAPEXTENT;
    struct im_ctl_bitmap_extent_setting extentSetting;
    // bitmap默认创建的是4M文件块
    static unsigned int bitmapSize;
    ProtectVolSize protectVolumeSize;
    const int bitmapUnit = 4096;
    uint32_t i;

    protectVolumeSize.disk_num = 0;
    protectVolumeSize.bitmap_size = 0;

    // first get the bitmap size
    if (DoIoctl(IM_CTL_GET_VOLUMES_SIZE, &protectVolumeSize) != SUCCESS) {
        printf("ioctl IM_CTL_GET_VOLUMES_SIZE failed.\n");
        goto end;
    }

    printf("get protected volume size successful, disk_num is %u, bitmap_size is %llu.\n",
           protectVolumeSize.disk_num, protectVolumeSize.bitmap_size);

    // get bitmap_size that is integer multiple of 4096 bytes
    bitmapSize = (protectVolumeSize.bitmap_size + bitmapUnit - 1) / bitmapUnit;

    // get disk offset of file
    ret = GetDiskOffset(IM_BITMAP_FILE_PATH.c_str(), &extentSetting, bitmapSize);
    if (ret != SUCCESS) {
        printf("get_diskoffset of bitmap file failed.\n");
        DeleteFile(IM_BITMAP_FILE_PATH.c_str());
        goto end;
    }
    for (i = 0; i < extentSetting.extent_num; ++i) {
        printf("offset:%llu, length:%u\n", extentSetting.data[i].offset, extentSetting.data[i].length);
    }

    // 将bitmap的磁盘数据块发给driver
    if (DoIoctl(cmd, &extentSetting) != SUCCESS) {
        printf("ioctl IM_CTL_BACKUP_SETBITMAPEXTENT failed.\n");
        DeleteFile(IM_BITMAP_FILE_PATH.c_str());
        goto end;
    }

    // 更新保护的配置文件中的磁盘major和minor信息
    if (UpdateConfigFile(extentSetting.dev_major, extentSetting.dev_minor, extentSetting.data[0].offset) 
        != SUCCESS) {
        goto end;
    }

    if (extentSetting.data) {
        free(extentSetting.data);
    }
    ret = SUCCESS;
end:
    return ret;
}

/*------------------------------------------------------------
Description  : 设置变量文件的位置到driver中，用于driver写flushtime等数据
Input        :
Output       :
Return       : 0 -- 成功
               非0-- 失败
Create By    :
Modification :
-------------------------------------------------------------*/
int SetConfigfileExtent()
{
    unsigned int cmd = IM_CTL_SETCONFIG_EXTENT;
    struct im_ctl_bitmap_extent_setting extentSetting;
    // bitmap默认创建的是4M文件块
    static unsigned int configSize = 1;
    uint32_t i;
    
    if (-1 == DirExist(IM_IOMIRROR_CONF_PATH.c_str())) {
        CreateDir(IM_IOMIRROR_CONF_PATH.c_str());
    }

    // get disk offset of file
    int ret = GetDiskOffset(IM_IOMIRROR_CONF_NAME.c_str(), &extentSetting, configSize);
    if (ret != SUCCESS) {
        printf("get_diskoffset of config file failed.\n");
        DeleteFile(IM_IOMIRROR_CONF_NAME.c_str());
        goto end;
    }

    for (i = 0; i < extentSetting.extent_num; ++i) {
        printf("offset:%llu, length:%u\n", extentSetting.data[i].offset, extentSetting.data[i].length);
    }

    // 将bitmap的磁盘数据块发给driver
    if (DoIoctl(cmd, &extentSetting) != SUCCESS) {
        printf("ioctl IM_CTL_SETCONFIG_EXTENT failed.\n");
        DeleteFile(IM_IOMIRROR_CONF_NAME.c_str());
        goto end;
    }

    if (extentSetting.data) {
        free(extentSetting.data);
    }

    ret = SUCCESS;
end:
    return ret;
}
int main(int argc, char** agrv)
{
    (void)argc;
    (void)agrv;
    // 如果配置文件没有，则不需要设置信息
    if (FileExists(IM_CTL_CONF_PATH.c_str()) != 0) {
        printf("config file is not exists, do not create bitmap.\n");
        return SUCCESS;
    }

    int stopSendCmd = IM_CTL_STOP_SEND_DATA;
    if (DoIoctl(stopSendCmd, NULL) != SUCCESS) {
        printf("ioctl IM_CTL_STOP_SEND_DATA failed.\n");
        return FAILED;
    }
    printf("send IM_CTL_STOP_SEND_DATA succ.\n");

    if (CreateBitmapFile() != SUCCESS) {
        printf("create_bitmap_file failed.\n");
        return FAILED;
    }

    if (SetConfigfileExtent() != SUCCESS) {
        printf("set_configfile_extent failed.\n");
        return FAILED;
    }

    return SUCCESS;
}
