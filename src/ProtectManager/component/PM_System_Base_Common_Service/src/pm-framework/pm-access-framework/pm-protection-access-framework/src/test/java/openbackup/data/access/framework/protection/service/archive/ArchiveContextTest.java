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
package openbackup.data.access.framework.protection.service.archive;

import com.huawei.LocalRedisConfiguration;
import openbackup.data.access.framework.protection.mocks.SlaMock;

import openbackup.data.access.framework.protection.service.context.ContextManager;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.protection.model.PolicyBo;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.redisson.spring.starter.RedissonAutoConfiguration;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.Optional;
import java.util.UUID;

/**
 **/
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {LocalRedisConfiguration.class, RedissonAutoConfiguration.class, ContextManager.class,
        RedissonClient.class})
public class ArchiveContextTest {
    @Autowired
    private ContextManager contextManager;

    @Autowired
    private RedissonClient redissonClient;

    private ArchiveContext archiveContext;

    private RMap<String, String> map;

    private static final String REQUEST_ID = UUID.randomUUID().toString();

    private static final String ORIGINAL_COPY_ID = UUID.randomUUID().toString();

    private static final String ARCHIVE_COPY_ID = UUID.randomUUID().toString();

    private static final String JOB_ID = UUID.randomUUID().toString();

    @Before
    public void initMock() {
        archiveContext = contextManager.getArchiveContext(REQUEST_ID);
        map = redissonClient.getMap(REQUEST_ID, StringCodec.INSTANCE);
    }

    @After
    public void clear() {
        map.clear();
    }

    /**
     * 用例名称：验证归档上下文中存在上下文的key时，获取对应的value数据时，正常返回<br/>
     * 前置条件：redis正常<br/>
     * check点：value信息不为空并且与期望值相同<br/>
     */
    @Test
    public void should_return_not_null_value_when_get_context_values_given_keys_exists_in_context() {
        // Given
        map.put(ArchiveContext.ARCHIVE_POLICY_KEY, SlaMock.getArchiveS3Policy());
        map.put(ArchiveContext.ORIGINAL_COPY_ID_KEY, ORIGINAL_COPY_ID);
        archiveContext.setArchiveCopyId(ARCHIVE_COPY_ID);
        map.put(ArchiveContext.SLA_KEY, SlaMock.getSla());
        map.put(ArchiveContext.JOB_ID, JOB_ID);
        map.put(ArchiveContext.AUTO_RETRY_TIMES, "5");
        final PolicyBo policyBo = JSONObject.fromObject(SlaMock.getArchiveS3Policy()).toBean(PolicyBo.class);
        final String extParams = JSONObject.fromObject(SlaMock.getArchiveS3Policy())
                .getString(ArchiveContext.ARCHIVE_POLICY_EXT_PARAMS_KEY);
        // When
        final PolicyBo policy = archiveContext.getPolicy();
        final String policyExtParams = archiveContext.getPolicyExtParams();
        final String originalCopyId = archiveContext.getOriginalCopyId();
        final Optional<String> archiveCopyId = archiveContext.getArchiveCopyId();
        final String slaJson = archiveContext.getSlaJson();
        final String slaName = archiveContext.getSlaName();
        final String jobId = archiveContext.getJobId();
        final int retryTimes = archiveContext.getRetryTimes();
        // Then
        Assert.assertEquals(policyBo, policy);
        Assert.assertEquals(extParams, policyExtParams);
        Assert.assertEquals(ORIGINAL_COPY_ID, originalCopyId);
        archiveCopyId.ifPresent(id -> Assert.assertEquals(ARCHIVE_COPY_ID, id));
        Assert.assertEquals(SlaMock.getSla(), slaJson);
        Assert.assertEquals("testaq", slaName);
        Assert.assertEquals(JOB_ID, jobId);
        Assert.assertEquals(5, retryTimes);
    }

    /**
     * 用例名称：验证requestId为空时，获取上下文异常<br/>
     * 前置条件：redis正常<br/>
     * check点：1.抛出异常 2.异常信息和错误码与期望相同<br/>
     */
    @Test
    public void should_throw_LegoCheckedException_when_get_context_given_requestId_is_empty() {
        LegoCheckedException exception1 =
            Assert.assertThrows(LegoCheckedException.class, () -> contextManager.getArchiveContext(null));
        Assert.assertEquals("requestId is empty, can not get context.", exception1.getMessage());
        Assert.assertEquals(CommonErrorCode.ERR_PARAM, exception1.getErrorCode());

        LegoCheckedException exception2 =
            Assert.assertThrows(LegoCheckedException.class, () -> contextManager.getArchiveContext(""));
        Assert.assertEquals("requestId is empty, can not get context.", exception2.getMessage());
        Assert.assertEquals(CommonErrorCode.ERR_PARAM, exception2.getErrorCode());
    }

    /**
     * 用例名称：验证归档上下文中不存在上下文的key时，获取对应的value数据时，抛出异常<br/>
     * 前置条件：redis正常<br/>
     * check点：1.抛出异常 2.异常信息和错误码与期望相同<br/>
     */
    @Test
    public void should_throw_LegoCheckedException_when_get_context_values_given_keys_not_exists_in_context() {
        // When and Then
        final Optional<String> archiveCopyId = archiveContext.getArchiveCopyId();
        Assert.assertFalse(archiveCopyId.isPresent());

        LegoCheckedException exception;

        exception = Assert.assertThrows(LegoCheckedException.class, () -> archiveContext.getPolicy());
        Assert.assertEquals("archive policy can not find in context", exception.getMessage());
        Assert.assertEquals(CommonErrorCode.OBJ_NOT_EXIST, exception.getErrorCode());

        exception = Assert.assertThrows(LegoCheckedException.class, () -> archiveContext.getPolicyExtParams());
        Assert.assertEquals("archive policy can not find in context", exception.getMessage());
        Assert.assertEquals(CommonErrorCode.OBJ_NOT_EXIST, exception.getErrorCode());

        exception = Assert.assertThrows(LegoCheckedException.class, () -> archiveContext.getOriginalCopyId());
        Assert.assertEquals("originalCopyId can not find in context", exception.getMessage());
        Assert.assertEquals(CommonErrorCode.OBJ_NOT_EXIST, exception.getErrorCode());

        exception = Assert.assertThrows(LegoCheckedException.class, () -> archiveContext.getSlaJson());
        Assert.assertEquals("sla can not find in context", exception.getMessage());
        Assert.assertEquals(CommonErrorCode.OBJ_NOT_EXIST, exception.getErrorCode());

        exception = Assert.assertThrows(LegoCheckedException.class, () -> archiveContext.getSlaName());
        Assert.assertEquals("sla can not find in context", exception.getMessage());
        Assert.assertEquals(CommonErrorCode.OBJ_NOT_EXIST, exception.getErrorCode());

        Assert.assertEquals(0, archiveContext.getRetryTimes());
    }
}