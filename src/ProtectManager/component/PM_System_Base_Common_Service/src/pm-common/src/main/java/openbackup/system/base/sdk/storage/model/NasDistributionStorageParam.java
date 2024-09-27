/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.storage.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

/**
 * nas分布式存储库查询参数
 *
 * @author nwx1077006
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-16
 */
@NoArgsConstructor
@AllArgsConstructor
@Data
public class NasDistributionStorageParam {
    private String name;

    private Integer pageNo = 0;

    private Integer pageSize = 10;

    private List<String> unitIds;

    private String userId;

    private String deviceType;
}
