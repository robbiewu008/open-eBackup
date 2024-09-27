/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.dee.model;

import lombok.Data;

import java.util.List;

/**
 * SourceScanCopy
 *
 * @author jwx701567
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-21
 */
@Data
public class FinegrainedRestoreCopy {
    private String uuid;

    private Integer gn;

    private String resourceId;

    private String generatedBy;

    private String chainId;

    private String resourceSubType;

    private String userId;

    private Boolean isIndexed;

    private Boolean isAggregation;

    private List<Snapshot> snapshots;

    private String resourcePlatform;

    private String sharePolicyType;

    /**
     * 设备esn
     */
    private String deviceEsn;

    /**
     * 存储单元Id
     */
    private String storageId;

    private String copyMetaData;
}
