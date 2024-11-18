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
package openbackup.data.access.framework.livemount.provider;

import openbackup.data.access.framework.livemount.provider.LiveMountJobCallbackProvider;
import openbackup.data.access.framework.livemount.service.LiveMountService;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import org.junit.BeforeClass;
import org.junit.Test;
import org.mockito.Mockito;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;

import static org.mockito.ArgumentMatchers.any;

/**
 * 功能描述: LiveMountJobCallbackProviderTest
 *
 */
public class LiveMountJobCallbackProviderTest {
    private static LiveMountJobCallbackProvider liveMountJobCallbackProvider;

    private static CopyRestApi copyRestApi;

    private static RedissonClient redissonClient;

    private static LiveMountService liveMountService;

    @BeforeClass
    public static void init() {
        copyRestApi = Mockito.mock(CopyRestApi.class);
        redissonClient = Mockito.mock(RedissonClient.class);
        liveMountService = Mockito.mock(LiveMountService.class);
        liveMountJobCallbackProvider = new LiveMountJobCallbackProvider(copyRestApi, redissonClient, liveMountService);
    }

    /**
     * 用例场景：即时挂载任务超时失败后，回退副本状态
     * 前置条件：无
     * 检查点：副本状态回退正常
     */
    @Test
    public void livemount_reset_copy_status_success() {
        JobBo mountJob = mockJob(JobTypeEnum.LIVE_MOUNT.getValue());
        Mockito.when(redissonClient.getMap(mountJob.getJobId(), StringCodec.INSTANCE)).thenReturn(Mockito.mock(RMap.class));
        liveMountJobCallbackProvider.doCallback(mountJob);
        Mockito.verify(copyRestApi, Mockito.times(1)).updateCopyStatus(any(), any());
        JobBo unmountJob = mockJob(JobTypeEnum.UNMOUNT.getValue());
        Mockito.when(redissonClient.getMap(unmountJob.getJobId(), StringCodec.INSTANCE)).thenReturn(Mockito.mock(RMap.class));
        liveMountJobCallbackProvider.doCallback(unmountJob);
        Mockito.verify(copyRestApi, Mockito.times(2)).updateCopyStatus(any(), any());
    }

    private JobBo mockJob(String jobType) {
        JobBo jobBo = new JobBo();
        jobBo.setJobId(UUIDGenerator.getUUID());
        jobBo.setCopyId(UUIDGenerator.getUUID());
        jobBo.setType(jobType);
        jobBo.setMessage("{\"topic\":\"live-mount.execute.request\",\"payload\":{\"live_mount\":{\"id\":\"138285a1-80a0-4c34-9b42-393660f0162d\",\"resource_type\":\"Fileset\",\"resource_sub_type\":\"Fileset\",\"resource_id\":\"dafa807db9144be5ad5b6d52dff762b2\",\"resource_name\":\"test\",\"resource_path\":\"192.168.153.53\",\"resource_ip\":\"192.168.153.53\",\"policy_id\":\"\",\"copy_id\":\"fa216849-7ac6-4bac-b011-2fd70fb8ba06\",\"target_location\":\"others\",\"target_resource_id\":\"19889e9f-6880-42a3-92e4-433db8b4b8ab\",\"target_resource_name\":\"localhost.localdomain(192.168.96.16)\",\"target_resource_path\":\"others\",\"target_resource_ip\":\"192.168.96.16\",\"parameters\":\"{\\\"performance\\\":{},\\\"name\\\":\\\"localhost.localdomain(192.168.96.16)\\\",\\\"dstPath\\\":\\\"/tmp\\\"}\",\"anonymization_status\":0,\"status\":\"ready\",\"enable_status\":\"activated\",\"created_time\":\"2022-09-24 14:51:39\",\"updated_time\":\"2022-09-24 14:51:39\",\"mounted_copy_id\":\"\",\"mounted_copy_display_timestamp\":null,\"mounted_source_copy_id\":\"\",\"schedule_id\":\"\",\"mounted_resource_id\":\"\",\"user_id\":\"88a94c476f12a21e016f12a246e50009\",\"file_system_share_info\":\"[{\\\"type\\\":1,\\\"fileSystemName\\\":\\\"fileset_mount_1664002252372\\\",\\\"accessPermission\\\":1,\\\"advanceParams\\\":{\\\"clientType\\\":0,\\\"clientName\\\":\\\"*\\\",\\\"squash\\\":1,\\\"rootSquash\\\":1,\\\"portSecure\\\":1}}]\",\"mount_job_id\":\"\"},\"source_copy\":{\"uuid\":\"fa216849-7ac6-4bac-b011-2fd70fb8ba06\",\"chain_id\":\"ef2b5b13-54e3-4af3-b587-c61941165587\",\"timestamp\":\"1663777557000000\",\"display_timestamp\":\"2022-09-22T00:25:57\",\"status\":\"Normal\",\"location\":\"Local\",\"backup_type\":1,\"generated_by\":\"Backup\",\"generated_time\":\"2022-09-22T00:25:57\",\"generation_type\":\"\",\"features\":2,\"indexed\":\"Unindexed\",\"generation\":1,\"parent_copy_uuid\":\"\",\"retention_type\":2,\"retention_duration\":1,\"duration_unit\":\"d\",\"expiration_time\":1663892757000,\"properties\":\"{\\\"snapshots\\\":[{\\\"id\\\":\\\"154@fa216849-7ac6-4bac-b011-2fd70fb8ba06\\\",\\\"parentName\\\":\\\"Fileset_dafa807db9144be5ad5b6d52dff762b2\\\"}],\\\"verifyStatus\\\":\\\"3\\\",\\\"repositories\\\":[{\\\"type\\\":1,\\\"protocol\\\":5,\\\"role\\\":0,\\\"remotePath\\\":[{\\\"type\\\":0,\\\"path\\\":\\\"/Fileset_dafa807db9144be5ad5b6d52dff762b2/source_policy_dafa807db9144be5ad5b6d52dff762b2_Context_Global_MD\\\"},{\\\"type\\\":1,\\\"path\\\":\\\"/Fileset_dafa807db9144be5ad5b6d52dff762b2/source_policy_dafa807db9144be5ad5b6d52dff762b2_Context\\\"}],\\\"extendInfo\\\":{\\\"esn\\\":\\\"2102353GTH10L8000006\\\"}},{\\\"type\\\":2,\\\"protocol\\\":5,\\\"role\\\":0,\\\"remotePath\\\":[{\\\"type\\\":1,\\\"path\\\":\\\"/Fileset_CacheDataRepository/dafa807db9144be5ad5b6d52dff762b2\\\"}],\\\"extendInfo\\\":{\\\"esn\\\":\\\"2102353GTH10L8000006\\\"}}],\\\"sub_object\\\":[{\\\"name\\\":\\\"/root\\\"}],\\\"dataBeforeReduction\\\":579111345,\\\"dataAfterReduction\\\":112637441,\\\"format\\\":0,\\\"multiFileSystem\\\":\\\"false\\\"}\",\"resource_id\":\"dafa807db9144be5ad5b6d52dff762b2\",\"resource_name\":\"test\",\"resource_type\":\"Fileset\",\"resource_sub_type\":\"Fileset\",\"resource_location\":\"192.168.153.53\",\"resource_status\":\"EXIST\",\"resource_environment_name\":\"host-8-40-147-9\",\"resource_environment_ip\":\"192.168.153.53\",\"resource_properties\":\"{\\\"name\\\":\\\"test\\\",\\\"path\\\":\\\"192.168.153.53\\\",\\\"root_uuid\\\":\\\"d968824d-c5f3-44d1-96c4-6a1fa294923b\\\",\\\"parent_name\\\":null,\\\"parent_uuid\\\":\\\"bd4c3114-e152-3ffd-9cfc-16d5c83642d6\\\",\\\"children_uuids\\\":null,\\\"type\\\":\\\"Fileset\\\",\\\"sub_type\\\":\\\"Fileset\\\",\\\"uuid\\\":\\\"dafa807db9144be5ad5b6d52dff762b2\\\",\\\"created_time\\\":\\\"2022-09-07T18:31:01.349000\\\",\\\"ext_parameters\\\":{\\\"consistent_backup\\\":false,\\\"cross_file_system\\\":false,\\\"backup_nfs\\\":false,\\\"sparse_file_detection\\\":false,\\\"backup_continue_with_files_backup_failed\\\":false,\\\"small_file_aggregation\\\":false,\\\"aggregation_file_size\\\":false,\\\"aggregation_file_max_size\\\":false,\\\"pre_script\\\":null,\\\"post_script\\\":null,\\\"failed_script\\\":null},\\\"authorized_user\\\":\\\"zheng\\\",\\\"user_id\\\":\\\"85acd668b9d8487893067d408c619561\\\",\\\"version\\\":null,\\\"sla_id\\\":\\\"14817c24-f4ed-4df3-a450-e4f2a8c46b74\\\",\\\"sla_name\\\":\\\"fileset_wyj_rep1\\\",\\\"sla_status\\\":true,\\\"sla_compliance\\\":true,\\\"protection_status\\\":1,\\\"environment_uuid\\\":\\\"d968824d-c5f3-44d1-96c4-6a1fa294923b\\\",\\\"environment_name\\\":\\\"host-8-40-147-9\\\",\\\"environment_endpoint\\\":\\\"192.168.153.53\\\",\\\"environment_os_type\\\":\\\"linux\\\",\\\"environment_type\\\":\\\"Host\\\",\\\"environment_sub_type\\\":\\\"UBackupAgent\\\",\\\"environment_is_cluster\\\":\\\"False\\\",\\\"environment_os_name\\\":\\\"linux\\\",\\\"extendInfo\\\":{\\\"templateName\\\":\\\"\\\",\\\"paths\\\":\\\"[{\\\\\\\"name\\\\\\\":\\\\\\\"/root\\\\\\\"}]\\\",\\\"filters\\\":\\\"[]\\\",\\\"templateId\\\":\\\"\\\"}}\",\"sla_name\":\"fileset_wyj_rep1\",\"sla_properties\":\"{\\\"uuid\\\": \\\"14817c24-f4ed-4df3-a450-e4f2a8c46b74\\\", \\\"name\\\": \\\"fileset_wyj_rep1\\\", \\\"created_time\\\": \\\"2022-09-21T16:21:04.476+08:00\\\", \\\"type\\\": 1, \\\"application\\\": \\\"Fileset\\\", \\\"policy_list\\\": [{\\\"uuid\\\": \\\"52778f41-4fe1-4482-91d2-5a9d357bb341\\\", \\\"name\\\": \\\"\\\\u5168\\\\u91cf01\\\", \\\"type\\\": \\\"backup\\\", \\\"action\\\": \\\"full\\\", \\\"retention\\\": {\\\"retention_type\\\": 2, \\\"retention_duration\\\": 1, \\\"duration_unit\\\": \\\"d\\\", \\\"daily_copies\\\": null, \\\"weekly_copies\\\": null, \\\"monthly_copies\\\": null, \\\"yearly_copies\\\": null}, \\\"schedule\\\": {\\\"trigger\\\": 1, \\\"interval\\\": 12, \\\"interval_unit\\\": \\\"h\\\", \\\"start_time\\\": \\\"2022-09-21T00:00:00\\\", \\\"window_start\\\": \\\"00:00:00\\\", \\\"window_end\\\": \\\"01:00:00\\\", \\\"days_of_week\\\": null, \\\"days_of_month\\\": null, \\\"days_of_year\\\": null, \\\"trigger_action\\\": null}, \\\"ext_parameters\\\": {\\\"qos_id\\\": \\\"\\\", \\\"alarm_after_failure\\\": false, \\\"source_deduplication\\\": false, \\\"auto_retry\\\": true, \\\"auto_retry_times\\\": 3, \\\"auto_retry_wait_minutes\\\": 5}, \\\"active\\\": true, \\\"is_active\\\": true}, {\\\"uuid\\\": \\\"0f16d695-542b-49ab-96c6-14f2674deb67\\\", \\\"name\\\": \\\"\\\\u7b56\\\\u75650\\\", \\\"type\\\": \\\"replication\\\", \\\"action\\\": \\\"replication\\\", \\\"retention\\\": {\\\"retention_type\\\": 2, \\\"retention_duration\\\": 1, \\\"duration_unit\\\": \\\"d\\\", \\\"daily_copies\\\": null, \\\"weekly_copies\\\": null, \\\"monthly_copies\\\": null, \\\"yearly_copies\\\": null}, \\\"schedule\\\": {\\\"trigger\\\": 2, \\\"interval\\\": null, \\\"interval_unit\\\": null, \\\"start_time\\\": \\\"2022-09-15T00:36:26\\\", \\\"window_start\\\": null, \\\"window_end\\\": null, \\\"days_of_week\\\": null, \\\"days_of_month\\\": null, \\\"days_of_year\\\": null, \\\"trigger_action\\\": null}, \\\"ext_parameters\\\": {\\\"qos_id\\\": \\\"\\\", \\\"external_system_id\\\": \\\"21\\\", \\\"link_deduplication\\\": false, \\\"link_compression\\\": false, \\\"alarm_after_failure\\\": false}, \\\"active\\\": true, \\\"is_active\\\": true}], \\\"resource_count\\\": null, \\\"archival_count\\\": null, \\\"replication_count\\\": null, \\\"is_global\\\": false}\",\"job_type\":\"\",\"user_id\":\"85acd668b9d8487893067d408c619561\",\"is_archived\":false,\"is_replicated\":false,\"name\":\"test_1663777557\",\"storage_id\":\"\",\"amount\":0,\"gn\":379,\"prev_copy_id\":\"9a6ac049-2086-41cb-8aa3-b4bd3fdf665c\",\"next_copy_id\":\"\",\"prev_copy_gn\":377,\"next_copy_gn\":0,\"deletable\":true},\"live_mount.debuts\":true,\"timestamp\":1664002299714,\"request_id\":\"906710cc-b87f-48d5-a8f5-da266aa0d228\"},\"traffic\":null,\"abolish\":[],\"inContext\":false,\"status\":\"READY\"}");
        return jobBo;
    }
}