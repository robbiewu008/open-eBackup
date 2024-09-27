package openbackup.system.base.util;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import com.huawei.emeistor.kms.kmc.util.KmcHelper;
import com.huawei.emeistor.kms.kmc.util.kmc.KmcInstance;
import openbackup.system.base.common.cmd.Command;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.util.SignOpenSslUtils;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.io.File;
import java.nio.file.Files;

/**
 * 功能描述
 *
 * @author l30023229
 * @since 2023-06-09
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest( {
    SignOpenSslUtils.class, File.class, Command.class, Files.class, KmcInstance.class, KmcHelper.class, IOUtils.class,
    FileUtils.class
})
public class SignOpenSslUtilsTest {

    @Test
    public void test_sign_success() throws Exception {
        File file = PowerMockito.mock(File.class);
        PowerMockito.whenNew(File.class).withAnyArguments().thenReturn(file);
        PowerMockito.when(file.exists()).thenReturn(true);
        PowerMockito.mockStatic(KmcHelper.class);
        KmcHelper kmcHelper = PowerMockito.mock(KmcHelper.class);
        PowerMockito.when(KmcHelper.getInstance()).thenReturn(kmcHelper);
        PowerMockito.when(kmcHelper.decrypt(anyString())).thenReturn("token");
        PowerMockito.mockStatic(Command.class);
        PowerMockito.when((Command.runWithSensitiveParams(any(), any()))).thenReturn(0);
        SignOpenSslUtils.sign("", "", "");
        Assert.assertTrue(true);
    }

    @Test(expected = LegoCheckedException.class)
    public void test_sign_command_fail() throws Exception {
        File file = PowerMockito.mock(File.class);
        PowerMockito.whenNew(File.class).withAnyArguments().thenReturn(file);
        PowerMockito.when(file.exists()).thenReturn(false);
        PowerMockito.mockStatic(KmcHelper.class);
        KmcHelper kmcHelper = PowerMockito.mock(KmcHelper.class);
        PowerMockito.when(KmcHelper.getInstance()).thenReturn(kmcHelper);
        PowerMockito.when(kmcHelper.decrypt(anyString())).thenReturn("token");
        PowerMockito.mockStatic(Command.class);
        PowerMockito.when((Command.runWithSensitiveParams(any(), any()))).thenReturn(1);
        SignOpenSslUtils.sign("", "", "");
    }
}
