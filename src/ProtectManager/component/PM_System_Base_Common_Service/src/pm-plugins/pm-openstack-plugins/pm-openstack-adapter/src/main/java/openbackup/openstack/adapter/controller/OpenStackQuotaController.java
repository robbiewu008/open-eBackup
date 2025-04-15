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
package openbackup.openstack.adapter.controller;

import com.huawei.emeistor.kms.kmc.util.security.exterattack.ExterAttack;

import lombok.extern.slf4j.Slf4j;
import openbackup.openstack.adapter.constants.OpenStackConstants;
import openbackup.openstack.adapter.controller.resp.OpenStackQuotaResp;
import openbackup.openstack.adapter.dto.OpenStackQuotaDto;
import openbackup.openstack.adapter.service.OpenStackQuotaAdapter;

import org.springframework.http.HttpStatus;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.ResponseStatus;
import org.springframework.web.bind.annotation.RestController;

import javax.servlet.http.HttpServletRequest;

/**
 * 云核OpenStack配额北向接口
 * <p>
 * 提供第三方调用，不使用系统内部token校验机制
 * </p>
 *
 */
@Slf4j
@RestController
@RequestMapping("/v2/backup_quota")
public class OpenStackQuotaController {
    private final OpenStackQuotaAdapter adapter;

    public OpenStackQuotaController(OpenStackQuotaAdapter adapter) {
        this.adapter = adapter;
    }

    /**
     * 设置配额
     *
     * @param quota 配额
     * @param request HttpServletRequest
     */
    @ExterAttack
    @ResponseStatus(HttpStatus.CREATED)
    @PostMapping
    public void setQuota(@Validated @RequestBody OpenStackQuotaDto quota, HttpServletRequest request) {
        String projectId = request.getAttribute(OpenStackConstants.PROJECT_ID).toString();
        log.info("OpenStack set project: {} quota: {}(GB).", projectId, quota.getSize());
        adapter.setQuota(projectId, quota);
    }

    /**
     * 查询配额
     *
     * @param request HttpServletRequest
     * @return 配额信息
     */
    @ExterAttack
    @GetMapping
    public OpenStackQuotaResp queryQuota(HttpServletRequest request) {
        String projectId = request.getAttribute(OpenStackConstants.PROJECT_ID).toString();
        log.info("OpenStack query project: {} quota.", projectId);
        return new OpenStackQuotaResp(adapter.getQuota(projectId));
    }
}
