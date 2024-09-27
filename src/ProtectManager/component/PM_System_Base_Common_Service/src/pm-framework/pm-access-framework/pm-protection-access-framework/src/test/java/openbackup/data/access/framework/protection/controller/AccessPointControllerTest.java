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
package openbackup.data.access.framework.protection.controller;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.core.common.enums.CopyIndexStatus;
import openbackup.data.access.framework.protection.controller.AccessPointController;
import openbackup.data.access.framework.protection.dto.CopyReplicationMetadata;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.CopyReplicationImport;
import openbackup.data.protection.access.provider.sdk.copy.ReplicationOriginCopyDuration;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.functionswitch.enums.FunctionTypeEnum;
import com.huawei.oceanprotect.sla.sdk.enums.ReplicationMode;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.enums.RetentionTypeEnum;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.auth.AuthRestApi;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.protection.constants.SlaConstant;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.reflect.Whitebox;
import org.springframework.test.util.ReflectionTestUtils;

import java.util.Date;

/**
 * {@link AccessPointController} 测试类
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-12
 */
public class AccessPointControllerTest {
    AccessPointController controller = Mockito.mock(AccessPointController.class);

    private AuthRestApi authRestApi = PowerMockito.mock(AuthRestApi.class);

    private DmeUnifiedRestApi dmeUnifiedRestApi = PowerMockito.mock(DmeUnifiedRestApi.class);

    private ResourceService resourceService =PowerMockito.mock(ResourceService.class);

    /**
     * 用例名称：reverse_copy为true，则副本生成方式为“反向复制”<br/>
     * 前置条件：无<br/>
     * check点：副本生成方式返回正确<br/>
     */
    @Test
    public void test_should_return_reverse_replication_when_build_copy_info() {
        Whitebox.setInternalState(controller, "authRestApi", authRestApi);
        Whitebox.setInternalState(controller, "dmeUnifiedRestApi", dmeUnifiedRestApi);
        Whitebox.setInternalState(controller, "resourceService", resourceService);
        JSONObject properties = new JSONObject();
        properties.put("reverse_copy", true);
        properties.put("chain_id", "1");
        CopyReplicationImport copyReplicationImport = new CopyReplicationImport();
        copyReplicationImport.setProperties(properties);
        copyReplicationImport.setMetadata(
                "{\"resource\":{},\"username\":\"test\",\"replication_policy\":{\"ext_parameters\":{},\"retention\":{}}}");
        TokenBo.UserInfo userInfo = new TokenBo.UserInfo();
        userInfo.setId("123");
        PowerMockito.when(authRestApi.queryUserInfoByName(any())).thenReturn(userInfo);
        Object copyInfoBo =
            ReflectionTestUtils.invokeMethod(controller, "createCopyInfoBo", copyReplicationImport, new Date(), true,
                "");
        assertThat(copyInfoBo).isNotNull();
        assertThat(copyInfoBo).isExactlyInstanceOf(CopyInfoBo.class);
        assertThat(((CopyInfoBo) copyInfoBo).getGeneratedBy()).isEqualTo(
            CopyGeneratedByEnum.BY_REVERSE_REPLICATION.value());
    }

    /**
     * 用例名称：replicate_count大于1，则副本生成方式为“级联复制”<br/>
     * 前置条件：无<br/>
     * check点：副本生成方式返回正确<br/>
     */
    @Test
    public void test_should_return_cascaded_replication_when_build_copy_info() {
        Whitebox.setInternalState(controller, "authRestApi", authRestApi);
        Whitebox.setInternalState(controller, "dmeUnifiedRestApi", dmeUnifiedRestApi);
        Whitebox.setInternalState(controller, "resourceService", resourceService);
        CopyReplicationImport copyReplicationImport = new CopyReplicationImport();
        JSONObject properties = new JSONObject();
        properties.put("replicate_count", 2);
        properties.put("chain_id", "1");
        copyReplicationImport.setProperties(properties);
        copyReplicationImport.setMetadata(
            "{\"resource\":{},\"username\":\"test\",\"replication_policy\":{\"ext_parameters\":{},\"retention\":{}}}");
        TokenBo.UserInfo userInfo = new TokenBo.UserInfo();
        userInfo.setId("123");
        PowerMockito.when(authRestApi.queryUserInfoByName(any())).thenReturn(userInfo);

        Object copyInfoBo =
            ReflectionTestUtils.invokeMethod(controller, "createCopyInfoBo", copyReplicationImport, new Date(), true,
                "");
        assertThat(copyInfoBo).isNotNull();
        assertThat(copyInfoBo).isExactlyInstanceOf(CopyInfoBo.class);
        assertThat(((CopyInfoBo) copyInfoBo).getGeneratedBy()).isEqualTo(
            CopyGeneratedByEnum.BY_CASCADED_REPLICATION.value());
    }

    /**
     * 用例名称：replicate_count小于或等于1，则副本生成方式为“复制”<br/>
     * 前置条件：无<br/>
     * check点：副本生成方式返回正确<br/>
     */
    @Test
    public void test_should_return_replication_when_build_copy_info() {
        Whitebox.setInternalState(controller, "authRestApi", authRestApi);
        Whitebox.setInternalState(controller, "dmeUnifiedRestApi", dmeUnifiedRestApi);
        Whitebox.setInternalState(controller, "resourceService", resourceService);
        CopyReplicationImport copyReplicationImport = new CopyReplicationImport();
        JSONObject properties = new JSONObject();
        properties.put("replicate_count", 1);
        properties.put("chain_id", "1");
        copyReplicationImport.setProperties(properties);
        copyReplicationImport.setMetadata(
                "{\"resource\":{},\"username\":\"test\",\"replication_policy\":{\"ext_parameters\":{},\"retention\":{}}}");
        TokenBo.UserInfo userInfo = new TokenBo.UserInfo();
        userInfo.setId("123");
        PowerMockito.when(authRestApi.queryUserInfoByName(any())).thenReturn(userInfo);
        Object copyInfoBo =
            ReflectionTestUtils.invokeMethod(controller, "createCopyInfoBo", copyReplicationImport, new Date(), true,
                "");
        assertThat(copyInfoBo).isNotNull();
        assertThat(copyInfoBo).isExactlyInstanceOf(CopyInfoBo.class);
        assertThat(((CopyInfoBo) copyInfoBo).getGeneratedBy()).isEqualTo(CopyGeneratedByEnum.BY_REPLICATED.value());
    }

    /**
     * 用例名称：主端副本为worm格式，从端副本保留时间取主端副本和复制策略保留时间较大值
     * 前置条件：主端副本为worm格式，永久保留，复制策略为临时保留
     * check点：副本保留时间取较长，为永久保留
     */
    @Test
    public void test_buildRetentionInfo_origin_is_permanent() throws JsonProcessingException {
        CopyInfoBo copyInfoBo = new CopyInfoBo();
        copyInfoBo.setResourceId("123");
        copyInfoBo.setUuid("123");
        JSONObject properties = new JSONObject();
        properties.put("format", CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
        copyInfoBo.setProperties(properties.toString());
        CopyReplicationImport copyReplicationImport = getCopyReplicationImport();
        copyReplicationImport.setTimestamp(1687502348);
        ReflectionTestUtils.invokeMethod(controller, "buildCopyRetentionInfo", copyInfoBo,
            new Date(System.currentTimeMillis()), true, copyReplicationImport);
        assertThat(copyInfoBo).isNotNull();
        assertThat(copyInfoBo).isExactlyInstanceOf(CopyInfoBo.class);
        assertThat(copyInfoBo.getRetentionType()).isEqualTo(RetentionTypeEnum.PERMANENT.getType());
    }

    /**
     * 用例名称：主端副本为worm格式，从端副本保留时间取主端副本和复制策略保留时间较大值
     * 前置条件：主端副本为worm格式，主从副本策略都为临时保留,主端副本保留时间较长
     * check点：副本保留时间取较长
     */
    @Test
    public void test_buildRetentionInfo_origin_is_tempraty() throws JsonProcessingException {
        CopyInfoBo copyInfoBo = new CopyInfoBo();
        copyInfoBo.setResourceId("123");
        copyInfoBo.setUuid("123");
        JSONObject properties = new JSONObject();
        properties.put("format", CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
        copyInfoBo.setProperties(properties.toString());
        CopyReplicationImport copyReplicationImport = getCopyReplicationImport();
        ReplicationOriginCopyDuration originCopyDuration = copyReplicationImport.getOriginCopyDuration();
        originCopyDuration.setRetentionType(RetentionTypeEnum.TEMPORARY.getType());
        originCopyDuration.setRetentionDuration(10);
        originCopyDuration.setDurationUnit("w");
        copyReplicationImport.setTimestamp(1687502348);
        ReflectionTestUtils.invokeMethod(controller, "buildCopyRetentionInfo", copyInfoBo,
            new Date(System.currentTimeMillis()), true, copyReplicationImport);
        assertThat(copyInfoBo).isNotNull();
        assertThat(copyInfoBo).isExactlyInstanceOf(CopyInfoBo.class);
        assertThat(copyInfoBo.getRetentionDuration()).isEqualTo(3);
        assertThat(copyInfoBo.getDurationUnit()).isEqualTo(originCopyDuration.getDurationUnit());
    }

    private CopyReplicationImport getCopyReplicationImport() throws JsonProcessingException {
        String properties
            = "{\"backup_id\":\"9191a1c7-ae44-4506-b484-4acd9f017ede\",\"oracle_metadata\":{\"ORACLEPARAM\":{\"db_name\":\"orcl\",\"is_asm\":0,\"is_cluster\":false,\"is_os_verify\":1,\"os_type\":\"linux\",\"verify_status\":\"true\",\"version\":\"18.0.0.0.0\"},\"PFILE\":\"{\\n   \\\"*.audit_file_dest\\\" : \\\"'/u01/app/oracle/admin/orcl/adump'\\\",\\n   \\\"*.audit_trail\\\" : \\\"'db'\\\",\\n   \\\"*.compatible\\\" : \\\"'18.0.0'\\\",\\n   \\\"*.control_files\\\" : \\\"'/u01/app/oracle/oradata/ORCL/controlfile/o1_mf_j1l9l0o6_.ctl','/u01/app/oracle/fast_recovery_area/ORCL/controlfile/o1_mf_j1l9l0p7_.ctl'\\\",\\n   \\\"*.db_block_size\\\" : \\\"8192\\\",\\n   \\\"*.db_create_file_dest\\\" : \\\"'/u01/app/oracle/oradata'\\\",\\n   \\\"*.db_name\\\" : \\\"'orcl'\\\",\\n   \\\"*.db_recovery_file_dest\\\" : \\\"'/u01/app/oracle/fast_recovery_area'\\\",\\n   \\\"*.db_recovery_file_dest_size\\\" : \\\"8106m\\\",\\n   \\\"*.diagnostic_dest\\\" : \\\"'/u01/app/oracle'\\\",\\n   \\\"*.dispatchers\\\" : \\\"'(PROTOCOL=TCP) (SERVICE=orclXDB)'\\\",\\n   \\\"*.open_cursors\\\" : \\\"300\\\",\\n   \\\"*.pga_aggregate_target\\\" : \\\"782m\\\",\\n   \\\"*.processes\\\" : \\\"320\\\",\\n   \\\"*.remote_login_passwordfile\\\" : \\\"'EXCLUSIVE'\\\",\\n   \\\"*.sga_target\\\" : \\\"2346m\\\",\\n   \\\"*.undo_tablespace\\\" : \\\"'UNDOTBS1'\\\",\\n   \\\"orcl.__data_transfer_cache_size\\\" : \\\"0\\\",\\n   \\\"orcl.__db_cache_size\\\" : \\\"1744830464\\\",\\n   \\\"orcl.__inmemory_ext_roarea\\\" : \\\"0\\\",\\n   \\\"orcl.__inmemory_ext_rwarea\\\" : \\\"0\\\",\\n   \\\"orcl.__java_pool_size\\\" : \\\"16777216\\\",\\n   \\\"orcl.__large_pool_size\\\" : \\\"33554432\\\",\\n   \\\"orcl.__oracle_base\\\" : \\\"'/u01/app/oracle'#ORACLE_BASE set from environment\\\",\\n   \\\"orcl.__pga_aggregate_target\\\" : \\\"822083584\\\",\\n   \\\"orcl.__sga_target\\\" : \\\"2466250752\\\",\\n   \\\"orcl.__shared_io_pool_size\\\" : \\\"134217728\\\",\\n   \\\"orcl.__shared_pool_size\\\" : \\\"520093696\\\",\\n   \\\"orcl.__streams_pool_size\\\" : \\\"0\\\"\\n}\\n\"}}";
        CopyReplicationImport importParam = new CopyReplicationImport();
        importParam.setTimestamp(System.currentTimeMillis() / 1000);
        importParam.setGeneratedTime(1615534657L);
        importParam.setProperties(JSONObject.fromObject(properties));
        ReplicationOriginCopyDuration originCopyDuration = new ReplicationOriginCopyDuration();
        originCopyDuration.setRetentionType(RetentionTypeEnum.PERMANENT.getType());
        importParam.setOriginCopyDuration(originCopyDuration);
        importParam.setMetadata(JSONObject.fromObject(getCopyReplicationMetadata()).toString());
        return importParam;
    }

    private CopyReplicationMetadata getCopyReplicationMetadata() throws JsonProcessingException {
        CopyReplicationMetadata copyReplicationMetadata = new CopyReplicationMetadata();
        PolicyBo policyBo = generatePolicy(FunctionTypeEnum.REPLICATION.value());
        ObjectMapper mapper = new ObjectMapper();
        policyBo.setExtParameters(mapper.readTree(
            "{\"specified_scope\":[{\"copy_type\":\"year\",\"generate_time_range\":\"3\",\"retention_unit\":\"y\",\"retention_duration\":1},{\"copy_type\":\"month\",\"generate_time_range\":\"first\",\"retention_unit\":\"MO\",\"retention_duration\":2},{\"copy_type\":\"week\",\"generate_time_range\":\"fri\",\"retention_unit\":\"w\",\"retention_duration\":3}],\"qos_id\":\"\",\"protocol\":2,\"storage_id\":\"27ad6ac00b0048e195fa93f5ff39c1bc\",\"network_access\":true,\"auto_retry\":false,\"replication_target_type\":2,\"alarm_after_failure\":false}"));
        copyReplicationMetadata.setReplicationPolicy(policyBo);

        return copyReplicationMetadata;
    }

    private PolicyBo generatePolicy(String type) {
        PolicyBo policyBo = new PolicyBo();
        policyBo.setType(type);
        JSONObject jsonObject = new JSONObject();
        jsonObject.set(SlaConstant.QOS_ID, "1");
        jsonObject.set("is_worm", true);
        ObjectMapper mapper = new ObjectMapper();
        JsonNode node;
        try {
            node = mapper.readTree(jsonObject.toString());
        } catch (JsonProcessingException e) {
            throw new RuntimeException(e);
        }
        policyBo.setExtParameters(node);
        return policyBo;
    }

    /**
     * 用例名称：FC级联复制副本索引状态为"Unsupport"<br/>
     * 前置条件：无<br/>
     * check点：索引状态正确<br/>
     */
    @Test
    public void test_FC_index_status_when_build_copy_info() {
        CopyInfoBo copyInfoBo = new CopyInfoBo();
        copyInfoBo.setResourceId("123");
        copyInfoBo.setUuid("123");
        JSONObject properties = new JSONObject();
        properties.put("format", CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
        copyInfoBo.setProperties(properties.toString());
        copyInfoBo.setResourceSubType(ResourceSubTypeEnum.FUSION_COMPUTE.getType());
        copyInfoBo.setGeneratedBy(CopyGeneratedByEnum.BY_CASCADED_REPLICATION.value());
        ReflectionTestUtils.invokeMethod(controller, "buildCopyIndexStatus", copyInfoBo);
        assertThat(copyInfoBo).isNotNull();
        assertThat(copyInfoBo).isExactlyInstanceOf(CopyInfoBo.class);
        assertThat(copyInfoBo.getIndexed()).isEqualTo(CopyIndexStatus.UNSUPPORT.getIndexStaus());
    }

    /**
     * 用例名称：域内复制副本更新userId
     * 前置条件：无
     * check点：userId正确
     */
    @Test
    public void test_intra_replication_reset_user_id() {
        // prepare
        CopyInfoBo copyInfoBo = new CopyInfoBo();
        copyInfoBo.setUserId("originUserId");
        JSONObject ext = new JSONObject();
        JSONObject policy = new JSONObject();

        // run
        ReflectionTestUtils.invokeMethod(controller, "resetIntraCopyUserId", policy, copyInfoBo, "resetUserId");
        // verify
        assertThat(copyInfoBo.getUserId()).isEqualTo("originUserId");

        policy.set("ext_parameters", ext);
        // run
        ReflectionTestUtils.invokeMethod(controller, "resetIntraCopyUserId", policy, copyInfoBo, "resetUserId");
        // verify
        assertThat(copyInfoBo.getUserId()).isEqualTo("originUserId");

        ext.put("replication_target_mode", ReplicationMode.INTRA.getValue());
        // run
        ReflectionTestUtils.invokeMethod(controller, "resetIntraCopyUserId", policy, copyInfoBo, "resetUserId");
        // verify
        assertThat(copyInfoBo.getUserId()).isEqualTo("resetUserId");
    }
}
