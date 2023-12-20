/**
 * @file datatypes/msg_queue.h
 *
 * @brief Message queue datatype
 *
 * Message queue datatype
 *
 * SPDX-FileCopyrightText: 2008-2022 HPDCS Group <rootsim@googlegroups.com>
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <core/core.h>
#include <lp/msg.h>

extern void msg_queue_global_init(memkind_const);
extern void msg_queue_global_fini(memkind_const);
extern void msg_queue_init(memkind_const);
extern void msg_queue_fini(memkind_const);
extern struct lp_msg *msg_queue_extract(void);
extern simtime_t msg_queue_time_peek(void);
extern void msg_queue_insert(struct lp_msg *msg);
