package openbackup.system.base.service.hostagent.model;

import lombok.Data;

/**
 * 客户端扩展信息
 *
 * @author q00654632
 * @since 2023-10-08
 */
@Data
public class AgentExtend {
    /**
     * CPU占用率百分比 单位：%
     */
    private double cpuRate;

    /**
     * 内存占用率百分比 单位：%
     */
    private double memRate;

    /**
     * 最近一次更新时间 距离1970年相对时间（带时区）
     */
    private long lastUpdateTime;

    /**
     *
     * 是否多租户共享，默认false
     */
    private Boolean isShared = false;
}
