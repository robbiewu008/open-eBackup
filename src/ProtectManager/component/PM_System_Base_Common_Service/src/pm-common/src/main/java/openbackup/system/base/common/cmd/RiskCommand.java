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

import lombok.extern.slf4j.Slf4j;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.nio.charset.StandardCharsets;

/**
 * 危险的命令执行类
 *
 * @author fwx1022842
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023/9/26
 */
@Slf4j
public class RiskCommand {
    private static final int SUCCESS_CODE = 0;

    private static final long MAX_LENGTH = 8000L;

    private static final long MAX_TIMEOUT = 3000L;

    private final String[] items;

    private final String password;

    public RiskCommand(String[] items, String password) {
        this.items = items;
        this.password = password;
    }

    /**
     * 执行命令，获取返回值
     *
     * @param items 命令执行的每个元素
     * @param password 密码
     * @return 命令的返回值
     * @throws IOException IOException
     * @throws InterruptedException InterruptedException
     */
    public static String run(String[] items, String password) throws InterruptedException, IOException {
        final Process process = new RiskCommand(items, password).run();
        final int code = process.waitFor();
        if (code != SUCCESS_CODE) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "run cmd error. code: " + code);
        }
        final InputStream stream = process.getInputStream();
        StringBuilder value = new StringBuilder();
        String line;
        final long startTime = System.currentTimeMillis();
        try (BufferedReader bf = new BufferedReader(new InputStreamReader(stream, StandardCharsets.UTF_8))) {
            while ((line = bf.readLine()) != null) {
                // 计算是否读取数据的超时时间达到上限
                if (System.currentTimeMillis() - startTime > RiskCommand.MAX_TIMEOUT) {
                    throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "read data is time out");
                }
                // 计算是否读取数据是否达到上限
                if (line.length() > RiskCommand.MAX_LENGTH) {
                    throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "read data length is max");
                }
                value.append(line).append(System.lineSeparator());
            }
        }
        return value.toString();
    }

    /**
     * 执行脚本
     *
     * @return Process对象
     * @throws IOException IOException
     */
    private Process run() throws IOException {
        ProcessBuilder processBuilder = new ProcessBuilder(items);
        Process process = processBuilder.start();
        try (OutputStream outputStream = process.getOutputStream();
             OutputStreamWriter outputStreamWriter = new OutputStreamWriter(outputStream, StandardCharsets.UTF_8);
             BufferedWriter writer = new BufferedWriter(outputStreamWriter)) {
            writer.write(password);
            writer.newLine();
            writer.flush();
        } catch (IOException e) {
            throw new LegoCheckedException("write param error. " + e.getMessage());
        }
        return process;
    }
}