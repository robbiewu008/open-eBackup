package openbackup.data.access.framework.copy.mng.util;

import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.util.BeanTools;

import org.junit.Assert;
import org.junit.Test;

import java.util.Arrays;
import java.util.List;
import java.util.OptionalInt;
import java.util.stream.Collectors;

/**
 * CopyUtilTest
 *
 * @author dwx1009286
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-12
 */
public class CopyUtilTest {
    /**
     * 用例场景：获取下个指定类型的副本
     * 前置条件：传入差异副本类型，查出gn值之后的第一差异副本
     * 检查点：获取成功
     */
    @Test
    public void get_next_copy_by_type_success() {
        List<Copy> copies = generateBackupCopies();
        Copy nextCopyByType = CopyUtil.getNextCopyByType(copies, BackupTypeConstants.CUMULATIVE_INCREMENT, 1);
        Assert.assertEquals(7, nextCopyByType.getGn());
    }

    /**
     * 用例场景：获取下个全量副本
     * 前置条件：传入全量副本类型，查出gn值之后的第一全量副本
     * 检查点：获取成功
     */
    @Test
    public void get_next_full_copy_success() {
        List<Copy> copies = generateBackupCopies();
        Copy nextCopyByType = CopyUtil.getNextFullCopy(copies, 1);
        Assert.assertEquals(6, nextCopyByType.getGn());
    }

    /**
     * 用例场景：获取下个增量副本
     * 前置条件：传入增量副本类型，查出gn值之后的第一增量副本
     * 检查点：获取成功
     */
    @Test
    public void get_next_difference_copy_success() {
        List<Copy> copies = generateBackupCopies();
        Copy nextCopyByType = CopyUtil.getNextDifferenceCopy(copies, 1);
        Assert.assertEquals(2, nextCopyByType.getGn());
    }

    /**
     * 用例场景：获取指定类型副本
     * 前置条件：传入全量副本类型，查出所有全量副本
     * 检查点：获取成功
     */
    @Test
    public void get_copies_by_copy_type_success() {
        List<Copy> copies = generateBackupCopies();
        List<Integer> gns = CopyUtil.getCopiesByCopyType(copies, BackupTypeConstants.FULL)
            .stream()
            .map(Copy::getGn)
            .collect(Collectors.toList());
        Assert.assertEquals(3, gns.size());
        List<Integer> assertGns = Arrays.asList(1, 6, 10);
        gns.removeAll(assertGns);
        Assert.assertEquals(0, gns.size());
    }

    /**
     * 用例场景：获取两个副本之间的所有副本
     * 前置条件：副本存在且中间有副本
     * 检查点：获取成功
     */
    @Test
    public void get_copies_uuids_between_two_copy_type_success() {
        List<Copy> copies = generateBackupCopies();
        Copy copy = new Copy();
        copy.setGn(1);
        Copy nextCopy = new Copy();
        nextCopy.setGn(5);
        List<String> uuids = CopyUtil.getCopyUuidsBetweenTwoCopy(copies, copy, nextCopy);
        Assert.assertEquals(3, uuids.size());
        List<String> assertUuids = Arrays.asList("difference_01", "log_01", "difference_02");
        uuids.removeAll(assertUuids);
        Assert.assertEquals(0, uuids.size());
    }

    /**
     * 用例场景：匹配副本类型和副本生成类型
     * 前置条件：传入类型与断言的匹配
     * 检查点：匹配成功返回true
     */
    @Test
    public void match_copy_type_success() {
        Copy copy = new Copy();
        copy.setGn(2);
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        copy.setBackupType(BackupTypeConstants.DIFFERENCE_INCREMENT.getAbBackupType());
        Assert.assertTrue(CopyUtil.match(CopyGeneratedByEnum.BY_BACKUP, copy.getGeneratedBy()));
        Assert.assertTrue(CopyUtil.match(BackupTypeConstants.DIFFERENCE_INCREMENT, copy.getBackupType()));
    }

    /**
     * 用例场景：获取gn值较小的副本，为空则返回另外一个
     * 前置条件：传入两个副本
     * 检查点：获取成功
     */
    @Test
    public void get_smaller_copy_success() {
        Copy copy1 = new Copy();
        copy1.setGn(1);
        Copy copy2 = new Copy();
        copy2.setGn(2);
        Assert.assertEquals(1, CopyUtil.getSmallerCopy(copy1, copy2).getGn());
    }

    private List<Copy> generateBackupCopies() {
        // 全增日增日全差差日全
        Copy fullCopy1 = new Copy();;
        fullCopy1.setGn(1);
        fullCopy1.setUuid("full_01");
        fullCopy1.setBackupType(BackupTypeConstants.FULL.getAbBackupType());
        fullCopy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        Copy differenceCopy1 = new Copy();;
        differenceCopy1.setGn(2);
        differenceCopy1.setUuid("difference_01");
        differenceCopy1.setBackupType(BackupTypeConstants.DIFFERENCE_INCREMENT.getAbBackupType());
        differenceCopy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        Copy logCopy1 = new Copy();
        logCopy1.setGn(3);
        logCopy1.setUuid("log_01");
        logCopy1.setBackupType(BackupTypeConstants.LOG.getAbBackupType());
        logCopy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        Copy differenceCopy2 = BeanTools.copy(differenceCopy1, Copy::new);
        differenceCopy2.setGn(4);
        differenceCopy2.setUuid("difference_02");
        Copy logCopy2 = BeanTools.copy(logCopy1, Copy::new);
        logCopy2.setGn(5);
        logCopy2.setUuid("log_02");
        Copy fullCopy2 = BeanTools.copy(fullCopy1, Copy::new);
        fullCopy2.setGn(6);
        fullCopy2.setUuid("full_02");
        Copy cumulativeCopy1 = new Copy();;
        cumulativeCopy1.setGn(7);
        cumulativeCopy1.setUuid("cumulative_01");
        cumulativeCopy1.setBackupType(BackupTypeConstants.CUMULATIVE_INCREMENT.getAbBackupType());
        cumulativeCopy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        Copy cumulativeCopy2 = BeanTools.copy(cumulativeCopy1, Copy::new);
        cumulativeCopy2.setGn(8);
        cumulativeCopy2.setUuid("cumulative_02");
        Copy logCopy3 = BeanTools.copy(logCopy1, Copy::new);
        logCopy3.setGn(9);
        logCopy3.setUuid("log_03");
        Copy fullCopy3 = BeanTools.copy(fullCopy1, Copy::new);
        fullCopy3.setGn(10);
        fullCopy3.setUuid("full_03");
        return Arrays.asList(fullCopy1, fullCopy2, fullCopy3, differenceCopy1, differenceCopy2, logCopy1,
            cumulativeCopy1, cumulativeCopy2, logCopy2, logCopy3);
    }

    /**
     * 用例场景：获取副本format
     * 前置条件：无
     * 检查点：从副本properties属性中获取format，若无则返回empty
     */
    @Test
    public void get_format_success_from_copy() {
        Copy copy1 = new Copy();
        OptionalInt format1 = CopyUtil.getFormat(copy1);
        Assert.assertFalse(format1.isPresent());

        Copy copy2 = new Copy();
        String properties2 = "{\"aa\":1}";
        copy2.setProperties(properties2);
        OptionalInt format2 = CopyUtil.getFormat(copy2);
        Assert.assertFalse(format2.isPresent());

        Copy copy3 = new Copy();
        String properties3 = "{\"format\":1}";
        copy3.setProperties(properties3);
        OptionalInt format3 = CopyUtil.getFormat(copy3);
        Assert.assertEquals(format3.getAsInt(), 1);
    }
}
