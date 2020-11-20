/**
* @file mm/buddy/buddy.c
*
* @brief A Buddy System implementation
*
* @copyright
* Copyright (C) 2008-2020 HPDCS Group
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
*/
#include <mm/buddy/buddy.h>

#include <core/core.h>
#include <lp/lp.h>
#include <core/intrinsics.h>

#include <stdlib.h>
#include <errno.h>

#define left_child(i) (((i) << 1U) + 1U)
#define right_child(i) (((i) << 1U) + 2U)
#define parent(i) ((((i) + 1) >> 1U) - 1U)
#define is_power_of_2(i) (!((i) & ((i) - 1)))
#define next_exp_of_2(i) (sizeof(i) * CHAR_BIT - SAFE_CLZ(i))

#ifdef ROOTSIM_COVERAGE
extern void *__real_malloc(size_t mem_size);
extern void *__real_realloc(void *ptr, size_t mem_size);
extern void __real_free(void *ptr);
#endif

void model_allocator_lp_init(void)
{
	struct mm_state *self = &current_lp->mm_state;
	uint_fast8_t node_size = B_TOTAL_EXP;

	for (uint_fast32_t i = 0;
		i < sizeof(self->longest) / sizeof(*self->longest); ++i) {
		self->longest[i] = node_size;
		node_size -= is_power_of_2(i + 2);
	}

	self->used_mem = 0;
	array_init(self->logs);
#ifdef ROOTSIM_INCREMENTAL
	memset(self->dirty, 0, sizeof(self->dirty));
	self->dirty_mem = 0;
#endif
}

void model_allocator_lp_fini(void)
{
	array_count_t i = array_count(current_lp->mm_state.logs);
	while(i--){
		mm_free(array_get_at(current_lp->mm_state.logs, i).c);
	}
	array_fini(current_lp->mm_state.logs);
}

void *__wrap_malloc(size_t req_size)
{
#ifdef ROOTSIM_COVERAGE
	if (unlikely(!current_lp))
		return __real_malloc(req_size);
#endif

	if(unlikely(!req_size))
		return NULL;

	struct mm_state *self = &current_lp->mm_state;

	uint_fast8_t req_blks = max(next_exp_of_2(req_size - 1), B_BLOCK_EXP);

	if (unlikely(self->longest[0] < req_blks)) {
		errno = ENOMEM;
		return NULL;
	}

	/* search recursively for the child */
	uint_fast8_t node_size = B_TOTAL_EXP;
	uint_fast32_t i = 0;
	while(node_size > req_blks) {
		/* choose the child with smaller longest value which
		 * is still large at least *size* */
		i = left_child(i);
		i += self->longest[i] < req_blks;
		--node_size;
	}

	/* update the *longest* value back */
	self->longest[i] = 0;
	self->used_mem += 1 << node_size;
#ifdef ROOTSIM_INCREMENTAL
	bitmap_set(self->dirty, i >> B_BLOCK_EXP);
#endif
	uint_fast32_t offset = ((i + 1) << node_size) - (1 << B_TOTAL_EXP);

	while (i) {
		i = parent(i);
		self->longest[i] = max(
			self->longest[left_child(i)],
			self->longest[right_child(i)]
		);
#ifdef ROOTSIM_INCREMENTAL
		bitmap_set(self->dirty, i >> B_BLOCK_EXP);
#endif
	}

	return ((char *)self->base_mem) + offset;
}

void *__wrap_calloc(size_t nmemb, size_t size)
{
	size_t tot = nmemb * size;
	void *ret = __wrap_malloc(tot);

	if(ret)
		memset(ret, 0, tot);

	return ret;
}

void __wrap_free(void *ptr)
{
#ifdef ROOTSIM_COVERAGE
	if (unlikely(!current_lp)) {
		__real_free(ptr);
		return;
	}
#endif

	if(unlikely(!ptr))
		return;

	struct mm_state *self = &current_lp->mm_state;
	uint_fast8_t node_size = B_BLOCK_EXP;
	uint_fast32_t i =
		(((uintptr_t)ptr - (uintptr_t)self->base_mem) >> B_BLOCK_EXP) +
		(1 << (B_TOTAL_EXP - B_BLOCK_EXP)) - 1;

	for (; self->longest[i]; i = parent(i)) {
		++node_size;
	}

	self->longest[i] = node_size;
	self->used_mem -= 1 << node_size;
#ifdef ROOTSIM_INCREMENTAL
	bitmap_set(self->dirty, i >> B_BLOCK_EXP);
#endif
	while (i) {
		i = parent(i);

		uint_fast8_t left_longest = self->longest[left_child(i)];
		uint_fast8_t right_longest = self->longest[right_child(i)];

		if (left_longest == node_size && right_longest == node_size) {
			self->longest[i] = node_size + 1;
		} else {
			self->longest[i] = max(left_longest, right_longest);
		}
#ifdef ROOTSIM_INCREMENTAL
		bitmap_set(self->dirty, i >> B_BLOCK_EXP);
#endif
		++node_size;
	}
}

void *__wrap_realloc(void *ptr, size_t req_size)
{
#ifdef ROOTSIM_COVERAGE
	if (unlikely(!current_lp))
		return __real_realloc(ptr, req_size);
#endif

	if (!req_size) {
		__wrap_free(ptr);
		return NULL;
	}
	if (!ptr) {
		return __wrap_malloc(req_size);
	}

	abort();
	return NULL;
}

#define buddy_tree_visit(longest, on_visit)				\
__extension__({								\
	bool __vis = false;						\
	uint_fast8_t __l = B_TOTAL_EXP;					\
	uint_fast32_t __i = 0;						\
	while(1) {							\
		uint_fast8_t __lon = longest[__i];			\
		if(!__lon) {						\
			uint_fast32_t __len = 1U << __l;		\
			uint_fast32_t __o = 				\
				((__i + 1) << __l) - (1 << B_TOTAL_EXP);\
			on_visit(__o, __len);				\
		} else if(__lon != __l) {				\
			__i = left_child(__i) + __vis;			\
			__vis = false;					\
			__l--;						\
			continue;					\
		}							\
		do {							\
			__vis = !(__i & 1U);				\
			__i = parent(__i);				\
			__l++;						\
		} while(__vis);						\
									\
		if (__l > B_TOTAL_EXP) break;				\
		__vis = true;						\
	}								\
})

#ifdef ROOTSIM_INCREMENTAL

void __write_mem(void *ptr, size_t siz)
{
	struct mm_state *self = &current_lp->mm_state;
	uintptr_t diff = (uintptr_t)ptr - (uintptr_t)self->base_mem;
	if(diff >= (1 << B_TOTAL_EXP))
		return;

	uint_fast32_t i = (diff >> B_BLOCK_EXP) +
		(1 << (B_TOTAL_EXP - 2 * B_BLOCK_EXP + 1));

	siz += diff & ((1 << B_BLOCK_EXP) - 1);
	--siz;
	siz >>= B_BLOCK_EXP;
	self->dirty_mem += siz;
	do {
		bitmap_set(self->dirty, i + siz);
	} while(siz--);
}

static struct _mm_checkpoint *checkpoint_incremental_take(struct mm_state *self)
{
	uint_fast32_t bset = bitmap_count_set(self->dirty, sizeof(self->dirty));

	struct mm_checkpoint *ret = mm_alloc(
		offsetof(struct _mm_checkpoint, longest) +
		bset * (1 << B_BLOCK_EXP));

	unsigned char *ptr = ret->longest;
	const unsigned char *src = self->longest;

#define copy_block_to_ckp(i) 						\
__extension__({								\
	memcpy(ptr, src + (i << B_BLOCK_EXP), 1 << B_BLOCK_EXP);	\
	ptr += 1 << B_BLOCK_EXP;					\
})

	bitmap_foreach_set(self->dirty, sizeof(self->dirty), copy_block_to_ckp);
#undef copy_block_to_ckp

	ret->used_mem = self->used_mem;
	memcpy(ret->dirty, self->dirty, sizeof(self->dirty));
	ret->is_incremental = true;
	return ret;
}

static void checkpoint_incremental_restore(struct mm_state *self,
	const struct mm_checkpoint *ckp)
{
	self->used_mem = ckp->used_mem;

	array_count_t i = array_count(self->logs) - 1;
	const struct mm_checkpoint *cur_ckp = array_get_at(self->logs, i).c;

	while (cur_ckp != ckp) {
		bitmap_merge_or(self->dirty, cur_ckp->dirty,
			sizeof(self->dirty));
		cur_ckp = array_get_at(self->logs, --i).c;
	}

#define copy_dirty_block(i) 						\
__extension__({								\
	if (bitmap_check(self->dirty, i)) {				\
		memcpy(self->longest + (i << B_BLOCK_EXP), ptr,		\
		       1 << B_BLOCK_EXP);				\
		bitmap_reset(self->dirty, i);				\
		bset--;							\
	}								\
	ptr += 1 << B_BLOCK_EXP;					\
})

	uint_fast32_t bset = bitmap_count_set(self->dirty, sizeof(self->dirty));
	do {
		const unsigned char *ptr = cur_ckp->longest;
		bitmap_foreach_set(cur_ckp->dirty, sizeof(cur_ckp->dirty),
			copy_dirty_block);
		cur_ckp = array_get_at(self->logs, --i).c;
	} while (bset && cur_ckp->is_incremental);

#undef copy_dirty_block

	if (!bset)
		return;

#define copy_block_from_ckp(i) 						\
__extension__({								\
	memcpy(self->longest + (i << B_BLOCK_EXP), cur_ckp->longest + 	\
		(i << B_BLOCK_EXP), 1 << B_BLOCK_EXP);			\
})

	const unsigned tree_bit_size =
		bitmap_required_size(1 << (B_TOTAL_EXP - 2 * B_BLOCK_EXP + 1));
	bitmap_foreach_set(self->dirty, tree_bit_size, copy_block_from_ckp);
#undef copy_block_from_ckp

#define buddy_block_dirty_from_ckp(offset, len)				\
__extension__({								\
	uint_fast32_t b_off = (offset >> B_BLOCK_EXP) + 		\
		(1 << (B_TOTAL_EXP - 2 * B_BLOCK_EXP + 1));		\
	uint_fast32_t b_len = len >> B_BLOCK_EXP;			\
	while (b_len--) {						\
		if (bitmap_check(self->dirty, b_off)) {			\
			memcpy(self->longest + (b_off << B_BLOCK_EXP), 	\
			       ptr, (1 << B_BLOCK_EXP));		\
		}							\
		ptr += 1 << B_BLOCK_EXP;				\
		b_off++;						\
	}								\
})

	const unsigned char *ptr = cur_ckp->base_mem;
	buddy_tree_visit(self->longest, buddy_block_dirty_from_ckp);
#undef buddy_block_dirty_from_ckp
}

#endif

static struct mm_checkpoint *checkpoint_full_take(const struct mm_state *self)
{
	struct mm_checkpoint *ret = mm_alloc(
		offsetof(struct mm_checkpoint, base_mem) + self->used_mem);

#ifdef ROOTSIM_INCREMENTAL
	ret->is_incremental = false;
	memcpy(ret->dirty, self->dirty, sizeof(self->dirty));
#endif

	ret->used_mem = self->used_mem;
	memcpy(ret->longest, self->longest, sizeof(ret->longest));

#define buddy_block_copy_to_ckp(offset, len)				\
__extension__({								\
	memcpy(ptr, self->base_mem + offset, len);			\
	ptr += len;							\
})

	unsigned char *ptr = ret->base_mem;
	buddy_tree_visit(self->longest, buddy_block_copy_to_ckp);

#undef buddy_block_copy_to_ckp
	return ret;
}

static void checkpoint_full_restore(struct mm_state *self,
	const struct mm_checkpoint *ckp)
{
	self->used_mem = ckp->used_mem;
	memcpy(self->longest, ckp->longest, sizeof(self->longest));

#define buddy_block_copy_from_ckp(offset, len)				\
__extension__({								\
	memcpy(self->base_mem + offset, ptr, len);			\
	ptr += len;							\
})

	const unsigned char *ptr = ckp->base_mem;
	buddy_tree_visit(self->longest, buddy_block_copy_from_ckp);

#undef buddy_block_copy_from_ckp
}

void model_allocator_checkpoint_take(array_count_t ref_i)
{
	if(ref_i % B_LOG_FREQUENCY)
		return;

	struct mm_state *self = &current_lp->mm_state;
	struct mm_checkpoint *ckp;

#ifdef ROOTSIM_INCREMENTAL
	if (self->dirty_mem < self->used_mem * B_LOG_INCREMENTAL_THRESHOLD) {
		ckp = checkpoint_incremental_take(self);
	} else {
#endif
		ckp = checkpoint_full_take(self);
#ifdef ROOTSIM_INCREMENTAL
	}
	self->dirty_mem = 0;
	memset(self->dirty, 0, sizeof(self->dirty));
#endif

	struct mm_log mm_log = {
		.ref_i = ref_i,
		.c = ckp
	};
	array_push(self->logs, mm_log);
}

void model_allocator_checkpoint_next_force_full(void)
{
#ifdef ROOTSIM_INCREMENTAL
	struct mm_state *self = &current_lp->mm_state;
	self->dirty_mem = self->used_mem * B_LOG_INCREMENTAL_THRESHOLD;
#endif
}

array_count_t model_allocator_checkpoint_restore(array_count_t ref_i)
{
	struct mm_state *self = &current_lp->mm_state;

	array_count_t i = array_count(self->logs) - 1;
	while (array_get_at(self->logs, i).ref_i > ref_i) {
		i--;
	}

	const struct mm_checkpoint *ckp = array_get_at(self->logs, i).c;
#ifdef ROOTSIM_INCREMENTAL
	if (ckp->is_incremental) {
		checkpoint_incremental_restore(self, ckp);
	} else {
#endif
		checkpoint_full_restore(self, ckp);
#ifdef ROOTSIM_INCREMENTAL
	}
	memset(self->dirty, 0, sizeof(self->dirty));
	self->dirty_mem = 0;
#endif

	for(array_count_t j = array_count(self->logs) - 1; j > i; --j) {
		mm_free(array_get_at(self->logs, j).c);
	}
	array_count(self->logs) = i + 1;

	return array_get_at(self->logs, i).ref_i;
}

array_count_t model_allocator_fossil_lp_collect(array_count_t tgt_ref_i)
{
	struct mm_state *self = &current_lp->mm_state;
	array_count_t log_i = array_count(self->logs) - 1;
	array_count_t ref_i = array_get_at(self->logs, log_i).ref_i;
	while (ref_i > tgt_ref_i) {
		--log_i;
		ref_i = array_get_at(self->logs, log_i).ref_i;
	}

#ifdef ROOTSIM_INCREMENTAL
	while (array_get_at(self->logs, log_i).c->is_incremental) {
		--log_i;
		ref_i = array_get_at(self->logs, log_i).ref_i;
	}
#endif

	array_count_t j = array_count(self->logs);
	while (j > log_i) {
		--j;
		array_get_at(self->logs, j).ref_i -= ref_i;
	}
	while (j--) {
		mm_free(array_get_at(self->logs, j).c);
	}

	array_truncate_first(self->logs, log_i);
	return ref_i;
}
