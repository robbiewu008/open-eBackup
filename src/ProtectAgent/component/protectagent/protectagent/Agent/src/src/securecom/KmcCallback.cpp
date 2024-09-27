#include "common/Path.h"
#include "securecom/CryptAlg.h"
#include "securecom/KmcCallback.h"

bool g_kmcNotUsePseudoRandomNumbers = true;

unsigned long GetRandomNumbers(unsigned char *buff, unsigned int len)
{
    COMMLOG(OS_LOG_INFO, "Start to Get GetRandom numbers, len: %d.", len);
    unsigned long ret = 0xF;
#ifndef WIN32
    FILE *fet = NULL;
    if (g_kmcNotUsePseudoRandomNumbers) {
        fet = fopen("/dev/random", "rb");
    } else {
        fet = fopen("/dev/urandom", "rb");
    }
    if (NULL != fet) {
        if (len == fread(buff, 1, len, fet)) {
            ret = 0;
        }
        fclose(fet);
        fet = NULL;
    }
#else
    HCRYPTPROV hProvider = 0;
    if (TRUE == CryptAcquireContextW(&hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
        if (TRUE == CryptGenRandom(hProvider, len, buff)) {
            ret = 0;
        }
        CryptReleaseContext(hProvider, 0);
    }
#endif
    COMMLOG(OS_LOG_INFO, "Get random num  success");
    return ret;
}

void *Fopen(const char *filePathName, const unsigned int mode)
{
    int flag = 0;
    int retFd = -1;
    int *ret = NULL;
#ifndef WIN32
    switch (KmcFileOpenMode(mode)) {
        case KMC_FILE_READ_BINARY: {
            flag = O_RDONLY;
            break;
        }
        case KMC_FILE_WRITE_BINARY: {
            flag = O_CREAT | O_WRONLY;
            break;
        }
        case KMC_FILE_READWRITE_BINARY: {
            flag = O_CREAT | O_RDWR;
            break;
        }
        default: return nullptr;
    }
    retFd = open(filePathName, flag, S_IRUSR | S_IWUSR);
#else
    switch (KmcFileOpenMode(mode)) {
        case KMC_FILE_READ_BINARY: {
            flag = _O_BINARY | _O_RDONLY;
            break;
        }
        case KMC_FILE_WRITE_BINARY: {
            flag = _O_CREAT | _O_BINARY | _O_WRONLY;
            break;
        }
        case KMC_FILE_READWRITE_BINARY: {
            flag = _O_CREAT | _O_BINARY | _O_RDWR;
            break;
        }
        default: return nullptr;
    }
    retFd = _open(filePathName, flag, _S_IREAD | _S_IWRITE);
#endif
    if (retFd != -1) {
        ret = (int *)malloc(sizeof(int));
        if (ret == nullptr) {
            Fclose(&retFd);
            return nullptr;
        }
        *ret = retFd;
    }
    return ret;
}

int Fclose(void *stream)
{
    int fd = *(int *)stream;
    int ret = 0;
#ifndef WIN32
    ret = close(fd);
#else
    ret = _close(fd);
#endif
    free(stream);
    return ret;
}

int Fread(void *buffer, size_t count, void *stream)
{
    int fd = *(int *)stream;
    long int ret = 0;
#ifndef WIN32
    ret = (long int)read(fd, buffer, count);
#else
    ret = (long int)_read(fd, buffer, count);
#endif
    return (count != (size_t)ret || ret == -1) ? WSEC_FALSE : WSEC_TRUE;
}

int Fwrite(const void *buffer, size_t count, void *stream)
{
    int fd = *(int *)stream;
    long int ret = 0;
#ifndef WIN32
    ret = (long int)write(fd, buffer, count);
    if (fsync(fd) != 0) {
        return WSEC_FALSE;
    }
#else
    ret = (long int)_write(fd, buffer, count);
    if (_commit(fd) != 0) {
        return WSEC_FALSE;
    }
#endif
    return (count != (size_t)ret || ret == -1) ? WSEC_FALSE : WSEC_TRUE;
}

// if success, return 0, faied return -1
int Fflush(void *stream)
{
    int fd = *(int *)stream;
    int ret = 0;
#ifndef WIN32
    ret = fsync(fd);
#else
    ret = _commit(fd);
#endif
    return ret;
}

// if success, return 0, faied return -1
int Fremove(const char *path)
{
    return remove(path);
}

long Ftell(void *stream)
{
    int fd = *(int *)stream;
    long ret = 0;
#ifndef WIN32
    ret = lseek(fd, 0, SEEK_CUR);
#else
    ret = _tell(fd);
#endif
    return ret;
}

// if success, return offset; if failed, return -1
long Fseek(void *stream, long offset, unsigned int origin)
{
    int realOri = 0;
    switch (KmcFileSeekPos(origin)) {
        case KMC_FILE_SEEK_CUR: {
            realOri = SEEK_CUR;
            break;
        }
        case KMC_FILE_SEEK_SET: {
            realOri = SEEK_SET;
            break;
        }
        case KMC_FILE_SEEK_END: {
            realOri = SEEK_END;
            break;
        }
        default: return -1;
            break;
    }
    int ret = -1;
    int fd = *(int *)stream;
#ifndef WIN32
    ret = lseek(fd, offset, realOri);
#else
    ret = _lseek(fd, offset, realOri);
#endif
    return ret;
}

int Feof(void *stream, int *endOfFile)
{
    if (endOfFile == NULL) {
        return -1;
    }
    int fd = *(int *)stream;
#ifndef WIN32
    int curPos = lseek(fd, 0, SEEK_CUR);
    if (curPos == -1) {
        return -1;
    }

    long len = lseek(fd, 0, SEEK_END);
    if (len == -1) {
        return -1;
    }

    if (lseek(fd, curPos, SEEK_SET) != curPos) {
        return -1;
    }
    if (len != curPos) {
        *endOfFile = WSEC_FALSE;
    } else {
        *endOfFile = WSEC_TRUE;
    }
#else
    long ret = _eof(fd);
    if (ret == -1) {
        return -1;
    }

    if (ret != 1) {
        *endOfFile = WSEC_FALSE;
    } else {
        *endOfFile = WSEC_TRUE;
    }
#endif
    return 0;
}

int Ferrno(void *stream)
{
    stream;
    return errno;
}

int Fexist(const char *filePathName)
{
    int ret = 0;
#ifndef WIN32
    ret = access(filePathName, F_OK);
#else
    ret = _access(filePathName, 0);
#endif
    return (ret == 0) ? WSEC_TRUE : WSEC_FALSE;
}

int CreateThreadLock(void **phMutex)
{
    COMMLOG(OS_LOG_DEBUG, "Begin to CreateThreadLock.");
    if (phMutex == NULL) {
        COMMLOG(OS_LOG_ERROR, "CreateThreadLock phMutex is NULL");
        return WSEC_FALSE;
    }
    if (*phMutex != NULL) {
        COMMLOG(OS_LOG_ERROR, "CreateThreadLock phMutex already value=0x%08x", *phMutex);
        return WSEC_FALSE;
    }
#ifdef WIN32
    CRITICAL_SECTION *css = (CRITICAL_SECTION *)malloc(sizeof(CRITICAL_SECTION));
#else
    pthread_mutex_t *css = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
#endif
    if (css == NULL) {
        COMMLOG(OS_LOG_ERROR, "CreateThreadLock css = %08X", css);
        return WSEC_FALSE;
    }
    int retVal = WSEC_FALSE;
#ifdef WIN32
    memset_s(css, sizeof(CRITICAL_SECTION), 0, sizeof(CRITICAL_SECTION));
    InitializeCriticalSection(css);
    retVal = WSEC_TRUE;
#else
    memset_s(css, sizeof(pthread_mutex_t), 0, sizeof(pthread_mutex_t));
    retVal = (0 == pthread_mutex_init(css, NULL)) ? WSEC_TRUE : WSEC_FALSE;
#endif
    if (retVal != 0) {
        *phMutex = css;
    } else {
        free(css);
        css = NULL;
    }
    COMMLOG(OS_LOG_DEBUG, "CreateThreadLock ret=%p", *phMutex);
    return retVal;
}

void DestroyThreadLock(void *hMutex)
{
#ifdef WIN32
    CRITICAL_SECTION *css = (CRITICAL_SECTION *)hMutex;
#else
    pthread_mutex_t *css = (pthread_mutex_t *)hMutex;
#endif
    if (css == NULL) {
        COMMLOG(OS_LOG_ERROR, "DestroyThreadLock hMutex is NULL %06d", 0);
        return;
    }
#ifdef WIN32
    DeleteCriticalSection(css);
#else
    pthread_mutex_destroy(css);
#endif
    free(css);
    css = NULL;
}

void ThreadLock(void *hMutex)
{
#ifdef WIN32
    CRITICAL_SECTION *css = (CRITICAL_SECTION *)hMutex;
#else
    pthread_mutex_t *css = (pthread_mutex_t *)hMutex;
#endif
    if (css == NULL) {
        COMMLOG(OS_LOG_ERROR, "ThreadLock hMutex is NULL", 0);
        return;
    }
#ifdef WIN32
    EnterCriticalSection(css);
#else
    int err = pthread_mutex_lock(css);
    if (err != 0) {
        COMMLOG(OS_LOG_ERROR, "ThreadLock hMutex err: %d.", err);
    }
#endif
}

void ThreadUnlock(void *hMutex)
{
#ifdef WIN32
    CRITICAL_SECTION *css = (CRITICAL_SECTION *)hMutex;
#else
    pthread_mutex_t *css = (pthread_mutex_t *)hMutex;
#endif
    if (NULL == css) {
        COMMLOG(OS_LOG_ERROR, "ThreadUnlock hMutex is NULL");
        return;
    }
#ifdef WIN32
    LeaveCriticalSection(css);
#else
    int err = pthread_mutex_unlock(css);
    if (err != 0) {
        COMMLOG(OS_LOG_ERROR, "ThreadUnlock err:%d.", err);
    }
#endif
}

int CreateProcLock(void **cProcLock)
{
    cProcLock;
    return WSEC_TRUE;
}

void DestroyProcLock(void *dProcLock)
{
    dProcLock;
    return;
}

void ProcLock(void *procLock)
{
    procLock;
    return;
}

void ProcUnlock(void *procUnlock)
{
    procUnlock;
    return;
}

int GetEntropy(unsigned char **ppEnt, size_t buffLen)
{
    int bre = WSEC_FALSE;
    if (buffLen <= 0 || ppEnt == NULL) {
        return bre;
    }

    *ppEnt = (unsigned char *)malloc(buffLen);
    if (*ppEnt == NULL) {
        return bre;
    }

    unsigned long ret = GetRandomNumbers(*ppEnt, buffLen);
    if (ret == 0) {
        bre = WSEC_TRUE;
    }
    if (bre != WSEC_TRUE) {
        memset_s(*ppEnt, buffLen, 0, buffLen);
        free(*ppEnt);
        *ppEnt = NULL;
    }

    return bre;
}

void CleanupEntropy(unsigned char *pEnt, size_t buffLen)
{
    if (pEnt == NULL || buffLen <= 0) {
        return;
    }
    memset_s(pEnt, buffLen, 0, buffLen);
    free(pEnt);
    pEnt = NULL;
}

int UtcTime(const time_t *curTime, struct tm *curTm)
{
    int ret;
#ifndef WIN32
    ret = (gmtime_r(curTime, curTm) == NULL) ? WSEC_FALSE : WSEC_TRUE;
#else
    ret = (gmtime_s(curTm, curTime) == 0) ? WSEC_TRUE : WSEC_FALSE;
#endif
    return ret;
}
