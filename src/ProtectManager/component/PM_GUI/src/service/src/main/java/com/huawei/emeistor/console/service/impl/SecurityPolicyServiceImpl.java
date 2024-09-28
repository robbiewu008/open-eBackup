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
package com.huawei.emeistor.console.service.impl;

import com.huawei.emeistor.console.bean.SecurityPolicyBo;
import com.huawei.emeistor.console.contant.DeployTypeEnum;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.service.SecurityPolicyService;
import com.huawei.emeistor.console.util.NormalizerUtil;

import com.alibaba.fastjson.JSONObject;

import org.redisson.api.RBucket;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpMethod;
import org.springframework.stereotype.Service;
import org.springframework.web.client.RestTemplate;

/**
 * 功能描述
 *
 */
@Service
public class SecurityPolicyServiceImpl implements SecurityPolicyService {
    private static final String SEC_POLICY = "SEC_POLICY_JSON";

    /**
     * 安全策略修改url
     */
    private static final String SEC_URL = "/v1/security";

    /**
     * 安全一体机安全策略修改url
     */
    private static final String OCEAN_CYBER_SEC_URL = "/v1/security/banner";

    /**
     * 部署类型
     */
    private static final String DEPLOY_TYPE = "DEPLOY_TYPE";

    @Value("${api.gateway.endpoint}")
    private String userApi;

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private RestTemplate restTemplate;

    @Autowired
    @Qualifier("userRestTemplate")
    private RestTemplate userRestTemplate;

    /**
     * 返回安全策略
     *
     * @param httpEntity 带header的请求体
     * @return 返回安全策略
     */
    @Override
    @ExterAttack
    public SecurityPolicyBo getSecurityPolicy(HttpEntity httpEntity) {
        RBucket<String> policy = redissonClient.getBucket(SEC_POLICY);
        if (policy.isExists()) {
            return JSONObject.parseObject(policy.get(), SecurityPolicyBo.class);
        }

        return userRestTemplate
            .exchange(NormalizerUtil.normalizeForString(userApi + SEC_URL), HttpMethod.GET, httpEntity,
                SecurityPolicyBo.class)
            .getBody();
    }

    /**
     * 功能描述
     *
     * @param httpEntity 请求入参
     */
    @Override
    @ExterAttack
    public void updateSecurityPolicy(HttpEntity<SecurityPolicyBo> httpEntity) {
        RBucket<String> policy = redissonClient.getBucket(SEC_POLICY);
        if (policy.isExists()) {
            policy.delete();
        }
        String product = System.getenv(DEPLOY_TYPE);
        if (DeployTypeEnum.getByValue(product) == DeployTypeEnum.OCEAN_CYBER) {
            restTemplate.exchange(NormalizerUtil.normalizeForString(userApi + OCEAN_CYBER_SEC_URL),
                HttpMethod.PUT, httpEntity, SecurityPolicyBo.class);
        } else {
            restTemplate.exchange(NormalizerUtil.normalizeForString(userApi + SEC_URL),
                HttpMethod.PUT, httpEntity, SecurityPolicyBo.class);
        }
        policy.set(JSONObject.toJSONString(httpEntity.getBody()));
    }
}
