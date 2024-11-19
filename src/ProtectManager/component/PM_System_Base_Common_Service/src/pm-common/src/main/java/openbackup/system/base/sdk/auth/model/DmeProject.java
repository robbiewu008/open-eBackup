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
package openbackup.system.base.sdk.auth.model;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * Dme资源集对象属性
 *
 */
@Setter
@Getter
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class DmeProject {
    /**
     * 资源集ID
     */
    private String id;

    /**
     * 资源集名称
     */
    private String name;

    /**
     * 资源集描述
     */
    private String description;

    /**
     * 资源集使能开关
     */
    private boolean enabled;

    /**
     * domain id
     */
    private String domainId;

    /**
     * domain名称
     */
    private String domainName;

    /**
     * VDC id
     */
    private String vdcId;

    /**
     * VDC名称
     */
    private String vdcName;

    /**
     * 创建时间
     */
    private Long createTime;

    /**
     * 资源集关联地域
     */
    private List<DmeProjectRegion> regions;
}