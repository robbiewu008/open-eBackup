/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.bean;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * NetWorkLogicIp
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-02-20
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class NetWorkLogicIp {
    private String ip;

    private String mask;
}
