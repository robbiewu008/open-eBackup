package openbackup.data.access.framework.copy.controller;

import openbackup.system.base.sdk.copy.model.StorageInfo;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.controller.req.CopyVerifyRequest;
import openbackup.data.access.framework.copy.index.service.impl.UnifiedCopyIndexService;
import openbackup.data.access.framework.copy.mng.service.CopyService;
import openbackup.data.access.framework.copy.verify.service.CopyVerifyTaskManager;
import openbackup.data.access.framework.core.model.CopySummaryResource;
import openbackup.data.access.framework.protection.controller.v2.req.DownloadFilesReq;
import openbackup.data.access.framework.protection.controller.v2.resp.DownLoadResp;
import openbackup.system.base.common.aspect.OperationLogAspect;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.hibernate.validator.constraints.Length;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import javax.servlet.http.HttpServletRequest;

/**
 * 内部副本相关的接口
 *
 * @author z30027603
 * @version [OceanProtect X800 1.3.0]
 * @since 2023-2-23
 */
@Slf4j
@RestController
@Validated
public class CopyInternalController {
    // 指定时间可用副本的分页最大值
    private static final int MAX_PAGE_SIZE = 100;

    // 指定副本资源查询的分页最大值
    private static final int MAX_PAGE_COPY_SUMMARY_RESOURCE = 200;

    private final CopyService copyService;

    private final UnifiedCopyIndexService unifiedCopyIndexService;

    private final CopyVerifyTaskManager copyVerifyTaskManager;

    /**
     * 副本控制器构造函数
     *
     * @param copyService  副本服务
     * @param unifiedCopyIndexService 统一副本所以服务
     * @param copyVerifyTaskManager  副本校验管理器
     */
    public CopyInternalController(CopyService copyService, UnifiedCopyIndexService unifiedCopyIndexService,
        CopyVerifyTaskManager copyVerifyTaskManager) {
        this.copyService = copyService;
        this.unifiedCopyIndexService = unifiedCopyIndexService;
        this.copyVerifyTaskManager = copyVerifyTaskManager;
    }

    /**
     * 下载副本中的文件
     *
     * @param copyId 副本id
     * @param downloadFilesReq 下载文件请求参数
     * @return request id
     */
    @ExterAttack
    @PostMapping("/v2/internal/copies/{copyId}/action/download")
    public DownLoadResp downloadFiles(@PathVariable("copyId") String copyId,
                                        @RequestBody DownloadFilesReq downloadFilesReq) {
        return new DownLoadResp(copyService.downloadFiles(copyId, downloadFilesReq.getPaths(),
                downloadFilesReq.getRecordId()));
    }

    /**
     * 删除资源对应副本的索引
     *
     * @param resourceId 资源Id
     * @param userId 用户Id
     */
    @ExterAttack
    @DeleteMapping("/v1/internal/copies/index")
    public void deleteResourceIndex(@RequestParam("resource_id") String resourceId,
        @RequestParam("user_id") String userId) {
        unifiedCopyIndexService.deleteResourceIndexTask(resourceId, userId);
    }

    /**
     * 内部校验副本校验
     *
     * @param copyId 副本id
     * @param copyVerifyRequest 副本校验参数
     * @param request request请求
     * @return 任务响应对象
     */
    @ExterAttack
    @PostMapping("/v2/internal/copies/{copyId}/action/verify")
    public UuidObject internalVerifyCopy(@PathVariable("copyId") String copyId,
        @RequestBody @Validated CopyVerifyRequest copyVerifyRequest,
        HttpServletRequest request) {
        buildTokenBo(copyVerifyRequest.getUserId(), request);
        return new UuidObject(copyVerifyTaskManager.init(copyId, copyVerifyRequest.getAgents()));
    }

    private void buildTokenBo(String userId, HttpServletRequest request) {
        if (VerifyUtil.isEmpty(userId)) {
            return;
        }
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setId(userId);
        TokenBo tokenBo = TokenBo.builder().user(userBo).build();
        request.setAttribute(OperationLogAspect.TOKEN_BO, tokenBo);
    }

    /**
     * 转发
     *
     * @param copyId copyId
     * @param esn esn
     */
    @ExterAttack
    @PostMapping("/v1/internal/copies/forward/{copy-id}/action/create-index/{esn}")
    public void createIndex(@PathVariable("copy-id") @Length(min = 1, max = 256) String copyId,
            @PathVariable("esn") @Length(min = 1, max = 256) String esn) {
        unifiedCopyIndexService.forwardCreateIndex(esn, copyId);
    }

    /**
     * 副本资源列表查询v2
     *
     * @param pageSize 分页数据条数
     * @param pageNo 分页页面编码
     * @param orders 排序字段
     * @param conditions 条件参数
     * @return 副本资源列表
     */
    @ExterAttack
    @GetMapping("/v2/internal/copies/summary/resources")
    public PageListResponse<CopySummaryResource> queryCopySummaryResource(
        @RequestParam(name = "pageSize", defaultValue = "10") int pageSize,
        @RequestParam(name = "pageNo", defaultValue = "0") int pageNo,
        @RequestParam(name = "orders", required = false) String[] orders,
        @RequestParam(name = "conditions", required = false) String conditions) {
        if (pageSize < 0 || pageSize > MAX_PAGE_COPY_SUMMARY_RESOURCE) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "page size is invalid.");
        }
        return copyService.listCopyResourceSummary(pageNo, pageSize, conditions, orders);
    }

    /**
     * 查询副本存储设备信息
     *
     * @param copyId 副本Id
     * @return 副本存储设备信息
     */
    @ExterAttack
    @GetMapping("/v1/internal/copies/storage-info/query")
    StorageInfo getStorageInfo(@RequestParam("copyId") String copyId) {
        return copyService.getStorageInfo(copyId);
    }
}