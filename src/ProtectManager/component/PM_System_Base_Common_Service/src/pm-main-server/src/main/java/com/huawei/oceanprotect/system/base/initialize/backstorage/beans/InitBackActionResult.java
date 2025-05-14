/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.backstorage.beans;

import com.huawei.oceanprotect.system.base.initialize.backstorage.enums.InitBackActionResultCode;

import lombok.Data;
import lombok.NoArgsConstructor;
import lombok.ToString;

import java.util.LinkedList;
import java.util.List;

/**
 * 初始化动作结果
 *
 * @author w00493811
 * @since 2020-12-28
 */
@Data
@ToString
@NoArgsConstructor
public class InitBackActionResult {
    /**
     * 初始化动作错误描述列表
     */
    private List<InitBackActionResultDesc> actionResults = new LinkedList<>();

    /**
     * 默认构造函数
     *
     * @param code 编码
     * @param desc 描述
     */
    public InitBackActionResult(InitBackActionResultCode code, String desc) {
        actionResults.add(new InitBackActionResultDesc(code, desc));
    }

    /**
     * 增加动作错误
     *
     * @param code 动作结果编码
     * @param desc 动作结果描述
     * @return 自身
     */
    public InitBackActionResult addInitBackActionResult(InitBackActionResultCode code, String desc) {
        actionResults.add(new InitBackActionResultDesc(code, desc));
        return this;
    }

    /**
     * 增加动作结果
     *
     * @param initBackActionResult 动作结果
     * @return 自身
     */
    public InitBackActionResult addInitBackActionResult(InitBackActionResult initBackActionResult) {
        actionResults.addAll(initBackActionResult.getActionResults());
        return this;
    }

    /**
     * 初始化是否OK
     *
     * @return 是否OK
     */
    public boolean isOkay() {
        for (InitBackActionResultDesc desc : actionResults) {
            if (!desc.getCode().isOkay()) {
                return false;
            }
        }
        return true;
    }
}
