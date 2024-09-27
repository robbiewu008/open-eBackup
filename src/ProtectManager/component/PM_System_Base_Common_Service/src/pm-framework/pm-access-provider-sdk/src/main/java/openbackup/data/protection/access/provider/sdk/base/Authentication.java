package openbackup.data.protection.access.provider.sdk.base;

import java.util.Map;

/**
 * 认证信息
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-11-19
 */
public class Authentication {
    /**
     * 无认证
     */
    public static final int NO_AUTH = 0;

    /**
     * OS用户名和密码认证
     */
    public static final int OS_PASSWORD = 1;

    /**
     * 应用密码认证
     */
    public static final int APP_PASSWORD = 2;

    /**
     * LADP认证
     */
    public static final int LADP = 3;

    /**
     * AKSK认证
     */
    public static final int AKSK = 4;

    /**
     * KERBEROS认证
     */
    public static final int KERBEROS = 5;

    /**
     * Token认证
     */
    public static final int TOKEN = 6;

    /**
     * OAUTH认证
     */
    public static final int OAUTH2 = 7;

    /**
     * 其他认证
     */
    public static final int OTHER = 8;

    /*
     * 0-NO_AUTH, 1-OS_PASSWORD, 2-APP_PASSWORD，3-LADP, 4-AKSK, 5-KERBEROS, 6-TOKEN, 7-OAUTH2,
     * 8-OTHER。1到3,authKey填用户名,authPwd填密码， 4 authKey填AK， authPwd填SK, 5,6,7,8认证认证信息在扩展字段中扩充。
     */
    private int authType;

    // 使用用户名密码认证时该字段填用户名，使用AK/SK认证时该字段填充AK
    private String authKey;

    // 使用用户名密码认证时该字段填密码，使用AK/SK认证时该字段填充SK
    private String authPwd;

    // 认证扩展信息，使用其他方式认证时，认证信息放在扩展该字段里
    private Map<String, String> extendInfo;

    public int getAuthType() {
        return authType;
    }

    public void setAuthType(int authType) {
        this.authType = authType;
    }

    public String getAuthKey() {
        return authKey;
    }

    public void setAuthKey(String authKey) {
        this.authKey = authKey;
    }

    public String getAuthPwd() {
        return authPwd;
    }

    public void setAuthPwd(String authPwd) {
        this.authPwd = authPwd;
    }

    public Map<String, String> getExtendInfo() {
        return extendInfo;
    }

    public void setExtendInfo(Map<String, String> extendInfo) {
        this.extendInfo = extendInfo;
    }
}
