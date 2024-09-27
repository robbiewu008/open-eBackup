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

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;

/**
 * dee database Anonymization
 *
 * @author y30044273
 * @since 2023-09-27
 */
@FeignClient(name = "deeDbAnonymizationRest", url = "${protectengine-e-dee-db-anonymization.url}/v1/internal",
    configuration = CommonFeignConfiguration.class)
public interface DeeDbAnonymizationRest {
    /**
     * 查询是否可以删除数据库脱敏策略
     *
     * @param dataBaseId 数据库资源
     * @return 是否可以删除
     */
    @GetMapping("/anonymization/job/isRunning")
    boolean isAnonymizationRunning(@RequestParam("dbId") String dataBaseId);
}
