package openbackup.system.base.common.model.repository;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.math.BigDecimal;

/**
 * 功能描述
 *
 * @author y00413474
 * @since 2020-07-14
 */
@Data
public class S3ConnectCheckResponse {
    @JsonProperty("Status")
    private int status;

    @JsonProperty("Capacity")
    private BigDecimal capacity;

    @JsonProperty("Quota")
    private BigDecimal quota;
}
