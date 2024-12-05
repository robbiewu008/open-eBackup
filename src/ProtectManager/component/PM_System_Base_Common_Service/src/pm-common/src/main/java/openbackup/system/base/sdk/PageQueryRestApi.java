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
package openbackup.system.base.sdk;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.model.BasePage;

import java.util.Collections;
import java.util.List;

/**
 * page query rest api
 *
 * @param <T> template type
 */
@FunctionalInterface
public interface PageQueryRestApi<T> {
    /**
     * get page query rest api
     *
     * @param api api
     * @param <T> template type
     * @return api
     */
    static <T> PageQueryRestApi<T> get(PageQueryRestApi<T> api) {
        return api;
    }

    /**
     * query method
     *
     * @param page page
     * @param size size
     * @param condition condition
     * @param orders orders
     * @return page data
     */
    BasePage<T> query(int page, int size, String condition, List<String> orders);

    /**
     * query copies
     *
     * @param page page
     * @param size size
     * @param orders orders
     * @param condition condition
     * @return copies
     */
    default BasePage<T> query(int page, int size, JSONObject condition, List<String> orders) {
        return query(page, size, condition.toString(), orders);
    }

    /**
     * query copies
     *
     * @param page page
     * @param size size
     * @param condition condition
     * @return copies
     */
    default BasePage<T> query(int page, int size, JSONObject condition) {
        return query(page, size, condition, Collections.emptyList());
    }

    /**
     * count items matched the condition
     *
     * @param condition condition
     * @return count result
     */
    default long count(JSONObject condition) {
        return query(0, 1, condition).getTotal();
    }

    /**
     * query one entity
     *
     * @param condition condition
     * @return one entity
     */
    default T queryOne(JSONObject condition) {
        return query(0, IsmNumberConstant.TWO, condition).one();
    }

    /**
     * query one entity
     *
     * @param condition condition
     * @param orders orders
     * @return one entity
     */
    default T queryOne(JSONObject condition, List<String> orders) {
        return query(0, IsmNumberConstant.TWO, condition, orders).one();
    }

    /**
     * query one entity
     *
     * @param condition condition
     * @param exceptions exceptions
     * @return one entity
     */
    default T queryOne(JSONObject condition, LegoCheckedException... exceptions) {
        return query(0, IsmNumberConstant.TWO, condition).one(exceptions);
    }

    /**
     * query one entity
     *
     * @param condition condition
     * @param isStrict strict
     * @return one entity
     */
    default T queryOne(JSONObject condition, boolean isStrict) {
        if (isStrict) {
            return queryOne(condition, new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST));
        }
        return queryOne(condition);
    }

    /**
     * query all
     *
     * @param condition condition
     * @return all entity
     */
    default BasePage<T> queryAll(JSONObject condition) {
        return BasePage.queryAll((page, size) -> query(page, size, condition));
    }
}
