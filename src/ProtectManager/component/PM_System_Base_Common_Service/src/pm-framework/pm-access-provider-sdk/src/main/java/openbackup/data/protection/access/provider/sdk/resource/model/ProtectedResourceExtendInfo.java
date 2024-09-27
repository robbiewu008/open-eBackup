/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 */

package openbackup.data.protection.access.provider.sdk.resource.model;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * 资源扩展信息
 *
 * @author w30044259
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-08-10
 */
@Builder
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
public class ProtectedResourceExtendInfo {
    /**
     * uuid
     */
    private String uuid;

    /**
     * 资源ID
     */
    private String resourceId;

    /**
     * 扩展字段key
     */
    private String key;

    /**
     * 扩展字段value
     */
    private String value;
}
