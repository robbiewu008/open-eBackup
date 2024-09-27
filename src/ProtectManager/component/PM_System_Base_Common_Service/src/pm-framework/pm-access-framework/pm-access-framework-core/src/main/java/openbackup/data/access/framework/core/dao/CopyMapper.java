/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 */

package openbackup.data.access.framework.core.dao;

import openbackup.data.access.framework.core.entity.CopiesEntity;
import openbackup.data.access.framework.core.model.CopySummaryCount;
import openbackup.data.access.framework.core.model.CopySummaryResource;
import openbackup.data.access.framework.core.model.CopySummaryResourceQuery;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;
import org.springframework.stereotype.Component;

import java.util.List;

/**
 * 副本mapper
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-03-01
 */
@Mapper
@Component
public interface CopyMapper extends BaseMapper<CopiesEntity> {
    /**
     * 分页查询副本资源列表
     *
     * @param query 查询条件
     * @return 副本资源列表
     */
    List<CopySummaryResource> selectCopySummaryResourceList(@Param("query") CopySummaryResourceQuery query);

    /**
     * 根据条件查询副本资源的总数
     *
     * @param query 查询条件
     * @return 总数
     */
    int selectCopySummaryResourceCount(@Param("query") CopySummaryResourceQuery query);

    /**
     * 根据resourceId查询当前资源下副本所有的deviceEsn
     *
     * @param resourceId resourceId 资源ID
     * @return List<String>  deviceEsnList</>
     */
    List<String> selectCopyDeviceEsnByResourceId(@Param("resource_id") String resourceId);

    /**
     * 统计副本
     *
     * @param domainId 用户所在域id，为空则统计所有
     * @return 副本统计计数
     */
    List<CopySummaryCount> queryCopyCount(@Param("domain_id") String domainId);

    /**
     * 更新副本状态
     *
     * @param copyIdList 副本ID列表
     * @param status 副本状态
     */
    void updateCopyStatus(@Param("copy_id_list") List<String> copyIdList, @Param("status") String status);
}
