/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.controller;

import openbackup.system.base.service.ConfigMapServiceImpl;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

/**
 * 内部查询secret信息入口类
 *
 * @author xwx1016404
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-28
 */
@RestController
@RequestMapping("/v1/secret")
public class SecretController {
    /**
     * Redis密码在secret中的key
     */
    private static final String REDIS_AUTH_KEY = "redis.password";

    @Autowired
    private ConfigMapServiceImpl configMapService;

    /**
     * 获取common-secret中Redis密码
     *
     * @return 密文
     */
    @GetMapping("/redis")
    public String encrypt() {
        return configMapService.getValueFromSecretByKey(REDIS_AUTH_KEY);
    }
}
