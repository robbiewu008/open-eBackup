package openbackup.system.base.sdk.cluster.model;

import openbackup.system.base.sdk.cluster.enums.ClusterEnum;

import com.fasterxml.jackson.annotation.JsonAlias;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Storage user info
 *
 * @author p30001902
 * @since 2020-12-11
 */
@Data
public class StorageUserInfo {
    @JsonProperty("userId")
    @JsonAlias("ID")
    private String userId;

    @JsonProperty("roleId")
    @JsonAlias("ROLEID")
    private String roleId;

    @JsonProperty("createTime")
    @JsonAlias("CREATETIME")
    private String createTime;

    // user type
    private ClusterEnum.DoradoCreateUserType userType;

    private String esn;
}
