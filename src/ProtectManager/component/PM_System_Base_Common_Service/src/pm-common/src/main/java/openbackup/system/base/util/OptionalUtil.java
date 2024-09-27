/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.util;

import java.util.Optional;
import java.util.function.Function;

/**
 * Optional Util
 *
 * @author l00650874
 * @since 2022-07-01
 */
public class OptionalUtil {
    /**
     * match stream element type
     *
     * @param type type
     * @param <E>  template type
     * @return matcher function
     */
    public static <E> Function<Object, Optional<E>> match(Class<E> type) {
        return (e) -> type.isInstance(e) ? Optional.of(type.cast(e)) : Optional.empty();
    }
}
