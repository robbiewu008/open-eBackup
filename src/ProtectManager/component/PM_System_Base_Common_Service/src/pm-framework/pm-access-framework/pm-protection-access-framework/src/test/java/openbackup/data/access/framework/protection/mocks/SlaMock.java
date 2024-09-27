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

/**
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/1/18
 **/
public class SlaMock {
    public static String getSla(){
        return "{\"uuid\":\"\",\"name\":\"testaq\",\"type\":1,\"application\":\"K8S-MySQL-dataset\",\"policy_list\":[{\"uuid\":\"\",\"name\":\"full\",\"type\":\"backup\",\"action\":\"full\",\"retention\":{\"retention_type\":2,\"retention_duration\":1,\"duration_unit\":\"d\"},\"schedule\":{\"trigger\":1,\"interval\":1,\"interval_unit\":\"h\",\"start_time\":\"2021-08-01\",\"window_start\":\"00:00:00\",\"window_end\":\"23:00:00\"},\"ext_parameters\":{\"qos_id\":\"\",\"detect_ransomware\":\"true\",\"auto_retry\":\"true\",\"auto_retry_times\":3,\"auto_retry_wait_minutes\":5}},{\"uuid\":\"\",\"name\":\"difference_increment\",\"type\":\"backup\",\"action\":\"difference_increment\",\"retention\":{\"retention_type\":2,\"retention_duration\":1,\"duration_unit\":\"d\"},\"schedule\":{\"trigger\":1,\"interval\":1,\"interval_unit\":\"h\",\"start_time\":\"2021-08-12\",\"window_start\":\"01:00:00\",\"window_end\":\"22:00:00\"},\"ext_parameters\":{\"qos_id\":\"\",\"detect_ransomware\":\"true\",\"auto_retry\":\"true\",\"auto_retry_times\":3,\"auto_retry_wait_minutes\":5}},{\"uuid\":\"\",\"name\":\"log\",\"type\":\"backup\",\"action\":\"log\",\"retention\":{\"retention_type\":2,\"retention_duration\":1,\"duration_unit\":\"d\"},\"schedule\":{\"trigger\":1,\"interval\":5,\"interval_unit\":\"m\",\"start_time\":\"2021-08-30 14:10:36\"},\"ext_parameters\":{\"qos_id\":\"\",\"detect_ransomware\":\"true\",\"auto_retry\":\"true\",\"auto_retry_times\":3,\"auto_retry_wait_minutes\":5}},{\"uuid\":\"\",\"name\":\"策略1\",\"type\":\"archiving\",\"action\":\"archiving\",\"ext_parameters\":{\"qos_id\":\"\",\"storage_id\":\"8c1d15372e134bcf9520d71e94a33b57\",\"archiving_scope\":\"latest\",\"network_access\":\"false\",\"auto_retry\":\"true\",\"auto_retry_times\":3,\"archive_target_type\":1,\"auto_retry_wait_minutes\":5},\"retention\":{\"retention_type\":2,\"retention_duration\":1,\"duration_unit\":\"d\"},\"schedule\":{\"trigger\":1,\"interval\":1,\"interval_unit\":\"h\",\"start_time\":\"2021-08-30 03:10:47\"}},{\"uuid\":\"\",\"name\":\"策略1\",\"action\":\"replication\",\"type\":\"replication\",\"ext_parameters\":{\"qos_id\":\"\",\"external_system_id\":\"2\"},\"retention\":{\"retention_type\":2,\"retention_duration\":1,\"duration_unit\":\"d\"},\"schedule\":{\"trigger\":1,\"interval\":1,\"interval_unit\":\"h\",\"start_time\":\"2021-08-30 08:10:58\"}}]}";
    }

    public static String getArchiveS3Policy() {
        return "{\"uuid\":\"fbc4dd02-81f5-46c8-a043-cf53ccae4074\",\"name\":\"策略0\",\"action\":\"archiving\",\"ext_parameters\":{\"qos_id\":\"a24420e1-f6ca-4afc-acee-36e26a7a3d0d\",\"storage_id\":\"6351d7a14ec1463c95c7f42aaab5ceb8\",\"protocol\":2,\"archive_target_type\":1,\"archiving_scope\":\"latest\",\"specified_scope\":null,\"network_access\":false,\"auto_retry\":true,\"auto_retry_times\":3,\"auto_retry_wait_minutes\":5,\"delete_import_copy\":null},\"retention\":{\"retention_type\":2,\"duration_unit\":\"d\",\"retention_duration\":55},\"schedule\":{\"trigger\":2,\"interval\":null,\"interval_unit\":null,\"start_time\":null,\"window_start\":null,\"window_end\":null,\"days_of_month\":null,\"days_of_year\":null,\"trigger_action\":null,\"days_of_week\":null},\"type\":\"archiving\"}";
    }

    public static String getArchiveTapePolicy() {
        return "{\"uuid\":\"fbc4dd02-81f5-46c8-a043-cf53ccae4074\",\"name\":\"策略0\",\"action\":\"archiving\",\"ext_parameters\":{\"qos_id\":\"a24420e1-f6ca-4afc-acee-36e26a7a3d0d\",\"storage_id\":\"6351d7a14ec1463c95c7f42aaab5ceb8\",\"protocol\":7,\"archive_target_type\":1,\"archiving_scope\":\"latest\",\"specified_scope\":null,\"network_access\":false,\"auto_retry\":true,\"auto_retry_times\":3,\"auto_retry_wait_minutes\":5,\"delete_import_copy\":null},\"retention\":{\"retention_type\":2,\"duration_unit\":\"d\",\"retention_duration\":55},\"schedule\":{\"trigger\":2,\"interval\":null,\"interval_unit\":null,\"start_time\":null,\"window_start\":null,\"window_end\":null,\"days_of_month\":null,\"days_of_year\":null,\"trigger_action\":null,\"days_of_week\":null},\"type\":\"archiving\"}";
    }
}
