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
package openbackup.data.access.framework.protection.mocks;

import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseStorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.model.Copy;

import java.util.ArrayList;
import java.util.List;

/**
 * 副本信息的Mock类，模拟测试用例中{@code Copy}对象的各种情况
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/6
 **/
public class CopyMocker {

    /**
     * 模拟通用的副本信息
     *
     * @return {@code Copy} 副本信息
     */
    public static Copy mockCommonCopy() {
        String copyInfoStr = "{\"archived\":false,\"replicated\":false,"
            + "\"resource_id\":\"9a1cb042-3e98-4424-b775-e3111e2807e7\",\"resource_name\":\"zrj_test\","
            + "\"resource_type\":\"Oracle\",\"resource_sub_type\":\"Oracle\",\"resource_location\":\"localhost"
            + ".localdomain\",\"resource_status\":\"NOT_EXIST\",\"resource_properties\":\"{\\\"name\\\":\\\"zrj_test\\\",\\\"paths\\\":[\\\"/zrj_test/test1.txt\\\"],\\\"filters\\\":[],\\\"path\\\":\\\"localhost.localdomain\\\",\\\"root_uuid\\\":\\\"84842098384c085a375c1a0674fc8472\\\",\\\"parent_name\\\":null,\\\"parent_uuid\\\":null,\\\"children_uuids\\\":null,\\\"type\\\":\\\"Fileset\\\",\\\"sub_type\\\":\\\"Fileset\\\",\\\"uuid\\\":\\\"9a1cb042-3e98-4424-b775-e3111e2807e7\\\",\\\"created_time\\\":\\\"2021-12-06T22:23:31.673110\\\",\\\"ext_parameters\\\":{\\\"before_protect_script\\\":null,\\\"after_protect_script\\\":null,\\\"protect_failed_script\\\":null},\\\"authorized_user\\\":null,\\\"user_id\\\":null,\\\"version\\\":null,\\\"sla_id\\\":\\\"13d6554f-15fa-4dc3-bb67-c54eac7e4496\\\",\\\"sla_name\\\":\\\"fileset_copyreplication_sla\\\",\\\"sla_status\\\":true,\\\"sla_compliance\\\":null,\\\"protection_status\\\":1,\\\"environment_uuid\\\":\\\"84842098384c085a375c1a0674fc8472\\\",\\\"environment_name\\\":\\\"localhost.localdomain\\\",\\\"environment_endpoint\\\":\\\"192.168.100.9\\\",\\\"environment_os_type\\\":\\\"linux\\\",\\\"environment_type\\\":\\\"Host\\\",\\\"environment_sub_type\\\":\\\"ABBackupClient\\\",\\\"environment_is_cluster\\\":\\\"False\\\",\\\"environment_os_name\\\":\\\"Linux el7\\\",\\\"cluster_type\\\":null}\",\"resource_environment_name\":\"localhost.localdomain\",\"resource_environment_ip\":\"192.168.100.9\",\"uuid\":\"bec9c4c8-9d7e-4f14-9935-7e81224d0dff\",\"chain_id\":\"b87a82f9-2c09-41af-adba-bdf6c919c5a0\",\"timestamp\":\"1638847307559126\",\"display_timestamp\":\"2021-12-07T11:21:47\",\"deletable\":true,\"status\":\"Normal\",\"location\":\"Local\",\"backup_type\":1,\"generated_by\":\"Replicated\",\"generated_time\":\"2021-12-06T22:26:11\",\"generation_type\":\"\",\"features\":2,\"indexed\":\"Indexed\",\"generation\":1,\"parent_copy_uuid\":\"\",\"retention_type\":2,\"retention_duration\":1,\"duration_unit\":\"d\",\"expiration_time\":1638887171552,\"properties\":\"{\\\"backup_id\\\":\\\"3cd29fa356a011ec92923a72c200ca94\\\",\\\"backup_type\\\":1}\",\"sla_name\":\"fileset_copyreplication_sla\",\"sla_properties\":\"{\\\"name\\\":\\\"fileset_copyreplication_sla\\\",\\\"type\\\":2,\\\"application\\\":\\\"Fileset\\\",\\\"created_time\\\":\\\"2021-12-06T22:23:25.040011\\\",\\\"uuid\\\":\\\"13d6554f-15fa-4dc3-bb67-c54eac7e4496\\\",\\\"is_global\\\":false,\\\"policy_list\\\":[{\\\"uuid\\\":\\\"87c630f1-3931-4d24-8521-df14f1f7c510\\\",\\\"name\\\":\\\"full\\\",\\\"action\\\":\\\"full\\\",\\\"ext_parameters\\\":{\\\"auto_retry\\\":false,\\\"auto_retry_times\\\":null,\\\"auto_retry_wait_minutes\\\":null,\\\"qos_id\\\":\\\"\\\",\\\"file_scan_channel_number\\\":null,\\\"read_and_send_channel_number\\\":null,\\\"encryption\\\":false,\\\"fine_grained_restore\\\":false,\\\"permanent_increment\\\":false},\\\"retention\\\":{\\\"retention_type\\\":2,\\\"duration_unit\\\":\\\"d\\\",\\\"retention_duration\\\":1},\\\"schedule\\\":{\\\"trigger\\\":1,\\\"interval\\\":1,\\\"interval_unit\\\":\\\"h\\\",\\\"start_time\\\":\\\"2021-12-07T00:00:00\\\",\\\"window_start\\\":\\\"22:23:53\\\",\\\"window_end\\\":\\\"23:59:59\\\",\\\"days_of_month\\\":null,\\\"days_of_year\\\":null,\\\"trigger_action\\\":null,\\\"days_of_week\\\":null},\\\"type\\\":\\\"backup\\\"},{\\\"uuid\\\":\\\"77280969-a0fc-45fd-8419-204e8dd96fef\\\",\\\"name\\\":\\\"zrj_copyreplication_colony_1\\\",\\\"action\\\":\\\"replication\\\",\\\"ext_parameters\\\":{\\\"qos_id\\\":\\\"\\\",\\\"external_system_id\\\":\\\"3\\\"},\\\"retention\\\":{\\\"retention_type\\\":2,\\\"duration_unit\\\":\\\"d\\\",\\\"retention_duration\\\":1},\\\"schedule\\\":{\\\"trigger\\\":2,\\\"interval\\\":0,\\\"interval_unit\\\":\\\"h\\\",\\\"start_time\\\":\\\"2020-12-12T00:00:00\\\",\\\"window_start\\\":null,\\\"window_end\\\":null,\\\"days_of_month\\\":null,\\\"days_of_year\\\":null,\\\"trigger_action\\\":null,\\\"days_of_week\\\":null},\\\"type\\\":\\\"replication\\\"}],\\\"resource_count\\\":null,\\\"archival_count\\\":null,\\\"replication_count\\\":null}\",\"job_type\":\"\",\"user_id\":\"3a613cef4efb419c8fcbf9df6e9980f9\",\"is_archived\":false,\"is_replicated\":false,\"amount\":0,\"gn\":95,\"prev_copy_id\":\"\",\"next_copy_id\":\"\",\"prev_copy_gn\":0,\"next_copy_gn\":0}";
        final Copy copy = JSONObject.toBean(copyInfoStr, Copy.class);
        copy.setProperties(mockCopyPropertiesRepositories());
        return copy;
    }

    public static Copy mockHdfsCopy() {
        String copyStr = "{\n" + "    \"uuid\": \"b3c157e3-49bd-4add-8ca4-271ab23f2367\",\n"
                + "    \"chain_id\": \"f6485d39-1804-4776-a8ed-0fbb4c17da78\",\n"
                + "    \"timestamp\": \"1640660309000000\",\n" + "    \"display_timestamp\": \"2021-12-28T10:58:29\",\n"
                + "    \"deletable\": true,\n" + "    \"status\": \"Normal\",\n" + "    \"location\": \"Local\",\n"
                + "    \"backup_type\": 1,\n" + "    \"generated_by\": \"Backup\",\n"
                + "    \"generated_time\": \"2021-12-28T10:58:29\",\n" + "    \"features\": 2,\n"
                + "    \"indexed\": \"Unindexed\",\n" + "    \"generation\": 1,\n" + "    \"parent_copy_uuid\": null,\n"
                + "    \"retention_type\": 2,\n" + "    \"retention_duration\": 1,\n"
                + "    \"duration_unit\": \"MO\",\n" + "    \"expiration_time\": \"2022-01-28T10:58:29\",\n"
                + "    \"properties\": \"{\\\"snapshots\\\":[{\\\"id\\\":\\\"24283@b3c157e3-49bd-4add-8ca4-271ab23f2367\\\",\\\"parentName\\\":\\\"HDFS_3c40f40f-e281-444d-9ec9-63acccd86e0a\\\"}],\\\"repositories\\\":[{\\\"type\\\":1,\\\"protocol\\\":5}]}\",\n"
                + "    \"resource_id\": \"3c40f40f-e281-444d-9ec9-63acccd86e0a\",\n"
                + "    \"resource_name\": \"yangtest\",\n" + "    \"resource_type\": \"HDFS\",\n"
                + "    \"resource_sub_type\": \"HDFSFileset\",\n" + "    \"resource_location\": \"test_simple\",\n"
                + "    \"resource_status\": \"EXIST\",\n"
                + "    \"resource_properties\": \"{\\\"name\\\": \\\"yangtest\\\", \\\"path\\\": null, \\\"root_uuid\\\": \\\"8.40.113.96\\\", \\\"parent_name\\\": null, \\\"parent_uuid\\\": null, \\\"children_uuids\\\": null, \\\"type\\\": \\\"HDFS\\\", \\\"sub_type\\\": \\\"HDFSFileset\\\", \\\"uuid\\\": \\\"3c40f40f-e281-444d-9ec9-63acccd86e0a\\\", \\\"created_time\\\": \\\"2021-12-28T10:54:46.199000\\\", \\\"ext_parameters\\\": {\\\"proxy_host_select_mode\\\": 0, \\\"agents\\\": null, \\\"before_protect_script\\\": null, \\\"after_protect_script\\\": null, \\\"protect_failed_script\\\": null}, \\\"authorized_user\\\": null, \\\"user_id\\\": null, \\\"version\\\": null, \\\"sla_id\\\": \\\"8556bb41-abe6-4821-870d-a0252f304dfc\\\", \\\"sla_name\\\": \\\"Gold\\\", \\\"sla_status\\\": true, \\\"sla_compliance\\\": null, \\\"protection_status\\\": 1, \\\"environment_uuid\\\": \\\"8.40.113.96\\\", \\\"environment_name\\\": \\\"test_simple\\\", \\\"environment_endpoint\\\": \\\"8.40.113.96\\\", \\\"environment_os_type\\\": null, \\\"environment_type\\\": \\\"BigData\\\", \\\"environment_sub_type\\\": \\\"HDFS\\\", \\\"environment_is_cluster\\\": \\\"False\\\", \\\"environment_os_name\\\": null}\",\n"
                + "    \"resource_environment_name\": \"test_simple\",\n"
                + "    \"resource_environment_ip\": \"8.40.113.96\",\n" + "    \"sla_name\": \"Gold\",\n"
                + "    \"sla_properties\": \"{\\\"name\\\": \\\"Gold\\\", \\\"type\\\": 1, \\\"application\\\": \\\"Common\\\", \\\"created_time\\\": \\\"9999-01-09T20:21:32.842417\\\", \\\"uuid\\\": \\\"8556bb41-abe6-4821-870d-a0252f304dfc\\\", \\\"is_global\\\": true, \\\"policy_list\\\": [{\\\"uuid\\\": \\\"3bd9e448-816d-43b1-9b16-1feca34ece65\\\", \\\"name\\\": \\\"full\\\", \\\"action\\\": \\\"full\\\", \\\"ext_parameters\\\": {\\\"auto_retry\\\": true, \\\"auto_retry_times\\\": 3, \\\"auto_retry_wait_minutes\\\": 5}, \\\"retention\\\": {\\\"retention_type\\\": 2, \\\"duration_unit\\\": \\\"MO\\\", \\\"retention_duration\\\": 1}, \\\"schedule\\\": {\\\"trigger\\\": 1, \\\"interval\\\": 1, \\\"interval_unit\\\": \\\"d\\\", \\\"start_time\\\": \\\"2021-04-20T00:00:00\\\", \\\"window_start\\\": \\\"00:30:00\\\", \\\"window_end\\\": \\\"00:30:00\\\", \\\"days_of_month\\\": null, \\\"days_of_year\\\": null, \\\"trigger_action\\\": null, \\\"days_of_week\\\": null}, \\\"type\\\": \\\"backup\\\"}, {\\\"uuid\\\": \\\"9b17382f-7164-4f5b-8d77-2910a0be348c\\\", \\\"name\\\": \\\"difference_increment\\\", \\\"action\\\": \\\"difference_increment\\\", \\\"ext_parameters\\\": {\\\"auto_retry\\\": true, \\\"auto_retry_times\\\": 3, \\\"auto_retry_wait_minutes\\\": 5}, \\\"retention\\\": {\\\"retention_type\\\": 2, \\\"duration_unit\\\": \\\"MO\\\", \\\"retention_duration\\\": 1}, \\\"schedule\\\": {\\\"trigger\\\": 1, \\\"interval\\\": 4, \\\"interval_unit\\\": \\\"h\\\", \\\"start_time\\\": \\\"2021-04-20T00:00:00\\\", \\\"window_start\\\": \\\"01:00:00\\\", \\\"window_end\\\": \\\"01:00:00\\\", \\\"days_of_month\\\": null, \\\"days_of_year\\\": null, \\\"trigger_action\\\": null, \\\"days_of_week\\\": null}, \\\"type\\\": \\\"backup\\\"}], \\\"resource_count\\\": null, \\\"archival_count\\\": null, \\\"replication_count\\\": null}\",\n"
                + "    \"user_id\": \"88a94c476f12a21e016f12a246e50009\",\n" + "    \"is_archived\": false,\n"
                + "    \"is_replicated\": false,\n" + "    \"gn\": 2,\n" + "    \"prev_copy_id\": null,\n"
                + "    \"next_copy_id\": null,\n" + "    \"prev_copy_gn\": null,\n" + "    \"next_copy_gn\": null\n"
                + "}";
        final Copy copy = JSONObject.toBean(copyStr, Copy.class);
        copy.setProperties(mockCopyPropertiesRepositories());
        return copy;
    }

    public static Copy mockNasCopy() {
        String copyStr = "\t\t{\n" + "\t\t\t\"uuid\": \"ab9ca198-a540-4857-805b-6673544aede8\",\n"
                + "\t\t\t\"chain_id\": \"6ee10417-86e9-4be4-8c62-ffd274490d2a\",\n"
                + "\t\t\t\"timestamp\": \"1645164893000000\",\n"
                + "\t\t\t\"display_timestamp\": \"2022-02-18T14:14:53\",\n" + "\t\t\t\"deletable\": true,\n"
                + "\t\t\t\"status\": \"Normal\",\n" + "\t\t\t\"location\": \"Local\",\n" + "\t\t\t\"backup_type\": 5,\n"
                + "\t\t\t\"generated_by\": \"Backup\",\n" + "\t\t\t\"generated_time\": \"2022-02-18T14:14:53\",\n"
                + "\t\t\t\"features\": 2,\n" + "\t\t\t\"indexed\": \"Index_fail\",\n" + "\t\t\t\"generation\": 1,\n"
                + "\t\t\t\"parent_copy_uuid\": null,\n" + "\t\t\t\"retention_type\": 2,\n"
                + "\t\t\t\"retention_duration\": 333,\n" + "\t\t\t\"duration_unit\": \"d\",\n"
                + "\t\t\t\"expiration_time\": \"2023-01-17T14:14:53\",\n"
                + "\t\t\t\"properties\": \"{\\\"snapshots\\\":[{\\\"id\\\":\\\"37641@ab9ca198-a540-4857-805b-6673544aede8\\\",\\\"parentName\\\":\\\"Storage_325fa308-ea59-396e-875d-909e966e7f88\\\"}],\\\"repositories\\\":[{\\\"type\\\":1,\\\"protocol\\\":5,\\\"extendInfo\\\":{\\\"fileSystemId\\\":\\\"664\\\",\\\"productEsn\\\":\\\"2102354DEY13D2121011\\\"}},{\\\"type\\\":0,\\\"protocol\\\":5,\\\"extendInfo\\\":{\\\"fileSystemId\\\":\\\"664\\\",\\\"productEsn\\\":\\\"2102354DEY13D2121011\\\"}},{\\\"type\\\":2,\\\"protocol\\\":5,\\\"extendInfo\\\":{\\\"fileSystemId\\\":\\\"664\\\",\\\"productEsn\\\":\\\"2102354DEY13D2121011\\\"}}],\\\"isAggregation\\\":\\\"false\\\"}\",\n"
                + "\t\t\t\"resource_id\": \"325fa308-ea59-396e-875d-909e966e7f88\",\n"
                + "\t\t\t\"resource_name\": \"lr_nas2\",\n" + "\t\t\t\"resource_type\": \"Storage\",\n"
                + "\t\t\t\"resource_sub_type\": \"NasFileSystem\",\n"
                + "\t\t\t\"resource_location\": \"OceanStor Dorado 6.1.3+\\\\dorado35\\\\System_vStore\\\\lr_nas2\",\n"
                + "\t\t\t\"resource_status\": \"EXIST\",\n"
                + "\t\t\t\"resource_properties\": \"{\\\"name\\\":\\\"lr_nas2\\\",\\\"path\\\":\\\"OceanStor Dorado 6.1.3+\\\\\\\\dorado35\\\\\\\\System_vStore\\\\\\\\lr_nas2\\\",\\\"root_uuid\\\":\\\"2102354DEY13D2121011\\\",\\\"parent_name\\\":\\\"dorado35\\\",\\\"parent_uuid\\\":\\\"2102354DEY13D2121011\\\",\\\"children_uuids\\\":null,\\\"type\\\":\\\"Storage\\\",\\\"sub_type\\\":\\\"NasFileSystem\\\",\\\"uuid\\\":\\\"325fa308-ea59-396e-875d-909e966e7f88\\\",\\\"created_time\\\":\\\"2022-02-18T11:14:36.207000\\\",\\\"ext_parameters\\\":{\\\"proxy_host_mode\\\":1,\\\"agents\\\":\\\"29bbf420-f8aa-42b5-b58a-d576d4033c26\\\"},\\\"authorized_user\\\":null,\\\"user_id\\\":null,\\\"version\\\":null,\\\"sla_id\\\":\\\"9a9464f7-d395-4950-9ccd-0f192b8665f5\\\",\\\"sla_name\\\":\\\"nas\\\",\\\"sla_status\\\":true,\\\"sla_compliance\\\":null,\\\"protection_status\\\":1,\\\"environment_uuid\\\":\\\"2102354DEY13D2121011\\\",\\\"environment_name\\\":\\\"dorado35\\\",\\\"environment_endpoint\\\":\\\"8.40.101.35\\\",\\\"environment_os_type\\\":null,\\\"environment_type\\\":\\\"StorageEquipment\\\",\\\"environment_sub_type\\\":\\\"DoradoV6\\\",\\\"environment_is_cluster\\\":\\\"False\\\",\\\"environment_os_name\\\":null,\\\"extendInfo\\\":{\\\"tenantName\\\":\\\"System_vStore\\\",\\\"fileSystemId\\\":\\\"664\\\",\\\"usedCapacity\\\":\\\"0\\\",\\\"onlineStatus\\\":\\\"27\\\",\\\"tenantId\\\":\\\"0\\\",\\\"capacity\\\":\\\"20971520\\\"}}\",\n"
                + "\t\t\t\"resource_environment_name\": \"dorado35\",\n"
                + "\t\t\t\"resource_environment_ip\": \"8.40.101.35\",\n" + "\t\t\t\"sla_name\": \"nas\",\n"
                + "\t\t\t\"sla_properties\": \"{\\\"name\\\": \\\"nas\\\", \\\"type\\\": 1, \\\"application\\\": \\\"NasFileSystem\\\", \\\"created_time\\\": \\\"2022-02-18T11:17:55.630585\\\", \\\"uuid\\\": \\\"9a9464f7-d395-4950-9ccd-0f192b8665f5\\\", \\\"is_global\\\": false, \\\"policy_list\\\": [{\\\"uuid\\\": \\\"ac032e12-9f7e-4af3-90c3-0729e97aad9b\\\", \\\"name\\\": \\\"permanent_increment\\\", \\\"action\\\": \\\"permanent_increment\\\", \\\"ext_parameters\\\": {\\\"auto_retry\\\": true, \\\"auto_retry_times\\\": 3, \\\"auto_retry_wait_minutes\\\": 5, \\\"qos_id\\\": \\\"\\\", \\\"auto_index\\\": true}, \\\"retention\\\": {\\\"retention_type\\\": 2, \\\"duration_unit\\\": \\\"d\\\", \\\"retention_duration\\\": 333}, \\\"schedule\\\": {\\\"trigger\\\": 1, \\\"interval\\\": 23, \\\"interval_unit\\\": \\\"h\\\", \\\"start_time\\\": \\\"2022-02-18T00:00:00\\\", \\\"window_start\\\": \\\"00:00:00\\\", \\\"window_end\\\": \\\"23:00:00\\\", \\\"days_of_month\\\": null, \\\"days_of_year\\\": null, \\\"trigger_action\\\": null, \\\"days_of_week\\\": null}, \\\"type\\\": \\\"backup\\\"}, {\\\"uuid\\\": \\\"d05e8ca1-1f7c-4bd1-866d-009608055cc5\\\", \\\"name\\\": \\\"\\\\u7b56\\\\u75650\\\", \\\"action\\\": \\\"archiving\\\", \\\"ext_parameters\\\": {\\\"qos_id\\\": \\\"\\\", \\\"storage_id\\\": \\\"90c0c3c8e8a6494ca3702e8b3c387001\\\", \\\"archive_target_type\\\": 1, \\\"archiving_scope\\\": \\\"latest\\\", \\\"specified_scope\\\": null, \\\"network_access\\\": false, \\\"auto_retry\\\": false, \\\"auto_retry_times\\\": null, \\\"auto_retry_wait_minutes\\\": null, \\\"delete_import_copy\\\": null, \\\"protocol\\\": 7}, \\\"retention\\\": {\\\"retention_type\\\": 1, \\\"duration_unit\\\": null, \\\"retention_duration\\\": null}, \\\"schedule\\\": {\\\"trigger\\\": 2, \\\"interval\\\": null, \\\"interval_unit\\\": null, \\\"start_time\\\": null, \\\"window_start\\\": null, \\\"window_end\\\": null, \\\"days_of_month\\\": null, \\\"days_of_year\\\": null, \\\"trigger_action\\\": null, \\\"days_of_week\\\": null}, \\\"type\\\": \\\"archiving\\\"}], \\\"resource_count\\\": null, \\\"archival_count\\\": null, \\\"replication_count\\\": null}\",\n"
                + "\t\t\t\"user_id\": \"88a94c476f12a21e016f12a246e50009\",\n" + "\t\t\t\"is_archived\": false,\n"
                + "\t\t\t\"is_replicated\": false,\n" + "\t\t\t\"detail\": null,\n" + "\t\t\t\"gn\": 2,\n"
                + "\t\t\t\"prev_copy_id\": \"490bba10-9566-4e6c-8050-6cf014a2bea4\",\n"
                + "\t\t\t\"next_copy_id\": \"293483cb-5137-4be2-b26a-00b86200378f\",\n" + "\t\t\t\"prev_copy_gn\": 1,\n"
                + "\t\t\t\"next_copy_gn\": 3\n" + "\t\t}";
        final Copy copy = JSONObject.toBean(copyStr, Copy.class);
        copy.setProperties(mockCopyPropertiesRepositories());
        return copy;
    }

    public static Copy mockHcsCopy() {
        String copyStr = "{\n" + "    \"uuid\": \"2107a05b-edeb-4e17-bcf6-5eb23e412ba0\",\n"
            + "    \"chain_id\": \"4aa1cbc7-1f2c-4312-b381-cc8c80f961df\",\n"
            + "    \"timestamp\": \"1659689899000000\",\n" + "    \"display_timestamp\": \"2022-08-05T16:58:19\",\n"
            + "    \"deletable\": true,\n" + "    \"status\": \"Normal\",\n" + "    \"location\": \"Local\",\n"
            + "    \"backup_type\": 1,\n" + "    \"generated_by\": \"Backup\",\n"
            + "    \"generated_time\": \"2022-08-05T16:58:19\",\n" + "    \"features\": 2,\n"
            + "    \"indexed\": \"Unindexed\",\n" + "    \"generation\": 1,\n" + "    \"parent_copy_uuid\": null,\n"
            + "    \"retention_type\": 2,\n" + "    \"retention_duration\": 10,\n" + "    \"duration_unit\": \"d\",\n"
            + "    \"expiration_time\": \"2022-08-15T16:58:19\",\n"
            + "    \"properties\": \"{\\\"snapshots\\\": [{\\\"id\\\": \\\"153@2107a05b-edeb-4e17-bcf6-5eb23e412ba0\\\", \\\"parentName\\\": \\\"CloudHost_66a5e75c-2728-4da3-8b79-57ea0d0fe037\\\"}], \\\"copyVerifyFile\\\": \\\"true\\\", \\\"verifyStatus\\\": \\\"0\\\", \\\"repositories\\\": [{\\\"type\\\": 1, \\\"protocol\\\": 5}, {\\\"type\\\": 2, \\\"protocol\\\": 5}], \\\"sub_object\\\": [], \\\"dataBeforeReduction\\\": 4274497, \\\"dataAfterReduction\\\": 1639908, \\\"multiFileSystem\\\": \\\"false\\\", \\\"volList\\\": [{\\\"bootable\\\": \\\"true\\\", \\\"datastore\\\": {\\\"dcMoRef\\\": \\\"\\\", \\\"extendInfo\\\": \\\"{\\\\\\\"volId\\\\\\\":\\\\\\\"133\\\\\\\",\\\\\\\"volName\\\\\\\":\\\\\\\"ecs-4d45-0008-volume-0000\\\\\\\",\\\\\\\"volWwn\\\\\\\":\\\\\\\"658f987100b749bcc5d44c8800000085\\\\\\\"}\\\\n\\\", \\\"ip\\\": \\\"\\\", \\\"moRef\\\": \\\"2102351NPT10J3000001\\\", \\\"name\\\": \\\"\\\", \\\"poolId\\\": \\\"-168868144\\\", \\\"port\\\": \\\"\\\", \\\"type\\\": \\\"OceanStorV5\\\", \\\"volumeName\\\": \\\"\\\"}, \\\"extendInfo\\\": \\\"\\\", \\\"metadata\\\": \\\"\\\", \\\"moRef\\\": \\\"feb072d5-4b20-419a-ad28-183161cc75a6\\\", \\\"name\\\": \\\"ecs-4d45-0008-volume-0000\\\", \\\"slotId\\\": \\\"\\\", \\\"type\\\": \\\"business_type_01\\\", \\\"uuid\\\": \\\"feb072d5-4b20-419a-ad28-183161cc75a6\\\", \\\"vmMoRef\\\": \\\"66a5e75c-2728-4da3-8b79-57ea0d0fe037\\\", \\\"volSizeInBytes\\\": 10737418240}], \\\"verifyTime\\\": \\\"2022-08-05T19:46:26.798\\\"}\",\n"
            + "    \"resource_id\": \"66a5e75c-2728-4da3-8b79-57ea0d0fe037\",\n"
            + "    \"resource_name\": \"ecs-4d45-0008\",\n" + "    \"resource_type\": \"CloudHost\",\n"
            + "    \"resource_sub_type\": \"HCSCloudHost\",\n"
            + "    \"resource_location\": \"test01/huangrong/西南/sc-cd-1_test/ecs-4d45-0008\",\n"
            + "    \"resource_status\": \"EXIST\",\n"
            + "    \"resource_properties\": \"{\\\"name\\\":\\\"ecs-4d45-0008\\\",\\\"path\\\":\\\"test01/huangrong/西南/sc-cd-1_test/ecs-4d45-0008\\\",\\\"root_uuid\\\":\\\"17ca9e5c-d87a-422d-8c51-2bc418952adc\\\",\\\"parent_name\\\":\\\"sc-cd-1_test\\\",\\\"parent_uuid\\\":\\\"f65f4acf-95d7-3823-aa57-435fc89efdab\\\",\\\"children_uuids\\\":null,\\\"type\\\":\\\"CloudHost\\\",\\\"sub_type\\\":\\\"HCSCloudHost\\\",\\\"uuid\\\":\\\"66a5e75c-2728-4da3-8b79-57ea0d0fe037\\\",\\\"created_time\\\":\\\"2022-08-04T15:52:42.691000\\\",\\\"ext_parameters\\\":{\\\"agents\\\":\\\"08838b64-7902-4c5d-87c7-da9e4159d5c2\\\",\\\"disk_info\\\":[]},\\\"authorized_user\\\":null,\\\"user_id\\\":null,\\\"version\\\":null,\\\"sla_id\\\":\\\"00802861-9a98-42f7-8dec-cbed090a1038\\\",\\\"sla_name\\\":\\\"CopyVerify_lj\\\",\\\"sla_status\\\":true,\\\"sla_compliance\\\":null,\\\"protection_status\\\":1,\\\"environment_uuid\\\":\\\"17ca9e5c-d87a-422d-8c51-2bc418952adc\\\",\\\"environment_name\\\":\\\"test01\\\",\\\"environment_endpoint\\\":\\\"demo.com\\\",\\\"environment_os_type\\\":null,\\\"environment_type\\\":\\\"HCS\\\",\\\"environment_sub_type\\\":\\\"HCSContainer\\\",\\\"environment_is_cluster\\\":\\\"False\\\",\\\"environment_os_name\\\":null,\\\"extendInfo\\\":{\\\"host\\\":\\\"{\\\\\\\"diskInfo\\\\\\\":[{\\\\\\\"architecture\\\\\\\":\\\\\\\"x86_64\\\\\\\",\\\\\\\"attr\\\\\\\":\\\\\\\"VDB\\\\\\\",\\\\\\\"id\\\\\\\":\\\\\\\"feb072d5-4b20-419a-ad28-183161cc75a6\\\\\\\",\\\\\\\"lunWWN\\\\\\\":\\\\\\\"658f987100b749bcc5d44c8800000085\\\\\\\",\\\\\\\"mode\\\\\\\":\\\\\\\"true\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"ecs-4d45-0008-volume-0000\\\\\\\",\\\\\\\"size\\\\\\\":\\\\\\\"10\\\\\\\"}],\\\\\\\"id\\\\\\\":\\\\\\\"66a5e75c-2728-4da3-8b79-57ea0d0fe037\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"ecs-4d45-0008\\\\\\\",\\\\\\\"projectId\\\\\\\":\\\\\\\"e38d227edcce4631be20bfa5aad7130b\\\\\\\",\\\\\\\"regionId\\\\\\\":\\\\\\\"sc-cd-1\\\\\\\",\\\\\\\"status\\\\\\\":\\\\\\\"ACTIVE\\\\\\\"}\\\\n\\\"}}\",\n"
            + "    \"resource_environment_name\": \"test01\",\n" + "    \"resource_environment_ip\": \"demo.com\",\n"
            + "    \"sla_name\": \"CopyVerify_lj\",\n"
            + "    \"sla_properties\": \"{\\\"uuid\\\": \\\"00802861-9a98-42f7-8dec-cbed090a1038\\\", \\\"name\\\": \\\"CopyVerify_lj\\\", \\\"created_time\\\": \\\"2022-08-05T10:56:30.484+08:00\\\", \\\"type\\\": 1, \\\"application\\\": \\\"HCSCloudHostS\\\", \\\"policy_list\\\": [{\\\"uuid\\\": \\\"0479cd18-33bd-48b1-afac-94ec77d33081\\\", \\\"name\\\": \\\"\\\\u5168\\\\u91cf01\\\", \\\"type\\\": \\\"backup\\\", \\\"action\\\": \\\"full\\\", \\\"retention\\\": {\\\"retention_type\\\": 2, \\\"retention_duration\\\": 10, \\\"duration_unit\\\": \\\"d\\\", \\\"daily_copies\\\": null, \\\"weekly_copies\\\": null, \\\"monthly_copies\\\": null, \\\"yearly_copies\\\": null}, \\\"schedule\\\": {\\\"trigger\\\": 1, \\\"interval\\\": 23, \\\"interval_unit\\\": \\\"h\\\", \\\"start_time\\\": \\\"2022-08-04T00:00:00\\\", \\\"window_start\\\": \\\"00:00:00\\\", \\\"window_end\\\": \\\"03:03:03\\\", \\\"days_of_week\\\": null, \\\"days_of_month\\\": null, \\\"days_of_year\\\": null, \\\"trigger_action\\\": null}, \\\"ext_parameters\\\": {\\\"source_deduplication\\\": false, \\\"alarm_after_failure\\\": false, \\\"deduplication\\\": false, \\\"copy_verify\\\": true, \\\"auto_retry\\\": true, \\\"auto_retry_times\\\": 3, \\\"auto_retry_wait_minutes\\\": 5}, \\\"active\\\": true, \\\"is_active\\\": true}], \\\"resource_count\\\": null, \\\"archival_count\\\": null, \\\"replication_count\\\": null, \\\"is_global\\\": false}\",\n"
            + "    \"user_id\": null,\n" + "    \"is_archived\": false,\n" + "    \"is_replicated\": false,\n"
            + "    \"detail\": null,\n" + "    \"name\": \"ecs-4d45-0008_1659689899\",\n"
            + "    \"storage_id\": \"\",\n" + "    \"gn\": 5,\n"
            + "    \"prev_copy_id\": \"a789e63d-06a7-4743-87af-ab4035041899\",\n" + "    \"next_copy_id\": null,\n"
            + "    \"prev_copy_gn\": 4,\n" + "    \"next_copy_gn\": null\n" + "}";
        return JSONObject.toBean(copyStr, Copy.class);
    }

    private static String mockCopyPropertiesRepositories() {
        List<BaseStorageRepository> repositories = new ArrayList<>();
        repositories.add(RepositoryMocker.mockBaseStorageRepository("1111", RepositoryProtocolEnum.NATIVE_NFS,
                RepositoryTypeEnum.DATA));
        repositories.add(
                RepositoryMocker.mockBaseStorageRepository("2222", RepositoryProtocolEnum.S3, RepositoryTypeEnum.DATA));
        JSONObject jsonObject = new JSONObject();
        jsonObject.set("aaa", "1234");
        jsonObject.set("bbb", new Endpoint("12.13.14.15", 9999));
        jsonObject.set(CopyPropertiesKeyConstant.KEY_REPOSITORIES, repositories);
        return jsonObject.toString();
    }

}
