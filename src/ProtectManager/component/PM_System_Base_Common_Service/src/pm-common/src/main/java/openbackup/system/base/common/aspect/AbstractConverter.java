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
package openbackup.system.base.common.aspect;

import java.util.Collection;
import java.util.stream.Collectors;

/**
 * Abstract Converter
 *
 */
public abstract class AbstractConverter implements DataConverter {
    private final String name;

    /**
     * constructor
     *
     * @param name name
     */
    protected AbstractConverter(String name) {
        this.name = name;
    }

    /**
     * converter name
     *
     * @return converter name
     */
    @Override
    public String getName() {
        return name;
    }

    /**
     * convert data
     *
     * @param data data
     * @return result
     */
    @Override
    public Collection<?> convert(Collection<?> data) {
        return data.stream().map(this::cast).collect(Collectors.toList());
    }

    /**
     * data cast
     *
     * @param data data
     * @return result
     */
    protected abstract Object cast(Object data);
}
