#ifndef __WT_MEMPOOL_H__
#define __WT_MEMPOOL_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef void* MemoryPoolContext;

MemoryPoolContext wt_mempool_create(const char* mempool_name, unsigned int minContextSize, unsigned int initBlockSize, unsigned int maxBlockSize);

void wt_mempool_destroy(MemoryPoolContext cxt);

void wt_mempool_reset(MemoryPoolContext cxt);

void* wt_palloc(MemoryPoolContext cxt, size_t size);

void* wt_palloc0(MemoryPoolContext cxt, size_t size);

char* wt_pstrdup(MemoryPoolContext cxt, const char* in);

void wt_pfree(void* pointer);

#ifdef _DEBUG
size_t wt_mempool_size(MemoryPoolContext cxt);
#endif

#ifdef __cplusplus
}
#endif

#endif // __WT_MEMPOOL_H__