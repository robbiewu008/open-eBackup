/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.copy;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

/**
 * Copy Resource Summary
 *
 * @author l00272247
 * @since 2020-11-09
 */
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class CopyResourceSummaryBo extends CopyResourceBase {
    private String slaName;

    private int copyCount;

    public String getSlaName() {
        return slaName;
    }

    public void setSlaName(String slaName) {
        this.slaName = slaName;
    }

    public int getCopyCount() {
        return copyCount;
    }

    public void setCopyCount(int copyCount) {
        this.copyCount = copyCount;
    }
}
