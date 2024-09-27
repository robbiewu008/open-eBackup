package openbackup.data.protection.access.provider.sdk.archive.v2;

import openbackup.data.protection.access.provider.sdk.backup.v2.DataLayout;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Qos;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 归档任务对象，归档时所需要的参数由该对象承载
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/1/12
 **/
public class ArchiveTask {
    /**
     * 是否开启自动索引
     */
    public static final String ENABLE_AUTO_INDEX = "autoCreateIndex";

    /**
     * 磁带归档支持的驱动数量
     */
    public static final String DRIVER_COUNT = "driverCount";

    /**
     * 是否开启网络加速
     */
    public static final String ENABLE_SPEED_UP = "speedUpEnabled";

    /**
     * 是否开启小文件聚合
     */
    public static final String ENABLE_SMALL_FILE_AGGREGATION = "smallFileAggregation";

    /**
     * 聚合文件大小
     */
    public static final String AGGREGATION_FILE_SIZE = "aggregationFileSize";

    /**
     * 待聚合文件大小最大值
     */
    public static final String AGGREGATION_FILE_MAX_SIZE = "aggregationFileMaxSize";

    /**
     * 归档任务请求id
     */
    private String requestId;

    /**
     * 归档任务id
     */
    private String taskId;

    /**
     * 源副本id
     */
    private String originCopyId;

    /**
     * 归档副本id
     */
    private String archiveCopyId;

    /**
     * 副本链的id
     */
    private String chainId;

    /**
     * 备份存储认证信息
     */
    private List<Authentication> backupStorages;

    /**
     * 存储库信息
     */
    private List<StorageRepository> repositories;

    /**
     * 限速配置
     */
    private Qos qos;

    /**
     * 数据布局：重删，压缩，加密
     */
    private DataLayout dataLayout;

    /**
     * 高级备份参数，key/value键值对存放
     */
    private Map<String, Object> advanceParams = new HashMap<>();

    /**
     * 归档介质所在的集群的esn
     */
    private String localEsn;

    public String getLocalEsn() {
        return localEsn;
    }

    public void setLocalEsn(String localEsn) {
        this.localEsn = localEsn;
    }

    public String getRequestId() {
        return requestId;
    }

    public void setRequestId(String requestId) {
        this.requestId = requestId;
    }

    public String getTaskId() {
        return taskId;
    }

    public void setTaskId(String taskId) {
        this.taskId = taskId;
    }

    public String getOriginCopyId() {
        return originCopyId;
    }

    public void setOriginCopyId(String originCopyId) {
        this.originCopyId = originCopyId;
    }

    public List<StorageRepository> getRepositories() {
        return repositories;
    }

    public void setRepositories(List<StorageRepository> repositories) {
        this.repositories = repositories;
    }

    public Qos getQos() {
        return qos;
    }

    public void setQos(Qos qos) {
        this.qos = qos;
    }

    public DataLayout getDataLayout() {
        return dataLayout;
    }

    public void setDataLayout(DataLayout dataLayout) {
        this.dataLayout = dataLayout;
    }

    public Map<String, Object> getAdvanceParams() {
        return advanceParams;
    }

    public void setAdvanceParams(Map<String, Object> advanceParams) {
        this.advanceParams = advanceParams;
    }

    public String getArchiveCopyId() {
        return archiveCopyId;
    }

    public void setArchiveCopyId(String archiveCopyId) {
        this.archiveCopyId = archiveCopyId;
    }

    public List<Authentication> getBackupStorages() {
        return backupStorages;
    }

    public void setBackupStorages(List<Authentication> backupStorages) {
        this.backupStorages = backupStorages;
    }

    /**
     * 设置归档任务扩展参数
     *
     * @param key 扩展参数map中的key
     * @param value 扩展参数map中的value
     */
    public void addAdvanceParam(String key, Object value) {
        this.advanceParams.put(key, value);
    }

    public String getChainId() {
        return chainId;
    }

    public void setChainId(String chainId) {
        this.chainId = chainId;
    }
}
