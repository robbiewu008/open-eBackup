/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.common.scurity;

import openbackup.system.base.common.constants.ErrorCodeConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;

import lombok.extern.slf4j.Slf4j;

import java.io.File;
import java.io.FileInputStream;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.KeyStore;
import java.util.Objects;
import java.util.Optional;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.function.Consumer;
import java.util.function.Supplier;

/**
 * The easy key store file watcher. Autoloading key store while key store file changed.
 *
 * @author l00272247
 * @version V200R001C50+: 8.0.1
 * @since 2019-10-30
 */
@Slf4j
public class KeyStoreLoader extends Watcher {
    private final String type;

    private final Supplier<String> token;

    private CompletableFuture<FutureResult<KeyStore>> future = new CompletableFuture<>();

    private final AtomicBoolean completed = new AtomicBoolean();

    private final Object lock = new Object();

    private final Path filePath;

    private Long modifyRecord;

    /**
     * constructor for KeyStoreLoader
     *
     * @param file  key store file path
     * @param token supplier for token, which return the password
     */
    public KeyStoreLoader(String file, Supplier<String> token) {
        this(KeyStore.getDefaultType(), file, token);
    }

    /**
     * constructor for KeyStoreLoader
     *
     * @param type  key store type
     * @param file  key store file path
     * @param token supplier for token, which return the password
     */
    public KeyStoreLoader(String type, String file, Supplier<String> token) {
        this(type, Paths.get(file), token);
    }

    /**
     * constructor for KeyStoreLoader
     *
     * @param file  key store file path
     * @param token supplier for token, which return the password
     */
    public KeyStoreLoader(Path file, Supplier<String> token) {
        this(KeyStore.getDefaultType(), file, token);
    }

    /**
     * constructor for KeyStoreLoader
     *
     * @param type  key store type
     * @param file  key store file path
     * @param token supplier for token, which return the password
     */
    public KeyStoreLoader(String type, Path file, Supplier<String> token) {
        super(file, Kind.INIT, Kind.CREATE, Kind.MODIFY);
        this.type = type;
        this.token = Objects.requireNonNull(token);
        this.filePath = file;
        handle(this::handle);
    }

    /**
     * get key store object
     *
     * @return key store object
     */
    public Optional<FutureResult<KeyStore>> get() {
        if (!running()) {
            start();
        }
        try {
            return Optional.ofNullable(getFuture(false).get());
        } catch (InterruptedException | ExecutionException e) {
            return Optional.of(new FutureResult<>(e));
        }
    }

    /**
     * get key store
     *
     * @return key store
     */
    public KeyStore getKeyStore() {
        return get().map(FutureResult::rethrow)
            .map(FutureResult::getData)
            .orElseThrow(
                () -> new LegoCheckedException(ErrorCodeConstant.SSL_INIT_OR_CONNECT_FAIL, "get ks store failed",
                    log));
    }

    /**
     * reload keystore when modified
     *
     * @return key store
     */
    public KeyStore getLastKeyStore() {
        File ksFile = this.filePath.toFile();
        log.debug("Ks file modify time: {}, record time: {}", ksFile.lastModified(), modifyRecord);
        if (modifyRecord == null || modifyRecord != ksFile.lastModified()) {
            modifyRecord = ksFile.lastModified();
            log.info("Change ks record time: {}", modifyRecord);
            Event event = new Event(this.filePath, "modify");
            handle(event);
        }
        return getKeyStore();
    }

    @Override
    protected void onInitialize() {
        super.onInitialize();
        if (!completed.get()) {
            log.error("The ks file may not exists");
            complete(new FutureResult<>(new LegoCheckedException("SSL_INIT_OR_CONNECT_FAIL")));
        }
    }

    @Override
    public KeyStoreLoader handle(Consumer<Event> handler) {
        super.handle(handler);
        return this;
    }

    private void handle(Event event) {
        String keyStoreFile = event.getPath().toAbsolutePath().toString();

        if (VerifyUtil.isEmpty(keyStoreFile)) {
            log.error("Ks file is null.");
            // 证书错误，根据证书初始化KeyStore失败
            complete(new FutureResult<>(new LegoCheckedException("SSL_INIT_OR_CONNECT_FAIL")));
            return;
        }
        File file = new File(keyStoreFile);
        if (!file.exists()) {
            log.error("Ks file is not exist.");
            // 证书错误，根据证书初始化KeyStore失败
            complete(new FutureResult<>(new LegoCheckedException("SSL_INIT_OR_CONNECT_FAIL")));
            return;
        }
        Path path = file.toPath();

        // Check
        if (!Files.isRegularFile(path, LinkOption.NOFOLLOW_LINKS)) {
            log.error("Not one regular file");
            complete(new FutureResult<>(new LegoCheckedException("Not one regular file")));
            return;
        }

        // 读取keystore文件到KeyStore对象
        // 偶现出现从PM挂载的文件系统，读取ks文件时，读取的字节数据不对，load时出现Short read of DER length报错
        // 这里加上重试机制
        int retryTimes = 0;
        while (retryTimes <= 3) {
            log.info("Start Read ks file. retryTimes: {}", retryTimes);
            try (FileInputStream in = new FileInputStream(file)) {
                KeyStore keyStore = KeyStore.getInstance(type);
                String password = token.get();
                if (VerifyUtil.isEmpty(password)) {
                    log.error("Ks pd is empty. retryTimes: {}", retryTimes);
                    complete(new FutureResult<>(new LegoCheckedException("SSL_INIT_OR_CONNECT_FAIL")));
                    return;
                }
                log.info("Start load ks file. retryTimes: {}", retryTimes);
                keyStore.load(in, password.toCharArray());
                log.info("Success load ks file. retryTimes: {}", retryTimes);
                complete(new FutureResult<>(keyStore));
                log.info("Init ks for {} success. retryTimes: {}", file.getName(), retryTimes);
                return;
            } catch (Exception ex) {
                log.error("Init ks({}) failed.", file.getName(), ExceptionUtil.getErrorMessage(ex));
                if (retryTimes == 3) {
                    // 证书错误，根据证书初始化KeyStore失败
                    complete(new FutureResult<>(new LegoCheckedException("SSL_INIT_OR_CONNECT_FAIL", ex)));
                } else {
                    // 每次重试之前，休眠2s，保证不会因为网络短时间闪断出现问题
                    loadKsSleep();
                }
            } finally {
                retryTimes++;
            }
        }
    }

    private void loadKsSleep() {
        // 每次重试之前，休眠2s，保证不会因为网络短时间闪断出现问题
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            log.error("Sleep error.");
        }
    }

    private void complete(FutureResult<KeyStore> futureResult) {
        completed.compareAndSet(false, true);
        getFuture(true).complete(futureResult);
    }

    private CompletableFuture<FutureResult<KeyStore>> getFuture(boolean isRefresh) {
        synchronized (lock) {
            if (!future.isDone()) {
                return future;
            }
            if (isRefresh) {
                future = new CompletableFuture<>();
            }
            return future;
        }
    }

    @Override
    public KeyStoreLoader delay(long delayTime) {
        super.delay(delayTime);
        return this;
    }

    @Override
    public KeyStoreLoader start() {
        super.start();
        return this;
    }

    @Override
    public KeyStoreLoader stop() {
        super.stop();
        return this;
    }
}
