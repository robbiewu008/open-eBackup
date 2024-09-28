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
package openbackup.system.base.controller;

import openbackup.system.base.service.ConfigMapServiceImpl;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

/**
 * 内部查询secret信息入口类
 *
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
