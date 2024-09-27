package openbackup.system.base.common.aspect;

import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;

import org.springframework.stereotype.Component;

import java.util.Collection;

/**
 * Json Converter
 *
 * @author l00272247
 * @since 2021-06-10
 */
@Component
public class JsonConverter extends AbstractConverter {
    /**
     * constructor
     */
    public JsonConverter() {
        super("json");
    }

    /**
     * data cast
     *
     * @param data data
     * @return result
     */
    @Override
    protected Object cast(Object data) {
        if (data == null) {
            return null;
        }
        if (data instanceof Collection || data.getClass().isArray()) {
            return JSONArray.fromObject(data);
        }
        return JSONObject.fromObject(data);
    }
}
