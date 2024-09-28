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
package openbackup.system.base.sdk.resource;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.resource.model.Database;
import openbackup.system.base.sdk.resource.model.DatasourceEntity;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.List;

/**
 * get protected resource
 *
 */
@FeignClient(name = "OracleRestApi", url = "${pm-resource-manager.url}/v1",
    configuration = CommonFeignConfiguration.class)
public interface OracleRestApi {
    /**
     * 获取数据库信息
     *
     * @param resourceId 资源ID
     * @return FileSetEntity
     */
    @ExterAttack
    @GetMapping("/internal/resource/{resource_id}")
    @ResponseBody
    DatasourceEntity queryResource(@PathVariable("resource_id") String resourceId);

    /**
     * query copies
     *
     * @param page      page
     * @param size      size
     * @param orders    orders
     * @param condition condition
     * @return copies
     */
    @ExterAttack
    @GetMapping("/internal/copies")
    @ResponseBody
    BasePage<DatasourceEntity> queryCopies(@RequestParam("page_no") int page, @RequestParam("page_size") int size,
        @RequestParam("conditions") String condition, @RequestParam("orders") List<String> orders);

    /**
     * query resources
     *
     * @param page      page
     * @param size      size
     * @param orders    orders
     * @param condition condition
     * @return resources
     */
    @ExterAttack
    @GetMapping("/internal/databases")
    BasePage<DatasourceEntity> queryResources(@RequestParam("page_no") int page, @RequestParam("page_size") int size,
        @RequestParam("conditions") String condition, @RequestParam("orders") List<String> orders);

    /**
     * query databases by agent
     *
     * @param hostIp hostIp
     * @return list databases of agent
     */
    @ExterAttack
    @GetMapping("/internal/agent/{host_ip}/databases")
    List<Database> queryDatabases(@PathVariable("host_ip") String hostIp);
}
