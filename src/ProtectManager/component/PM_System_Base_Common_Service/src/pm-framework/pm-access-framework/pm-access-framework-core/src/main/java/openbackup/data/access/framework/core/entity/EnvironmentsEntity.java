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
package openbackup.data.access.framework.core.entity;

import com.baomidou.mybatisplus.annotation.TableField;
import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import lombok.Data;

/**
 * 环境Entity
 *
 */
@Data
@TableName("environments")
public class EnvironmentsEntity {
    @TableId(value = "UUID")
    private String uuid;

    @TableField(value = "ENDPOINT")
    private String endpoint;

    @TableField(value = "PORT")
    private String port;

    @TableField(value = "USER_NAME")
    private String userName;

    @TableField(value = "PASSWORD")
    private String password;

    @TableField(value = "LINK_STATUS")
    private String linkStatus;

    @TableField(value = "LOCATION")
    private String location;

    @TableField(value = "OS_TYPE")
    private String osType;

    @TableField(value = "OS_NAME")
    private String osName;

    @TableField(value = "IS_CLUSTER")
    private Boolean isCluster;

    @TableField(value = "SCAN_INTERVAL")
    private Integer scanInterval;

    @TableField(value = "CERT_NAME")
    private String certName;

    @TableField(value = "TIME_ZONE")
    private String timeZone;

    @TableField(value = "AGENT_VERSION")
    private String agentVersion;

    @TableField(value = "AGENT_TIMESTAMP")
    private String agentTimestamp;
}
