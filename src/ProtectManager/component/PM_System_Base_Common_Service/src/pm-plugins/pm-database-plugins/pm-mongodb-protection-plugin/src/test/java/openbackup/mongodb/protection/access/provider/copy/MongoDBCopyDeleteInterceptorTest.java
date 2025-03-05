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
package openbackup.mongodb.protection.access.provider.copy;

import static org.junit.Assert.assertNotNull;
import static org.mockito.ArgumentMatchers.*;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.when;

import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Optional;

public class MongoDBCopyDeleteInterceptorTest {
    private final CopyRestApi copyRestApi = Mockito.mock(CopyRestApi.class);

    private final ResourceService resourceService = Mockito.mock(ResourceService.class);

    private final MongoDBCopyDeleteInterceptor mongoDBCopyDeleteInterceptor = new MongoDBCopyDeleteInterceptor(
            copyRestApi, resourceService);

    /**
     * 用例场景：MongoDB单机和集群注册下发provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(mongoDBCopyDeleteInterceptor.applicable(ResourceSubTypeEnum.MONGODB_SINGLE.getType()));
        Assert.assertTrue(mongoDBCopyDeleteInterceptor.applicable(ResourceSubTypeEnum.MONGODB_CLUSTER.getType()));
    }

    /**
     * 用例场景：MongoDB单机和集群注册下发provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void should_supply_agent() {
        Assert.assertFalse(mongoDBCopyDeleteInterceptor.shouldSupplyAgent(new DeleteCopyTask(), new CopyInfoBo()));
    }

    /**
     * 用例场景：MongoDB handle_task
     * 前置条件：无
     * 检查点：NA
     */
    @Test
    public void handle_task() {
        DeleteCopyTask deleteCopyTask = new DeleteCopyTask();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("uuid");
        taskResource.setSubType(ResourceSubTypeEnum.MONGODB_CLUSTER.getType());
        deleteCopyTask.setProtectObject(taskResource);
        taskEnvironment.setExtendInfo(new HashMap<>());
        deleteCopyTask.setProtectEnv(taskEnvironment);
        given(resourceService.getBasicResourceById(any())).willReturn(Optional.of(new ProtectedResource()));
        mongoDBCopyDeleteInterceptor.handleTask(deleteCopyTask, new CopyInfoBo());
        Assert.assertTrue(true);
    }


    @Test
    public void test_getCopiesCopyTypeIsFull2_should_return_not_null_when_condition() throws Exception {
        // setup
        Copy copy = new Copy();
        copy.setUuid("string");
        copy.setGn(0);
        copy.setResourceId("string");
        Optional<Copy> optional = Optional.of(copy);
        when(copyRestApi.queryLatestFullBackupCopies(anyString(), anyInt(), anyInt())).thenReturn(optional);

        List<Copy> copies = new ArrayList<>();
        Copy copy1 = new Copy();
        copy1.setUuid("string");
        copy1.setGn(0);
        copy1.setResourceId("string");
        copies.add(copy1);

        Copy thisCopy = new Copy();
        thisCopy.setUuid("string");
        thisCopy.setGn(0);
        thisCopy.setResourceId("string");

        Copy nextFullCopy = new Copy();
        nextFullCopy.setUuid("string");
        nextFullCopy.setGn(0);
        nextFullCopy.setResourceId("string");

        // run the test
        List<String> result = mongoDBCopyDeleteInterceptor.getCopiesCopyTypeIsFull(copies, thisCopy, nextFullCopy);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_getCopiesCopyTypeIsFull2_should_throws_null_pointer_exception_when_objects_is_null()
            throws Exception {
        // setup
        when(copyRestApi.queryLatestFullBackupCopies(anyString(), anyInt(), anyInt())).thenReturn(null);

        // run the test
        List<String> result = mongoDBCopyDeleteInterceptor.getCopiesCopyTypeIsFull((List) null, (Copy) null, (Copy) null);
    }

    @Test(expected = NullPointerException.class)
    public void test_getCopiesCopyTypeIsFull2_should_throws_null_pointer_exception_when_objects_is_null1()
            throws Exception {

        // run the test
        List<String> result = mongoDBCopyDeleteInterceptor.getCopiesCopyTypeIsFull((List) null, (Copy) null, (Copy) null);
    }
}
