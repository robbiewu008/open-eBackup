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

import openbackup.system.base.common.desensitization.SensitiveConfig;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;

import java.util.List;

/**
 * 敏感字段校验类
 *
 * @author t30028453
 * @version [OceanProtect A8000 1.2.1]
 * @since 2022-10-11
 */
@Slf4j
public class SensitiveValidateUtil {
    /**
     * 模糊匹配
     */
    private static final String RULE_FUZZ = "fuzz";

    /**
     * 模糊匹配的key列表
     */
    private static final List<String> fuzzRuleKeys = SensitiveConfig.getFuzzRuleKeys(RULE_FUZZ);

    /**
     * 验证是否包含了敏感字段
     *
     * @param toValidateStrs 待验证字段集合
     */
    public static void doValidate(List<String> toValidateStrs) {
        if (VerifyUtil.isEmpty(toValidateStrs)) {
            return ;
        }
        log.info("Sensitive field verification, size:{}", toValidateStrs.size());
        toValidateStrs.forEach(str -> check(str));
    }

    private static void check(String str) {
        if (CollectionUtils.isNotEmpty(fuzzRuleKeys) && fuzzRuleKeys.stream().anyMatch(str::contains)) {
            log.error("The sensitive field name is found. Please confirm.");
            throw new LegoCheckedException("The sensitive field name is found. Please confirm.");
        }
    }
}
