package openbackup.data.access.framework.protection.common.converters;

import openbackup.data.access.framework.core.common.enums.v2.filter.ResourceFilter;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResourceFilter;

/**
 * TaskResourceFilter对象转换器，用于跟controller层对象转换
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/2
 **/
public class TaskResourceFilterConverter {
    /**
     * 转换为TaskResourceFilter对象
     *
     * @param filter controller层资源过滤器对象
     * @return TaskResourceFilter对象
     */
    public static TaskResourceFilter covertToTaskResourceFilter(ResourceFilter filter) {
        TaskResourceFilter taskResourceFilter = new TaskResourceFilter();
        taskResourceFilter.setFilterBy(filter.getFilterBy().getCondition());
        taskResourceFilter.setMode(filter.getMode().getMode());
        taskResourceFilter.setRule(filter.getRule().getRule());
        taskResourceFilter.setType(filter.getType().getType());
        taskResourceFilter.setValues(filter.getValues());
        return taskResourceFilter;
    }
}
