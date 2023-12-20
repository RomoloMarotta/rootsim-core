/**
 * @file parallel/parallel.c
 *
 * @brief Concurrent simulation engine
 *
 * SPDX-FileCopyrightText: 2008-2022 HPDCS Group <rootsim@googlegroups.com>
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <parallel/parallel.h>

#include <arch/thread.h>
#include <core/core.h>
#include <core/sync.h>
#include <datatypes/msg_queue.h>
#include <distributed/mpi.h>
#include <gvt/fossil.h>
#include <log/stats.h>
#include <mm/msg_allocator.h>

static void worker_thread_init(rid_t this_rid, int where)
{
	rid = this_rid;
	stats_init();
	auto_ckpt_init();
	msg_allocator_init(where);
	msg_queue_init(where);
	sync_thread_barrier();
	lp_init(where);

	if(sync_thread_barrier()) {
		mpi_node_barrier();
		lp_initialized_set();
	}

	if(sync_thread_barrier()) {
		logger(LOG_INFO, "Starting simulation");
		stats_global_time_take(STATS_GLOBAL_EVENTS_START);
	}
}

static void worker_thread_fini(int where)
{
	gvt_msg_drain(where);

	if(sync_thread_barrier()) {
		stats_dump();
		stats_global_time_take(STATS_GLOBAL_EVENTS_END);
		logger(LOG_INFO, "Finalizing simulation");

		mpi_node_barrier();
	}

	lp_fini(where);
	msg_queue_fini(where);
	sync_thread_barrier();
	msg_allocator_fini(where);
}

static thrd_ret_t THREAD_CALL_CONV parallel_thread_run(void *rid_arg)
{

	where = DRAM_MEM;
	worker_thread_init((uintptr_t)rid_arg, where);

	while(likely(termination_cant_end())) {
		mpi_remote_msg_handle(where);

		unsigned i = 64;
		while(i--)
			process_msg(where);

		simtime_t current_gvt = gvt_phase_run();
		if(unlikely(current_gvt != 0.0)) {
			termination_on_gvt(current_gvt);
			auto_ckpt_on_gvt();
			fossil_on_gvt(current_gvt);
			msg_allocator_on_gvt(current_gvt, where);
			stats_on_gvt(current_gvt);
		}
	}

	worker_thread_fini(where);

	return THREAD_RET_SUCCESS;
}

static void parallel_global_init(memkind_const where)
{

	stats_global_init(where);
	lp_global_init(where);
	msg_queue_global_init(where); ///aligned
	termination_global_init();
	gvt_global_init();
}

static void parallel_global_fini(memkind_const where)
{
	msg_queue_global_fini(where); ///aligned
	lp_global_fini(where);
	stats_global_fini(where);
}

int parallel_simulation(void)
{
	logger(LOG_INFO, "Initializing parallel simulation");
	where = DRAM_MEM;
	parallel_global_init(where);
	stats_global_time_take(STATS_GLOBAL_INIT_END);

	thr_id_t thrs[global_config.n_threads];
	rid_t i = global_config.n_threads;
	while(i--) {
		if(thread_start(&thrs[i], parallel_thread_run, (void *)(uintptr_t)i)) {
			logger(LOG_FATAL, "Unable to create threads!");
			abort();
		}
		if(global_config.core_binding && thread_affinity_set(thrs[i], i)) {
			logger(LOG_FATAL, "Unable to set thread affinity!");
			abort();
		}
	}

	i = global_config.n_threads;
	while(i--)
		thread_wait(thrs[i], NULL);

	stats_global_time_take(STATS_GLOBAL_FINI_START);
	where = DRAM_MEM;
	parallel_global_fini(where);

	return 0;
}
