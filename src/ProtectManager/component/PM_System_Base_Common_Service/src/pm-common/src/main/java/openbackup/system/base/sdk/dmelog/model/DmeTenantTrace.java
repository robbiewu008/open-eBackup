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
package openbackup.system.base.sdk.dmelog.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;
import lombok.ToString;

import java.io.Serializable;

/**
 * DME运营面租户日志
 *
 */
@Getter
@Setter
@ToString
public class DmeTenantTrace implements Serializable {
    private static final long serialVersionUID = 1245186589545376257L;

    /**
     * 操作英文名称。
     * 1~64个字符，且满足正则表达^((?!.*(\b|\t|\f|\r|\n|[\u0000-\u001f]|&lt;|&gt;)).)*$
     */
    private String operation;

    /**
     * 操作中文名称。
     * 1~64个字符，且满足正则表达^((?!.*(\b|\t|\f|\r|\n|[\u0000-\u001f]|&lt;|&gt;)).)*$
     */
    @JsonProperty("operation_cn")
    private String operationCn;

    /**
     * 日志级别，取值范围：WARNING（提示） ，MINOR（一般）， RISK（危险）。
     */
    private DmeLogLevelEnum level;

    /**
     * 来源英文名称。
     * 1~32个字符，且满足正则表达^((?!.*(\b|\t|\f|\r|\n|[\u0000-\u001f]|&lt;|&gt;)).)*$
     */
    private String source;

    /**
     * 来源中文名称。
     * 1~32个字符，且满足正则表达^((?!.*(\b|\t|\f|\r|\n|[\u0000-\u001f]|&lt;|&gt;)).)*$
     */
    @JsonProperty("source_cn")
    private String sourceCn;

    /**
     * 操作对象英文名称。
     * 1~32个字符，且满足正则表达^((?!.*(\b|\t|\f|\r|\n|[\u0000-\u001f]|&lt;|&gt;)).)*$
     */
    @JsonProperty("target_obj_cn")
    private String targetObjCn;

    /**
     * 操作对象中文名称。
     * 1~32个字符，且满足正则表达^((?!.*(\b|\t|\f|\r|\n|[\u0000-\u001f]|&lt;|&gt;)).)*$
     */
    @JsonProperty("target_obj")
    private String targetObj;

    /**
     * 操作日志详情。
     * 1~1024个字符，且满足正则表达^((?!.*(\b|\t|\f|\r|\n|[\u0000-\u001f]|&lt;|&gt;)).)*$
     */
    private String detail;

    /**
     * 操作日志详情。
     * 1~1024个字符，且满足正则表达^((?!.*(\b|\t|\f|\r|\n|[\u0000-\u001f]|&lt;|&gt;)).)*$
     */
    @JsonProperty("detail_cn")
    private String detailCn;

    /**
     * 操作结果： SUCCESSFUL,FAILURE, PARTIAL_SUCCESS。
     */
    private DmeLogResultEnum result;

    /**
     * 操作执行时间。
     */
    private long time;

    /**
     * 资源集ID。
     * 1~32个字符，且满足正则表达^((?!.*(\b|\t|\f|\r|\n|[\u0000-\u001f]|&lt;|&gt;)).)*$
     */
    @JsonProperty("project_id")
    private String projectId;

    /**
     * 资源集名称。
     * 1~64个字符，且满足正则表达^((?!.*(\b|\t|\f|\r|\n|[\u0000-\u001f]|&lt;|&gt;)).)*$
     */
    @JsonProperty("project_name")
    private String projectName;

    /**
     * 用户ID。
     */
    @JsonProperty("user_id")
    private String userId;

    /**
     * 用户名称。
     */
    @JsonProperty("user_name")
    private String userName;

    /**
     * 操作来源ip。
     */
    @JsonProperty("source_ip")
    private String sourceIp;
}
