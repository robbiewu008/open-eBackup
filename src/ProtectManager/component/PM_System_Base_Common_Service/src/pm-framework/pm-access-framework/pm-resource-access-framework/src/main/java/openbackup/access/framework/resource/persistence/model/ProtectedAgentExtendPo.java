package openbackup.access.framework.resource.persistence.model;

import openbackup.system.base.common.constants.IsmNumberConstant;

import com.baomidou.mybatisplus.annotation.TableField;
import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import lombok.Getter;
import lombok.Setter;

/**
 * ProtectedAgentExtendPo
 *
 * @author z00613137
 * @since 2023-08-08
 */
@Getter
@Setter
@TableName("T_HOST_AGENT_INFO")
public class ProtectedAgentExtendPo {
    /**
     * 资源UUID
     */
    @TableId
    private String uuid;

    /**
     * CPU占用率百分比 单位：%
     */
    @TableField("CPU_RATE")
    private double cpuRate;

    /**
     * CPU占用率超过告警阈值连续次数
     */
    @TableField("CPU_RATE_ALARM_THRESHOLD_COUNT")
    private int cpuRateAlarmThresholdCount;

    /**
     * CPU占用率小于清除告警阈值连续次数
     */
    @TableField("CPU_RATE_CLEAN_ALARM_THRESHOLD_COUNT")
    private int cpuRateClearAlarmThresholdCount;

    /**
     * 内存占用率百分比 单位：%
     */
    @TableField("MEM_RATE")
    private double memRate;

    /**
     * 内存占用率超过告警阈值连续次数
     */
    @TableField("MEM_RATE_ALARM_THRESHOLD_COUNT")
    private int memRateAlarmThresholdCount;

    /**
     * 内存占用率小于清除告警阈值连续次数
     */
    @TableField("MEM_RATE_CLEAN_ALARM_THRESHOLD_COUNT")
    private int memRateClearAlarmThresholdCount;

    /**
     * 上报cpu告警时的阈值
     */
    @TableField("SEND_CPU_RATE_ALARM_THRESHOLD")
    private double sendCpuRateAlarmThreshold = IsmNumberConstant.ONE_HUNDRED;

    /**
     * 上报内存告警时的阈值
     */
    @TableField("SEND_MEM_RATE_ALARM_THRESHOLD")
    private double sendMemRateAlarmThreshold = IsmNumberConstant.ONE_HUNDRED;

    /**
     * 最近一次更新时间 距离1970年相对时间（带时区）
     */
    @TableField("LAST_UPDATE_TIME")
    private long lastUpdateTime;

    /**
     * 多租户是否共享 true/false
     */
    @TableField("IS_SHARED")
    private Boolean isShared;
}