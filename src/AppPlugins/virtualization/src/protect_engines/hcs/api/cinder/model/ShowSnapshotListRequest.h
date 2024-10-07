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
#ifndef SHOW_SNAPSHOT_LIST_REQUEST_H
#define SHOW_SNAPSHOT_LIST_REQUEST_H

#include <string>
#include "common/model/ModelBase.h"
#include "protect_engines/hcs/common/HcsMacros.h"
#include "common/token_mgr/TokenDetail.h"

using namespace VirtPlugin;

namespace HcsPlugin {
class ShowSnapshotListRequest : public ModelBase {
public:
    ShowSnapshotListRequest();
    virtual ~ShowSnapshotListRequest();

    virtual Scope GetScopeType() const override;
    virtual ApiType GetApiType() override;

    int32_t GetLimit() const;
    bool LimitIsSet() const;
    void UnsetLimit();
    void SetLimit(const int32_t& limit);

    int32_t GetOffset() const;
    bool OffsetIsSet() const;
    void UnsetOffset();
    void SetOffset(const int32_t& offset);

    std::string GetVolumeId() const;
    bool VolumeIdIsSet() const;
    void SetVolumeId(const std::string& id);

protected:
    int32_t m_limit;
    bool m_limitIsSet { false };

    int32_t m_offset;
    bool m_offsetIsSet { false };

    std::string m_volumeID;
    bool m_volumeIDIsSet { false };
};
}

#endif // SHOW_SNAPSHOT_LIST_REQUEST_H
