package openbackup.system.base.common.scurity;

import openbackup.system.base.common.utils.CommonUtil;
import openbackup.system.base.common.utils.ExceptionUtil;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.nio.file.ClosedWatchServiceException;
import java.nio.file.FileSystems;
import java.nio.file.FileVisitResult;
import java.nio.file.FileVisitor;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.PathMatcher;
import java.nio.file.StandardWatchEventKinds;
import java.nio.file.WatchEvent;
import java.nio.file.WatchKey;
import java.nio.file.WatchService;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Queue;
import java.util.Set;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicLong;
import java.util.function.Consumer;

/**
 * A easy FileSystem watcher
 *
 * @author l00272247
 * @version V200R001C50+: 8.0.1
 * @since 2019-10-30
 */
public class Watcher {
    private static final Logger logger = LoggerFactory.getLogger(BcmX509TrustManager.class);

    private static final Map<Kind, WatchEvent.Kind> EVENT_KIND_MAP = getEventKindMap();

    private static final long DEFAULT_DELAY_TIME = 1000L;

    private final Path path;

    private final PathMatcher matcher;

    private final Set<Kind> kinds;

    private final Queue<Consumer<Event>> handlers = new ConcurrentLinkedQueue<>();

    private final AtomicLong delay = new AtomicLong(DEFAULT_DELAY_TIME);

    private final AtomicBoolean running = new AtomicBoolean();

    private final AtomicBoolean waiting = new AtomicBoolean();

    private final BlockingQueue<Event> queue = new LinkedBlockingQueue<>();

    private WatchService service;

    private Thread shutdownHook;

    /**
     * filw watcher constructor witch default delay time
     *
     * @param file file path
     * @param kinds event kinds to watch
     */
    public Watcher(Path file, Kind... kinds) {
        this(getBasePath(file), getFileName(file), kinds);
    }

    /**
     * constructor
     *
     * @param path path to watch
     * @param glob glob expression
     * @param kinds events to watch
     */
    public Watcher(Path path, String glob, Kind... kinds) {
        this.path = Objects.requireNonNull(path);
        this.matcher = FileSystems.getDefault().getPathMatcher("glob:" + Objects.requireNonNull(glob));
        this.kinds = new HashSet<>(Arrays.asList(Objects.requireNonNull(kinds)));
    }

    private static Path getBasePath(Path file) {
        return Optional.ofNullable(file).map(Path::getParent).orElse(null);
    }

    private static String getFileName(Path file) {
        return Optional.ofNullable(file).map(Path::getFileName).map(Path::toString).orElse(null);
    }

    /**
     * set delay time
     *
     * @param delayTime delay time
     * @return this watcher
     */
    public Watcher delay(long delayTime) {
        delay.set(Math.max(delayTime, DEFAULT_DELAY_TIME));
        return this;
    }

    /**
     * add a handler to watcher
     *
     * @param handler watcher handler
     * @return this watcher
     */
    public Watcher handle(Consumer<Event> handler) {
        if (handler != null) {
            handlers.add(handler);
            if (running.get() && waiting.get()) {
                logger.info("auto start watcher for '%s'", path);
                launch();
            }
        }
        return this;
    }

    private void handle(Event event) {
        for (Consumer<Event> handler : handlers) {
            try {
                handler.accept(event);
            } catch (RuntimeException e) {
                logger.error("handler runtime error, cause: ", ExceptionUtil.getErrorMessage(e));
            }
        }
    }

    /**
     * get running status
     *
     * @return running status
     */
    public boolean running() {
        return running.get();
    }

    /**
     * start watcher
     *
     * @return this watcher
     */
    public Watcher start() {
        if (running.compareAndSet(false, true)) {
            if (!handlers.isEmpty()) {
                launch();
            } else {
                waiting.set(true);
                logger.info("watcher handler list of {} is empty, watcher will start after one handler added", path);
            }
        }
        return this;
    }

    private void launch() {
        logger.debug("watcher of path:{} is starting", path);
        waiting.set(false);
        try {
            walk();
        } catch (IOException e) {
            logger.error("walk for matched files failed, cause: ", ExceptionUtil.getErrorMessage(e));
        }
        new Thread(this::watch, "FileSystemEventWatch").start();
        new Thread(this::dispatch, "FileSystemEventDispatch").start();
        shutdownHook = new Thread(this::onShutdown, "FileSystemWatcherShutdown");
        shutdownHook.setUncaughtExceptionHandler(
            (thread, throwable) -> logger.error("shutdownHook thread execute failed.", throwable));
        Runtime.getRuntime().addShutdownHook(shutdownHook);
    }

    private boolean offer(Event event) {
        return queue.offer(event);
    }

    private void walk() throws IOException {
        if (!kinds.contains(Kind.INIT)) {
            return;
        }
        Files.walkFileTree(path, new FileVisitor<Path>() {
            @Override
            public FileVisitResult preVisitDirectory(Path dir, BasicFileAttributes attrs) throws IOException {
                if (path.equals(dir)) {
                    return FileVisitResult.CONTINUE;
                }
                Path item = path.relativize(dir);
                if (matcher.matches(item)) {
                    handle(new Event(dir, "init"));
                }
                return FileVisitResult.SKIP_SUBTREE;
            }

            @Override
            public FileVisitResult visitFile(Path file, BasicFileAttributes attrs) throws IOException {
                Path item = path.relativize(file);
                if (matcher.matches(item)) {
                    handle(new Event(file, "init"));
                }
                return FileVisitResult.CONTINUE;
            }

            @Override
            public FileVisitResult visitFileFailed(Path file, IOException exc) throws IOException {
                return FileVisitResult.CONTINUE;
            }

            @Override
            public FileVisitResult postVisitDirectory(Path dir, IOException exc) throws IOException {
                return FileVisitResult.CONTINUE;
            }
        });
        onInitialize();
    }

    /**
     * called when init walk done
     */
    protected void onInitialize() {
    }

    private void dispatch() {
        while (running.get()) {
            try {
                Event event = queue.take();
                if (event.getPath() == null) {
                    return;
                }
                Date now = new Date();
                long diff = now.getTime() - event.getTimestamp().getTime();
                long delayTime = delay.get();
                if (diff < delayTime) {
                    CommonUtil.sleep(delayTime - diff);
                }
                if (combine(event)) {
                    continue;
                }
                handle(event);
            } catch (InterruptedException e) {
                logger.error("take event from queue failed, cause: ", ExceptionUtil.getErrorMessage(e));
            }
        }
    }

    private boolean combine(Event event) {
        for (Event item : queue) {
            if (item.getPath().equals(event.getPath())) {
                logger.info("combine {} to {}", event, item);
                item.getKinds().addAll(0, event.getKinds());
                return true;
            }
        }
        return false;
    }

    private void onShutdown() {
        logger.info("close watcher of {} before system shutdown", path);
        stop(false);
    }

    /**
     * stop watcher
     *
     * @return this watcher
     */
    public Watcher stop() {
        stop(true);
        return this;
    }

    private void stop(boolean isManual) {
        if (running.compareAndSet(true, false)) {
            logger.debug("watcher of {} will be stop", path);
            if (waiting.get()) {
                return;
            }
            if (service != null) {
                try {
                    service.close();
                    logger.info("close watch service of {} success", path);
                } catch (IOException e) {
                    logger.error("close watch service of '{}' failed, cause: ", path, ExceptionUtil.getErrorMessage(e));
                }
            }
            if (!offer(new Event(null, Collections.emptyList()))) {
                logger.error("fail to add end event to queue");
            }
            if (isManual) {
                removeShutdownHook();
            }
        }
    }

    private void removeShutdownHook() {
        if (shutdownHook == null) {
            return;
        }
        try {
            Runtime.getRuntime().removeShutdownHook(shutdownHook);
        } catch (IllegalStateException e) {
            logger.error("remove shutdown hook failed", e);
        }
    }

    private void watch() {
        logger.debug("watcher of {} is started", path);
        try {
            service = FileSystems.getDefault().newWatchService();
            WatchEvent.Kind[] items = this.kinds.stream().map(EVENT_KIND_MAP::get).filter(Objects::nonNull)
                    .toArray(WatchEvent.Kind[]::new);
            path.register(service, items);
        } catch (ClosedWatchServiceException | IOException e) {
            logger.error("start watcher failed, cause: ", ExceptionUtil.getErrorMessage(e));
            stop();
            return;
        }
        while (running.get()) {
            WatchKey wk;
            try {
                wk = service.take();
                logger.info("take watch key");
            } catch (ClosedWatchServiceException | InterruptedException ex) {
                logger.error("get watch key failed", ex);
                stop();
                return;
            }

            for (WatchEvent event : wk.pollEvents()) {
                @SuppressWarnings("unchecked")
                WatchEvent<Path> watchEvent = (WatchEvent<Path>) event;
                if (event.kind() == StandardWatchEventKinds.OVERFLOW) {
                    continue;
                }
                Path item = watchEvent.context();
                if (!matcher.matches(item)) {
                    continue;
                }
                String kind = watchEvent.kind().name().replaceFirst("^[^_]*_", "").toLowerCase(Locale.US);
                if (!offer(new Event(path.resolve(item), kind))) {
                    logger.error("fail to add {} event to queue", kind);
                }
            }

            if (!wk.reset()) {
                logger.error("reset watch key failed after event processed");
                stop();
            }
        }
    }

    /**
     * watcher event
     *
     * @since 2019-10-30
     */
    public enum Kind {
        /**
         * INIT
         */
        INIT,
        /**
         * CREATE
         */
        CREATE,
        /**
         * MODIFY
         */
        MODIFY,
        /**
         * DELETE
         */
        DELETE
    }

    /**
     * Watcher EventOfAlarm
     *
     * @since 2019-10-30
     */
    public static class Event {
        private final Path path;

        private final List<String> kinds;

        private final Date timestamp;

        /**
         * event constructor
         *
         * @param path path
         * @param kind event kind
         */
        public Event(Path path, String kind) {
            this(path, Collections.singletonList(kind));
        }

        /**
         * event constructor
         *
         * @param path path
         * @param kinds event kinds
         */
        public Event(Path path, List<String> kinds) {
            this.path = path;
            this.kinds = new ArrayList<>(kinds);
            this.timestamp = new Date();
        }

        public List<String> getKinds() {
            return kinds;
        }

        public Path getPath() {
            return path;
        }

        public Date getTimestamp() {
            return timestamp;
        }

        @Override
        public String toString() {
            return "Event{" + "path=" + path + ", kinds=" + kinds + ", timestamp=" + timestamp + '}';
        }
    }

    private static Map<Kind, WatchEvent.Kind> getEventKindMap() {
        Map<Kind, WatchEvent.Kind> map = new HashMap<>();
        map.put(Kind.CREATE, StandardWatchEventKinds.ENTRY_CREATE);
        map.put(Kind.MODIFY, StandardWatchEventKinds.ENTRY_MODIFY);
        map.put(Kind.DELETE, StandardWatchEventKinds.ENTRY_DELETE);
        return Collections.unmodifiableMap(map);
    }
}
