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

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.core.io.ClassPathResource;
import org.yaml.snakeyaml.Yaml;
import org.yaml.snakeyaml.constructor.Constructor;

import java.io.IOException;
import java.util.List;
import java.util.Locale;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * 敏感信息过滤的配置文件解析类
 *
 **/
@Slf4j
public final class SensitiveConfig {
    /**
     * 敏感过滤配置文件
     */
    public static final String SETTING_FILE = "sensitive-rule.yaml";

    /**
     * 跟配置文件映射的规则集
     */
    private static RuleSet ruleSet;

    static {
        // 静态初始化块，读取配置文件，映射成对象
        Yaml yaml = new Yaml(new Constructor(RuleSet.class));
        try {
            ruleSet = yaml.load(new ClassPathResource(SETTING_FILE).getInputStream());
        } catch (IOException e) {
            log.error("cant not find sensitive rule file");
            ruleSet = new RuleSet().defaultInit();
        }
    }

    /**
     * 根据rule获取配置文件中的敏感词列表
     *
     * @param rule 规则key
     * @return 敏感词列表
     */
    public static List<String> getFuzzRuleKeys(String rule) {
        return ruleSet.getSensitiveWords()
            .stream()
            .collect(Collectors.groupingBy(Rule::getRule))
            .get(rule)
            .stream()
            .flatMap(item -> Stream.of(StringUtils.split(item.getKeys(), ",")))
            .map(msg -> msg.toLowerCase(Locale.ROOT))
            .collect(Collectors.toList());
    }

    /**
     * 是否开启敏感词过滤
     *
     * @return result 表示是否开启功能
     */
    static boolean isEnable() {
        return ruleSet.isEnable();
    }
}
