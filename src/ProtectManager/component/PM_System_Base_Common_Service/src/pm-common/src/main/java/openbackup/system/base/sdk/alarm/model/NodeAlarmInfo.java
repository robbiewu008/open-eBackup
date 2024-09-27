package openbackup.system.base.sdk.alarm.model;

import lombok.Data;

/**
 * 节点告警统计数据
 *
 * @author w00607005
 * @since 2023-07-11
 */
@Data
public class NodeAlarmInfo {
    /**
     * 节点esn
     */
    private String esn;

    /**
     * 节点名称
     */
    private String nodeName;

    /**
     * 节点角色
     */
    private String nodeRole;

    /**
     * 关键告警
     */
    private int critical;

    /**
     * 警告告警
     */
    private int warning;

    /**
     * 主要告警
     */
    private int major;

    /**
     * 次要告警
     */
    private int minor;
}
