package openbackup.system.base.common.enums;

/**
 * 功能描述: 保护对象数据一致性状态枚举类型
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-26
 */
public enum ConsistentStatusEnum {
    /**
     * 待检测
     */
    UNDETECTED("undetected"),

    /**
     * 一致
     */
    CONSISTENT("consistent"),

    /**
     * 不一致
     */
    INCONSISTENT("inconsistent");

    private final String status;

    ConsistentStatusEnum(String status) {
        this.status = status;
    }

    /**
     * getter
     *
     * @return status
     */
    public String getStatus() {
        return status;
    }
}
