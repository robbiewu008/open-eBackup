package openbackup.system.base.sdk.anti.model;

import lombok.Data;

import java.util.List;

/**
 * AirGap策略信息对象返回
 *
 * @author q00654632
 * @since 2023-07-12
 */
@Data
public class AirGapPolicyInfoRsp {
    private String id;

    /**
     * 策略名称
     */
    private String name;

    /**
     * 描述
     */
    private String description;

    /**
     * 策略频率
     */
    private String triggerCycle;

    /**
     * 策略周期
     */
    private String triggerWeekFreq;

    /**
     * 策略时间窗
     */
    private List<AirGapPolicyWindowInfoRsp> airGapPolicyWindows;

    /**
     * 关联设备数量
     */
    private Integer deviceCount;
}
