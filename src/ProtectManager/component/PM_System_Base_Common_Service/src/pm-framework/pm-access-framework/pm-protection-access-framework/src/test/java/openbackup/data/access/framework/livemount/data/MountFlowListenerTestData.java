/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.livemount.data;

import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.sdk.copy.model.Copy;

/**
 * mount flow listener test
 *
 * @author h30003246
 * @since 2021-02-25
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
