#include "recomp.h"

#ifdef __cplusplus
extern "C" {
#endif

void recomp_entrypoint(uint8_t* rdram, recomp_context* ctx);
void bootproc(uint8_t* rdram, recomp_context* ctx);
void main_switch_handler(uint8_t* rdram, recomp_context* ctx);
void osAiSetNextBuffer_recomp(uint8_t* rdram, recomp_context* ctx);
void osCreatePiManager_recomp(uint8_t* rdram, recomp_context* ctx);
void _VirtualToPhysicalTask(uint8_t* rdram, recomp_context* ctx);
void osSpTaskLoad_recomp(uint8_t* rdram, recomp_context* ctx);
void osSpTaskStartGo_recomp(uint8_t* rdram, recomp_context* ctx);
void osCreateViManager_recomp(uint8_t* rdram, recomp_context* ctx);
void osCreateThread_recomp(uint8_t* rdram, recomp_context* ctx);
void allocate_memory(uint8_t* rdram, recomp_context* ctx);
void osMotorStop_recomp(uint8_t* rdram, recomp_context* ctx);
void osMotorStart_recomp(uint8_t* rdram, recomp_context* ctx);
void osMotorInit_recomp(uint8_t* rdram, recomp_context* ctx);
void alHeapInit(uint8_t* rdram, recomp_context* ctx);
void alHeapDBAlloc(uint8_t* rdram, recomp_context* ctx);
void alSeqFileNew(uint8_t* rdram, recomp_context* ctx);
