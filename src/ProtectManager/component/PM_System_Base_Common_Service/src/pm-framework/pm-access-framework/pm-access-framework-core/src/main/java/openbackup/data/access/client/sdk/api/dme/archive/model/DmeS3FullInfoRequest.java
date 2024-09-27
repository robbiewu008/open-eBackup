package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * S3存储信息
 *
 * @author y00490893
 * @since 2020-12-19
 */
@Data
public class DmeS3FullInfoRequest {
    @JsonProperty("Url")
    private String url;

    @JsonProperty("Port")
    private int port;

    @JsonProperty("Username")
    private String username;

    @JsonProperty("Password")
    private String password;

    @JsonProperty("Certificate")
    private String certificate;

    @JsonProperty("BucketName")
    private String bucketName;

    @JsonProperty("ProxyEnable")
    private boolean isProxyEnable;

    @JsonProperty("ProxyHostName")
    private String proxyHostName;

    @JsonProperty("ProxyPort")
    private String proxyPort;

    @JsonProperty("ProxyUserName")
    private String proxyUserName;

    @JsonProperty("ProxyUserPassword")
    private String proxyUserPassword;

    @JsonProperty("SpeedUpEnable")
    private boolean isSpeedUpEnable;

    @JsonProperty("SpeedUpMethod")
    private String speedUpMethod;

    @JsonProperty("UseHttps")
    private boolean isUseHttps;

    /**
     * 连接模式
     */
    private Integer connectType;

    /**
     * 云存储类型
     */
    private Integer cloudType;
}
