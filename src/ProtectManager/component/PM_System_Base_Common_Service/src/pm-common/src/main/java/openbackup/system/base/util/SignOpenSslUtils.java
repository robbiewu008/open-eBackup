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
package openbackup.system.base.util;

import com.huawei.emeistor.kms.kmc.util.KmcHelper;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.cmd.Command;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.files.FileUtil;

import org.apache.commons.lang3.StringUtils;

import java.io.File;
import java.io.IOException;
import java.util.Arrays;

/**
 * SignOpenSslUtils
 *
 */
@Slf4j
public class SignOpenSslUtils {
    /**
     * 获取generate key
     *
     * @param privateKeyPath 私钥路径
     * @return generate key
     */
    public static String getGenerateKey(String privateKeyPath) {
        String pass = null;
        try {
            pass = FileUtil.readFileToString(privateKeyPath + Constants.SIGN_PASS);
        } catch (IOException e) {
            log.error("pass file not exist", ExceptionUtil.getErrorMessage(e));
        }
        return KmcHelper.getInstance().decrypt(pass);
    }

    /**
     * 加签
     *
     * @param filePath 文件路径
     * @param privateKeyPath 私钥路劲
     * @param signatureFilePath 签名存放路径
     */
    public static void sign(String filePath, String privateKeyPath, String signatureFilePath) {
        String[] sensitiveParams = new String[0];
        try {
            sensitiveParams = new String[] {filePath, signatureFilePath, getGenerateKey(privateKeyPath)};
            int errorCode = Command.runWithSensitiveParams(sensitiveParams, "sh", "/app/sign.sh", "sign");
            log.info("errorCode: {}", errorCode);
            if (errorCode != 0 || !new File(signatureFilePath).exists()) {
                throw new LegoCheckedException("generate sign file fail");
            }
        } finally {
            Arrays.asList(sensitiveParams).stream().forEach(StringUtil::clean);
        }
    }

    /**
     * 验签
     *
     * @param filePath 文件路径
     * @param signatureFilePath 签名路径
     * @param resultPath 结果路径
     * @return 验签结果
     */
    public static boolean verify(String filePath, String signatureFilePath, String resultPath) {
        String[] sensitiveParams = {resultPath, filePath, signatureFilePath};
        int errorCode = Command.runWithSensitiveParams(sensitiveParams, "sh", "/app/sign.sh", "verify");
        String result = StringUtils.EMPTY;
        try {
            result = FileUtil.readFileToString(resultPath);
        } catch (IOException e) {
            log.error("verify result file not exist");
        }
        log.info("result: {}, error code: {}", result, errorCode);

        // Verified OK 为命令输出，用contain是因为OK后面有特殊字符
        return result.contains("Verified OK") && errorCode == 0;
    }
}
