/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

����:
wanghao 			w00296180

��ģ��ʵ�ֱ������ص�����

ʵ�ּ�¼�����ʵĹ���

�����������֧�֣���Ҫע�͵�HAS_LLVM����


�±���������Ϊ��ģ��ʵ�ֵĶ����ṩ�ĸ����ʷ���������
�������������Ҳ��ʵ�ָ����ʷ�������afl����ʵ���±������������滻��ģ��
LlvmTracePcIsHasNewFeature
LlvmTracePcStartFeature
LlvmTracePcEndFeature

*/
#include "PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifdef HAS_TRACE_PC
 
ATTRIBUTE_INTERFACE
void __sanitizer_cov_trace_pc_guard_init(uint32_t *Start, uint32_t *Stop) 
{ 
    LlvmRecordGuards(Start, Stop);
}

ATTRIBUTE_INTERFACE
void __sanitizer_cov_8bit_counters_init(uint8_t *Start, uint8_t *Stop) 
{
    //hw_printf("\r\n __sanitizer_cov_8bit_counters_init  %p  %p   \r\n", Start, Stop);
}

ATTRIBUTE_INTERFACE
void __sanitizer_cov_pcs_init(const uint8_t *pcs_beg, const uint8_t *pcs_end) 
{
    //hw_printf("\r\n __sanitizer_cov_pcs_init  %p  %p   \r\n", pcs_beg, pcs_end);
}

ATTRIBUTE_INTERFACE
ATT_NO_SANITIZE_ALL
void __sanitizer_cov_trace_pc_indirect(uintptr_t Callee) 
{ 
}

ATTRIBUTE_INTERFACE
ATT_NO_SANITIZE_ALL
void __sanitizer_cov_trace_pc_indir(uintptr_t Callee) 
{ 
}

#ifdef _WIN32__

#pragma section(".SCOV$A", read, write) // NOLINT
#pragma section(".SCOV$Z", read, write) // NOLINT
    
__declspec(allocate(".SCOV$A")) uint32_t __start___sancov_guards = 0;
__declspec(allocate(".SCOV$Z")) uint32_t __stop___sancov_guards = 0;
#endif

ATTRIBUTE_INTERFACE
ATT_NO_SANITIZE_ALL
void __sanitizer_cov_trace_pc_guard(uint32_t *Guard) 
{
    if (g_globalThead.tracepcModule.isEnableFeature == 0)
    {
        return;
    }

    uintptr_t pc = (uintptr_t)(__builtin_return_address(0));
#ifndef _WIN32
    int idx = *Guard;
#else
    int idx = (size_t)pc % (MAX_PC_NUM);
#endif

    LlvmDoTracePc(idx, pc);
}

// Best-effort support for -fsanitize-coverage=trace-pc, which is available
// in both Clang and GCC.
ATTRIBUTE_INTERFACE
ATT_NO_SANITIZE_ALL
void __sanitizer_cov_trace_pc() 
{
    if (g_globalThead.tracepcModule.isEnableFeature == 0)
    {
        return;
    }

    uintptr_t pc = (uintptr_t)(__builtin_return_address(0));
    int idx = (size_t)pc % (MAX_PC_NUM);

    LlvmDoTracePc(idx, pc);
}

#else
#endif


int LlvmTracePcIsHasNewFeature()
{
    int ret = g_globalThead.tracepcModule.isHasNewFeature;
    g_globalThead.tracepcModule.isHasNewFeature = 0;
    return ret;
}

void LlvmTracePcStartFeature(void)
{
    g_globalThead.tracepcModule.isHasNewFeature = 0;
    g_globalThead.tracepcModule.isEnableFeature = 1;
    g_globalThead.tracepcModule.weight = 0;
    g_globalThead.tracepcModule.pc2Sum = 0;
}

void LlvmTracePcEndFeature(void)
{
    g_globalThead.tracepcModule.isEnableFeature = 0;
}


void LlvmTracePcSetHasNewFeature(int weight)
{
    g_globalThead.tracepcModule.isHasNewFeature = 1;
    g_globalThead.tracepcModule.weight = weight;
}

int LlvmTracePcGetWeight(void)
{
    return g_globalThead.tracepcModule.weight;
}

#ifdef __cplusplus
}
#endif
