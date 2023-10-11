/**
 * @file mm/buddy/buddy.h
 *
 * @brief A Buddy System implementation
 *
 * SPDX-FileCopyrightText: 2008-2023 HPDCS Group <rootsim@googlegroups.com>
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <datatypes/bitmap.h>
#include <mm/distributed_mem.h>

#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define B_TOTAL_EXP DISTR_MEM_CHUNK_EXP
#define B_BLOCK_EXP 6U

#define next_exp_of_2(i) (sizeof(i) * CHAR_BIT - intrinsics_clz(i))
#define buddy_allocation_block_compute(req_size) next_exp_of_2(max(req_size, 1U << B_BLOCK_EXP) - 1);

/// The checkpointable memory context of a single buddy system
struct buddy_state {
	/// The next struct buddy_state in the list
	struct buddy_state *next;
	/// The memory buffer served to the model
	struct distr_mem_chunk *chunk;
	/// The checkpointed binary tree representing the buddy system
	/** the last char is actually unused */
	uint8_t longest[(1U << (B_TOTAL_EXP - B_BLOCK_EXP))];
	/*/// Keeps track of memory blocks which have been dirtied by a write
	block_bitmap dirty[
		bitmap_required_size(
		// this tracks writes to the allocation tree
			(1 << (B_TOTAL_EXP - 2 * B_BLOCK_EXP + 1)) +
		// while this tracks writes to the actual memory buffer
			(1 << (B_TOTAL_EXP - B_BLOCK_EXP))
		)
	];*/
};

extern void buddy_init(struct buddy_state *self);
extern void buddy_fini(struct buddy_state *self);
extern void buddy_moved(struct buddy_state *self);
extern void *buddy_malloc(struct buddy_state *self, uint_fast8_t req_blks_exp);
extern uint_fast32_t buddy_free(void *ptr);

struct buddy_realloc_res {
	bool handled;
	union {
		int_fast32_t variation;
		uint_fast32_t original;
	};
};
extern struct buddy_realloc_res buddy_best_effort_realloc(void *ptr, size_t req_size);
extern void buddy_dirty_mark(const void *ptr, size_t s);
