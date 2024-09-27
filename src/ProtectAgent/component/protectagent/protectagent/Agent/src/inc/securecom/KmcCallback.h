#ifndef KMC_CALL_NACK
#define KMC_CALL_NACK

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

#ifdef WIN32
#include <WinSock2.h>
#include <Wincrypt.h>
#include <Ws2tcpip.h>
#include <direct.h>
#include <io.h>
#include <windows.h>
#else
#include <netinet/in.h>
#include <pthread.h>
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "common/Log.h"
#include "include/wsecv2_type.h"

extern bool g_kmcNotUsePseudoRandomNumbers;

void *Fopen(const char *filePathName, const unsigned int mode);
int Fclose(void *stream);
int Fread(void *buffer, size_t count, void *stream);
int Fwrite(const void *buffer, size_t count, void *stream);
int Fflush(void *stream);
int Fremove(const char *path);
long Ftell(void *stream);
long Fseek(void *stream, long offset, unsigned int origin);
int Feof(void *stream, int *endOfFile);
int Ferrno(void *stream);
int Fexist(const char *filePathName);

int UtcTime(const time_t *curTime, struct tm *curTm);

int  CreateThreadLock(void **phMutex);
void DestroyThreadLock(void *hMutex);
void ThreadLock(void *hMutex);
void ThreadUnlock(void *hMutex);

int  CreateProcLock(void **cProcLock);
void DestroyProcLock(void *dProcLock);
void ProcLock(void *procLock);
void ProcUnlock(void *procUnlock);

int GetEntropy(unsigned char **ppEnt, size_t buffLen);
void CleanupEntropy(unsigned char *pEnt, size_t buffLen);


#endif
