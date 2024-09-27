package openbackup.system.base.invoke;

/**
 * invoker
 *
 * @param <T> param template type
 * @param <R> result template type
 * @author l00272247
 * @since 2021-10-19
 */
public interface Invoker<T, R> {
    /**
     * invoke method
     *
     * @param invocation invocation
     * @param param param
     * @return result
     */
    R invoke(Invocation<T, R> invocation, T param);
}
