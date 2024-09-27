/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.copy.model;

import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

/**
 * 备份信息模型
 *
 * @author t00482481
 * @since 2020-07-02
 */
@Data
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
public class Copy extends CopyInfo {
    private String uuid;

    private int amount;

    private int gn;

    private String prevCopyId;

    private String nextCopyId;

    private int prevCopyGn;

    private int nextCopyGn;

    private String deviceEsn;

    private String storageUnitName;
}
