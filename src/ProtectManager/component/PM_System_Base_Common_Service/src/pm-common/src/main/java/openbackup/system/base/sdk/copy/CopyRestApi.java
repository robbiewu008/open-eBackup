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
package openbackup.system.base.sdk.copy;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.CopyColumnConstant;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.rest.EmeistorFeignConfiguration;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.PageQueryRestApi;
import openbackup.system.base.sdk.archive.model.ArchiveMsg;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.sdk.copy.model.AddCopyArchiveMapRequest;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyAntiRansomwareReq;
import openbackup.system.base.sdk.copy.model.CopyAntiRansomwareRes;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.copy.model.CopyInfo;
import openbackup.system.base.sdk.copy.model.CopyResourceSummary;
import openbackup.system.base.sdk.copy.model.CopyRetentionPolicy;
import openbackup.system.base.sdk.copy.model.CopyStatusUpdateByEsnParam;
import openbackup.system.base.sdk.copy.model.CopyStatusUpdateParam;
import openbackup.system.base.sdk.copy.model.CopyWormStatusUpdateParam;
import openbackup.system.base.sdk.copy.model.DeleteExcessCopiesRequest;
import openbackup.system.base.sdk.copy.model.ReplicatedCopies;
import openbackup.system.base.sdk.copy.model.SaveImportCopyParam;
import openbackup.system.base.sdk.copy.model.UpdateCopyIndexStatusRequest;
import openbackup.system.base.sdk.copy.model.UpdateCopyPropertiesRequest;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

import java.text.SimpleDateFormat;
import java.time.Instant;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * JobCenter Client Service
 *
 */
@FeignClient(
        contextId = "copyClient",
        name = "copy-rest-api",
        url = "${service.url.pm-copies-catalog}/v1",
        configuration = EmeistorFeignConfiguration.class)
public interface CopyRestApi {
    /**
     * 日志备份
     */
    Integer LOG_BACKUP = 4;

    /**
     * query copy by id
     *
     * @param copyId copy id
     * @return copy
     */
    default Copy queryCopyByID(String copyId) {
        return queryCopyByID(copyId, true);
    }

    /**
     * query copy by id
     *
     * @param copyId copy id
     * @param isStrict isStrict
     * @return copy
     */
    default Copy queryCopyByID(String copyId, boolean isStrict) {
        List<Copy> copies;
        if (!VerifyUtil.isEmpty(copyId)) {
            copies = queryCopies(0, IsmNumberConstant.TWO, Collections.singletonMap("uuid", copyId)).getItems();
        } else {
            copies = Collections.emptyList();
        }
        int size = copies.size();
        if (size > 1) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "found multi copies by id " + copyId);
        }
        if (size == 0) {
            if (isStrict) {
                throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "not found copy by id " + copyId);
            } else {
                return null;
            }
        }
        return copies.get(0);
    }

    /**
     * 通过资源ID和gn值查询该副本以后的所有{generatedBy}副本（不包含本副本）
     *
     * @param resourceId resourceId
     * @param gn gn值
     * @param generatedBy 生成方式
     * @return 副本集合
     */
    default List<Copy> queryLaterCopiesByGeneratedBy(String resourceId, int gn, String generatedBy) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("resource_id", resourceId);
        conditions.put("gn_range", Arrays.asList(gn, null));
        conditions.put("generated_by", generatedBy);
        List<Copy> copyList = new ArrayList<>();
        BasePage<Copy> basePage;
        int count = 0;
        do {
            basePage = queryCopies(count, 100, conditions, Collections.singletonList("-display_timestamp"));
            copyList.addAll(basePage.getItems());
            count++;
        } while (basePage.getItems().size() == 100);
        return copyList;
    }

    /**
     * 通过资源ID查询所有{generatedBy}副本
     *
     * @param resourceId resourceId
     * @param generatedBy 生成方式
     * @return 副本集合
     */
    default List<Copy> queryCopiesByResourceIdAndGeneratedBy(String resourceId, List<String> generatedBy) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(CopyColumnConstant.RESOURCE_ID, resourceId);
        conditions.put(CopyColumnConstant.GENERATED_BY, generatedBy);
        List<Copy> copyList = new ArrayList<>();
        BasePage<Copy> basePage;
        int count = 0;
        do {
            basePage = queryCopies(count, 100, conditions);
            copyList.addAll(basePage.getItems());
            count++;
        } while (basePage.getItems().size() == 100);
        return copyList;
    }

    /**
     * 通过资源ID和gn值查询该副本以后的所有备份副本（不包含本副本）
     *
     * @param resourceId resourceId
     * @param gn gn值
     * @return 副本集合
     */
    default List<Copy> queryLaterBackupCopies(String resourceId, int gn) {
        return queryLaterCopiesByGeneratedBy(resourceId, gn, CopyGeneratedByEnum.BY_BACKUP.value());
    }

    /**
     * 通过资源ID和gn值以及备份类型(比如全量)查询该副本之前的所有备份副本（不包含本副本）
     *
     * @param resourceId resourceId
     * @param gn gn值
     * @param backupType 副本类型
     * @return 副本集合
     */
    default Optional<Copy> queryLatestFullBackupCopies(String resourceId, int gn, int backupType) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("resource_id", resourceId);
        conditions.put("gn_range", Arrays.asList(null, gn));
        conditions.put("backup_type", backupType);
        conditions.put("generated_by", CopyGeneratedByEnum.BY_BACKUP.value());
        BasePage<Copy> page = queryCopies(0, 1, conditions, Collections.singletonList("-display_timestamp"));
        return page.getTotal() > 0 ? Optional.ofNullable(page.getItems().get(0)) : Optional.empty();
    }

    /**
     * 通过资源ID和备份生成方式类型(比如差异方式生成)，查询最近一次以该类型生成的备份副本
     *
     * @param resourceId resourceId
     * @param sourceBackupTypeList 备份生成方式类型列表
     * @param generatedByList 副本生成方式类型列表
     * @param copyStatus copyStatus
     * @return 副本集合
     */
    default Optional<Copy> queryLatestCopiesByResourceIdAndSourceCopyType(String resourceId,
        List<Integer> sourceBackupTypeList, List<String> generatedByList, List<String> copyStatus) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("resource_id", resourceId);
        if (!VerifyUtil.isEmpty(sourceBackupTypeList)) {
            conditions.put("source_copy_type", sourceBackupTypeList);
        }
        conditions.put("generated_by", generatedByList);
        conditions.put("status", copyStatus);
        BasePage<Copy> page = queryCopies(0, 1, conditions, Collections.singletonList("-display_timestamp"));
        return page.getTotal() > 0 ? Optional.ofNullable(page.getItems().get(0)) : Optional.empty();
    }

    /**
     * 通过资源ID和备份类型(比如全量)，查询最近一次该类型的备份副本
     *
     * @param resourceId resourceId
     * @param backupTypeList 备份类型列表
     * @return 副本集合
     */
    default Optional<Copy> queryLatestCopiesByResourceIdAndBackupType(String resourceId, List<Integer> backupTypeList) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("resource_id", resourceId);
        if (!VerifyUtil.isEmpty(backupTypeList)) {
            conditions.put("backup_type", backupTypeList);
        }
        conditions.put("generated_by", CopyGeneratedByEnum.BY_BACKUP.value());
        BasePage<Copy> page = queryCopies(0, 1, conditions, Collections.singletonList("-display_timestamp"));
        return page.getTotal() > 0 ? Optional.ofNullable(page.getItems().get(0)) : Optional.empty();
    }

    /**
     * 通过esn和poolId批量查询副本
     *
     * @param unitId unitId
     * @return 副本集合
     */
    default List<Copy> queryCopiesByStorageUnit(String unitId) {
        Map<String, Object> filterMap = new HashMap<>();
        filterMap.put("storage_unit_id", unitId);
        List<Copy> copyList = new ArrayList<>();
        BasePage<Copy> basePage;
        int count = 0;
        do {
            basePage = queryCopies(count, 100, filterMap);
            copyList.addAll(basePage.getItems());
            count++;
        } while (basePage.getItems().size() == 100);
        return copyList;
    }

    /**
     * 通过esn批量查询副本
     *
     * @param esn esn
     * @return 副本集合
     */
    default List<Copy> queryCopiesByEsn(String esn) {
        Map<String, Object> filterMap = new HashMap<>();
        filterMap.put("device_esn", esn);
        List<Copy> copyList = new ArrayList<>();
        BasePage<Copy> basePage;
        int count = 0;
        do {
            basePage = queryCopies(count, 100, filterMap);
            copyList.addAll(basePage.getItems());
            count++;
        } while (basePage.getItems().size() == 100);
        return copyList;
    }

    /**
     * 通过资源ID批量查询副本
     *
     * @param resourceId 资源ID
     * @param indexStatus 索引状态
     * @return 副本集合
     */
    default List<Copy> queryCopiesByResourceIdAndIndexStatus(String resourceId, String indexStatus) {
        Map<String, Object> filterMap = new HashMap<>();
        filterMap.put("resource_id", resourceId);
        filterMap.put("indexed", indexStatus);
        List<Copy> copyList = new ArrayList<>();
        BasePage<Copy> basePage;
        int count = 0;
        do {
            basePage = queryCopies(count, 100, filterMap);
            copyList.addAll(basePage.getItems());
            count++;
        } while (basePage.getItems().size() == 100);
        return copyList;
    }

    /**
     * query copies
     *
     * @param page page
     * @param size size
     * @param orders orders
     * @param condition condition
     * @return copies
     */
    @ExterAttack
    @GetMapping("/internal/copies")
    @ResponseBody
    BasePage<Copy> queryCopies(
            @RequestParam("page_no") int page,
            @RequestParam("page_size") int size,
            @RequestParam("conditions") String condition,
            @RequestParam("orders") List<String> orders);

    /**
     * query copies
     *
     * @param page page
     * @param size size
     * @param orders orders
     * @param condition condition
     * @return copies
     */
    default BasePage<Copy> queryCopies(int page, int size, Map<String, Object> condition, List<String> orders) {
        String json = JSONObject.fromObject(condition).toString();
        return queryCopies(page, size, json, orders);
    }

    /**
     * query copies
     *
     * @param page page
     * @param size size
     * @param condition condition
     * @return copies
     */
    default BasePage<Copy> queryCopies(int page, int size, Map<String, Object> condition) {
        return queryCopies(page, size, condition, Collections.emptyList());
    }

    /**
     * query latest copy
     *
     * @param resourceId resource id
     * @param range limit
     * @param filter filter
     * @return copy
     */
    default Copy queryLatestBackupCopy(String resourceId, String range, Map<String, Object> filter) {
        Map<String, Object> condition = getQueryParams(filter, resourceId, range);
        BasePage<Copy> page = queryCopies(0, 1, condition, Collections.singletonList("-display_timestamp"));
        return page.getTotal() > 0 ? page.getItems().get(0) : null;
    }

    /**
     * 生成参数
     *
     * @param filter filter
     * @param resourceId resourceId
     * @param range range
     * @return 条件
     */
    default Map<String, Object> getQueryParams(Map<String, Object> filter, String resourceId, String range) {
        Map<String, Object> condition = new HashMap<>(Optional.ofNullable(filter).orElse(Collections.emptyMap()));
        condition.put("resource_id", resourceId);
        condition.put("generated_by", CopyGeneratedByEnum.BY_BACKUP.value());
        if (range != null) {
            Calendar calendar = Calendar.getInstance();
            calendar.setTime(Date.from(Instant.now()));
            HashMap<String, Integer> calendarMap = new HashMap<String, Integer>() {
                {
                    put("hour", Calendar.HOUR);
                    put("date", Calendar.DATE);
                    put("week", Calendar.WEEK_OF_YEAR);
                    put("month", Calendar.MONTH);
                    put("year", Calendar.YEAR);
                }
            };
            Integer calendarEnum = calendarMap.get(range);
            if (calendarEnum == Calendar.WEEK_OF_YEAR) {
                calendar.add(calendarEnum, -2);
            } else {
                calendar.add(calendarEnum, -1);
            }
            HashMap<String, String> rangMap = new HashMap<String, String>() {
                {
                    put("hour", "yyyy-MM-dd HH");
                    put("date", "yyyy-MM-dd");
                    put("week", "yyyy-w");
                    put("month", "yyyy-MM");
                    put("year", "yyyy");
                }
            };
            String format = rangMap.get(range);
            condition.put(range, new SimpleDateFormat(format).format(calendar.getTime()));
        }
        return condition;
    }

    /**
     * save copy
     *
     * @param copyInfo copy info
     * @return uuid object
     */
    @ExterAttack
    @PostMapping("/internal/copies")
    UuidObject saveCopy(@RequestBody CopyInfo copyInfo);

    /**
     * save batch copy list
     *
     * @param copyInfos copyInfos
     */
    @PostMapping("/internal/batchCopies")
    void saveBatchCopies(@RequestBody List<Copy> copyInfos);

    /**
     * update copy retention
     *
     * @param copyId copy id
     * @param copyRetentionPolicy copy retention policy
     * @return result
     */
    @ExterAttack
    @PostMapping("/internal/copies/{copy_id}/action/update-retention")
    Object updateCopyRetention(
            @PathVariable("copy_id") String copyId, @RequestBody CopyRetentionPolicy copyRetentionPolicy);

    /**
     * delete copy
     *
     * @param copyId copy id
     * @param userId user id
     */
    default void deleteCopy(@PathVariable("copy_id") String copyId, @RequestParam("user_id") String userId) {
        deleteCopy(copyId, userId, false, true, null);
    }

    /**
     * delete copy
     *
     * @param copyId copyId
     * @param userId userId
     * @param isForced 是否强删
     * @param isAssociated 是否关联删除
     * @param jobType 任务类型
     */
    @DeleteMapping("/internal/copies/{copy_id}")
    void deleteCopy(@PathVariable("copy_id") String copyId, @RequestParam("user_id") String userId,
        @RequestParam("is_forced") Boolean isForced, @RequestParam("is_associated") Boolean isAssociated,
        @RequestParam("job_type") String jobType);

    /**
     * only delete copy in databse
     *
     * @param copyIds copy id list
     */
    @DeleteMapping("/internal/copies")
    void deleteCopiesForDatabase(@RequestParam("copy_id_list") List<String> copyIds);

    /**
     * delete batch copy in database
     *
     * @param copyIds copy id list
     */
    @DeleteMapping("/internal/batchCopies")
    void deleteBatchCopies(@RequestParam("copy_id_list") List<String> copyIds);

    /**
     * update copy status
     *
     * @param copyId copy id
     * @param param param
     */
    @PutMapping("/internal/copies/{copy_id}/status")
    void updateCopyStatus(@PathVariable("copy_id") String copyId, @RequestBody CopyStatusUpdateParam param);

    /**
     * update copy worm status
     *
     * @param copyId copy id
     * @param param param
     */
    @PutMapping("/internal/copies/{copy_id}/worm-status")
    void updateCopyWormStatus(@PathVariable("copy_id") String copyId, @RequestBody CopyWormStatusUpdateParam param);

    /**
     * update copy index status
     *
     * @param copyId copy id
     * @param indexStatus index status
     * @param errorCode error code
     */
    @PutMapping("/internal/copies/{copy_id}/index-status")
    void updateCopyIndexStatus(
            @PathVariable("copy_id") String copyId,
            @RequestParam("index_status") String indexStatus,
            @RequestParam(value = "error_code", required = false, defaultValue = "") String errorCode);

    /**
     * update copy index status
     *
     * @param copyId copy id
     * @param indexStatus index status
     */
    default void updateCopyIndexStatus(String copyId, String indexStatus) {
        updateCopyIndexStatus(copyId, indexStatus, "");
    }

    /**
     * query copy resource summary
     *
     * @param page page
     * @param size size
     * @param condition condition
     * @param orders orders
     * @return resource summary of copy
     */
    @ExterAttack
    @GetMapping("/internal/copies/summary/resources")
    @ResponseBody
    @Deprecated
    BasePage<CopyResourceSummary> queryCopyResourceSummary(
            @RequestParam("page_no") int page,
            @RequestParam("page_size") int size,
            @RequestParam("conditions") String condition,
            @RequestParam("orders") List<String> orders);

    /**
     * query copy resource summary
     *
     * @param resourceId resource id
     * @return resource summary of copy
     */
    default CopyResourceSummary queryCopyResourceSummary(String resourceId) {
        return queryCopyResourceSummary(resourceId, true);
    }

    /**
     * query copy resource summary
     *
     * @param resourceId resource id
     * @param isStrict strict mode
     * @return resource summary of copy
     */
    default CopyResourceSummary queryCopyResourceSummary(String resourceId, boolean isStrict) {
        return PageQueryRestApi.get(this::queryCopyResourceSummary)
                .queryOne(new JSONObject().set("resource_id", resourceId), isStrict);
    }

    /**
     * query copy resource properties as resource bean of the special type
     *
     * @param resourceId resource id
     * @param isStrict isStrict
     * @param clazz clazz
     * @param <T> template type
     * @return the resource bean
     */
    default <T> T queryCopyResourceSummary(String resourceId, boolean isStrict, Class<T> clazz) {
        CopyResourceSummary copyResourceSummary = queryCopyResourceSummary(resourceId, isStrict);
        return JSONObject.fromObject(copyResourceSummary.getResourceProperties()).toBean(clazz);
    }

    /**
     * query copy resource properties as resource bean of the special type
     *
     * @param resourceId resource id
     * @param clazz clazz
     * @param <T> template type
     * @return the resource bean
     */
    default <T> T queryCopyResourceSummary(String resourceId, Class<T> clazz) {
        return queryCopyResourceSummary(resourceId, true, clazz);
    }

    /**
     * verify copy ownership
     *
     * @param userId user id
     * @param copyUuidList policy uuid list
     */
    @GetMapping("/internal/copies/action/verify")
    @ResponseBody
    void verifyCopyOwnership(
            @RequestParam("user_id") String userId, @RequestParam("copy_uuid_list") List<String> copyUuidList);

    /**
     * query copies
     *
     * @param copyId 副本ID
     * @param isSortedKey 排序
     * @param condition 条件
     * @return copies
     */
    @ExterAttack
    @GetMapping("/internal/copies/{copy_id}/latest-related-copy")
    @ResponseBody
    Copy queryCopyByCondition(
            @PathVariable("copy_id") String copyId,
            @RequestParam("sorted_key") Boolean isSortedKey,
            @RequestParam("conditions") String condition);

    /**
     * 重置Copy user id
     *
     * @param userId user id
     */
    @PutMapping("/internal/copies/action/revoke/{user_id}")
    void revokeCopyUserId(@PathVariable("user_id") String userId);

    /**
     * query archive copies
     *
     * @param storageId STORAGE ID
     * @return count
     */
    @ExterAttack
    @GetMapping("/internal/copies/archive/{storage_id}")
    @ResponseBody
    int queryArchiveCopyCountByStorageId(@PathVariable("storage_id") String storageId);

    /**
     * query signed deleted copies
     *
     * @param chainId CAHIN ID
     * @return copies
     */
    @ExterAttack
    @GetMapping("/internal/copies/{chain_id}/signed-deleted-copies")
    @ResponseBody
    List<Copy> queryDeletedCopies(@PathVariable("chain_id") String chainId);

    /**
     * save import copy
     *
     * @param param save import copy param
     */
    @PostMapping("/internal/copies/action/import")
    void saveImportCopy(SaveImportCopyParam param);

    /**
     * 写入防勒索检测信息
     *
     * @param param 防勒索检测信息
     * @param copyId 副本id
     */
    @PostMapping("/internal/copies/{copy_id}/detection-reports")
    void createDetectionReports(@PathVariable("copy_id") String copyId, @RequestBody CopyAntiRansomwareReq param);

    /**
     * 获得未检测防勒索的副本列表
     *
     * @param resourceId 资源id
     * @param generatedByList 副本生成类型
     * @param copyStartTime 副本完成起始时间
     * @param page page
     * @param size size
     * @return 副本基表
     */
    @ExterAttack
    @GetMapping("/internal/copies/status/undetected")
    @ResponseBody
    BasePage<Copy> queryUndetectResources(
            @RequestParam("resource_id") String resourceId,
            @RequestParam(name = "generated_by_list", required = false) List<String> generatedByList,
            @RequestParam(name = "copy_start_time", required = false) String copyStartTime,
            @RequestParam(name = "page_no") Integer page,
            @RequestParam(name = "page_size") Integer size);

    /**
     * 增加copy archive map映射
     *
     * @param request 增加copy archive请求
     */
    @PostMapping("/internal/copies/action/archive")
    void addCopyArchiveMap(@RequestBody AddCopyArchiveMapRequest request);

    /**
     * 更新资源Id对应副本的索引状态
     *
     * @param resourceId 资源Id
     * @param indexStatus 索引状态
     * @param errorCode 错误码
     */
    @PutMapping("/internal/copies/index-status")
    void updateResourceCopyIndexStatus(
            @RequestParam("resource_id") String resourceId,
            @RequestParam("index_status") String indexStatus,
            @RequestParam(value = "error_code", required = false, defaultValue = "") String errorCode);

    /**
     * 查询防勒索检测信息
     *
     * @param copyId 副本id
     * @return 防勒索检测信息
     */
    @ExterAttack
    @GetMapping("/internal/copies/{copy_id}/detection-reports")
    @ResponseBody
    CopyAntiRansomwareRes queryAntiRansomwareDetail(@PathVariable("copy_id") String copyId);

    /**
     * 删除归档存储下所有副本
     *
     * @param storageId 归档存储UUID
     */
    @DeleteMapping("/internal/copies/action/delete-storage/{storage_id}")
    @ResponseBody
    void deleteArchiveCopyByStorageId(@PathVariable("storage_id") String storageId);

    /**
     * 根据副本id，统计该副本经过live mount产生的子副本数量
     *
     * @param copyId 副本id
     * @return 子副本数量
     */
    @ExterAttack
    @GetMapping("/internal/copies/{copy_id}/action/count")
    @ResponseBody
    int countCopyByParentId(@PathVariable("copy_id") String copyId);

    /**
     * 更新副本扩展参数
     *
     * @param copyId  副本id
     * @param request 更新请求
     */
    @ExterAttack
    @ResponseBody
    @PutMapping("/internal/copies/{copyId}/properties")
    void updateProperties(@PathVariable("copyId") String copyId, @RequestBody UpdateCopyPropertiesRequest request);

    /**
     * 通过资源ID所有备份副本
     *
     * @param resourceId 资源ID
     * @return 副本集合
     */
    default List<Copy> queryCopiesByResourceId(String resourceId) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("resource_id", resourceId);
        List<Copy> copyList = new ArrayList<>();
        BasePage<Copy> basePage;
        int count = 0;
        do {
            basePage = queryCopies(count, IsmNumberConstant.HUNDRED, conditions);
            copyList.addAll(basePage.getItems());
            count++;
        } while (basePage.getItems().size() == IsmNumberConstant.HUNDRED);
        return copyList;
    }

    /**
     * 查询副本已复制对应的副本记录
     *
     * @param copyId  副本id
     * @return 复制的esn
     */
    @ExterAttack
    @ResponseBody
    @GetMapping("/internal/replicated_copies")
    List<ReplicatedCopies> queryReplicationCopies(@RequestParam("copy_id")String copyId);

    /**
     * 删除资源多余副本
     *
     * @param resourceId 资源id
     * @param request 删除多余副本请求体
     */
    @ExterAttack
    @DeleteMapping("/internal/copies/{resource_id}/excess-copies")
    void deleteExcessCopies(@PathVariable("resource_id") String resourceId,
        @RequestBody DeleteExcessCopiesRequest request);

    /**
     * update copy status by esn
     *
     * @param param param
     */
    @PutMapping("/internal/copies/status")
    void updateCopyStatusByEsn(@RequestBody CopyStatusUpdateByEsnParam param);

    /**
     * 内部接口归档消息转发
     *
     * @param msg ArchiveMsg
     */
    @PutMapping("/internal/archive/dispatch")
    void dispatchArchive(@RequestBody ArchiveMsg msg);

    /**
     * 查询用户域下指定资源索引状态的副本
     *
     * @param domainId 域id
     * @param resourceId 资源id
     * @param indexStatus 索引状态
     * @return 副本id列表
     */
    @GetMapping("/internal/user/{domain_id}/copies/{resource_id}")
    List<String> queryUserIndexCopyByResourceId(@PathVariable("domain_id") String domainId,
        @PathVariable("resource_id") String resourceId, @RequestParam("index_status") String indexStatus);

    /**
     * 更新指定副本的索引状态
     *
     * @param request 更新副本索引请求
     */
    @PutMapping("/internal/batch/copies/index/status")
    void updateCopyListIndexStatus(@RequestBody UpdateCopyIndexStatusRequest request);

    /**
     * 查询最新的副本，且不是日志副本
     *
     * @param resourceId 资源ID
     * @param range 范围
     * @param filter 查询条件
     * @return 副本
     */
    default Optional<Copy> queryLatestBackupCopyWithOutLog(String resourceId, String range,
        Map<String, Object> filter) {
        Map<String, Object> condition = getQueryParams(filter, resourceId, range);
        Optional<Copy> copyOptional = Optional.empty();
        int pageIndex = 0;
        BasePage<Copy> page = queryCopies(pageIndex, 200, condition, Collections.singletonList("-display_timestamp"));
        while (page.getTotal() > 0) {
            copyOptional = page.getItems().stream().filter(o -> o.getBackupType() != LOG_BACKUP).findFirst();
            if (copyOptional.isPresent()) {
                break;
            }
            pageIndex++;
            page = queryCopies(pageIndex, 200, condition, Collections.singletonList("-display_timestamp"));
        }
        return copyOptional;
    }
}
