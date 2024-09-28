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

import ch.qos.logback.classic.spi.ILoggingEvent;
import ch.qos.logback.core.filter.Filter;
import ch.qos.logback.core.spi.FilterReply;

import org.apache.commons.collections.CollectionUtils;

import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.regex.Pattern;

/**
 * 敏感日志过滤器，配置在logback-spring.xml中生效
 * <p>
 * 根据配置规则进行过滤，当前支持两种过滤规则当规则匹配时过滤当前日志：</br>
 * 1.fuzz 模糊匹配：整条日志任意位置包含即匹配 </br>
 * 2.word 全匹配：整条日志中任意单词相同即匹配 </br>
 * </p>
 *
 **/
public class LogFilter extends Filter<ILoggingEvent> {
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

    /**
     * 正则表达式中单词分界标记
     */
    private static final String REGEX_WORD = "\\b";

    /**
     * 模糊匹配的key列表
     */
    private final List<String> fuzzRuleKeys = SensitiveConfig.getFuzzRuleKeys(RULE_FUZZ);

    /**
     * 单词匹配的key列表
     */
    private final List<String> allRuleKeys = SensitiveConfig.getFuzzRuleKeys(RULE_WORD);

    /**
     * 单词匹配的正则pattern缓存，提高性能
     */
    private final Map<String, Pattern> wordPatternMap = new HashMap<>();

    @Override
    public FilterReply decide(ILoggingEvent event) {
        if (!IS_ENABLE) {
            return FilterReply.ACCEPT;
        }
        // 全部转成小写，忽略大小写进行匹配
        final String message = event.getFormattedMessage().toLowerCase(Locale.ROOT);
        if (CollectionUtils.isNotEmpty(fuzzRuleKeys) && fuzzRuleKeys.stream().anyMatch(message::contains)) {
            // 模糊匹配的key，任意匹配过滤当前整条日志
            return FilterReply.DENY;
        }
        if (CollectionUtils.isNotEmpty(allRuleKeys) && allRuleKeys.stream()
            .anyMatch(item -> this.matchWord(item, message))) {
            // 精确匹配的key，任意匹配过滤当前整条日志
            return FilterReply.DENY;
        }
        return FilterReply.ACCEPT;
    }

    private boolean matchWord(String key, String message) {
        return wordPatternMap.computeIfAbsent(key, word -> Pattern.compile(REGEX_WORD + word + REGEX_WORD))
            .matcher(message)
            .find();
    }
}
