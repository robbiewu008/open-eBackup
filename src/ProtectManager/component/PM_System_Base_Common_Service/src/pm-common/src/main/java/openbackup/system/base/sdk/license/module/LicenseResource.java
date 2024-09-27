package openbackup.system.base.sdk.license.module;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * dorado activelicense 接口返回内容
 *
 * @author g00500588
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/6/8
 */
@Getter
@Setter
public class LicenseResource {
    @JsonProperty("FeatureId")
    private String featureId;

    @JsonProperty("ResourceNum")
    private String resourceNum;
}
