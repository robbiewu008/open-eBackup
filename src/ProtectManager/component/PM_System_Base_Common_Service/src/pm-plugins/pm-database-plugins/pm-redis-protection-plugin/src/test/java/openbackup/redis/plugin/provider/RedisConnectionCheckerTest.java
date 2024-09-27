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
package openbackup.redis.plugin.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.database.base.plugin.common.DatabaseConstants;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.redis.plugin.constant.RedisConstant;
import openbackup.redis.plugin.service.RedisService;
import com.huawei.oceanprotect.system.base.kerberos.service.KerberosService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.UUID;

/**
 * redis资源检查checker测试类
 *
 * @author x30028756
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-15
 */
@RunWith(PowerMockRunner.class)
public class RedisConnectionCheckerTest {
    private RedisConnectionChecker redisConnectionChecker;

    private AgentUnifiedService agentUnifiedService;

    private RedisService redisService;

    private KerberosService kerberosService;

    private EncryptorService encryptorService;

    @Before
    public void setUp() {
        ProtectedEnvironmentRetrievalsService environmentRetrievalsService = PowerMockito.mock(
            ProtectedEnvironmentRetrievalsService.class);
        agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);
        redisService = PowerMockito.mock(RedisService.class);
        kerberosService = PowerMockito.mock(KerberosService.class);
        encryptorService = PowerMockito.mock(EncryptorService.class);
        redisConnectionChecker = new RedisConnectionChecker(environmentRetrievalsService, agentUnifiedService,
            redisService, kerberosService, encryptorService);
    }

    /**
     * 用例场景：redis类型识别
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void applicable_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.REDIS.getType());
        Assert.assertTrue(redisConnectionChecker.applicable(protectedResource));
    }

    /**
     * 用例场景：获取检查结果
     * 前置条件：连通性检查成功
     * 检查点： 返回检查结果成功
     */
    @Test
    public void collect_action_results_success() {
        ActionResult actionResult = new ActionResult();
        actionResult.setMessage("success");
        actionResult.setCode(0);
        List<CheckReport<Object>> checkReports = new ArrayList<>();
        List<CheckResult<Object>> results = new ArrayList<>();
        CheckReport<Object> checkReport = new CheckReport<>();
        CheckResult<Object> result = new CheckResult<>();
        results.add(result);
        result.setResults(actionResult);
        checkReport.setResults(results);
        checkReports.add(checkReport);
        List<ActionResult> resultList = redisConnectionChecker.collectActionResults(checkReports, new HashMap<>());
        Assert.assertEquals(resultList.size(), 1);
    }

    /**
     * 用例场景：获取检查结果
     * 前置条件：连通性检查失败
     * 检查点： 抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_generate_check_result_failed() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setName("192_168_143_104_22401");
        protectedResource.setUuid("123");
        protectedResource.setType(ResourceTypeEnum.DATABASE.getType());
        protectedResource.setSubType(ResourceSubTypeEnum.REDIS.getType());
        AgentBaseDto agent = new AgentBaseDto();
        agent.setErrorCode("0");
        PowerMockito.when(agentUnifiedService.checkApplicationNoRetry(ArgumentMatchers.any(), ArgumentMatchers.any()))
            .thenReturn(agent);
        protectedResource.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = -3072900173876080471L;

            {
                put("kerberosId", "15cf44359b63400cbbca9dfe167ae01b");
            }
        });
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setEndpoint("192.167.1.1");
        protectedEnvironment.setPort(11);
        protectedResource.setEnvironment(protectedEnvironment);
        Authentication auth = new Authentication();
        auth.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = -1485587773853006904L;

            {
                put("kerberosId", "15cf44359b63400cbbca9dfe167ae01b");
            }
        });
        auth.setAuthType(Authentication.NO_AUTH);
        protectedResource.setAuth(auth);
        protectedResource.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = -2029513112707309025L;

            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "127");
                put(RedisConstant.IP, "192.164.1.4");
                put(RedisConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedResource host1 = new ProtectedResource();
        host1.setUuid(UUID.randomUUID().toString());
        protectedResource.setDependencies(Collections.singletonMap("agents", Arrays.asList(host1)));
        CheckResult<Object> result = redisConnectionChecker.generateCheckResult(protectedResource);
        Assert.assertEquals(0L, result.getResults().getCode());
    }

    /**
     * 用例场景：获取检查结果
     * 前置条件：连通性检查失败
     * 检查点： 抛出异常
     */
    @Test
    public void collectConnectableResources_success() throws Exception {
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("123");
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        PowerMockito.when(agentUnifiedService.checkApplicationNoRetry(ArgumentMatchers.any(), ArgumentMatchers.any()))
            .thenReturn(agentBaseDto);
        resource.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = 283472507331789663L;

            {
                put("kerberosId", "15cf44359b63400cbbca9dfe167ae01b");
            }
        });
        Authentication authentication = new Authentication();
        authentication.setAuthType(Authentication.NO_AUTH);
        authentication.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = 5241759212132558558L;

            {
                put("kerberosId", "15cf44359b63400cbbca9dfe167ae01b");
            }
        });
        resource.setAuth(authentication);
        ProtectedResource son01 = new ProtectedResource();
        son01.setName("xxx1");
        son01.setUuid("123");
        ProtectedResource son2 = new ProtectedResource();
        son2.setName("xxx1");
        son2.setUuid("456");
        authentication.setAuthType(Authentication.NO_AUTH);
        son01.setAuth(authentication);
        son2.setAuth(authentication);
        son01.setDependencies(Collections.singletonMap(DatabaseConstants.AGENTS, Lists.newArrayList(son2)));
        resource.setDependencies(ImmutableMap.of(ResourceConstants.CHILDREN, Arrays.asList(son01)));
        ProtectedEnvironmentService environmentService = PowerMockito.mock(ProtectedEnvironmentService.class);
        PowerMockito.when(environmentService.getEnvironmentById(ArgumentMatchers.anyString()))
            .thenReturn(new ProtectedEnvironment());
        Class<?> classType = redisConnectionChecker.getClass();
        Field a = classType.getDeclaredField("environmentService");
        a.setAccessible(true);
        a.set(redisConnectionChecker, environmentService);
        redisConnectionChecker.collectConnectableResources(resource);
    }
}