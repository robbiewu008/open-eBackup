/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.livemount.provider;

import openbackup.data.access.framework.livemount.common.model.LiveMountCloneRequest;
import openbackup.data.access.framework.livemount.common.model.LiveMountCreateCheckParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountExecuteParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountMigrateParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountRefreshParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountUnmountParam;
import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.system.base.common.model.livemount.LiveMountEntity;

import java.util.List;

/**
 * Live Mount Provider
 *
 * @author l00272247
 * @since 2020-09-17
 */
public interface LiveMountProvider extends DataProtectionProvider<String> {
    /**
     * create live mount pre-check
     *
     * @param liveMountCreateCheckParam live mount create check param
     */
    void createLiveMountPreCheck(LiveMountCreateCheckParam liveMountCreateCheckParam);

    /**
     * execute live mount
     *
     * @param liveMountExecuteParam live mount execute param
     */
    void executeLiveMount(LiveMountExecuteParam liveMountExecuteParam);

    /**
     * unmount live mount
     *
     * @param unmountParam unmount param
     */
    void unmountLiveMount(LiveMountUnmountParam unmountParam);

    /**
     * clone copy
     *
     * @param request request
     * @return clone copy info
     */
    CopyInfoBo cloneCopy(LiveMountCloneRequest request);

    /**
     * update performance setting
     *
     * @param liveMountEntity live mount entity
     */
    void updatePerformanceSetting(LiveMountEntity liveMountEntity);

    /**
     * refresh target resource
     *
     * @param liveMountRefreshParam live mount refresh param
     * @return resource uuid
     */
    List<String> refreshTargetResource(LiveMountRefreshParam liveMountRefreshParam);

    /**
     * migrate live mount
     *
     * @param migrateParam live mount migrate param
     */
    void migrateLiveMount(LiveMountMigrateParam migrateParam);

    /**
     * 添加live mount文件系统名称用于更新
     *
     * @param liveMountEntity liveMountEntity
     * @return LiveMountEntity
     */
    default LiveMountEntity addLiveMountFileSystemName(LiveMountEntity liveMountEntity) {
        return liveMountEntity;
    }
}
