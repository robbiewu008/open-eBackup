package openbackup.system.base.sdk.accesspoint.model;

import openbackup.system.base.config.business.initialize.StorageVolumeConfig;

import org.junit.Assert;
import org.junit.Test;

/**
 * StorageVolumeConfig test
 *
 * @author l00347293
 * @since 2021-07-03
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
