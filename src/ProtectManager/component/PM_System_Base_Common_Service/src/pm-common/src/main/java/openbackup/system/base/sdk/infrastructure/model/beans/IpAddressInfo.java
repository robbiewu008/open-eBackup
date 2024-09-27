package openbackup.system.base.sdk.infrastructure.model.beans;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * 备份网络ip信息
 *
 * @author s00574739
 * @since 2021-09-23
 */
@Data
public class IpAddressInfo {
    @JsonProperty(value = "name")
    private String name;

    @JsonProperty(value = "ips")
    private List<String> ips;
}
