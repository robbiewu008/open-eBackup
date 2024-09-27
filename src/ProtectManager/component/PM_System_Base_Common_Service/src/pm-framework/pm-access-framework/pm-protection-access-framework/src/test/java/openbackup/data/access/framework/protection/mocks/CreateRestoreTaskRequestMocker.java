package openbackup.data.access.framework.protection.mocks;

import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;
import openbackup.data.access.framework.core.common.enums.v2.filter.FilterConditionEnum;
import openbackup.data.access.framework.core.common.enums.v2.filter.FilterModeEnum;
import openbackup.data.access.framework.core.common.enums.v2.filter.FilterRuleEnum;
import openbackup.data.access.framework.core.common.enums.v2.filter.FilterTypeEnum;
import openbackup.data.access.framework.core.common.enums.v2.filter.ResourceFilter;
import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.data.protection.access.provider.sdk.constants.RestoreTaskExtendInfoConstant;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * CreateRestoreTaskRequestMocker
 *
 * @description: CreateRestoreTaskRequest的Mock类，用于mock各种场景的对象
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/11/30
 **/
public class CreateRestoreTaskRequestMocker {

    public static ObjectMapper mapper = new ObjectMapper();
    /**
     * 构造空对象
     *
     * @return 恢复任务请求对象
     */
    private static CreateRestoreTaskRequest buildEmpty() {
        return new CreateRestoreTaskRequest();
    }

    /**
     * 根据参数构造请求对象
     *
     * @param restoreType  恢复类型枚举
     * @param location 恢复位置枚举
     * @return 创建恢复任务请求 {@code CreateRestoreTaskRequest}
     */
    public static CreateRestoreTaskRequest buildWithParams(RestoreTypeEnum restoreType, RestoreLocationEnum location){
        CreateRestoreTaskRequest request = new CreateRestoreTaskRequest();
        request.setCopyId(UUID.randomUUID().toString());
        request.setRestoreType(restoreType);
        request.setTargetLocation(location);
        request.setTargetEnv(UUID.randomUUID().toString());
        request.setTargetObject(UUID.randomUUID().toString());
        request.setAgents(Collections.singletonList("1111"));
        addFilters(request);
        return request;
    }
    /**
     * 填充必填字段
     *
     * @param request 恢复任务请求对象
     * @return
     */
    private static CreateRestoreTaskRequest fillRequiredInfo(CreateRestoreTaskRequest request) {
        request.setCopyId(UUID.randomUUID().toString());
        request.setRestoreType(RestoreTypeEnum.CR);
        request.setTargetEnv(UUID.randomUUID().toString());
        request.setTargetLocation(RestoreLocationEnum.NEW);
        request.setTargetObject("/a/b/c");
        return request;
    }

    /**
     * 构造正确场景的请求对象
     *
     * @return 恢复任务请求对象
     */
    public static CreateRestoreTaskRequest mockSuccessRequest() {
        final CreateRestoreTaskRequest createRestoreTaskRequest = CreateRestoreTaskRequestMocker.buildEmpty();
        CreateRestoreTaskRequestMocker.fillRequiredInfo(createRestoreTaskRequest);
        createRestoreTaskRequest.setAgents(Collections.singletonList(UUID.randomUUID().toString()));
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("aaa", "aa11");
        extendInfo.put("bbb", "bb11");
        extendInfo.put(RestoreTaskExtendInfoConstant.ENABLE_COPY_VERIFY, "true");
        createRestoreTaskRequest.setExtendInfo(extendInfo);
        return createRestoreTaskRequest;
    }

    public static void addFilters(CreateRestoreTaskRequest request){
        List<ResourceFilter> filters = new ArrayList<>();
        ResourceFilter filter1 = new ResourceFilter();
        filter1.setRule(FilterRuleEnum.FUZZY);
        filter1.setMode(FilterModeEnum.INCLUDE);
        filter1.setFilterBy(FilterConditionEnum.ID);
        filter1.setType(FilterTypeEnum.FILE);
        filter1.setValues(Arrays.asList("1", "2", "3"));
        filters.add(filter1);
        request.setFilters(filters);
    }

    public static void addSubObjects(CreateRestoreTaskRequest request){
        request.setSubObjects(Arrays.asList("a.txt", "b.txt", "c.txt"));
    }

    public static String toJsonString(CreateRestoreTaskRequest request) {
        try {
            return mapper.writeValueAsString(request);
        } catch (JsonProcessingException e) {
            e.printStackTrace();
            throw new RuntimeException(e);
        }
    }
}
