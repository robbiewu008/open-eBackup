#include "common/StackTracerTest.h"
#include <vector>

static mp_void StubCLoggerLog(mp_void){
    return;
}

TEST_F(StackTracerTest, StackTracerTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    StackTracer work;
    std::vector<mp_string> stackStream;
    work.OutputMaps(stackStream);
    EXPECT_GT(stackStream.size(), 0);
}

TEST_F(StackTracerTest, SignalHandlerTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    StackTracer work;
    int signum = SIGINT;
    char siginfo[20] = "123456789";
    char ucontext[20] = "123456789";
    work.SignalHandler(signum, siginfo, ucontext);
    signum = SIGSEGV;
    // work.SignalHandler(signum, siginfo, ucontext);

}
