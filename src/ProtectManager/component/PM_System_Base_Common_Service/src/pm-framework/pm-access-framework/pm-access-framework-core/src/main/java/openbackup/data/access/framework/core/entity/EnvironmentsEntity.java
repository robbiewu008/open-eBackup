/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.framework.core.entity;

import com.baomidou.mybatisplus.annotation.TableField;
import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import lombok.Data;

/**
 * 环境Entity
 *
 * @author w30042425
 * @since 2023-11-27
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
