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
package openbackup.gaussdbdws.protection.access.util;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 功能描述: 校验 DWS各种参数的基本格式等信息的测试类
 *
 */
public class DwsValidatorTest {
    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    /**
     * 用例场景：校验dws cluster超过规格
     * 前置条件：集群数量大于8个
     * 检查点：失败
     */
    @Test
    public void check_check_dws_count_fail() {
        expectedException.expect(LegoCheckedException.class);
        List<ProtectedEnvironment> existingEnvironments = new ArrayList<>();
        for (int i = 0; i < 8; i++) {
            existingEnvironments.add(new ProtectedEnvironment());
        }
        DwsValidator.checkDwsCount(existingEnvironments);
    }

    /**
     * 用例场景：校验dws cluster 是否存在相同的uuid
     * 前置条件：存在相同的uuid
     * 检查点：失败
     */
    @Test
    public void check_dws_resource_exist_id_fail() {
        expectedException.expect(LegoCheckedException.class);
        List<ProtectedEnvironment> existingEnvironments = new ArrayList<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("123456789");
        existingEnvironments.add(protectedEnvironment);
        DwsValidator.checkDwsResourceExistById(existingEnvironments, "123456789");
    }

    /**
     * 用例场景：校验dws cluster 是否存在相同的endpoint
     * 前置条件：存在相同的endpoint
     * 检查点：失败
     */
    @Test
    public void should_throw_LegoCheckedException_if_exist_the_same_endpoint_when_check_Dws_resource() {
        List<ProtectedEnvironment> existingEnvironments = new ArrayList<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setEndpoint("123456789");
        existingEnvironments.add(protectedEnvironment);
        Assert.assertThrows(LegoCheckedException.class,
            () -> DwsValidator.checkDwsResourceExistByEndpoint(existingEnvironments, "123456789"));
    }

    /**
     * 用例场景：校验 dws 集群的 rootUuid 是否存在
     * 前置条件：rootUuid不存在
     * 检查点：失败
     */
    @Test
    public void check_dws_root_uuid_fail() {
        expectedException.expect(LegoCheckedException.class);
        DwsValidator.checkDwsValue(null);
    }

    /**
     * 用例场景：校验 dws 集群的 名称 是否存在
     * 前置条件：名称不存在
     * 检查点：失败
     */
    @Test
    public void check_dws_name_format_fail() {
        expectedException.expect(LegoCheckedException.class);
        DwsValidator.checkDwsNameFormat(null);
    }

    /**
     * 用例场景：校验 dws 集群的 名称 是否存在
     * 前置条件：名称不存在
     * 检查点：失败
     */
    @Test
    public void check_dws_exist_same_database_name_fail() {
        expectedException.expect(LegoCheckedException.class);
        String s = "cailingling/schema1,cailingling/schema2,postgres/schema_test";
        DwsValidator.checkDwsExistSameDatabaseName(s.split(","));
    }

    /**
     * 用例场景：校验 dws 集群的 tables的数量 是否大于size
     * 前置条件：tables和 size存在且tables.size >size
     * 检查点：失败
     */
    @Test
    public void check_dws_tables_size_fail() {
        expectedException.expect(LegoCheckedException.class);
        String[] tablesInfo = new String[5];
        DwsValidator.checkDwsSize(tablesInfo, 3);
    }

    /**
     * 用例场景：ProtectedEnvironment是否存在并在线
     * 前置条件：ProtectedEnvironment 不在线
     * 检查点：失败
     */
    @Test
    public void check_environment_fail() {
        expectedException.expect(LegoCheckedException.class);
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(new ArrayList<>());
        DwsValidator.checkEnvironment(pageListResponse);
    }

    /**
     * 用例场景：校验 是否存在相同的表
     * 前置条件：存在相同的表
     * 检查点：失败
     */
    @Test
    public void check_same_table_fail() {
        expectedException.expect(LegoCheckedException.class);
        String[] tablesInfo = new String[1];
        tablesInfo[0] = "database1/schema1/table2";
        DwsValidator.checkSameTable(Arrays.asList(
            "database1/schema1/table1,database1/schema1/table2,database1/schema1/table3".split(",")), tablesInfo,
            ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType());
    }

    /**
     * 用例场景：校验 dws 集群的 是否存在相同agent的集群uuid或者主机uuid
     * 前置条件：uuid都不相同
     * 检查点：成功
     */
    @Test
    public void check_dws_exist_same_cluster_or_host_success() {
        List<ProtectedEnvironment> existingEnvironments = new ArrayList<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> protectedResources = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("277277522");
        protectedResources.add(protectedResource);
        dependencies.put(DwsConstant.DWS_CLUSTER_AGENT, protectedResources);
        dependencies.put(DwsConstant.HOST_AGENT, protectedResources);
        protectedEnvironment.setDependencies(dependencies);
        existingEnvironments.add(protectedEnvironment);
        List<String> uuidList = new ArrayList<>();
        uuidList.add("1154154");
        DwsValidator.checkDwsExistSameClusterOrHost(existingEnvironments, uuidList);
    }

    /**
     * 用例场景：校验 dws 集群中Agent uuid 是否存在相同
     * 前置条件：存在相同uuid
     * 检查点：失败
     */
    @Test
    public void should_throw_LegoCheckedException_if_uuid_exist_same_when_check_exist_uuid() {
        List<String> uuidList = new ArrayList<>();
        uuidList.add("1154154");
        Assert.assertThrows("The dws cluster id: 1154154 already exist.", LegoCheckedException.class,
            () -> DwsValidator.checkExistUuid(uuidList, "1154154"));
    }

    /**
     * 用例场景：校验存在相同uuid情况下name不相同校验
     * 前置条件：存在相同uuid name不同
     * 检查点：失败
     */
    @Test
    public void should_throw_LegoCheckedException_i_exist_different_same_when_check_the_same_name() {
        Assert.assertThrows("dws_24-25-26_intrusive and ws_24-25-26_intrusiv1 is not equal name",
            LegoCheckedException.class,
            () -> DwsValidator.checkTheSameName("dws_24-25-26_intrusive", "dws_24-25-26_intrusiv1"));
    }
}
