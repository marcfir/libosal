/**
 * \file trace.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 02 May 2023
 *
 * \brief OSAL trace source.
 *
 * OSAL trace source.
 */

/*
 * This file is part of libosal.
 *
 * libosal is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * libosal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with libosal; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <libosal/config.h>
#endif

#include <libosal/osal.h>
#include <libosal/trace.h>
#include <assert.h>
#include <stdlib.h>

#if LIBOSAL_HAVE_MATH_H == 1
#include <math.h>
#endif

#if LIBOSAL_HAVE_STRING_H == 1
#include <string.h>
#endif

//! \brief Allocate trace struct.
/*!
 * \param[out]  trace   Pointer to trace* where allocated trace struct is returned.
 * \param[in]   cnt     Number of samples to allocate.
 *                      the defaults of the underlying task will be used.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_trace_alloc(osal_trace_t **trace, osal_uint32_t cnt) {
    assert(trace != NULL);
    osal_retval_t ret = OSAL_OK;

    (*trace) = malloc(sizeof(osal_trace_t));
    memset((*trace), 0, sizeof(osal_trace_t));

    if ((*trace) == NULL) {
        ret = OSAL_ERR_OUT_OF_MEMORY;
    } else {
        (*trace)->cnt       = cnt;
        (*trace)->act_buf   = 0;
        (*trace)->pos       = 0;

        ret = osal_binary_semaphore_init(&(*trace)->sync_sem, NULL);
        if (ret != OSAL_OK) {
            goto error_exit;
        }
        
        (*trace)->time_in_ns[0] = malloc(sizeof(osal_uint64_t) * cnt);
        (*trace)->time_in_ns[1] = malloc(sizeof(osal_uint64_t) * cnt);
        (*trace)->tmp           = malloc(sizeof(osal_uint64_t) * cnt);

        if (    ((*trace)->time_in_ns[0] == NULL) ||
                ((*trace)->time_in_ns[1] == NULL) ||
                ((*trace)->tmp           == NULL)) {
            ret = OSAL_ERR_OUT_OF_MEMORY;
            goto error_exit;
        }

        memset((*trace)->time_in_ns[0], 0, sizeof(osal_uint64_t) * cnt);
        memset((*trace)->time_in_ns[1], 0, sizeof(osal_uint64_t) * cnt);
        memset((*trace)->tmp, 0, sizeof(osal_uint64_t) * cnt);
    }

    return ret;

error_exit:
    if ((*trace) != NULL) {
        if ((*trace)->tmp != 0) {
            free((*trace)->tmp);
        }

        if ((*trace)->time_in_ns[1] != 0) {
            free((*trace)->time_in_ns[1]);
        }

        if ((*trace)->time_in_ns[0] != 0) {
            free((*trace)->time_in_ns[0]);
        }

        free((*trace));
    }

    return ret;
}

//! \brief Free trace struct.
/*!
 * \param[in]   trace   Pointer to trace struct to free.
 *
 * \return N/A
 */
void osal_trace_free(osal_trace_t *trace) {
    assert(trace != NULL);

    if (trace->tmp != 0) {
        free(trace->tmp);
    }

    if (trace->time_in_ns[1] != 0) {
        free(trace->time_in_ns[1]);
    }

    if (trace->time_in_ns[0] != 0) {
        free(trace->time_in_ns[0]);
    }

    free(trace);
}

//! \brief Trace time.
/*!
 * \param[in]   trace   Pointer to trace struct.
 *
 * \return stored trace time in [ns].
 */
osal_uint64_t osal_trace_point(osal_trace_t *trace) {
    osal_uint64_t ret_time;

    assert(trace != NULL);

    ret_time = osal_timer_gettime_nsec();
    osal_trace_time(trace, ret_time);

    return ret_time;
}

//! \brief Trace time.
/*!
 * \param[in]   trace   Pointer to trace struct.
 * \param[in]   time    Time to store in trace.
 *
 * \return N/A
 */
void osal_trace_time(osal_trace_t *trace, osal_uint64_t time) {
    assert(trace != NULL);

    trace->time_in_ns[trace->act_buf][trace->pos] = time;

    trace->pos++;
    if (trace->pos >= trace->cnt) {
        trace->act_buf = trace->act_buf == 0 ? 1 : 0;
        trace->pos = 0;

        osal_binary_semaphore_post(&(trace->sync_sem));
    }
}

//! \brief Return last time stored in trace.
/*!
 * \param[in]   trace   Pointer to trace struct.
 * 
 * \return last store trace time in [ns].
 */
osal_uint64_t osal_trace_get_last_time(osal_trace_t *trace) {
    assert(trace != NULL);

    osal_uint64_t last_time = 0u;
    osal_uint32_t last_buf = trace->act_buf;

    if (trace->pos == 0) {
        last_buf = trace->act_buf == 0 ? 1 : 0;
        last_time = trace->time_in_ns[last_buf][trace->cnt - 1];
    } else {
        last_time = trace->time_in_ns[last_buf][trace->pos - 1];
    }

    return last_time;
}

//! \brief Sync to trace when buffer is full.
/*!
 * \param[in]   trace   Pointer to trace struct.
 *
 * \return N/A
 */
osal_retval_t osal_trace_timedwait(osal_trace_t *trace, osal_timer_t *timeout) {
    osal_retval_t ret = osal_binary_semaphore_timedwait(&(trace->sync_sem), timeout);
    return ret;
}

//! \brief Analyze trace and return average and jitters.
/*!
 * \param[in]   trace   Pointer to trace struct.
 * \param[out]  avg     Return average time interval.
 * \param[out]  avg_jit Return average jitter (std-dev).
 * \param[out]  max_jit Return maximum jitter.
 *
 * \return N/A
 */
void osal_trace_analyze(osal_trace_t *trace, osal_uint64_t *avg, osal_uint64_t *avg_jit, osal_uint64_t *max_jit) {
    osal_trace_analyze_min_max(trace, avg, avg_jit, max_jit, NULL, NULL);
}

//! \brief Analyze trace and return average and jitters.
/*!
 * \param[in]   trace   Pointer to trace struct.
 * \param[out]  avg     Return average time interval.
 * \param[out]  avg_jit Return average jitter (std-dev).
 * \param[out]  max_jit Return maximum jitter.
 * \param[out]  min_val Return minimum interval value.
 * \param[out]  max_val Return maximum interval value.
 *
 * \return N/A
 */
void osal_trace_analyze_min_max(osal_trace_t *trace, osal_uint64_t *avg, osal_uint64_t *avg_jit, 
        osal_uint64_t *max_jit, osal_uint64_t *min_val, osal_uint64_t *max_val)
{
    assert(trace != NULL);
    assert(avg != NULL);
    assert(avg_jit != NULL);
    assert(max_jit != NULL);

    (*avg)     = 0u;
    (*avg_jit) = 0u;
    (*max_jit) = 0u;

    int act_buffer = trace->act_buf == 1 ? 0 : 1;

    for (unsigned i = 0; i < (trace->cnt - 1u); ++i) {
        trace->tmp[i] = trace->time_in_ns[act_buffer][i + 1u] - trace->time_in_ns[act_buffer][i];
        (*avg) += trace->tmp[i];

        if (i == 0) {
            if (min_val) { *min_val = trace->tmp[i]; }
            if (max_val) { *max_val = trace->tmp[i]; }
        }

        if (min_val && (*min_val > trace->tmp[i])) { *min_val = trace->tmp[i]; }
        if (max_val && (*max_val < trace->tmp[i])) { *max_val = trace->tmp[i]; }
    }

    (*avg) /= (trace->cnt - 1u);

    for (unsigned i = 0; i < (trace->cnt - 1u); ++i) {
        osal_int64_t dev = (osal_int64_t)(*avg) - trace->tmp[i];
        if (dev < 0) { dev *= -1; }
        if ((osal_uint64_t)dev > (*max_jit)) { (*max_jit) = dev; }

        (*avg_jit) += (dev * dev);
    }

    (*avg_jit) = sqrt((*avg_jit) / trace->cnt);
}


//! \brief Analyze trace with relative timestamps and return average and jitters.
/*!
 * \param[in]   trace   Pointer to trace struct.
 * \param[out]  avg     Return average time interval.
 * \param[out]  avg_jit Return average jitter (std-dev).
 * \param[out]  max_jit Return maximum jitter.
 *
 * \return N/A
 */
void osal_trace_analyze_rel(osal_trace_t *trace, osal_uint64_t *avg, osal_uint64_t *avg_jit, osal_uint64_t *max_jit) {
    osal_trace_analyze_rel_min_max(trace, avg, avg_jit, max_jit, NULL, NULL);
}

//! \brief Analyze trace with relative timestamps and return average and jitters.
/*!
 * \param[in]   trace   Pointer to trace struct.
 * \param[out]  avg     Return average time interval.
 * \param[out]  avg_jit Return average jitter (std-dev).
 * \param[out]  max_jit Return maximum jitter.
 * \param[out]  min_val Return minimum interval value.
 * \param[out]  max_val Return maximum interval value.
 *
 * \return N/A
 */
void osal_trace_analyze_rel_min_max(osal_trace_t *trace, osal_uint64_t *avg, osal_uint64_t *avg_jit, 
        osal_uint64_t *max_jit, osal_uint64_t *min_val, osal_uint64_t *max_val)
{
    assert(trace != NULL);
    assert(avg != NULL);
    assert(avg_jit != NULL);
    assert(max_jit != NULL);

    (*avg)     = 0u;
    (*avg_jit) = 0u;
    (*max_jit) = 0u;

    int act_buffer = trace->act_buf == 1 ? 0 : 1;

    for (unsigned i = 0; i < trace->cnt; ++i) {
        (*avg) += trace->time_in_ns[act_buffer][i];
        
        if (i == 0) {
            if (min_val) { *min_val = trace->time_in_ns[act_buffer][i]; }
            if (max_val) { *max_val = trace->time_in_ns[act_buffer][i]; }
        }

        if (min_val && (*min_val > trace->time_in_ns[act_buffer][i])) { *min_val = trace->time_in_ns[act_buffer][i]; }
        if (max_val && (*max_val < trace->time_in_ns[act_buffer][i])) { *max_val = trace->time_in_ns[act_buffer][i]; }
    }

    (*avg) /= trace->cnt;

    for (unsigned i = 0; i < trace->cnt; ++i) {
        osal_int64_t dev = (osal_int64_t)(*avg) - trace->time_in_ns[act_buffer][i];
        if (dev < 0) { dev *= -1; }
        if ((osal_uint64_t)dev > (*max_jit)) { (*max_jit) = dev; }

        (*avg_jit) += (dev * dev);
    }

    (*avg_jit) = sqrt((*avg_jit) / trace->cnt);
}
