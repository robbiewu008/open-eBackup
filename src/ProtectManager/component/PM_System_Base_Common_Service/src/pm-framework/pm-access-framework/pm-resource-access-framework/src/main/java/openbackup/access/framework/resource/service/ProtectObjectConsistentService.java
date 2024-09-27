package openbackup.access.framework.resource.service;

/**
 * 功能描述: ProtectObjectService
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-01
 */
public interface ProtectObjectConsistentService {
    /**
     * 刷新所有保护对象数据一致性状态
     */
    void refreshProtectObjectConsistentStatus();

    /**
     * 检测数据保护对象一致性状态
     *
     * @param isInit 是否进程重启后触发的
     */
    void checkProtectObjectConsistentStatus(boolean isInit);
}