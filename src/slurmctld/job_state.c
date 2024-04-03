/*****************************************************************************\
 *  job_state.c
 *****************************************************************************
 *  Copyright (C) SchedMD LLC.
 *  Written by Nathan Rini <nate@schedmd.com>
 *
 *  This file is part of Slurm, a resource management program.
 *  For details, see <https://slurm.schedmd.com/>.
 *  Please also read the included file: DISCLAIMER.
 *
 *  Slurm is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  In addition, as a special exception, the copyright holders give permission
 *  to link the code of portions of this program with the OpenSSL library under
 *  certain conditions as described in each individual source file, and
 *  distribute linked combinations including the two. You must obey the GNU
 *  General Public License in all respects for all of the code used other than
 *  OpenSSL. If you modify file(s) with this exception, you may extend this
 *  exception to your version of the file(s), but you are not obligated to do
 *  so. If you do not wish to do so, delete this exception statement from your
 *  version.  If you delete this exception statement from all source files in
 *  the program, then also delete it here.
 *
 *  Slurm is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with Slurm; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
\*****************************************************************************/

#include "src/common/macros.h"

#include "src/slurmctld/locks.h"
#include "src/slurmctld/slurmctld.h"

#define MAGIC_JOB_STATE_ARGS 0x0a0beeee

typedef struct {
	int magic; /* MAGIC_JOB_STATE_ARGS */
	int rc;
	uint32_t count;
	job_state_response_job_t *jobs;
	bool count_only;
} job_state_args_t;

typedef struct {
	job_record_t *job_ptr;
	job_state_args_t *job_state_args;
} foreach_het_job_state_args_t;

#ifndef NDEBUG

#define T(x) { x, XSTRINGIFY(x) }
static const struct {
	uint32_t flag;
	char *string;
} job_flags[] = {
	T(JOB_LAUNCH_FAILED),
	T(JOB_UPDATE_DB),
	T(JOB_REQUEUE),
	T(JOB_REQUEUE_HOLD),
	T(JOB_SPECIAL_EXIT),
	T(JOB_RESIZING),
	T(JOB_CONFIGURING),
	T(JOB_COMPLETING),
	T(JOB_STOPPED),
	T(JOB_RECONFIG_FAIL),
	T(JOB_POWER_UP_NODE),
	T(JOB_REVOKED),
	T(JOB_REQUEUE_FED),
	T(JOB_RESV_DEL_HOLD),
	T(JOB_SIGNALING),
	T(JOB_STAGE_OUT),
};

static void _check_job_state(const uint32_t state)
{
	uint32_t flags;

	if (!(slurm_conf.debug_flags & DEBUG_FLAG_TRACE_JOBS))
		return;

	flags = (state & JOB_STATE_FLAGS);

	xassert((state & JOB_STATE_BASE) < JOB_END);

	for (int i = 0; i < ARRAY_SIZE(job_flags); i++)
		if ((flags & job_flags[i].flag) == job_flags[i].flag)
			flags &= ~(job_flags[i].flag);

	/* catch any bits that are not known flags */
	xassert(!flags);
}

static void _log_job_state_change(const job_record_t *job_ptr,
				  const uint32_t new_state)
{
	char *before_str, *after_str;

	if (!(slurm_conf.debug_flags & DEBUG_FLAG_TRACE_JOBS))
		return;

	before_str = job_state_string_complete(job_ptr->job_state);
	after_str = job_state_string_complete(new_state);

	if (job_ptr->job_state == new_state)
		log_flag(TRACE_JOBS, "%s: [%pJ] no-op change state: %s",
			 __func__, job_ptr, before_str);
	else
		log_flag(TRACE_JOBS, "%s: [%pJ] change state: %s -> %s",
			 __func__, job_ptr, before_str, after_str);

	xfree(before_str);
	xfree(after_str);
}

#else /* NDEBUG */

#define _check_job_state(state) {}
#define _log_job_state_change(job_ptr, new_state) {}

#endif /* NDEBUG */

extern void job_state_set(job_record_t *job_ptr, uint32_t state)
{
	_check_job_state(state);
	_log_job_state_change(job_ptr, state);

	job_ptr->job_state = state;
}

extern void job_state_set_flag(job_record_t *job_ptr, uint32_t flag)
{
	uint32_t job_state;

	xassert(!(flag & JOB_STATE_BASE));
	xassert(flag & JOB_STATE_FLAGS);

	job_state = job_ptr->job_state | flag;
	_check_job_state(job_state);
	_log_job_state_change(job_ptr, job_state);

	job_ptr->job_state = job_state;
}

extern void job_state_unset_flag(job_record_t *job_ptr, uint32_t flag)
{
	uint32_t job_state;

	xassert(!(flag & JOB_STATE_BASE));
	xassert(flag & JOB_STATE_FLAGS);

	job_state = job_ptr->job_state & ~flag;
	_check_job_state(job_state);
	_log_job_state_change(job_ptr, job_state);

	job_ptr->job_state = job_state;
}

static job_state_response_job_t *_append_job_state(job_state_args_t *args)
{
	int index;
	job_state_response_job_t *rjob;

	xassert(args->magic == MAGIC_JOB_STATE_ARGS);

	args->count++;

	if (args->count_only)
		return NULL;

	index = args->count - 1;
	rjob = &args->jobs[index];
	xassert(!rjob->job_id);
	return rjob;
}

static bitstr_t *_job_state_array_bitmap(const job_record_t *job_ptr)
{
	if (!job_ptr->array_recs)
		return NULL;

	if (job_ptr->array_recs->task_id_bitmap &&
	    (bit_ffs(job_ptr->array_recs->task_id_bitmap) != -1))
		return bit_copy(job_ptr->array_recs->task_id_bitmap);

	return NULL;
}

static foreach_job_by_id_control_t _foreach_job(const job_record_t *job_ptr,
						void *arg)
{
	job_state_args_t *args = arg;
	job_state_response_job_t *rjob;

	xassert(args->magic == MAGIC_JOB_STATE_ARGS);

	rjob = _append_job_state(args);

	if (args->count_only)
		return FOR_EACH_JOB_BY_ID_EACH_CONT;

	if (!rjob)
		return FOR_EACH_JOB_BY_ID_EACH_FAIL;

	rjob->job_id = job_ptr->job_id;
	rjob->array_job_id = job_ptr->array_job_id;
	rjob->array_task_id = job_ptr->array_task_id;
	rjob->array_task_id_bitmap = _job_state_array_bitmap(job_ptr);
	rjob->het_job_id = job_ptr->het_job_id;
	rjob->state = job_ptr->job_state;
	return FOR_EACH_JOB_BY_ID_EACH_CONT;
}

static void _dump_job_state_locked(job_state_args_t *args,
				   const uint32_t filter_jobs_count,
				   const slurm_selected_step_t *filter_jobs_ptr)
{

	xassert(verify_lock(JOB_LOCK, READ_LOCK));
	xassert(args->magic == MAGIC_JOB_STATE_ARGS);

	if (!filter_jobs_count) {
		slurm_selected_step_t filter = SLURM_SELECTED_STEP_INITIALIZER;
		(void) foreach_job_by_id_ro(&filter, _foreach_job, args);
	} else {
		for (int i = 0; !args->rc && (i < filter_jobs_count); i++) {
			(void) foreach_job_by_id_ro(&filter_jobs_ptr[i],
						    _foreach_job, args);
		}
	}
}

extern int dump_job_state(const uint32_t filter_jobs_count,
			  const slurm_selected_step_t *filter_jobs_ptr,
			  uint32_t *jobs_count_ptr,
			  job_state_response_job_t **jobs_pptr)
{
	job_state_args_t args = {
		.magic = MAGIC_JOB_STATE_ARGS,
		.count_only = true,
	};

	/*
	 * Loop once to grab the job count and then allocate the job array and
	 * then populate the array.
	 */

	_dump_job_state_locked(&args, filter_jobs_count, filter_jobs_ptr);

	if (!try_xrecalloc(args.jobs, args.count, sizeof(*args.jobs)))
		return ENOMEM;

	/* reset count */
	args.count_only = false;
	args.count = 0;

	_dump_job_state_locked(&args, filter_jobs_count, filter_jobs_ptr);

	*jobs_pptr = args.jobs;
	*jobs_count_ptr = args.count;
	return args.rc;
}
