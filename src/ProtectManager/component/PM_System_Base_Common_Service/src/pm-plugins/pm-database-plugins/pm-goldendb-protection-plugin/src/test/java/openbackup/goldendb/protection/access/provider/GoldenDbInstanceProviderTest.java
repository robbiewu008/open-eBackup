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
package openbackup.goldendb.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.when;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.goldendb.protection.access.constant.GoldenDbConstant;
import openbackup.goldendb.protection.access.dto.cluster.Node;
import openbackup.goldendb.protection.access.dto.instance.GoldenInstance;
import openbackup.goldendb.protection.access.dto.instance.Gtm;
import openbackup.goldendb.protection.access.dto.instance.MysqlNode;
import openbackup.goldendb.protection.access.service.GoldenDbService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.assertj.core.util.Lists;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;
import org.powermock.api.mockito.PowerMockito;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Arrays;
import java.util.Collections;

@RunWith(MockitoJUnitRunner.class)
public class GoldenDbInstanceProviderTest {

    @Mock
    private GoldenDbService mockGoldenDbService;

    private GoldenDbInstanceProvider goldenDbInstanceProviderUnderTest;

    @Before
    public void setUp() {
        goldenDbInstanceProviderUnderTest = new GoldenDbInstanceProvider(mockGoldenDbService);
    }

    @Test
    public void testApplicable() {
        // Setup
        final ProtectedResource object = new ProtectedResource();
        object.setName("name");
        object.setSubType(ResourceSubTypeEnum.GOLDENDB_CLUSETER_INSTANCE.getType());

        // Run the test
        boolean result = goldenDbInstanceProviderUnderTest.applicable(object);

        // Verify the results
        Assert.assertTrue(result);
    }

    @Test
    public void testBeforeCreate() {
        when(mockGoldenDbService.singleConnectCheck(any(), any())).thenReturn(true);

        // Configure GoldenDbService.getGtmNode(...).

        String clusterInfo = getEnvironment().getExtendInfo().get("clusterInfo");
        GoldenInstance instance = JsonUtil.read(clusterInfo, GoldenInstance.class);

        when(mockGoldenDbService.getGtmNode(any())).thenReturn(instance.getGtm());

        ProtectedEnvironment environment = getEnvironment();
        environment.setEndpoint("8.8.8.8");
        when(mockGoldenDbService.getEnvironmentById(any())).thenReturn(environment);

        // Run the test
        goldenDbInstanceProviderUnderTest.beforeCreate(getEnvironment());
        goldenDbInstanceProviderUnderTest.beforeUpdate(getEnvironment());
        Assert.assertTrue(true);
    }

    @Test
    public void testBeforeCreateException() {
        // Configure GoldenDbService.getGtmNode(...).
        String clusterInfo = getEnvironment().getExtendInfo().get("clusterInfo");
        GoldenInstance instance = JsonUtil.read(clusterInfo, GoldenInstance.class);
        ProtectedEnvironment environment = getEnvironment();
        environment.setEndpoint("8.8.8.8");
        MysqlNode mysqlNode = new MysqlNode();
        mysqlNode.setParentUuid("666");
        mysqlNode.setIp("8.40.147.52");
        Node node = new Node();
        node.setNodeType("managerNode");
        node.setOsUser("root");
        when(mockGoldenDbService.getComputeNode(any())).thenReturn(Collections.singletonList(mysqlNode));
        environment.getExtendInfo().put("agentIpList", "6.6.6.6");

        // Run the test
        try {
            goldenDbInstanceProviderUnderTest.beforeCreate(getEnvironment());
            Assert.fail();
        } catch (LegoCheckedException exception) {
            Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, exception.getErrorCode());
        }
    }

    @Test
    public void testManagerNodeCheckException() throws NoSuchMethodException, IllegalAccessException {
        GoldenDbInstanceProvider goldenDbInstanceProvider = new GoldenDbInstanceProvider(mockGoldenDbService);
        Method managerNodeCheck = goldenDbInstanceProvider.getClass()
            .getDeclaredMethod("managerNodeCheck", ProtectedEnvironment.class);
        managerNodeCheck.setAccessible(true);
        PowerMockito.when(mockGoldenDbService.getEnvironmentById(any())).thenReturn(getEnvironment());
        PowerMockito.when(mockGoldenDbService.getManageDbNode(any())).thenReturn(Arrays.asList(new Node()));
        PowerMockito.when(mockGoldenDbService.singleConnectCheck(any(), any())).thenReturn(false);
        try {
            managerNodeCheck.invoke(goldenDbInstanceProvider, getEnvironment());
            Assert.fail();
        } catch (InvocationTargetException exception) {
            Assert.assertTrue(exception.getTargetException() instanceof LegoCheckedException);
            LegoCheckedException legoCheckedException = (LegoCheckedException) exception.getTargetException();
            Assert.assertEquals(legoCheckedException.getErrorCode(), CommonErrorCode.CLUSTER_NODES_QUERY_FAILED);
        }
    }

    @Test
    public void testComputeNodeCheckException() throws NoSuchMethodException, IllegalAccessException {
        GoldenDbInstanceProvider goldenDbInstanceProvider = new GoldenDbInstanceProvider(mockGoldenDbService);
        Method computeNodeCheck = goldenDbInstanceProvider.getClass()
            .getDeclaredMethod("computeNodeCheck", ProtectedEnvironment.class);
        computeNodeCheck.setAccessible(true);
        PowerMockito.when(mockGoldenDbService.singleConnectCheck(any(), any())).thenReturn(false);
        try {
            computeNodeCheck.invoke(goldenDbInstanceProvider, getEnvironment());
            Assert.fail();
        } catch (InvocationTargetException exception) {
            Assert.assertTrue(exception.getTargetException() instanceof LegoCheckedException);
            LegoCheckedException legoCheckedException = (LegoCheckedException) exception.getTargetException();
            Assert.assertEquals(legoCheckedException.getErrorCode(), CommonErrorCode.CLUSTER_NODES_QUERY_FAILED);
        }
    }

    @Test
    public void testGtmNodeCheckException() throws NoSuchMethodException, IllegalAccessException {
        GoldenDbInstanceProvider goldenDbInstanceProvider = new GoldenDbInstanceProvider(mockGoldenDbService);
        Method gtmNodeCheck = goldenDbInstanceProvider.getClass()
            .getDeclaredMethod("gtmNodeCheck", ProtectedEnvironment.class);
        gtmNodeCheck.setAccessible(true);
        PowerMockito.when(mockGoldenDbService.getGtmNode(any())).thenReturn(Arrays.asList(new Gtm()));
        PowerMockito.when(mockGoldenDbService.singleConnectCheck(any(), any())).thenReturn(false);
        try {
            gtmNodeCheck.invoke(goldenDbInstanceProvider, getEnvironment());
            Assert.fail();
        } catch (InvocationTargetException exception) {
            Assert.assertTrue(exception.getTargetException() instanceof LegoCheckedException);
            LegoCheckedException legoCheckedException = (LegoCheckedException) exception.getTargetException();
            Assert.assertEquals(legoCheckedException.getErrorCode(), CommonErrorCode.CLUSTER_NODES_QUERY_FAILED);
        }
    }

    @Test
    public void testGtmNodeMatchException() throws NoSuchMethodException, IllegalAccessException {
        GoldenDbInstanceProvider goldenDbInstanceProvider = new GoldenDbInstanceProvider(mockGoldenDbService);
        Method checkGtmNodeMatch = goldenDbInstanceProvider.getClass()
            .getDeclaredMethod("checkGtmNodeMatch", ProtectedResource.class);
        checkGtmNodeMatch.setAccessible(true);
        Gtm gtm = new Gtm();
        gtm.setGtmIp("6.6.6.6");
        PowerMockito.when(mockGoldenDbService.getGtmNode(any())).thenReturn(Lists.newArrayList(gtm));
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setExtendInfoByKey(GoldenDbConstant.AGENT_IP_LIST, "8.8.8.8");
        PowerMockito.when(mockGoldenDbService.getEnvironmentById(any())).thenReturn(agent);
        try {
            checkGtmNodeMatch.invoke(goldenDbInstanceProvider, getEnvironment());
            Assert.fail();
        } catch (InvocationTargetException exception) {
            Assert.assertTrue(exception.getTargetException() instanceof LegoCheckedException);
            LegoCheckedException legoCheckedException = (LegoCheckedException) exception.getTargetException();
            Assert.assertEquals(legoCheckedException.getErrorCode(), CommonErrorCode.CLUSTER_NODES_INCONSISTENT);
        }
    }

    @Test
    public void testComputeNodeMatchException() throws NoSuchMethodException, IllegalAccessException {
        GoldenDbInstanceProvider goldenDbInstanceProvider = new GoldenDbInstanceProvider(mockGoldenDbService);
        Method checkNodeMatch = goldenDbInstanceProvider.getClass()
            .getDeclaredMethod("checkNodeMatch", ProtectedResource.class);
        checkNodeMatch.setAccessible(true);
        MysqlNode mysqlNode = new MysqlNode();
        mysqlNode.setIp("6.6.6.6");
        PowerMockito.when(mockGoldenDbService.getComputeNode(any())).thenReturn(Arrays.asList(mysqlNode));
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setExtendInfoByKey(GoldenDbConstant.AGENT_IP_LIST, "8.8.8.8");
        PowerMockito.when(mockGoldenDbService.getEnvironmentById(any())).thenReturn(agent);
        try {
            checkNodeMatch.invoke(goldenDbInstanceProvider, getEnvironment());
            Assert.fail();
        } catch (InvocationTargetException exception) {
            Assert.assertTrue(exception.getTargetException() instanceof LegoCheckedException);
            LegoCheckedException legoCheckedException = (LegoCheckedException) exception.getTargetException();
            Assert.assertEquals(legoCheckedException.getErrorCode(), CommonErrorCode.AGENT_MISMATCH_NODE);
        }
    }

    @Test
    public void testComputeNodeNotNull() throws NoSuchMethodException, IllegalAccessException {
        GoldenDbInstanceProvider goldenDbInstanceProvider = new GoldenDbInstanceProvider(mockGoldenDbService);
        Method checkComputeNodeName = goldenDbInstanceProvider.getClass()
            .getDeclaredMethod("checkComputeNodeName", MysqlNode.class);
        Method checkComputeNodeIp = goldenDbInstanceProvider.getClass()
            .getDeclaredMethod("checkComputeNodeIp", MysqlNode.class);
        Method checkComputeNodePartParent = goldenDbInstanceProvider.getClass()
            .getDeclaredMethod("checkComputeNodePartParent", MysqlNode.class);

        checkComputeNodeName.setAccessible(true);
        checkComputeNodeIp.setAccessible(true);
        checkComputeNodePartParent.setAccessible(true);

        MysqlNode mysqlNode = new MysqlNode();
        mysqlNode.setId("1");
        mysqlNode.setName("test");
        try {
            checkComputeNodeName.invoke(goldenDbInstanceProvider, mysqlNode);
            Assert.fail();
        } catch (InvocationTargetException exception) {
            Assert.assertTrue(exception.getTargetException() instanceof LegoCheckedException);
            LegoCheckedException legoCheckedException = (LegoCheckedException) exception.getTargetException();
            Assert.assertEquals(legoCheckedException.getErrorCode(), CommonErrorCode.ILLEGAL_PARAM);
        }
        mysqlNode.setIp("8.8.8.8");
        mysqlNode.setPort("66");
        try {
            checkComputeNodeIp.invoke(goldenDbInstanceProvider, mysqlNode);
            Assert.fail();
        } catch (InvocationTargetException exception) {
            Assert.assertTrue(exception.getTargetException() instanceof LegoCheckedException);
            LegoCheckedException legoCheckedException = (LegoCheckedException) exception.getTargetException();
            Assert.assertEquals(legoCheckedException.getErrorCode(), CommonErrorCode.ILLEGAL_PARAM);
        }
        mysqlNode.setGroup("1");
        mysqlNode.setParentUuid("123");
        try {
            checkComputeNodePartParent.invoke(goldenDbInstanceProvider, mysqlNode);
            Assert.fail();
        } catch (InvocationTargetException exception) {
            Assert.assertTrue(exception.getTargetException() instanceof LegoCheckedException);
            LegoCheckedException legoCheckedException = (LegoCheckedException) exception.getTargetException();
            Assert.assertEquals(legoCheckedException.getErrorCode(), CommonErrorCode.ILLEGAL_PARAM);
        }
    }

    @Test
    public void testGtmNodeNotNull() throws NoSuchMethodException, IllegalAccessException {
        GoldenDbInstanceProvider goldenDbInstanceProvider = new GoldenDbInstanceProvider(mockGoldenDbService);
        Method checkGtmNodeType = goldenDbInstanceProvider.getClass().getDeclaredMethod("checkGtmNodeType", Gtm.class);
        Method checkGtmNodeIp = goldenDbInstanceProvider.getClass().getDeclaredMethod("checkGtmNodeIp", Gtm.class);
        Method checkGtmNodeFlag = goldenDbInstanceProvider.getClass().getDeclaredMethod("checkGtmNodeFlag", Gtm.class);
        checkGtmNodeType.setAccessible(true);
        checkGtmNodeIp.setAccessible(true);
        checkGtmNodeFlag.setAccessible(true);
        Gtm gtm = new Gtm();
        gtm.setNodeType("Gtm");
        gtm.setOsUser("123");
        try {
            checkGtmNodeType.invoke(goldenDbInstanceProvider, gtm);
            Assert.fail();
        } catch (InvocationTargetException exception) {
            Assert.assertTrue(exception.getTargetException() instanceof LegoCheckedException);
            LegoCheckedException legoCheckedException = (LegoCheckedException) exception.getTargetException();
            Assert.assertEquals(legoCheckedException.getErrorCode(), CommonErrorCode.ILLEGAL_PARAM);
        }
        gtm.setGtmId("6");
        gtm.setGtmIp("8.8.8.8");
        try {
            checkGtmNodeType.invoke(goldenDbInstanceProvider, gtm);
            Assert.fail();
        } catch (InvocationTargetException exception) {
            Assert.assertTrue(exception.getTargetException() instanceof LegoCheckedException);
            LegoCheckedException legoCheckedException = (LegoCheckedException) exception.getTargetException();
            Assert.assertEquals(legoCheckedException.getErrorCode(), CommonErrorCode.ILLEGAL_PARAM);
        }
        try {
            checkGtmNodeType.invoke(goldenDbInstanceProvider, gtm);
            Assert.fail();
        } catch (InvocationTargetException exception) {
            Assert.assertTrue(exception.getTargetException() instanceof LegoCheckedException);
            LegoCheckedException legoCheckedException = (LegoCheckedException) exception.getTargetException();
            Assert.assertEquals(legoCheckedException.getErrorCode(), CommonErrorCode.ILLEGAL_PARAM);
        }
    }

    /**
     * 将ResourceFeature对象的isSupportedLanFree属性置为false表示不支持lanfree并返回
     *
     * @return 资源是否支持lanfree
     */
    @Test
    public void getResourceFeature() {
        Assert.assertFalse(goldenDbInstanceProviderUnderTest.getResourceFeature().isSupportedLanFree());
    }

    private ProtectedEnvironment getEnvironment() {
        String json
            = "{\"uuid\":\"eb46ed20eeff45499afbec5e739220a1\",\"name\":\"def\",\"type\":\"Database\",\"subType\":\"GoldenDB-clusterInstance\",\"path\":\"192.168.138.42,192.168.138.44,192.168.138.46\",\"createdTime\":\"2023-04-06 17:51:47.807\",\"parentUuid\":\"7e5b99c1-71c5-3c84-9214-77867dd11e47\",\"rootUuid\":\"7e5b99c1-71c5-3c84-9214-77867dd11e47\",\"sourceType\":\"register\",\"protectionStatus\":0,\"extendInfo\":{\"clusterInfo\":\"{\\\"id\\\":\\\"1\\\",\\\"name\\\":\\\"cluster1\\\",\\\"group\\\":[{\\\"groupId\\\":\\\"1\\\",\\\"databaseNum\\\":\\\"2\\\",\\\"mysqlNodes\\\":[{\\\"id\\\":\\\"1\\\",\\\"name\\\":\\\"DN1\\\",\\\"role\\\":\\\"master\\\",\\\"ip\\\":\\\"8.46.138.44\\\",\\\"port\\\":\\\"5501\\\",\\\"linkStatus\\\":\\\"1\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"group\\\":\\\"DBGroup1\\\",\\\"parentUuid\\\":\\\"9292c22a-fbad-4c54-a69b-ac20401a4a5c\\\",\\\"parentName\\\":\\\"localhost.localdomain(192.168.138.44)\\\",\\\"osUser\\\":\\\"zxdb1\\\",\\\"parent\\\":null},{\\\"id\\\":\\\"2\\\",\\\"name\\\":\\\"DN2\\\",\\\"role\\\":\\\"slave\\\",\\\"ip\\\":\\\"8.46.138.46\\\",\\\"port\\\":\\\"5501\\\",\\\"linkStatus\\\":\\\"1\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"group\\\":\\\"DBGroup1\\\",\\\"parentUuid\\\":\\\"2cd27f74-da99-43b3-b972-faff996dd487\\\",\\\"parentName\\\":\\\"localhost.localdomain(192.168.138.46)\\\",\\\"osUser\\\":\\\"zxdb1\\\",\\\"parent\\\":null}]},{\\\"groupId\\\":\\\"2\\\",\\\"databaseNum\\\":\\\"2\\\",\\\"mysqlNodes\\\":[{\\\"id\\\":\\\"3\\\",\\\"name\\\":\\\"DN3\\\",\\\"role\\\":\\\"master\\\",\\\"ip\\\":\\\"8.46.138.46\\\",\\\"port\\\":\\\"5502\\\",\\\"linkStatus\\\":\\\"1\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"group\\\":\\\"DBGroup2\\\",\\\"parentUuid\\\":\\\"2cd27f74-da99-43b3-b972-faff996dd487\\\",\\\"parentName\\\":\\\"localhost.localdomain(192.168.138.46)\\\",\\\"osUser\\\":\\\"zxdb2\\\",\\\"parent\\\":null},{\\\"id\\\":\\\"4\\\",\\\"name\\\":\\\"DN4\\\",\\\"role\\\":\\\"slave\\\",\\\"ip\\\":\\\"8.46.138.44\\\",\\\"port\\\":\\\"5502\\\",\\\"linkStatus\\\":\\\"1\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"group\\\":\\\"DBGroup2\\\",\\\"parentUuid\\\":\\\"9292c22a-fbad-4c54-a69b-ac20401a4a5c\\\",\\\"parentName\\\":\\\"localhost.localdomain(192.168.138.44)\\\",\\\"osUser\\\":\\\"zxdb2\\\",\\\"parent\\\":null}]}],\\\"gtm\\\":[{\\\"gtmId\\\":\\\"1\\\",\\\"gtmIp\\\":\\\"8.46.138.44\\\",\\\"port\\\":\\\"6026\\\",\\\"masterFlag\\\":\\\"1\\\",\\\"parentUuid\\\":\\\"9292c22a-fbad-4c54-a69b-ac20401a4a5c\\\",\\\"parentName\\\":\\\"localhost.localdomain(192.168.138.44)\\\",\\\"nodeType\\\":\\\"gtmNode\\\",\\\"osUser\\\":\\\"zxgtm1\\\",\\\"parent\\\":null}]}\",\"local_ini_cnf\":\"W2NvbW1vbl0KI1Jvb3QgZGlyIG9mIGJhY2t1cGluZzsKI3VuaXQ6IE5BLCByYW5nZTogTkEsIGRlZmF1bHQ6ICRIT01FL2JhY2t1cF9yb290CmJhY2t1cF9yb290ZGlyID0K\",\"agentIpList\":\"8.46.138.44\"},\"userId\":\"62a76fceaefd4bbc99f5adf0686f6181\",\"authorizedUser\":\"baohu\",\"auth\":{\"authType\":2,\"authKey\":\"super\",\"authPwd\":\"Huawei@123\",\"extendInfo\":{}},\"environment\":{\"uuid\":\"7e5b99c1-71c5-3c84-9214-77867dd11e47\",\"name\":\"集群42_1\",\"type\":\"Database\",\"subType\":\"GoldenDB-cluster\",\"createdTime\":\"2023-03-31 16:44:01.403\",\"rootUuid\":\"7e5b99c1-71c5-3c84-9214-77867dd11e47\",\"sourceType\":\"register\",\"version\":\"V6.1\",\"protectionStatus\":0,\"extendInfo\":{\"GoldenDB\":\"{\\\"nodes\\\":[{\\\"parentUuid\\\":\\\"80d9ae7f-e87f-4485-9234-a13ee47bf450\\\",\\\"parentName\\\":\\\"goldendb41(192.168.138.42)\\\",\\\"osUser\\\":\\\"zxmanager\\\",\\\"nodeType\\\":\\\"managerNode\\\"}]}\"},\"userId\":\"62a76fceaefd4bbc99f5adf0686f6181\",\"authorizedUser\":\"baohu\",\"endpoint\":\"192.168.138.42\",\"port\":0,\"linkStatus\":\"1\",\"scanInterval\":3600,\"cluster\":false},\"dependencies\":{\"agents\":[{\"uuid\":\"9292c22a-fbad-4c54-a69b-ac20401a4a5c\"},{\"uuid\":\"2cd27f74-da99-43b3-b972-faff996dd487\"}],\"-agents\":[]}}";
        ProtectedEnvironment read = JsonUtil.read(json, ProtectedEnvironment.class);
        return read;
    }
}
