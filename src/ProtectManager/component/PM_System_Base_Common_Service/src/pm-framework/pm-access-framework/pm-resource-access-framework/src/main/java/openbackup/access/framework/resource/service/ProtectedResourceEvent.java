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
package openbackup.access.framework.resource.service;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

/**
 * Protected Resource Event
 *
 */
public class ProtectedResourceEvent {
    private String type;
    private ProtectedResource resource;

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }

    public ProtectedResource getResource() {
        return resource;
    }

    public void setResource(ProtectedResource resource) {
        this.resource = resource;
    }
}
