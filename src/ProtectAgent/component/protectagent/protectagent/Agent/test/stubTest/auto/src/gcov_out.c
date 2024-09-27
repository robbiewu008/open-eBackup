// module system test ʱ��rdagent��monitor��������ֻ��ͨ��kill�ķ�ʽ�˳�
// Ĭ�ϵ�kill�˳��󣬲�������.gcda�ļ���������Ҫ���źŽ��в��񣬲�ʹ��exit�����˳�
// ʹ֮����.gcda�ļ�
// ���ļ�����gcov_out.so��ͨ��LD_PRELOAD�����������þ���·��
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
void sighandler(int signo)
{
    exit(signo);
}
__attribute__ ((constructor))
void ctor()
{
    int sigs[] = {
        SIGILL, SIGFPE, SIGABRT, SIGBUS,
        SIGSEGV, SIGHUP, SIGINT, SIGQUIT,
        SIGTERM
    };
    int i;
    struct sigaction sa;
    sa.sa_handler = sighandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND;
    for(i = 0; i < sizeof(sigs)/sizeof(sigs[0]); ++i) {
        if (sigaction(sigs[i], &sa, NULL) == -1) {
            perror("Could not set signal handler");
        }
    }
}