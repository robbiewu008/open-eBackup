package openbackup.system.base.security.download;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * 存在该注解表明需要支持校验用户权限完成下载吊销列表权限
 *
 * @author swx1010572
 * @version [OceanProtect X8000 2.1.0]
 * @since 2022-08-19
 */
@Target({ElementType.METHOD, ElementType.ANNOTATION_TYPE})
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
public @interface DownloadRightsControl {
}
