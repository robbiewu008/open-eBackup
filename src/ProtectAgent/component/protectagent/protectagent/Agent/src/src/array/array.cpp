#include "array/array.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/Defines.h"
#include "common/MpString.h"
#include "securecom/UniqueId.h"
#include "common/Path.h"
#include "securecom/RootCaller.h"
#include "common/CSystemExec.h"
using namespace std;

Array::Array()
{}

Array::~Array()
{}

// Description  :获取 通道的设备列表
// Input        :strDevice -- 设备名称
// Return       : MP_SUCCESS -- 成功
//                非MP_SUCCESS -- 失败，返回特定错误码
mp_int32 Array::GetHostLunIDInfo(const mp_string& strDevice, vector<mp_int32>& vecHostLunID)
{
#ifndef WIN32
    vector<mp_string> vecResult;
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_HOSTLUNID, strDevice, &vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get host LunID of device failed.");
        return iRet;
    }
    if (vecResult.empty()) {
        COMMLOG(OS_LOG_ERROR, "Get host LunID of device failed.");
        return MP_FAILED;
    }
    for (vector<mp_string>::iterator iter = vecResult.begin(); iter != vecResult.end(); ++iter) {
        mp_int32 temp = atoi((*iter).c_str());
        vecHostLunID.push_back(temp);
    }
#endif
    return MP_SUCCESS;
}

// windows平台
#ifdef WIN32
/* ------------------------------------------------------------
Description  : 打开设备
Return       :  MP_SUCCESS---打开成功
------------------------------------------------------------- */
mp_int32 Array::OpenDev(const mp_string& strDev, HANDLE& handle)
{
    return MP_SUCCESS;
}
/* ------------------------------------------------------------
Description  : 得到设备handle
Return       :  MP_SUCCESS---获取成功
               MP_FAILED---获取失败
------------------------------------------------------------- */
mp_int32 Array::GetDevHandle(const mp_string& pszDeviceName, FILE_HANDLE& pHandle)
{
    DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    DWORD dwAccessMode = GENERIC_READ | GENERIC_WRITE;

    // 调用CreateFile得到设备handle
    FILE_HANDLE handletemp = CreateFile(pszDeviceName.c_str(), dwAccessMode, dwShareMode, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == handletemp) {
        COMMLOG(OS_LOG_ERROR, "CreateFile handle is invalid,device(%s)", pszDeviceName.c_str());

        return MP_FAILED;
    }
    pHandle = handletemp;

    return MP_SUCCESS;
}
/* ------------------------------------------------------------
Description  : 得到设备SCSI地址信息
Input        : fHandle---SCSI文件handle
Output       : pstDevInfo---硬盘设备信息
Return       :  MP_SUCCESS---获取成功
               MP_FAILED---获取失败
------------------------------------------------------------- */
mp_int32 Array::GetDevSCSIAddress(FILE_HANDLE fHandle, win_dev_info_t& pstDevInfo)
{
    COMMLOG(OS_LOG_DEBUG, "Begin GetDiskSCSIAddress.");

    mp_ulong ulReturn = 0;
    SCSI_ADDRESS stScsiAddress;

    stScsiAddress.Length = sizeof(SCSI_ADDRESS);

    mp_int32 iStatus = DeviceIoControl(fHandle, IOCTL_SCSI_GET_ADDRESS, &stScsiAddress,
        sizeof(stScsiAddress), &stScsiAddress, sizeof(stScsiAddress), &ulReturn, NULL);
    if (!iStatus) {
        COMMLOG(OS_LOG_ERROR, "GetDiskSCSIAddress io error(%d)", iStatus);
        return MP_FAILED;
    }

    pstDevInfo.iPathId = stScsiAddress.PathId;
    pstDevInfo.iPortId = stScsiAddress.PortNumber;
    pstDevInfo.iScsiId = stScsiAddress.TargetId;
    pstDevInfo.iLunId = stScsiAddress.Lun;

    COMMLOG(OS_LOG_DEBUG, "GetDiskSCSIAddress io end");
    return MP_SUCCESS;
}
/* ------------------------------------------------------------
Description  :设置设备SCSI缓存值
Return       :  MP_SUCCESS---获取成功
               MP_FAILED---获取失败
------------------------------------------------------------- */
mp_void Array::SetScsiBufValues(
    scsi_pass_through_with_buff_t& stSCSIPass, mp_ulong& ulLength, mp_uchar ucCdb, mp_uchar ucCmd)
{
    stSCSIPass.pt.Length = sizeof(SCSI_PASS_THROUGH);
    stSCSIPass.pt.CdbLength = ARRAY_NUM_6;
    stSCSIPass.pt.SenseInfoLength = ARRAY_NUM_32;
    stSCSIPass.pt.DataIn = SCSI_IOCTL_DATA_IN;
    stSCSIPass.pt.DataTransferLength = 0xff;
    stSCSIPass.pt.TimeOutValue = ARRAY_NUM_60;
    stSCSIPass.pt.DataBufferOffset = offsetof(scsi_pass_through_with_buff_t, aucData);
    stSCSIPass.pt.SenseInfoOffset = offsetof(scsi_pass_through_with_buff_t, aucSense);
    stSCSIPass.pt.Cdb[0] = SCSIOP_INQUIRY;
    stSCSIPass.pt.Cdb[1] = ucCdb;
    stSCSIPass.pt.Cdb[ARRAY_INDEX_2] = ucCmd;
    stSCSIPass.pt.Cdb[ARRAY_INDEX_3] = 0x00;
    stSCSIPass.pt.Cdb[ARRAY_INDEX_4] = 0xff;
    stSCSIPass.pt.Cdb[ARRAY_INDEX_5] = 0x00;

    ulLength = offsetof(scsi_pass_through_with_buff_t, aucData) + stSCSIPass.pt.DataTransferLength;
}

mp_int32 Array::GetLunInfoWin(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID)
{
    mp_int32 iRet = GetDisk83Page(strDev, strLunWWN, strLunID);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get 83 page of device failed.");
        return iRet;
    }
    // if support C8,get lun id from C8
    vector<mp_string> vecResult;
    iRet = GetDisk00Page(strDev, vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get 00 page of device failed.");
        return iRet;
    }
    mp_bool isSupport = IsSupportXXPage("c8", vecResult);
    if (MP_TRUE == isSupport) {
        iRet = GetDiskC8Page(strDev, strLunID);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get C8 page of device failed.");
            return iRet;
        }
    }

    return MP_SUCCESS;
}

mp_int32 Array::GetDiskC8PageWin(const mp_string& strDevice, mp_string& strLunID)
{
    win_dev_info_t pstDevInfo;
    FILE_HANDLE fHandle;
    
    // CodeDex误报，FORTIFY.Path_Manipulation
    mp_int32 iStatus = memset_s(&pstDevInfo, sizeof(win_dev_info_t), 0, sizeof(win_dev_info_t));
    if (iStatus != EOK) {
        COMMLOG(OS_LOG_ERROR, "Call memset_s failed, ret %d.", iStatus);
        return MP_FAILED;
    }

    iStatus = GetDevHandle(strDevice, fHandle);
    if (iStatus != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GetDevHandle open device failed, device(%s)", strDevice.c_str());
        return MP_FAILED;
    }

    iStatus = GetDevSCSIAddress(fHandle, pstDevInfo);
    if (iStatus != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GetDevSCSIAddress failed, device(%s)", strDevice.c_str());
        (mp_void) CloseHandle(fHandle);
        return MP_FAILED;
    }
    
    iStatus = GetDiskC8PageWin_Ex(fHandle, pstDevInfo, strLunID);
    if (iStatus == MP_SUCCESS) {
        COMMLOG(OS_LOG_INFO, "Get C8 page of disk(%s) succ, LunID %s.", strDevice.c_str(), strLunID.c_str());
    }
    (mp_void) CloseHandle(fHandle);
    return iStatus;
}

mp_int32 Array::GetDiskC8PageWin_Ex(const FILE_HANDLE& fHandle, const win_dev_info_t& pstDevInfo, mp_string& strLunID)
{
    scsi_pass_through_with_buff_t stSCSIPass;
    mp_ulong ulLength = 0;
    SetScsiBufValues(stSCSIPass, ulLength, PAGE_CDB_1, PAGE_C8);
    stSCSIPass.pt.PathId = (mp_uchar)pstDevInfo.iPathId;
    stSCSIPass.pt.TargetId = (mp_uchar)pstDevInfo.iScsiId;
    stSCSIPass.pt.Lun = (mp_uchar)pstDevInfo.iLunId;

    mp_ulong ulReturn = 0;
    mp_int32 iStatus = DeviceIoControl(fHandle, IOCTL_SCSI_PASS_THROUGH, &stSCSIPass, sizeof(SCSI_PASS_THROUGH),
        &stSCSIPass, ulLength, &ulReturn, FALSE);
    if (!iStatus) {
        COMMLOG(OS_LOG_ERROR, "GetDisk0xC8Page io error");
        (mp_void)CloseHandle(fHandle);
        return MP_FAILED;
    }
    mp_uint32 validLen = VDS_HS_PAGE_BUF_LEN(stSCSIPass.aucData);
    if (validLen < C8_PAGE_WITH_LUN_ID_LEN) {
        COMMLOG(OS_LOG_INFO, "Disk 0xC8 Page not contain lun id");
        (mp_void)CloseHandle(fHandle);
        return MP_FAILED;
    }
    mp_uint32 lunid = stSCSIPass.aucData[ARRAY_INDEX_192];
    lunid = (lunid << ARRAY_NUM_8) | (stSCSIPass.aucData[ARRAY_INDEX_193]);
    lunid = (lunid << ARRAY_NUM_8) | (stSCSIPass.aucData[ARRAY_INDEX_194]);
    lunid = (lunid << ARRAY_NUM_8) | (stSCSIPass.aucData[ARRAY_INDEX_195]);
    mp_char acDevLUNID[MAX_LUNID_LEN] = { 0 };

    mp_int32 iCheckFailRet = snprintf_s(acDevLUNID, MAX_LUNID_LEN, MAX_LUNID_LEN - 1, "%u", lunid);
    if (iCheckFailRet == MP_FAILED) {
        COMMLOG(OS_LOG_ERROR, "Call SNPRINTF_S failed, ret %d.", iCheckFailRet);
        (mp_void)CloseHandle(fHandle);
        return MP_FAILED;
    }
    (mp_void)CloseHandle(fHandle);
    strLunID = acDevLUNID;
    
    return MP_SUCCESS;
}

mp_int32 Array::GetDisk00PageWin(const mp_string& strDevice, vector<mp_string>& vecResult)
{
    win_dev_info_t pstDevInfo;
    FILE_HANDLE fHandle;
    
    // CodeDex误报，FORTIFY.Path_Manipulation
    mp_int32 iRet = memset_s(&pstDevInfo, sizeof(win_dev_info_t), 0, sizeof(win_dev_info_t));
    if (iRet != EOK) {
        COMMLOG(OS_LOG_ERROR, "GetDisk00PageWin: Call memset_s failed, ret %d.", iRet);
        return MP_FAILED;
    }

    iRet = GetDevHandle(strDevice, fHandle);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GetDisk00PageWin: GetDevHandle open device failed, device(%s)", strDevice.c_str());
        return MP_FAILED;
    }

    iRet = GetDevSCSIAddress(fHandle, pstDevInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GetDevSCSIAddress failed, device(%s)", strDevice.c_str());
        (mp_void) CloseHandle(fHandle);
        return MP_FAILED;
    }

    iRet = GetDisk00PageWin_Ex(fHandle, pstDevInfo, vecResult);
    if (iRet == MP_SUCCESS) {
        COMMLOG(OS_LOG_INFO, "Get 00 page of disk(%s) succ.", strDevice.c_str());
    }
    (mp_void) CloseHandle(fHandle);
    return iRet;
}

mp_int32 Array::GetDisk00PageWin_Ex(FILE_HANDLE& fHandle, win_dev_info_t& pstDevInfo, std::vector<mp_string>& vecResult)
{
    mp_ulong ulLength = 0;
    mp_ulong ulReturn = 0;
    scsi_pass_through_with_buff_t stSCSIPass;
    SetScsiBufValues(stSCSIPass, ulLength, PAGE_CDB_1, PAGE_00);
    stSCSIPass.pt.PathId = (mp_uchar)pstDevInfo.iPathId;
    stSCSIPass.pt.TargetId = (mp_uchar)pstDevInfo.iScsiId;
    stSCSIPass.pt.Lun = (mp_uchar)pstDevInfo.iLunId;

    if (DeviceIoControl(fHandle, IOCTL_SCSI_PASS_THROUGH, &stSCSIPass, sizeof(SCSI_PASS_THROUGH), &stSCSIPass,
        ulLength, &ulReturn, FALSE) == MP_FALSE) {
        COMMLOG(OS_LOG_ERROR, "GetDisk0x00Page io error");
        (mp_void)CloseHandle(fHandle);
        return MP_FAILED;
    }

    mp_int32 validLen = stSCSIPass.aucData[ARRAY_INDEX_3];
    if (validLen == 0) {
        COMMLOG(OS_LOG_ERROR, "GetDisk0x00Page Page Length error");
        (mp_void)CloseHandle(fHandle);
        return MP_FAILED;
    }
    mp_char pageCode[MAX_PAGETCODE_LEN] = { 0 };
    mp_string tmp = "";
    for (int i = 1; i <= validLen; ++i) {
        if (MP_SUCCESS != BinaryToAscii(pageCode, MAX_PAGETCODE_LEN, stSCSIPass.aucData, ARRAY_INDEX_3 + i, 1)) {
            COMMLOG(OS_LOG_ERROR, "Binary to ascii failed.");
            (mp_void)CloseHandle(fHandle);
            return MP_FAILED;
        }
        tmp = pageCode;
        vecResult.push_back(tmp);
    }
    (mp_void)CloseHandle(fHandle);
    return MP_SUCCESS;
}

mp_int32 Array::GetDisk80PageWin(const mp_string& strDevice, mp_string& strSN)
{
    scsi_pass_through_with_buff_t stSCSIPass;
    win_dev_info_t pstDevInfo;
    FILE_HANDLE fHandle;
    mp_ulong ulLength = 0;
    mp_ulong ulReturn = 0;
    mp_char acArraySN[MAX_ARRAY_SN_LEN + 1] = {0};
    // CodeDex误报，FORTIFY.Path_Manipulation
    mp_int32 iStatus = memset_s(&pstDevInfo, sizeof(win_dev_info_t), 0, sizeof(win_dev_info_t));
    if (iStatus != EOK) {
        COMMLOG(OS_LOG_ERROR, "Call memset_s failed, ret %d.", iStatus);
        return MP_FAILED;
    }

    if (GetDevHandle(strDevice, fHandle) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GetDevHandle open device failed, device(%s)", strDevice.c_str());

        return MP_FAILED;
    }

    if (GetDevSCSIAddress(fHandle, pstDevInfo) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GetDevSCSIAddress failed, device(%s)", strDevice.c_str());
        (mp_void) CloseHandle(fHandle);
        return MP_FAILED;
    }

    SetScsiBufValues(stSCSIPass, ulLength, PAGE_CDB_1, PAGE_80);
    stSCSIPass.pt.PathId = (mp_uchar)pstDevInfo.iPathId;
    stSCSIPass.pt.TargetId = (mp_uchar)pstDevInfo.iScsiId;
    stSCSIPass.pt.Lun = (mp_uchar)pstDevInfo.iLunId;

    iStatus = DeviceIoControl(fHandle,
        IOCTL_SCSI_PASS_THROUGH,
        &stSCSIPass,
        sizeof(SCSI_PASS_THROUGH),
        &stSCSIPass,
        ulLength,
        &ulReturn,
        FALSE);
    if (!iStatus) {
        COMMLOG(OS_LOG_ERROR, "GetDisk0x80Page io error");
        (mp_void) CloseHandle(fHandle);
        return MP_FAILED;
    }

    if (stSCSIPass.aucData[ARRAY_INDEX_3] > VALUE_LEN) {
        COMMLOG(OS_LOG_ERROR, "GetDisk0x80Page Page Length error");
        (mp_void) CloseHandle(fHandle);
        return MP_FAILED;
    }

    (mp_void) CloseHandle(fHandle);
    CHECK_NOT_OK(memcpy_s(acArraySN, sizeof(acArraySN), stSCSIPass.aucData + ARRAY_INDEX_4, MAX_ARRAY_SN_LEN));
    strSN = acArraySN;
    COMMLOG(OS_LOG_INFO, "Get 80 page of disk(%s) succ.", strDevice.c_str());
    return MP_SUCCESS;
}

// 非windows平台
#else

/* ------------------------------------------------------------
Description  : 打开设备
Input        :strDev -- 设备名
Output       : iDevFd -- 设备描述符
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lufei 00253739
------------------------------------------------------------- */
mp_int32 Array::OpenDev(const mp_string& strDev, mp_int32& iDevFd)
{
    COMMLOG(OS_LOG_DEBUG, "Begin open dev, dev %s.", strDev.c_str());
    mp_string strDevicePath = strDev;
    CMpString::FormattingPath(strDevicePath);
    mp_char szErr[ARRAY_BYTES_256] = {0};
#ifdef LINUX
    // scsi设备
    mp_int32 fd = open(strDevicePath.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < MP_SUCCESS) {
        mp_int32 iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR,
            "Open scsi dev failed, dev %s, errno[%d]: %s.",
            strDev.c_str(),
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return ERROR_COMMON_OPER_FAILED;
    }

    iDevFd = fd;
#else  // HP/AIX
    // scsi设备
    mp_int32 fd = open(strDevicePath.c_str(), O_RDWR | O_NONBLOCK);
    if (fd < MP_SUCCESS) {
        mp_int32 iErr = GetOSError();
        COMMLOG(OS_LOG_DEBUG,
            "Open scsi dev failed, dev %s, errno[%d]: %s.",
            strDev.c_str(),
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        // 其他设备
        fd = open(strDevicePath.c_str(), O_RDONLY | O_NONBLOCK);
        if (fd < MP_SUCCESS) {
            iErr = GetOSError();
            COMMLOG(OS_LOG_ERROR,
                "Open dev failed, dev %s, errno[%d]: %s.",
                strDev.c_str(),
                iErr,
                GetOSStrErr(iErr, szErr, sizeof(szErr)));
            return ERROR_COMMON_OPER_FAILED;
        }
    }
#endif
    COMMLOG(OS_LOG_DEBUG, "Open dev succ.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :获取磁盘所属阵列信息
Input        :strDevice -- 设备名
Output       : strVendor -- 阵列型号
                   strVendor -- 阵列厂商
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lufei 00253739
------------------------------------------------------------- */
mp_int32 Array::GetDiskArrayInfo(const mp_string& strDevice, mp_string& strVendor, mp_string& strProduct)
{
    // 打开scsi设备
    COMMLOG(OS_LOG_INFO, "Begin GetDiskArrayInfo strDevice = %s.", strDevice.c_str());
    mp_string strDevicePath = strDevice;
    CMpString::FormattingPath(strDevicePath);
    mp_int32 iFd = open(strDevicePath.c_str(), O_RDWR | O_NONBLOCK);
    if (iFd < 0) {
        iFd = open(strDevicePath.c_str(), O_RDONLY | O_NONBLOCK);
    }
    if (iFd < 0) {
        mp_char szErr[ARRAY_BYTES_256] = {0};
        mp_int32 iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR,
            "Open scsi device(%s) failed, errno[%d]: %s.",
            strDevice.c_str(),
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
    COMMLOG(OS_LOG_INFO, "Begin get vendor and product.");
    mp_int32 iRet = GetVendorAndProduct(iFd, strVendor, strProduct);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get Disk array info failed, iRet = %d.", iRet);
        close(iFd);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "End GetDiskArrayInfo strVendor[%s] , strProduct[%s].",
        strVendor.c_str(), strProduct.c_str());
    close(iFd);
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :获取80和83页信息
Input        :iFd -- 设备描述符
Output       : ucCmd -- 80,83页标识
                   aucBuffer -- 80,83页信息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lufei 00253739
------------------------------------------------------------- */
mp_int32 Array::GetDiskPage(mp_int32 iFd, mp_uchar ucCmd, mp_uchar aucBuffer[], mp_int32 bufferLen)
{
#if defined LINUX
    return GetDiskPageLinux(iFd, ucCmd, aucBuffer, bufferLen);
#elif defined AIX
    return GetDiskPageAIX(iFd, ucCmd, aucBuffer, bufferLen);
#elif defined HP_UX_IA
    return GetDiskPageHP(iFd, ucCmd, aucBuffer, bufferLen);
#elif defined SOLARIS
    return GetDiskPageSolaris(iFd, ucCmd, aucBuffer, bufferLen);
#endif
    return MP_SUCCESS;
}

#if defined LINUX
mp_int32 Array::GetDiskPageLinux(mp_int32 iFd, mp_uchar ucCmd, mp_uchar aucBuffer[], mp_int32 bufferLen)
{
    mp_uchar aucCdb[CDB6GENERIC_LENGTH] = {0};
    // 初始化CDB
    aucCdb[0] = SCSIOP_INQUIRY;
    aucCdb[1] = 0x01;
    aucCdb[ARRAY_INDEX_2] = ucCmd;
    aucCdb[ARRAY_INDEX_3] = 0;
    aucCdb[ARRAY_INDEX_4] = ARRAY_NUM_255;

    sg_io_hdr_t io_hdr;
    mp_uchar aucSense[SCSI_MAX_SENSE_LEN] = {0};

    // 初始化sg_io_hdr_t
    CHECK_NOT_OK(memset_s(&io_hdr, sizeof(sg_io_hdr_t), 0, sizeof(sg_io_hdr_t)));
    io_hdr.interface_id = 'S';
    io_hdr.cmdp = aucCdb;
    io_hdr.cmd_len = CDB6GENERIC_LENGTH;

    io_hdr.sbp = aucSense;
    io_hdr.mx_sb_len = SCSI_MAX_SENSE_LEN;

    io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;

    io_hdr.dxferp = aucBuffer;
    io_hdr.dxfer_len = bufferLen;

    io_hdr.timeout = SCSI_CMD_TIMEOUT_LINUX;

    mp_int32 iRet = ioctl(iFd, SG_IO, &io_hdr);
    if (iRet != MP_SUCCESS || io_hdr.status != 0) {
        COMMLOG(OS_LOG_ERROR, "Ioctl failed, ret %d, io_status is %d.", iRet, io_hdr.status);
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 Array::GetVendorAndProductLinux(mp_int32 iFd, mp_string& strVendor, mp_string& strProduct)
{
    mp_char acBuffer[DISK_BYTE_OF_SECTOR] = {0};
    mp_uchar aucCdb[CDB6GENERIC_LENGTH] = {0};
    mp_char pszProduct[MAX_PRODUCT_LEN] = {0};
    mp_char pszVendor[MAX_VENDOR_LEN] = {0};
    struct sg_io_hdr stScsiCmd;

    CHECK_NOT_OK(memset_s(&stScsiCmd, sizeof(struct sg_io_hdr), 0, sizeof(struct sg_io_hdr)));
    stScsiCmd.interface_id = 'S';
    stScsiCmd.dxfer_direction = SG_DXFER_FROM_DEV;

    aucCdb[0] = SCSIOP_INQUIRY;
    aucCdb[1] = 0;
    aucCdb[ARRAY_INDEX_2] = 0;
    aucCdb[ARRAY_INDEX_3] = 0;
    aucCdb[ARRAY_INDEX_4] = 0xff;
    aucCdb[ARRAY_INDEX_5] = 0;
    // 下发给设备的命令
    stScsiCmd.cmdp = aucCdb;
    stScsiCmd.cmd_len = CDB6GENERIC_LENGTH;

    CHECK_NOT_OK(memset_s(acBuffer, DISK_BYTE_OF_SECTOR, 0, DISK_BYTE_OF_SECTOR));
    stScsiCmd.dxferp = acBuffer;
    stScsiCmd.dxfer_len = DISK_BYTE_OF_SECTOR;
    // 下发ioctl的超时时间
    stScsiCmd.timeout = SCSI_CMD_TIMEOUT_LINUX;
    // 把构造好的命令通过ioctl下发
    mp_int32 iRet = ioctl(iFd, SG_IO, &stScsiCmd);
    if (iRet < 0 || stScsiCmd.status != 0) {
        COMMLOG(OS_LOG_ERROR, "Ioctl failed, ret %d.", iRet);
        return MP_FAILED;
    }
    // CodeDex误报，FORTIFY.Path_Manipulation
    CHECK_CLOSE_FD(memcpy_s(pszVendor, MAX_VENDOR_LEN, acBuffer + ARRAY_INDEX_8, ARRAY_NUM_8));
    CHECK_CLOSE_FD(memcpy_s(pszProduct, MAX_PRODUCT_LEN, acBuffer + ARRAY_NUM_16, ARRAY_NUM_16));
    strVendor = pszVendor;
    strProduct = pszProduct;
    return MP_SUCCESS;
}

#elif defined (_AIX)
mp_int32 Array::GetDiskPageAIX(mp_int32 iFd, mp_uchar ucCmd, mp_uchar* aucBuffer, mp_int32 bufferLen)
{
    mp_uchar aucCdb[CDB6GENERIC_LENGTH] = {0};
    // 初始化CDB
    aucCdb[0] = SCSIOP_INQUIRY;
    aucCdb[1] = 0x01;
    aucCdb[ARRAY_INDEX_2] = ucCmd;
    aucCdb[ARRAY_INDEX_3] = 0;
    aucCdb[ARRAY_INDEX_4] = ARRAY_NUM_255;

    struct sc_passthru ioCmd;
    CHECK_NOT_OK(memset_s(&ioCmd, sizeof(ioCmd), 0, sizeof(ioCmd)));

    // 设置CDB
    CHECK_NOT_OK(memcpy_s(ioCmd.scsi_cdb, sizeof(ioCmd.scsi_cdb), aucCdb, sizeof(aucCdb)));

    // 设置CDB的长度
    ioCmd.command_length = sizeof(aucCdb);

    // 设置超时时间
    ioCmd.timeout_value = SCSI_CMD_TIMEOUT_AIX;

    // 传输数据的缓冲区
    ioCmd.buffer = (mp_char*)aucBuffer;
    // 传输数据的长度
    ioCmd.data_length = bufferLen;

    // 传输数据的方向，读还是写
    ioCmd.flags = B_READ;
    ioCmd.flags |= SC_ASYNC;

    // ioctl发送SCSI数据
    mp_int32 iRet = ioctl(iFd, DK_PASSTHRU, &ioCmd);
    if (iRet != 0) {
        COMMLOG(OS_LOG_ERROR, "Ioctl failed, iRet = %d.", iRet);
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 Array::GetVendorAndProductAIX(mp_int32 iFd, mp_string& strVendor, mp_string& strProduct)
{
    mp_char acBuffer[BUFFER_LEN_36] = {0};
    mp_char pszProduct[MAX_PRODUCT_LEN] = {0};
    mp_char pszVendor[MAX_VENDOR_LEN] = {0};
    mp_uchar aucCdb[CDB6GENERIC_LENGTH] = {0};
    mp_int32 iRet;
    mp_uint32 uiRetryCount = ARRAY_INDEX_3;
    mp_uint32 uiRetryIndex = 0;

    struct sc_passthru ioCmd;
    CHECK_NOT_OK(memset_s(&ioCmd, sizeof(ioCmd), 0, sizeof(ioCmd)));

    // CDB
    aucCdb[0] = SCSIOP_INQUIRY;
    aucCdb[1] = 0x0;
    aucCdb[ARRAY_INDEX_2] = 0x0;
    aucCdb[ARRAY_INDEX_4] = (mp_uchar)BUFFER_LEN_36;

    // 设置CDB
    CHECK_NOT_OK(memcpy_s(ioCmd.scsi_cdb, sizeof(ioCmd.scsi_cdb), aucCdb, sizeof(aucCdb)));

    // 设置CDB的长度
    ioCmd.command_length = sizeof(aucCdb);
    // 设置超时时间
    ioCmd.timeout_value = SCSI_CMD_TIMEOUT_AIX;
    // 传输数据的缓冲区
    ioCmd.buffer = acBuffer;
    // 传输数据的长度
    ioCmd.data_length = BUFFER_LEN_36;
    // 传输数据的方向，读还是写
    ioCmd.flags = B_READ;
    ioCmd.flags |= SC_ASYNC;

    // ioctl发送SCSI数据,若发送失败，则重试
    for (uiRetryIndex = 0; uiRetryIndex < uiRetryCount; ++uiRetryIndex) {
        iRet = ioctl(iFd, DK_PASSTHRU, &ioCmd);
        if (iRet != 0) {
            COMMLOG(OS_LOG_ERROR, "Ioctl failed, ret %d.", iRet);
        } else {
            break;
        }

        DoSleep(ARRAY_NUM_500);
    }

    if (iRet != 0) {
        COMMLOG(OS_LOG_ERROR, "Ioctl failed, ret = %d.", iRet);
        return MP_FAILED;
    }

    // CodeDex误报，FORTIFY.Path_Manipulation
    CHECK_CLOSE_FD(memcpy_s(pszVendor, MAX_VENDOR_LEN, acBuffer + ARRAY_INDEX_8, ARRAY_NUM_8));
    CHECK_CLOSE_FD(memcpy_s(pszProduct, MAX_PRODUCT_LEN, acBuffer + ARRAY_NUM_16, ARRAY_NUM_16));
    strVendor = pszVendor;
    strProduct = pszProduct;
    return MP_SUCCESS;
}

#elif defined HP_UX_IA
mp_int32 Array::GetDiskPageHP(mp_int32 iFd, mp_uchar ucCmd, mp_uchar aucBuffer[], mp_int32 bufferLen)
{
    mp_uchar aucCdb[CDB6GENERIC_LENGTH] = {0};
    // 初始化CDB
    aucCdb[0] = SCSIOP_INQUIRY;
    aucCdb[1] = 0x01;
    aucCdb[ARRAY_INDEX_2] = ucCmd;
    aucCdb[ARRAY_INDEX_3] = 0;
    aucCdb[ARRAY_INDEX_4] = ARRAY_NUM_255;

    struct sctl_io ioCmd;

    // 初始化sctl_io
    CHECK_NOT_OK(memset_s(&ioCmd, sizeof(struct sctl_io), 0, sizeof(struct sctl_io)));
    ioCmd.flags = B_READ;
    ioCmd.cdb_length = CDB6GENERIC_LENGTH;
    CHECK_NOT_OK(memcpy_s(ioCmd.cdb, sizeof(ioCmd.cdb), aucCdb, ioCmd.cdb_length));
    ioCmd.data = aucBuffer;
    ioCmd.data_length = bufferLen;
    ioCmd.max_msecs = SCSI_CMD_TIMEOUT_HP;

    mp_int32 iRet = ioctl(iFd, SIOC_IO, &ioCmd);
    if (iRet < MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Ioctl failed, iRet = %d.", iRet);
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 Array::GetVendorAndProductHP(mp_int32 iFd, mp_string& strVendor, mp_string& strProduct)
{
    mp_char acBuffer[DATA_LEN_256] = {0};
    mp_char pszProduct[MAX_PRODUCT_LEN] = {0};
    mp_char pszVendor[MAX_VENDOR_LEN] = {0};
    mp_uchar aucCdb[CDB6GENERIC_LENGTH] = {0};
    struct sctl_io ioCmd;

    CHECK_NOT_OK(memset_s(&ioCmd, sizeof(struct sctl_io), 0, sizeof(struct sctl_io)));
    ioCmd.flags = B_READ;

    aucCdb[0] = SCSIOP_INQUIRY;
    aucCdb[1] = 0;
    aucCdb[ARRAY_INDEX_2] = 0;
    aucCdb[ARRAY_INDEX_3] = 0;
    aucCdb[ARRAY_INDEX_4] = 0xff;
    aucCdb[ARRAY_INDEX_5] = 0;

    // CDB
    ioCmd.cdb_length = CDB6GENERIC_LENGTH;
    CHECK_NOT_OK(memcpy_s(ioCmd.cdb, sizeof(ioCmd.cdb), aucCdb, ioCmd.cdb_length));

    ioCmd.data = acBuffer;
    ioCmd.data_length = DATA_LEN_256;

    ioCmd.max_msecs = SCSI_CMD_TIMEOUT_HP;

    // 把构造好的命令通过ioctl下发
    mp_int32 iRet = ioctl(iFd, SIOC_IO, &ioCmd);
    if (iRet < 0) {
        COMMLOG(OS_LOG_ERROR, "Ioctl failed, iRet = %d.", iRet);
        return MP_FAILED;
    }

    // CodeDex误报，FORTIFY.Path_Manipulation
    CHECK_CLOSE_FD(memcpy_s(pszVendor, MAX_VENDOR_LEN, acBuffer + ARRAY_INDEX_8, ARRAY_NUM_8));
    CHECK_CLOSE_FD(memcpy_s(pszProduct, MAX_PRODUCT_LEN, acBuffer + ARRAY_NUM_16, ARRAY_NUM_16));
    strVendor = pszVendor;
    strProduct = pszProduct;
    return MP_SUCCESS;
}
#elif defined SOLARIS
mp_int32 Array::GetDiskPageSolaris(mp_int32 iFd, mp_uchar ucCmd, mp_uchar aucBuffer[], mp_int32 bufferLen)
{
    mp_uchar aucCdb[CDB6GENERIC_LENGTH] = {0};
    // 初始化CDB
    aucCdb[0] = SCSIOP_INQUIRY;
    aucCdb[1] = 0x01;
    aucCdb[ARRAY_INDEX_2] = ucCmd;
    aucCdb[ARRAY_INDEX_3] = 0;
    aucCdb[ARRAY_INDEX_4] = ARRAY_NUM_255;
    aucCdb[ARRAY_INDEX_5] = 0x00;
    struct uscsi_cmd ioCmd;
    CHECK_NOT_OK(memset_s(&ioCmd, sizeof(struct uscsi_cmd), 0, sizeof(struct uscsi_cmd)));
    ioCmd.uscsi_flags = USCSI_READ;

    // CDB
    ioCmd.uscsi_cdblen = ARRAY_NUM_6;
    ioCmd.uscsi_cdb = (mp_char*)(aucCdb);

    ioCmd.uscsi_bufaddr = (mp_char*)(aucBuffer);
    ioCmd.uscsi_buflen = bufferLen;

    ioCmd.uscsi_timeout = SCSI_CMD_TIMEOUT_SOLARIS;
    mp_int32 iRet = ioctl(iFd, USCSICMD, &ioCmd);
    if (iRet < MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Ioctl failed, iRet = %d.", iRet);
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 Array::GetVendorAndProductSolaris(mp_int32 iFd, mp_string& strVendor, mp_string& strProduct)
{
    mp_char acBuffer[DATA_LEN_256] = {0};
    mp_char pszProduct[MAX_PRODUCT_LEN] = {0};
    mp_char pszVendor[MAX_VENDOR_LEN] = {0};
    mp_uchar aucCdb[CDB6GENERIC_LENGTH] = {0};
    struct uscsi_cmd ioCmd;
    aucCdb[0] = SCSIOP_INQUIRY;
    aucCdb[1] = 0x00;
    aucCdb[ARRAY_INDEX_2] = 0x00;
    aucCdb[ARRAY_INDEX_3] = 0x00;
    aucCdb[ARRAY_INDEX_4] = 0xff;
    aucCdb[ARRAY_INDEX_5] = 0x00;
    CHECK_NOT_OK(memset_s(&ioCmd, sizeof(struct uscsi_cmd), 0, sizeof(struct uscsi_cmd)));
    ioCmd.uscsi_flags = USCSI_READ;

    // CDB
    ioCmd.uscsi_cdblen = ARRAY_NUM_6;
    ioCmd.uscsi_cdb = reinterpret_cast<mp_char*>(aucCdb);

    ioCmd.uscsi_bufaddr = reinterpret_cast<mp_char*>(acBuffer);
    ioCmd.uscsi_buflen = ARRAY_BYTES_256;

    ioCmd.uscsi_timeout = SCSI_CMD_TIMEOUT_SOLARIS;
    mp_int32 iRet = ioctl(iFd, USCSICMD, &ioCmd);
    if (iRet < 0) {
        COMMLOG(OS_LOG_ERROR, "Ioctl failed, iRet = %d.", iRet);
        return MP_FAILED;
    }

    // CodeDex误报，FORTIFY.Path_Manipulation
    CHECK_CLOSE_FD(memcpy_s(pszVendor, MAX_VENDOR_LEN, acBuffer + ARRAY_INDEX_8, ARRAY_NUM_8));
    CHECK_CLOSE_FD(memcpy_s(pszProduct, MAX_PRODUCT_LEN, acBuffer + ARRAY_NUM_16, ARRAY_NUM_16));
    strVendor = pszVendor;
    strProduct = pszProduct;
    return MP_SUCCESS;
}
#endif

/* ------------------------------------------------------------
Description  :获取阵列信息
Input        :iFd -- 设备描述符
Output       : aucBuffer -- 阵列厂商和型号信息字符串
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lufei 00253739
------------------------------------------------------------- */
mp_int32 Array::GetVendorAndProduct(mp_int32 iFd, mp_string& strVendor, mp_string& strProduct)
{
    COMMLOG(OS_LOG_INFO, "Entry get vendor and product.");
#if defined LINUX
    return GetVendorAndProductLinux(iFd, strVendor, strProduct);
#elif defined AIX
    return GetVendorAndProductAIX(iFd, strVendor, strProduct);
#elif defined HP_UX_IA
    return GetVendorAndProductHP(iFd, strVendor, strProduct);
#elif defined SOLARIS
    return GetVendorAndProductSolaris(iFd, strVendor, strProduct);
#endif
}

mp_int32 Array::GetLunInfoNoWin(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID)
{
    vector<mp_string> vecResult;
    CRootCaller rootCaller;
    mp_int32 iCount = 0;

    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_83PAGE, strDev, &vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get 83 page of device failed.");
        return iRet;
    }

    if (vecResult.empty()) {
        COMMLOG(OS_LOG_ERROR, "Get 83 page of device failed.");
        return MP_FAILED;
    }

    for (vector<mp_string>::iterator iter = vecResult.begin(); iter != vecResult.end(); ++iter) {
        if (COUNT_LUNWWN == iCount) {
            strLunWWN = *iter;
        }
        if (COUNT_LUNID == iCount) {
            strLunID = *iter;
        }
        ++iCount;
    }

    return GetLunInfoNoWinAfter(strDev, strLunID);
}

mp_int32 Array::GetLunInfoNoWinAfter(mp_string& strDev, mp_string& strLunID)
{
    // if support C8,get lun id from C8
    vector<mp_string> vecResult1;
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_00PAGE, strDev, &vecResult1);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get 00 page of device failed.");
        return iRet;
    }

    if (vecResult1.empty()) {
        COMMLOG(OS_LOG_ERROR, "Get 00 page of device failed.");
        return MP_FAILED;
    }

    mp_bool isSupport = IsSupportXXPage("c8", vecResult1);
    if (MP_TRUE == isSupport) {
        vector<mp_string> vecResult2;
        iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_C8PAGE, strDev, &vecResult2);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get C8 page of device failed.");
            return iRet;
        }
        mp_int32 iCount1 = 0;
        for (vector<mp_string>::iterator iter = vecResult2.begin(); iter != vecResult2.end(); ++iter) {
            if (C8_COUNT_LUNID == iCount1) {
                strLunID = *iter;
            }
            ++iCount1;
        }
    }

    return MP_SUCCESS;
}

mp_int32 Array::GetDiskC8PageNoWin(const mp_string& strDevice, mp_string& strLunID)
{
    COMMLOG(OS_LOG_INFO, "Begin to get C8 page of disk(%s).", strDevice.c_str());
    mp_string strDevicePath = strDevice;
    CMpString::FormattingPath(strDevicePath);
    // 打开scsi设备
    mp_int32 iFd = open(strDevicePath.c_str(), O_RDWR | O_NONBLOCK);
    if (iFd < 0) {
        iFd = open(strDevicePath.c_str(), O_RDONLY | O_NONBLOCK);
    }
    if (iFd < 0) {
        COMMLOG(OS_LOG_ERROR, "Open scsi device failed, iFd = %d.", iFd);
        return MP_FAILED;
    }
#if (defined HP_UX_IA) || (defined LINUX)
    mp_uchar aucBuffer[DATA_LEN_256] = {0};
#elif defined AIX || (defined SOLARIS)
    mp_uchar aucBuffer[BUFFER_LEN_255] = {0};
#endif
    int bufferLen = sizeof(aucBuffer);
    mp_int32 iRet = GetDiskPage(iFd, PAGE_C8, aucBuffer, bufferLen);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get Disk C8Page failed, iRet = %d.", iRet);
        close(iFd);
        return iRet;
    }

    mp_uint32 validLen = VDS_HS_PAGE_BUF_LEN(aucBuffer);
    if (validLen < C8_PAGE_WITH_LUN_ID_LEN) {
        COMMLOG(OS_LOG_INFO, "Disk 0xC8 Page not contain lun id");
        close(iFd);
        return MP_SUCCESS;
    }
    mp_uint32 lunid = aucBuffer[ARRAY_INDEX_192];
    lunid = (lunid << ARRAY_NUM_8) | (aucBuffer[ARRAY_INDEX_193]);
    lunid = (lunid << ARRAY_NUM_8) | (aucBuffer[ARRAY_INDEX_194]);
    lunid = (lunid << ARRAY_NUM_8) | (aucBuffer[ARRAY_INDEX_195]);
    mp_char acDevLUNID[MAX_LUNID_LEN] = {0};
    iRet = snprintf_s(acDevLUNID, MAX_LUNID_LEN, MAX_LUNID_LEN - 1, "%u", lunid);
    if (iRet == MP_FAILED) {
        COMMLOG(OS_LOG_ERROR, "Call SNPRINTF_S failed, ret %d.", iRet);
        close(iFd);
        return MP_FAILED;
    }

    strLunID = acDevLUNID;
    close(iFd);

    COMMLOG(OS_LOG_INFO, "Get C8 page of disk(%s) succ, LunID %s.", strDevice.c_str(), strLunID.c_str());
    return MP_SUCCESS;
}

mp_int32 Array::GetDisk00PageNoWin(const mp_string& strDevice, vector<mp_string>& vecResult)
{
#if (defined HP_UX_IA) || (defined LINUX)
    mp_uchar aucBuffer[DATA_LEN_256] = {0};
#elif defined AIX || (defined SOLARIS)
    mp_uchar aucBuffer[BUFFER_LEN_255] = {0};
#endif
    COMMLOG(OS_LOG_INFO, "Begin to get 00 page of disk(%s).", strDevice.c_str());
    mp_string strDevicePath = strDevice;
    CMpString::FormattingPath(strDevicePath);
    // 打开scsi设备
    mp_int32 iFd = open(strDevicePath.c_str(), O_RDWR | O_NONBLOCK);
    if (iFd < 0) {
        iFd = open(strDevicePath.c_str(), O_RDONLY | O_NONBLOCK);
    }
    if (iFd < 0) {
        COMMLOG(OS_LOG_ERROR, "Open scsi device failed, iFd = %d.", iFd);
        return MP_FAILED;
    }

    int bufferLen = sizeof(aucBuffer);
    mp_int32 iRet = GetDiskPage(iFd, PAGE_00, aucBuffer, bufferLen);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get Disk 00Page failed, iRet = %d.", iRet);
        close(iFd);
        return iRet;
    }
    mp_int32 validLen = aucBuffer[ARRAY_INDEX_3];
    if (validLen == 0) {
        COMMLOG(OS_LOG_ERROR, "Page length error, page len %u.", validLen);
        close(iFd);
        return MP_FAILED;
    }
    mp_char pageCode[MAX_PAGETCODE_LEN] = {0};
    mp_string tmp = "";
    for (int i = 1; i <= validLen; ++i) {
        iRet = BinaryToAscii(pageCode, MAX_PAGETCODE_LEN, aucBuffer, ARRAY_INDEX_3 + i, 1);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Binary to ascii failed, ret %d.", iRet);
            close(iFd);
            return iRet;
        }
        tmp = pageCode;
        vecResult.push_back(tmp);
    }
    close(iFd);
    COMMLOG(OS_LOG_INFO, "Get 00 page of disk(%s) succ.", strDevice.c_str());
    return MP_SUCCESS;
}

mp_int32 Array::GetDisk80PageNoWin(const mp_string& strDevice, mp_string& strSN)
{
#if (defined HP_UX_IA) || (defined LINUX)
    mp_uchar aucBuffer[DATA_LEN_256] = {0};
#elif defined AIX || (defined SOLARIS)
    mp_uchar aucBuffer[BUFFER_LEN_255] = {0};
#endif

    COMMLOG(OS_LOG_INFO, "Begin to Get 80 page of disk(%s).", strDevice.c_str());
    mp_string strDevicePath = strDevice;
    CMpString::FormattingPath(strDevicePath);
    // 打开scsi设备
    mp_int32 iFd = open(strDevicePath.c_str(), O_RDWR | O_NONBLOCK);
    if (iFd < 0) {
        iFd = open(strDevicePath.c_str(), O_RDONLY | O_NONBLOCK);
    }
    if (iFd < 0) {
        COMMLOG(OS_LOG_ERROR, "Open scsi device failed, iFd = %d.", iFd);
        return MP_FAILED;
    }

    int bufferLen = sizeof(aucBuffer);
    mp_int32 iRet = GetDiskPage(iFd, PAGE_80, aucBuffer, bufferLen);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get Disk 80Page failed, iRet = %d.", iRet);
        close(iFd);
        return iRet;
    }

    // 厂商和型号
    mp_string strVendor;
    mp_string strProduct;
    if (Array::GetArrayVendorAndProduct(strDevice, strVendor, strProduct) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get array info of disk(%s) failed.", strDevice.c_str());
        close(iFd);
        return ERROR_COMMON_QUERY_APP_LUN_FAILED;
    }

    COMMLOG(OS_LOG_INFO, "strProduct info(%s).", strProduct.c_str());
    if (strcmp(CMpString::Trim(strProduct).c_str(), PRODUCT_VBS.c_str()) != 0) {
        iRet = GetDiskSN(aucBuffer, bufferLen, strSN);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get disk SN failed, ret %d.", iRet);
        }
    } else {
        iRet = GetDiskFusionStorageSN(aucBuffer, bufferLen, strSN);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get FusionStorage SN failed, ret %d.", iRet);
        }
    }

    close(iFd);
    COMMLOG(OS_LOG_INFO, "Get 80 page of disk(%s) succ.", strDevice.c_str());
    return iRet;
}

#endif

/* ------------------------------------------------------------
Description  :获取 通道的设备列表
Input        :strDevice -- 设备名称
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 Array::GetHostLunIDList(const mp_string& strDevice, vector<mp_int32>& vecHostLunID)
{
#ifdef LINUX
    mp_int32 iFd = -1;
    mp_uchar rlCmdBlk[ARRAY_NUM_12];

    memset_s(rlCmdBlk, sizeof(rlCmdBlk), 0, sizeof(rlCmdBlk));
    rlCmdBlk[0] = 0xa0;
    // 打开scsi设备
    mp_int32 iRet = OpenDev(strDevice, iFd);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Open scsi device failed, iFd = %d.", iFd);
        return MP_FAILED;
    }
    mp_uint32 mx_resp_len = DEF_RLUNS_BUFF_LEN;
    rlCmdBlk[ARRAY_INDEX_6] = (mp_uchar)((mx_resp_len >> ARRAY_NUM_24) & 0xff);
    rlCmdBlk[ARRAY_INDEX_7] = (mp_uchar)((mx_resp_len >> ARRAY_NUM_16) & 0xff);
    rlCmdBlk[ARRAY_INDEX_8] = (mp_uchar)((mx_resp_len >> ARRAY_NUM_8) & 0xff);
    rlCmdBlk[ARRAY_INDEX_9] = (mp_uchar)(mx_resp_len & 0xff);

    struct sg_io_hdr io_hdr;
    iRet = memset_s(&io_hdr, sizeof(struct sg_io_hdr), 0, sizeof(struct sg_io_hdr));
    if (iRet != EOK) {
        COMMLOG(OS_LOG_ERROR, "memset io_hdr failed, ret %d.", iRet);
        close(iFd);
        return MP_FAILED;
    }
    io_hdr.interface_id = 'S';
    io_hdr.cmdp = rlCmdBlk;
    io_hdr.cmd_len = sizeof(rlCmdBlk);

    mp_uchar sense_b[ARRAY_NUM_32] = {0};
    io_hdr.sbp = sense_b;
    io_hdr.mx_sb_len = sizeof(sense_b);

    static mp_uchar gaucResponseBuff[MAX_RLUNS_BUFF_LEN] = {0};
    iRet = memset_s(gaucResponseBuff, sizeof(gaucResponseBuff), 0, sizeof(gaucResponseBuff));
    if (iRet != EOK) {
        COMMLOG(OS_LOG_ERROR, "memset gaucResponseBuff, ret %d.", iRet);
        close(iFd);
        return MP_FAILED;
    }
    io_hdr.dxferp = gaucResponseBuff;
    io_hdr.dxfer_len = mx_resp_len;
    io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;

    io_hdr.timeout = DEF_COM_TIMEOUT;

    return GetHostLunID(strDevice, iFd, io_hdr, gaucResponseBuff, MAX_RLUNS_BUFF_LEN, mx_resp_len, vecHostLunID);
#else
    COMMLOG(OS_LOG_ERROR, "Not linux is not supported reporting lun list.");
    return MP_FAILED;
#endif
}

#ifdef LINUX
mp_int32 Array::GetHostLunID(const mp_string& strDevice, mp_int32& iFd, struct sg_io_hdr& io_hdr,
    const mp_uchar gaucResponseBuff[], mp_uint32 buffLen, mp_uint32& mx_resp_len, vector<mp_int32>& vecHostLunID)
{
    if (ioctl(iFd, SG_IO, &io_hdr) != 0) {
        mp_int32 iErr = GetOSError();
        mp_char szErr[ARRAY_BYTES_256] = {0};
        COMMLOG(OS_LOG_ERROR,
            "Open scsi dev failed, dev %s, errno[%d]: %s.",
            strDevice.c_str(),
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        close(iFd);
        return MP_FAILED;
    }
    close(iFd);

    if (io_hdr.status || io_hdr.host_status || io_hdr.driver_status) {
        COMMLOG(OS_LOG_ERROR,
            "report lun failed. status:%d, host_status:%d, driver_status:%d\n",
            io_hdr.status,
            io_hdr.host_status,
            io_hdr.driver_status);
        return MP_FAILED;
    }
    mp_uint32 list_len = (mp_uint32)(gaucResponseBuff[0] << ARRAY_NUM_24);
    list_len += (mp_uint32)(gaucResponseBuff[1] << ARRAY_NUM_16);
    list_len += (mp_uint32)(gaucResponseBuff[ARRAY_INDEX_2] << ARRAY_NUM_8);
    list_len += (mp_uint32)(gaucResponseBuff[ARRAY_INDEX_3]);
    mp_int32 lunCount = list_len / ARRAY_NUM_8;

    COMMLOG(OS_LOG_DEBUG, "Lun list length = %d.", lunCount);
    if ((list_len + ARRAY_NUM_8) > mx_resp_len) {
        COMMLOG(OS_LOG_ERROR, "dev %s, too many luns.", strDevice.c_str());
        return MP_FAILED;
    }

    mp_int32 off = ARRAY_NUM_8;
    mp_uint32 lunID = 0;
    vecHostLunID.reserve(lunCount);
    for (mp_int32 i = 0; i < lunCount; ++i) {
        // 参考多路径代码，off进行调整
        // CodeDex误报，CSEC_LOOP_ARRAY_CHECKING，数组下标不会越界
        off += ARRAY_NUM_8;
        lunID = (mp_uint32)(gaucResponseBuff[off - ARRAY_NUM_8] << ARRAY_NUM_8);
        lunID += (mp_uint32)(gaucResponseBuff[off - ARRAY_INDEX_7]);
        lunID += (mp_uint32)(gaucResponseBuff[off - ARRAY_NUM_6] << ARRAY_NUM_24);
        lunID += (mp_uint32)(gaucResponseBuff[off - ARRAY_INDEX_5] << ARRAY_NUM_16);
        vecHostLunID.emplace_back((mp_int32)lunID);
    }
    return MP_SUCCESS;
}
#endif

/* ------------------------------------------------------------
Description  :获取阵列信息
Input        :strDev -- 设备名
Output       : strproduct -- 阵列厂商
                   strvendor --型号
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lufei 00253739
------------------------------------------------------------- */
#ifdef WIN32
mp_int32 Array::GetArrayVendorAndProduct(const mp_string& strDev, mp_string& strvendor, mp_string& strproduct)
{
    mp_char acVendor[MAX_VENDOR_LEN] = {0};
    mp_char acProduct[MAX_PRODUCT_LEN] = {0};
    mp_char acVersion[MAX_VVERSION_LEN] = {0};
    mp_ulong ulReturn = 0;
    mp_ulong ulLength = 0;
    FILE_HANDLE hOpenDeviceRet;
    scsi_pass_through_with_buff_t stSCSIPass;

    mp_int32 iRet = GetDevHandle(strDev, hOpenDeviceRet);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GetDevHandle open device failed, device(%s)", strDev.c_str());
        return MP_FAILED;
    }

    SetScsiBufValues(stSCSIPass, ulLength, PAGE_CDB_0, PAGE_CDB_0);
    iRet = DeviceIoControl(hOpenDeviceRet, IOCTL_SCSI_PASS_THROUGH, &stSCSIPass, sizeof(SCSI_PASS_THROUGH),
        &stSCSIPass, ulLength, &ulReturn, FALSE);
    if (!iRet) {
        COMMLOG(OS_LOG_ERROR, "GetArrayVendorAndProduct io error, device(%s)", strDev.c_str());
        (mp_void) CloseHandle(hOpenDeviceRet);
        return MP_FAILED;
    }

    // 厂商
    mp_int32 iCheckRet = memcpy_s(acVendor, sizeof(acVendor), stSCSIPass.aucData + ARRAY_NUM_8, ARRAY_NUM_8);
    if (iCheckRet != EOK) {
        COMMLOG(OS_LOG_ERROR, "Call memcpy_s failed, ret %d.", iCheckRet);
        (mp_void) CloseHandle(hOpenDeviceRet);

        return MP_FAILED;
    }
    strvendor = acVendor;
    // 型号
    iCheckRet = memcpy_s(acProduct, sizeof(acProduct), stSCSIPass.aucData + ARRAY_NUM_16, ARRAY_NUM_16);
    if (iCheckRet != EOK) {
        COMMLOG(OS_LOG_ERROR, "Call memcpy_s failed, ret %d.", iCheckRet);
        (mp_void) CloseHandle(hOpenDeviceRet);

        return MP_FAILED;
    }
    strproduct = acProduct;
    (mp_void) CloseHandle(hOpenDeviceRet);

    COMMLOG(OS_LOG_INFO, "Get array info of disk(%s) succ, vendor %s, product %s.",
        strDev.c_str(), strvendor.c_str(), strproduct.c_str());
    return MP_SUCCESS;
}
#else  // 非WIN系统
mp_int32 Array::GetArrayVendorAndProduct(const mp_string& strDev, mp_string& strvendor, mp_string& strproduct)
{
    COMMLOG(OS_LOG_DEBUG, "Begin to get array info of disk(%s).", strDev.c_str());
    vector<mp_string> vecResult;
    vector<mp_string>::iterator iter;
    mp_int32 iCount = 0;
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_VENDORANDPRODUCT, strDev, &vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get vendor and product info of device failed.");
        return iRet;
    }

    if (vecResult.empty()) {
        COMMLOG(OS_LOG_ERROR, "Get vendor and product info of device failed.");
        return MP_FAILED;
    }

    for (iter = vecResult.begin(); iter != vecResult.end(); ++iter) {
        if (iCount == COUNT_VENDOR) {
            strvendor = *iter;
        }
        if (iCount == COUNT_PRODUCT) {
            strproduct = *iter;
        }
        ++iCount;
    }
    COMMLOG(OS_LOG_INFO, "Get array info of disk(%s) succ, vendor %s, product %s.",
        strDev.c_str(), strvendor.c_str(), strproduct.c_str());
    return MP_SUCCESS;
}
#endif

/* ------------------------------------------------------------
Description  :获取00页信息
Input        :strDev -- 设备名
Output       : vecResul--VPD pages code
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : longjiang wx290533
------------------------------------------------------------- */
mp_int32 Array::GetDisk00Page(const mp_string& strDevice, vector<mp_string>& vecResult)
{
#ifdef WIN32
    return GetDisk00PageWin(strDevice, vecResult);
#else  // 非WIN系统
    return GetDisk00PageNoWin(strDevice, vecResult);
#endif  // 非WIN系统
}

/* ------------------------------------------------------------
Description  :获取C8页信息
Input        :strDev -- 设备名
Output       : strLunID --LUN的ID
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : longjiang wx290533
------------------------------------------------------------- */
mp_int32 Array::GetDiskC8Page(const mp_string& strDevice, mp_string& strLunID)
{
#ifdef WIN32
    return GetDiskC8PageWin(strDevice, strLunID);
#else  // 非WIN系统
    return GetDiskC8PageNoWin(strDevice, strLunID);
#endif  // 非WIN系统
}

/* ------------------------------------------------------------
Description  :获取83页信息
Input        :strDev -- 设备名
Output       : strLunWWN -- LUN的WWN
                   strLunID --LUN的ID
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lufei 00253739
------------------------------------------------------------- */
mp_int32 Array::GetDisk83Page(const mp_string& strDevice, mp_string& strLunWWN, mp_string& strLunID)
{
#ifdef WIN32
    mp_int32 iRet = GetDisk83PageWin(strDevice, strLunWWN, strLunID);
#else  // 非WIN系统
    mp_int32 iRet = GetDisk83PageNoWin(strDevice, strLunWWN, strLunID);
#endif  // 非WIN系统
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "Get 83 page of disk(%s) succ, wwn %s, LunID %s.",
        strDevice.c_str(), strLunWWN.c_str(), strLunID.c_str());
    return MP_SUCCESS;
}

#ifdef WIN32
mp_int32 Array::InitDisk83PageRes(const mp_string& strDevice, win_dev_info_t& pstDevInfo, FILE_HANDLE& fHandle)
{
    mp_int32 ret = memset_s(&pstDevInfo, sizeof(win_dev_info_t), 0, sizeof(win_dev_info_t));
    if (ret != EOK) {
        COMMLOG(OS_LOG_ERROR, "Call memset_s failed, ret %d.", ret);
        return MP_FAILED;
    }

    ret = GetDevHandle(strDevice, fHandle);
    if (ret != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GetDevHandle open device failed, device(%s)", strDevice.c_str());
        return MP_FAILED;
    }

    ret = GetDevSCSIAddress(fHandle, pstDevInfo);
    if (ret != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GetDevSCSIAddress failed, device(%s)", strDevice.c_str());
        (mp_void) CloseHandle(fHandle);
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 Array::GetDisk83PageWin(const mp_string& strDevice, mp_string& strLunWWN, mp_string& strLunID)
{
    scsi_pass_through_with_buff_t stSCSIPass;
    win_dev_info_t pstDevInfo;
    FILE_HANDLE fHandle;
    mp_ulong ulLength = 0;
    mp_ulong ulReturn = 0;
    mp_char acLUNWWN[MAX_WWN_LEN] = {0};

    // CodeDex误报，FORTIFY.Path_Manipulation
    mp_int32 iStatus = InitDisk83PageRes(strDevice, pstDevInfo, fHandle);
    if (iStatus != MP_SUCCESS) {
        return iStatus;
    }

    SetScsiBufValues(stSCSIPass, ulLength, PAGE_CDB_1, PAGE_83);
    stSCSIPass.pt.PathId = (mp_uchar)pstDevInfo.iPathId;
    stSCSIPass.pt.TargetId = (mp_uchar)pstDevInfo.iScsiId;
    stSCSIPass.pt.Lun = (mp_uchar)pstDevInfo.iLunId;

    if (DeviceIoControl(fHandle, IOCTL_SCSI_PASS_THROUGH, &stSCSIPass, sizeof(SCSI_PASS_THROUGH),
        &stSCSIPass, ulLength, &ulReturn, FALSE) != MP_TRUE) {
        COMMLOG(OS_LOG_ERROR, "GetDisk0x83Page io error");
        (mp_void) CloseHandle(fHandle);
        return MP_FAILED;
    }

    if (stSCSIPass.aucData[ARRAY_INDEX_7] > MAX_SN_LEN) {
        COMMLOG(OS_LOG_ERROR, "GetDisk0x80Page Page Length error");
        (mp_void) CloseHandle(fHandle);
        return MP_FAILED;
    }

    // WWN
    mp_uchar pucSnLength = stSCSIPass.aucData[ARRAY_INDEX_7];
    if (MAX_WWN_LEN <= pucSnLength * ARRAY_NUM_2) {
        COMMLOG(OS_LOG_ERROR, "GetDisk0x80Page Page Length error");
        (mp_void) CloseHandle(fHandle);
        return MP_FAILED;
    }

    if (MP_SUCCESS != BinaryToAscii(acLUNWWN, MAX_WWN_LEN, stSCSIPass.aucData, ARRAY_NUM_8, pucSnLength)) {
        COMMLOG(OS_LOG_ERROR, "Binary to ascii failed.");
        (mp_void) CloseHandle(fHandle);
        return MP_FAILED;
    }
    strLunWWN = acLUNWWN;

    // LUN ID
    mp_char acHexData[MAX_LUNID_LEN] = {0};
    mp_char acDevLUNID[MAX_LUNID_LEN] = {0};

    if (MP_SUCCESS != ConvertLUNIDtoAscii(acHexData, MAX_LUNID_LEN, stSCSIPass.aucData + ARRAY_NUM_20, ARRAY_NUM_9)) {
        COMMLOG(OS_LOG_ERROR, "Convert lun id to ascii failed.");
        (mp_void) CloseHandle(fHandle);
        return MP_FAILED;
    }
    (mp_void) HextoDec(acDevLUNID, acHexData, MAX_LUNID_LEN);
    strLunID = acDevLUNID;
    (mp_void) CloseHandle(fHandle);

    return MP_SUCCESS;
}
#else
mp_int32 Array::GetDisk83PageNoWin(const mp_string& strDevice, mp_string& strLunWWN, mp_string& strLunID)
{
#if (defined HP_UX_IA) || (defined LINUX)
    mp_uchar aucBuffer[DATA_LEN_256] = {0};
#elif defined AIX || (defined SOLARIS)
    mp_uchar aucBuffer[BUFFER_LEN_255] = {0};
#endif

    COMMLOG(OS_LOG_INFO, "Begin to get 83 page of disk(%s).", strDevice.c_str());
    mp_string strDevicePath = strDevice;
    CMpString::FormattingPath(strDevicePath);
    // 打开scsi设备
    mp_int32 iFd = open(strDevicePath.c_str(), O_RDWR | O_NONBLOCK);
    if (iFd < 0) {
        iFd = open(strDevicePath.c_str(), O_RDONLY | O_NONBLOCK);
    }
    if (iFd < 0) {
        COMMLOG(OS_LOG_ERROR, "Open scsi device failed, iFd = %d.", iFd);
        return MP_FAILED;
    }

    int bufferLen = sizeof(aucBuffer);
    mp_int32 iRet = GetDiskPage(iFd, PAGE_83, aucBuffer, bufferLen);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get Disk 83Page failed, iRet = %d.", iRet);
        close(iFd);
        return iRet;
    }

    // 厂商和型号
    mp_string strVendor;
    mp_string strProduct;
    if (Array::GetArrayVendorAndProduct(strDevicePath, strVendor, strProduct) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get array info of disk(%s) failed.", strDevice.c_str());
        close(iFd);
        return ERROR_COMMON_QUERY_APP_LUN_FAILED;
    }

    strProduct = CMpString::Trim(strProduct);
    if (strcmp(strProduct.c_str(), PRODUCT_VBS.c_str()) != 0) {
        iRet = GetDiskWWNAndLUNID(aucBuffer, bufferLen, strLunWWN, strLunID);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get disk WWN and LUN ID failed, ret %d.", iRet);
        }
    } else {
        iRet = GetFusionStorageWWNAndLUNID(aucBuffer, bufferLen, strLunWWN, strLunID);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get FusionStorage WWN and LUN ID failed, ret %d.", iRet);
        }
    }

    close(iFd);
    return iRet;
}
#endif

/* ------------------------------------------------------------
Description  :获取WWN及LUNID信息
Input        :aucBuffer --83page页信息
Output       : strLunWWN -- WWN信息
                   strLunID -- LUN ID信息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lufei 00253739
------------------------------------------------------------- */
mp_int32 Array::GetDiskWWNAndLUNID(mp_uchar aucBuffer[], mp_int32 buffLen, mp_string& strLunWWN, mp_string& strLunID)
{
    mp_int32 iPageLen = aucBuffer[ARRAY_INDEX_7];
    if (iPageLen * ARRAY_INDEX_2 > MAX_WWN_LEN - 1) {
        COMMLOG(OS_LOG_ERROR, "Page length error, page len %d.", iPageLen);
        return MP_FAILED;
    }

    // 华为阵列WWN
    mp_char acLUNWWN[MAX_WWN_LEN] = {0};
    mp_int32 iRet = BinaryToAscii(acLUNWWN, MAX_WWN_LEN, aucBuffer, ARRAY_NUM_8, iPageLen);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Binary to ascii failed, ret %d.", iRet);
        return iRet;
    }
    strLunWWN = acLUNWWN;

    // LUN ID
    mp_char acHexData[MAX_LUNID_LEN] = {0};

    iRet = ConvertLUNIDtoAscii(acHexData, MAX_LUNID_LEN, aucBuffer + ARRAY_NUM_20, ARRAY_INDEX_9);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Convert lun id to ascii failed, ret %d.", iRet);
        return iRet;
    }
    mp_char acDevLUNID[MAX_LUNID_LEN] = {0};
    (mp_void) HextoDec(acDevLUNID, acHexData, MAX_LUNID_LEN);
    strLunID = acDevLUNID;
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :获取WWN及LUNID信息
Input        :aucBuffer --83page页信息
Output       : strLunWWN -- WWN信息
                   strLunID -- LUN ID信息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lufei 00253739
------------------------------------------------------------- */
mp_int32 Array::GetFusionStorageWWNAndLUNID(mp_uchar aucBuffer[], mp_int32 buffLen, mp_string &strLunWWN,
    mp_string &strLunID)
{
    // FusionStorage存储83页存的WWN长度
    mp_int32 iPageLen = aucBuffer[ARRAY_NUM_47];
    if (iPageLen * ARRAY_INDEX_2 > MAX_WWN_LEN - 1) {
        COMMLOG(OS_LOG_ERROR, "Page length error, page len %d.", iPageLen);
        return MP_FAILED;
    }

    // 华为阵列WWN
    mp_char acLUNWWN[MAX_WWN_LEN] = {0};
    mp_int32 iRet = BinaryToAscii(acLUNWWN, MAX_WWN_LEN, aucBuffer, ARRAY_NUM_48, iPageLen);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Binary to ascii failed, ret %d.", iRet);
        return iRet;
    }
    strLunWWN = acLUNWWN;

    // LUN ID
    mp_char acHexData[MAX_LUNID_LEN] = {0};
    mp_char acDevLUNID[MAX_LUNID_LEN] = {0};
    CHECK_NOT_OK(memcpy_s(acHexData, sizeof(acHexData), aucBuffer + ARRAY_NUM_32, ARRAY_NUM_8));
    (mp_void) HextoDec(acDevLUNID, acHexData, MAX_LUNID_LEN);
    strLunID = acDevLUNID;
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :获取80页信息
Input        :strDev -- 设备名
Output       : strSN --阵列SN
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lufei 00253739
------------------------------------------------------------- */
mp_int32 Array::GetDisk80Page(const mp_string& strDevice, mp_string& strSN)
{
#ifdef WIN32
    return GetDisk80PageWin(strDevice, strSN);
#else  // 非WIN系统
    return GetDisk80PageNoWin(strDevice, strSN);
#endif  // 非WIN系统
}

/* ------------------------------------------------------------
Description  :获取SN信息
Input        :aucBuffer --80page页信息
Output       : strSN -- SN信息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lufei 00253739
------------------------------------------------------------- */
mp_int32 Array::GetDiskSN(const mp_uchar aucBuffer[], mp_int32 buffLen, mp_string& strSN)
{
    mp_char acDiskSn[VALUE_LEN] = {0};
    mp_char acSN[MAX_SN_LEN] = {0};
    mp_int32 iPageLen = aucBuffer[ARRAY_INDEX_3];
    // 页长度
    if (iPageLen > VALUE_LEN - 1) {
        COMMLOG(OS_LOG_ERROR, "Page length error, page len %d.", iPageLen);
        return MP_FAILED;
    }

    // 序列号
    CHECK_NOT_OK(memcpy_s(acDiskSn, sizeof(acDiskSn), &aucBuffer[ARRAY_INDEX_4], iPageLen));
    // snprintf_s函数，若源字符串拷贝完全，返回拷贝的字符串数量，反之返回-1
    CHECK_FAIL(snprintf_s(acSN, mp_size(MAX_SN_LEN), mp_size(MAX_SN_HW) - 1, "%s", acDiskSn));

    strSN = acSN;
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :获取SN信息
Input        :aucBuffer --80page页信息
Output       : strSN -- SN信息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lufei 00253739
------------------------------------------------------------- */
mp_int32 Array::GetDiskFusionStorageSN(const mp_uchar aucBuffer[], mp_int32 bufferLen, mp_string& strSN)
{
    mp_char acDiskSn[VALUE_LEN] = {0};
    mp_char acSN[MAX_SN_LEN] = {0};
    // 页长度
    mp_int32 iPageLen = aucBuffer[ARRAY_INDEX_3];
    if (iPageLen > VALUE_LEN - 1) {
        COMMLOG(OS_LOG_ERROR, "Page length error, page len %d.", iPageLen);
        return MP_FAILED;
    }

    // 序列号
    CHECK_NOT_OK(memcpy_s(acDiskSn, sizeof(acDiskSn), aucBuffer + ARRAY_INDEX_4, iPageLen));
    mp_string strAcDiskSn = acDiskSn;
    strAcDiskSn = CMpString::Trim(strAcDiskSn);
    CHECK_NOT_OK(memcpy_s(acSN, sizeof(acSN), strAcDiskSn.c_str() + ARRAY_INDEX_3, ARRAY_NUM_16));
    strSN = acSN;
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :获取LUN信息
Input        :strDev -- 设备名
Output       : strLunWWN -- LUN的WWN
                   strLunID --LUN的ID
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lufei 00253739
------------------------------------------------------------- */
mp_int32 Array::GetLunInfo(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID)
{
    COMMLOG(OS_LOG_INFO, "Begin to get lun info of disk(%s).", strDev.c_str());
#ifdef WIN32
    mp_int32 iRet = GetLunInfoWin(strDev, strLunWWN, strLunID);
#else
    mp_int32 iRet = GetLunInfoNoWin(strDev, strLunWWN, strLunID);
#endif
    if (iRet != MP_SUCCESS) {
        return iRet;
    } else {
        COMMLOG(OS_LOG_INFO, "Get lun info of disk(%s) succ.", strDev.c_str());
        return MP_SUCCESS;
    }
}

/* ------------------------------------------------------------
Description  :获取阵列SN信息
Input        :strDev -- 设备名
Output       : strSN --阵列SN
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lufei 00253739
------------------------------------------------------------- */
mp_int32 Array::GetArraySN(const mp_string& strDev, mp_string& strSN)
{
    COMMLOG(OS_LOG_INFO, "Begin get SN info of disk(%s).", strDev.c_str());
#ifdef WIN32
    mp_int32 iRet = GetDisk80Page(strDev, strSN);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get 80 page of device failed.");
        return iRet;
    }
#else
    vector<mp_string> vecResult;
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_80PAGE, strDev, &vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get 80 page of device failed.");
        return iRet;
    }

    if (vecResult.empty()) {
        COMMLOG(OS_LOG_ERROR, "Get 80 page of device failed.");
        return MP_FAILED;
    }

    strSN = vecResult.front();
#endif
    COMMLOG(OS_LOG_INFO, "Get SN info of disk(%s) succ, sn %s.", strDev.c_str(), strSN.c_str());

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :二进制转为ASCII
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lufei 00253739
------------------------------------------------------------- */
mp_int32 Array::BinaryToAscii(
    mp_char pszHexBuffer[], mp_int32 iBufferLen, const mp_uchar pucBuffer[], mp_int32 iStartBuf, mp_int32 iLength)
{
    if (pucBuffer == NULL) {
        return MP_FAILED;
    }

    if (iBufferLen - 1 < iLength * ARRAY_NUM_2) {
        return MP_FAILED;
    }

    for (mp_int32 iBuffLen = 0; iBuffLen < iLength; ++iBuffLen) {
        CHECK_FAIL(sprintf_s(&pszHexBuffer[iBuffLen * ARRAY_NUM_2], iBufferLen - iBuffLen * ARRAY_NUM_2,
            "%02x", pucBuffer[iBuffLen + iStartBuf]));
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :LUN ID转为ASCII
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lufei 00253739
------------------------------------------------------------- */
mp_int32 Array::ConvertLUNIDtoAscii(
    mp_char puszAsciiLunID[], mp_int32 iBufferLen, mp_uchar puszLunLunID[], mp_int32 iLen)
{
    if ((puszAsciiLunID == NULL) || (puszLunLunID == NULL)) {
        return MP_FAILED;
    }

    if (iBufferLen - 1 < iLen) {
        return MP_FAILED;
    }

    for (mp_int32 iCount = 0; iCount < iLen / ARRAY_INDEX_2; ++iCount) {
        CHECK_FAIL(sprintf_s(&puszAsciiLunID[iCount * ARRAY_INDEX_2], iBufferLen - iCount * ARRAY_INDEX_2,
            "%02x", puszLunLunID[iCount]));
    }

    return MP_SUCCESS;
}
/* ------------------------------------------------------------
Description  : 计算16进制指数值
Input        : iStep---16进制指数值
Return       :  iResult---返回计算结果
------------------------------------------------------------- */
mp_int32 Array::CalStep(mp_int32 iStep)
{
    mp_int32 iResult = 1;

    while (iStep > 0) {
        iResult = iResult * ARRAY_NUM_16;
        iStep--;
    }

    return iResult;
}

/* ------------------------------------------------------------
Description  : 判断vecResult是否有page这个字符串
Input        : vecResult---VPD page code
Return       :  MP_TRUE--支持
               MP_FALSE---不支持
Create By    : longjiang wx290533
------------------------------------------------------------- */
mp_bool Array::IsSupportXXPage(const mp_string& page, std::vector<mp_string>& vecResult)
{
    vector<mp_string>::iterator iter = vecResult.begin();
    for (; iter != vecResult.end(); ++iter) {
        if (page == *iter) {
            return MP_TRUE;
        }
    }
    return MP_FALSE;
}

/* ------------------------------------------------------------
Description  : 十六进制转换十进制
Input        : pHexStr---十六进制
Output       : pDecStr---十进制
Return       :  MP_SUCCESS---转换成功
               MP_FAILED---转换失败
------------------------------------------------------------- */
mp_int32 Array::HextoDec(mp_char pDecStr[], mp_char pHexStr[], mp_int32 iMaxLen)
{
    mp_int32 iDecNum = 0;
    mp_bool bFlag = (pDecStr == NULL) || (pHexStr == NULL);
    if (bFlag) {
        return MP_FAILED;
    }
    mp_int32 iStrLen = (mp_int32)strlen(pHexStr);
    for (mp_int32 iTemp = 0; iTemp < iStrLen; ++iTemp) {
        mp_int32 iStep = (iStrLen - iTemp) - 1;
        if (pHexStr[iTemp] == '0') {
            continue;
        }

        bFlag = (pHexStr[iTemp] > '0') && (pHexStr[iTemp] <= '9');
        if (bFlag) {
            iDecNum = iDecNum + (pHexStr[iTemp] - '0') * CalStep(iStep);
        } else if (((pHexStr[iTemp] >= 'a') && (pHexStr[iTemp] <= 'f')) ||
                   ((pHexStr[iTemp] >= 'A') && (pHexStr[iTemp] <= 'F'))) {
            (void)HexEncode(pHexStr[iTemp], iStep, iDecNum);
        } else {
            return MP_FAILED;
        }
    }
    CHECK_FAIL(snprintf_s(pDecStr, (mp_uint32)iMaxLen, (mp_uint32)iMaxLen - 1, "%d", iDecNum));

    return MP_SUCCESS;
}
/* ------------------------------------------------------------
Description  : 将十六进制编码转换成十进制
               iDecNum = iDecNum + (cHex - 55) * CalStep(iStep); A->10, B->11.
Input        : cHex---十六进制，iStep---16进制指数值
Output       : iDecNum---十进制数
Return       :  MP_SUCCESS---编码成功
               MP_FAILED---编码失败
------------------------------------------------------------- */
mp_int32 Array::HexEncode(mp_char cHex, mp_int32 iStep, mp_int32& iDecNum)
{
    mp_bool bRet = ((cHex >= 'A' && cHex <= 'F') || (cHex >= 'a' && cHex <= 'f'));
    if (!bRet) {
        COMMLOG(OS_LOG_ERROR, "Hex encode failed, cHex: %c.", cHex);
        return MP_FAILED;
    }

    if (cHex >= 'a' && cHex <= 'z') {
        cHex -= 'a' - 'A';  // 'a' --> 'A' 小写转大写
    }

    iDecNum = iDecNum + (cHex - ARRAY_NUM_55) * CalStep(iStep);  // A:10, B:11...F:15   (A: 65 - 55 = 10)
    return MP_SUCCESS;
}

mp_bool Array::CheckHuaweiLUN(const mp_string& strVendor)
{
    // 排除掉非华为的产品
    if (strcmp(strVendor.c_str(), ARRAY_VENDER_HUAWEI.c_str()) != 0 &&
        strcmp(strVendor.c_str(), VENDOR_ULTRAPATH_HUAWEI.c_str()) != 0 &&
        strcmp(strVendor.c_str(), ARRAY_VENDER_HUASY.c_str()) != 0 &&
        strcmp(strVendor.c_str(), ARRAY_VENDER_FUSION_STORAGE.c_str()) != 0) {
        return MP_FALSE;
    }
    return MP_TRUE;
}
