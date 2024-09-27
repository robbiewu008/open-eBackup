package openbackup.system.base.common.utils.json;

import com.fasterxml.jackson.core.type.TypeReference;

import java.lang.reflect.Type;

/**
 * Simple Type Reference
 *
 * @author l00272247
 * @since 2020-07-13
 */
public class SimpleTypeReference extends TypeReference<Object> {
    private Type type;

    /**
     * constructor
     *
     * @param type type
     */
    public SimpleTypeReference(Type type) {
        this.type = type;
    }

    /**
     * constructor
     *
     * @return type
     */
    @Override
    public Type getType() {
        return type;
    }
}
