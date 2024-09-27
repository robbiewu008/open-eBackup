package openbackup.system.base.sdk.system;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.system.model.ConfigStatus;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.ResponseBody;

/**
 * 查询系统是否初始化完毕
 *
 * @author w30042425
 * @since 2022-12-28
 */
@FeignClient(name = "SystemFeignClient", url = "${pm-system-base.url}/v1/system",
    configuration = CommonFeignConfiguration.class)
public interface SystemRestApi {
    /**
     * 查询初始化情况
     *
     * @return 初始化状态
     */
    @ExterAttack
    @GetMapping(value = "/internal/initConfig")
    @ResponseBody
    ConfigStatus getInitConfigInternal();
}
