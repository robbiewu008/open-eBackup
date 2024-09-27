package openbackup.data.access.framework.copy.mng.provider;

import org.springframework.stereotype.Component;

/**
 * 默认副本删除拦截器
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-22
 */
@Component
public class DefaultCopyDeleteInterceptor extends BaseCopyDeleteInterceptor {
    @Override
    public boolean applicable(String object) {
        return false;
    }
}
