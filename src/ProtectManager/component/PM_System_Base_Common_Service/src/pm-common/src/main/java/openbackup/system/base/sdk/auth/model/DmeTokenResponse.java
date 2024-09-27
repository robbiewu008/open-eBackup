/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model;

import lombok.Data;

/**
 * 解析dme token返回体
 *
 * @author z30062305
 * @since 2024-07-30
 */
@Data
public class DmeTokenResponse {
    private DmeToken token;
}
