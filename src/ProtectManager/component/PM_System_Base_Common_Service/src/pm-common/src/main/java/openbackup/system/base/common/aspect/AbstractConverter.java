package openbackup.system.base.common.aspect;

import java.util.Collection;
import java.util.stream.Collectors;

/**
 * Abstract Converter
 *
 * @author l00272247
 * @since 2021-06-10
 */
public abstract class AbstractConverter implements DataConverter {
    private final String name;

    /**
     * constructor
     *
     * @param name name
     */
    protected AbstractConverter(String name) {
        this.name = name;
    }

    /**
     * converter name
     *
     * @return converter name
     */
    @Override
    public String getName() {
        return name;
    }

    /**
     * convert data
     *
     * @param data data
     * @return result
     */
    @Override
    public Collection<?> convert(Collection<?> data) {
        return data.stream().map(this::cast).collect(Collectors.toList());
    }

    /**
     * data cast
     *
     * @param data data
     * @return result
     */
    protected abstract Object cast(Object data);
}
