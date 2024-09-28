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
package openbackup.data.access.framework.livemount.data;

import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.sdk.copy.model.Copy;

/**
 * mount flow listener test
 *
 */
public class MountFlowListenerTestData extends LiveMountCommonTestData {
    /**
     * get copy uuid object
     *
     * @return copy id
     */
    public static UuidObject getSaveCopyUuidObject() {
        UuidObject uuidObject = new UuidObject();
        uuidObject.setUuid("83445bf0-f601-4509-b6c1-05534318206d");
        return uuidObject;
    }

    /**
     * get saved clone copy
     *
     * @return nodes
     */
    public static Copy getSavedCloneCopy() {
        Copy copy = new Copy();
        copy.setStatus("Normal");
        copy.setResourceId("uuid0");
        copy.setGeneration(1);
        copy.setUuid("83445bf0-f601-4509-b6c1-05534318206d");
        copy.setTimestamp(Long.toString(System.currentTimeMillis()));
        return copy;
    }
}
