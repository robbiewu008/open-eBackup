package openbackup.system.base.common.aspect;

import java.util.Collection;

/**
 * Converter
 *
 * @author l00272247
 * @since 2021-01-14
 */
public interface DataConverter {
    /**
     * converter name
     *
     * @return converter name
     */
    String getName();

    /**
     * convert data
     *
     * @param data data
     * @return result
     */
    Collection<?> convert(Collection<?> data);
}
