/*
 * patchfinder64.c
 *
 * Created by Vladimir Putin on 14.12.16.
 * Fixes by Alex Hude and Max Bazaliy
 * Updated by sakuRdev on 2021/12/02.
 * Some parts of code from Luca Todesco and Pangu
 *
 * Copyright (c) 2016 -2017 FriedApple Team.
 * Copyright (c) 2021 sakuRdev.
 * Copyright (c) 2022 dora2ios.
 *
 * This file is part of kokeshidoll.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
// printf
#include <stdio.h>

uint32_t* find_insn_maskmatch_match(uint8_t* data, size_t size, uint32_t* matches, uint32_t* masks, int count)
{
    int found = 0;
    if(sizeof(matches) != sizeof(masks))
        return NULL;
    
    uint32_t* current_inst = (uint32_t*)data;
    while((uintptr_t)current_inst < (uintptr_t)data + size - 4 - (count*4)) {
        current_inst++;
        found = 1;
        for(int i = 0; i < count; i++)
        {
            if((matches[i] & masks[i]) != (current_inst[i] & masks[i]))
            {
                found = 0;
                break;
            }
        }
        if(found)
        {
            return current_inst;
        }
    }
    
    return NULL;
}

uint32_t* find_next_insn_matching_64(uint64_t region, uint8_t* kdata, size_t ksize, uint32_t* current_instruction, int (*match_func)(uint32_t*))
{
    while((uintptr_t)current_instruction < (uintptr_t)kdata + ksize - 4) {
        current_instruction++;
        
        if(match_func(current_instruction)) {
            return current_instruction;
        }
    }
    
    return NULL;
}

uint32_t* find_insn_matching_64_with_count(uint64_t region, uint8_t* kdata, size_t ksize, uint32_t* current_instruction, int (*match_func)(uint32_t*), unsigned int count)
{
    unsigned int cnt = 0;
    while((uintptr_t)current_instruction < (uintptr_t)kdata + ksize - 4) {
        current_instruction++;
        
        if(match_func(current_instruction)) {
            if(cnt == count)
            {
                return current_instruction;
            }
            cnt++;
        }
        
    }
    
    return NULL;
}

uint32_t bit_range_64(uint32_t x, int start, int end)
{
    x = (x << (31 - start)) >> (31 - start);
    x = (x >> end);
    return x;
}

uint64_t real_signextend_64(uint64_t imm, uint8_t bit)
{
    if ((imm >> bit) & 1) {
        return (-1LL << (bit + 1)) + imm;
    } else
        return imm;
}

uint64_t signextend_64(uint64_t imm, uint8_t bit)
{
    //assert(bit > 0);
    return real_signextend_64(imm, bit - 1);
    /*
     if ((imm >> bit) & 1)
     return (uint64_t)(-1) - (~((uint64_t)1 << bit)) + imm;
     else
     return imm;
     */
}

int insn_is_mov_imm_64(uint32_t* i)
{
    if ((*i & 0x7f800000) == 0x52800000)
        return 1;
    
    return 0;
}

int insn_mov_imm_rd_64(uint32_t* i)
{
    return (*i & 0x1f);
}

uint32_t insn_mov_imm_imm_64(uint32_t* i)
{
    return bit_range_64(*i, 20, 5);
}

int insn_is_ldr_literal_64(uint32_t* i)
{
    // C6.2.84 LDR (literal) LDR Xt
    if ((*i & 0xff000000) == 0x58000000)
        return 1;
    
    // C6.2.84 LDR (literal) LDR Wt
    if ((*i & 0xff000000) == 0x18000000)
        return 1;
    
    // C6.2.95 LDR (literal) LDRSW Xt
    if ((*i & 0xff000000) == 0x98000000)
        return 1;
    
    return 0;
}

int insn_nop_64(uint32_t *i)
{
    return (*i == 0xD503201F);
}

int insn_add_reg_rm_64(uint32_t* i)
{
    return ((*i >> 16) & 0x1f);
}

int insn_ldr_literal_rt_64(uint32_t* i)
{
    return (*i & 0x1f);
}

uint64_t insn_ldr_literal_imm_64(uint32_t* i)
{
    uint64_t imm = (*i & 0xffffe0) >> 3;
    return signextend_64(imm, 21);
}

uint64_t insn_adr_imm_64(uint32_t* i)
{
    uint64_t immhi = bit_range_64(*i, 23, 5);
    uint64_t immlo = bit_range_64(*i, 30, 29);
    uint64_t imm = (immhi << 2) + (immlo);
    return signextend_64(imm, 19+2);
}

uint64_t insn_adrp_imm_64(uint32_t* i)
{
    uint64_t immhi = bit_range_64(*i, 23, 5);
    uint64_t immlo = bit_range_64(*i, 30, 29);
    uint64_t imm = (immhi << 14) + (immlo << 12);
    return signextend_64(imm, 19+2+12);
}

int insn_is_adrp_64(uint32_t* i)
{
    if ((*i & 0x9f000000) == 0x90000000) {
        return 1;
    }
    
    return 0;
}

int insn_adrp_rd_64(uint32_t* i)
{
    return (*i & 0x1f);
}

int insn_is_add_imm_64(uint32_t* i)
{
    if ((*i & 0x7f000000) == 0x11000000)
        return 1;
    
    return 0;
}

int insn_add_imm_rd_64(uint32_t* i)
{
    return (*i & 0x1f);
}

int insn_add_imm_rn_64(uint32_t* i)
{
    return ((*i >> 5) & 0x1f);
}

uint64_t insn_add_imm_imm_64(uint32_t* i)
{
    uint64_t imm = bit_range_64(*i, 21, 10);
    if (((*i >> 22) & 3) == 1)
        imm = imm << 12;
    return imm;
}

int insn_add_reg_rd_64(uint32_t* i)
{
    return (*i & 0x1f);
}

int insn_add_reg_rn_64(uint32_t* i)
{
    return ((*i >> 5) & 0x1f);
}

int insn_is_add_reg_64(uint32_t* i)
{
    if ((*i & 0x7fe00c00) == 0x0b200000)
        return 1;
    
    return 0;
}

int insn_is_adr_64(uint32_t* i)
{
    if ((*i & 0x9f000000) == 0x10000000)
        return 1;
    
    return 0;
}

int insn_is_bl_64(uint32_t* i)
{
    if ((*i & 0xfc000000) == 0x94000000)
        return 1;
    else
        return 0;
}

uint64_t insn_bl_imm32_64(uint32_t* i)
{
    uint64_t imm = (*i & 0x3ffffff) << 2;
    //PFExtLog("imm = %llx\n", imm);
    // sign extend
    uint64_t res = real_signextend_64(imm, 27);
    
    //PFExtLog("real_signextend_64 = %llx\n", res);
    
    return res;
}

int insn_is_funcbegin_64(uint32_t* i)
{
    if (*i == 0xa9bf7bfd)
        return 1;
    if (*i == 0xa9bc5ff8)
        return 1;
    if (*i == 0xa9bd57f6)
        return 1;
    if (*i == 0xa9ba6ffc)
        return 1;
    if (*i == 0xa9bb67fa)
        return 1;
    if (*i == 0xa9be4ff4)
        return 1;
    return 0;
}

int insn_is_cbz_w32(uint32_t* i)
{
    return (*i >> 24 == 0x34);
}

int insn_is_b_unconditional_64(uint32_t* i)
{
    if ((*i & 0xfc000000) == 0x14000000)
        return 1;
    else
        return 0;
}

uint32_t* find_literal_ref_64(uint64_t region, uint8_t* kdata, size_t ksize, uint32_t* insn, uint64_t address)
{
    uint32_t* current_instruction = insn;
    uint64_t registers[32];
    memset(registers, 0, sizeof(registers));
    
    while((uintptr_t)current_instruction < (uintptr_t)(kdata + ksize))
    {
        if (insn_is_mov_imm_64(current_instruction))
        {
            int reg = insn_mov_imm_rd_64(current_instruction);
            uint64_t value = insn_mov_imm_imm_64(current_instruction);
            registers[reg] = value;
            //PFExtLog("%s:%d mov (immediate): reg[%d] is reset to %#llx\n", __func__, __LINE__, reg, value);
        }
        else if (insn_is_ldr_literal_64(current_instruction))
        {
            uintptr_t literal_address  = (uintptr_t)current_instruction + (uintptr_t)insn_ldr_literal_imm_64(current_instruction);
            if(literal_address >= (uintptr_t)kdata && (literal_address + 4) <= ((uintptr_t)kdata + ksize))
            {
                int reg = insn_ldr_literal_rt_64(current_instruction);
                uint64_t value =  *(uint64_t*)(literal_address);
                registers[reg] = value;
                //PFExtLog("%s:%d ldr (literal): reg[%d] is reset to %#llx\n", __func__, __LINE__, reg, value);
            }
        }
        else if (insn_is_adrp_64(current_instruction))
        {
            int reg = insn_adrp_rd_64(current_instruction);
            uint64_t value = ((((uintptr_t)current_instruction - (uintptr_t)kdata) >> 12) << 12) + insn_adrp_imm_64(current_instruction);
            registers[reg] = value;
            //PFExtLog("%s:%d adrp: reg[%d] is reset to %#llx\n", __func__, __LINE__, reg, value);
        }
        else if (insn_is_adr_64(current_instruction))
        {
            uint64_t value = (uintptr_t)current_instruction - (uintptr_t)kdata + insn_adr_imm_64(current_instruction);
            if (value == address)
            {
                //PFExtLog("%s:%d FINAL pointer is %#llx\n", __func__, __LINE__, (uint64_t)current_instruction - (uint64_t)kdata);
                return current_instruction;
            }
        }
        else if(insn_is_add_reg_64(current_instruction))
        {
            int reg = insn_add_reg_rd_64(current_instruction);
            if(insn_add_reg_rm_64(current_instruction) == 15 && insn_add_reg_rn_64(current_instruction) == reg)
            {
                uint64_t value = ((uintptr_t)current_instruction - (uintptr_t)kdata) + 4;
                registers[reg] += value;
                //PFExtLog("%s:%d adrp: reg[%d] += %#llx\n", __func__, __LINE__, reg, value);
                if(registers[reg] == address)
                {
                    //PFExtLog("%s:%d FINAL pointer is %#llx\n", __func__, __LINE__, (uint64_t)current_instruction - (uint64_t)kdata);
                    return current_instruction;
                }
            }
        }
        else if (insn_is_add_imm_64(current_instruction))
        {
            int reg = insn_add_imm_rd_64(current_instruction);
            if (insn_add_imm_rn_64(current_instruction) == reg)
            {
                uint64_t value = insn_add_imm_imm_64(current_instruction);
                registers[reg] += value;
                //PFExtLog("%s:%d adrp: reg[%d] += %#llx\n", __func__, __LINE__, reg, value);
                if (registers[reg] == address)
                {
                    //PFExtLog("%s:%d FINAL pointer is %#llx\n", __func__, __LINE__, (uint64_t)current_instruction - (uint64_t)kdata);
                    return current_instruction;
                }
            }
            
        }
        
        current_instruction++;
    }
    
    //PFExtLog("%s:%d FINAL pointer is NULL\n", __func__, __LINE__);
    return NULL;
}

// search next instruction, decrementing mode
uint32_t* find_last_insn_matching_64(uint64_t region, uint8_t* kdata, size_t ksize, uint32_t* current_instruction, int (*match_func)(uint32_t*))
{
    while((uintptr_t)current_instruction > (uintptr_t)kdata) {
        current_instruction--;
        
        if(match_func(current_instruction)) {
            return current_instruction;
        }
    }
    
    return NULL;
}


uint32_t make_branch(uint64_t w, uint64_t a)
{
    uint64_t res = 0;
    uint32_t opcode = 0;
    if(a > w) {
        opcode |= 5<<26;
        res = (a - w) / 4;
        opcode |= (res % (1<<25));
    } else {
        opcode |= 3<<27;
        res = (w - a) / 4;
        opcode -= (res % (1<<25));
    }
    return opcode;
}
