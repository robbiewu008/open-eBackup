package openbackup.system.base.query;

import java.lang.reflect.Field;

/**
 * Field Naming Strategy
 *
 * @author l00272247
 * @since 2021-06-03
 */
public interface FieldNamingStrategy {
    /**
     * translate
     *
     * @param type type
     * @param field field
     * @return result
     */
    String translate(Class<?> type, Field field);
}
