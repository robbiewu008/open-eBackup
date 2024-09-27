/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.bean;

import lombok.Data;

import java.util.Set;

/**
 * 文件校验规则
 *
 * @author w00607005
 * @since 2023-10-05
 */
@Data
public class FileCheckRule {
    /**
     * 文件名要求长度
     */
    private int maxNameLength;

    /**
     * 文件大小
     */
    private long maxSize;

    /**
     * 允许后缀
     */
    private Set<String> allowedFormats;

    /**
     * 临时目录
     */
    private String tempPath;

    /**
     * 最大深度
     */
    private int maxDepth;

    /**
     * 解压缩后最大大小
     */
    private long maxUnZipSize;

    /**
     * 单个entrySize大小
     */
    private long maxEntrySize;

    /**
     * 压缩文件内文件数量
     */
    private int maxEntryNum;

    /**
     * 压缩包内文件白名单
     */
    private Set<String> zipWhiteList;
}
