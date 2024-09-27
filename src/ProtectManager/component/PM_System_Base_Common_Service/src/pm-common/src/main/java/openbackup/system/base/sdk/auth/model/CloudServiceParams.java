package openbackup.system.base.sdk.auth.model;

import lombok.Getter;
import lombok.Setter;

/**
 * 云服务参数
 *
 * @author y30021475
 * @since 2023-07-27
 */
@Getter
@Setter
public class CloudServiceParams {
    /**
     * region码
     */
    private String regionCode;

    private String cloudServiceId;

    private String cloudServiceIndexName;

    private String paramId;

    /**
     * 参数key
     */
    private String paramName;

    /**
     * 参数value
     */
    private String paramValue;
}
