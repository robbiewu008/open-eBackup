package openbackup.access.framework.resource.persistence.model;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import com.baomidou.mybatisplus.annotation.TableField;

/**
 * Protected Environment Po
 *
 * @author l00272247
 * @since 2021-10-18
 */
public class ProtectedEnvironmentPo extends ProtectedResourcePo {
    private String endpoint;

    private Integer port;

    private String linkStatus;

    @TableField("user_name")
    private String username;

    private String password;

    private String location;

    private String osType;

    private String osName;

    private boolean isCluster = false;

    // 定时扫描环境的时间间隔，单位为秒
    private Integer scanInterval;

    public String getEndpoint() {
        return endpoint;
    }

    public void setEndpoint(String endpoint) {
        this.endpoint = endpoint;
    }

    public Integer getPort() {
        return port;
    }

    public void setPort(Integer port) {
        this.port = port;
    }

    public String getLinkStatus() {
        return linkStatus;
    }

    public void setLinkStatus(String linkStatus) {
        this.linkStatus = linkStatus;
    }

    public String getUsername() {
        return username;
    }

    public void setUsername(String username) {
        this.username = username;
    }

    public String getPassword() {
        return password;
    }

    public void setPassword(String password) {
        this.password = password;
    }

    public String getLocation() {
        return location;
    }

    public void setLocation(String location) {
        this.location = location;
    }

    public String getOsType() {
        return osType;
    }

    public void setOsType(String osType) {
        this.osType = osType;
    }

    public String getOsName() {
        return osName;
    }

    public void setOsName(String osName) {
        this.osName = osName;
    }

    public Integer getScanInterval() {
        return scanInterval;
    }

    public void setScanInterval(Integer scanInterval) {
        this.scanInterval = scanInterval;
    }

    public boolean isCluster() {
        return isCluster;
    }

    public void setCluster(boolean isCluster) {
        this.isCluster = isCluster;
    }

    /**
     * create a ProtectedEnvironment
     *
     * @return ProtectedEnvironment
     */
    @Override
    protected ProtectedResource create() {
        return new ProtectedEnvironment();
    }
}
