package openbackup.data.access.framework.restore.service;

import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;
import com.huawei.oceanprotect.functionswitch.template.service.FunctionSwitchService;
import openbackup.system.base.common.license.LicenseValidateService;
import openbackup.system.base.sdk.license.enums.FunctionEnum;

import org.springframework.stereotype.Service;

/**
 * 恢复任务校验服务
 *
 * @author y00559272
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/7/25
 **/
@Service
public class RestoreValidateService {
    private final LicenseValidateService licenseValidateService;
    private final FunctionSwitchService functionSwitchService;

    /**
     * 恢复任务校验服务构造函数
     *
     * @param licenseValidateService license校验服务
     * @param functionSwitchService 功能开关服务
     */
    public RestoreValidateService(LicenseValidateService licenseValidateService,
        FunctionSwitchService functionSwitchService) {
        this.licenseValidateService = licenseValidateService;
        this.functionSwitchService = functionSwitchService;
    }

    /**
     * 校验恢复任务license权限
     *
     * @param resourceSubType 资源子类型
     * @param type 恢复任务类型枚举
     */
    public void checkLicense(String resourceSubType, RestoreTypeEnum type) {
        switch (type) {
            case CR:
                licenseValidateService.validate(resourceSubType, FunctionEnum.RECOVERY);
                break;
            case IR:
                licenseValidateService.validate(resourceSubType, FunctionEnum.INSTANT_RECOVERY);
                break;
            case FLR:
                licenseValidateService.validate(resourceSubType, FunctionEnum.FINE_GRAINED_RECOVERY);
                break;
            default:
                throw new IllegalStateException("Unsupported restore type: " + type);
        }
    }
}
