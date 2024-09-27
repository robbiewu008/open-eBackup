package openbackup.system.base.sdk.livemount.model;

import lombok.Data;

import java.util.Objects;

/**
 * Live Mount Context
 *
 * @param <T> template T
 * @author l00272247
 * @since 2020-09-18
 */
@Data
public class Identity<T> {
    private String type;

    private T data;

    /**
     * Default constructor
     */
    public Identity() {
        this(null, null);
    }

    /**
     * constructor with parameters
     *
     * @param type type
     * @param data data
     */
    public Identity(String type, T data) {
        this.type = Objects.requireNonNull(type);
        this.data = data;
    }
}
