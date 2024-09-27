/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.common.annotation;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * 更新路由切面注解
 *
 * @author n30046257
 * @since 2024-02-27
 */

@Target({ElementType.METHOD, ElementType.TYPE})
@Retention(RetentionPolicy.RUNTIME)
public @interface Routing {
    /**
     * 目标IP
     *
     * @return 目标IP
     */
    String destinationIp() default "";

    /**
     * required data
     *
     * @return required data
     */
    String[] requires() default {};

    /**
     * 添加路由的网络平面
     *
     * @return 添加路由的网络平面
     */
    String onNetPlane() default "backup";
}
