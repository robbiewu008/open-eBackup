package openbackup.oracle.service;

import openbackup.oracle.constants.ScnCopy;

import java.util.List;

/**
 * Oracle副本服务
 *
 * @author c30038333
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-04-04
 */
public interface OracleCopyService {
    /**
     * 根据scn查询副本
     *
     * @param resourceId 资源id
     * @param filterValue 筛选值
     * @return 副本信息
     */
    List<ScnCopy> listCopiesInfo(String resourceId, String filterValue);
}
