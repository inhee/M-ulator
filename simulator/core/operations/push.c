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
#include "cpu/core.h"

static void push_v6(uint16_t inst) {
	uint32_t sp = CORE_reg_read(SP_REG);

	int hamming = 0;
	{
		int reg_list = inst & 0xff;
		for (hamming = 0; reg_list; hamming++) {
			reg_list &= (reg_list - 1); // clears LSB
		}
		hamming += !!(inst & 0x100); // LR?

		assert((hamming > 0) && (hamming <= 9));
	}


	uint32_t start_address = sp - 4*hamming;
	uint32_t end_address = sp - 4;
	uint32_t address = start_address;

	int i;
	for (i = 0; i < 8; i++) {
		if (inst & (1 << i)) {
			write_word(address, CORE_reg_read(i));
			address += 4;
		}
	}
	if (inst & 0x100) {
		write_word(address, CORE_reg_read(LR_REG));
		address += 4;
	}

	assert ((end_address == address - 4) && "Sanity check");

	sp = sp - 4*hamming;
	CORE_reg_write(SP_REG, sp);

	DBG2("push {%sregisters %x (bitwise)}\n",
			(inst & 0x100)?"LR and ":"", inst & 0xff);
}

static void push(const uint16_t registers) {
	uint32_t sp = CORE_reg_read(SP_REG);

	uint32_t address = sp - 4 * hamming(registers);

	int i;
	for (i=0; i <= 14; i++) {
		if (registers & (1 << i)) {
			write_word(address, CORE_reg_read(i));
			address += 4;
		}
	}

	CORE_reg_write(SP_REG, sp - 4 * hamming(registers));
}

static void push_t2(uint32_t inst) {
	uint16_t register_list = inst & 0x1fff;
	bool M = (inst >> 14) & 0x1;

	uint16_t registers = (M << 14) | register_list;

	if (hamming(registers) < 2)
		CORE_ERR_unpredictable("Too few regs to push\n");

	push(registers);
}

static void push_t3(uint32_t inst) {
	uint8_t rt = (inst >> 12) & 0xf;
	uint16_t registers = (1 << rt);

	if (BadReg(rt))
		CORE_ERR_unpredictable("bad reg\n");

	push(registers);
}

__attribute__ ((constructor))
void register_opcodes_push(void) {
	// 1011 010x <x's>
	register_opcode_mask_16(0xb400, 0x4a00, push_v6);

	// 1110 1001 0010 1101 0x0x xxxx xxxx xxxx
	register_opcode_mask_32(0xe92d0000, 0x16d2a000, push_t2);

	// 1111 1000 0100 1101 xxxx 1101 0000 0100 (t3)
	register_opcode_mask_32(0xf84d0d04, 0x07b202fb, push_t3);
}
