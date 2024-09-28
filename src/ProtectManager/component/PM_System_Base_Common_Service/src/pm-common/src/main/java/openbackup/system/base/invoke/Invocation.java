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
package openbackup.system.base.invoke;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Objects;
import java.util.Optional;
import java.util.function.Function;

/**
 * Invocation
 *
 * @param <T> param template type
 * @param <R> result template type
 */
public class Invocation<T, R> {
    private final List<Invoker<T, R>> invokers;
    private final Function<T, R> function;

    /**
     * constructor
     *
     * @param function function
     * @param invokers invokers
     */
    public Invocation(Function<T, R> function, List<Invoker<T, R>> invokers) {
        this.function = Objects.requireNonNull(function);
        this.invokers = Objects.requireNonNull(invokers);
    }

    /**
     * constructor
     *
     * @param function function
     * @param invokers invokers
     */
    @SafeVarargs
    public Invocation(Function<T, R> function, Invoker<T, R>... invokers) {
        this(function, Optional.ofNullable(invokers).map(Arrays::asList).orElseGet(Collections::emptyList));
    }

    /**
     * invoke function with param
     *
     * @param param param
     * @return result
     */
    public R invoke(T param) {
        if (invokers.isEmpty()) {
            return function.apply(param);
        } else {
            Invoker<T, R> invoker = invokers.get(0);
            Invocation<T, R> invocation = new Invocation<>(function, invokers.subList(1, invokers.size()));
            return invoker.invoke(invocation, param);
        }
    }
}
