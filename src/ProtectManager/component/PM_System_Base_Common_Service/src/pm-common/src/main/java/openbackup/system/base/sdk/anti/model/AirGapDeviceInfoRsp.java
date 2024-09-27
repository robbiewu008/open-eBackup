package openbackup.system.base.sdk.anti.model;

import lombok.Data;

/**
 * AirGap设备信息对象返回
 *
 * @author q00654632
 * @since 2023-07-12
 */
@Data
public class AirGapDeviceInfoRsp {
    /**
     * 设备id
     */
    private String id;

    /**
     * 设备esn
     */
    private String esn;

    /**
     * 设备名称
     */
    private String name;

    /**
     * 设备状态 在线1，离线0
     */
    private String linkStatus;

    /**
     * 复制链路状态 连通open,断开close，未知unknown
     */
    private String replicationLinkStatus;

    /**
     * 策略状态 应用中enable,未应用disable，已失效invalid
     */
    private String policyStatus;

    /**
     * 是否强制终止复制
     */
    private boolean isForceStop;

    /**
     * 是否联动侦测
     */
    private boolean isLinkedDetection;

    /**
     * 设备类型
     */
    private String deviceType;

    /**
     * 复制任务ids
     */
    private String repTaskIds;

    /**
     * 关联策略信息
     */
    private AirGapPolicyInfoRsp airGapPolicyInfo;

    /**
     * 集群名称
     */
    private String clusterName;
}
