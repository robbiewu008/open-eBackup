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
package openbackup.openstack.adapter.service;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyList;
import static org.mockito.ArgumentMatchers.anyMap;

import openbackup.openstack.adapter.constants.OpenStackErrorCodes;
import openbackup.openstack.adapter.dto.OpenStackCopyDto;
import openbackup.openstack.adapter.exception.OpenStackException;
import openbackup.openstack.adapter.service.OpenStackCopyManager;
import openbackup.openstack.adapter.service.OpenStackUserManager;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import feign.FeignException;

import org.assertj.core.api.Assertions;
import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.Arrays;
import java.util.Date;
import java.util.List;

/**
 * {@link OpenStackCopyManager} 测试类
 *
 */
public class OpenStackCopyManagerTest {
    private final CopyRestApi copyRestApi = Mockito.mock(CopyRestApi.class);
    private final OpenStackUserManager userManager = Mockito.mock(OpenStackUserManager.class);
    private final OpenStackCopyManager manager = new OpenStackCopyManager(copyRestApi, userManager);

    /**
     * 用例场景：调用副本删除接口正常返回
     * 前置条件：protection服务正常
     * 检查点： 副本删除接口正常调用
     */
    @Test
    public void should_callCopyRestApiDeleteCopyOneTime_when_deleteCopy() {
        String copyId = UUIDGenerator.getUUID();
        String userId = UUIDGenerator.getUUID();
        Mockito.when(userManager.obtainUserId()).thenReturn(userId);
        Mockito.doNothing()
            .when(copyRestApi)
            .deleteCopy(copyId, userId, false, false, JobTypeEnum.COPY_DELETE.getValue());
        Assertions.assertThatNoException().isThrownBy(() -> manager.deleteCopy(copyId));
    }

    /**
     * 用例场景：调用远程接口失败时，抛出异常
     * 前置条件：protection服务异常
     * 检查点： protection服务异常，抛出指定异常
     */
    @Test
    public void should_throwLegoCheckedException_when_deleteCopy_given_feignException() {
        String copyId = UUIDGenerator.getUUID();
        String userId = UUIDGenerator.getUUID();
        Mockito.when(userManager.obtainUserId()).thenReturn(userId);
        Mockito.doThrow(FeignException.class)
            .when(copyRestApi)
            .deleteCopy(copyId, userId, false, false, JobTypeEnum.COPY_DELETE.getValue());
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> manager.deleteCopy(copyId));
        assertThat(exception.getErrorCode()).isEqualTo(CommonErrorCode.SYSTEM_ERROR);
        assertThat(exception.getMessage()).isEqualTo(String.format("Delete copy: %s fail.", copyId));
    }

    /**
     * 用例场景：查询所有副本时，成功返回两个副本
     * 前置条件：protection服务异常
     * 检查点： 副本属性设置正确
     */
    @Test
    public void should_returnTwoCopies_when_queryCopies_given_resourceIdHasTwoCopies() {
        String resourceId = UUIDGenerator.getUUID();

        BasePage<Copy> response = new BasePage<>();
        Copy firstCopy = createCopy("1", resourceId);
        Copy secondCopy = createCopy("2", resourceId);
        response.setItems(Arrays.asList(firstCopy, secondCopy));
        response.setTotal(2);

        Mockito.when(copyRestApi.queryCopies(anyInt(), anyInt(), anyMap(), anyList())).thenReturn(response);
        List<OpenStackCopyDto> copies = manager.queryCopies(resourceId);
        assertThat(copies).hasSize(2);
        assertThat(copies).element(0)
            .hasFieldOrPropertyWithValue("id", firstCopy.getUuid())
            .hasFieldOrPropertyWithValue("backupJobId", firstCopy.getResourceId())
            .hasFieldOrPropertyWithValue("size", 1)
            .hasFieldOrPropertyWithValue("generateTime", firstCopy.getGeneratedTime())
            .hasFieldOrPropertyWithValue("generateId", 1)
            .hasFieldOrPropertyWithValue("isLatest", false);
        assertThat(copies).element(1)
            .hasFieldOrPropertyWithValue("id", secondCopy.getUuid())
            .hasFieldOrPropertyWithValue("backupJobId", secondCopy.getResourceId())
            .hasFieldOrPropertyWithValue("size", 1)
            .hasFieldOrPropertyWithValue("generateTime", secondCopy.getGeneratedTime())
            .hasFieldOrPropertyWithValue("generateId", 2)
            .hasFieldOrPropertyWithValue("isLatest", true);
    }

    /**
     * 用例场景：查询单个副本时，正确返回
     * 前置条件：protection服务异常
     * 检查点： 副本属性设置正确
     */
    @Test
    public void should_querySuccess_when_queryCopy() {
        String resourceId = UUIDGenerator.getUUID();

        BasePage<Copy> response = new BasePage<>();
        Copy firstCopy = createCopy("1", resourceId);
        Copy secondCopy = createCopy("2", resourceId);
        response.setItems(Arrays.asList(firstCopy, secondCopy));
        response.setTotal(2);

        Mockito.when(copyRestApi.queryCopies(anyInt(), anyInt(), anyMap(), anyList())).thenReturn(response);

        Mockito.when(copyRestApi.queryCopyByID("2", false)).thenReturn(secondCopy);

        OpenStackCopyDto copy = manager.queryCopy("2");
        assertThat(copy)
            .hasFieldOrPropertyWithValue("id", secondCopy.getUuid())
            .hasFieldOrPropertyWithValue("backupJobId", secondCopy.getResourceId())
            .hasFieldOrPropertyWithValue("size", 1)
            .hasFieldOrPropertyWithValue("generateTime", secondCopy.getGeneratedTime())
            .hasFieldOrPropertyWithValue("generateId", 2)
            .hasFieldOrPropertyWithValue("isLatest", true);
    }

    /**
     * 用例场景：查询指定副本时，如果副本不存在，抛出异常
     * 前置条件：副本不存在
     * 检查点： 副本不存在应抛出异常
     */
    @Test
    public void should_throwOpenStackException_when_queryCopy_given_nonExistCopyId() {
        String copyId = UUIDGenerator.getUUID();
        Mockito.when(copyRestApi.queryCopyByID(copyId, false)).thenReturn(null);

        OpenStackException exception = Assert.assertThrows(OpenStackException.class,
            () -> manager.queryCopy(copyId));
        assertThat(exception.getErrorCode()).isEqualTo(OpenStackErrorCodes.NOT_FOUND);
        assertThat(exception.getMessage()).isEqualTo(String.format("Copy: %s is not exists.", copyId));
    }

    /**
     * 用例场景：调用远程接口失败时，抛出异常
     * 前置条件：protection服务异常
     * 检查点： protection服务异常，抛出指定异常
     */
    @Test
    public void should_throwLegoCheckedException_when_queryCopy_given_feignException() {
        String copyId = UUIDGenerator.getUUID();
        Mockito.doThrow(FeignException.class).when(copyRestApi).queryCopyByID(copyId, false);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> manager.queryCopy(copyId));
        assertThat(exception.getErrorCode()).isEqualTo(CommonErrorCode.SYSTEM_ERROR);
        assertThat(exception.getMessage()).isEqualTo(String.format("Query copy: %s fail.", copyId));
    }

    private Copy createCopy(String id, String resourceId) {
        Copy copy = new Copy();
        copy.setUuid(id);
        copy.setTimestamp(String.valueOf(System.nanoTime()));
        copy.setGeneratedTime(new Date().toString());
        copy.setResourceId(resourceId);
        return copy;
    }
}
