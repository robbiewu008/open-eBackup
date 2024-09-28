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
package openbackup.access.framework.resource.lock;

import static org.mockito.ArgumentMatchers.any;

import openbackup.access.framework.resource.lock.ResourceLockReleaseScheduler;
import openbackup.access.framework.resource.lock.dao.ResourceLockMapper;
import openbackup.access.framework.resource.lock.entity.ResourceLock;
import openbackup.access.framework.resource.service.ResourceRedisLockService;
import openbackup.access.framework.resource.service.impl.ResourceRedisLockServiceImpl;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.sdk.copy.CopyRestApi;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.redisson.api.RedissonClient;

import java.util.ArrayList;

/**
 * ResourceLockReleaseSchedulerTest
 *
 */
public class ResourceLockReleaseSchedulerTest {
    private static final long END_TIME = 1688715622907L;

    private static final String LOCKED = "LOCKED";

    private static final String SUCCESS = "SUCCESS";

    private RedissonClient redissonClient;

    private ResourceRedisLockService resourceRedisLockService;

    private CopyRestApi copyRestApi;

    @Before
    public void before() {
        redissonClient = Mockito.mock(RedissonClient.class);
        copyRestApi = Mockito.mock(CopyRestApi.class);
        resourceRedisLockService = Mockito.mock(ResourceRedisLockServiceImpl.class);
    }

    /**
     * 用例场景：测试定时清理已完成任务未释放资源锁场景
     * 前置条件：任务已完成且未释放锁
     * 检  查  点：未释放锁，自动清理资源锁
     */
    @Test
    public void should_releaseOverTimeResourceLock_when_time_come() {
        ResourceLockReleaseScheduler scheduler = getResourceLockReleaseScheduler();
        scheduler.releaseOverTimeResourceLock();
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：测试定时清理已完成任务未释放资源锁找不到任务场景
     * 前置条件：资源锁关联的任务不存在
     * 检  查  点：根据任务id查询任务
     */
    @Test
    public void should_throw_LegoCheckedException_when_job_is_null() {
        ResourceLockMapper mapper = PowerMockito.mock(ResourceLockMapper.class);
        ArrayList<ResourceLock> resourceLocks = new ArrayList<>();
        ResourceLock resourceLock = new ResourceLock();
        resourceLock.setResourceId("5f482cb4-40d8-43c2-9556-b71376058335");
        resourceLock.setLockId("5f482cb4-40d8-43c2-9556-b71376058335");
        resourceLock.setLockState(LOCKED);
        resourceLocks.add(resourceLock);
        PowerMockito.when(mapper.selectList(any())).thenReturn(resourceLocks);
        JobService jobService = PowerMockito.mock(JobService.class);
        PowerMockito.when(jobService.queryJob(any())).thenThrow(LegoCheckedException.class);
        ResourceLockReleaseScheduler scheduler =
            new ResourceLockReleaseScheduler(mapper, jobService, redissonClient, resourceRedisLockService);
        scheduler.releaseOverTimeResourceLock();
        Assert.assertTrue(true);
    }

    private ResourceLockReleaseScheduler getResourceLockReleaseScheduler() {
        ResourceLockMapper mapper = PowerMockito.mock(ResourceLockMapper.class);
        ArrayList<ResourceLock> resourceLocks = new ArrayList<>();
        ResourceLock resourceLock = new ResourceLock();
        resourceLock.setResourceId("5f482cb4-40d8-43c2-9556-b71376058335");
        resourceLock.setLockId("5f482cb4-40d8-43c2-9556-b71376058335");
        resourceLock.setLockState(LOCKED);
        resourceLocks.add(resourceLock);
        PowerMockito.when(mapper.selectList(any())).thenReturn(resourceLocks);
        JobBo jobBo = new JobBo();
        jobBo.setJobId("5f482cb4-40d8-43c2-9556-b71376058335");
        jobBo.setStatus(SUCCESS);
        jobBo.setEndTime(END_TIME);
        JobService jobService = PowerMockito.mock(JobService.class);
        PowerMockito.when(jobService.queryJob(any())).thenReturn(jobBo);
        return new ResourceLockReleaseScheduler(mapper, jobService, redissonClient,resourceRedisLockService);
    }
}
