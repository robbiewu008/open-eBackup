/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
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
