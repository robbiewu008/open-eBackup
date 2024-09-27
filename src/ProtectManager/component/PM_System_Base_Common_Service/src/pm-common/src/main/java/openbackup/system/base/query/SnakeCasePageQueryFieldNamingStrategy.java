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
package openbackup.system.base.query;

import com.fasterxml.jackson.databind.PropertyNamingStrategy;

import org.springframework.stereotype.Component;

/**
 * SnakeCasePageQueryFieldNamingStrategy
 *
 * @author l00272247
 * @since 2021-06-03
 */
@Component(SnakeCasePageQueryFieldNamingStrategy.NAME)
public class SnakeCasePageQueryFieldNamingStrategy extends DefaultPageQueryFieldNamingStrategy {
    /**
     * snakeCasePageQueryFieldNamingStrategy
     */
    public static final String NAME = "snakeCasePageQueryFieldNamingStrategy";

    /**
     * get Property Naming Strategy
     *
     * @param type type
     * @return result
     */
    @Override
    protected PropertyNamingStrategy.PropertyNamingStrategyBase getPropertyNamingStrategy(Class<?> type) {
        PropertyNamingStrategy.PropertyNamingStrategyBase strategy = super.getPropertyNamingStrategy(type);
        if (strategy != null) {
            return strategy;
        }
        return new PropertyNamingStrategy.SnakeCaseStrategy();
    }
}
