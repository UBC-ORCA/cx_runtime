#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
// Note: dirnet is not supported with riscv

#include "../include/parser.h"

static cx_config_info_t gen_cx_config_info_t(cx_config_t *cx_config, int32_t num_cxs)
{
  cx_config_info_t cx_config_info;
  cx_config_info.cx_config = cx_config;
  cx_config_info.num_cxs = num_cxs;

  return cx_config_info;
}

static cx_config_t readConf(char* filename) {

  cx_config_t cx_config = {0};

  FILE* test_file = fopen(filename, "r");
  if (test_file == NULL) {
    fprintf(stderr, "open error for %s, errno = %d\n", filename, errno);
    exit(0);
  }

  int cx_guid = 0;
  int num_states = 0;
  char id[15];

  while(1) {
    int value;
    if (fscanf(test_file, "%s %d", id, &value) < 2){
        break;
    }
    if (strstr(id, "cx_guid")) {
        cx_config.cx_guid = value;
        cx_guid = 1;
    } else if (strstr(id, "num_states")) {
        cx_config.num_states = value;
        num_states = 1;
    } else {
        printf("Unrecognized input\n");
    }
  }

  fclose(test_file);

  if (cx_guid == 0 || num_states == 0) {
      printf("Could not find both cx_guid and num_states.\n");
      exit(0);
  }
  return cx_config;
}

// TODO: there should be another (better) way to do this - note that 
//       dirnet is not supported with the riscv toolchain
cx_config_info_t read_files(char **paths, int32_t num_cxs) 
{

  cx_config_t *cx_config = (cx_config_t *) malloc(sizeof(cx_config_t) * num_cxs);
  for (int32_t i = 0; i < num_cxs; i++) {
    cx_config[i] = readConf(paths[i]);
  }
  
  cx_config_info_t cx_config_info = gen_cx_config_info_t(cx_config, num_cxs);

  return cx_config_info;
}
