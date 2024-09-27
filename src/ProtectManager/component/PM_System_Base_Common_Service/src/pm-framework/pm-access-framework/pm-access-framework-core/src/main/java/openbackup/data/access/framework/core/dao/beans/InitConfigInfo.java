/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.data.access.framework.core.dao.beans;

import com.baomidou.mybatisplus.annotation.FieldStrategy;
import com.baomidou.mybatisplus.annotation.TableField;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 初始化配置，与数据库对应
 *
 * @author l00347293
 * @since 2020-12-21
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class InitConfigInfo {
    /**
     * 初始化类型
     */
    private String initType;

    /**
     * 初始化值
     */
    private String initValue;

    /**
     * 初始化配置创建时间
     */
    @TableField(updateStrategy = FieldStrategy.IGNORED)
    private Long createTime;

    /**
     * esn
     */
    private String esn;

    public InitConfigInfo(String initType, String initValue) {
        this.initType = initType;
        this.initValue = initValue;
    }

    /**
     * 三参构造函数
     *
     * @param initType 类型
     * @param initValue 值
     * @param createTime 创建时间
     */
    public InitConfigInfo(String initType, String initValue, Long createTime) {
        this(initType, initValue);
        this.createTime = createTime;
    }
}