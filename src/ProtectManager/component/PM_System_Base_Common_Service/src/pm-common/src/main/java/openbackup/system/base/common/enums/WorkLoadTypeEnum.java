package openbackup.system.base.common.enums;

import lombok.Getter;

/**
 * 工作负载类型 k8s的
 *
 * @author y30046482
 * @since 2023-10-18
 */
@Getter
public enum WorkLoadTypeEnum {
    DAEMON_SET(0, "DaemonSet"),
    DEPLOYMENT(1, "Deployment"),
    REPLICA_SET(2, "ReplicaSet"),
    STATEFUL_SET(3, "StatefulSet"),
    JOB(4, "Job"),
    CRON_JOB(5, "CronJob");

    private final Integer id;
    private final String type;
    WorkLoadTypeEnum(Integer id, String type) {
        this.id = id;
        this.type = type;
    }

    /**
     * 检查工作负载类型是否属于上述类型之一。
     *
     * @param type string类型的工作负载类型
     * @return 是否包含在枚举类中
     */
    public static boolean contains(String type) {
        for (WorkLoadTypeEnum workLoadTypeEnum : WorkLoadTypeEnum.values()) {
            if (workLoadTypeEnum.getType().equals(type)) {
                return true;
            }
        }
        return false;
    }
}
