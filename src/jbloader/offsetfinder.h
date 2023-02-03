#ifndef OFFSETFINDER_H
#define OFFSETFINDER_H

#include <stdint.h>
#include <strings.h>

uint32_t* find_next_insn_matching_64(uint64_t region, uint8_t* kdata, size_t ksize, uint32_t* current_instruction, int (*match_func)(uint32_t*));
uint32_t* find_insn_matching_64_with_count(uint64_t region, uint8_t* kdata, size_t ksize, uint32_t* current_instruction, int (*match_func)(uint32_t*), unsigned int count);
uint32_t bit_range_64(uint32_t x, int start, int end);
uint64_t real_signextend_64(uint64_t imm, uint8_t bit);
uint64_t signextend_64(uint64_t imm, uint8_t bit);
uint32_t insn_mov_imm_imm_64(uint32_t* i);
uint32_t make_branch(uint64_t w, uint64_t a);
uint32_t* find_insn_maskmatch_match(uint8_t* data, size_t size, uint32_t* matches, uint32_t* masks, int count);
uint32_t* find_next_insn_matching_64(uint64_t region, uint8_t* kdata, size_t ksize, uint32_t* current_instruction, int (*match_func)(uint32_t*));
uint32_t* find_insn_matching_64_with_count(uint64_t region, uint8_t* kdata, size_t ksize, uint32_t* current_instruction, int (*match_func)(uint32_t*), unsigned int count);
uint32_t* find_literal_ref_64(uint64_t region, uint8_t* kdata, size_t ksize, uint32_t* insn, uint64_t address);
uint32_t* find_last_insn_matching_64(uint64_t region, uint8_t* kdata, size_t ksize, uint32_t* current_instruction, int (*match_func)(uint32_t*));
#endif