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
package openbackup.data.access.framework.restore.converter;

import openbackup.data.access.framework.core.common.enums.v2.filter.ResourceFilter;
import openbackup.data.access.framework.protection.mocks.CreateRestoreTaskRequestMocker;
import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResourceFilter;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import org.junit.Assert;
import org.junit.Test;

import static org.assertj.core.api.BDDAssertions.then;

/**
 * 功能描述
 *
 * @description:
 **/
public class RestoreTaskConverterTest {
    /**
     * 用例名称：验证对象转换器在没传过滤器的场景下转转换成功<br/>
     * 前置条件：无<br/>
     * check点：对象前后转换的属性一致<br/>
     */
    @Test
    public void should_success_when_covert_to_restore_task_given_no_filters_request() {
        // Given
        final CreateRestoreTaskRequest mockRequest = CreateRestoreTaskRequestMocker.mockSuccessRequest();
        // When
        final RestoreTask restoreTask = RestoreTaskConverter.convertToRestoreTask(mockRequest);
        // Then
        Assert.assertEquals(restoreTask.getCopyId(), mockRequest.getCopyId());
        Assert.assertEquals(restoreTask.getRestoreType(), mockRequest.getRestoreType().getType());
        Assert.assertEquals(restoreTask.getTargetLocation(), mockRequest.getTargetLocation());
    }

    /**
     * 用例名称：验证对象转换器在传入过滤器的场景下转转换成功<br/>
     * 前置条件：无<br/>
     * check点：对象前后转换的属性一致<br/>
     */
    @Test
    public void should_success_when_covert_to_restore_task_given_filters_request() {
        // Given
        final CreateRestoreTaskRequest mockRequest = CreateRestoreTaskRequestMocker.mockSuccessRequest();
        CreateRestoreTaskRequestMocker.addFilters(mockRequest);
        // When
        final RestoreTask restoreTask = RestoreTaskConverter.convertToRestoreTask(mockRequest);
        // Then
        then(restoreTask.getCopyId()).isEqualTo(mockRequest.getCopyId());
        then(restoreTask.getRestoreType()).isEqualTo(mockRequest.getRestoreType().getType());
        then(restoreTask.getTargetLocation()).isEqualTo(mockRequest.getTargetLocation());
        then(restoreTask.getSubObjects()).isNullOrEmpty();
        then(restoreTask.getFilters()).isNotEmpty().hasSize(mockRequest.getFilters().size());
        final ResourceFilter requestFilter = mockRequest.getFilters().get(0);
        final TaskResourceFilter filter = restoreTask.getFilters().get(0);
        then(filter.getFilterBy()).isEqualTo(requestFilter.getFilterBy().getCondition());
        then(filter.getType()).isEqualTo(requestFilter.getType().getType());
        then(filter.getMode()).isEqualTo(requestFilter.getMode().getMode());
        then(filter.getRule()).isEqualTo(requestFilter.getRule().getRule());
        then(filter.getValues()).isSameAs(requestFilter.getValues());
        Assert.assertEquals(filter.getValues(), requestFilter.getValues());
        Assert.assertEquals(filter.getRule(), requestFilter.getRule().getRule());
        Assert.assertEquals(filter.getMode(), requestFilter.getMode().getMode());
    }

    /**
     * 用例名称：验证对象转换器在传入资源子对象的场景下转转换成功<br/>
     * 前置条件：无<br/>
     * check点：对象前后转换的属性一致<br/>
     */
    @Test
    public void should_success_when_covert_to_restore_task_given_sub_objects_request() {
        // Given
        final CreateRestoreTaskRequest mockRequest = CreateRestoreTaskRequestMocker.mockSuccessRequest();
        CreateRestoreTaskRequestMocker.addSubObjects(mockRequest);
        // When
        final RestoreTask restoreTask = RestoreTaskConverter.convertToRestoreTask(mockRequest);
        // Then
        then(restoreTask.getCopyId()).isEqualTo(mockRequest.getCopyId());
        then(restoreTask.getRestoreType()).isEqualTo(mockRequest.getRestoreType().getType());
        then(restoreTask.getTargetLocation()).isEqualTo(mockRequest.getTargetLocation());
        then(restoreTask.getFilters()).isNullOrEmpty();
        then(restoreTask.getSubObjects())
                .isNotEmpty()
                .hasSize(3)
                .allMatch(item -> mockRequest.getSubObjects().contains(item.getName()));
        Assert.assertEquals(restoreTask.getCopyId(), mockRequest.getCopyId());
        Assert.assertEquals(restoreTask.getRestoreType(), mockRequest.getRestoreType().getType());
        Assert.assertEquals(restoreTask.getTargetLocation(), mockRequest.getTargetLocation());
    }
}
