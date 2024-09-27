/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
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
 * @author p30001902
 * @since 2021-01-03
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
