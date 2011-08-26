#ifndef QSYS_Q_HEAP_H
#define QSYS_Q_HEAP_H

#define HEAP_TRACK_DEBUG 0//置1会打开记录mallco申请者的功能，但会增加额外内存开销

void DebugHeap(void);
void QS_HeapInit(void);
void QS_MonitorFragment(void);
#if HEAP_TRACK_DEBUG == 1
void *QS_Mallco(u16 Size,u8 *pFuncName,u32 Lines);
bool QS_Free(void *Ptr,u8 *pFuncName,u32 Lines);
#else
void *QS_Mallco(u16 Size);
bool QS_Free(void *Ptr);
#endif


#endif

