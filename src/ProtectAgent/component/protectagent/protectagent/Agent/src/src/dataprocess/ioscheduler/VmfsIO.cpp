#include <dirent.h>
#include <climits>
#include <cstdlib>
#include "dataprocess/ioscheduler/VmfsIO.h"
#include "dataprocess/vmwarenative/Define.h"
#include "common/Utils.h"

namespace NSVmfsIO {

mp_int32 VmfsIO::Init()
{
    for (const auto &wwn : m_wwn) {
        COMMLOG(OS_LOG_INFO, "vmfsio - initialize vmfs device, wwn: %s", wwn.c_str());
        if (!ResolveDevicePath(wwn)) {
            COMMLOG(OS_LOG_ERROR, "vmfsio - resolve device path from wwn failed, wwn: %s", wwn.c_str());
            return MP_FAILED;
        }
    }

    if (m_device.size() == 0) {
        COMMLOG(OS_LOG_ERROR, "vmfsio - no device for vmfs initialize.");
        return MP_FAILED;
    }

    Vmfs5IO::VmfsFlagsT flags;
    flags.packed = 0;
    flags.allowMissingExtents = 1;
    COMMLOG(OS_LOG_INFO, "vmfsio - try open device and load as vmfs4/5.");
    m_vmfs = Vmfs5IO::VmfsFs::Instance()->FSOpen(m_device, flags);
    if (m_vmfs != nullptr) {
        COMMLOG(OS_LOG_INFO, "vmfsio - Open vmfs4/5 filesystem success.");
        m_vmfsVersion = VmfsVerT::VMFS5;
        return MP_SUCCESS;
    }

    Vmfs6IO::VmfsFlagsT flags6;
    flags6.packed = 0;
    flags6.allowMissingExtents = 1;
    COMMLOG(OS_LOG_INFO, "vmfsio - try open device and load as vmfs6.");
    m_vmfs6 = Vmfs6IO::VmfsFs::Instance()->FSOpen(m_device, flags6);
    if (m_vmfs6 != nullptr) {
        COMMLOG(OS_LOG_INFO, "vmfsio - Open vmfs6 filesystem success.");
        m_vmfsVersion = VmfsVerT::VMFS6;
        return MP_SUCCESS;
    }

    COMMLOG(OS_LOG_INFO, "vmfsio - initialize failed.");
    return MP_FAILED;
}

bool VmfsIO::ResolveDevicePath(const std::string &wwn)
{
    std::vector<std::string> fileList;
    std::string diskRoot("/dev/disk/by-id/");
    GetFiles(diskRoot, fileList);

    COMMLOG(OS_LOG_DEBUG, "vmfsio - resolve disk file, wwn: %s", wwn.c_str());
    for (const auto &file : fileList) {
        std::string regStr = "wwn-0x" + wwn + "-part\\d+";
        std::regex reg(regStr);
        bool ret = std::regex_search(file, reg);
        if (!ret) {
            continue;
        }

        mp_char reslvedPath[PATH_MAX + 1] = {0};
        if (realpath(file.c_str(), reslvedPath) == NULL) {
            COMMLOG(OS_LOG_ERROR, "vmfsio - resolve file realpath failed. file: %s", file.c_str());
            return false;
        }
        m_device.insert(std::string(reslvedPath));
        COMMLOG(OS_LOG_DEBUG, "vmfsio - resolve disk(%s) to device(%s).", file.c_str(), reslvedPath);
    }
    return true;
}

void VmfsIO::GetFiles(const std::string &pathName, std::vector<std::string> &files)
{
    COMMLOG(OS_LOG_DEBUG, "vmfsio - list files in directory %s", pathName.c_str());
    DIR *dir;
    struct dirent *ptr;
    if ((dir = opendir(pathName.c_str())) == NULL) {
        COMMLOG(OS_LOG_ERROR, "vmfsio - open dir error: errno[%d]:[%s]", errno, strerror(errno));
        return;
    }

    while ((ptr = readdir(dir)) != NULL) {
        COMMLOG(OS_LOG_INFO, "vmfsio - item: %s, type: %d", ptr->d_name, ptr->d_type);
        if (strcmp(ptr->d_name, ".") == MP_SUCCESS ||
            strcmp(ptr->d_name, "..") == MP_SUCCESS) { // current dir OR parrent dir
            continue;
        } else if (ptr->d_type == DT_REG || ptr->d_type == DT_LNK) { // file
            std::string strFile = pathName;
            strFile += "/";
            strFile += ptr->d_name;
            files.push_back(strFile);
        } else {
            continue;
        }
    }
    closedir(dir);
}

}
