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
package openbackup.system.base.common.utils.json;

import static org.assertj.core.api.BDDAssertions.then;

import com.google.common.base.CaseFormat;
import openbackup.system.base.common.utils.JSONObject;

import org.junit.Assert;
import org.junit.Test;

/**
 * JSONObjectCovert工具类测试用例集合
 *
 **/
public class JSONObjectCovertTest {

    /**
     * 用例名称：验证JSONObject中key从小写下划线转为小写驼峰成功<br/>
     * 前置条件：无<br/>
     * check点：1.转换对象不为空 2.对应属性转换成功<br/>
     */
    @Test
    public void should_success_when_test_covertLowerUnderscoreKeyToLowerCamel_given_lower_underscore_data(){
        // Given
        JSONObject mockSource = new JSONObject();
        mockSource.put("resource_id", "aaaaa");
        mockSource.put("resource_name", "bbbbb");
        // When
        final JSONObject target = JSONObjectCovert.covertLowerUnderscoreKeyToLowerCamel(mockSource);
        // Then
        then(target).isNotNull()
            .isNotEmpty()
            .hasFieldOrPropertyWithValue("resourceId", "aaaaa")
            .hasFieldOrPropertyWithValue("resourceName", "bbbbb");
    }

    /**
     * 用例名称：验证JSONObject中key从小写下划线转为小写驼峰成功<br/>
     * 前置条件：无<br/>
     * check点：1.转换对象不为空 2.对应属性转换成功 3.转换前后size相同 4.转换前后value属性没变
     */
    @Test
    public void should_same_after_transfer(){
        String resourceProperties="{\"name\":\"[special]\",\"path\":\"192.168.153.63\",\"root_uuid\":\"498g9b28-bd1e-44bb-ae60-c55b76ca4de3\",\"parent_name\":null,\"parent_uuid\":\"068e1ba7-4af2-3127-b7d0-6eb4fb0bf165\",\"children_uuids\":null,\"type\":\"Fileset\",\"sub_type\":\"Fileset\",\"uuid\":\"71fdf74230764ba9bb526f646c9e4f3a\",\"created_time\":\"2024-11-15T10:11:40.323000\",\"ext_parameters\":{\"first_backup_esn\":\"2102355AUP10X1110101\",\"last_backup_esn\":\"2102355AUP10X1110101\",\"priority_backup_esn\":\"2102355AUP10X1110101\",\"first_backup_target\":\"2a03a61b-ab53-444e-bbbe-0c72f7372326\",\"last_backup_target\":\"2a03a61b-ab53-444e-bbbe-0c72f7372326\",\"priority_backup_target\":\"2a03a61b-ab53-444e-bbbe-0c72f7372326\",\"failed_node_esn\":null,\"enable_security_archive\":false,\"worm_switch\":false,\"consistent_backup\":true,\"cross_file_system\":true,\"backup_nfs\":true,\"backup_smb\":false,\"channels\":1,\"sparse_file_detection\":false,\"backup_continue_with_files_backup_failed\":true,\"small_file_aggregation\":false,\"aggregation_file_size\":0,\"aggregation_file_max_size\":0,\"pre_script\":null,\"post_script\":null,\"failed_script\":null,\"archive_res_auto_index\":null,\"tape_archive_auto_index\":null,\"backup_res_auto_index\":false,\"snapshot_size_percent\":5},\"authorized_user\":null,\"user_id\":null,\"version\":null,\"sla_id\":\"fffa620b-5729-407a-a6a4-f7c0d6a06da6\",\"sla_name\":\"file_119_100_3\",\"sla_status\":true,\"sla_compliance\":null,\"protection_status\":1,\"environment_uuid\":\"498g9b28-bd1e-44bb-ae60-c55b76ca4de3\",\"environment_name\":\"localhost.localdomain\",\"environment_endpoint\":\"192.168.153.63\",\"environment_os_type\":\"linux\",\"environment_type\":\"Host\",\"environment_sub_type\":\"UBackupAgent\",\"environment_is_cluster\":\"False\",\"environment_os_name\":\"linux\",\"labelList\":null,\"extendInfo\":{\"templateName\":\"\",\"paths\":\"[{\\\"name\\\":\\\"/etc/resolv.conf\\\"}]\",\"filters\":\"[]\",\"templateId\":\"\"}}";
        JSONObject jsonObject = JSONObject.fromObject(resourceProperties);
        JSONObject targetJsonObject = JSONObjectCovert.covertLowerUnderscoreKeyToLowerCamel(
                jsonObject);
        Assert.assertEquals(jsonObject.size(), targetJsonObject.size());
        jsonObject.toMap(Object.class)
                .forEach(
                        (key, value) -> {
                            String lowerCamelKey = key;
                            // 如果key为下划线格式，将其转为小驼峰
                            if (key.contains("_")) {
                                lowerCamelKey = CaseFormat.LOWER_UNDERSCORE.to(CaseFormat.LOWER_CAMEL, key);
                            }
                            Assert.assertEquals(value,targetJsonObject.get(lowerCamelKey));
                        });
    }

}