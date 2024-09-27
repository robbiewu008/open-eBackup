package openbackup.data.access.framework.core.common.enums;


import openbackup.data.access.framework.core.common.enums.DmeJobStatusEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * RestoreTypeEnum LLT
 *
 * @author z30027603
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-07-26
 */
public class DmeJobStatusEnumTest {
    @Test
    public void from_status_success() {
        Assert.assertEquals(DmeJobStatusEnum.fromStatus(1).getTypeName(), 1);
        Assert.assertEquals(DmeJobStatusEnum.fromStatus(2).getTypeName(), 2);
        Assert.assertEquals(DmeJobStatusEnum.fromStatus(3).getTypeName(), 3);
        Assert.assertEquals(DmeJobStatusEnum.fromStatus(4).getTypeName(), 4);
        Assert.assertEquals(DmeJobStatusEnum.fromStatus(5).getTypeName(), 5);
        Assert.assertEquals(DmeJobStatusEnum.fromStatus(6).getTypeName(), 6);
        Assert.assertEquals(DmeJobStatusEnum.fromStatus(13).getTypeName(), 13);
    }

}
