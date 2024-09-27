/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.common.desensitization;

import java.util.Collections;
import java.util.List;

/**
 * 敏感词过滤规则集合
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/3/22
 **/
public class RuleSet {
    /**
     * 是否开启过滤
     */
    private boolean isEnable;

    /**
     * 敏感词配置规则集
     */
    private List<Rule> sensitiveWords;

    /**
     * 默认初始化为空，开关为false
     *
     * @return 默认配置对象
     */
    public RuleSet defaultInit() {
        this.isEnable = Boolean.FALSE;
        this.sensitiveWords = Collections.emptyList();
        return this;
    }

    public List<Rule> getSensitiveWords() {
        return sensitiveWords;
    }

    public void setSensitiveWords(List<Rule> sensitiveWords) {
        this.sensitiveWords = sensitiveWords;
    }

    public boolean isEnable() {
        return isEnable;
    }

    public void setEnable(boolean isEnable) {
        this.isEnable = isEnable;
    }
}
