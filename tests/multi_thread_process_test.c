#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "../../../../research/riscv-tools/cx_runtime/include/ci.h"
#include "../../../../research/riscv-tools/cx_runtime/zoo/mulacc/mulacc.h"

#include <sys/types.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>


static int a = 3, b = 5, c = 2;
// static cx_stctxs_t expected_stctxs = {.sel = {
//                                 .cs = CX_DIRTY,
//                                 .error = 0,
//                                 .initializer = CX_HW_INIT,
//                                 .state_size = 1
//                               }};

void* cx_mac_thread(void *ptr) {
    int val = *(int *)ptr;
    mac(val, val);
    return NULL;
}

void basic_multiP_multiT() { 

  int N = 10;

  cx_error_t cx_error;
  uint cx_status;
  int32_t state_result;
  cx_sel_t cx_index, cx_index_0;

  cx_sel_t cx_sel_0 = cx_open(CX_GUID_MULACC, CX_FULL_VIRT, -1);
  assert(cx_sel_0 > 0);

  pthread_t tid[ N ];
  void *ret;

  cx_sel_t cx_sel_A0 = cx_open(CX_GUID_MULACC, CX_FULL_VIRT, -1);
  assert(cx_sel_A0 > 0);
  cx_sel(cx_sel_A0);

  pid_t pid = fork();
  assert(pid >= 0);

  if (pid == 0) { 
  // child process
    int *ptr = malloc(sizeof(int));
    *ptr = b;
    for (int i = 0; i < N; i++) {
        assert( pthread_create(&tid[i], NULL, cx_mac_thread, ptr) == 0 );
    }

    for (int i = 0; i < N; i++) {
        assert( pthread_join(tid[i], &ret) == 0 );
    }

    int res = CX_READ_STATE(0);
    assert( res == N * b * b);
    assert(cx_csr_read(CX_STATUS) == 0);
    cx_close(cx_sel_A0);
    cx_close(cx_sel_0);

    exit(EXIT_SUCCESS);
  
  } else {
  // parent process
    int *ptr = malloc(sizeof(int));
    *ptr = a;
    for (int i = 0; i < N; i++) {
        assert( pthread_create(&tid[i], NULL, cx_mac_thread, ptr) == 0 );
    }

    for (int i = 0; i < N; i++) {
        assert( pthread_join(tid[i], &ret) == 0 );
    }

    int res = CX_READ_STATE(0);
    assert( res == N * a * a);
    assert(cx_csr_read(CX_STATUS) == 0);
    cx_close(cx_sel_A0);

    int status;
    // Wait for child
    waitpid(pid, &status, 0);
    assert(status == 0);
  }
  cx_close(cx_sel_0);
  cx_sel(CX_LEGACY);

}

#define SIZE_0 256
#define SIZE_1 128
#define NTHREAD_0 16
#define NTHREAD_1 4

int** alloc_mat(int m, int n) {
  int **matrix = (int **)calloc(n, sizeof(int*));
  for(int i = 0; i < n; i++) {
      matrix[i] = (int *)calloc(m, sizeof(int));
  }
  return matrix;
}

int **gen_mat(int m, int n) 
{
    int **matrix = alloc_mat(m, n);
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < m; j++){
            matrix[i][j] = rand() % 10;
        }
    }
    return matrix;
}

typedef struct thread_args {
  int **A;
  int **B;
  int **C;
  int m;
  int n;
  int nThreads;
  int tid;
  cx_sel_t cx_sel;
} thread_args;

thread_args** init_thread_args(int m, int n, int nThreads) {
  int **A = gen_mat(m, n);
  int **B = gen_mat(m, n);
  int **C = alloc_mat(m, n);
  thread_args **mat_args = malloc(sizeof(thread_args *) * nThreads);
  for (int i = 0; i < nThreads; i++) {
    mat_args[i] = malloc(sizeof(thread_args));
    mat_args[i]->A = A;
    mat_args[i]->B = B;
    mat_args[i]->C = C;
    mat_args[i]->m = m;
    mat_args[i]->n = n;
    mat_args[i]->nThreads = nThreads;
    mat_args[i]->tid = i;
  }
  return mat_args;
}

void scalar_matmul(int** A, int** B, int** C, int m, int n) {
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      for (int k = 0; k < m; k++) {
        C[i][j] += A[i][k] * B[k][j];
      }
    }
  }
}

void print_mat(int **A, int m, int n) {
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      printf("%d ", A[i][j]);
    }
    printf("\n");
  }
}

void* mm_worker(void *targs) {
  thread_args *args = (thread_args *)targs;
  int **A = args->A;
  int **B = args->B;
  int **C = args->C;
  int m = args->m;
  int n = args->n;
  int nThreads = args->nThreads;
  int tid = args->tid;
  cx_sel(args->cx_sel);
  for (int i = tid; i < m; i += nThreads) {
    for (int j = 0; j < n; j++) {
      for (int k = 0; k < m; k++) {
        mac(A[i][k], B[k][j]);
      }
      C[i][j] = CX_READ_STATE(0);
      reset(0, 0);
    }
  }
  return NULL;
}

void mat_check_equal(int **A, int **B, int m, int n) {
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      assert(A[i][j] == B[i][j]);
    }
  }
}

void threaded_matmul(int m, int n, int nThreads, cx_sel_t cx_sel_A0) {
    pthread_t tid[ nThreads ];
    void *ret;

    thread_args **mat_args = init_thread_args(m, n, nThreads);

    for (int i = 0; i < nThreads; i++) {
      cx_sel_t cx_sel_thread = cx_open(CX_GUID_MULACC, CX_FULL_VIRT, cx_sel_A0);
      mat_args[i]->cx_sel = cx_sel_thread;
      assert( pthread_create(&tid[i], NULL, mm_worker, (void *)mat_args[i]) == 0 );
    }

    for (int i = 0; i < nThreads; i++) {
      assert( pthread_join(tid[i], &ret) == 0 );
    }

    int **C_ref = alloc_mat(m, n);
    scalar_matmul(mat_args[0]->A, mat_args[0]->B, C_ref, m, n);
    mat_check_equal(mat_args[0]->C, C_ref, m, n);
    cx_close(cx_sel_A0);

    for (int i = 0; i < nThreads; i++) {
      cx_close(mat_args[i]->cx_sel);
    }
}

void matmul_multiP_multiT() {
  cx_error_t cx_error;
  uint cx_status;
  int32_t state_result;
  cx_sel_t cx_index, cx_index_0;

  cx_sel_t cx_sel_A0 = cx_open(CX_GUID_MULACC, CX_FULL_VIRT, -1);
  assert(cx_sel_A0 > 0);
  cx_sel(cx_sel_A0);

  pid_t pid = fork();
  assert(pid >= 0);

  if (pid == 0) { 
  // child process
    threaded_matmul(SIZE_0, SIZE_0, NTHREAD_0, cx_sel_A0);
    exit(EXIT_SUCCESS);
  
  } else {
  // parent process
    threaded_matmul(SIZE_1, SIZE_1, NTHREAD_1, cx_sel_A0);
    int status;
    // Wait for child
    waitpid(pid, &status, 0);
    assert(status == 0);
  }
  cx_sel(CX_LEGACY);

  return; 
}


int main() {
    cx_sel(CX_LEGACY);
    // test_matmul();
    // basic_multiP_multiT();
    matmul_multiP_multiT();
    printf("Finished Multi-thread Multi-process test\n");
    return 0;
}
