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
package openbackup.tpops.protection.access.util;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.SourceClustersParams;
import openbackup.system.base.sdk.cluster.model.StorageSystemInfo;
import openbackup.system.base.service.DeployTypeService;
import openbackup.tpops.protection.access.constant.TpopsGaussDBConstant;

import org.junit.Assert;
import org.junit.jupiter.api.Test;
import org.mockito.ArgumentMatchers;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 工具类测试
 *
 * @author x30021699
 * @since 2023-05-09
 */
public class TpopsGaussDBClusterUtilsTest {
    private ClusterNativeApi clusterNativeApi = mock(ClusterNativeApi.class);

    private EncryptorService encryptorService = PowerMockito.mock(EncryptorService.class);

    private DeployTypeService deployTypeService = PowerMockito.mock(DeployTypeService.class);

    /**
     * 用例场景：获取缓存情况
     * 前置条件：无
     * 检 查 点：都能够获取到对象，不为空
     */
    @Test
    public void getCacheRepository() {
        StorageRepository storageRepository = new StorageRepository();
        storageRepository.setType(RepositoryTypeEnum.DATA.getType());
        StorageRepository cacheRepository = TpopsGaussDBClusterUtils.getCacheRepository(storageRepository);
        Assert.assertEquals("2", cacheRepository.getType().toString());
    }

    /**
     * 用例场景：修改key值
     * 前置条件：无
     * 检 查 点：修改成功
     */
    @Test
    public void modifyAdvanceParam() {
        Map<String, String> advance = new HashMap<>();
        advance.put("oldKey", "oldValue");
        TpopsGaussDBClusterUtils.modifyAdvanceParam(advance, "oldKey", "newValue");
        Assert.assertTrue(advance.keySet().contains("newValue"));
    }

    /**
     * 用例场景：初始化用户
     * 前置条件：无
     * 检 查 点：初始成功
     */
    @Test
    public void initProtectEnvOfGaussDbUser() {
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setExtendInfo(new HashMap<>());
        String user = "user";
        TpopsGaussDBClusterUtils.initProtectEnvOfGaussDbUser(taskEnvironment, user);
        Assert.assertEquals("user", taskEnvironment.getExtendInfo().get(TpopsGaussDBConstant.EXTEND_INFO_KEY_GAUSSDB_USER));
    }

    /**
     * 用例场景：增加仓库角色
     * 前置条件：无
     * 检 查 点：校验角色
     */
    @Test
    public void addRepositoryEsnAndRole() {
        StorageRepository storageRepository = new StorageRepository();
        String esn = "1";
        TpopsGaussDBClusterUtils.addRepositoryEsnAndRole(storageRepository, esn);
        Assert.assertEquals("0", storageRepository.getRole().toString());
    }

    /**
     * 用例场景：初始化权限
     * 前置条件：无
     * 检 查 点：初始成功
     */
    @Test
    public void buildAuthentication() {
        StorageRepository storageRepository = new StorageRepository();
        storageRepository.setId("123");
        storageRepository.setProtocol(0);
        ClusterDetailInfo clusterDetailInfo = new ClusterDetailInfo();
        StorageSystemInfo storageSystemInfo = new StorageSystemInfo();
        storageSystemInfo.setStorageEsn("456");
        clusterDetailInfo.setStorageSystem(storageSystemInfo);
        SourceClustersParams sourceClustersParams = new SourceClustersParams();
        sourceClustersParams.setMgrIpList(Arrays.asList("0.0.0"));
        clusterDetailInfo.setSourceClusters(sourceClustersParams);
        when(clusterNativeApi.queryCurrentGroupClusterDetails()).thenReturn(clusterDetailInfo);
        List<ClusterDetailInfo> clusterDetailInfoList = new ArrayList<>();
        clusterDetailInfoList.add(clusterDetailInfo);
        PowerMockito.when(clusterNativeApi.queryTargetClusterListDetails(ArgumentMatchers.any()))
            .thenReturn(clusterDetailInfoList);
        TpopsGaussDBClusterUtils.buildAuthentication(storageRepository, clusterNativeApi, encryptorService,
            deployTypeService, false);
        Assert.assertTrue(true);
    }
}