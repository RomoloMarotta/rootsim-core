#ifndef IME_INTEL_EVENT
#define IME_INTEL_EVENT

#define EVENT(e, u) ((u << 8) | e)

/* Architectural events */
#define EVT_INSTRUCTIONS_RETIRED 			EVENT(0xC0, 0x00)

#define EVT_UNHALTED_CORE_CYCLES 			EVENT(0x3C, 0x00)
#define EVT_UNHALTED_REFERENCE_CYCLES 		EVENT(0x3C, 0x10)

#define EVT_BR_INST_RETIRED_ALL_BRANCHES	EVENT(0xC4, 0x00)

#define EVT_MEM_INST_RETIRED_ALL_LOADS		EVENT(0xD0, 0x81)
#define EVT_MEM_INST_RETIRED_ALL_STORES		EVENT(0xD0, 0x82)

#define EVT_MEM_LOAD_RETIRED_L3_HIT			EVENT(0xD1, 0x20)

#define MAX_NUM_EVENT 7

//PEBS EVENT

#define INST_RETIRED_TOTAL_CYCLES           EVENT(0xC0, 0x01)

#define BR_INST_RETIRED_CONDITIONAL         EVENT(0xC4, 0x01)
#define BR_INST_RETIRED_NEAR_CALL           EVENT(0xC4, 0x02)
#define BR_INST_RETIRED_ALL_BRANCHES        EVENT(0xC4, 0x04)
#define BR_INST_RETIRED_NEAR_RETURN         EVENT(0xC4, 0x08)
#define BR_INST_RETIRED_NEAR_TAKEN          EVENT(0xC4, 0x20)
#define BR_INST_RETIRED_FAR_BRANCH          EVENT(0xC4, 0x40)
#define BR_MISP_RETIRED_CONDITIONAL         EVENT(0xC5, 0x01)
#define BR_MISP_RETIRED_ALL_BRANCHES        EVENT(0xC5, 0x04)
#define BR_MISP_RETIRED_NEAR_TAKEN          EVENT(0xC5, 0x20) 

#define FRONTEND_RETIRED_DSB_MISS           EVENT(0xC6, 0x01) //MSR_PEBS_FRONTEND.EVTSEL=11H.
#define FRONTEND_RETIRED_L1I_MISS           EVENT(0xC6, 0x01) //MSR_PEBS_FRONTEND.EVTSEL=12H.
#define FRONTEND_RETIRED_L2_MISS            EVENT(0xC6, 0x01) //MSR_PEBS_FRONTEND.EVTSEL=13H.
#define FRONTEND_RETIRED_ITLB_MISS          EVENT(0xC6, 0x01) //MSR_PEBS_FRONTEND.EVTSEL=14H.
#define FRONTEND_RETIRED_STLB_MISS          EVENT(0xC6, 0x01) //MSR_PEBS_FRONTEND.EVTSEL=15H.
#define FRONTEND_RETIRED_LATENCY_GE_16      EVENT(0xC6, 0x01) //MSR_PEBS_FRONTEND.EVTSEL=16H.

#define MEM_TRANS_RETIRED_LOAD_LATENCY      EVENT(0xCD, 0x01) 
#define MEM_INST_RETIRED_STLB_MISS          EVENT(0xD0, 0x11) 
#define MEM_INST_RETIRED_STLB_MISS          EVENT(0xD0, 0x12) 
#define MEM_INST_RETIRED_LOCK_LOADS         EVENT(0xD0, 0x21) 
#define MEM_INST_RETIRED_SPLIT_LOADS        EVENT(0xD0, 0x41) 
#define MEM_INST_RETIRED_SPLIT_STORES       EVENT(0xD0, 0x42) 
#define MEM_INST_RETIRED_ALL_LOAS           EVENT(0xD0, 0x81) 
#define MEM_INST_RETIRED_ALL_STORES         EVENT(0xD1, 0x82) 

#define MEM_LOAD_RETIRED_L1_HIT             EVENT(0xD1, 0x01) 
#define MEM_LOAD_RETIRED_L2_HIT             EVENT(0xD1, 0x02) 
#define MEM_LOAD_RETIRED_L3_HIT             EVENT(0xD1, 0x04) 
#define MEM_LOAD_RETIRED_L1_MISS            EVENT(0xD1, 0x08) 
#define MEM_LOAD_RETIRED_L2_MISS            EVENT(0xD1, 0x10) 
#define MEM_LOAD_RETIRED_L3_MISS            EVENT(0xD1, 0x20) 
#define MEM_LOAD_RETIRED_FB_HIT             EVENT(0xD2, 0x40) 

#define MEM_LOAD_L3_HIT_RETIRED_XSNP_MISS   EVENT(0xD2, 0x01) 
#define MEM_LOAD_L3_HIT_RETIRED_XSNP_HIT    EVENT(0xD2, 0x02) 
#define MEM_LOAD_L3_HIT_RETIRED_XSNP_HITM   EVENT(0xD2, 0x04) 
#define MEM_LOAD_L3_HIT_RETIRED_XSNP_NONE   EVENT(0xD2, 0x08)



// 04H 01H MEM_UOPS_RETIRED.L1_MISS_LOADS 
// 04H 02H MEM_UOPS_RETIRED.L2_HIT_LOADS 
// 04H 04H MEM_UOPS_RETIRED.L2_MISS_LOADS 
// 04H 08H MEM_UOPS_RETIRED.DTLB_MISS_LOADS 
// 04H 10H MEM_UOPS_RETIRED.UTLB_MISS 
// 04H 20H MEM_UOPS_RETIRED.HITM 
// 04H 40H MEM_UOPS_RETIRED.ALL_LOADS 
// 04H 80H MEM_UOP_RETIRED.ALL_STORES 

#endif