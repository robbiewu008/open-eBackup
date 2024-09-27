package openbackup.data.protection.access.provider.sdk.resource;

import com.fasterxml.jackson.annotation.JsonIgnore;

import lombok.ToString;

/**
 * Protected Environment
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2020-06-17
 */
@ToString(exclude = {"password"})
public class ProtectedEnvironment extends ProtectedResource {
    // 一小时
    private static final int ONE_HOUR = 3600;

    private String endpoint;

    private Integer port;

    private String linkStatus;

    private String username;

    private String password;

    private String location;

    private String osType;

    private String osName;

    private Boolean isCluster;

    // 1，注册安装，2，更新
    private String registerType;

    // 定时扫描环境的时间间隔，单位为秒
    private Integer scanInterval = ONE_HOUR;

    // 触发定时扫描任务的开始时间
    @JsonIgnore
    private String startDate;

    @Override
    public String getEndpoint() {
        return endpoint;
    }

    @Override
    public void setEndpoint(String endpoint) {
        this.endpoint = endpoint;
    }

    @Override
    public Integer getPort() {
        return port;
    }

    @Override
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

    public Boolean isCluster() {
        return isCluster;
    }

    public void setCluster(Boolean isCluster) {
        this.isCluster = isCluster;
    }

    public String getStartDate() {
        return startDate;
    }

    public void setStartDate(String startDate) {
        this.startDate = startDate;
    }

    public String getRegisterType() {
        return registerType;
    }

    public void setRegisterType(String registerType) {
        this.registerType = registerType;
    }
}
