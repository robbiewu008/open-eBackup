/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.livemount.data;

import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

/**
 * mount flow listener test
 *
 * @author h30003246
 * @since 2021-02-25
 */
public class UnmountFlowListenerTestData extends LiveMountCommonTestData {
    /**
     * get clone copy
     *
     * @return copy
     */
    public static Copy getVMWareMountedCopy() {
        Copy copy = new Copy();
        copy.setStatus("Normal");
        copy.setResourceId("uuid0");
        copy.setGeneration(1);
        copy.setTimestamp(Long.toString(System.currentTimeMillis()));
        copy.setResourceType(ResourceTypeEnum.VM.getType());
        copy.setResourceSubType(ResourceSubTypeEnum.VMWARE.getType());
        return copy;
    }
}
