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
#ifndef HYPERV_CREATE_VHD_MODEL_H
#define HYPERV_CREATE_VHD_MODEL_H

namespace HyperVPlugin {
class CreateVHDRequest {
public:
	std::string m_path;
    uint64_t m_size;
    std::string m_type; // fix/dynamic/differencing
    std::string m_volumeType; // vhd/vhdx
    std::string m_authKey;
    std::string m_authPwd;
};

class CreateVHDResponse {
public:
	uint64_t m_fileSize;
    uint64_t m_size;
    std::string m_id;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_fileSize, FileSize)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_size, Size)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, DiskIdentifier)
    END_SERIAL_MEMEBER
};
}

#endif