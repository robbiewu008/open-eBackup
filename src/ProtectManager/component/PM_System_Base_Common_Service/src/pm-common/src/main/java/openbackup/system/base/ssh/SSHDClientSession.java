/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.system.base.ssh;

import static openbackup.system.base.common.constants.IsmNumberConstant.ZERO;

import com.google.common.collect.Sets;

import lombok.Cleanup;
import lombok.Getter;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.RetryTemplateUtil;
import openbackup.system.base.common.utils.VerifyUtil;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang3.StringUtils;
import org.apache.sshd.client.ClientBuilder;
import org.apache.sshd.client.SshClient;
import org.apache.sshd.client.channel.ChannelExec;
import org.apache.sshd.client.channel.ClientChannelEvent;
import org.apache.sshd.client.session.ClientSession;
import org.apache.sshd.common.NamedFactory;
import org.apache.sshd.common.PropertyResolverUtils;
import org.apache.sshd.common.SshException;
import org.apache.sshd.common.channel.PtyChannelConfiguration;
import org.apache.sshd.common.kex.BuiltinDHFactories;
import org.apache.sshd.common.kex.KeyExchangeFactory;
import org.apache.sshd.core.CoreModuleProperties;
import org.apache.sshd.sftp.client.SftpClient;
import org.apache.sshd.sftp.client.SftpClientFactory;
import org.apache.sshd.sftp.client.fs.SftpFileSystem;
import org.apache.sshd.sftp.common.SftpException;
import org.eclipse.jgit.internal.transport.sshd.JGitSshClient;
import org.eclipse.jgit.transport.sshd.ProxyData;
import org.springframework.retry.RecoveryCallback;
import org.springframework.retry.RetryCallback;
import org.springframework.retry.support.RetryTemplate;

import java.io.BufferedWriter;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.net.InetSocketAddress;
import java.net.Proxy;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.time.Duration;
import java.util.Collections;
import java.util.EnumSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

import javax.naming.AuthenticationException;

/**
 * SSH客户端(非root用户待完善)
 *
 */
@Slf4j
@Getter
public class SSHDClientSession implements AutoCloseable {
    private static final int DEFAULT_WAIT_TIME_FOR_AUTH_MS = 20000;

    private static final int WAIT_TIME_FOR_CONNECTION_MS = 5000;

    private static final int BUFFER_SIZE = 10 * 1024 * 1024;

    private static final int AGENT_INSTALL_OUT_TIME = 60;

    private static final int AGENT_STAGES_INSTALL_OUT_TIME = 10;

    private static final int AGENT_INSTALL_RETRY_TIME = 6;

    private static final int AGENT_INSTALL_BUFFERING_TIME = 6;

    private static final String AUTH_ERROR_MSG = "No more authentication methods available";

    private static final String LINE_SEPARATOR = "\n";

    private AgentRegisterService agentRegisterService;

    private SshClient sshClient;

    private ClientSession sshdSession;

    private ClientSession sftpSession;

    private SftpClient sftpClient;

    private SftpFileSystem sftpFileSystem;

    private SshUser sshUser;

    private long waitTimeForAuthMS = DEFAULT_WAIT_TIME_FOR_AUTH_MS;

    /**
     * 获取单例方法
     *
     * @param sshUser 用户
     * @return 实例
     * @throws SshException 异常
     * @throws SftpException 异常
     * @throws AuthenticationException 认证异常
     */
    public static SSHDClientSession getInstance(SshUser sshUser)
            throws SshException, SftpException, AuthenticationException {
        return getInstance(sshUser, DEFAULT_WAIT_TIME_FOR_AUTH_MS);
    }

    /**
     * 获取单例方法
     *
     * @param sshUser 用户
     * @param waitTimeForAuth 认证等待时间
     * @return 实例
     * @throws SshException 异常
     * @throws SftpException 异常
     * @throws AuthenticationException 认证异常
     */
    public static SSHDClientSession getInstance(SshUser sshUser, long waitTimeForAuth)
            throws SshException, SftpException, AuthenticationException {
        sshUser.validate();
        SSHDClientSession session = new SSHDClientSession();
        session.waitTimeForAuthMS = waitTimeForAuth;
        session.sshUser = sshUser;
        session.init();
        return session;
    }

    /**
     * 初始化
     *
     * @throws SshException 异常
     * @throws SftpException 异常
     * @throws AuthenticationException 认证异常
     */
    public void init() throws SshException, SftpException, AuthenticationException {
        try {
            if (StringUtils.isNotEmpty(sshUser.getProxyHost()) && sshUser.getProxyPort() > 0) {
                sshClient = initProxyClient();
            } else {
                sshClient = initClient();
            }
            if (sshUser.getPort() > 0) {
                initSsh();
            }
            if (sshUser.getSftpPort() > 0) {
                initSftp();
            }
        } catch (SshException e) {
            if (e.getMessage().contains(AUTH_ERROR_MSG)) {
                throw new AuthenticationException("Authenticate Failed");
            }
            log.error("ssh connect error", ExceptionUtil.getErrorMessage(e));
            throw e;
        } catch (SftpException e) {
            log.error("sftp connect error", ExceptionUtil.getErrorMessage(e));
            throw e;
        } catch (AuthenticationException e) {
            log.error("authenticate info error", ExceptionUtil.getErrorMessage(e));
            throw e;
        } catch (IOException e) {
            log.error("connect error");
        }
    }

    private SshClient initClient() {
        SshClient client = SshClient.setUpDefaultClient();
        initSshClient(client);
        return client;
    }

    private void initSshClient(SshClient client) {
        List<KeyExchangeFactory> factories = NamedFactory.setUpTransformedFactories(false, BuiltinDHFactories.VALUES,
                ClientBuilder.DH2KEX);
        client.setKeyExchangeFactories(factories);
        if (!VerifyUtil.isEmpty(sshUser.getMacTypeList())) {
            client.setMacFactories(sshUser.getMacTypeList());
        } else {
            log.warn("Fail to set mac type list due to empty list.");
        }
        PropertyResolverUtils.updateProperty(client, CoreModuleProperties.AUTH_TIMEOUT.getName(),
                DEFAULT_WAIT_TIME_FOR_AUTH_MS);
        client.start();
    }

    private SshClient initProxyClient() {
        SshClient sshClient0 = ClientBuilder.builder().factory(JGitSshClient::new).build();
        if (sshClient0 instanceof JGitSshClient) {
            JGitSshClient client = (JGitSshClient) sshClient0;
            initSshClient(client);
            Proxy proxy = new Proxy(Proxy.Type.HTTP,
                    new InetSocketAddress(sshUser.getProxyHost(), sshUser.getProxyPort()));
            ProxyData proxyData = new ProxyData(proxy, sshUser.getUsername(), sshUser.getPassword().toCharArray());
            client.setProxyDatabase(remote -> proxyData);
            return client;
        }
        return sshClient0;
    }

    public void setSshService(AgentRegisterService agentRegisterService) {
        this.agentRegisterService = agentRegisterService;
    }

    /**
     * 初始化连接
     *
     * @throws IOException 异常
     * @throws AuthenticationException 认证异常
     */
    public void initSsh() throws IOException, AuthenticationException {
        RetryCallback<Object, Throwable> retryCallback = context -> {
            sshdSession = sshClient.connect(sshUser.getUsername(), sshUser.getIp(), sshUser.getPort())
                    .verify(waitTimeForAuthMS, TimeUnit.MILLISECONDS).getSession();
            sshdSession.addPasswordIdentity(sshUser.getPassword());
            sshdSession.auth().verify();
            if (!sshdSession.isAuthenticated()) {
                throw new AuthenticationException("Authenticate Failed");
            }
            return true;
        };
        RecoveryCallback<Object> recoveryCallback = context -> {
            throw new AuthenticationException("Authenticate Failed");
        };
        RetryTemplate retryTemplate = RetryTemplateUtil.fixedBackOffRetryTemplate(3, 1000L,
                Collections.singletonMap(Exception.class, true));
        try {
            retryTemplate.execute(retryCallback, recoveryCallback);
        } catch (Throwable throwable) {
            throw new AuthenticationException("Authenticate Failed");
        }
    }

    /**
     * 初始化连接
     *
     * @throws IOException 异常
     * @throws AuthenticationException 认证异常
     * @throws SftpException 认证异常
     * @throws SshException 认证异常
     */
    public void initSftp() throws IOException, AuthenticationException, SftpException, SshException {
        RetryCallback<Object, Throwable> retryCallback = context -> {
            if (sshdSession != null) {
                sftpSession = sshdSession;
            } else {
                sftpSession = sshClient.connect(sshUser.getUsername(), sshUser.getIp(), sshUser.getSftpPort())
                        .verify(waitTimeForAuthMS, TimeUnit.MILLISECONDS).getSession();
                sftpSession.addPasswordIdentity(sshUser.getPassword());
                sftpSession.auth().verify();
                if (!sftpSession.isAuthenticated()) {
                    throw new AuthenticationException("Authenticate Failed");
                }
            }
            sftpClient = SftpClientFactory.instance().createSftpClient(sftpSession);
            sftpFileSystem = SftpClientFactory.instance().createSftpFileSystem(sftpSession);
            log.info("connect sftp {} success", sshUser.getIp());
            return true;
        };
        RecoveryCallback<Object> recoveryCallback = context -> {
            Throwable lastException = context.getLastThrowable();
            if (lastException instanceof Exception) {
                throw (Exception) lastException;
            }
            return null;
        };
        RetryTemplate retryTemplate = RetryTemplateUtil.fixedBackOffRetryTemplate(3, 1000L,
                Collections.singletonMap(Exception.class, true));
        try {
            retryTemplate.execute(retryCallback, recoveryCallback);
        } catch (Throwable throwable) {
            if (throwable instanceof AuthenticationException) {
                log.error("AuthenticationException", ExceptionUtil.getErrorMessage(throwable));
                throw (AuthenticationException) throwable;
            } else if (throwable instanceof SftpException) {
                log.error("SftpException", ExceptionUtil.getErrorMessage(throwable));
                throw (SftpException) throwable;
            } else if (throwable instanceof SshException) {
                log.error("SshException", ExceptionUtil.getErrorMessage(throwable));
                throw (SshException) throwable;
            } else {
                log.error("need to add more Exception to cover", ExceptionUtil.getErrorMessage(throwable));
                throw new AuthenticationException("Authenticate Failed");
            }
        }
    }

    /**
     * 执行单个命令的返回
     *
     * @param command 命令
     * @return 返回值
     * @throws IOException 异常
     */
    public String exec(String command) throws IOException {
        if (sshUser.isSuperUser()) {
            try (ByteArrayOutputStream outputErr = new ByteArrayOutputStream()) {
                return sshdSession.executeRemoteCommand(command, outputErr, StandardCharsets.UTF_8)
                        .replaceAll(LINE_SEPARATOR, "").trim();
            }
        }
        PtyChannelConfiguration ptyConfig = new PtyChannelConfiguration();
        String sudoCommand = sudoCommand(command);
        try (ChannelExec channel = sshdSession.createExecChannel(sudoCommand, ptyConfig, Collections.emptyMap());
                ByteArrayOutputStream output = new ByteArrayOutputStream();
                ByteArrayOutputStream outputErr = new ByteArrayOutputStream()) {
            channel.setOut(output);
            channel.setErr(outputErr);
            channel.setUsePty(true);
            channel.open().verify(WAIT_TIME_FOR_CONNECTION_MS, TimeUnit.MILLISECONDS);
            if (Boolean.FALSE.equals(sshUser.getIsScapeSudoPassword())) {
                String pwd = sshUser.getSuperPassword().concat(LINE_SEPARATOR);
                channel.getInvertedIn().write(pwd.getBytes(StandardCharsets.UTF_8));
                channel.getInvertedIn().flush();
            }
            channel.waitFor(EnumSet.of(ClientChannelEvent.CLOSED), Duration.ofSeconds(AGENT_INSTALL_OUT_TIME));
            return output.toString().replaceAll("xxxxx", "")
                    .replaceAll(
                            Boolean.FALSE.equals(sshUser.getIsScapeSudoPassword()) ? sshUser.getSuperPassword() : "",
                            "")
                    .replaceAll(LINE_SEPARATOR, "").trim();
        }
    }

    /**
     * 检查命令是否存在
     *
     * @param command command
     * @return 检查结果
     * @throws IOException IOException
     */
    public CmdExecRes checkCommand(String command) throws IOException {
        try (ChannelExec channel = sshdSession.createExecChannel(command);
                ByteArrayOutputStream output = new ByteArrayOutputStream();
                ByteArrayOutputStream outputErr = new ByteArrayOutputStream()) {
            channel.setOut(output);
            channel.setErr(outputErr);
            channel.setUsePty(true);
            channel.open().verify(WAIT_TIME_FOR_CONNECTION_MS, TimeUnit.MILLISECONDS);
            channel.waitFor(EnumSet.of(ClientChannelEvent.CLOSED), 0L);
            return new CmdExecRes(channel.getExitStatus(), outputErr.toString());
        }
    }

    /**
     * 执行命令返回执行exitStatus
     *
     * @param command 命令
     * @return 返回exitStatus
     * @throws IOException 异常
     */
    public CmdExecRes execExitStatus(String command) throws IOException {
        log.info("command:{},isSuperUser:{}", command, sshUser.isSuperUser());
        if (sshUser.isSuperUser()) {
            try (ChannelExec channel = sshdSession.createExecChannel(command);
                    ByteArrayOutputStream output = new ByteArrayOutputStream();
                    ByteArrayOutputStream outputErr = new ByteArrayOutputStream()) {
                channel.setOut(output);
                channel.setErr(outputErr);
                channel.setUsePty(true);
                channel.open().verify(WAIT_TIME_FOR_CONNECTION_MS, TimeUnit.MILLISECONDS);
                channel.waitFor(EnumSet.of(ClientChannelEvent.CLOSED), 0L);
                return new CmdExecRes(channel.getExitStatus(), outputErr.toString());
            }
        } else {
            PtyChannelConfiguration ptyConfig = new PtyChannelConfiguration();
            String sudoCommand = sudoCommand(command);
            try (ChannelExec channel = sshdSession.createExecChannel(sudoCommand, ptyConfig, Collections.emptyMap());
                    ByteArrayOutputStream output = new ByteArrayOutputStream();
                    ByteArrayOutputStream outputErr = new ByteArrayOutputStream()) {
                channel.setOut(output);
                channel.setErr(outputErr);
                channel.setUsePty(true);
                channel.open().verify(WAIT_TIME_FOR_CONNECTION_MS, TimeUnit.MILLISECONDS);
                if (Boolean.FALSE.equals(sshUser.getIsScapeSudoPassword())) {
                    log.info("Exec command without sudo pd,userName={}", sshUser.getUsername());
                    String pwd = sshUser.getSuperPassword().concat(LINE_SEPARATOR);
                    channel.getInvertedIn().write(pwd.getBytes(StandardCharsets.UTF_8));
                    channel.getInvertedIn().flush();
                }
                channel.waitFor(EnumSet.of(ClientChannelEvent.CLOSED), Duration.ofSeconds(AGENT_INSTALL_OUT_TIME));
                if (StringUtils.isNotEmpty(outputErr.toString())) {
                    log.error("execute command error:{}", outputErr);
                }
                return new CmdExecRes(channel.getExitStatus(), output.toString());
            } catch (NullPointerException e) {
                log.error("execExitStatus error", ExceptionUtil.getErrorMessage(e));
                throw new IOException("Channel is closed", e);
            }
        }
    }

    /**
     * 执行命令交互的参数(Agent注册专用)
     *
     * @param ip agent的endpoint
     * @param command 命令
     * @param param 参数
     * @param isRegister 是否是agent注册命令
     * @return 执行结果
     * @throws IOException 异常
     */
    public CmdExecRes execExitStatusForAgent(String ip, String command, String param, boolean isRegister)
            throws IOException {
        log.info("command:{},isSuperUser:{},params:{},isRegister:{}", command, sshUser.isSuperUser(), param,
                isRegister);
        if (sshUser.isSuperUser()) {
            try (ChannelExec channel = sshdSession.createExecChannel(command);
                    ByteArrayOutputStream output = new ByteArrayOutputStream();
                    ByteArrayOutputStream outputErr = new ByteArrayOutputStream()) {
                channel.setOut(output);
                channel.setErr(outputErr);
                channel.setUsePty(true);
                channel.open().verify(WAIT_TIME_FOR_CONNECTION_MS, TimeUnit.MILLISECONDS);
                // agent注册成功条件，反向注册成功或是脚本执行返回
                return getCmdExecRes(ip, channel, outputErr, isRegister);
            }
        } else {
            PtyChannelConfiguration ptyConfig = new PtyChannelConfiguration();
            String sudoCommand = sudoCommand(command);
            try (ChannelExec channel = sshdSession.createExecChannel(sudoCommand, ptyConfig, Collections.emptyMap());
                    ByteArrayOutputStream output = new ByteArrayOutputStream();
                    ByteArrayOutputStream outputErr = new ByteArrayOutputStream()) {
                channel.setOut(output);
                channel.setErr(outputErr);
                channel.setUsePty(true);
                channel.open().verify(WAIT_TIME_FOR_CONNECTION_MS, TimeUnit.MILLISECONDS);
                @Cleanup
                BufferedWriter bufferedWriter = new BufferedWriter(
                        new OutputStreamWriter(channel.getInvertedIn(), StandardCharsets.UTF_8));
                if (Boolean.FALSE.equals(sshUser.getIsScapeSudoPassword())) {
                    log.info("Exec command without sudo pd,userName={}", sshUser.getUsername());
                    String pwd = sshUser.getSuperPassword().concat(LINE_SEPARATOR);
                    bufferedWriter.write(pwd);
                    bufferedWriter.flush();
                }
                // agent注册成功条件，反向注册成功或是脚本执行返回
                return getCmdExecRes(ip, channel, outputErr, isRegister);
            } catch (IOException | NullPointerException e) {
                log.error("execExitStatus error", ExceptionUtil.getErrorMessage(e));
                throw new IOException("Channel is closed", e);
            }
        }
    }

    private CmdExecRes getCmdExecRes(String ip, ChannelExec channel, ByteArrayOutputStream outputErr,
            boolean isRegister) {
        int correctRetryTimes = ZERO;
        // 非注册agent场景等待返回
        if (!isRegister) {
            channel.waitFor(EnumSet.of(ClientChannelEvent.CLOSED), Duration.ofMinutes(AGENT_INSTALL_OUT_TIME));
            return new CmdExecRes(channel.getExitStatus(), outputErr.toString());
        }
        // 等待注册脚本返回，重试6次最多等待60分钟，期间每十分钟检查环境agent注册情况
        while (correctRetryTimes++ < AGENT_INSTALL_RETRY_TIME) {
            // 异常场景，反向注册成功但是通道尚未关闭
            if (agentRegisterService.isAgentOnline(ip) && !channel.isClosed()) {
                channel.waitFor(EnumSet.of(ClientChannelEvent.CLOSED),
                        Duration.ofMinutes(AGENT_INSTALL_BUFFERING_TIME));
                // 通道正常关闭
                if (channel.getExitStatus() != null) {
                    return new CmdExecRes(channel.getExitStatus(), outputErr.toString());
                }
                log.info("The channel of {} is not closed properly.", ip);
                channel.close(false);
                // 强行将任务置为成功
                return new CmdExecRes(0, outputErr.toString());
            }
            // 正常场景处理
            channel.waitFor(EnumSet.of(ClientChannelEvent.CLOSED), Duration.ofMinutes(AGENT_STAGES_INSTALL_OUT_TIME));
            if (channel.getExitStatus() != null) {
                return new CmdExecRes(channel.getExitStatus(), outputErr.toString());
            }
        }
        return new CmdExecRes(channel.getExitStatus(), outputErr.toString());
    }

    private String sudoCommand(String command) {
        return "sudo -p 'xxxxx' ".concat(command);
    }

    /**
     * 上传文件
     *
     * @param local 本地文件名
     * @param remote 远程文件路径
     * @param filePermissions 文件权限
     * @param dirPermissions 目录权限
     * @throws SftpException sftpException
     */
    public void uploadFile(String local, String remote, Set<PosixFilePermission> filePermissions,
            Set<PosixFilePermission> dirPermissions) throws SftpException {
        try {
            validateSftpChannel();
            File file = new File(remote);
            String directory = file.getParent();
            String fileName = file.getName();
            Path remoteParentPath = sftpFileSystem.getDefaultDir().resolve(directory);
            if (Files.notExists(remoteParentPath)) {
                Files.createDirectories(remoteParentPath);
                if (CollectionUtils.isNotEmpty(dirPermissions)) {
                    Files.setPosixFilePermissions(remoteParentPath, dirPermissions);
                }
            }
            Path remoteFilePath = remoteParentPath.resolve(fileName);
            if (Files.exists(remoteFilePath)) {
                Files.delete(remoteFilePath);
            }
            Files.copy(Paths.get(local), remoteFilePath);
            if (CollectionUtils.isNotEmpty(filePermissions)) {
                Files.setPosixFilePermissions(remoteFilePath, filePermissions);
            }
            log.info("upload file success, file name :{}", remoteFilePath.getFileName());
        } catch (SftpException e) {
            log.error("upload file sftp error", ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.CLIENT_REGISTER_FAILED, "insufficient user rights.");
        } catch (IOException e) {
            log.error("upload file error", ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "upload file error.");
        }
    }

    /**
     * 强制上传文件，非root但是有root权限用户会修改属主并上传
     *
     * @param local 本地路径
     * @param remote 远程路径
     * @param filePermissions 文件权限
     * @param dirPermissions 目录权限
     * @throws IOException 异常
     */
    public void uploadFileForce(String local, String remote, Set<PosixFilePermission> filePermissions,
            Set<PosixFilePermission> dirPermissions) throws IOException {
        validateSftpChannel();
        if (!sshUser.isSuperUser()) {
            File file = new File(remote);
            String directory = file.getParent();
            Path remoteParentPath = sftpFileSystem.getDefaultDir().resolve(directory);
            exec("mkdir -p " + remoteParentPath);
            exec("chown -R " + sshUser.getUsername() + " " + remoteParentPath);
        }
        uploadFile(local, remote, filePermissions, dirPermissions);
    }

    private void validateSftpChannel() throws SftpException {
        if (Objects.isNull(sftpClient) || !sftpClient.getChannel().isOpen()) {
            throw new SftpException(3, "Permission Deny");
        }
    }

    /**
     * 创建文件夹
     *
     * @param remote 文件夹名称
     * @param dirPermissions 文件夹权限
     * @throws SftpException 异常
     */
    public void mkdir(String remote, Set<PosixFilePermission> dirPermissions) throws SftpException {
        try {
            validateSftpChannel();
            Path remoteParentPath = sftpFileSystem.getDefaultDir().resolve(remote);
            if (Files.notExists(remoteParentPath)) {
                Files.createDirectories(remoteParentPath);
                if (CollectionUtils.isNotEmpty(dirPermissions)) {
                    Files.setPosixFilePermissions(remoteParentPath, dirPermissions);
                }
            } else {
                sftpClient.open(remote);
                sftpClient.read(remote);
                if (!Files.isDirectory(remoteParentPath)) {
                    sftpClient.write(remote);
                }
            }
            log.info("create directory success, directory name :{}", remoteParentPath.getFileName());
        } catch (SftpException e) {
            log.error("mkdir sftp error", ExceptionUtil.getErrorMessage(e));
            throw e;
        } catch (IOException e) {
            log.error("mkdir sftp io error", ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "upload file error.");
        }
    }

    /**
     * 测试时校验文件夹权限
     *
     * @param remote 文件夹名称
     * @throws SftpException 异常
     */
    public void checkPathPermission(String remote) throws SftpException {
        try {
            validateSftpChannel();
            Path remoteParentPath = sftpFileSystem.getDefaultDir().resolve(remote);
            if (Files.notExists(remoteParentPath)) {
                Files.createDirectories(remoteParentPath);
                Files.setPosixFilePermissions(remoteParentPath, Sets.newHashSet(PosixFilePermission.OWNER_EXECUTE,
                        PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE));
                return;
            }
            sftpClient.open(remote);
            sftpClient.read(remote);
            if (!Files.isDirectory(remoteParentPath)) {
                sftpClient.write(remote);
            }
            log.info("Test sftp success :{}", remoteParentPath.getFileName());
        } catch (SftpException e) {
            log.error("Test sftp error", ExceptionUtil.getErrorMessage(e));
            throw e;
        } catch (IOException e) {
            log.error("Test sftp io error", ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "upload file error.");
        }
    }

    /**
     * 下载文件
     *
     * @param local 本地文件路径
     * @param remote 远程文件路径
     * @throws SftpException sftpException
     */
    public void downloadFile(String local, String remote) throws SftpException {
        try {
            validateSftpChannel();
            File file = new File(remote);
            String directory = file.getParent();
            String fileName = file.getName();
            Path localPath = Paths.get(local);
            Files.deleteIfExists(localPath);
            Path localParent = Paths.get(local.substring(0, local.lastIndexOf(File.separator)));
            if (Files.notExists(localParent)) {
                Files.createDirectories(localParent);
            }
            Path remotePath = sftpFileSystem.getDefaultDir().resolve(directory).resolve(fileName);
            Files.copy(remotePath, localPath);
        } catch (SftpException e) {
            log.error("download file sftp error", ExceptionUtil.getErrorMessage(e));
            throw e;
        } catch (IOException e) {
            log.error("download file error", ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "download file error.");
        }
    }

    /**
     * 文件大小
     *
     * @param remote 远程文件路径
     * @return 大小
     * @throws SftpException 异常
     */
    public long fileSize(String remote) throws SftpException {
        try {
            validateSftpChannel();
            File file = new File(remote);
            String directory = file.getParent();
            String fileName = file.getName();
            Path remotePath = sftpFileSystem.getDefaultDir().resolve(directory).resolve(fileName);
            if (!Files.exists(remotePath)) {
                return 0L;
            }
            return Files.size(remotePath);
        } catch (SftpException e) {
            log.error("query remote file size error", ExceptionUtil.getErrorMessage(e));
            throw e;
        } catch (IOException e) {
            log.error("remote file not exists", ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "delete file error.");
        }
    }

    /**
     * 删除文件
     *
     * @param remote 远程文件
     * @throws SftpException sftpException
     */
    public void deleteFile(String remote) throws SftpException {
        try {
            validateSftpChannel();
            File file = new File(remote);
            String directory = file.getParent();
            String fileName = file.getName();
            Path remotePath = sftpFileSystem.getDefaultDir().resolve(directory).resolve(fileName);
            if (Files.exists(remotePath)) {
                Files.delete(remotePath);
            }
        } catch (SftpException e) {
            log.error("delete file sftp error", ExceptionUtil.getErrorMessage(e));
            throw e;
        } catch (IOException e) {
            log.error("delete file error", ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "delete file error.");
        }
    }

    /**
     * 删除文件，在文件夹为空的情况下，删除上一级目录
     *
     * @param remote 文件路径
     * @throws SftpException sftpException
     */
    public void deleteSingleDirectory(String remote) throws SftpException {
        try {
            validateSftpChannel();
            File file = new File(remote);
            String directory = file.getParent();
            String fileName = file.getName();
            Path remoteParentPath = sftpFileSystem.getDefaultDir().resolve(directory);
            Path remotePath = remoteParentPath.resolve(fileName);
            Files.delete(remotePath);
            List<Path> allFile = Files.list(remoteParentPath).collect(Collectors.toList());
            if (CollectionUtils.isEmpty(allFile)) {
                Files.delete(remoteParentPath);
            }
        } catch (SftpException e) {
            log.error("delete file sftp error", ExceptionUtil.getErrorMessage(e));
            throw e;
        } catch (IOException e) {
            log.error("delete file error", ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "delete file error.");
        }
    }

    /**
     * 关闭
     */
    @Override
    public void close() {
        IOUtils.closeQuietly(sshClient, sshdSession, sftpSession, sftpClient, sftpFileSystem);
    }
}
