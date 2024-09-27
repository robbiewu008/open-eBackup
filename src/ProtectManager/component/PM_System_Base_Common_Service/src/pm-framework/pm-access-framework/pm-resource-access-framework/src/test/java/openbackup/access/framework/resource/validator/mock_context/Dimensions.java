package openbackup.access.framework.resource.validator.mock_context;

import lombok.Data;

/**
 * JsonSchema校验测试实体类
 *
 * @author w00616953
 * @since 2021-10-12
 */
@Data
public class Dimensions {
    private String name;
    private double length;
    private double width;
    private double height;
}
