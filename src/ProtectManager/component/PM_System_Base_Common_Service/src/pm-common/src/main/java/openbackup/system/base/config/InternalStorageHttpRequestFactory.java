package openbackup.system.base.config;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.client.SimpleClientHttpRequestFactory;

import java.io.IOException;
import java.net.HttpURLConnection;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLSocketFactory;

/**
 * 自定义InternalStorageHttpRequestFactory
 *
 * @author y30046482
 * @since 2023-12-23
 */
public class InternalStorageHttpRequestFactory extends SimpleClientHttpRequestFactory {
    private static final Logger logger = LoggerFactory.getLogger(InternalStorageHttpRequestFactory.class);

    private final SSLSocketFactory internalTrustingSslSocketFactory;

    public InternalStorageHttpRequestFactory(SSLSocketFactory internalTrustingSslSocketFactory) {
        this.internalTrustingSslSocketFactory = internalTrustingSslSocketFactory;
    }

    @Override
    protected void prepareConnection(HttpURLConnection connection, String httpMethod) throws IOException {
        if (connection instanceof HttpsURLConnection) {
            prepareHttpsConnection((HttpsURLConnection) connection);
        }
        super.prepareConnection(connection, httpMethod);
    }

    private void prepareHttpsConnection(HttpsURLConnection connection) {
        HostnameVerifier hnv = (hostname, session) -> true;
        connection.setHostnameVerifier(hnv);
        connection.setSSLSocketFactory(internalTrustingSslSocketFactory);
    }
}

