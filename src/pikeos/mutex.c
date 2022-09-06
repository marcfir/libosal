/**
 * \file pikeos/mutex.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL mutex pikeos source.
 *
 * OSAL mutex pikeos source.
 */

/*
 * This file is part of libosal.
 *
 * libosal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libosal is distributed in the hope that 
 * it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libosal. If not, see <http://www.gnu.org/licenses/>.
 */

#include <libosal/osal.h>

#include <assert.h>

//! \brief Initialize a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial mutex attributes. Can be NULL then
 *                      the defaults of the underlying mutex will be used.
 *
 * \return OK or ERROR_CODE.
 */
int osal_mutex_init(osal_mutex_t *mtx, const osal_mutex_attr_t *attr) {
    assert(mtx != NULL);

    (void)attr;

    int ret = OSAL_OK;
    p4_mutex_init(&mtx->pike_mtx, P4_MUTEX_SHARED);

    return ret;
}

//! \brief Locks a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_mutex_lock(osal_mutex_t *mtx) {
    assert(mtx != NULL);

    int ret = OSAL_OK;
    int local_ret = p4_mutex_lock(&mtx->vx_mtx, P4_TIMEOUT_NULL);
    if (local_ret != P4_E_OK) {
        switch (local_ret) {
            case P4_E_STATE:        // if the caller already owns the mutex (recursive locking attempt on non-recursive mutex).
                ret = OSAL_ERR_BUSY;
                break;
            case P4_E_LIMIT:        // if the maximum recursion level P4_MUTEX_MAX_RECURSION is reached. / if the maximum number of robust mutexes is reached.
                ret = OSAL_ERR_SYSTEM_LIMIT_REACHED;
                break;
            case P4_E_TIMEOUT:      // if the specified timeout has expired before the lock was acquired by the caller.
                ret = OSAL_ERR_TIMEOUT;
                break;
            case P4_E_BADTIMEOUT:   // if the specified timeout is invalid or in the past.
            case P4_E_BADUID:       // if mutex references invalid waiting threads.
            case P4_E_INVAL:        // if mutex is NULL or does not point to a valid address or exceeds the caller’s virtual address space.
                ret = OSAL_ERR_INVALID_PARAM;
                break;
            case P4_E_PAGEFAULT:    // if mutex is not fully mapped in the caller’s virtual address space.
            case P4_E_CANCEL:       // if the mutex is cancelable (flag P4_MUTEX_CANCELABLE is set), and the function was canceled by another thread, the calling thread was moved to another time partition, or the thread was migrated to another CPU.
            case P4_E_ABORT:        // if the previous lock owner of the robust mutex mutex died. Note that at most
            case P4_ULOCK_LIMIT:    // threads waiting for a robust mutex are woken up when the lock owner dies.
            case P4_E_NOABILITY:    // if the mutex is shareable (P4_MUTEX_SHARED is used), but the task of the calling thread does not have the ability P4_AB_ULOCK_SHARED enabled.
            default:
                ret = OSAL_ERR_UNAVAILABLE;
                break;
    }

    return ret;
}

//! \brief Tries to lock a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_mutex_trylock(osal_mutex_t *mtx) {
    assert(mtx != NULL);

    int ret = OSAL_OK;
    int local_ret = p4_mutex_trylock(&mtx->vx_mtx);
    if (local_ret != 0) {
    }

    return ret;
}

//! \brief Unlocks a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_mutex_unlock(osal_mutex_t *mtx) {
    assert(mtx != NULL);

    int ret = OSAL_OK;
    int local_ret = p4_mutex_unlock(&mtx->vx_mtx);
    if (local_ret != 0) {
    }

    return ret;
}

//! \brief Destroys a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_mutex_destroy(osal_mutex_t *mtx) {
    assert(mtx != NULL);

    int ret = OSAL_OK;
    return ret;
}