package openbackup.data.access.framework.core.common.util;

import com.google.common.collect.Lists;

import openbackup.data.access.framework.core.common.util.CopyInfoBuilder;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.enums.CopyFeatureEnum;
import openbackup.data.protection.access.provider.sdk.resource.Resource;

import openbackup.system.base.common.enums.RetentionTypeEnum;
import openbackup.system.base.common.enums.TimeUnitEnum;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.protection.model.RetentionBo;
import openbackup.system.base.sdk.protection.model.SlaBo;

import org.junit.Assert;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;

/**
 *
 */
public class CopyInfoBuilderTest {

    /**
     * 用例名称：验证计算副本过期时间的正确性。<br/>
     * 前置条件：无。<br/>
     * check点：<br/>
     * 1、在不同的时间单位下，计算过期时间的正确性<br/>
     */
    @Test
    public void test_computeExpirationTime() {
        Date date = new Date(0L);
        Date minute = CopyInfoBuilder.computeExpirationTime(date.getTime(), TimeUnitEnum.MINUTES, 1);
        Assert.assertEquals(new Date(60L * 1000), minute);
        Date hour = CopyInfoBuilder.computeExpirationTime(date.getTime(), TimeUnitEnum.HOURS, 1);
        Assert.assertEquals(new Date(60 * 60 * 1000L), hour);
        Date day = CopyInfoBuilder.computeExpirationTime(date.getTime(), TimeUnitEnum.DAYS, 1);
        Assert.assertEquals(new Date(24 * 60 * 60 * 1000L), day);
        Date week = CopyInfoBuilder.computeExpirationTime(date.getTime(), TimeUnitEnum.WEEKS, 1);
        Assert.assertEquals(new Date(7 * 24 * 60 * 60 * 1000L), week);
        Date month = CopyInfoBuilder.computeExpirationTime(date.getTime(), TimeUnitEnum.MONTHS, 1);
        Calendar nextMonth = Calendar.getInstance();
        nextMonth.setTime(date);
        nextMonth.add(Calendar.MONTH, 1);
        Assert.assertEquals(nextMonth.getTime(), month);
        Date year = CopyInfoBuilder.computeExpirationTime(date.getTime(), TimeUnitEnum.YEARS, 1);
        Calendar nextYear = Calendar.getInstance();
        nextYear.setTime(date);
        nextYear.add(Calendar.YEAR, 1);
        Assert.assertEquals(nextYear.getTime(), year);
    }

    /**
     * 用例名称：验证生产副本信息对象。<br/>
     * 前置条件：无。<br/>
     * check点：<br/>
     * 1、无异常抛出<br/>
     */
    @Test
    public void test_build_without_exception() {
        ArrayList<CopyFeatureEnum> copyFeatureEnums = Lists.newArrayList(
                CopyFeatureEnum.RESTORE, CopyFeatureEnum.INSTANT_RESTORE, CopyFeatureEnum.MOUNT);
        HashMap<String, Object> properties = new HashMap<>();
        Resource resource = new Resource();
        SlaBo slaBo = new SlaBo();
        PolicyBo policyBo = new PolicyBo();
        RetentionBo retentionBo = new RetentionBo();
        retentionBo.setRetentionDuration(1);
        retentionBo.setDurationUnit(TimeUnitEnum.MINUTES.getUnit());
        retentionBo.setRetentionType(RetentionTypeEnum.TEMPORARY.getType());
        policyBo.setRetention(retentionBo);
        CopyInfoBuilder builder = CopyInfoBuilder.builder()
                .setCopyId("backupId")
                .setBaseCopyInfo()
                .setEnvironmentInfo("envName", "envEndPoint")
                .setEBackupTimestamp(0L)
                .setCopyFeature(copyFeatureEnums)
                .setResourceInfo(resource)
                .setRetentionInfo(slaBo, null, 0L)
                .setRetentionInfo(slaBo, policyBo, 0L)
                .setProperties(properties)
                .setOtherInfo("chainId", "resourceStr", "slaString")
                .setGeneratedBy("Backup")
                .setDeletable(Boolean.TRUE) // 设置副本是否可删除的类型 高级备份默认可以删除
                .setBackupType(1)
                .setSourceCopyType(1)
                .setUserId("id")
                .setName("name")
                .setLocation("Local")
                .setStorageId("StorageId");
        retentionBo.setRetentionType(RetentionTypeEnum.QUANTITY.getType());
        builder.setRetentionInfo(slaBo, policyBo, 0L);
        CopyInfoBo copyInfoBo = builder.build();
        Assert.assertNotNull(copyInfoBo);
    }
}
