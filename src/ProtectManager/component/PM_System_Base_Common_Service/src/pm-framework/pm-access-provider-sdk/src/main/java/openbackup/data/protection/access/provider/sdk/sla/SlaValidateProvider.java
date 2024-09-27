package openbackup.data.protection.access.provider.sdk.sla;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import com.huawei.oceanprotect.sla.sdk.dto.SlaBase;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;

/**
 * SLA应用校验扩展接口
 * <p>
 * 应用插件实现此接口可以定制SLA校验框架的行为并且可以实现自己特有的校验逻辑
 * </p>
 *
 * @author y00559272
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/4/29
 **/
public interface SlaValidateProvider extends DataProtectionProvider<String> {
    /**
     * 获取SLA校验器配置
     * <p>
     * 1.应用插件可以指定支持的策略规格配置
     * 2.应用插件可以根据自己的业务情况进行配置规则的开关，用于跳过框架中某些校验规则
     * 3.接口为可选实现，不实现的情况下配置参考默认方法 {@link SlaValidateConfig#defaults()}
     * </p>
     *
     * @return 校验配置类 {@link SlaValidateConfig}
     */
    default SlaValidateConfig getConfig() {
        return SlaValidateConfig.defaults();
    }

    /**
     * 应用插件执行应用自定义校验逻辑
     *
     * @param slaBase 基础的SLA信息
     */
    default void validateSLA(SlaBase slaBase) {
    }
}
