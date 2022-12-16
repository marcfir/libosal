/**
 * \file io.h
 *
 * \author Robert Burger <robert.burger@dlr.de>
 * \author Martin Stelzer <martin.stelzer@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL io header.
 *
 * OSAL io include header.
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

#ifndef LIBOSAL_IO__H
#define LIBOSAL_IO__H

#include <libosal/config.h>
#include <libosal/types.h>
#include <libosal/timer.h>

#define LIBOSAL_IO_SHM_MAX_MSG_SIZE 512

#ifdef __cplusplus
extern "C" {
#endif

//! \brief Get the current state of a created thread.
/*!
 * \param[in]   fmt     Print format.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_printf(const osal_char_t *fmt, ...)  __attribute__ ((format (printf, 1, 2)));

//! \brief Write message to stdout
/*!
 * \param[in]   msg     Message to be printed.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_puts(const osal_char_t *msg);

//! \brief Set up printing to shm instead of stdout
/*!
 * \param[in]   shm_name    Name of logging shared memory.
 *
 * \return OSAL_OK on success, otherwise OSAL_ERR_*
 */
osal_retval_t osal_io_shm_setup(const osal_char_t *shm_name, const osal_size_t max_msgs, const osal_size_t max_msg_size);

//! \brief Get next message printed to shm.
/*!
 * \param[out]   msg        Message to be returned.
 * \param[in]    to         Timeout when waiting if no message is available.
 *
 * \return OSAL_OK on success otherwise OSAL_ERR_UNAVAILABLE 
 */
osal_retval_t osal_io_shm_get_message(osal_char_t msg[LIBOSAL_IO_SHM_MAX_MSG_SIZE],
        const osal_timer_t *to);

#ifdef __cplusplus
};
#endif

#endif /* LIBOSAL_MUTEX__H */

