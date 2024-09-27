/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2024. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.Setter;

import java.util.Set;

/**
 * 资源授权类
 *
 * @author z00842230
 * @version [OceanProtect DataBackup 1.7.0]
 * @since 2024-05-17
 */
@Getter
@Setter
@AllArgsConstructor
public class ResourceSetAuthorization {
    private String roleId;

    private Set<String> resourceSetIds;
}