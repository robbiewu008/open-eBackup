package openbackup.data.access.framework.core.common.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;
import lombok.ToString;

/**
 * 存储设备的信息实体类
 *
 * @author z00425178
 * @since 2021-11-13
 */
@ToString(exclude = "password")
@Getter
@Setter
public class RestoreStorageInfo {
    /**
     * 存储设备的ip或域名
     */
    @JsonProperty("ip")
    private String ip;

    /**
     * 端口
     */
    @JsonProperty("port")
    private String port;

    /**
     * 用户名
     */
    @JsonProperty("username")
    private String username;

    /**
     * 密码
     */
    @JsonProperty("password")
    private String password;

    /**
     * 存储设备类型
     */
    @JsonProperty("storage_type")
    private String storageType;

    /**
     * 存储协议
     */
    @JsonProperty("protocol")
    private String protocol;

    /**
     * 设备esn
     */
    @JsonProperty("device_esn")
    private String deviceEsn;

    /**
     * 存储单元id
     */
    @JsonProperty("storage_id")
    private String storageId;
}
