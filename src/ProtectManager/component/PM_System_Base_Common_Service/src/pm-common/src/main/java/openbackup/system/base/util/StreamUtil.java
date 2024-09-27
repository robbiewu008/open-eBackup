package openbackup.system.base.util;

import java.util.function.Function;
import java.util.stream.Stream;

/**
 * Stream Util
 *
 * @author l00650874
 * @since 2022-07-01
 */
public class StreamUtil {
    private StreamUtil() {
    }

    /**
     * match stream element type
     *
     * @param type type
     * @param <E>  template type
     * @return matcher function
     */
    public static <E> Function<Object, Stream<E>> match(Class<E> type) {
        return (e) -> Stream.of(e).filter(type::isInstance).map(type::cast);
    }
}
