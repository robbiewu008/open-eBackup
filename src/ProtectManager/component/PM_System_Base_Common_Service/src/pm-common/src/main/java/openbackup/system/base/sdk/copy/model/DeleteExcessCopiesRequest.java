/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.copy.model;

import lombok.Getter;
import lombok.Setter;

/**
 * 删除资源多余副本请求体
 *
 * @author w00616953
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-15
 */
@Getter
@Setter
public class DeleteExcessCopiesRequest {
    /**
     * 副本保留数量
     */
    private int retentionQuantity;

    /**
     * 副本生成方式
     */
    private String generatedBy;

    /**
     * 用户id
     */
    private String userId;
}
