package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 双活pairBO
 *
 * @author mwx776342
 * @since 2022/03/17
 */
@Data
public class HyperMetroPairBo {
    /**
     * ID
     */
    @JsonProperty("ID")
    private String id;

    /**
     * 判断 是否是主端(ISPRIMARY, true：是主端 ，false：不是主端)
     */
    @JsonProperty("ISPRIMARY")
    private boolean isPrimary;

    /**
     * 双活域id
     */
    @JsonProperty("DOMAINID")
    private String domainId;
}
