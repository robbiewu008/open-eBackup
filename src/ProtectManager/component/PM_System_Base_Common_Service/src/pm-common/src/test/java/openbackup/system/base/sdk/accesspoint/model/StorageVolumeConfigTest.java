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
package openbackup.system.base.sdk.accesspoint.model;

import openbackup.system.base.config.business.initialize.StorageVolumeConfig;

import org.junit.Assert;
import org.junit.Test;

/**
 * StorageVolumeConfig test
 *
 */
public class StorageVolumeConfigTest {
    @Test
    public void testGetVolumeName() {
        String volumeFlag = StorageVolumeConfig.VOLUME_NAME_PREFIX_STANDARD_BACKUP;
        String volumePrefix = "ab";
        String volumeIndex = "01";
        String realVolumeName = volumePrefix + volumeFlag + volumeIndex;
        String volumeName = StorageVolumeConfig.getVolumeName(volumePrefix, volumeFlag, Integer.valueOf(volumeIndex));
        Assert.assertEquals(volumeName, realVolumeName);
    }
}
