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
package openbackup.data.access.framework.core.model;

import com.fasterxml.jackson.annotation.JsonIgnore;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 副本资源查询条件
 *
 */
@Getter
@Setter
public class CopySummaryResourceCondition {
    /**
     * 资源名称，模糊匹配
     */
    private String resourceName;

    /**
     * 资源位置，模糊匹配
     */
    private String resourceLocation;

    /**
     * 资源子类型
     */
    private List<String> resourceSubType;

    /**
     * 资源状态
     */
    private List<String> resourceStatus;

    /**
     * 资源环境IP，模糊匹配
     */
    private String resourceEnvironmentIp;

    /**
     * 资源环境名称，模糊匹配
     */
    private String resourceEnvironmentName;

    /**
     * 资源ID列表
     */
    private List<String> resourceIds;

    private String indexed;

    private String gnLte;

    private String gnGte;

    private String deviceEsn;

    private String chainId;

    private List<String> generatedBy;

    /**
     * 复制副本SLA ID
     */
    private String protectedSlaId;

    /**
     * 复制副本SLA名称，模糊匹配
     */
    private String protectedSlaName;

    /**
     * 保护状态
     */
    private List<Boolean> protectedStatus;

    /**
     * 当前登录用户的域
     */
    @JsonIgnore
    private String domainId;

    /**
     * 当前登录用户
     */
    @JsonIgnore
    private String userId;
}
