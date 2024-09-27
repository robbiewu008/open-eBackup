package openbackup.data.access.framework.core.common.enums.v2;


import openbackup.data.access.framework.core.common.enums.v2.CopyTypeEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * DmeCopyTypeEnum LLT
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-17
 */
public class CopyTypeEnumTest {
    @Test
    public void test() {
        Assert.assertEquals(CopyTypeEnum.getCopyType("full").getCopyType(), "full");
        Assert.assertEquals(CopyTypeEnum.getCopyType("increment").getCopyType(), "difference_increment");
        Assert.assertEquals(CopyTypeEnum.getCopyType("foreverIncrement").getCopyType(), "permanent_increment");
        Assert.assertEquals(CopyTypeEnum.getCopyType("diff").getCopyType(), "cumulative_increment");
        Assert.assertEquals(CopyTypeEnum.getCopyType("log").getCopyType(), "log");
        Assert.assertEquals(CopyTypeEnum.getCopyType("s3Archive").getCopyType(), "s3Archive");
        Assert.assertEquals(CopyTypeEnum.getCopyType("replication").getCopyType(), "replication");
        Assert.assertEquals(CopyTypeEnum.getCopyType("tapeArchive").getCopyType(), "tapeArchive");
        Assert.assertEquals(CopyTypeEnum.getCopyType("nativeSnapshot").getCopyType(), "snapshot");
    }
}
