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
package openbackup.system.base.sdk.host;

import com.huawei.emeistor.kms.kmc.util.security.exterattack.ExterAttack;
import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.host.model.AsmAuthEntity;
import openbackup.system.base.sdk.host.model.AsmInfo;
import openbackup.system.base.sdk.host.model.Host;
import openbackup.system.base.sdk.host.model.HostDetail;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.List;

/**
 * JobCenter Client Service
 *
 * @author h30003246
 * @since 2020-07-16
 */
@FeignClient(name = "host-service", url = "${pm-resource-manager.url}/v1",
    configuration = CommonFeignConfiguration.class)
public interface HostRestApi {
    /**
     * query host
     *
     * @param hostId sorting
     * @return PageListResponse list
     */
    @ExterAttack
    @GetMapping("/internal/resource/host/{host_id}")
    @ResponseBody
    Host queryHostByID(@PathVariable("host_id") String hostId);

    /**
     * query asm info for oracle target host
     *
     * @param hostId sorting
     * @return PageListResponse list
     */
    @ExterAttack
    @GetMapping("/internal/resource/host/{host_id}/asm-info")
    @ResponseBody
    List<AsmInfo> queryHostAsmById(@PathVariable("host_id") String hostId);

    /**
     * query asm auth info for oracle target host
     *
     * @param hostId sorting
     * @return PageListResponse list
     */
    @ExterAttack
    @GetMapping("/internal/resource/host/{host_id}/asm-authinfo")
    @ResponseBody
    AsmAuthEntity queryHostAsmAuthById(@PathVariable("host_id") String hostId);

    /**
     * query hosts
     *
     * @param page      page
     * @param size      size
     * @param typeOfApp type of app
     * @param uuid      host uuid
     * @return hosts
     */
    @ExterAttack
    @GetMapping("/resource/host/")
    BasePage<HostDetail> queryHosts(@RequestParam("page_no") int page, @RequestParam("page_size") int size,
        @RequestParam(value = "type_of_app", required = false) String typeOfApp,
        @RequestParam(value = "uuid", required = false) String uuid);

    /**
     * refresh host
     *
     * @param hostId       host id
     * @param resourceName resource name
     * @param isValid      valid
     * @param hasClearProtection  has clear protection object
     * @return resource uuid
     */
    @ExterAttack
    @PutMapping("/internal/resource/host/{host_id}/refresh_host")
    List<String> refreshHost(@PathVariable("host_id") String hostId, @RequestParam("db_name") String resourceName,
        @RequestParam("is_valid") Boolean isValid,
        @RequestParam("is_clear_protection_object") Boolean hasClearProtection);
}
