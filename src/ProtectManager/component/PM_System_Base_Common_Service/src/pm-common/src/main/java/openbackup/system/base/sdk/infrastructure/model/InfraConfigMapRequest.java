/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.infrastructure.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * k8s configMap 对象
 *
 * @author fwx1022842
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023/2/7
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class InfraConfigMapRequest {
    /**
     * 命名空间
     */
    private String nameSpace;

    /**
     * configMap的名称
     */
    private String secretMap;

    /**
     * data中的key
     */
    private String secretKey;

    /**
     * data中的value
     */
    private String secretValue;
}
