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
package openbackup.system.base.sdk.kerberos;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.kerberos.model.KerberosBo;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;

/**
 * Kerberos Rest Api
 *
 */
@FeignClient(
        name = "kerberos-rest-api",
        url = "${pm-system-base.url}/v1",
        configuration = CommonFeignConfiguration.class)
public interface KerberosRestApi {
    /**
     * query kerberos by id
     *
     * @param kerberosId kerberosId
     * @return KerberosBo KerberosBo
     */
    @ExterAttack
    @GetMapping("internal/kerberos/{kerberos_id}")
    KerberosBo queryKerberos(@PathVariable("kerberos_id") String kerberosId);
}
