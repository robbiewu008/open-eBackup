package openbackup.system.base.sdk.protection.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * replication qos返回类
 *
 * @author twx1009756
 * @since 2020-12-26
 */
@Data
public class QosBo {
    /**
     * qos的uuid
     */
    private String uuid;

    /**
     * qos的名字
     */
    private String name;

    /**
     * qos的限速带宽
     */
    @JsonProperty("speed_limit")
    private int speedLimit;

    /**
     * qos的详情描述
     */
    private String description;

    /**
     * get qos bo speed limit
     *
     * @param qosBo qos bo
     * @return speed limit
     */
    public static int getQosBoSpeedLimit(QosBo qosBo) {
        return qosBo != null ? qosBo.getSpeedLimit() : 0;
    }
}
