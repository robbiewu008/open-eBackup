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
package openbackup.system.base.sdk.dee;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.dee.model.CommonResponse;
import openbackup.system.base.sdk.dee.model.ModifyEsClusterReq;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;

/**
 * 功能描述
 *
 */
@FeignClient(name = "deeConfigRest", url = "${protectengine.url}/v1/internal",
    configuration = CommonFeignConfiguration.class)
public interface DeeConfigRest {
    /**
     * es集群信息配置接口
     *
     * @param modifyEsClusterReq 信息
     * @return 配置结果
     */
    @ExterAttack
    @PutMapping("/config/escluster")
    CommonResponse modifyEsCluster(@RequestBody @Validated ModifyEsClusterReq modifyEsClusterReq);
}
