/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.enums;

import lombok.Getter;

/**
 * 恢复模式
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/7/14
 */
public enum RestoreModeEnum {
    /**
     * 远端副本先下载到本地再恢复
     */
    DOWNLOAD_RESTORE("DownloadRestore"),

    /**
     * 远端副本直接恢复
     */
    REMOTE_RESTORE("RemoteRestore"),

    /**
     * 本地副本直接恢复
     */
    LOCAL_RESTORE("LocalRestore");

    @Getter
    private final String mode;

    /**
     * 构造方法
     *
     * @param mode 恢复模式
     */
    RestoreModeEnum(String mode) {
        this.mode = mode;
    }
}
