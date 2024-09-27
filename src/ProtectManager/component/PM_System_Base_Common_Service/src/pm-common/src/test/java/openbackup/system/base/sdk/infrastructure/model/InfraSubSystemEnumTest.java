package openbackup.system.base.sdk.infrastructure.model;

import openbackup.system.base.sdk.infrastructure.model.InfraSubSystemEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * InfraSubSystemEnum test
 *
 * @author jwx701567
 * @since 2021-03-12
 */
public class InfraSubSystemEnumTest {

    @Test
    public void get_infra_sub_system_enum_status_success() {
        String AISHU = InfraSubSystemEnum.valueOf("AISHU").getValue();
        Assert.assertEquals("AISHU", AISHU);
    }

}
