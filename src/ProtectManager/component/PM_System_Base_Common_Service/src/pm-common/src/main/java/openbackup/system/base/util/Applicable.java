package openbackup.system.base.util;

/**
 * Applicable
 *
 * @param <T> template type
 * @author l00272247
 * @since 2020-07-14
 */
public interface Applicable<T> {
    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    boolean applicable(T object);
}
