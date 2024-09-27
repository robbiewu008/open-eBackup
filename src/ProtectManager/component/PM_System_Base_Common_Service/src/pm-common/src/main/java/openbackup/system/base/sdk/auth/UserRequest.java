package openbackup.system.base.sdk.auth;

import openbackup.system.base.common.enums.UserTypeEnum;
import openbackup.system.base.sdk.auth.model.ResourceSetAuthorization;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import org.apache.logging.log4j.util.Strings;

import java.util.HashSet;
import java.util.Set;

/**
 * 功能描述
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2019-11-26
 */
@Getter
@Setter
public class UserRequest {
    private String userId;

    private String userName;

    private String userPassword;

    private String confirmPassword;

    private String description = Strings.EMPTY;

    private Set<String> rolesIdsSet;

    @JsonProperty("sessionControl")
    private boolean isSessionControl;

    private Set<ResourceSetAuthorization> resourceSetAuthorizationSets = new HashSet<>();

    private int sessionLimit = 1;

    private String userType = UserTypeEnum.COMMON.getValue();

    private int loginType;

    private String dynamicCodeEmail = Strings.EMPTY;
}
