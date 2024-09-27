package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * VM hardware信息
 *
 * @author h30003246
 * @since 2020-12-04
 */
@Data
public class VmHardware {
    @JsonProperty("num_cpu")
    private Integer numCpu;

    @JsonProperty("num_cores_per_socket")
    private Integer numCoresPerSocket;

    @JsonProperty("memory")
    private Integer memory;
}
