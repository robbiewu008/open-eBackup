/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
