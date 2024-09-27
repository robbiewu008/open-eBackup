package openbackup.data.access.framework.livemount.provider.mock;

import openbackup.data.access.framework.livemount.common.model.LiveMountCreateCheckParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountExecuteParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountMigrateParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountRefreshParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountUnmountParam;
import openbackup.data.access.framework.livemount.provider.AbstractLiveMountProvider;
import openbackup.data.access.framework.livemount.provider.CloneCopyParam;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountModifyParam;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountRemoveQosParam;
import openbackup.system.base.common.model.livemount.LiveMountEntity;

import org.springframework.stereotype.Component;

import java.util.List;

/**
 * AbstractLiveMountProvider的测试实现
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-20
 */
@Component
public class TestAbstractLiveMountProvider extends AbstractLiveMountProvider {
    @Override
    public void unmountLiveMount(LiveMountUnmountParam unmountParam) {
    }

    @Override
    public List<String> refreshTargetResource(LiveMountRefreshParam liveMountRefreshParam) {
        return null;
    }

    @Override
    public void migrateLiveMount(LiveMountMigrateParam migrateParam) {
    }

    @Override
    public LiveMountEntity addLiveMountFileSystemName(LiveMountEntity liveMountEntity) {
        return liveMountEntity;
    }

    @Override
    public void executeLiveMount(LiveMountExecuteParam liveMountExecuteParam) {
    }

    @Override
    public void createLiveMountPreCheck(LiveMountCreateCheckParam liveMountCreateCheckParam) {
    }

    @Override
    protected void cloneBackup(CloneCopyParam cloneCopyParam) {
    }

    @Override
    protected void removeLiveMountQos(LiveMountRemoveQosParam param) {
    }

    @Override
    protected void modifyLiveMountQos(LiveMountModifyParam param) {
        param.setAppType("alter-type");
    }

    @Override
    public boolean applicable(String object) {
        return true;
    }
}
