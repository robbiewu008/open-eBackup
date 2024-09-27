/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.core.common.util;

import openbackup.data.access.framework.core.common.util.EngineUtil;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * Engine Util Test
 *
 * @author g00500588
 * @since 2021/12/15
 */
public class EngineUtilTest {
    /**
     * 测试场景：cloud backup 取消备份任务
     * 前置条件：PM功能正常
     * 检查点：cloud backup EngineTaskTypeKey正确
     */
    @Test
    public void test_get_cloud_backup_engine_task_type_key_success() {
        String taskTypeKey = EngineUtil.getEngineTaskTypeKey(ResourceSubTypeEnum.CLOUD_BACKUP_FILE_SYSTEM.getType(),
            JobTypeEnum.BACKUP);
        Assert.assertEquals("Engine_DataMover_CloudBackupFileSystem_BACKUP", taskTypeKey);
    }

    /**
     * 测试场景：验证nas归档获取备份引擎key直接返回任务类型
     * 前置条件：无
     * 检查点：1、nas文件系统返回归档任务类型 2、nas share返回归档任务类型
     */
    @Test
    public void should_return_archive_when_getEngineTaskTypeKey_given_resource_type_is_nas() {
        // Given and When
        String nasFileSystemKey = EngineUtil.getEngineTaskTypeKey(ResourceSubTypeEnum.NAS_FILESYSTEM.getType(),
            JobTypeEnum.ARCHIVE);
        String nasShareKey = EngineUtil.getEngineTaskTypeKey(ResourceSubTypeEnum.NAS_SHARE.getType(),
            JobTypeEnum.ARCHIVE);
        // Then
        Assert.assertEquals(JobTypeEnum.ARCHIVE.getValue(), nasFileSystemKey);
        Assert.assertEquals(JobTypeEnum.ARCHIVE.getValue(), nasShareKey);
    }
}
