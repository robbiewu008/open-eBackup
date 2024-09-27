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
package openbackup.system.base.common.process;

import lombok.Data;

import java.util.ArrayList;
import java.util.List;

/**
 * ProcessResult
 *
 * @author w00493811
 * @since 2021-08-17
 */
@Data
public class ProcessResult {
    /**
     * 缓存最大数量
     */
    public static final int CACHE_MAX_SIZE = 32768;

    /**
     * 缓存初始数量
     */
    private static final int CACHE_INT_SIZE = 16;

    /**
     * 结果编码
     */
    private int exitCode = Integer.MIN_VALUE;

    /**
     * 标准输出
     */
    private List<String> outputList = new ArrayList<>(CACHE_INT_SIZE);

    /**
     * 错误输出
     */
    private List<String> errorsList = new ArrayList<>(CACHE_INT_SIZE);

    /**
     * 增加标准输出
     *
     * @param line 标准输出
     */
    public void addOutput(String line) {
        if (outputList.size() < CACHE_MAX_SIZE) {
            outputList.add(line);
        }
    }

    /**
     * 增加错误输出
     *
     * @param line 错误输出
     */
    public void addErrors(String line) {
        if (errorsList.size() < CACHE_MAX_SIZE) {
            errorsList.add(line);
        }
    }

    /**
     * 是否结果OK
     *
     * @return 是否结果OK
     */
    public boolean isOk() {
        return getExitCode() == 0;
    }

    /**
     * 是否结果NOK
     *
     * @return 是否结果NOK
     */
    public boolean isNok() {
        return getExitCode() != 0;
    }
}
