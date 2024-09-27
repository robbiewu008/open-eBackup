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
package openbackup.data.protection.access.provider.sdk.restore;

import openbackup.data.protection.access.provider.sdk.base.Filter;
import openbackup.data.protection.access.provider.sdk.base.Parameter;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * Restore Object
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2020-06-17
 */
@Data
public class RestoreObject {
    @JsonProperty("request_id")
    private String requestId;

    // 副本Id
    @JsonProperty("copy_id")
    private String copyId;

    // 副本所对应保护对象的类型，如Oracle/FILESETS等等
    @JsonProperty("object_type")
    private String objectType;

    // 恢复位置，原位置还是新位置
    @JsonProperty("restore_location")
    private String restoreLocation;

    // 恢复的类型，普通恢复/及时恢复
    @JsonProperty("restore_type")
    private String restoreType;

    // 副本生成方式
    @JsonIgnore
    private String copyGeneratedBy = "Backup";

    private List<Filter> filters;

    // 恢复参数
    private List<Parameter> parameters;

    // 恢复对象
    private List<String> restoreObjects;

    // 恢复目标
    @JsonProperty("restore_target")
    private RestoreTarget target;

    // 导出文件id
    @JsonProperty("record_id")
    private String recordId;
}
