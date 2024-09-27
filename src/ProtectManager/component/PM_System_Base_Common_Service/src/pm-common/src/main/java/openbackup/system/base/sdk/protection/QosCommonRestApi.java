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
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.protection.model.QosBo;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

/**
 * qos调用的api
 *
 * @author twx1009756
 * @since 2020-12-26
 */
@FeignClient(name = "qos-common-rest-api", url = "${pm-data-protection-service.url}/v1",
    configuration = CommonFeignConfiguration.class)
public interface QosCommonRestApi {
    /**
     * 获取qos表数据
     *
     * @param qosId 带宽限速策略
     * @return PolicyResponse 回复
     */
    @ExterAttack
    @GetMapping("/internal/qos/{qos_id}")
    @ResponseBody
    QosBo queryQos(@PathVariable("qos_id") String qosId);

    /**
     * 重置Qos user id
     *
     * @param userId user id
     */
    @ExterAttack
    @PutMapping("/internal/qos/action/revoke/{user_id}")
    void revokeQosUserId(@PathVariable("user_id") String userId);

    /**
     * 获取qos所有列表
     *
     * @param pageNo 第几页
     * @param pageSize 每页数量
     * @return BasePage 分页列表
     */
    @ExterAttack
    @GetMapping("/internal/qos")
    BasePage<QosBo> queryQosList(@RequestParam("page_no") Integer pageNo, @RequestParam("page_size") Integer pageSize);
}
