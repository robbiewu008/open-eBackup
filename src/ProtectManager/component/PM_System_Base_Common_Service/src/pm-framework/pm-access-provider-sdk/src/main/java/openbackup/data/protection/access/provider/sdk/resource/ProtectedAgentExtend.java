package openbackup.data.protection.access.provider.sdk.resource;

import lombok.Getter;
import lombok.Setter;

/**
 * 页面展示ProtectedAgentExtendPo
 *
 * @author z00613137
 * @since 2023-08-11
 */
@Getter
@Setter
public class ProtectedAgentExtend {
    /**
     * CPU占用率百分比 单位：%
     */
    private double cpuRate;

    /**
     * 内存占用率百分比  单位：%
     */
    private double memRate;

    /**
     * 最近一次更新时间 距离1970年相对时间（带时区）
     */
    private long lastUpdateTime;

    /**
     * 是否开启多租户共享
     */
    private Boolean isShared;
}