package openbackup.data.access.framework.livemount.common.enums;

/**
 * Live Mount操作类型枚举类
 *
 * @author tWX1009756
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-03-28
 */
public enum OperationEnums {
    // 创建即时挂载
    CREATE(0),
    // 修改即时挂载
    MODIFY(1),
    // 更新及时挂载
    UPDATE(2),
    // 取消即时挂载
    CANCEL(3);


    private final int type;

    OperationEnums(int type) {
        this.type = type;
    }

    public int getType() {
        return type;
    }
}
