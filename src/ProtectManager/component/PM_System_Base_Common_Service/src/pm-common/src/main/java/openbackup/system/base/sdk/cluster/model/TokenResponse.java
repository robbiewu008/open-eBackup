package openbackup.system.base.sdk.cluster.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Token Response
 *
 * @author l00272247
 * @since 2020-11-20
 */
@Data
public class TokenResponse {
    private String token;

    /**
     * 密码是否需要初始化。true需要；false不需要
     */
    @JsonProperty("modifyPassword")
    private boolean shouldModifyPassword;
}
