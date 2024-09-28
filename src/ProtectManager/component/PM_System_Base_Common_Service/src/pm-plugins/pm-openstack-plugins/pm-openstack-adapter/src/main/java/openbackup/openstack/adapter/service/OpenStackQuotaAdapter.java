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
package openbackup.openstack.adapter.service;

import openbackup.openstack.adapter.dto.OpenStackQuotaDto;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;

/**
 * OpenStack配额适配器
 *
 */
@Slf4j
@Component
public class OpenStackQuotaAdapter {
    private final OpenStackQuotaManager quotaManager;

    public OpenStackQuotaAdapter(OpenStackQuotaManager quotaManager) {
        this.quotaManager = quotaManager;
    }

    /**
     * 设置项目配额
     *
     * @param projectId OpenStack项目id
     * @param quota 配额
     */
    public void setQuota(String projectId, OpenStackQuotaDto quota) {
        quotaManager.setQuota(projectId, quota);
    }

    /**
     * 获取项目配额
     *
     * @param projectId OpenStack项目id
     * @return 项目配额
     */
    public List<OpenStackQuotaDto> getQuota(String projectId) {
        return quotaManager.getQuota(projectId);
    }
}
