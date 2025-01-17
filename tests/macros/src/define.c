//! translate_const_macros

#define TEST_FN_MACRO(x) ((x) * (x))

int test_fn_macro(int x) {
  return TEST_FN_MACRO(x);
}

#include <stddef.h>
#include <stdint.h>
typedef  uint64_t U64;
typedef  uint32_t U32;

#define TEST_CONST1 1
#define TEST_NESTED 2
#define TEST_CONST2 TEST_NESTED
#define TEST_PARENS (TEST_CONST2 + 1) * 3

int reference_define() {
  int x = TEST_CONST1;
  x += TEST_CONST2;
  if (3 < TEST_PARENS)
    x += TEST_PARENS;
  return x;
}

// Exercise an edge case where a struct initializer needs to be in an unsafe
// block
struct fn_ptrs {
  void *v;
  int (*fn1)(void);
  int (*fn2)(int);
};

typedef int (*fn_ptr_ty)(char);

struct fn_ptrs const fns = {NULL, NULL, NULL};


// Make sure we can't refer to globals in a const macro
#define GLOBAL_REF &fns
struct fn_ptrs* p = GLOBAL_REF;

#define ZSTD_STATIC_ASSERT(c) (void)sizeof(char[(c) ? 1 : -1])
#define ZSTD_WINDOWLOG_MAX_32    30
#define ZSTD_WINDOWLOG_MAX_64    31
#define ZSTD_WINDOWLOG_MAX     ((int)(sizeof(size_t) == 4 ? ZSTD_WINDOWLOG_MAX_32 : ZSTD_WINDOWLOG_MAX_64))
U64 test_zstd() {
  // This static assert was causing us trouble by somehow giving a valid
  // expression for ZSTD_WINDOWLOG_MAX which shouldn't be possible to translate
  // to a const.
  ZSTD_STATIC_ASSERT(ZSTD_WINDOWLOG_MAX <= 31);
  return ZSTD_WINDOWLOG_MAX;
}

#define inc(ptr) ({\
  (*ptr)++;\
  *ptr;\
})

// Ensure the macro generated stmt expr block is codegen'd
int stmt_expr_inc(void) {
  int a = 0;
  int* b = &a;

  // unused
  inc(b);

  // used
  return inc(b);
}

int test_switch(int x) {
  switch (x) {
  case TEST_CONST1:
    return 10;
  case TEST_NESTED:
    return 20;
  }

  return 0;
}

// This must not be translated into a const expression in rust
// even with translate_const_macros
// The generated code will simply not compile if it still does
#define TEST_CALL_MACRO (not_pure_fn())

static global = 6;
int not_pure_fn() {
    return global;
}

int test_no_const_fn_call() {
  return TEST_CALL_MACRO;
}