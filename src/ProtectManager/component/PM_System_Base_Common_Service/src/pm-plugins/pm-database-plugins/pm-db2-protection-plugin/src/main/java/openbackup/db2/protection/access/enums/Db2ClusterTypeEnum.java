package openbackup.db2.protection.access.enums;

import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;

import java.util.Arrays;

/**
 * db2集群类型
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-01-06
 */
public enum Db2ClusterTypeEnum {
    SINGLE("single"),

    DPF("dpf"),

    POWER_HA("powerHA"),

    HADR("hadr");

    Db2ClusterTypeEnum(String type) {
        this.type = type;
    }

    private final String type;

    public String getType() {
        return type;
    }

    /**
     * 根据type获取到对应的枚举
     *
     * @param type 枚举值
     * @return enum
     */
    public static Db2ClusterTypeEnum getByType(String type) {
        return Arrays.stream(Db2ClusterTypeEnum.values())
            .filter(location -> location.type.equals(type))
            .findFirst()
            .orElseThrow(IllegalArgumentException::new);
    }

    /**
     * 根据集群类型获取到对应的部署类型
     *
     * @param clusterType 集群类型
     * @return 部署类型
     */
    public static String getDeployType(String clusterType) {
        Db2ClusterTypeEnum clusterTypeEnum = Db2ClusterTypeEnum.getByType(clusterType);
        switch (clusterTypeEnum) {
            case DPF:
                return DatabaseDeployTypeEnum.AP.getType();
            case POWER_HA:
                return DatabaseDeployTypeEnum.SHARDING.getType();
            case HADR:
                return DatabaseDeployTypeEnum.AA.getType();
            default:
                return DatabaseDeployTypeEnum.SINGLE.getType();
        }
    }
}
