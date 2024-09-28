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
package openbackup.system.base.pack.lock.mapper;

import openbackup.system.base.pack.lock.entity.LockEntity;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import org.apache.ibatis.annotations.Param;
import org.apache.ibatis.annotations.Update;

import java.util.Date;

/**
 * T_DISTRIBUTED_LOCK DAO
 *
 */
public interface LockMapper extends BaseMapper<LockEntity> {
    /**
     * 释放占有的锁
     *
     * @param unLockTime 释放时间
     * @param owner owner
     */
    @Update("update T_DISTRIBUTED_LOCK set UNLOCK_TIME=#{unlockTime} where OWNER=#{owner}")
    void unlockAll(@Param("unlockTime") Date unLockTime, @Param("owner") String owner);
}
