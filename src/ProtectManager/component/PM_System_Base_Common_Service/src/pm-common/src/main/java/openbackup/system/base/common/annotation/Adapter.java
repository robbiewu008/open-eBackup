/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.common.annotation;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * 多service注解实现
 *
 * @author l00272247
 * @since 2019-12-30
 */
@Target({ElementType.TYPE})
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
public @interface Adapter {
    /**
     * service别名
     *
     * @return service别名
     */
    String[] names();

    /**
     * 抽象类类名
     *
     * @return 抽象类类名
     */
    String abstractClass();
}
