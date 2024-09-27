package openbackup.system.base.common.constants;

import lombok.Getter;

/**
 * redis类型类
 *
 * @author t00482481
 * @since 2020-07-05
 */
@Getter
public enum RedisTypeEnum {
    STRING("string"),
    LIST("list"),
    HASH("hash"),
    SET("set"),
    ZSET("zset");

    private final String value;

    RedisTypeEnum(String value) {
        this.value = value;
    }
}
