package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 双活domainBO
 *
 * @author mwx776342
 * @since 2022/03/23
 */
@Data
public class HyperMetroDomainBo {
    /**
     * ID
     */
    @JsonProperty("ID")
    private String id;

    /**
     * 双活主备站点(CONFIGROLE, 0-备 1-主)
     */
    @JsonProperty("CONFIGROLE")
    private String configRole;

    /**
     * 双活工作模式：
     * 0:双活AA模式；
     * 1:同步模式；
     * 2:双活AP模式。
     */
    private String workMode;
}
