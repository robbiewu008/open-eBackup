package openbackup.system.base.sdk.auth.api;

import openbackup.system.base.sdk.auth.model.DmeToken;

/**
 * 调用DME平台的Api
 *
 * @author z30062305
 * @version [OceanProtect 1.6.0]
 * @since 2024-07-30
 */
public interface DmeTokenApi {
    /**
     * IAM 校验DME token是否正确，正确则解析
     *
     * @param subjectToken 待校验的token
     * @return DmeToken
     */
    DmeToken verifyAuthToken(String subjectToken);
}