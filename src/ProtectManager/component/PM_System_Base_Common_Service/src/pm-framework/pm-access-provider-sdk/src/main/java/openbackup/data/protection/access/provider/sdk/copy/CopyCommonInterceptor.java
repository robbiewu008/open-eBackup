package openbackup.data.protection.access.provider.sdk.copy;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyInfo;
import openbackup.system.base.sdk.dee.model.CopyCatalogsRequest;

/**
 * 副本拦截器
 *
 * @author t30028453
 * @version OceanProtect DataBackup 1.3.0
 * @since 2023-05-10
 */
public interface CopyCommonInterceptor extends DataProtectionProvider<String> {
    /**
     * 浏览副本中文件和目录信息 构造request
     *
     * @param copy 副本
     * @param catalogsRequest 副本相关的信息
     */
    default void buildCatalogsRequest(Copy copy, CopyCatalogsRequest catalogsRequest) {
    }

    /**
     * 备份构造副本后置流程
     *
     * @param copyInfo 副本
     */
    default void backupBuildCopyPostprocess(CopyInfo copyInfo) {
    }
}
