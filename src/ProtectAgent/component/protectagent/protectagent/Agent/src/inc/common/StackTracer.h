#ifndef _AGENT_STACK_TRACER_H_
#define _AGENT_STACK_TRACER_H_

#include <string>
#include <vector>
#include "common/Defines.h"

class StackTracer {
public:
    StackTracer();
    ~StackTracer();

private:
    void InstallSignalHandler();
    void UninstallSignalHandler();
    
    static void OutputMaps(std::vector<mp_string>& stackStream);
    static int SignalHandler(int signum, void *siginfo, void *ucontext);
    static void* btarray[sizeof(int32_t) * sizeof(int64_t)];
    static void WriteStackContent(const mp_string& stackFile, const std::vector<mp_string>& vecInput);
};

#endif  // STACK_TRACER_H_
