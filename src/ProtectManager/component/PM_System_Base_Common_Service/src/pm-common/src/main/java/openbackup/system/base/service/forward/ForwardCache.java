/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.service.forward;

import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * function
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/4/5
 */
@Setter
@Getter
@NoArgsConstructor
public class ForwardCache {
    private String cacheKey;

    public ForwardCache(String cacheKey) {
        this.cacheKey = cacheKey;
    }
}
