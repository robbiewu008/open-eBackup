package openbackup.system.base.sdk.storage;

import openbackup.system.base.common.rest.CommonFeignConfiguration;

import org.springframework.cloud.openfeign.FeignClient;

/**
 * 生产存储的Feign客户端
 *
 * @author y00559272
 * @version [A8000 1.0.0]
 * @since 2021-01-05
 */
@FeignClient(
        name = "productStorageClientRestApi",
        url = "${service.url.pm-system-base}/v1",
        configuration = CommonFeignConfiguration.class)
public interface ProductStorageClientRestApi extends ProductStorageRestApi {
}
