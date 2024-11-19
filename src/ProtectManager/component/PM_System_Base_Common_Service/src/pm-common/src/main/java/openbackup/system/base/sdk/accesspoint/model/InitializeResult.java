/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.system.base.sdk.accesspoint.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;
import lombok.ToString;

import java.util.LinkedList;
import java.util.List;

/**
 * 初始化动作结果
 *
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
@ToString
public class InitializeResult {
    /**
     * 初始化动作错误描述列表
     */
    private List<InitializeResultDesc> actionResults = new LinkedList<>();

    /**
     * 带错误码构造函数
     *
     * @param initializeResultDesc 动作结果描述
     */
    public InitializeResult(InitializeResultDesc initializeResultDesc) {
        actionResults.add(initializeResultDesc);
    }

    /**
     * 增加动作错误
     *
     * @param initializeResultDesc 动作结果描述
     * @return 自身
     */
    public InitializeResult addActionResultDesc(InitializeResultDesc initializeResultDesc) {
        actionResults.add(initializeResultDesc);
        return this;
    }

    /**
     * 增加动作结果
     *
     * @param initializeResult 动作结果
     * @return 自身
     */
    public InitializeResult addActionError(InitializeResult initializeResult) {
        actionResults.addAll(initializeResult.getActionResults());
        return this;
    }

    /**
     * 初始化是否OK
     *
     * @return 是否OK
     */
    public boolean isOk() {
        for (InitializeResultDesc desc : actionResults) {
            if (!desc.getCode().isOk()) {
                return false;
            }
        }
        return true;
    }
}