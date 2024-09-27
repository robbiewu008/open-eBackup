/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.config.business.initialize.beans;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 文件系统信息
 *
 * @author w00493811
 * @since 2021-04-05
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class StorageFileSystemBean {
    /**
     * 主键自增
     */
    private long id;

    /**
     * 系统名称
     */
    private String name;

    /**
     * 系统路径
     */
    private String path;

    /**
     * 系统位置
     */
    private String site;

    /**
     * 存储类型
     */
    private String type;

    /**
     * 系统分区
     */
    private String zone;

    /**
     * 创建时间
     */
    private String date;

    /**
     * 随机编码
     */
    private String code;
}
