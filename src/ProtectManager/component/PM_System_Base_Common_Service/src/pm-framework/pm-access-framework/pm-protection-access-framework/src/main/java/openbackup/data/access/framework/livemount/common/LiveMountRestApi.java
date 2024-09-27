package openbackup.data.access.framework.livemount.common;

import openbackup.data.access.framework.livemount.common.model.LiveMountCloneRequest;
import openbackup.data.access.framework.livemount.common.model.LiveMountCreateCheckParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountExecuteParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountMigrateParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountRefreshParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountUnmountParam;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.sdk.copy.model.CopyInfo;
import openbackup.system.base.sdk.livemount.model.Identity;

import java.util.List;

/**
 * Live Mount Rest Api
 *
 * @author l00272247
 * @since 2020-09-19
 */
public interface LiveMountRestApi {
    /**
     * create live mount pre-check
     *
     * @param identity live mount context
     */
    void createLiveMountPreCheck(Identity<LiveMountCreateCheckParam> identity);

    /**
     * execute live mount
     *
     * @param liveMountExecuteParam live mount execute param
     */
    void executeLiveMount(Identity<LiveMountExecuteParam> liveMountExecuteParam);

    /**
     * destroy live mount
     *
     * @param destroyParamIdentity destroy param identity
     */
    void unmountLiveMount(Identity<LiveMountUnmountParam> destroyParamIdentity);

    /**
     * clone copy
     *
     * @param cloneRequestIdentity clone request identity
     * @return clone copy
     */
    CopyInfo cloneCopy(Identity<LiveMountCloneRequest> cloneRequestIdentity);

    /**
     * update performance setting
     *
     * @param liveMountEntity live mount entity
     */
    void updatePerformanceSetting(Identity<LiveMountEntity> liveMountEntity);

    /**
     * refresh target resource
     *
     * @param identity live mount identity
     * @return target resource uuids
     */
    List<String> refreshTargetResource(Identity<LiveMountRefreshParam> identity);

    /**
     * migrate live mount
     *
     * @param identity live mount migrate param identity
     */
    void migrateLiveMount(Identity<LiveMountMigrateParam> identity);

    /**
     * 添加live mount文件系统名称用于更新
     *
     * @param identity identity
     * @return LiveMountEntity
     */
    LiveMountEntity addLiveMountFileSystemName(Identity<LiveMountEntity> identity);
}
