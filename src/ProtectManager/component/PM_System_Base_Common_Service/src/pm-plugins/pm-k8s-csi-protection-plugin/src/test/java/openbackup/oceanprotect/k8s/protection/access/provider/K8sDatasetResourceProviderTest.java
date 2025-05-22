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
package openbackup.oceanprotect.k8s.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.oceanprotect.k8s.protection.access.constant.K8sConstant;
import openbackup.oceanprotect.k8s.protection.access.provider.K8sDatasetResourceProvider;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import java.io.File;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

import static org.mockito.ArgumentMatchers.any;

/**
 * 功能描述: K8sDatasetResourceProviderTest
 *
 */
public class K8sDatasetResourceProviderTest {
    private final ResourceService resourceService = Mockito.mock(ResourceService.class);
    private K8sDatasetResourceProvider k8sDatasetResourceProvider;

    @Before
    public void init() {
        k8sDatasetResourceProvider = new K8sDatasetResourceProvider(resourceService);
    }


    @Test
    public void test_applicable_success() {
        ProtectedResource object = new ProtectedResource();
        object.setSubType(ResourceSubTypeEnum.KUBERNETES_DATASET_COMMON.getType());
        Assert.assertTrue(k8sDatasetResourceProvider.applicable(object));
    }

    /**
     * 用例场景：注册流程检查K8SDataset
     * 前置条件：无
     * 检查点: 正确配置name和parentUuid，无异常抛出
     */
    @Test
    public void test_check_success() {
        ProtectedResource resource = mockk8sDataset();
        resource.setParentUuid("parentUuid");
        resource.setName("name");
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("parentUuid", resource.getParentUuid());
        PageListResponse<ProtectedResource> listResponse = new PageListResponse<>();
        listResponse.setTotalCount(1);
        Mockito.when(resourceService.basicQuery(false, 0,
                K8sConstant.DATASET_MAX_COUNT_IN_NAMESPACE, conditions)).thenReturn(listResponse);
        ProtectedResource ns = new ProtectedResource();
        ns.setPath("NamespacePath");
        Mockito.when(resourceService.getBasicResourceById(any())).thenReturn(Optional.of(ns));
        k8sDatasetResourceProvider.beforeCreate(resource);
        Mockito.verify(resourceService, Mockito.times(1)).basicQuery(false, 0,
                K8sConstant.DATASET_MAX_COUNT_IN_NAMESPACE, conditions);
        Assert.assertEquals("NamespacePath" + File.separator + resource.getName() , resource.getPath());
    }

    /**
     * 用例场景：注册流程检查K8SDataset
     * 前置条件：无
     * 检查点: 不配置name和parentUuid，按顺序抛出对应异常，得到正确异常信息
     */
    @Test
    public void test_check_parameter_error() {
        ProtectedResource resource = mockk8sDataset();
        resource.setName (null);
        resource.setParentUuid (null);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> k8sDatasetResourceProvider.beforeCreate(resource));
        Assert.assertEquals("K8s Dataset name is illegal.", exception.getMessage());
        resource.setName("test");
        exception = Assert.assertThrows(LegoCheckedException.class,
                () -> k8sDatasetResourceProvider.beforeCreate(resource));
        Assert.assertEquals("K8s Dataset parentUuid is empty.", exception.getMessage());
    }

    /**
     * 用例场景：更新流程检查K8SDataset
     * 前置条件：无
     * 检查点: 不配置name，抛出对应异常，得到正确异常信息
     */
    @Test
    public void test_update_check_parameter_error() {
        ProtectedResource resource = mockk8sDataset();
        resource.setName(null);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> k8sDatasetResourceProvider.beforeUpdate(resource));
        Assert.assertEquals("K8s Dataset name is illegal.", exception.getMessage());
    }

    /**
     * 用例场景：注册流程检查Dataset最大数目
     * 前置条件：无
     * 检查点: 模拟得到超出限制的totalcount，抛出对应异常
     */
    @Test
    public void test_check_K8s_dataset_count_error() {
        ProtectedResource resource = mockk8sDataset();
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("parentUuid", resource.getParentUuid());
        PageListResponse<ProtectedResource> protectedResourcePageListResponse = new PageListResponse<>();
        protectedResourcePageListResponse.setTotalCount(K8sConstant.DATASET_MAX_COUNT_IN_NAMESPACE);
        Mockito.when(resourceService.basicQuery(false, 0, K8sConstant.DATASET_MAX_COUNT_IN_NAMESPACE,
                conditions)).thenReturn(protectedResourcePageListResponse);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> k8sDatasetResourceProvider.beforeCreate(resource));
        // 验证异常内的信息是否正确
        Assert.assertEquals(CommonErrorCode.RESOURCE_NUM_EXCEED_LIMIT, exception.getErrorCode());
        Assert.assertEquals(1, exception.getParameters().length);
        Assert.assertEquals(String.valueOf(K8sConstant.DATASET_MAX_COUNT_IN_NAMESPACE), exception.getParameters()[0]);
        Assert.assertEquals("The number of K8s exceeds the maximum.", exception.getMessage());
    }
    private ProtectedResource mockk8sDataset() {
        ProtectedResource k8sDataset = new ProtectedResource();
        k8sDataset.setParentUuid("parentUuid");
        k8sDataset.setName("text");
        k8sDataset.setExtendInfo(new HashMap<>());
        return k8sDataset;
    }
}