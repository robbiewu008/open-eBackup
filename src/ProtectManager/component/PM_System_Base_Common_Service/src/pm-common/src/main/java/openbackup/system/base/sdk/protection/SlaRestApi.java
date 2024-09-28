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
package openbackup.system.base.sdk.protection;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.protection.model.SlaBo;
import openbackup.system.base.sdk.protection.model.SlaUpdateBo;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.List;

/**
 * Common sla request api
 *
 */
@FeignClient(name = "slaRestApi", url = "${services.endpoints.protectmanager.system-base}/v1",
    configuration = CommonFeignConfiguration.class)
public interface SlaRestApi {
    /**
     * request sla policies by user defined key and value
     *
     * @param key request key, like:external_system_id
     * @param value request value
     * @return sla response
     */
    @ExterAttack
    @GetMapping("/internal/slas/policies/ext-parameters")
    @ResponseBody
    List<SlaBo> querySlaPolicyByExtParams(@RequestParam("key") String key, @RequestParam("value") String value);

    /**
     * 重置Sla user id
     *
     * @param userId user id
     */
    @PutMapping("/internal/sla/action/revoke/{user_id}")
    void revokeSlaUserId(@PathVariable("user_id") String userId);

    /**
     * query sla by id
     *
     * @param slaId sla id
     * @return sla
     * */
    @ExterAttack
    @GetMapping("/internal/slas/{sla_id}")
    @ResponseBody
    SlaBo querySlaById(@PathVariable("sla_id") String slaId);

    /**
     * 修改SLA
     *
     * @param slaBo 修改参数
     * @return 被修改SLA的UUID
     */
    @ExterAttack
    @PostMapping("/slas")
    @ResponseBody
    SlaUpdateBo updateSla(SlaBo slaBo);
}
