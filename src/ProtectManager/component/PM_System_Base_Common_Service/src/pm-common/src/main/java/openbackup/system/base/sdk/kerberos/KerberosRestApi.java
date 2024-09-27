/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
 * @author m00576658
 * @since 2021-08-18
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
