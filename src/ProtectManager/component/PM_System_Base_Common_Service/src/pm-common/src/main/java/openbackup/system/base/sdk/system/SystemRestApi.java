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
package openbackup.system.base.sdk.system;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.system.model.ConfigStatus;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.ResponseBody;

/**
 * 查询系统是否初始化完毕
 *
 * @author w30042425
 * @since 2022-12-28
 */
@FeignClient(name = "SystemFeignClient", url = "${pm-system-base.url}/v1/system",
    configuration = CommonFeignConfiguration.class)
public interface SystemRestApi {
    /**
     * 查询初始化情况
     *
     * @return 初始化状态
     */
    @ExterAttack
    @GetMapping(value = "/internal/initConfig")
    @ResponseBody
    ConfigStatus getInitConfigInternal();
}
