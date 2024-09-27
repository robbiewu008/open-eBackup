/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.bean;

import openbackup.system.base.common.enums.AddressFamily;

import lombok.Getter;
import lombok.Setter;

/**
 * 网络基本信息
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/6/3
 */
@Getter
@Setter
public class NetworkBaseInfo {
    private String ip;
    private String mask;
    private AddressFamily ipType;
}
