/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.restore.v2;

import lombok.Getter;
import lombok.Setter;

/**
 * 恢复脚本
 *
 * @since 2022-09-15
 */
@Getter
@Setter
public class RestoreScript {
    private String preScript;

    private String postScript;

    private String failPostScript;
}