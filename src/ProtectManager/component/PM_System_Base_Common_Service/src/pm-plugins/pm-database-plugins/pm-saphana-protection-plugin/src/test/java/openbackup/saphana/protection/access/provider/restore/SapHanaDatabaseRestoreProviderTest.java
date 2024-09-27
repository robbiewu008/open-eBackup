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
package openbackup.saphana.protection.access.provider.restore;

import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.RestoreFeature;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.saphana.protection.access.constant.SapHanaConstants;
import openbackup.saphana.protection.access.constant.SapHanaErrorCode;
import openbackup.saphana.protection.access.service.SapHanaResourceService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.ArgumentMatchers;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.test.util.ReflectionTestUtils;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * {@link SapHanaDatabaseRestoreProvider Test}
 *
 * @author wWX1013713
 * @version [DataBackup 1.5.0]
 * @since 2023-05-24
 */
public class SapHanaDatabaseRestoreProviderTest {
    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);

    private final SapHanaResourceService hanaResourceService = PowerMockito.mock(SapHanaResourceService.class);

    private final SapHanaDatabaseRestoreProvider provider = new SapHanaDatabaseRestoreProvider(copyRestApi,
        hanaResourceService);

    /**
     * 用例场景：根据subType检测Provider
     * 前置条件：subType为SAPHANA-database
     * 检查点：返回true
     */
    @Test
    public void applicable_sap_hana_database_success() {
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.SAPHANA_DATABASE.getType()));
    }

    /**
     * 用例场景：获取锁定资源对象
     * 前置条件：task包含目标资源
     * 检查点：返回锁资源对象
     */
    @Test
    public void getLockResources_success() {
        TaskResource targetObject = new TaskResource();
        targetObject.setUuid("dec9aa85698d4af3b31acafea1027236");
        RestoreTask task = new RestoreTask();
        task.setTargetObject(targetObject);
        List<LockResourceBo> lockResourceBoList = provider.getLockResources(task);
        Assert.assertEquals(1, lockResourceBoList.size());
    }

    /**
     * 用例场景：拦截恢复请求，对恢复请求进行拦截
     * 前置条件：请求参数正确
     * 检查点：返回恢复任务信息
     */
    @Test
    public void intercept_success() {
        PowerMockito.when(hanaResourceService.getResourceById("dec9aa85698d4af3b31acafea1027236"))
            .thenReturn(mockClusterTenantDbResource());
        PowerMockito.when(copyRestApi.queryCopyByID("f69e3f29-30d4-400d-9af0-5158c0aa427f"))
            .thenReturn(mockBackupCopy());
        PowerMockito.when(hanaResourceService.getResourceById("5ded83e5f6364678a26f08bd8cccd2a9"))
            .thenReturn(mockClusterInstResource());
        List<ProtectedEnvironment> envList = mockProtectedEnvironmentList();
        PowerMockito.doReturn(envList).when(hanaResourceService).queryEnvironments(ArgumentMatchers.anyList());
        RestoreTask oriTask = mockRestoreTask();
        RestoreTask lastTask = provider.initialize(oriTask);
        // 恢复模式是本地副本直接恢复
        Assert.assertEquals(RestoreModeEnum.LOCAL_RESTORE.getMode(), lastTask.getRestoreMode());
        // 集群租户数据库是分布式
        Assert.assertEquals(DatabaseDeployTypeEnum.DISTRIBUTED.getType(),
            lastTask.getTargetEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE));
        // 检查恢复目标位置
        Assert.assertEquals("cluster-inst1/tenant1", lastTask.getTargetObject().getTargetLocation());
    }

    /**
     * 用例场景：检查源实例版本和目标实例版本是否匹配
     * 前置条件：目标版本为空
     * 检查点：抛出LegoCheckedException异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_ver_is_empty_when_checkVersionForRestore() {
        String srcVersion = "2.00.020.00.1500920972";
        String tgtVersion = "";
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> ReflectionTestUtils.invokeMethod(SapHanaDatabaseRestoreProvider.class, "checkVersionForRestore",
                srcVersion, tgtVersion));
        Assert.assertEquals(SapHanaErrorCode.VERSION_DISMATCH, exception.getErrorCode());
    }

    /**
     * 用例场景：检查源实例版本和目标实例版本是否匹配
     * 前置条件：源版本为空
     * 检查点：抛出LegoCheckedException异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_ver_is_invalid_when_checkVersionForRestore() {
        String srcVersion = "";
        String tgtVersion = "2.00.020.00.1500920972";
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> ReflectionTestUtils.invokeMethod(SapHanaDatabaseRestoreProvider.class, "checkVersionForRestore",
                srcVersion, tgtVersion));
        Assert.assertEquals(SapHanaErrorCode.VERSION_DISMATCH, exception.getErrorCode());
    }

    /**
     * 用例场景：检查源实例版本和目标实例版本是否匹配
     * 前置条件：高版本到低版本不允许恢复
     * 检查点：抛出LegoCheckedException异常
     */
    @Test
    public void should_return_false_if_version_not_match_when_checkVersionForRestore() {
        String srcVersion = "2.00.020.00.1500920972";
        String tgtVersion = "1.00.020.00.1500920972";
        LegoCheckedException firException = Assert.assertThrows(LegoCheckedException.class,
            () -> ReflectionTestUtils.invokeMethod(SapHanaDatabaseRestoreProvider.class, "checkVersionForRestore",
                srcVersion, tgtVersion));
        Assert.assertEquals(SapHanaErrorCode.VERSION_DISMATCH, firException.getErrorCode());
    }

    /**
     * 用例场景：检查源实例版本和目标实例版本是否匹配
     * 前置条件：源实例版本和目标实例版本匹配
     * 检查点：返回void，不抛异常
     */
    @Test
    public void should_return_true_if_version_match_when_checkVersionForRestore() {
        // 同版本
        String srcVersion = "2.00.020.00.1500920972";
        String tgtVersion = "2.01";
        Assert.assertNull(ReflectionTestUtils.invokeMethod(SapHanaDatabaseRestoreProvider.class, "checkVersionForRestore",
            srcVersion, tgtVersion));
        // 低版本到高版本
        String srcVersion2 = "1.00.020.00.1500920972";
        String tgtVersion2 = "2.00.020.00.1500920972";
        Assert.assertNull(ReflectionTestUtils.invokeMethod(SapHanaDatabaseRestoreProvider.class, "checkVersionForRestore",
            srcVersion2, tgtVersion2));
    }

    private RestoreTask mockRestoreTask() {
        TaskResource targetObject = new TaskResource();
        targetObject.setUuid("dec9aa85698d4af3b31acafea1027236");
        targetObject.setParentUuid("5ded83e5f6364678a26f08bd8cccd2a9");
        RestoreTask oriTask = new RestoreTask();
        oriTask.setTaskId("3baeae46-ef69-4ed6-8e99-3756369bd413");
        oriTask.setTargetObject(targetObject);
        oriTask.setCopyId("f69e3f29-30d4-400d-9af0-5158c0aa427f");
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        oriTask.setTargetEnv(taskEnvironment);
        oriTask.setTargetLocation(RestoreLocationEnum.ORIGINAL);
        return oriTask;
    }

    private Copy mockBackupCopy() {
        Copy copy = new Copy();
        copy.setUuid("f69e3f29-30d4-400d-9af0-5158c0aa427f");
        copy.setResourceId("dec9aa85698d4af3b31acafea1027236");
        HashMap<String, String> resourcePropertyMap = new HashMap<>();
        resourcePropertyMap.put(DatabaseConstants.VERSION, "2.0");
        HashMap<String, String> resourceExtInfo = new HashMap<>();
        resourceExtInfo.put(SapHanaConstants.SYSTEM_ID, "a00");
        resourceExtInfo.put(SapHanaConstants.SAP_HANA_DB_TYPE, SapHanaConstants.TENANT_DB_TYPE);
        List<ProtectedEnvironment> envList = mockProtectedEnvironmentList();
        resourceExtInfo.put(SapHanaConstants.NODES, JSONObject.stringify(envList));
        resourcePropertyMap.put(DatabaseConstants.EXTEND_INFO, JSONObject.stringify(resourceExtInfo));
        copy.setResourceProperties(JSONObject.stringify(resourcePropertyMap));
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        copy.setBackupType(1);
        return copy;
    }

    private ProtectedResource mockClusterTenantDbResource() {
        ProtectedResource dbReqResource = new ProtectedResource();
        dbReqResource.setUuid("dec9aa85698d4af3b31acafea1027236");
        dbReqResource.setSourceType(ResourceTypeEnum.DATABASE.getType());
        dbReqResource.setSubType(ResourceSubTypeEnum.SAPHANA_DATABASE.getType());
        dbReqResource.setName("tenant1");
        dbReqResource.setParentUuid("5ded83e5f6364678a26f08bd8cccd2a9");
        dbReqResource.setVersion("2.0");
        List<ProtectedEnvironment> envList = mockProtectedEnvironmentList();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(SapHanaConstants.SAP_HANA_DB_TYPE, SapHanaConstants.TENANT_DB_TYPE);
        extendInfo.put(SapHanaConstants.SYSTEM_ID, "a00");
        extendInfo.put(SapHanaConstants.NODES, JSONObject.stringify(envList));
        dbReqResource.setExtendInfo(extendInfo);
        return dbReqResource;
    }

    private List<ProtectedEnvironment> mockProtectedEnvironmentList() {
        ProtectedEnvironment firstEnv = new ProtectedEnvironment();
        firstEnv.setUuid("0ed8b119-7d23-475d-9ad3-4fa8e353ed0b");
        firstEnv.setEndpoint("10.10.10.11");
        firstEnv.setPort(22);
        ProtectedEnvironment secondEnv = new ProtectedEnvironment();
        secondEnv.setUuid("cfedb495-6574-41e3-843e-e1cb2fc7afd3");
        secondEnv.setEndpoint("10.10.10.12");
        secondEnv.setPort(22);
        return Arrays.asList(firstEnv, secondEnv);
    }

    private ProtectedResource mockClusterInstResource() {
        ProtectedResource instResource = new ProtectedResource();
        instResource.setUuid("5ded83e5f6364678a26f08bd8cccd2a9");
        instResource.setSourceType(ResourceTypeEnum.DATABASE.getType());
        instResource.setSubType(ResourceSubTypeEnum.SAPHANA_INSTANCE.getType());
        instResource.setName("cluster-inst1");
        instResource.setVersion("2.0");
        List<ProtectedEnvironment> envList = mockProtectedEnvironmentList();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(SapHanaConstants.SYSTEM_ID, "a00");
        extendInfo.put(SapHanaConstants.NODES, JSONObject.stringify(envList));
        instResource.setExtendInfo(extendInfo);
        return instResource;
    }

    /**
     * 用例场景：恢复特性开关
     * 前置条件：SAP HANA数据库恢复不检查实例状态
     * 检查点: 是否检查Environment在线为false
     */
    @Test
    public void should_not_check_instance_online_if_db_when_getRestoreFeature() {
        RestoreFeature feature = provider.getRestoreFeature();
        Assert.assertFalse(feature.isShouldCheckEnvironmentIsOnline());
    }
}
