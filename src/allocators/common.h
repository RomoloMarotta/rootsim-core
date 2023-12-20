#ifndef __COMMON_ALLOCATORS__
#define __COMMON_ALLOCATORS__

#include <mm_nvram.h>


#define WRAP_TO_STD 0
#define WRAP_TO_REA 1
#define WRAP_TO_MKD 2

#ifndef MEMKIND_EVENTS
#define MEMKIND_EVENTS DRAM_MEM
#endif

#ifndef MEMKIND_OTHERS
#define MEMKIND_OTHERS DRAM_MEM
#endif

#ifndef MEMKIND_THREAD
#define MEMKIND_THREAD MEMKIND_OTHERS
#endif

#ifndef MEMKIND_LPSTAT
#define MEMKIND_LPSTAT DRAM_MEM
#endif

#ifndef MEMKIND_LPCKPT
#define MEMKIND_LPCKPT DRAM_MEM
#endif

#ifndef MEMKIND_LPMETA
#define MEMKIND_LPMETA DRAM_MEM
#endif




#endif