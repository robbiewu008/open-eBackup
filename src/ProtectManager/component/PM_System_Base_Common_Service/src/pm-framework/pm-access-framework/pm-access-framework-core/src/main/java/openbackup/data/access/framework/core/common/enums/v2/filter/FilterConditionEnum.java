package openbackup.data.access.framework.core.common.enums.v2.filter;

/**
 * FilterByEnum
 *
 * @description: 资源过滤条件枚举类，用于跟框架层传递数据
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/11/30
 **/
public enum FilterConditionEnum {
    NAME("Name"),
    ID("ID"),
    FORMAT("Format"),
    MODIFY_TIME("ModifyTime"),
    CREATE_TIME("CreateTime"),
    ACCESS_TIME("AccessTime");

    private String condition;

    FilterConditionEnum(String condition) {
        this.condition = condition;
    }

    public String getCondition() {
        return condition;
    }
}
