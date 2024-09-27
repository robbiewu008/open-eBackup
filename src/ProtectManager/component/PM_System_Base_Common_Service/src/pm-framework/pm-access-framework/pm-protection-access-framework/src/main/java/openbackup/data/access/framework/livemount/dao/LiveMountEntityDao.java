package openbackup.data.access.framework.livemount.dao;

import openbackup.system.base.common.annotation.DbMangerMapper;
import openbackup.system.base.common.model.livemount.LiveMountEntity;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import org.apache.ibatis.annotations.Param;
import org.apache.ibatis.annotations.Select;

import java.util.List;

/**
 * Live Mount Entity Dao
 *
 * @author l00272247
 * @since 2020-09-18
 */
@DbMangerMapper
public interface LiveMountEntityDao extends BaseMapper<LiveMountEntity> {
    /**
     * JOIN_SQL
     */
    String JOIN_SQL = "select a.* from live_mount a left join live_mount_policy b on a.policy_id = b.policy_id "
        + "where resource_id=#{id} and schedule_policy='after_backup_done'";

    /**
     * page query
     *
     * @param resourceId resource id
     * @return page
     */
    @Select(JOIN_SQL)
    List<LiveMountEntity> queryAutoUpdateWhenBackupDoneLiveMounts(@Param("id") String resourceId);
}
