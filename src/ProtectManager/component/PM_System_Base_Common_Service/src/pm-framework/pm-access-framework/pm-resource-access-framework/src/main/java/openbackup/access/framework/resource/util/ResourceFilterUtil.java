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
package openbackup.access.framework.resource.util;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.persistence.model.VirtualResourceExtendPo;
import openbackup.data.access.framework.core.common.enums.v2.filter.FilterModeEnum;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceFilterParam;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupExtendParam;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;

import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * 资源过滤
 *
 */
@Slf4j
public final class ResourceFilterUtil {
    private static final String RULE_ALL = "ALL";

    private static final String RULE_START_WITH = "START_WITH";

    private static final String RULE_END_WITH = "END_WITH";

    private static final String RULE_FUZZY = "FUZZY";

    private static final String COMMA = ",";

    private ResourceFilterUtil() {}

    /**
     *  根据规则过滤资源
     *
     * @param virtualResources 资源
     * @param extendParam 过滤条件
     * @return 过滤后的资源
     */
    public static List<VirtualResourceExtendPo> filterResources(List<VirtualResourceExtendPo> virtualResources,
        ResourceGroupExtendParam extendParam) {
        if (VerifyUtil.isEmpty(extendParam) || VerifyUtil.isEmpty(virtualResources)) {
            return virtualResources;
        }
        List<VirtualResourceExtendPo> resources = virtualResources;
        List<ResourceFilterParam> resourceFilters = extendParam.getResourceFilters();
        resources = filterByName(resources, resourceFilters);
        List<ResourceFilterParam> tagFilters = extendParam.getResourceTagFilters();
        resources = filterByTag(resources, tagFilters);
        return resources;
    }

    private static List<VirtualResourceExtendPo> filterByName(List<VirtualResourceExtendPo> virtualResources,
        List<ResourceFilterParam> resourceFilters) {
        if (VerifyUtil.isEmpty(resourceFilters)) {
            return virtualResources;
        }
        Map<String, List<ResourceFilterParam>> batchFilterMap = resourceFilters.stream()
            .collect(Collectors.groupingBy(ResourceFilterParam::getMode));

        return virtualResources.stream()
            .filter(v -> {
                boolean include = true;
                boolean unExclude = true;
                if (!VerifyUtil.isEmpty(batchFilterMap.get(FilterModeEnum.EXCLUDE.getMode()))) {
                    unExclude = !filterByRule(v.getName(), batchFilterMap.get(FilterModeEnum.EXCLUDE.getMode()));
                }
                if (!VerifyUtil.isEmpty(batchFilterMap.get(FilterModeEnum.INCLUDE.getMode()))) {
                    include = filterByRule(v.getName(), batchFilterMap.get(FilterModeEnum.INCLUDE.getMode()));
                }
                return include && unExclude;
            })
            .collect(Collectors.toList());
    }

    private static List<VirtualResourceExtendPo> filterByTag(List<VirtualResourceExtendPo> virtualResources,
        List<ResourceFilterParam> resourceFilters) {
        if (VerifyUtil.isEmpty(resourceFilters)) {
            return virtualResources;
        }
        Map<String, List<ResourceFilterParam>> batchFilterMap = resourceFilters.stream()
            .collect(Collectors.groupingBy(ResourceFilterParam::getMode));

        return virtualResources.stream()
            .filter(v -> checkTag(v.getTags(), batchFilterMap))
            .collect(Collectors.toList());
    }

    private static boolean checkTag(String tags, Map<String, List<ResourceFilterParam>> batchFilterMap) {
        // 没有tag标记时，并且有include条件说明该虚拟机不符合条件
        if (VerifyUtil.isEmpty(tags)) {
            return !batchFilterMap.containsKey(FilterModeEnum.INCLUDE.getMode());
        }

        String[] tagList = tags.split(COMMA);
        boolean include = true;
        boolean unExclude = true;
        if (!VerifyUtil.isEmpty(batchFilterMap.get(FilterModeEnum.INCLUDE.getMode()))) {
            include = Stream.of(tagList).anyMatch(v -> {
                String tag = null;
                try {
                    tag = URLDecoder.decode(v, "UTF-8");
                } catch (UnsupportedEncodingException e) {
                    log.error("tag {} decode error", tag, ExceptionUtil.getErrorMessage(e));
                    return false;
                }
                return filterByRule(tag, batchFilterMap.get(FilterModeEnum.INCLUDE.getMode()));
            });
        }
        if (!VerifyUtil.isEmpty(batchFilterMap.get(FilterModeEnum.EXCLUDE.getMode()))) {
            unExclude = !Stream.of(tagList).anyMatch(v -> {
                String tag = null;
                try {
                    tag = URLDecoder.decode(v, "UTF-8");
                } catch (UnsupportedEncodingException e) {
                    log.error("tag {} decode error", tag, ExceptionUtil.getErrorMessage(e));
                    return false;
                }
                return filterByRule(tag, batchFilterMap.get(FilterModeEnum.EXCLUDE.getMode()));
            });
        }
        return include && unExclude;
    }

    // 选择多个包含或非包含条件时,满足一个即可
    private static boolean filterByRule(String value, List<ResourceFilterParam> resourceFilters) {
        for (ResourceFilterParam filter : resourceFilters) {
            if (filterByRule(value, filter)) {
                return true;
            }
        }
        return false;
    }

    private static boolean filterByRule(String value, ResourceFilterParam filter) {
        switch (filter.getRule()) {
            case RULE_ALL:
                return filter.getValues().contains(value);
            case RULE_END_WITH:
                return matchEnd(value, filter.getValues());
            case RULE_START_WITH:
                return matchStart(value, filter.getValues());
            case RULE_FUZZY:
                return matchFuzzy(value, filter.getValues());
            default:
                return false;
        }
    }

    private static boolean matchEnd(String value, List<String> values) {
        for (String end : values) {
            if (value.endsWith(end)) {
                return true;
            }
        }
        return false;
    }

    private static boolean matchStart(String value, List<String> values) {
        for (String start : values) {
            if (value.startsWith(start)) {
                return true;
            }
        }
        return false;
    }

    private static boolean matchFuzzy(String value, List<String> values) {
        String lowerCaseValue = value.toLowerCase(Locale.ROOT);
        for (String fuzzy : values) {
            if (lowerCaseValue.contains(fuzzy.toLowerCase(Locale.ROOT))) {
                return true;
            }
        }
        return false;
    }
}
