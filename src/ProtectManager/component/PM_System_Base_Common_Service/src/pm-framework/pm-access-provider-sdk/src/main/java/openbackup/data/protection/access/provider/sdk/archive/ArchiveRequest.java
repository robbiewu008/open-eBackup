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
package openbackup.data.protection.access.provider.sdk.archive;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * 副本归档请求体
 *
 * @author y00490893
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-11-19
 */
public class ArchiveRequest {
    // 副本Id
    @JsonProperty("copy_id")
    private String copyId;

    // 保护策略
    @JsonProperty("policy")
    private String policy;

    // 副本所对应保护对象的类型，如Oracle
    @JsonProperty("resource_sub_type")
    private String resourceSubType;

    @JsonProperty("resource_type")
    private String resourceType;

    @JsonProperty("sla_name")
    private String slaName;

    public String getCopyId() {
        return copyId;
    }

    public void setCopyId(String copyId) {
        this.copyId = copyId;
    }

    public String getPolicy() {
        return policy;
    }

    public void setPolicy(String policy) {
        this.policy = policy;
    }

    public String getResourceSubType() {
        return resourceSubType;
    }

    public void setResourceSubType(String resourceSubType) {
        this.resourceSubType = resourceSubType;
    }

    public String getResourceType() {
        return resourceType;
    }

    public void setResourceType(String resourceType) {
        this.resourceType = resourceType;
    }

    public String getSlaName() {
        return slaName;
    }

    public void setSlaName(String slaName) {
        this.slaName = slaName;
    }
}
