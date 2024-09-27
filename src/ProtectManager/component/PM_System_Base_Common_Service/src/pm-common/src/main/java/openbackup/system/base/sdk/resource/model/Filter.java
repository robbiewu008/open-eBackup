/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import com.alibaba.fastjson.JSONObject;
import com.fasterxml.jackson.annotation.JsonAlias;

import lombok.Data;

import java.util.ArrayList;
import java.util.List;

/**
 * Filter Entity
 *
 * @author l00272247
 * @since 2020-07-14
 */
@Data
public class Filter {
    // 1表示文件，2表示目录，3表示类型，4表示日期
    @JsonAlias("type")
    int filterType;

    // 1表示排除，2表示包含
    @JsonAlias("model")
    int filterMode;

    // 过滤内容
    Object content;

    /**
     * parse filter
     *
     * @param filters filters
     * @return parsed fileters
     */
    public static List<Filter> parse(List<Filter> filters) {
        List<Filter> filterList = new ArrayList<>();
        for (Filter filter : filters) {
            Filter convertFilter = new Filter();
            convertFilter.setFilterMode(filter.getFilterMode());
            convertFilter.setFilterType(filter.getFilterType());
            final Object obj = filter.getContent();
            if (!(obj instanceof String)) {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "obj is not instance of String");
            }
            convertFilter.setContent(JSONObject.parse((String) obj));
            filterList.add(convertFilter);
        }
        return filterList;
    }
}
