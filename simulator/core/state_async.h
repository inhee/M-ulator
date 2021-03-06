/* Mulator - An extensible {ARM} {e,si}mulator
 * Copyright 2011-2012  Pat Pannuto <pat.pannuto@gmail.com>
 *
 * This file is part of Mulator.
 *
 * Mulator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mulator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mulator.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef STATE_ASYNC_H
#define STATE_ASYNC_H

#ifndef PP_STRING
#define PP_STRING "STa"
#include "pretty_print.h"
#endif

#define DIRECT_STATE_H_CHECK
#include "state.h"

// Latchable state
#define SR_A(_l) state_read_async(STAGE, false, (_l))
#define SR_AB(_l) state_read_async(STAGE, true, (_l))
uint32_t state_read_async(enum stage, bool in_block, uint32_t *loc)
	__attribute__ ((nonnull));
#define SRP_A(_l) state_read_p_async(STAGE, false, (_l))
#define SRP_AB(_l) state_read_p_async(STAGE, true, (_l))
uint32_t* state_read_p_async(enum stage, bool in_block, uint32_t **loc)
	__attribute__ ((nonnull));
#ifdef DEBUG1
#define SW_A(_l, _v) state_write_async_dbg(STAGE, false, (_l), (_v),\
		__FILE__, __func__, __LINE__, VAL2STR(_l))
#define SW_AB(_l, _v) state_write_async_dbg(STAGE, true, (_l), (_v),\
		__FILE__, __func__, __LINE__, VAL2STR(_l))
void state_write_async_dbg(enum stage, bool in_block,
		uint32_t *loc, uint32_t val,
		const char *file, const char *func,
		const int line, const char *target) __attribute__ ((nonnull));
#define SWP_A(_l, _v) state_write_p_async_dbg(STAGE, false, (_l), (_v),\
		__FILE__, __func__, __LINE__, VAL2STR(_l))
#define SWP_AB(_l, _v) state_write_p_async_dbg(STAGE, true, (_l), (_v),\
		__FILE__, __func__, __LINE__, VAL2STR(_l))
void state_write_p_async_dbg(enum stage, bool in_block,
		uint32_t **ploc, uint32_t *pval,
		const char *file, const char* func,
		const int line, const char *target)
			__attribute__ ((nonnull (3, 5, 6, 8)));
#else
#define SW_A(_l, _v) state_write_async(STAGE, false, (_l), (_v))
#define SW_AB(_l, _v) state_write_async(STAGE, true, (_l), (_v))
void state_write_async(enum stage, bool in_block, uint32_t *loc, uint32_t val)
	__attribute__ ((nonnull));
#define SWP_A(_l, _v) state_write_p_async(STAGE, false, (_l), (_v))
#define SWP_AB(_l, _v) state_write_p_async(STAGE, true, (_l), (_v))
void state_write_p_async(enum stage, bool in_block, uint32_t **ploc, uint32_t *pval)
	__attribute__ ((nonnull (3)));
#endif

#endif // STATE_ASYNC_H
