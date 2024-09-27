/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.constants;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 本地文件系统
 *
 * @author g30003063
 * @since 2021-12-14
 */
@Getter
@Setter
public class LocalFileSystem {
    /**
     * ID
     */
    private String id;

    /**
     * 名称
     */
    private String name;

    /**
     * 远程复制ID列表
     */
    private List<String> remoteReplicationIds;

    /**
     * 双活PairID列表
     */
    private List<String> hyperMetroPairIds;

    /**
     * NAS协议支持多种安全模式
     */
    private String securityStyle;

    /**
     * 文件系统类型
     */
    private String subType;

    /**
     * 是否是克隆文件系统
     */
    private boolean isCloneFs;
}
