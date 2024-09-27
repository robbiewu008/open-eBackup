/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model;

import lombok.Getter;
import lombok.Setter;

/**
 * 解析hcs token返回体
 *
 * @author y30021475
 * @since 2023-08-01
 */
@Getter
@Setter
public class HcsTokenResponse {
    private HcsToken token;
}
