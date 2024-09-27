package openbackup.data.protection.access.provider.sdk.enums;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

import java.util.Arrays;

/**
 * SDK中使用的LiveMountLocation枚举类
 *
 * @author y00559272
 * @since 2021-10-11
 */
public enum ProviderLiveMountLocationEnum {
    ORIGINAL("original"),
    OTHERS("others");

    private String value;

    ProviderLiveMountLocationEnum(String value) {
        this.value = value;
    }

    /**
     * get enum by value
     *
     * @param value value
     * @return enum
     */
    @JsonCreator
    public static ProviderLiveMountLocationEnum get(String value) {
        return Arrays.stream(ProviderLiveMountLocationEnum.values())
            .filter(location -> location.value.equals(value))
            .findFirst()
            .orElseThrow(IllegalArgumentException::new);
    }

    @JsonValue
    public String getValue() {
        return value;
    }
}
