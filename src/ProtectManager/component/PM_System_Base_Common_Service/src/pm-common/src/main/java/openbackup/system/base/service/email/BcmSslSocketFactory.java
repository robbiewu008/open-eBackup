package openbackup.system.base.service.email;

import openbackup.system.base.common.constants.ErrorCodeConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;

import lombok.Getter;
import lombok.Setter;
import lombok.extern.slf4j.Slf4j;

import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;

import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;

/**
 * SSL Socket工厂
 *
 * @author l90005176
 * @version [OceanStor BCManager V200R001C00, 2016年2月16日]
 * @since 2018-01-01
 */
@Slf4j
@Getter
@Setter
public abstract class BcmSslSocketFactory extends SSLSocketFactory {
    /**
     * IP地址
     */
    private String ip;

    /**
     * 安全协议
     */
    private String[] protocols;

    /**
     * 使用指定的TrustManager构造SSLSocketFactory
     *
     * @param ipAddress ip地址
     * @param protocols 建立连接时使用的使用加密协议，如果为空，则默认使用"TLSv1.1","TLSv1.2""TLSv1.3"协议尝试连接。
     */
    public BcmSslSocketFactory(String ipAddress, String... protocols) {
        if (VerifyUtil.isEmpty(ipAddress)) {
            log.error("ipAddress is null.");
            throw new LegoCheckedException(ErrorCodeConstant.SSL_INIT_OR_CONNECT_FAIL);
        }

        this.ip = ipAddress;
        this.protocols = VerifyUtil.isEmpty(protocols) ? new String[] {"TLSv1.1", "TLSv1.2", "TLSv1.3"} : protocols;
    }

    /**
     * 创建一个未连接的套接字。
     *
     * @return Socket 套接字
     * @throws IOException 如果不能创建套接字
     */
    @Override
    public Socket createSocket() throws IOException {
        SSLSocketFactory sslSocketFactory = createSslSocketFactory();
        Socket socket = sslSocketFactory.createSocket();
        if (socket instanceof SSLSocket) {
            ProtocolManager.resetEnabledProtocolsAndCipherSuites((SSLSocket) socket, protocols);
        }

        return socket;
    }

    /**
     * 返回在连接到指定主机的给定端口的现有套接字上分层的套接字。
     *
     * @param socket 已有的套接字
     * @param host 服务器主机
     * @param port 服务器断开
     * @param isAutoClose 关闭此套接字时关闭底层套接字
     * @return 连接到指定主机和端口的套接字
     * @throws IOException 如果创建套接字时出现 I/O 错误
     */
    @Override
    public Socket createSocket(Socket socket, String host, int port, boolean isAutoClose) throws IOException {
        SSLSocketFactory sslSocketFactory = createSslSocketFactory();
        Socket sslSocket = sslSocketFactory.createSocket(socket, host, port, isAutoClose);
        if (sslSocket instanceof SSLSocket) {
            ProtocolManager.resetEnabledProtocolsAndCipherSuites((SSLSocket) sslSocket, protocols);
        }

        return sslSocket;
    }

    /**
     * 创建一个套接字并把它连接到指定远程主机上的指定远程端口。
     *
     * @param host 服务器主机
     * @param port 服务器端口
     * @return Socket 套接字
     * @throws IOException 如果创建套接字时出现 I/O 错误
     */
    @Override
    public Socket createSocket(String host, int port) throws IOException {
        SSLSocketFactory sslSocketFactory = createSslSocketFactory();
        Socket socket = sslSocketFactory.createSocket(host, port);
        if (socket instanceof SSLSocket) {
            ProtocolManager.resetEnabledProtocolsAndCipherSuites((SSLSocket) socket, protocols);
        }

        return socket;
    }

    /**
     * 创建一个套接字并把它连接到指定地址上的指定端口号。
     *
     * @param host 服务器主机
     * @param port 服务器端口
     * @return Socket 套接字
     * @throws IOException 如果创建套接字时出现 I/O 错误
     */
    @Override
    public Socket createSocket(InetAddress host, int port) throws IOException {
        SSLSocketFactory sslSocketFactory = createSslSocketFactory();
        Socket socket = sslSocketFactory.createSocket(host, port);
        if (socket instanceof SSLSocket) {
            ProtocolManager.resetEnabledProtocolsAndCipherSuites((SSLSocket) socket, protocols);
        }

        return socket;
    }

    /**
     * 创建一个套接字并把它连接到指定远程主机上的指定远程端口。套接字还会绑定到提供的本地地址和端口。
     *
     * @param host 服务器主机
     * @param port 服务器端口
     * @param localHost 套接字绑定到的本地地址
     * @param localPort 套接字绑定到的本地端口
     * @return Socket 套接字
     * @throws IOException 如果创建套接字时出现 I/O 错误
     */
    @Override
    public Socket createSocket(String host, int port, InetAddress localHost, int localPort) throws IOException {
        SSLSocketFactory sslSocketFactory = createSslSocketFactory();
        Socket socket = sslSocketFactory.createSocket(host, port, localHost, localPort);
        if (socket instanceof SSLSocket) {
            ProtocolManager.resetEnabledProtocolsAndCipherSuites((SSLSocket) socket, protocols);
        }
        return socket;
    }

    /**
     * 创建一个套接字并把它连接到指定远程端口上的指定远程地址。
     * 套接字还会绑定到提供的本地地址和端口。使用为此工厂建立的套接字选项来配置此套接字。
     *
     * @param address 服务器网络地址
     * @param port 服务器端口
     * @param localAddr 客户端网络地址
     * @param localPort 客户端端口
     * @return Socket 套接字
     * @throws IOException 如果创建套接字时出现 I/O 错误
     */
    @Override
    public Socket createSocket(InetAddress address, int port, InetAddress localAddr, int localPort) throws IOException {
        SSLSocketFactory sslSocketFactory = createSslSocketFactory();
        Socket socket = sslSocketFactory.createSocket(address, port, localAddr, localPort);
        if (socket instanceof SSLSocket) {
            ProtocolManager.resetEnabledProtocolsAndCipherSuites((SSLSocket) socket, protocols);
        }

        return socket;
    }

    /**
     * 返回默认情况下启用的密码套件的列表。
     *
     * @return 默认情况下启用的密码套件的数组。
     */
    @Override
    public String[] getDefaultCipherSuites() {
        SSLSocketFactory sslSocketFactory = createSslSocketFactory();
        return sslSocketFactory.getDefaultCipherSuites();
    }

    /**
     * 返回可以在 SSL 连接上启用的密码套件的名称。
     *
     * @return 密码套件名称的数组
     */
    @Override
    public String[] getSupportedCipherSuites() {
        SSLSocketFactory sslSocketFactory = createSslSocketFactory();
        return sslSocketFactory.getSupportedCipherSuites();
    }

    /**
     * 创建SSLSocketFactory
     *
     * @return SSLSocket工厂类
     */
    public abstract SSLSocketFactory createSslSocketFactory();
}
