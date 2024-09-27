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
package openbackup.system.base.sdk.anti.model;

import lombok.Data;

import java.util.ArrayList;
import java.util.List;

/**
 * 防勒索策略返回对象
 *
 * @author nwx1077006
 * @since 2021-11-16
 */
@Data
public class AntiRansomwarePolicyRes {
    // 策略id
    private Integer id;

    // 策略名称
    private String policyName;

    // 策略描述
    private String description;

    // 集群id
    private Integer clusterId;

    // 数据源类型
    private String dataSourceType;

    // 资源子类型
    private String resourceSubType;

    // 资源列表
    private List<AntiRansomwarePolicyResourceRes> selectedResources = new ArrayList<>();

    // 调度计划
    private AntiRansomwareScheduleRes schedule;

    // 角色id
    private String roleId;

    // 租户数量
    private Long vStoreCount;
}