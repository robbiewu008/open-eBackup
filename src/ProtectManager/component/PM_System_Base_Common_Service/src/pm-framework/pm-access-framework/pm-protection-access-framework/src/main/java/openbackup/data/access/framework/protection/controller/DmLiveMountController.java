/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.data.access.framework.protection.controller;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.livemount.common.LiveMountRestApi;
import openbackup.data.access.framework.livemount.common.model.LiveMountCloneRequest;
import openbackup.data.access.framework.livemount.common.model.LiveMountCreateCheckParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountExecuteParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountMigrateParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountRefreshParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountUnmountParam;
import openbackup.data.access.framework.livemount.provider.LiveMountProvider;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.sdk.copy.model.CopyInfo;
import openbackup.system.base.sdk.livemount.model.Identity;

import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;
import org.springframework.web.bind.annotation.RequestBody;

import java.util.List;
import java.util.function.BiConsumer;
import java.util.function.BiFunction;

/**
 * Live Mount Controller
 *
 * @author l00272247
 * @since 2020-09-19
 */
@Component
public class DmLiveMountController implements LiveMountRestApi {
    @Autowired
    private ProviderManager providerManager;

    /**
     * create live mount pre-check
     *
     * @param identity live mount context
     */
    @Override
    public void createLiveMountPreCheck(@RequestBody Identity<LiveMountCreateCheckParam> identity) {
        process(identity, LiveMountProvider::createLiveMountPreCheck);
    }

    /**
     * execute live mount
     *
     * @param identity live mount execute param identity
     */
    @Override
    public void executeLiveMount(@RequestBody Identity<LiveMountExecuteParam> identity) {
        process(identity, LiveMountProvider::executeLiveMount);
    }

    /**
     * destroy param
     *
     * @param identity destroy param identity
     */
    @Override
    public void unmountLiveMount(@RequestBody Identity<LiveMountUnmountParam> identity) {
        process(identity, LiveMountProvider::unmountLiveMount);
    }

    /**
     * clone copy
     *
     * @param identity identity
     * @return clone copy
     */
    @Override
    public CopyInfo cloneCopy(@RequestBody Identity<LiveMountCloneRequest> identity) {
        final CopyInfoBo copyInfoBo = process(identity, LiveMountProvider::cloneCopy);
        CopyInfo copyInfo = new CopyInfo();
        BeanUtils.copyProperties(copyInfoBo, copyInfo);
        return copyInfo;
    }

    private <A, B> B process(Identity<A> identity, BiFunction<LiveMountProvider, A, B> function) {
        String type = identity.getType(); // resource type
        LiveMountProvider provider = providerManager.findProvider(LiveMountProvider.class, type);
        return function.apply(provider, identity.getData());
    }

    private <A> void process(Identity<A> identity, BiConsumer<LiveMountProvider, A> function) {
        process(identity, (provider, arg) -> {
            function.accept(provider, arg);
            return null;
        });
    }

    /**
     * update performance setting
     *
     * @param identity live mount entity
     */
    @Override
    public void updatePerformanceSetting(@RequestBody Identity<LiveMountEntity> identity) {
        process(identity, LiveMountProvider::updatePerformanceSetting);
    }

    /**
     * refresh target resource
     *
     * @param identity identity
     * @return target resource uuid
     */
    @Override
    public List<String> refreshTargetResource(@RequestBody Identity<LiveMountRefreshParam> identity) {
        return process(identity, LiveMountProvider::refreshTargetResource);
    }

    /**
     * migrateLiveMount
     *
     * @param identity identity
     */
    @Override
    public void migrateLiveMount(@RequestBody Identity<LiveMountMigrateParam> identity) {
        process(identity, LiveMountProvider::migrateLiveMount);
    }

    /**
     * 添加live mount文件系统名称用于更新
     *
     * @param identity identity
     * @return LiveMountEntity
     */
    @Override
    public LiveMountEntity addLiveMountFileSystemName(@RequestBody Identity<LiveMountEntity> identity) {
        return process(identity, LiveMountProvider::addLiveMountFileSystemName);
    }
}
