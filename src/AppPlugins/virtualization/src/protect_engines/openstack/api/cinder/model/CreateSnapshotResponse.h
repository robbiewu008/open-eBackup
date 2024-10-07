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
#ifndef OPENSTACK_CREATE_SNAPSHOT_RESPONSE_H
#define OPENSTACK_CREATE_SNAPSHOT_RESPONSE_H

#include "SnapshotDetails.h"
#include "common/model/ResponseModel.h"
#include "common/token_mgr/TokenDetail.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"

using VirtPlugin::ResponseModel;

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

class CreateSnapshotResponse : public ResponseModel {
public:
    CreateSnapshotResponse();
    virtual ~CreateSnapshotResponse();

    bool Serial();
    const SnapshotDetailsMsg& GetSnapshotDetails() const;

protected:
    SnapshotDetailsMsg m_snapshot;
};

OPENSTACK_PLUGIN_NAMESPACE_END

#endif // CREATE_SNAPSHOT_RESPONSE_H