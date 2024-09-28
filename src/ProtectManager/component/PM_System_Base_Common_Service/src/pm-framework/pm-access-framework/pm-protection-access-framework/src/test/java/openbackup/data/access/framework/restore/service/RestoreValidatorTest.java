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
package openbackup.data.access.framework.restore.service;

import openbackup.data.access.client.sdk.api.framework.dme.CopyVerifyStatusEnum;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.protection.mocks.CopyMocker;
import openbackup.data.access.framework.protection.mocks.ProtectedResourceMocker;

import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.restore.RestoreFeature;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import org.assertj.core.api.Assertions;
import org.junit.Assert;
import org.junit.Test;
import org.springframework.beans.BeanUtils;

import java.util.HashMap;
import java.util.Map;

/**
 * RestoreValidator工具类单元测试集合
 *
 **/
public class RestoreValidatorTest {
    /**
     * 用例名称：验证环境连接状态离线时，调用校验方法抛出异常<br/>
     * 前置条件：无<br/>
     * check点：1.异常与期望一致 2. 异常信息与期望一致<br/>
     */
    @Test
    public void should_throw_LegoCheckedException_when_checkTargetEnvIsOnline_given_status_offline() {
        // Given
        final ProtectedEnvironment protectedEnvironment = ProtectedResourceMocker.mockTaskEnv();
        protectedEnvironment.setLinkStatus(String.valueOf(LinkStatusEnum.OFFLINE.getStatus()));
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        BeanUtils.copyProperties(protectedEnvironment, taskEnvironment);
        RestoreFeature restoreFeature = new RestoreFeature();
        // When and Then
        Assert.assertThrows("target environment[id=%s] is offline", LegoCheckedException.class,
                () -> RestoreValidator.checkEnvironmentIsOnline(taskEnvironment, restoreFeature));
    }

    /**
     * 用例名称：验证环境连接状态离线时，如果无需校验环境连接状态，则跳过校验<br/>
     * 前置条件：无<br/>
     * check点：无需校验环境连接状态，则环境离线不抛异常<br/>
     */
    @Test
    public void should_success_when_checkTargetEnvIsOnline_given_status_offline_and_should_check_online_false() {
        // Given
        final ProtectedEnvironment protectedEnvironment = ProtectedResourceMocker.mockTaskEnv();
        protectedEnvironment.setLinkStatus(String.valueOf(LinkStatusEnum.OFFLINE.getStatus()));
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        BeanUtils.copyProperties(protectedEnvironment, taskEnvironment);
        RestoreFeature restoreFeature = new RestoreFeature();
        restoreFeature.setShouldCheckEnvironmentIsOnline(false);
        // When and Then
        Assertions.assertThatNoException()
            .isThrownBy(() -> RestoreValidator.checkEnvironmentIsOnline(taskEnvironment, restoreFeature));
    }

    /**
     * 用例名称：校验副本状态为非normal时，调用校验副本接口报错<br/>
     * 前置条件：无<br/>
     * check点：1.异常与期望一致 2. 异常信息与期望一致<br/>
     */
    @Test
    public void should_throw_LegoCheckedException_when_test_checkCopyCanRestore_given_copy_status_not_normal() {
        // Given
        final Copy mockCopy = CopyMocker.mockHdfsCopy();
        mockCopy.setStatus(CopyStatus.RESTORING.getValue());
        Assert.assertThrows("Copy[id=%s] status is [%s], can not restore", LegoCheckedException.class,
                () -> RestoreValidator.checkCopyCanRestore(mockCopy, new HashMap<>()));
    }

    /**
     * 用例名称：校验副本状态为非normal时，如果副本校验状态为失效且需要强制恢复，则不进行副本状态校验<br/>
     * 前置条件：无<br/>
     * check点：副本校验失效且需强制恢复时，非normal状态副本可执行恢复<br/>
     */
    @Test
    public void should_success_when_checkCopyCanRestore_given_force_restore_true_and_copy_status_not_normal() {
        // Given
        final Copy mockCopy = CopyMocker.mockHdfsCopy();
        mockCopy.setStatus(CopyStatus.RESTORING.getValue());
        JSONObject properties = JSONObject.fromObject(mockCopy.getProperties());
        properties.put(CopyPropertiesKeyConstant.KEY_VERIFY_STATUS,
            CopyVerifyStatusEnum.VERIFY_FAILED.getVerifyStatus());
        mockCopy.setProperties(properties.toString());

        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("force_recovery", "true");
        Assertions.assertThatNoException().isThrownBy(() -> RestoreValidator.checkCopyCanRestore(mockCopy, extendInfo));
    }
}
