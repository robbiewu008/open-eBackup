/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resourcegroup.resp;

import lombok.Getter;
import lombok.Setter;

/**
 * 资源组成员返回体
 *
 * @author c00631681
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-25
 */
@Getter
@Setter
public class ResourceGroupMemberVo {
    private String uuid;

    private String sourceId;

    private String sourceName;

    private String sourceType;

    private String sourceSubType;

    private String path;

    private String status;

    private String resourceGroupId;
}