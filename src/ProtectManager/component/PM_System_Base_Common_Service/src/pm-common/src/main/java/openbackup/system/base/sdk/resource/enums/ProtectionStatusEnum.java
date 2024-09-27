package openbackup.system.base.sdk.resource.enums;

import java.util.stream.Stream;

/**
 * 保护状态枚举
 *
 * @author lWX1012372
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-10-28
 */
public enum ProtectionStatusEnum {
    /**
     * 未保护
     */
    UNPROTECTED(0),

    /**
     * 已保护
     */
    PROTECTED(1),

    /**
     * 保护中
     */
    PROTECTING(2);

    private final Integer type;

    ProtectionStatusEnum(Integer type) {
        this.type = type;
    }

    public Integer getType() {
        return type;
    }

    /**
     * 生成ProtectionStatusEnum
     *
     * @param type type
     * @return ProtectionStatusEnum
     */
    public static ProtectionStatusEnum getStatus(Integer type) {
        return Stream.of(ProtectionStatusEnum.values())
            .filter(status -> status.getType().equals(type))
            .findFirst().orElseThrow(() -> new IllegalArgumentException("protection status is illegal."));
    }
}
