package openbackup.system.base.sdk.user.enums;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * ResourceSetTypeEnum 资源集枚举类
 *
 * @author z30027603
 * @version [OceanProtect DataBackup 1.7.0]
 * @since 2024-05-13
 */
@Getter
@AllArgsConstructor
public enum ResourceSetTypeEnum {
    RESOURCE("RESOURCE"),
    RESOURCE_GROUP("RESOURCE_GROUP"),
    SLA("SLA"),
    QOS("QOS"),
    AGENT("AGENT"),
    COPY("COPY"),
    JOB("JOB"),
    ALARM("ALARM"),
    EVENT("EVENT"),
    REPORT("REPORT"),
    EXERCISE("EXERCISE"),
    JOB_LOG("JOB_LOG"),
    FILE_SET_TEMPLATE("FILE_SET_TEMPLATE"),
    AIR_GAP("AIR_GAP"),
    PREVENT_EXTORTION_AND_WORM("PREVENT_EXTORTION_AND_WORM"),
    DESENSITIZATION("DESENSITIZATION"),
    LIVE_MOUNT_POLICY("LIVE_MOUNT_POLICY"),
    LIVE_MOUNT("LIVE_MOUNT"),
    KERBEROS("KERBEROS");

    private String type;

    /**
     * 根据类型获取枚举值
     *
     * @param type 类型
     * @return 枚举值
     */
    public static ResourceSetTypeEnum getResourceSetTypeEnumByType(String type) {
        for (ResourceSetTypeEnum resourceSetTypeEnum : ResourceSetTypeEnum.values()) {
            if (resourceSetTypeEnum.getType().equals(type)) {
                return resourceSetTypeEnum;
            }
        }
        return RESOURCE;
    }
}
