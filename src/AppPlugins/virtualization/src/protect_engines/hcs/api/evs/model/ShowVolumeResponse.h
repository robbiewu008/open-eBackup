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
#ifndef __SHOW_VOLUME_RESPONSE_H__
#define __SHOW_VOLUME_RESPONSE_H__

#include "common/model/ResponseModel.h"
#include "protect_engines/hcs/common/HcsMacros.h"
#include "protect_engines/hcs/api/evs/model/VolumeDetail.h"

using namespace VirtPlugin;

namespace HcsPlugin {
class ShowVolumeResponse : public ResponseModel {
public:
    ShowVolumeResponse();
    virtual ~ShowVolumeResponse();

    bool Serial();
    HSCVolDetail GetHSCVolDetail() const;
    std::string GetArch() const;
    std::string GetStatus() const;
    uint64_t GetSize() const;
    std::vector<Attachments> GetAttachPoints() const;
    std::string GetDSType() const;
    std::string GetDSSN() const;
    std::string GetLUNID() const;
    std::string GetLUNWWN() const;
    std::string GetName() const;
    std::string GetVolId() const;
    std::string GetVolBootable() const;
    std::string GetStorageIp() const;
    int32_t GetPoolId() const;
    std::string GetVolNameOnStorage() const;

protected:
    HSCVolDetail m_volumeDetail;
};

class ShowVolumeDetailResponse : public ResponseModel {
public:
    ShowVolumeDetailResponse() {}
    virtual ~ShowVolumeDetailResponse() {}

    bool Serial();
    VolumeDetailList GetHSCVolDetail() const;
protected:
    VolumeDetailList m_volumeDetailList;
};
}

#endif