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
package openbackup.system.base.sdk.livemount.model;

import lombok.Data;

import java.util.Objects;

/**
 * Live Mount Context
 *
 * @param <T> template T
 * @author l00272247
 * @since 2020-09-18
 */
@Data
public class Identity<T> {
    private String type;

    private T data;

    /**
     * Default constructor
     */
    public Identity() {
        this(null, null);
    }

    /**
     * constructor with parameters
     *
     * @param type type
     * @param data data
     */
    public Identity(String type, T data) {
        this.type = Objects.requireNonNull(type);
        this.data = data;
    }
}
