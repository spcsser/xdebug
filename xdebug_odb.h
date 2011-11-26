#ifndef XDEBUG_ODB_H
#define XDEBUG_ODB_H

#include "ext/standard/php_string.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

void xdebug_odb_handle_exception(zval *exception);
void xdebug_odb_handle_statement(function_stack_entry *i, char *file, int lineno);
void xdebug_var_export_json(zval **struc, xdebug_str *str, int level, int debug_zval, xdebug_var_export_options *options TSRMLS_DC);
char* xdebug_get_zval_json_value(zval *val, int debug_zval, xdebug_var_export_options *options);
void xdebug_odb_call_entry_dtor(void *elem);

char* xdebug_return_trace_assignment_json(function_stack_entry *i, char *varname, zval *retval, char *op, char *file, int fileno TSRMLS_DC);

char* return_trace_stack_frame_json(function_stack_entry* i, int fnr, int whence TSRMLS_DC);

#define return_trace_stack_frame_begin_json(i,f)  return_trace_stack_frame_json((i), (f), 0 TSRMLS_CC)
#define return_trace_stack_frame_end_json(i,f)    return_trace_stack_frame_json((i), (f), 1 TSRMLS_CC)

#endif
