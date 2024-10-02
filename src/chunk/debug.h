//
// Created by ran on 2024-03-14.
//

#ifndef ZHI_CHUNK_DEBUGS_H_
#define ZHI_CHUNK_DEBUGS_H_
#include "object.h"
#ifdef __cplusplus
extern "C" {
#endif
void disassemble_chunk(Chunk *chunk, const char *name);
size_t disassemble_instruction(Chunk *chunk, size_t offset);
#ifdef __cplusplus
}
#endif
#endif //ZHI_CHUNK_DEBUGS_H_
