#ifndef _AGENTCLI_GET_FILE_FROM_ARCHIVE_H_
#define _AGENTCLI_GET_FILE_FROM_ARCHIVE_H_

#include <vector>
#include <thread>
#include <memory>
#include <thread>

#include "common/Types.h"
#include "message/archivestream/ArchiveStreamService.h"

class GetFileFromArchive {
public:
    static mp_int32 Handle(const mp_string &backupId, const mp_string &busiIp,
        const mp_string &localPath, const mp_string &dirList);

private:
    static mp_int32 GetFileListInfo(std::unique_ptr<ArchiveStreamService> &clientHandler, const mp_string &localPath);
    static mp_int32 HandleFileListInfo(std::unique_ptr<ArchiveStreamService> &clientHandler, mp_string &splitFile,
        const mp_string &localPath);
    static mp_int32 DownloadFile(std::unique_ptr<ArchiveStreamService> &clientHandler,
        const mp_string &strDecoFileName, const mp_string &fsID, const mp_string &fileName);
};

#endif  // _AGENTCLI_GET_FILE_FROM_ARCHIVE_H_
