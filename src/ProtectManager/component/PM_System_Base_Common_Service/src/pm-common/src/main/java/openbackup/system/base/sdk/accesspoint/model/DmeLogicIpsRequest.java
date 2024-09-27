package openbackup.system.base.sdk.accesspoint.model;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * DME 修改逻辑端口链路请求
 *
 * @author swx1010572
 * @since 2021-01-12
 */
@Data
@JsonInclude(JsonInclude.Include.NON_NULL)
public class DmeLogicIpsRequest {
    /**
     * old name
     */
    @JsonProperty("originLogicIpNames")
    private List<String> originLogicIpNames;

    /**
     * new name
     */
    @JsonProperty("newLogicIpNames")
    private List<String> newLogicIpNames;

    /**
     * Dme Local Device
     */
    @JsonProperty("LocalDevice")
    private DmeLocalDevice localDevice;
}
