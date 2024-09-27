/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.openstack.adapter.interceptor;

import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Lazy;
import org.springframework.web.servlet.config.annotation.InterceptorRegistry;
import org.springframework.web.servlet.config.annotation.WebMvcConfigurer;

/**
 * OpenStack北向接口MVC配置类
 *
 * @author w00616953
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-11
 */
@Configuration
public class OpenStackWebMvcConfig implements WebMvcConfigurer {
    private final OpenStackInterceptor interceptor;

    public OpenStackWebMvcConfig(@Lazy OpenStackInterceptor interceptor) {
        this.interceptor = interceptor;
    }

    @Override
    public void addInterceptors(InterceptorRegistry registry) {
        registry.addInterceptor(interceptor)
            .addPathPatterns("/v2/backup_jobs/**", "/v2/backup_restore/**", "/v2/backup_copies/**",
                "/v2/backup_quota/**");
        WebMvcConfigurer.super.addInterceptors(registry);
    }
}
