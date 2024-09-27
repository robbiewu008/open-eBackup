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
#ifndef HUAWEICLOUD_SDK_EVS_V2_MODEL_SHOW_VOLUME_REQUEST_H
#define HUAWEICLOUD_SDK_EVS_V2_MODEL_SHOW_VOLUME_REQUEST_H

#include <string>
#include "common/model/ModelBase.h"
#include "protect_engines/hcs/common/HcsCommonInfo.h"
#include "protect_engines/hcs/common/HcsMacros.h"
#include "common/token_mgr/TokenDetail.h"

using namespace VirtPlugin;

namespace HcsPlugin {
class ShowVolumeRequest : public ModelBase {
public:
    ShowVolumeRequest();
    virtual ~ShowVolumeRequest();

    virtual Scope GetScopeType() const override;
    virtual ApiType GetApiType() override;

    std::string GetVolumeId() const;

    void SetVolumeId(const std::string& value);

protected:
    std::string m_volumeId; // 云硬盘ID。
};

class ShowVolumeListRequest : public ModelBase {
public:
    ShowVolumeListRequest() {}
    virtual ~ShowVolumeListRequest() {}

    virtual Scope GetScopeType() const override;
    virtual ApiType GetApiType() override;

    int32_t GetVolumeOffSet() const;
    void SetVolumeOffset(int32_t offset);
    bool GetVolumeOffsetIsSet() const;

    int32_t GetVolumeLimit() const;
    void SetVolumeLimit(int32_t limit);
    bool GetVolumeLimitIsSet() const;

    std::string GetVolumeStatus() const;
    void SetVolumeStatus(const std::string& status);
    bool GetVolumeStatusIsSet() const;
protected:
    int32_t m_offset;
    bool m_offstIsSet = false;
    int32_t m_limit;
    bool m_limitIsSet = false;
    std::string m_status = "";
    bool m_statusIsSet = false;
};
}

#endif