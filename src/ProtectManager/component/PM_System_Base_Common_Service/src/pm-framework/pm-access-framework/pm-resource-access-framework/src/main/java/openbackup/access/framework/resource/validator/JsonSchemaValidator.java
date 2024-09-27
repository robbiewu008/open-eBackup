package openbackup.access.framework.resource.validator;

import java.util.List;

/**
 * JsonSchema验证接口定义
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-10-14
 */
public interface JsonSchemaValidator {
    /**
     * 根据json schema校验传入对象的合法性
     *
     * @param <T> 被校验对象的类型
     * @param bean 被校验的对象
     * @param schemaName JsonSchema文件名字，按照统一规定给出，如资源相关的文件名为resourceSubType_define.json
     */
    default <T> void doValidate(T bean, String schemaName) {
        this.doValidate(bean, new String[] {schemaName});
    }

    /**
     * 根据json schema校验传入对象的合法性
     *
     * @param <T> 被校验对象的类型
     * @param bean 被校验的对象
     * @param schemaNames JsonSchema文件名字列表
     */
    <T> void doValidate(T bean, String[] schemaNames);

    /**
     * 返回被校验对象中，类型为secret的字段
     *
     * @param schemaName JsonSchema文件名字，按照统一规定给出，如资源相关的文件名为resourceSubType_define.json
     * @return 被校验对象中，类型为secret的字段列表
     */
    List<String> getSecretFields(String schemaName);

    /**
     * 返回被校验对象中，可被编辑的字段
     *
     * @param schemaName JsonSchema文件名字，按照统一规定给出，如资源相关的文件名为resourceSubType_define.json
     * @return 被校验对象中，可被编辑的字段列表
     */
    List<String> getEditableFields(String schemaName);
}
