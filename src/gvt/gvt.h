/**
* @file gvt/gvt.h
*
* @brief Global Virtual Time
*
* This module implements the GVT reduction. The current implementation
* is non blocking for observable simulation plaftorms.
*
* @copyright
* Copyright (C) 2008-2019 HPDCS Group
* https://hpdcs.github.io
*
* This file is part of ROOT-Sim (ROme OpTimistic Simulator).
*
* ROOT-Sim is free software; you can redistribute it and/or modify it under the
* terms of the GNU General Public License as published by the Free Software
* Foundation; only version 3 of the License applies.
*
* ROOT-Sim is distributed in the hope that it will be useful, but WITHOUT ANY
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
* A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* ROOT-Sim; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*
* @author Alessandro Pellegrini
* @author Francesco Quaglia
*
* @date June 14, 2014
*/

#pragma once

#include <ROOT-Sim.h>
#include <mm/state.h>

<<<<<<< HEAD
/* API from gvt.c */
extern bool is_idle(void);
extern void update_GVT(void);
=======
#define CANCELBACK_COOLDOWN         2
#define CANCELBACK_MEM_THRESHOLD    0.25
#define RAND() ((double)rand() / (double)RAND_MAX)


/** API from gvt.c */
>>>>>>> origin/cancelback
extern void gvt_init(void);
extern void update_participants(void);
extern void gvt_fini(void);
extern simtime_t gvt_operations(void);
inline extern simtime_t get_last_gvt(void);

<<<<<<< HEAD
/* API from fossil.c */
extern void adopt_new_gvt(simtime_t);
=======
/** API from fossil.c */
extern void adopt_new_gvt(simtime_t, simtime_t);

<<<<<<< HEAD
/** API from cancelback.c */
extern bool attempt_cancelback(simtime_t);
extern bool is_memory_limit_exceeded();
extern inline void synch_for_cancelback();
extern void send_cancelback_messages(unsigned int);
extern bool is_cancelback_started();
extern int num_unprocessed_msgs(unsigned int);
extern inline size_t get_cancelback_threshold();
>>>>>>> origin/cancelback
=======
/* API from models.c */
extern void gvt_recompute_models(void);

>>>>>>> origin/reverse

/* API from ccgs.c */
extern void ccgs_init(void);
extern void ccgs_fini(void);
