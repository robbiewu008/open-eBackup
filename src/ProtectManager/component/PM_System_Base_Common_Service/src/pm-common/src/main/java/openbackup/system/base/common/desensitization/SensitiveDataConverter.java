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
package openbackup.system.base.common.desensitization;

import ch.qos.logback.classic.pattern.MessageConverter;
import ch.qos.logback.classic.spi.ILoggingEvent;
import openbackup.system.base.common.utils.VerifyUtil;

import org.apache.commons.lang3.StringUtils;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Locale;

/**
 * 功能描述 日志敏感信息匿名
 *
 */
public class SensitiveDataConverter extends MessageConverter {
    /**
     * 模糊匹配
     */
    private static final String RULE_FUZZ = "fuzz";

    /**
     * 单词匹配
     */
    private static final String RULE_WORD = "word";

    /**
     * 是否开启过滤功能
     */
    private static final boolean IS_ENABLE = SensitiveConfig.isEnable();

    private static final List<Character> HEAD_SEPARATIONS = Collections.singletonList(' ');

    private static final List<Character> TAIL_SEPARATIONS = Arrays.asList(':', '：', '=');

    /**
     * 模糊匹配的key列表
     */
    private final List<String> fuzzRuleKeys = SensitiveConfig.getFuzzRuleKeys(RULE_FUZZ);

    /**
     * 单词匹配的key列表
     */
    private final List<String> allRuleKeys = SensitiveConfig.getFuzzRuleKeys(RULE_WORD);


    @Override
    public String convert(ILoggingEvent event) {
        String originLogMsg = event.getFormattedMessage();
        return invokeMsg(originLogMsg);
    }

    /**
     * 对敏感信息脱敏 + crlf过滤
     *
     * @param originMsg 原始日志消息
     * @return String 脱敏后 + crlf过滤后的日志消息
     */
    public String invokeMsg(String originMsg) {
        if (!IS_ENABLE) {
            return originMsg;
        }
        if (VerifyUtil.isEmpty(fuzzRuleKeys) && VerifyUtil.isEmpty(allRuleKeys)) {
            return originMsg;
        }
        String desensitizedMsg = desensitizeMsg(originMsg);
        return StringCrlfUtil.escapeCrlf(desensitizedMsg);
    }

    private String desensitizeMsg(String sensitiveMsg) {
        String tempSensitiveMsg = desensitizeMsgByAll(sensitiveMsg, allRuleKeys, true);
        return desensitizeMsgByAll(tempSensitiveMsg, fuzzRuleKeys, false);
    }

    private String desensitizeMsgByAll(String sensitiveMsg, List<String> sensitiveKeyAlls, boolean isAll) {
        String tempSensitiveMsg = sensitiveMsg;
        for (String sensitiveKey : sensitiveKeyAlls) {
            int index = -1;
            sensitiveKey = StringUtils.strip(sensitiveKey);
            index = tempSensitiveMsg.toLowerCase(Locale.ENGLISH)
                .indexOf(sensitiveKey.toLowerCase(Locale.ENGLISH), index + 1);
            if (index == -1) {
                continue;
            }

            if (isAll && !isAllMatch(tempSensitiveMsg, sensitiveKey, index)) {
                continue;
            }
            // 寻找值开始的下标， ':', '=', 中文'：'之后为值开始的地方
            int valueStart = getValueStartIndex(tempSensitiveMsg, index);
            // 寻找值结束的下标
            int valueEnd = tempSensitiveMsg.length();
            if (valueStart >= valueEnd) {
                continue;
            }
            tempSensitiveMsg = tempSensitiveMsg.substring(0, valueStart) + "******" + tempSensitiveMsg.substring(
                valueEnd);
        }
        return tempSensitiveMsg;
    }

    private int getValueStartIndex(String msg, int index) {
        for (int i = index; i < msg.length(); i++) {
            char tempChar = msg.charAt(i);
            if (msg.charAt(i) == ' ') {
                return msg.length();
            }
            if (TAIL_SEPARATIONS.contains(tempChar)) {
                return i + 1;
            }
        }
        return msg.length();
    }

    private boolean isAllMatch(String msg, String key, int index) {
        if (index > 0 && !HEAD_SEPARATIONS.contains(msg.charAt(index - 1))) {
            return false;
        }
        if (!TAIL_SEPARATIONS.contains(msg.charAt(index + key.length()))) {
            return false;
        }
        return true;
    }
}
