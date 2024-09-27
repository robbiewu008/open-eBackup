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
package openbackup.data.protection.access.provider.sdk.livemount;

import openbackup.data.protection.access.provider.sdk.base.v2.BaseTask;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Live Mount Task
 *
 * @author l00272247
 * @since 2021-12-27
 */
public class LiveMountCreateTask extends BaseTask {
    private List<ProtectedResource> subObjects;
    private LiveMountPerformance mountQos;
    private Map<String, Object> advanceParams;
    private LiveMountScript scripts;

    public LiveMountScript getScripts() {
        return scripts;
    }

    public void setScripts(LiveMountScript scripts) {
        this.scripts = scripts;
    }

    public List<ProtectedResource> getSubObjects() {
        return subObjects;
    }

    public void setSubObjects(List<ProtectedResource> subObjects) {
        this.subObjects = subObjects;
    }

    public LiveMountPerformance getMountQos() {
        return mountQos;
    }

    public void setMountQos(LiveMountPerformance mountQos) {
        this.mountQos = mountQos;
    }

    public Map<String, Object> getAdvanceParams() {
        return advanceParams;
    }

    public void setAdvanceParams(Map<String, Object> advanceParams) {
        this.advanceParams = advanceParams;
    }

    /**
     * 添加高级参数
     *
     * @param param 待新增的高级参数map
     */
    public void addParameters(Map<String, String> param) {
        if (this.advanceParams == null) {
            this.advanceParams = new HashMap<>();
        }
        this.advanceParams.putAll(param);
    }
}
