// #ifdef __cplusplus
// extern "C" {
// #endif

#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

#define CX_GUID_ADDSUB 3
#define CX_GUID_MULDIV 7
#define CX_GUID_TEMP 15

typedef struct {
  int32_t cx_guid;
  int32_t num_states;
} cx_config_t;

typedef struct {
  cx_config_t *cx_config;
  int32_t num_cxs;
} cx_config_info_t;

extern cx_config_info_t read_files(char **paths, int32_t num_cxs);

#endif

// #ifdef __cplusplus
// }
// #endif