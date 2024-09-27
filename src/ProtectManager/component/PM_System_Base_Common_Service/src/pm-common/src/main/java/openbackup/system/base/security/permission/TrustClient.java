package openbackup.system.base.security.permission;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * 校验客户端ip是否授信
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-29
 */
@Target({ElementType.METHOD})
@Retention(RetentionPolicy.RUNTIME)
@Documented
public @interface TrustClient {
}
