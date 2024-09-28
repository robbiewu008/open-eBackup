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
package openbackup.system.base.common.cmd;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.thread.ThreadPoolTool;
import openbackup.system.base.common.utils.ExceptionUtil;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.Objects;

/**
 * Command
 *
 */
public class Command {
    private static final Logger log = LoggerFactory.getLogger(Command.class);

    private final String[] items;

    private String[] sensitiveParams;

    private boolean isPrintInputInfo = true;

    private boolean isPrintErrorInfo = true;

    /**
     * constructor
     *
     * @param items items
     * @throws LegoCheckedException 假设参数包含黑名单字符，抛出非法参数异常。
     */
    public Command(String... items) {
        this.items = Objects.requireNonNull(items);
        paramBlock(items);
    }

    /**
     * 敏感参数的构造函数
     *
     * @param items 参数列表
     * @param sensitiveParams 敏感参数
     * @throws LegoCheckedException 假设参数包含黑名单字符，抛出非法参数异常。
     */
    public Command(String[] items, String[] sensitiveParams) {
        this.items = items;
        this.sensitiveParams = sensitiveParams;
        paramBlock(items);
    }

    /**
     * 敏感参数的构造函数
     *
     * @param items 参数列表
     * @param sensitiveParams 敏感参数
     * @param isPrintInputInfo 是否打印输入流信息
     * @param isPrintErrorInfo 是否打印错误流信息
     */
    public Command(String[] items, String[] sensitiveParams, boolean isPrintInputInfo, boolean isPrintErrorInfo) {
        this.items = items;
        this.sensitiveParams = sensitiveParams;
        this.isPrintInputInfo = isPrintInputInfo;
        this.isPrintErrorInfo = isPrintErrorInfo;
        paramBlock(items);
    }

    /**
     * constructor
     *
     * @param isNeedBlock isNeedBlock 是否校验参数包含黑名单字符
     * @param items       items
     * @throws LegoCheckedException 假设参数包含黑名单字符，抛出非法参数异常。
     */
    public Command(boolean isNeedBlock, String... items) {
        this.items = Objects.requireNonNull(items);
        if (isNeedBlock) {
            paramBlock(items);
        }
    }

    /**
     * 参数拦截器，根据判断每个参数是否包含恶意字符，如果包含字符，则抛出异常。
     *
     * @param items Command参数列表
     * @throws LegoCheckedException 假设参数包含黑名单字符，抛出非法参数异常。
     */
    private static void paramBlock(String... items) {
        Arrays.stream(items).forEach((param) -> {
            for (String blackParam : EvilParams.getEvilParams()) {
                if (param.contains(blackParam)) {
                    log.error("Dangerous param in command");
                    throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Illegal param!");
                }
            }
        });
    }

    /**
     * run
     *
     * @param items items
     * @return result
     */
    public static int run(String... items) {
        return new Command(items).run();
    }

    /**
     * 执行脚本
     *
     * @return 状态码
     */
    public int run() {
        try {
            Process process = new ProcessBuilder(items).inheritIO().start();
            printMessage(process.getInputStream(), isPrintInputInfo);
            printMessage(process.getErrorStream(), isPrintErrorInfo);
            return process.waitFor();
        } catch (InterruptedException | IOException e) {
            throw new LegoCheckedException("run failed " + e.getMessage());
        }
    }

    /**
     * runWithSensitiveParams
     *
     * @param items items
     * @param sensitiveParams 敏感信息，通过输出流传参
     * @return result
     */
    public static int runWithSensitiveParams(String[] sensitiveParams, String... items) {
        if (sensitiveParams == null || sensitiveParams.length == 0) {
            throw new LegoCheckedException("sensitive params is null.");
        }
        return new Command(items, sensitiveParams, false, false).runWithSensitiveParams();
    }

    /**
     * 执行脚本
     *
     * @return 状态码
     */
    private int runWithSensitiveParams() {
        try {
            ProcessBuilder processBuilder = new ProcessBuilder(items);
            Process process = processBuilder.start();

            // 传参
            try (OutputStream outputStream = process.getOutputStream();
                OutputStreamWriter outputStreamWriter = new OutputStreamWriter(outputStream, StandardCharsets.UTF_8);
                BufferedWriter writer = new BufferedWriter(outputStreamWriter)) {
                for (String param : sensitiveParams) {
                    writer.write(param);
                    writer.newLine();
                    writer.flush();
                }
            } catch (IOException e) {
                log.error("write param in OutputStream error.", ExceptionUtil.getErrorMessage(e));
                throw new LegoCheckedException("write param error. " + e.getMessage());
            }
            printMessage(process.getInputStream(), isPrintInputInfo);
            printMessage(process.getErrorStream(), isPrintErrorInfo);
            return process.waitFor();
        } catch (InterruptedException | IOException e) {
            log.error("process run error.", ExceptionUtil.getErrorMessage(e));
            return 1;
        }
    }

    private static void printMessage(InputStream input, boolean isPrintInfo) {
        // 处理输入流，错误流
        ThreadPoolTool.getPool().execute(() -> {
            String line;
            try (InputStream inputStream = input;
                InputStreamReader inputStreamReader = new InputStreamReader(inputStream, StandardCharsets.UTF_8);
                BufferedReader bf = new BufferedReader(inputStreamReader)) {
                while ((line = bf.readLine()) != null) {
                    if (isPrintInfo) {
                        log.info(line);
                    }
                }
            } catch (IOException e) {
                log.error("print message exception", ExceptionUtil.getErrorMessage(e));
            }
        });
    }
}
