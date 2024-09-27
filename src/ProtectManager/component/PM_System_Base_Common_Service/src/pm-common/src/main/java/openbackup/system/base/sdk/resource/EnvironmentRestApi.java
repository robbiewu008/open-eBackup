package openbackup.system.base.sdk.resource;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.resource.model.Environment;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;

import java.util.List;

/**
 * 虚拟机资源api
 *
 * @author t00482481
 * @since 2020-10-29
 */
@FeignClient(name = "EnvironmentRestApi", url = "${pm-resource-manager.url}/v1",
    configuration = CommonFeignConfiguration.class)
public interface EnvironmentRestApi {
    /**
     * query environment
     *
     * @param page      page
     * @param size      size
     * @param orders    orders
     * @param condition condition
     * @return environment
     */
    @ExterAttack
    @GetMapping("/internal/environments")
    BasePage<Environment> queryEnvironment(@RequestParam("page_no") int page, @RequestParam("page_size") int size,
        @RequestParam("conditions") String condition, @RequestParam("orders") List<String> orders);
}
