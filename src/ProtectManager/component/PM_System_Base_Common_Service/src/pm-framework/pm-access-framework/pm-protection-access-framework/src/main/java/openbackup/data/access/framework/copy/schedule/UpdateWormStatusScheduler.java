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
package openbackup.data.access.framework.copy.schedule;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.dao.CopyMapper;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.util.SQLDistributeLock;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.core.annotation.Order;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

import java.text.SimpleDateFormat;

/**
 * 功能描述 定时更新副本表中worm状态
 *
 */
@Slf4j
@Component
@Order(0)
public class UpdateWormStatusScheduler {
    // 每30分钟开始更新副本表中的worm_status已过期的状态
    private static final long SCHEDULED_DELAY = 1000 * 60 * 30;

    @Autowired
    private CopyMapper copyMapper;

    /**
     * 定时更新副本表中worm状态：将已worm且过期时间大于当前时间的副本worm状态更新为已过期
     */
    @Scheduled(fixedRate = SCHEDULED_DELAY)
    @SQLDistributeLock(lockName = "WORM_STATUS_UPDATE_MONITOR")
    public void monitor() {
        try {
            log.debug("Start to schedule worm expired status.");
            SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
            String currentTime = dateFormat.format(System.currentTimeMillis());
            copyMapper.updateWormCopyExpiredStatus(currentTime);
        } catch (LegoCheckedException e) {
            log.error("Fail to schedule worm expired status.", ExceptionUtil.getErrorMessage(e));
        }
    }
}
