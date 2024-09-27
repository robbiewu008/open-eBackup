package openbackup.data.protection.access.provider.sdk.base.v2;

import com.huawei.emeistor.kms.kmc.util.KmcHelper;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.system.base.common.utils.StringUtil;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * 存储仓库信息，用于跟DME相关接口的信息传递 </br>
 * <p>
 * path字段 ：如果是nas，就是文件系统的共享名称加上路径名；如果是S3，就是桶名加上路径名。
 * 多文件系统：
 * 1. 构建两个结构体，抽取出remotePath；
 * 2. 删除副本的时候，只需要auth和endpoint信息；
 * </p>
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/11/30
 **/
@Data
public class StorageRepository extends BaseStorageRepository {
    /**
     * ens key 名称
     */
    public static final String REPOSITORIES_KEY_ENS = "esn";

    /**
     * 容量阈值 key 名称
     */
    public static final String CAPACITY_AVAILABLE = "capacityAvailable";

    /**
     * 是否为A8000本地存储
     */
    @JsonProperty("isLocal")
    private Boolean isLocal;

    /**
     * 多集群场景，是否为当前任务执行节点的存储
     */
    @JsonProperty("isLocalCluster")
    private Boolean isLocalCluster;

    /**
     * 当isLocal为True的时候，此字段为空；
     * 当isLocal为False的时候，此字段传远端存储的路径
     */
    private String path;

    /**
     * 存储仓库的认证信息
     */
    private Authentication auth;

    /**
     * 存储仓库的认证信息
     */
    private Authentication extendAuth;

    /**
     * 连接模式
     */
    private Integer connectType;

    /**
     * 云存储类型
     */
    private Integer cloudType;

    /**
     * 主机连接信息
     */
    private Endpoint endpoint;

    /**
     * 存储仓路径
     */
    private List<RemotePath> remotePath;

    /**
     * 存储库代理信息
     */
    private Proxy proxy;

    /**
     * 传输协议：http/https
     */
    private String transProtocol;

    public Boolean getLocal() {
        return isLocal;
    }

    public void setLocal(Boolean isLocal) {
        this.isLocal = isLocal;
    }

    public void setLocalCluster(Boolean isLocalCluster) {
        this.isLocalCluster = isLocalCluster;
    }

    public Boolean getLocalCluster() {
        return isLocalCluster;
    }

    @Override
    public List<RemotePath> getRemotePath() {
        return remotePath;
    }

    @Override
    public void setRemotePath(List<RemotePath> remotePath) {
        this.remotePath = remotePath;
    }

    /**
     * 加密auth里面authPwd字段
     */
    public void encryptPassword() {
        if (auth != null) {
            String tempPassword = auth.getAuthPwd();
            try {
                auth.setAuthPwd(KmcHelper.getInstance().encrypt(auth.getAuthPwd()));
            } finally {
                StringUtil.clean(tempPassword);
            }
        }
        if (extendAuth != null) {
            String tempPassword = extendAuth.getAuthPwd();
            try {
                extendAuth.setAuthPwd(KmcHelper.getInstance().encrypt(extendAuth.getAuthPwd()));
            } finally {
                StringUtil.clean(tempPassword);
            }
        }
    }

    /**
     * 解密auth里面authPwd字段
     */
    public void decryptPassword() {
        if (auth != null) {
            auth.setAuthPwd(KmcHelper.getInstance().decrypt(auth.getAuthPwd()));
        }
        if (extendAuth != null) {
            extendAuth.setAuthPwd(KmcHelper.getInstance().decrypt(extendAuth.getAuthPwd()));
        }
    }

    /**
     * 清理pwd
     */
    public void cleanAuth() {
        if (extendAuth != null) {
            StringUtil.clean(extendAuth.getAuthPwd());
        }
        if (auth != null) {
            StringUtil.clean(auth.getAuthPwd());
        }
    }
}
