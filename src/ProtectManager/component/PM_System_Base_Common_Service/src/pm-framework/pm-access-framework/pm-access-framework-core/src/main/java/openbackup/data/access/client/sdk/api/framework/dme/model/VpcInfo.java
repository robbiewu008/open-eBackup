/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.dme.model;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;

import lombok.Getter;
import lombok.Setter;

/**
 * op服务化VPC信息，循环依赖了
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-05-09
 */
@Getter
@Setter
@JsonIgnoreProperties(ignoreUnknown = true)
public class VpcInfo {
    private String vpcId;

    private String projectId;

    private String markId;
}
