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

import openbackup.system.base.common.model.PageListResponse;

import org.apache.commons.collections.CollectionUtils;

import java.util.ArrayList;
import java.util.List;

/**
 * 批处理工具类
 *
 * @author w00607005
 * @since 2023-07-24
 */
public class BatchHandleUtil {
    /**
     * 假分页
     *
     * @param sourceList 源数据
     * @param pageNo 页码
     * @param pageSize 分页大小
     * @param <T> 泛型
     * @return 分页结果
     */
    public static <T> PageListResponse<T> fakePaginationPage(List<T> sourceList, int pageNo, int pageSize) {
        PageListResponse<T> response = new PageListResponse<>();
        response.setTotalCount(0);
        if (CollectionUtils.isEmpty(sourceList)) {
            return response;
        }

        response.setTotalCount(sourceList.size());
        response.setRecords(fakePaginationList(sourceList, pageNo, pageSize));
        return response;
    }

    /**
     * 假分页
     *
     * @param sourceList 源数据
     * @param pageNo 页码
     * @param pageSize 分页大小
     * @param <T> 泛型
     * @return 分页结果
     */
    public static <T> List<T> fakePaginationList(List<T> sourceList, int pageNo, int pageSize) {
        if (CollectionUtils.isEmpty(sourceList)) {
            return new ArrayList<>();
        }

        int start = (pageNo - 1) * pageSize;
        int end = pageNo * pageSize;

        if (sourceList.size() <= start) {
            return new ArrayList<>();
        }

        if (sourceList.size() < end) {
            end = sourceList.size();
        }

        return sourceList.subList(start, end);
    }
}
