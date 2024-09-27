package openbackup.system.base.common.scurity;

import openbackup.system.base.common.rest.CommonX509TrustHandler;
import openbackup.system.base.common.scurity.BcmX509TrustManager;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import sun.security.x509.X509CertImpl;

import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.X509TrustManager;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateExpiredException;
import java.security.cert.CertificateNotYetValidException;
import java.util.Arrays;
import java.util.Enumeration;
import java.util.List;

import static org.mockito.ArgumentMatchers.any;

/**
 * 类描述
 *
 * @author fwx1022842
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022/4/22
 */
@RunWith(PowerMockRunner.class)
@PowerMockIgnore("javax.security.*")
@PrepareForTest({BcmX509TrustManager.class, KeyStore.class, TrustManagerFactory.class, X509TrustManager.class, X509CertImpl.class})
public class BcmX509TrustManagerTest {
    @Test
    public void test() throws InvocationTargetException, IllegalAccessException, NoSuchAlgorithmException, KeyStoreException, CertificateNotYetValidException, CertificateExpiredException {
        PowerMockito.mockStatic(TrustManagerFactory.class);

        TrustManagerFactory factory = PowerMockito.mock(TrustManagerFactory.class);

        PowerMockito.when(TrustManagerFactory.getInstance(any())).thenReturn(factory);

        X509TrustManager x509TrustManager = PowerMockito.mock(X509TrustManager.class);
        TrustManager [] x509TrustManagerArr = {x509TrustManager};
        PowerMockito.when(factory.getTrustManagers()).thenReturn(x509TrustManagerArr);

        KeyStore keyStore = PowerMockito.mock(KeyStore.class);


        Enumeration<String> enumeration = new Enumeration<String>() {
            final List<String> str = Arrays.asList("1", "2", "3");
            int num = 0;
            @Override
            public boolean hasMoreElements() {
                return num <= str.size() - 1;
            }

            @Override
            public String nextElement() {
                String s = str.get(num);
                num++;
                return s;
            }
        };

        PowerMockito.when(keyStore.aliases()).thenReturn(enumeration);
        CommonX509TrustHandler commonX509TrustHandler = PowerMockito.mock(CommonX509TrustHandler.class);
        BcmX509TrustManager bcmX509TrustManager = new BcmX509TrustManager(keyStore, commonX509TrustHandler);
        X509CertImpl certificateValid1 = PowerMockito.mock(X509CertImpl.class);
        PowerMockito.when(keyStore.getCertificate("1")).thenReturn(certificateValid1);
        X509CertImpl certificateInValid2 = PowerMockito.mock(X509CertImpl.class);
        PowerMockito.when(keyStore.getCertificate("2")).thenReturn(certificateInValid2);
        X509CertImpl certificateValid3 = PowerMockito.mock(X509CertImpl.class);
        PowerMockito.when(keyStore.getCertificate("3")).thenReturn(certificateValid3);
        PowerMockito.doNothing().when(certificateValid1).checkValidity();
        PowerMockito.doNothing().when(certificateValid3).checkValidity();
        PowerMockito.doThrow(new CertificateExpiredException()).when(certificateInValid2).checkValidity();

        PowerMockito.doThrow(new KeyStoreException("Uninitialized keystore")).when(keyStore).deleteEntry("1");
        PowerMockito.doThrow(new KeyStoreException("Uninitialized keystore")).when(keyStore).deleteEntry("3");

        Method method = PowerMockito.method(BcmX509TrustManager.class, "removeExpireKeyStore", KeyStore.class);
        method.invoke(bcmX509TrustManager, keyStore);
    }

    /**
     * 用例场景：获取受信的证书别名
     * 前置条件：keystore不是null
     * 检查点: 不报错
     */
    @Test
    public void get_accepted_issuers_when_keystore_is_not_null() throws Exception {
        PowerMockito.mockStatic(TrustManagerFactory.class);
        TrustManagerFactory factory = PowerMockito.mock(TrustManagerFactory.class);
        PowerMockito.when(TrustManagerFactory.getInstance(any())).thenReturn(factory);
        X509TrustManager x509TrustManager = PowerMockito.mock(X509TrustManager.class);
        TrustManager [] x509TrustManagerArr = {x509TrustManager};
        PowerMockito.when(factory.getTrustManagers()).thenReturn(x509TrustManagerArr);
        KeyStore keyStore = PowerMockito.mock(KeyStore.class);
        Enumeration<String> enumeration = new Enumeration<String>() {
            final List<String> str = Arrays.asList("1", "2", "3");
            int num = 0;
            @Override
            public boolean hasMoreElements() {
                return num <= str.size() - 1;
            }

            @Override
            public String nextElement() {
                String s = str.get(num);
                num++;
                return s;
            }
        };
        PowerMockito.when(keyStore.aliases()).thenReturn(enumeration);
        CommonX509TrustHandler commonX509TrustHandler = PowerMockito.mock(CommonX509TrustHandler.class);
        BcmX509TrustManager bcmX509TrustManager = new BcmX509TrustManager(keyStore, commonX509TrustHandler);
        Method method = PowerMockito.method(BcmX509TrustManager.class, "getAcceptedIssuers");
        method.invoke(bcmX509TrustManager);
    }

    /**
     * 用例场景：获取受信的证书别名
     * 前置条件：keystore是null
     * 检查点: 不报错
     */
    @Test
    @Ignore
    public void get_accepted_issuers_when_keystore_is_null() throws InvocationTargetException, IllegalAccessException {
        BcmX509TrustManager bcmX509TrustManager = new BcmX509TrustManager(new CommonX509TrustHandler());
        Method method = PowerMockito.method(BcmX509TrustManager.class, "getAcceptedIssuers");
        method.invoke(bcmX509TrustManager);
    }
}
