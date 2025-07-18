/**
 * \file osal.h
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL main header.
 *
 * OSAL main include header.
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

#ifndef LIBOSAL_OSAL__H
#define LIBOSAL_OSAL__H

#define OSAL_OK                         0       //!< \brief Ok return code.
#define OSAL_ERR_OPERATION_FAILED       -1      //!< \brief Error operation failed.
#define OSAL_ERR_INVALID_PARAM          -2      //!< \brief Error invalid input parameter.
#define OSAL_ERR_PERMISSION_DENIED      -3      //!< \brief Error permission denied.
#define OSAL_ERR_SYSTEM_LIMIT_REACHED   -4      //!< \brief Error system limit reached.
#define OSAL_ERR_TIMEOUT                -5      //!< \brief Error timeout occured.
#define OSAL_ERR_UNAVAILABLE            -6      //!< \brief Error currently unavailable.
#define OSAL_ERR_OUT_OF_MEMORY          -7      //!< \brief Error system out of memory.
#define OSAL_ERR_NOT_RECOVERABLE        -8      //!< \brief Error not recoverable.
#define OSAL_ERR_OWNER_DEAD             -9      //!< \brief Error current owner has died.
#define OSAL_ERR_DEAD_LOCK              -10     //!< \brief Error dead lock detected.
#define OSAL_ERR_BUSY                   -11     //!< \brief Error resource currently busy.
#define OSAL_ERR_NOT_FOUND              -12     //!< \brief Error resource not found.
#define OSAL_ERR_NOT_IMPLEMENTED        -13     //!< \brief Error function not implemented.
#define OSAL_ERR_NO_DATA                -14     //!< \brief Error no date.
#define OSAL_ERR_INTERRUPTED            -15     //!< \brief Error function call was interrupted.
#define OSAL_ERR_MUTEX_IS_LOCKED        -16     //!< \brief Error mutex is locked.

#ifdef HAVE_CONFIG_H
#include <libosal/config.h>
#endif
#include <libosal/types.h>
#include <libosal/task.h>
#include <libosal/mutex.h>
#include <libosal/semaphore.h>
#include <libosal/spinlock.h>
#include <libosal/binary_semaphore.h>
#include <libosal/timer.h>
#include <libosal/condvar.h>
#ifdef LIBOSAL_BUILD_STM32
#include <libosal/stm32/osal.h>
#endif

//! Initialize OSAL internals.
void osal_init(void);

//! Destroy OSAL internals.
void osal_destroy(void);

#endif /* LIBOSAL_OSAL__H */

