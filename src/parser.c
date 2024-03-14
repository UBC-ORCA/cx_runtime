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

static cx_config_t* readConf(char* filename) {
  /* TODO: Find the number of cx_config files here */

  cx_config_t *cx_config = (cx_config_t *) malloc(sizeof(cx_config));
  const char* file = "/home/bf/research/riscv-tools/cx_runtime/addsub.txt";
  printf("filename: %s\n", filename);
  FILE* test_file = fopen(file, "w");
  if (test_file == NULL) {
    fprintf(stderr, "open error for %s, errno = %d\n", file, errno);
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
        cx_config->cx_guid = value;
        cx_guid = 1;
    } else if (strstr(id, "num_states")) {
        cx_config->num_states = value;
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
cx_config_info_t read_files(char *path) 
{
  // Unfortunately, qemu can't access host files. This means that the
  // structs need to be defined in code, or that we shift to using spike
  // at some point down the road.
  int32_t num_cxs = 3;
  // *readConf("addsub.txt");

  cx_config_t static cx_config[] = {
    {.cx_guid = CX_GUID_ADDSUB, .num_states = 0},
    {.cx_guid = CX_GUID_MULDIV, .num_states = 3},
    {.cx_guid = CX_GUID_TEMP,   .num_states = 1025}
  };

  // int32_t num_cxs = 2;
  // const char * cx_metadata_files[] = { 
  //   "addsub.yaml",
  //   "muldiv.yaml"
  // };

  // cx_config_t *cx_config = (cx_config_t *) malloc(sizeof(cx_config_t) * num_cxs);
  // char *new_path = "/home/bf/research/riscv-tools/cx_runtime/cx_metadata/";
  // for (int32_t i = 0; i < num_cxs; i++) {
  //   char buf[256];
  //   snprintf(buf, sizeof(buf), "%s%s", new_path, cx_metadata_files[i]);
  //   cx_config[i] = *readConf(buf);
  // }
  // printf("in parser\n");
  
  cx_config_info_t cx_config_info = gen_cx_config_info_t(cx_config, num_cxs);

  return cx_config_info;
}

// int main() {
//   cx_config_info_t* metadata = read_files("../cx_metadata/");
//   printf("guid addsub: %d, guid muldiv: %d\n", metadata->cx_config[0].cx_guid, metadata->cx_config[1].cx_guid);
// }