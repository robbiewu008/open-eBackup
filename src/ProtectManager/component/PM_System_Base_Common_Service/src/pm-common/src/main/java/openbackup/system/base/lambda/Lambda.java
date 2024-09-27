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
