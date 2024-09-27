package openbackup.system.base.sdk.protection.model;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

import java.util.Optional;

/**
 * 策略数据模型
 *
 * @author y00559272
 * @version [OceanProtect A8000 1.0.0]
 * @since 2020/10/9
 **/
@Data
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class PolicyBo {
    /**
     * uuid
     */
    private String uuid;

    /**
     * policy type
     */
    private String type;

    /**
     * policy name
     */
    private String name;

    /**
     * backup policy action
     */
    private String action;

    /**
     * policy extend parameters
     */
    private JsonNode extParameters;

    /**
     * policy schedule info
     */
    private ScheduleBo schedule;

    /**
     * policy retention info
     */
    private RetentionBo retention;

    /**
     * 根据key获取policy扩展参数的boolean值
     *
     * @param key 扩展参数的key
     * @return key对应的value
     */
    public Optional<Boolean> getBooleanFormExtParameters(String key) {
        return Optional.ofNullable(extParameters.get(key)).map(JsonNode::asBoolean);
    }

    /**
     * 据key获取policy扩展参数的Integer值
     *
     * @param key key
     * @param defaultValue 如果为空默认的值
     * @return key对应的value
     */
    public Integer getIntegerFormExtParameters(String key, Integer defaultValue) {
        return Optional.ofNullable(extParameters.get(key)).map(JsonNode::asInt).orElse(defaultValue);
    }
}
