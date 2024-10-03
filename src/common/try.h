//
// Created by ran on 2024/7/1.
// https://dev.to/pauljlucas/setjmp-longjmp-and-exception-handling-in-c-1h7h
//
#ifndef ZHI_TRY_H_
#define ZHI_TRY_H_

#include <setjmp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifdef __cplusplus
// While this C library would never be used in a pure C++ program, it may be
// used in a program with both C and C++ code.
extern "C" {
#endif /* __cplusplus */

/**
 * Inorder to be able to nest try blocks, either directly in the same function,
 * or indirectly in called functions. This means we’ll need multiple jmp_buf
 * variables and a linked list linking a try block to its parent, if any.
 * We can declare a data structure to hold this information.
*/

/**
 * Restrictions
 * - The requirement of volatile variables and prohibition of VLAs still apply. There’s simply no way around these.
 *
 * - Within a try, catch, or finally block, you must never break unless it’s within your own loop or switch due to
 *   the use of the for loop in the implementation.
 *
 * - Similarly, within a try, catch, or finally block, you must never either goto outside the blocks nor return from
 *   the function. In addition to the finally block, if any, not being executed, cx_impl_try_block_head will become
 *   a dangling pointer to the defunct cx_tb variable. (Fortunately, this situation can be detected, so the library
 *   checks for it and reports the error.)
 *
 * - Within a try or catch block, continue will jump to the finally block, if any. (Perhaps this is a nice thing?)
 *
 * - For catch and throw, the () are required with no space between the catch or throw and the (.
 *
 * - When compiling your own code using the library, you have to use the compiler options -Wno-dangling-else and
 *   Wno-shadow (or equivalent for your compiler) to suppress warnings. There’s simply no way to use {} to suppress
 *   “dangling else” warnings nor use unique names for cx_tb to avoid the “no shadow” warnings and keep the “natural
 *   looking” code.
 */

#if   __STDC_VERSION__ >= 202311L
# define CX_IMPL_THREAD_LOCAL     thread_local
#elif __STDC_VERSION__ >= 201112L
#define CX_IMPL_THREAD_LOCAL     _Thread_local
#elif defined( _MSC_VER )
#define CX_IMPL_THREAD_LOCAL     __declspec( thread )
#elif defined( __GNUC__ )
#define CX_IMPL_THREAD_LOCAL     __thread
#else
#error "Don't know how to declare thread-local variables on this platform."
#endif

#ifndef _Noreturn
# ifdef __cplusplus
#   if __cplusplus >= 201103L
#     define _Noreturn                [[noreturn]]
#   else
#     define _Noreturn                /* nothing */
#   endif
# else /* must be C */
#   if __STDC_VERSION__ >= 202311L
#     define _Noreturn                [[noreturn]]
#   else
#     /* _Noreturn is fine as-is. */

#   endif
# endif /* __cplusplus */
#endif /* _Noreturn */

#if defined(__GNUC__) || defined(__clang__)
#define unreachable() __builtin_unreachable()
#elif defined(_MSC_VER)
#define unreachable() __assume(0)
#else
#define unreachable() assert(0 && "unreachable code")
#endif


/**
 * Matches any exception ID.
 */
#define CX_XID_ANY  0

struct cx_exception {
  char const *file; // The file where exception was thrown
  int line; // The line number where exception was thrown
  int thrown_xid; // The exception ID
  void *user_data; // Optional user data pass via throw
};
typedef struct cx_exception cx_exception_t;

/**
 * The signature for a "terminate handler" function that is called by
 * cx_terminate().
 *
 */
typedef void(*cx_terminate_handler_t)(cx_exception_t const *cex);

/**
 * The signature for a "exception ID matcher" function that is called by
 * #cx_catch clauses to determine whether the thrown exception matches a
 * particular exception.
 *
 * Since C doesn't have inheritance, there's no way to create exception
 * hierarchies.  As a substitute, you can create numeric groups and catch _any_
 * exception in a group.
 *
 * For example, if you have:
 *  ```c
 *  #define EX_FILE_ANY         0x0100
 *  #define EX_FILE_IO_ERROR    (EX_FILE_ANY | 0x01)
 *  #define EX_FILE_NOT_FOUND   (EX_FILE_ANY | 0x02)
 *  #define EX_FILE_PERMISSION  (EX_FILE_ANY | 0x03)
 *  // ...
 *
 *  bool my_cx_xid_matcher( int thrown_xid, int catch_xid ) {
 *    if ( (catch_xid & 0x00FF) == 0x00 )
 *      thrown_xid &= 0xFF00;
 *    return thrown_xid == catch_xid;
 *  }
 *  ```
 * then you can do:
 *  ```c
 *  cx_set_xid_matcher( &my_cx_xid_matcher );
 *  cx_try {
 *    // ...
 *  }
 *  cx_catch( EX_FILE_NOT_FOUND ) {
 *    // handle file-not-found specifically
 *  }
 *  cx_catch( EX_FILE_ANY ) {
 *    // handle any other file error
 *  }
 *  ```
 *
 * @param thrown_xid The thrown exception ID.
 * @param catch_xid The exception ID to match \a thrown_xid against.
 * @return Returns `true` only if \a thrown_xid matches \a catch_xid.
 *
 * @sa cx_set_xid_matcher()
 */
typedef bool (*cx_xid_matcher_t)(int thrown_xid, int catch_xid);

/**
 * Gets the current exception, if any.
 *
 * @return If an exception is in progress, returns a pointer to it; otherwise
 * returns NULL.
 *
 * @sa cx_user_data()
 */
cx_exception_t *cx_current_exception(void);

/**
 * Begins a "try" block to be followed by zero or more #cx_catch blocks and
 * zero or one #cx_finally block.
 *
 * @warning Any variables declared outside the <code>%cx_try</code> block that
 * are modified inside the block and used again outside the block _must_ be
 * declared `volatile`:
 *  ```c
 *  int volatile n = 0;
 *  cx_try {
 *    // ...
 *    ++n;
 *  }
 *  cx_catch( EX_FILE_NOT_FOUND ) {
 *    // ...
 *  }
 *  printf( "n = %d\n", n );
 *  ```
 *
 * @warning Within a function that uses a <code>%cx_try</code> block, you must
 * _never_ use variable-length arrays.
 *
 * @warning Within a <code>%cx_try</code> block, you must _never_ `break`
 * unless it's within your own loop or `switch` due to the way in which
 * <code>%cx_try</code> is implemented.  For example, do _not_ do something
 * like:
 *  ```c
 *  while ( true ) {
 *    cx_try {
 *      // ...
 *      if ( some_condition )
 *        break;                        // Does NOT break out of while loop.
 *    }
 *    // ...
 *  }
 *  ```
 * If possible, put the `while` inside the <code>%cx_try</code> instead:
 *  ```c
 *  cx_try {
 *    while ( true ) {
 *      // ...
 *      if ( some_condition )
 *        break;                        // Breaks out of while loop.
 *    }
 *    // ...
 *  }
 *  ```
 *
 * @warning Within a <code>%cx_try</code> block, you must _never_ `goto`
 * outside the block nor `return` from the function. See #cx_cancel_try().
 *
 * @warning Within a <code>%cx_try</code> block, `continue` will cause the
 * block to exit immediately and jump to the #cx_finally block, if any.
 *
 * @sa #cx_cancel_try()
 * @sa #cx_catch()
 * @sa #cx_finally
 * @sa #cx_throw()
 */
#define cx_try                                             \
  for ( cx_impl_try_block_t cx_tb = {.try_file = __FILE__, \
                                     .try_line = __LINE__, \
                                     .state=CX_IMPL_INIT}; \
        cx_impl_try_condition( &cx_tb ); )              \
    if ( cx_tb.state != CX_IMPL_FINALLY )               \
      if ( setjmp( cx_tb.env ) == 0 )


/**
 * Begins a "catch" block possibly catching an exception and executing the code
 * in the block.
 *
 * @remarks
 * @parblock
 * This can be used in one of two ways:
 *
 *  1. With an exception ID:
 *      @code
 *      cx_catch( EX_FILE_NOT_FOUND ) {
 *      @endcode
 *     that catches the given exception ID.
 *
 *  2. With no exception ID:
 *      @code
 *      cx_catch() {
 *      @endcode
 *     that catches any exception like the C++ `...` does.
 * @endparblock
 *
 * @note Unlike the C++ equivalent, the `()` are _required_ with _no_ space
 * between the <code>%cx_catch</code> and the `(`.
 *
 * @note For a given #cx_try block, there may be zero or more
 * <code>%cx_catch</code> blocks.  However, if there are zero, there _must_ be
 * one #cx_finally block.  Multiple <code>%cx_catch</code> blocks are tried in
 * the order declared and at most one <code>%cx_catch</code> block will be
 * matched.
 *
 * @warning Similarly to a #cx_try block, within a <code>%cx_catch</code>
 * block, you must _never_ `break` unless it's within your own loop or `switch`
 * due to the way in which <code>%cx_catch</code> is implemented.
 *
 * @warning Within a <code>%cx_catch</code> block, you must _never_ `goto`
 * outside the block nor `return` from the function. See #cx_cancel_try().
 *
 * @warning Within a <code>%cx_catch</code> block, `continue` will cause the
 * block to exit immediately and jump to the #cx_finally block, if any.
 *
 * @sa #cx_cancel_try()
 * @sa #cx_finally
 * @sa #cx_set_xid_matcher()
 * @sa #cx_throw()
 * @sa #cx_try
 */
#define cx_catch(...)             CX_IMPL_DEF_ARGS(CX_IMPL_CATCH_, __VA_ARGS__)


/**
 * Throws an exception.
 *
 * @remarks
 * @parblock
 * This can be called in one of three ways:
 *
 *  1. With an exception ID:
 *     @code
 *      cx_throw( EX_FILE_NOT_FOUND );
 *     @endcode
 *     that throws a new exception.  It may be any non-zero value.
 *
 *  2. With an exception ID and user-data:
 *     @code
 *      cx_throw( EX_FILE_NOT_FOUND, path );
 *     @endcode
 *     that throws a new exception passing `path` as a `void*` argument.
 *
 *  3. Without arguments from within either a #cx_catch or #cx_finally block:
 *     @code
 *      cx_throw();
 *     @endcode
 *     that rethrows the most recent exception with the same user-data, if any.
 *     If no exception has been caught, calls cx_terminate().
 * @endparblock
 *
 * @note Unlike C++, the `()` are _required_ with _no_ space between the
 * <code>%cx_throw</code> and the `(`.
 *
 * @warning If user-data is supplied and it's an actual pointer, the object to
 * which it points must _not_ be a local variable that will go out of scope.
 *
 * @warning An exception that is thrown but not caught will result in
 * cx_terminate() being called.
 *
 * @sa #cx_set_terminate()
 * @sa #cx_try
 * @sa #cx_catch()
 * @sa #cx_finally
 */
#define cx_throw(...)             CX_IMPL_DEF_ARGS(CX_IMPL_THROW_, __VA_ARGS__)


/**
 * Cancels a current #cx_try, #cx_catch, or #cx_finally block in the current
 * scope allowing you to then safely `break`, `goto` out of the block, or
 * `return` from the function:
 *  ```c
 *  cx_try {
 *    // ...
 *    if ( some_condition ) {
 *        cx_cancel_try();
 *        return;
 *    }
 *    // ...
 *  }
 *  ```
 *
 * @warning After calling <code>%cx_cancel_try()</code>, you _must_ exit the
 * block via `break`, `goto`, or `return`.
 *
 * @warning Any uncaught exception will _not_ be rethrown.
 *
 * @warning Calling <code>%cx_cancel_try()</code> from a #cx_try or #cx_catch
 * block will result in the #cx_finally block, if any, _not_ being executed.
 * An alternative that still executes the #cx_finally block is:
 *  ```c
 *  bool volatile return_early = false;
 *  cx_try {
 *    // ...
 *    if ( some_condition ) {
 *      return_early = true;
 *      continue;                       // Jumps to finally block, if any.
 *    }
 *    // ...
 *  }
 *  cx_finally {
 *    // ...
 *  }
 *  if ( return_early )
 *    return;
 *  // ...
 *  ```
 */
#define cx_cancel_try()           cx_impl_cancel_try( &cx_tb )


typedef struct cx_impl_try_block cx_impl_try_block_t;

/**
 * Default exception matcher function.
 *
 * @param thrown_xid The thrown exception ID.
 * @param catch_xid The exception ID to match \a thrown_xid against.
 * @return Returns `true` only if \a thrown_xid equals \a catch_xid.
 */
static bool cx_impl_default_xid_matcher(int thrown_xid, int catch_xid) {
  return thrown_xid == catch_xid;
}

/**
 * Gets the user-data, if any, associated with the current exception, if any.
 *
 * @return If an exception is in progress, returns the user-data; otherwise
 * returns NULL.
 *
 * @sa cx_current_exception()
 */
inline void *cx_user_data(void) {
  cx_exception_t *const cex = cx_current_exception();
  return cex != NULL ? cex->user_data : NULL;
}

extern inline void *cx_user_data(void);

/**
 * Default terminate handler.
 *
 * @param cex A pointer to a cx_exception object that has information about the
 * exception that was thrown.
 */
_Noreturn
static void cx_impl_default_terminate_handler(cx_exception_t const *cex) {
  assert(cex != NULL);
  fprintf(stderr,
          "%s:%d: unhandled exception %d (0x%X)\n",
          cex->file, cex->line,
          cex->thrown_xid, (unsigned) cex->thrown_xid
  );
  abort();
}

static CX_IMPL_THREAD_LOCAL cx_impl_try_block_t *cx_impl_try_block_head;

/**
 * Current exception matcher function.
 */
static cx_xid_matcher_t cx_xid_matcher = &cx_impl_default_xid_matcher;

// See <https://stackoverflow.com/a/11742317/99089>

#define CX_IMPL_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N
#define CX_IMPL_COMMA(...)        ,
#define CX_IMPL_REV_SEQ_N         9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define CX_IMPL_HAS_COMMA_N       1, 1, 1, 1, 1, 1, 1, 1, 0, 0
#define CX_IMPL_HAS_COMMA(...) \
  CX_IMPL_NARG_( __VA_ARGS__, CX_IMPL_HAS_COMMA_N )

#define CX_IMPL_NARG_(...)        CX_IMPL_ARG_N( __VA_ARGS__ )
#define CX_IMPL_NARG(...)                               \
  CX_IMPL_NARG_HELPER1(                                 \
    CX_IMPL_HAS_COMMA( __VA_ARGS__ ),                   \
    CX_IMPL_HAS_COMMA( CX_IMPL_COMMA __VA_ARGS__ () ),  \
    CX_IMPL_NARG_( __VA_ARGS__, CX_IMPL_REV_SEQ_N ) )

#define CX_IMPL_NARG_HELPER1(A, B, N)   CX_IMPL_NARG_HELPER2(A, B, N)
#define CX_IMPL_NARG_HELPER2(A, B, N)   CX_IMPL_NARG_HELPER3_ ## A ## B(N)
#define CX_IMPL_NARG_HELPER3_01(N)    0
#define CX_IMPL_NARG_HELPER3_00(N)    1
#define CX_IMPL_NARG_HELPER3_11(N)    N

#define CX_IMPL_NAME2(A, B)        CX_IMPL_NAME2_HELPER(A,B)
#define CX_IMPL_NAME2_HELPER(A, B) A##B

#define CX_IMPL_DEF_ARGS(PREFIX, ...) \
  CX_IMPL_NAME2(PREFIX, CX_IMPL_NARG(__VA_ARGS__))(__VA_ARGS__)

#define CX_IMPL_CATCH_0()         CX_IMPL_CATCH_1( CX_XID_ANY )
#define CX_IMPL_CATCH_1(XID)      else if ( cx_impl_catch( (XID), &cx_tb ) )

#define CX_IMPL_THROW_0()         CX_IMPL_THROW_1( cx_tb.thrown_xid )
#define CX_IMPL_THROW_1(XID)      CX_IMPL_THROW_2( (XID), cx_user_data() )
#define CX_IMPL_THROW_2(XID, DATA) \
  cx_impl_throw( __FILE__, __LINE__, (XID), (void*)(DATA) )

enum cx_impl_state {
  CX_IMPL_INIT,       // Initial state.
  CX_IMPL_TRY,        // No exception thrown.
  CX_IMPL_THROWN,     // Exception thrown, but uncaught.
  CX_IMPL_CAUGHT,     // Exception caught.
  CX_IMPL_FINALLY,    // Final block
};
typedef enum cx_impl_state cx_impl_state_t;

struct cx_impl_try_block {
  char const *try_file;     // File containing the #try
  int try_line;             // Line within try file
  jmp_buf env;              //Jump buffer.
  cx_impl_try_block_t *parent; // Enclosing parent try
  cx_impl_state_t state; // current state
  int thrown_xid;  // Thrown exception, if any.
  int caught_xid;  // Caught exception, if any.
};

static CX_IMPL_THREAD_LOCAL cx_exception_t cx_impl_exception;

/**
 * Current terminate handler.
 */
static cx_terminate_handler_t cx_impl_terminate_handler =
    &cx_impl_default_terminate_handler;

_Noreturn
static void cx_terminate(void) {

  assert(cx_impl_terminate_handler != NULL);
  (*cx_impl_terminate_handler)(&cx_impl_exception);
  unreachable();
}

_Noreturn
static void cx_impl_do_throw(void) {
  if (cx_impl_try_block_head == NULL)
    cx_terminate();
  cx_impl_try_block_head->state = CX_IMPL_THROWN;
  cx_impl_try_block_head->thrown_xid = cx_impl_exception.thrown_xid;
  longjmp(cx_impl_try_block_head->env, 1);
}

_Noreturn
static void cx_impl_throw(char const *file, int line, int xid, void *user_data) {
  cx_impl_exception = (cx_exception_t) {
      .file = file,
      .line = line,
      .thrown_xid = xid,
      .user_data = user_data
  };
  cx_impl_do_throw();
}
/**
 * Asserts that \a tb is the current \ref cx_impl_try_block.
 *
 * @param tb A pointer to the \ref cx_impl_try_block to check.
 */
static void cx_impl_assert_try_block(cx_impl_try_block_t const *tb) {
  assert(tb != NULL);
  if (tb == cx_impl_try_block_head)
    return;
  fprintf(stderr,
          "%s:%d: \"try\" not exited cleanly via "
          "\"break\", \"goto\", or \"return\"\n",
          cx_impl_try_block_head->try_file,
          cx_impl_try_block_head->try_line
  );
  abort();
}
static bool cx_impl_try_condition(cx_impl_try_block_t *tb) {
  switch (tb->state) {
    case CX_IMPL_INIT: {
      tb->parent = cx_impl_try_block_head;
      cx_impl_try_block_head = tb;
      tb->state = CX_IMPL_TRY;
      return true;
    }
    case CX_IMPL_CAUGHT: {
      tb->thrown_xid = 0;      // Reset for finally case.
    }// fallthrough
    case CX_IMPL_TRY:
    case CX_IMPL_THROWN: {
      cx_impl_assert_try_block(tb);
      tb->state = CX_IMPL_FINALLY;
      return true;
    }
    case CX_IMPL_FINALLY: {
      cx_impl_assert_try_block(tb);
      cx_impl_try_block_head = tb->parent;
      if (tb->thrown_xid != 0)
        cx_impl_do_throw();    // Rethrow uncaught exception.
      cx_impl_exception = (cx_exception_t) {0};
      return false;
    }
  } // switch
}

//#define throw(XID) \
//  cx_impl_throw( __FILE__, __LINE__, (XID),NULL )

static bool cx_impl_catch(int catch_xid, cx_impl_try_block_t *tb) {
  assert(tb != NULL);
  assert(tb->state == CX_IMPL_THROWN);
  if (tb->caught_xid == tb->thrown_xid) {
    //
    // This can happen when the same exception is thrown from a "catch" block.
    // For example, given:
    //
    //      try {
    //        try {
    //          throw( XID_1 );         // 1
    //        }
    //        catch( XID_1 ) {          // 2
    //          throw();                // 3
    //        }
    //        finally {                 // 4
    //          // ...
    //        }
    //      }
    //      catch( XID_1 ) {            // 5
    //        // ...
    //      }
    //
    // we want the flow to go from 1 to 5 in sequence. If this check were not
    // here, we'd loop endlessly between 2-3.
    //
    // Once an exception is caught at the current try/catch nesting level, it
    // can never be recaught at the same level.  By returning "false" for all
    // catches at the current level, we execute the "finally" block, if any, of
    // the current level; then the CX_IMPL_FINALLY state will pop us up to the
    // parent level, if any, where this check will fail (because the parent's
    // caught_xid will be 0) and we can possibly recatch the exception at the
    // parent level.
    //
    return false;
  }

  if (catch_xid != CX_XID_ANY) {
    assert(cx_xid_matcher != NULL);
    if (!(*cx_xid_matcher)(tb->thrown_xid, catch_xid))
      return false;
  }

  tb->state = CX_IMPL_CAUGHT;
  tb->caught_xid = tb->thrown_xid;
  return true;
}

cx_exception_t *cx_current_exception(void) {
  return cx_impl_exception.file == NULL ? NULL : &cx_impl_exception;
}

/**
 * Sets the current \ref cx_xid_matcher_t.
 *
 * @param fn The new \ref cx_xid_matcher_t or NULL to use the default.
 * @return Returns the previous \ref cx_xid_matcher_t, if any.
 *
 * @sa cx_get_xid_matcher()
 */
cx_xid_matcher_t cx_set_xid_matcher(cx_xid_matcher_t fn);
/**
 * Gets the current \ref cx_xid_matcher_t, if any.
 *
 * @return Returns said function or NULL if none.
 *
 * @sa cx_set_xid_matcher()
 */
cx_xid_matcher_t cx_get_xid_matcher(void);

/**
 * Sets the current \ref cx_terminate_handler_t.
 *
 * @param fn The new \ref cx_terminate_handler_t or NULL to use the default.
 * @return Returns the previous \ref cx_terminate_handler_t, if any.
 *
 * @warning Terminate handler functions _must not_ return.
 *
 * @sa cx_get_terminate()
 * @sa cx_terminate()
 */
cx_terminate_handler_t cx_set_terminate(cx_terminate_handler_t fn);

/**
 * Gets the current \ref cx_terminate_handler_t, if any.
 *
 * @return Returns said handler or NULL if none.
 *
 * @sa cx_set_terminate()
 * @sa cx_terminate()
 */
cx_terminate_handler_t cx_get_terminate(void);

/**
 * Implements #cx_cancel_try().
 *
 * @param tb A pointer to the current \ref cx_impl_try_block.
 *
 * @sa #cx_cancel_try()
 */
void cx_impl_cancel_try( cx_impl_try_block_t const *tb );


void cx_impl_cancel_try( cx_impl_try_block_t const *tb ) {
  assert( tb != NULL );
  cx_impl_assert_try_block( tb );
  cx_impl_try_block_head = tb->parent;
}

cx_xid_matcher_t cx_get_xid_matcher(void) {
  return cx_xid_matcher == &cx_impl_default_xid_matcher ? NULL : cx_xid_matcher;
}

cx_xid_matcher_t cx_set_xid_matcher(cx_xid_matcher_t fn) {
  cx_xid_matcher_t const rv = cx_get_xid_matcher();
  cx_xid_matcher = fn == NULL ? &cx_impl_default_xid_matcher : fn;
  return rv;
}

cx_terminate_handler_t cx_get_terminate(void) {
  return cx_impl_terminate_handler == &cx_impl_default_terminate_handler ?
         NULL : cx_impl_terminate_handler;
}

cx_terminate_handler_t cx_set_terminate(cx_terminate_handler_t fn) {
  cx_terminate_handler_t const rv = cx_get_terminate();
  cx_impl_terminate_handler = fn == NULL ?
                              &cx_impl_default_terminate_handler : fn;
  return rv;
}

//#define catch(XID) \
//  else if ( cx_impl_catch( (XID), &cx_tb ) )

#define cx_finally                                \
    else /* setjmp() != 0 */ /* do nothing */; \
  else /* cx_tb.state == CX_IMPL_FINALLY */

#ifdef __cplusplus
} // extern "C"
#endif /* __cplusplus */

#endif