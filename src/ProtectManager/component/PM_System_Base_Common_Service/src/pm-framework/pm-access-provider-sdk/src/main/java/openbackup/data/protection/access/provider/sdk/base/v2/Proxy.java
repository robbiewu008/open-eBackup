package openbackup.data.protection.access.provider.sdk.base.v2;

import com.fasterxml.jackson.annotation.JsonProperty;

import java.util.Map;

/**
 * 存储库代理信息对象
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/1/15
 **/
public class Proxy {
    /**
     * 是否开启代理
     */
    @JsonProperty("enabled")
    private boolean isEnabled;

    /**
     * 代理服务器域名/IP
     */
    @JsonProperty("hostname")
    private String hostName;

    /**
     * 代理端口
     */
    private int port;

    /**
     * 代理用户名
     */
    private String userName;

    /**
     * 代理密码
     */
    private String password;

    /**
     * 扩展参数
     */
    private Map<String, Object> extendInfo;

    public boolean isEnabled() {
        return isEnabled;
    }

    public void setEnabled(boolean isEnabled) {
        this.isEnabled = isEnabled;
    }

    public String getHostName() {
        return hostName;
    }

    public void setHostName(String hostName) {
        this.hostName = hostName;
    }

    public int getPort() {
        return port;
    }

    public void setPort(int port) {
        this.port = port;
    }

    public String getUserName() {
        return userName;
    }

    public void setUserName(String userName) {
        this.userName = userName;
    }

    public String getPassword() {
        return password;
    }

    public void setPassword(String password) {
        this.password = password;
    }

    public Map<String, Object> getExtendInfo() {
        return extendInfo;
    }

    public void setExtendInfo(Map<String, Object> extendInfo) {
        this.extendInfo = extendInfo;
    }
}
