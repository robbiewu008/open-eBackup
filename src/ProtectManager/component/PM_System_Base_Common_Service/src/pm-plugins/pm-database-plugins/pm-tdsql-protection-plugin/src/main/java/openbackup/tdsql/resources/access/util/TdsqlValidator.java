package openbackup.tdsql.resources.access.util;

import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.MapUtils;

import java.util.Map;

/**
 * 功能描述
 *
 * @author z30047175
 * @since 2023-05-23
 */
@Slf4j
public class TdsqlValidator {
    private TdsqlValidator() {
    }

    /**
     * 创建/更新Tdsql集群时，校验参数
     *
     * @param protectedEnvironment 集群环境
     */
    public static void checkTdsql(ProtectedEnvironment protectedEnvironment) {
        verifyEnvName(protectedEnvironment.getName());
        Map<String, String> extendInfo = protectedEnvironment.getExtendInfo();
        if (MapUtils.isEmpty(extendInfo)) {
            log.error("TDSQL cluster extendInfo is null.");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "TDSQL cluster extendInfo is null.");
        }
    }

    private static void verifyEnvName(String name) {
        EnvironmentParamCheckUtil.checkEnvironmentNameEmpty(name);
        EnvironmentParamCheckUtil.checkEnvironmentNamePattern(name);
    }
}
