/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.model;

import lombok.Data;

import java.util.List;

/**
 * 备份软件信息
 *
 * @author w00607005
 * @since 2023-07-18
 */
@Data
public class SoftwareVersion {
    /**
     * 备份软件型号
     */
    private String productModeString;

    /**
     * 备份软件版本
     */
    private String pointRelease;

    /**
     * 备份软件C版本
     */
    private String cVersion;

    /**
     * 备份软件支持的功能
     */
    private List<String> function;
}
