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

#include "opcodes.h"
#include "helpers.h"

#include "cpu/registers.h"

static void udiv(uint8_t rd, uint8_t rn, uint8_t rm) {
	uint32_t rn_val = CORE_reg_read(rn);
	uint32_t rm_val = CORE_reg_read(rm);

	if (rm_val == 0)
		CORE_ERR_not_implemented("Divide by 0 exception\n");

	uint32_t result;
	result = rn_val / rm_val;

	CORE_reg_write(rd, result);
}

static void udiv_t1(uint32_t inst) {
	uint8_t rm = inst & 0xf;
	uint8_t rd = (inst >> 8) & 0xf;
	uint8_t rn = (inst >> 16) & 0xf;

	if (BadReg(rd) || BadReg(rn) || BadReg(rm))
		CORE_ERR_unpredictable("bad reg\n");

	return udiv(rd, rn, rm);
}

__attribute__ ((constructor))
void register_opcodes_div(void) {
	// udiv_t1: 1111 1011 1011 xxxx 1111 xxxx 1111 xxxx
	register_opcode_mask_32(0xfbb0f0f0, 0x04400000, udiv_t1);
}
