package openbackup.data.access.client.sdk.api.storage.model;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 功能描述
 *
 * @author y00413474
 * @since 2020-07-01
 */
@Data
@JsonIgnoreProperties(ignoreUnknown = true)
public class StoragePool {
    @JsonProperty("ID")
    private String id;

    @JsonProperty("NAME")
    private String name;

    @JsonProperty("RUNNINGSTATUS")
    private Integer status;

    /**
     * 总容量。
     * 单位：sectors
     */
    @JsonProperty("USERTOTALCAPACITY")
    private Long userTotalCapacity;

    /**
     * 空闲容量。
     * 单位：sectors
     */
    @JsonProperty("USERFREECAPACITY")
    private Long userFreeCapacity;

    /**
     * 已用容量。
     * 单位：sectors
     */
    @JsonProperty("USERCONSUMEDCAPACITY")
    private Long userConsumedCapacity;
}
