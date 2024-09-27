package openbackup.system.base.pack.lock.mapper;

import openbackup.system.base.pack.lock.entity.LockEntity;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import org.apache.ibatis.annotations.Param;
import org.apache.ibatis.annotations.Update;

import java.util.Date;

/**
 * T_DISTRIBUTED_LOCK DAO
 *
 * @author w30042425
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-06-10
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