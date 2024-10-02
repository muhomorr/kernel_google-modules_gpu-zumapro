/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 *
 * (C) COPYRIGHT 2019-2024 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 */

#if !defined(_KBASE_TIMELINE_PRIV_H)
#define _KBASE_TIMELINE_PRIV_H

#include <mali_kbase.h>
#include "mali_kbase_tlstream.h"

#if MALI_USE_CSF
#include "csf/mali_kbase_csf_tl_reader.h"
#include "csf/mali_kbase_csf_trace_buffer.h"
#endif

#include <linux/timer.h>
#include <linux/atomic.h>
#include <linux/mutex.h>

/* The minimum amount of time timeline must be acquired for before release is
 * allowed, to prevent DoS attacks.
 */
#define TIMELINE_HYSTERESIS_TIMEOUT_MS ((s64)500)

/**
 * struct kbase_timeline - timeline state structure
 * @streams:                The timeline streams generated by kernel
 * @streams_buf_lock:       Lock to allocate and free streams.
 * @tl_kctx_list:           List of contexts for timeline.
 * @tl_kctx_list_lock:      Lock to protect @tl_kctx_list.
 * @autoflush_timer:        Autoflush timer
 * @autoflush_timer_active: If non-zero autoflush timer is active
 * @reader_lock:            Reader lock. Only one reader is allowed to
 *                          have access to the timeline streams at any given time.
 * @event_queue:            Timeline stream event queue
 * @bytes_collected:        Number of bytes read by user
 * @timeline_flags:         Zero, if timeline is disabled. Timeline stream flags
 *                          otherwise. See kbase_timeline_acquire().
 * @obj_header_btc:         Remaining bytes to copy for the object stream header
 * @aux_header_btc:         Remaining bytes to copy for the aux stream header
 * @last_acquire_time:      The time at which timeline was last acquired.
 * @csf_tl_reader:          CSFFW timeline reader
 */
struct kbase_timeline {
	struct kbase_tlstream streams[TL_STREAM_TYPE_COUNT];
	struct mutex streams_buf_lock;
	struct list_head tl_kctx_list;
	struct mutex tl_kctx_list_lock;
	struct timer_list autoflush_timer;
	atomic_t autoflush_timer_active;
	struct mutex reader_lock;
	wait_queue_head_t event_queue;
#if MALI_UNIT_TEST
	atomic_t bytes_collected;
#endif /* MALI_UNIT_TEST */
	atomic_t *timeline_flags;
	size_t obj_header_btc;
	size_t aux_header_btc;
	ktime_t last_acquire_time;
#if MALI_USE_CSF
	struct kbase_csf_tl_reader csf_tl_reader;
#endif
};

void kbase_create_timeline_objects(struct kbase_device *kbdev);

/**
 * kbase_timeline_acquire - acquire timeline for a userspace client.
 * @kbdev:     An instance of the GPU platform device, allocated from the probe
 *             method of the driver.
 * @flags:     Timeline stream flags
 *
 * Each timeline instance can be acquired by only one userspace client at a time.
 *
 * Return: Zero on success, error number on failure (e.g. if already acquired).
 */
int kbase_timeline_acquire(struct kbase_device *kbdev, u32 flags);

/**
 * kbase_timeline_release - release timeline for a userspace client.
 * @timeline:     Timeline instance to be stopped. It must be previously acquired
 *                with kbase_timeline_acquire().
 *
 * Releasing the timeline instance allows it to be acquired by another userspace client.
 */
void kbase_timeline_release(struct kbase_timeline *timeline);

#endif /* _KBASE_TIMELINE_PRIV_H */
