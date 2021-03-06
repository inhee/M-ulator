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
#include "cpu/misc.h"

static void lsl1(uint16_t inst) {
	uint8_t immed5 = (inst & 0x7c0) >> 6;
	uint8_t rm = (inst & 0x38) >> 3;
	uint8_t rd = (inst & 0x7) >> 0;

	uint32_t rm_val = CORE_reg_read(rm);
	uint8_t c_flag;
	uint32_t result;

	union apsr_t apsr = CORE_apsr_read();

	if (immed5 == 0) {
		result = rm_val;
		c_flag = apsr.bits.C;
	} else {
		c_flag = !!( rm_val & (1 << (32 - immed5)) );
		result = rm_val << immed5;
	}

	CORE_reg_write(rd, result);

	if (!in_ITblock()) {
		apsr.bits.N = HIGH_BIT(result);
		apsr.bits.Z = result == 0;
		apsr.bits.C = c_flag;
		CORE_apsr_write(apsr);
	}

	DBG2("lsls r%02d, r%02d, #%d\n", rd, rm, immed5);
}

static void lsl_reg(uint8_t rd, uint8_t rn, uint8_t rm, bool setflags) {
	uint8_t shift_n = CORE_reg_read(rm);

	uint32_t result;
	bool carry;

	union apsr_t apsr = CORE_apsr_read();

	Shift_C(CORE_reg_read(rn), 32, SRType_LSL, shift_n, apsr.bits.C,
			&result, &carry);

	CORE_reg_write(rd, result);

	if (setflags) {
		apsr.bits.N = HIGH_BIT(result);
		apsr.bits.Z = result == 0;
		apsr.bits.C = carry;
		CORE_apsr_write(apsr);
	}
}

static void lsl_reg_t1(uint16_t inst) {
	uint8_t rdn = inst & 0x7;
	uint8_t rm = (inst >> 3) & 0x7;
	bool setflags = !in_ITblock();

	return lsl_reg(rdn, rdn, rm, setflags);
}

static void lsr1(uint16_t inst) {
	uint8_t immed5 = (inst & 0x7c0) >> 6;
	uint8_t rm = (inst & 0x38) >> 3;
	uint8_t rd = (inst & 0x7) >> 0;

	uint32_t rm_val = CORE_reg_read(rm);
	uint8_t c_flag;
	uint32_t result;

	union apsr_t apsr = CORE_apsr_read();

	if (immed5 == 0) {
		result = rm_val;
		c_flag = apsr.bits.C;
	} else {
		c_flag = !!( rm_val & (1 << (immed5 - 1)) );
		result = rm_val >> immed5;
	}

	CORE_reg_write(rd, result);

	if (!in_ITblock()) {
		apsr.bits.N = HIGH_BIT(result);
		apsr.bits.Z = result == 0;
		apsr.bits.C = c_flag;
		CORE_apsr_write(apsr);
	}

	DBG2("lsrs r%02d, r%02d, #%d\n", rd, rm, immed5);
}

static void asr_imm(union apsr_t apsr, uint8_t setflags, uint8_t rd, uint8_t rm,
		enum SRType shift_t, uint8_t shift_n) {
	uint32_t rm_val = CORE_reg_read(rm);

	uint32_t result;
	bool carry;
	Shift_C(rm_val, 32, shift_t, shift_n, apsr.bits.C, &result, &carry);

	if (rd == 15) {
		// ALUWritePC(result);
		CORE_ERR_not_implemented("ALUWritePC asr_imm\n");
	} else {
		CORE_reg_write(rd, result);
		if (setflags) {
			apsr.bits.N = HIGH_BIT(result);
			apsr.bits.Z = result == 0;
			apsr.bits.C = carry;
			CORE_apsr_write(apsr);
		}
	}

	DBG2("asr_imm complete\n");
}

static void asr_imm_t1(uint16_t inst) {
	uint8_t rd = inst & 0x7;
	uint8_t rm = (inst >> 3) & 0x7;
	uint8_t imm5 = (inst >> 6) & 0x1f;
	bool setflags = !in_ITblock();

	enum SRType shift_t;
	uint8_t shift_n;
	DecodeImmShift(0x2, imm5, &shift_t, &shift_n);

	return asr_imm(CORE_apsr_read(), setflags, rd, rm,
			shift_t, shift_n);
}

static void mov_shifted_reg(uint32_t inst, enum SRType shift_t) {
	union apsr_t apsr = CORE_apsr_read();

	uint8_t rm = inst & 0xf;
	uint8_t imm2 = (inst >> 6) & 0x3;
	uint8_t rd = (inst >> 8) & 0xf;
	uint8_t imm3 = (inst >> 12) & 0x7;
	bool S = !!(inst & 0x100000);

	uint8_t setflags = (S == 1);

	uint8_t imm5 = (imm3 << 2) | imm2;
	uint8_t shift_n;
	DecodeImmShift(SRType_ENUM_TO_MASK(shift_t), imm5, &shift_t, &shift_n);

	if (BadReg(rd) || BadReg(rm))
		CORE_ERR_unpredictable("BadReg in (fake)mov_shifted_reg\n");

	return asr_imm(apsr, setflags, rd, rm, shift_t, shift_n);
}

static void asr_imm_t2(uint32_t inst) {
	return mov_shifted_reg(inst, ASR);
}

static void lsl_imm_t2(uint32_t inst) {
	return mov_shifted_reg(inst, LSL);
}

static void lsr_imm_t2(uint32_t inst) {
	return mov_shifted_reg(inst, LSR);
}

static void ror_imm(uint8_t rd, uint8_t rm, uint8_t shift_n, bool setflags) {
	uint32_t rm_val = CORE_reg_read(rm);

	uint32_t result;
	bool carry_out;

	union apsr_t apsr = CORE_apsr_read();

	Shift_C(rm_val, 32, ROR, shift_n, apsr.bits.C, &result, &carry_out);

	CORE_reg_write(rd, result);

	if (setflags) {
		apsr.bits.N = HIGH_BIT(result);
		apsr.bits.Z = result == 0;
		apsr.bits.C = carry_out;
		CORE_apsr_write(apsr);
	}
}

static void ror_imm_t1(uint32_t inst) {
	uint8_t rm = inst & 0xf;
	uint8_t imm2 = (inst >> 6) & 0x3;
	uint8_t rd = (inst >> 8) & 0xf;
	uint8_t imm3 = (inst >> 12) & 0x7;
	bool S = !!(inst & 0x100000);

	bool setflags = S;

	uint8_t imm5 = (imm3 << 2) | imm2;
	enum SRType shift_t __attribute__ ((unused));
	uint8_t shift_n;
	DecodeImmShift(0x3, imm5, &shift_t, &shift_n);

	if (BadReg(rd) || BadReg(rm))
		CORE_ERR_unpredictable("Bad reg\n");

	return ror_imm(rd, rm, shift_n, setflags);
}

__attribute__ ((constructor))
void register_opcodes_shift(void) {
	// lsl1: 0000 0<x's>
	register_opcode_mask_16_ex(0x0000, 0xf800, lsl1,
			0x0, 0x07c0, 0, 0);

	// lsl_reg_t1: 0100 0000 10 <x's>
	register_opcode_mask_16(0x4080, 0xbf40, lsl_reg_t1);

	// lsr1: 0000 1<x's>
	register_opcode_mask_16(0x0800, 0xf000, lsr1);

	// asr_imm_t1: 0001 0xxx xxxx xxxx
	register_opcode_mask_16(0x1000, 0xe800, asr_imm_t1);

	// asr_imm_t2: 1110 1010 010x 1111 0xxx xxxx xx10 xxxx
	register_opcode_mask_32(0xea4f0020, 0x15a08010, asr_imm_t2);

	// lsl_imm_t2: 1110 1010 010x 1111 0xxx xxxx xx00 xxxx
	register_opcode_mask_32(0xea4f0000, 0x15a08030, lsl_imm_t2);

	// lsr_imm_t2: 1110 1010 010x 1111 0xxx xxxx xx01 xxxx
	register_opcode_mask_32(0xea4f0010, 0x15a08020, lsr_imm_t2);

	// ror_imm_t1: 1110 1010 010x 1111 0xxx xxxx xx11 xxxx
	register_opcode_mask_32_ex(0xea4f0030, 0x15a08000, ror_imm_t1,
			0x0, 0x70c0,
			0, 0);
}
