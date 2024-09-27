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
package openbackup.system.base.lambda;

import java.util.function.Supplier;

/**
 * Lambda
 *
 * @author l00272247
 * @since 2021-12-25
 */
public final class Lambda {
    private Lambda() {}

    /**
     * cast runnable to supplier
     *
     * @param runnable runnable
     * @param <T> template type T
     * @return result
     */
    public static <T> Supplier<T> supplier(Runnable runnable) {
        return () -> {
            runnable.run();
            return null;
        };
    }
}
