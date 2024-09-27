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
package openbackup.saphana.protection.access.util;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.enums.NodeType;
import openbackup.saphana.protection.access.constant.SapHanaConstants;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import org.junit.Assert;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * {@link SapHanaUtil Test}
 *
 * @author dwx1009286
 * @version [DataBackup 1.5.0]
 * @since 2023-05-16
 */
public class SapHanaUtilTest {
    /**
     * 用例场景：测试环境拓展信息是否为空
     * 前置条件：拓展信息为空
     * 检查点：是否报错
     */
    @Test
    public void should_throw_LegoCheckedException_when_environment_extend_info_param_is_empty() {
        Assert.assertThrows(LegoCheckedException.class,
            () -> SapHanaUtil.checkEnvironmentExtendInfoParam(new ProtectedEnvironment()));
    }

    /**
     * 用例场景：测试资源拓展信息是否为空
     * 前置条件：拓展信息为空
     * 检查点：是否报错
     */
    @Test
    public void should_throw_LegoCheckedException_when_resource_extend_info_param_is_empty() {
        Assert.assertThrows(LegoCheckedException.class,
            () -> SapHanaUtil.checkResourceExtendInfoParam(new ProtectedResource()));
    }

    /**
     * 用例场景：测试数据库类型是够合法
     * 前置条件：数据库类型不匹配
     * 检查点：是否报错
     */
    @Test
    public void should_throw_LegoCheckedException_when_sap_hana_database_type_is_invalid() {
        ProtectedResource resource = createResource();
        resource.setExtendInfoByKey(SapHanaConstants.SAP_HANA_DB_TYPE, "database");
        Assert.assertThrows(LegoCheckedException.class, () -> SapHanaUtil.checkDbTypeParam(resource));
    }

    /**
     * 用例场景：测试将systemId转为小写
     * 前置条件：systemId存在
     * 检查点：是否转为小写
     */
    @Test
    public void set_sap_hana_system_id_to_lower_case_success() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        Map<String, String> map = new HashMap<>();
        map.put(SapHanaConstants.SYSTEM_ID, "systemId");
        environment.setExtendInfo(map);
        SapHanaUtil.setSystemId(environment);
        Assert.assertEquals("systemid", environment.getExtendInfoByKey(SapHanaConstants.SYSTEM_ID));
    }

    /**
     * 用例场景：测试将数据库名称转为大写
     * 前置条件：名称存在
     * 检查点：是否转为大写
     */
    @Test
    public void set_sap_hana_database_name_to_upper_case_success() {
        ProtectedResource resource = createResource();
        SapHanaUtil.setDatabaseName(resource);
        Assert.assertEquals("DATABASE", resource.getName());
    }

    /**
     * 用例场景：测试将填充操作类型
     * 前置条件：无
     * 检查点：是否填充成功
     */
    @Test
    public void set_sap_hana_operation_type_extend_info_success() {
        ProtectedResource resource = createResource();
        SapHanaUtil.setOperationTypeExtendInfo(resource, SapHanaConstants.TEST_CONNECT_OPERATION_TYPE);
        Assert.assertEquals(SapHanaConstants.TEST_CONNECT_OPERATION_TYPE,
            resource.getExtendInfoByKey(SapHanaConstants.OPERATION_TYPE));
    }

    /**
     * 用例场景：测试将填充操作类型
     * 前置条件：无
     * 检查点：是否填充成功
     */
    @Test
    public void setNodeRole_success() {
        ProtectedEnvironment environment = mockHost();
        SapHanaUtil.setNodeRole(environment, NodeType.MASTER.getNodeType());
        Assert.assertEquals(NodeType.MASTER.getNodeType(), environment.getExtendInfoByKey(DatabaseConstants.ROLE));
    }

    /**
     * 用例场景：设置实例的nodes扩展信息
     * 前置条件：nodes信息不为空
     * 检查点：设置nodes扩展信息成功
     */
    @Test
    public void setInstanceEnvExtendInfoNodes_success() {
        ProtectedEnvironment instance = mockInstance(false);
        ProtectedEnvironment environment = mockHost();
        List<ProtectedEnvironment> nodes = Collections.singletonList(environment);
        SapHanaUtil.setInstanceEnvExtendInfoNodes(instance, nodes);
        Assert.assertNotNull(instance.getExtendInfoByKey(SapHanaConstants.NODES));
        Assert.assertNull(instance.getExtendInfoByKey(SapHanaConstants.OPERATION_TYPE));
    }

    /**
     * 用例场景：设置实例的nodes扩展信息
     * 前置条件：nodes信息不为空
     * 检查点：设置nodes扩展信息成功
     */
    @Test
    public void setDatabaseResourceLinkStatus_success() {
        ProtectedResource dbResource = mockTenantDbResource(false);
        SapHanaUtil.setDatabaseResourceLinkStatus(dbResource, LinkStatusEnum.ONLINE.getStatus().toString());
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(),
            dbResource.getExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY));
        Assert.assertNull(dbResource.getExtendInfoByKey(SapHanaConstants.OPERATION_TYPE));
    }

    /**
     * 用例场景：从数据库资源信息中获取nodes的值
     * 前置条件：nodes信息为空
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_ex_if_resource_nodes_info_is_null_when_getDatabaseExtendNodesInfo() {
        ProtectedResource dbResource = mockTenantDbResource(false);
        Assert.assertThrows(LegoCheckedException.class, () -> SapHanaUtil.getResourceExtendNodesInfo(dbResource));
    }

    /**
     * 用例场景：解析数据库的节点ProtectedEnvironment信息列表
     * 前置条件：nodes信息不为空
     * 检查点：解析ProtectedEnvironment信息列表成功
     */
    @Test
    public void parseHostProtectedEnvironmentList_success() {
        ProtectedResource dbResource = mockTenantDbResource(true);
        List<ProtectedEnvironment> protectedEnvList = SapHanaUtil.parseHostProtectedEnvironmentList(dbResource);
        Assert.assertEquals(1, protectedEnvList.size());
        Assert.assertEquals(ProtectedEnvironment.class, protectedEnvList.get(0).getClass());
    }

    /**
     * 用例场景：解析数据库主机资源信息列表
     * 前置条件：nodes信息不为空
     * 检查点：解析ProtectedResource列表成功
     */
    @Test
    public void parseDbHostProtectedResourceList_success() {
        ProtectedResource dbResource = mockTenantDbResource(true);
        List<ProtectedResource> protectedEnvList = SapHanaUtil.parseDbHostProtectedResourceList(dbResource);
        Assert.assertEquals(1, protectedEnvList.size());
        Assert.assertEquals(ProtectedResource.class, protectedEnvList.get(0).getClass());
    }

    /**
     * 用例场景：将ProtectedEnvironment信息列表转换为TaskEnvironment信息列表
     * 前置条件：ProtectedEnvironment信息列表不为空
     * 检查点：转换TaskEnvironment信息列表成功
     */
    @Test
    public void convertEnvListToTaskEnvList_success() {
        ProtectedEnvironment env = mockHost();
        List<TaskEnvironment> taskEnvList = SapHanaUtil.convertEnvListToTaskEnvList(Collections.singletonList(env));
        Assert.assertEquals(1, taskEnvList.size());
        Assert.assertEquals(TaskEnvironment.class, taskEnvList.get(0).getClass());
    }

    /**
     * 用例场景：将ProtectedEnvironment信息列表转换为Endpoint信息列表
     * 前置条件：ProtectedEnvironment信息列表不为空
     * 检查点：转换Endpoint信息列表成功
     */
    @Test
    public void convertEnvListToEndpointList_success() {
        ProtectedEnvironment env = mockHost();
        List<Endpoint> endpointList = SapHanaUtil.convertEnvListToEndpointList(Collections.singletonList(env));
        Assert.assertEquals(1, endpointList.size());
        Assert.assertEquals(Endpoint.class, endpointList.get(0).getClass());
    }

    /**
     * 用例场景：从操作结果中获取指定键的值
     * 前置条件：测试一：key存在；测试二：key不存在
     * 检查点：测试一：获取到正确的值；测试二：获取到空字符串
     */
    @Test
    public void getValueFromActionResultByKey_success() {
        ActionResult actionResult = new ActionResult();
        actionResult.setMessage(
            "{\"version\": \"2.00.020.00.1500920972\", \"landscapeId\": \"6ab385fc-b67a-ad4d-92c0-72aa876c511d\"}");
        String existValue = SapHanaUtil.getValueFromActionResultByKey(actionResult, "landscapeId");
        Assert.assertEquals("6ab385fc-b67a-ad4d-92c0-72aa876c511d", existValue);
        String notExistValue = SapHanaUtil.getValueFromActionResultByKey(actionResult, "notExistKey");
        Assert.assertEquals("", notExistValue);
    }

    /**
     * 用例场景：从实例资源信息中获取nodes的值
     * 前置条件：nodes信息为空
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_ex_if_instance_nodes_info_is_null_when_getInstanceExtendNodesInfo() {
        ProtectedEnvironment instance = mockInstance(false);
        Assert.assertThrows(LegoCheckedException.class, () -> SapHanaUtil.getInstanceExtendNodesInfo(instance));
    }

    /**
     * 用例场景：解析实例的主机环境信息列表
     * 前置条件：实例nodes信息不为空
     * 检查点：解析主机环境信息列表成功
     */
    @Test
    public void parseInstanceHostEnvList_success() {
        ProtectedEnvironment instance = mockInstance(true);
        List<ProtectedEnvironment> hostEnvList = SapHanaUtil.parseInstanceHostEnvList(instance);
        Assert.assertEquals(1, hostEnvList.size());
        Assert.assertEquals(ProtectedEnvironment.class, hostEnvList.get(0).getClass());
    }

    /**
     * 用例场景：解析实例的主机资源信息列表
     * 前置条件：实例nodes信息不为空
     * 检查点：解析主机环境资源列表成功
     */
    @Test
    public void parseInstanceHostResourceList_success() {
        ProtectedEnvironment instance = mockInstance(true);
        List<ProtectedResource> hostResList = SapHanaUtil.parseInstanceHostResourceList(instance);
        Assert.assertEquals(1, hostResList.size());
        Assert.assertEquals(ProtectedResource.class, hostResList.get(0).getClass());
    }

    /**
     * 用例场景：检查否为系统数据库
     * 前置条件：参数为系统数据库
     * 检查点：是否返回true
     */
    @Test
    public void assert_database_type_is_system_database_success() {
        ProtectedResource resource = createResource();
        SapHanaUtil.isSystemDatabase(resource);
        Assert.assertTrue(SapHanaUtil.isSystemDatabase(resource));
    }

    /**
     * 用例场景：测试数据库参数是够为空
     * 前置条件：数据库参数为空
     * 检查点：是否报错
     */
    @Test
    public void should_throw_LegoCheckedException_when_extend_info_database_param_is_empty() {
        ProtectedResource resource = createResource();
        resource.setExtendInfoByKey(SapHanaConstants.SAP_HANA_DB_TYPE, null);
        Assert.assertThrows(LegoCheckedException.class, () -> SapHanaUtil.isSystemDatabase(resource));
    }

    /**
     * 用例场景：测试数据库类型是够合法
     * 前置条件：数据库类型不匹配
     * 检查点：是否报错
     */
    @Test
    public void should_throw_LegoCheckedException_when_extend_info_database_param_is_invalid() {
        ProtectedResource resource = createResource();
        resource.setExtendInfoByKey(SapHanaConstants.SAP_HANA_DB_TYPE, "database");
        Assert.assertThrows(LegoCheckedException.class, () -> SapHanaUtil.isSystemDatabase(resource));
    }

    /**
     * 用例场景：获取数据库部署类型
     * 前置条件：数据库只有一个节点
     * 检查点：无论系统数据库还是租户数据库都返回单机部署类型
     */
    @Test
    public void should_return_single_deploy_type_if_db_has_one_host_when_getDeployType() {
        Assert.assertEquals(DatabaseDeployTypeEnum.SINGLE.getType(),
            SapHanaUtil.getDeployType(1, SapHanaConstants.SYSTEM_DB_TYPE));
        Assert.assertEquals(DatabaseDeployTypeEnum.SINGLE.getType(),
            SapHanaUtil.getDeployType(1, SapHanaConstants.TENANT_DB_TYPE));
    }

    /**
     * 用例场景：获取数据库部署类型
     * 前置条件：数据库只有两个节点
     * 检查点：系统数据库返回主备部署类型
     */
    @Test
    public void should_return_ap_deploy_type_if_db_has_two_hosts_when_getDeployType() {
        Assert.assertEquals(DatabaseDeployTypeEnum.AP.getType(),
            SapHanaUtil.getDeployType(2, SapHanaConstants.SYSTEM_DB_TYPE));
    }

    /**
     * 用例场景：获取数据库部署类型
     * 前置条件：数据库有两个节点
     * 检查点：租户数据库返回分布式部署类型
     */
    @Test
    public void should_return_distributed_deploy_type_if_db_has_two_hosts_when_getDeployType() {
        Assert.assertEquals(DatabaseDeployTypeEnum.DISTRIBUTED.getType(),
            SapHanaUtil.getDeployType(2, SapHanaConstants.TENANT_DB_TYPE));
    }

    /**
     * 用例场景：从通用数据库SAP HANA数据库的自定义参数中获取system id
     * 前置条件：自定义参数不合法
     * 检查点：返回空字符串
     */
    @Test
    public void should_return_empty_str_if_custom_params_is_invalid_when_getSystemIdFromCustomParams() {
        String customParams = null;
        Assert.assertEquals("", SapHanaUtil.getSystemIdFromCustomParams(customParams));
        String customParams2 = "";
        Assert.assertEquals("", SapHanaUtil.getSystemIdFromCustomParams(customParams2));
        String customParams3 = ",abc=";
        Assert.assertEquals("", SapHanaUtil.getSystemIdFromCustomParams(customParams3));
    }

    /**
     * 用例场景：从通用数据库SAP HANA数据库的自定义参数中获取system id
     * 前置条件：自定义参数合法
     * 检查点：获取到正确的system id
     */
    @Test
    public void should_return_system_id_if_exist_sap_hana_db_in_general_databases_when_getSystemIdFromCustomParams() {
        String customParams = "systemDbPort=30113,systemId=s00";
        Assert.assertEquals("s00", SapHanaUtil.getSystemIdFromCustomParams(customParams));
    }

    private ProtectedResource createResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setName("database");
        resource.setSubType(ResourceTypeEnum.DATABASE.getType());
        resource.setSubType(ResourceSubTypeEnum.SAPHANA_DATABASE.getType());
        Map<String, String> map = new HashMap<>();
        map.put(SapHanaConstants.SAP_HANA_DB_TYPE, "SystemDatabase");
        resource.setExtendInfo(map);
        return resource;
    }

    private ProtectedResource mockTenantDbResource(boolean includeNodes) {
        ProtectedResource resource = new ProtectedResource();
        resource.setName("tenant-1");
        resource.setSubType(ResourceTypeEnum.DATABASE.getType());
        resource.setSubType(ResourceSubTypeEnum.SAPHANA_DATABASE.getType());
        resource.setExtendInfoByKey(SapHanaConstants.SAP_HANA_DB_TYPE, SapHanaConstants.TENANT_DB_TYPE);
        resource.setExtendInfoByKey(SapHanaConstants.OPERATION_TYPE, SapHanaConstants.TEST_CONNECT_OPERATION_TYPE);
        if (includeNodes) {
            List<ProtectedEnvironment> nodes = new ArrayList<>();
            ProtectedEnvironment node1 = mockHost();
            nodes.add(node1);
            resource.setExtendInfoByKey(SapHanaConstants.NODES, JSONObject.stringify(nodes));
        }
        return resource;
    }

    private ProtectedEnvironment mockHost() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setName("node-11");
        environment.setUuid("0ed8b119-7d23-475d-9ad3-4fa8e353ed0c");
        environment.setEndpoint("10.10.10.11");
        environment.setPort(22);
        return environment;
    }

    private ProtectedEnvironment mockInstance(boolean includeNodes) {
        ProtectedEnvironment instance = new ProtectedEnvironment();
        instance.setName("hana-inst1");
        instance.setUuid("50658ac36d4f46a0a95f915c7002a77e");
        instance.setSubType(ResourceTypeEnum.DATABASE.getType());
        instance.setSubType(ResourceSubTypeEnum.SAPHANA_INSTANCE.getType());
        instance.setExtendInfoByKey(SapHanaConstants.OPERATION_TYPE, SapHanaConstants.TEST_CONNECT_OPERATION_TYPE);
        if (includeNodes) {
            List<ProtectedEnvironment> nodes = new ArrayList<>();
            ProtectedEnvironment node1 = mockHost();
            nodes.add(node1);
            instance.setExtendInfoByKey(SapHanaConstants.NODES, JSONObject.stringify(nodes));
        }
        return instance;
    }
}
