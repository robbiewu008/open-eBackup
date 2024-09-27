package openbackup.system.base.lambda;

/**
 * Throwable Supplier
 *
 * @param <ET> template type ET
 * @param <T> template type T
 * @author l00272247
 * @since 2021-12-14
 */
public interface ThrowableSupplier<ET extends Throwable, T> {
    /**
     * getter
     *
     * @return value
     * @throws ET exception
     */
    T get() throws ET;
}
