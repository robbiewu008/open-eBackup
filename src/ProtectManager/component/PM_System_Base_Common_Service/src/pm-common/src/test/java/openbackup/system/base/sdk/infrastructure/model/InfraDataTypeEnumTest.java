package openbackup.system.base.sdk.infrastructure.model;

import openbackup.system.base.sdk.infrastructure.model.InfraDataTypeEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * InfraDataTypeEnum test
 *
 * @author jwx701567
 * @since 2021-03-12
 */
public class InfraDataTypeEnumTest {

    @Test
    public void get_infra_data_type_enum_ok() {
        String DB = InfraDataTypeEnum.valueOf("DB").getValue();
        Assert.assertEquals("DB", DB);
    }
}
