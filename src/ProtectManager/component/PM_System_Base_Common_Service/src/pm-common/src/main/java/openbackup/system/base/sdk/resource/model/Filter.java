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
package openbackup.system.base.sdk.resource.model;

import com.alibaba.fastjson.JSONObject;
import com.fasterxml.jackson.annotation.JsonAlias;

import lombok.Data;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import java.util.ArrayList;
import java.util.List;

/**
 * Filter Entity
 *
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
