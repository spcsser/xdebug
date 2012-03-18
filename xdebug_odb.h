#ifndef XDEBUG_ODB_H
#define XDEBUG_ODB_H

#include "ext/standard/php_string.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

void xdebug_odb_handle_exception(zval *exception);
void xdebug_odb_handle_statement(function_stack_entry *i, char *file, int lineno);
void xdebug_var_export_json(zval **struc, xdebug_str *str, int level, int debug_zval, xdebug_var_export_options *options TSRMLS_DC);
char* xdebug_get_zval_json_value(zval *val, int debug_zval, xdebug_var_export_options *options);

char* xdebug_return_trace_assignment_json(function_stack_entry *i, char *varname, zval *retval, zval *varval, char *op, char *file, int fileno TSRMLS_DC);

static char* return_trace_stack_frame_json(function_stack_entry* i, int fnr, int whence TSRMLS_DC);

#define return_trace_stack_frame_begin_json(i,f)  return_trace_stack_frame_json((i), (f), 0 TSRMLS_CC)
#define return_trace_stack_frame_end_json(i,f)    return_trace_stack_frame_json((i), (f), 1 TSRMLS_CC)

static char* return_trace_stack_frame_json(function_stack_entry* i, int fnr, int whence TSRMLS_DC)
{
	char *tmp_name;
	char *time;
	char *mem;
	time=xdebug_sprintf("%f", i->time - XG(start_time));
	mem=xdebug_sprintf("%lu", i->memory);
	//_sprintf(time, "%f", i->time - XG(start_time));
	//_sprintf(mem, "%lu", i->memory);

	xdebug_str str = {0, 0, NULL};
	xdebug_str dstr = {0, 0, NULL};

	xdebug_str_add(&str, "\n{\"lvl\":", 0);
	xdebug_str_add(&str, xdebug_sprintf("%d", XG(level)), 1);
	xdebug_str_add(&str, ",\"aid\":", 0);
	xdebug_str_add(&str, xdebug_sprintf("%d", fnr), 1);

	if (whence == 0) { /* start */
		tmp_name = xdebug_show_fname(i->function, 0, 0 TSRMLS_CC);

		xdebug_str_add(&str, ",\"atp\":0", 0);
		xdebug_str_add(&str, ",\"tme\":", 0);
		xdebug_str_add(&str, time, 1);
#if HAVE_PHP_MEMORY_USAGE
		xdebug_str_add(&str, ",\"mem\":", 0);
		xdebug_str_add(&str, mem, 1);
#else
		//xdebug_str_add(&str, "\t", 0);
#endif
		if(strstr(tmp_name,"->__construct")){
			i->function.type=XFUNC_NEW;
		}

		xdebug_str_add(&str, ",\"nme\":\"", 0);
		xdebug_str_add(&str, tmp_name, 0);
		xdebug_str_add(&str, "\",\"ext\":", 0);
		xdebug_str_add(&str, i->user_defined == XDEBUG_EXTERNAL ? "1" : "0", 0);
		xdebug_str_add(&str, ",\"ftp\":", 0);
		xdebug_str_add(&str, xdebug_sprintf("%d", i->function.type), 1);
		xdfree(tmp_name);

		if (i->include_filename) {
			int tmp_len;

			char *escaped;
			escaped = php_addcslashes(i->include_filename, strlen(i->include_filename), &tmp_len, 0, "\\\0..\37", 5 TSRMLS_CC);
			xdebug_str_add(&str, ",\"inc\":\"", 0);
			xdebug_str_add(&str, escaped, 0);
			xdebug_str_add(&str, "\"", 0);
			efree(escaped);
		}

		/* Filename and Lineno (9, 10) */
		xdebug_str_add(&str, ",\"fle\":\"", 0);
		xdebug_str_add(&str, i->filename, 0);
		xdebug_str_add(&str, "\",\"lne\":", 0);
		xdebug_str_add(&str, xdebug_sprintf("%d", i->lineno), 1);
		xdebug_str_add(&str, "}", 0);

		/* Nr of arguments (11) */
		//xdebug_str_add(&str, xdebug_sprintf(",\"argcount\":%d", i->varc), 1);

		/* Time to collect the data into seperate string and then file*/
		xdebug_str_add(&dstr, "\n{\"aid\":",0);
		xdebug_str_add(&dstr, xdebug_sprintf("%d",fnr),1);
		xdebug_str_add(&dstr, ",\"atp\":0,\"obj\":",0);

		/* First the object scope*/
		char *tmp_obj;
		tmp_obj = xdebug_get_zval_json_value(EG(This), i->function.type==XFUNC_NEW, NULL);
		if(tmp_obj) {
			xdebug_str_add(&dstr, tmp_obj, 1);
		} else {
			xdebug_str_add(&dstr, "{\"typ\":\"NULL\",\"id\":0}", 0);
		}
		xdebug_str_add(&dstr, ",\"arg\":[", 0);

		if (XG(collect_params) > 0 && i->varc > 0) {
			int j = 0; /* Counter */

			/* Arguments (12-...) */
			for (j = 0; j < i->varc; j++) {
				char *tmp_value;
				if (i->var[j].name) {
					xdebug_str_add(&dstr, "{\"nme\":\"$", 0);
					xdebug_str_add(&dstr, i->var[j].name, 0);
				} else {
					xdebug_str_add(&dstr, "{\"nme\":\"", 0);
					xdebug_str_add(&dstr, xdebug_sprintf("%d", j), 1);
				}
				xdebug_str_add(&dstr, "\"",0);

				//xdebug_str_add(&dstr, xdebug_sprintf(",\"ast\":\"%d\",\"val\":",fnr), 1);
				xdebug_str_add(&dstr, ",\"val\":", 0);

				tmp_value = xdebug_get_zval_json_value(i->var[j].addr, 0, NULL);

				if (tmp_value) {
					xdebug_str_add(&dstr, tmp_value, 1);
				} else {
					xdebug_str_add(&dstr, "\"???\"", 0);
				}
				xdebug_str_add(&dstr,"},",0);
			}
			xdebug_str_chop(&dstr, 1); //remove trailing colon
		}
		xdebug_str_add(&dstr, "]}", 0);

		if (fprintf(XG(tracedata_file), "%s", dstr.d) < 0) {
			fclose(XG(tracedata_file));
			XG(tracedata_file) = NULL;
		} else {
			fflush(XG(tracedata_file));
		}
		xdebug_str_free(&dstr);

		/* Trailing \n */
		//xdebug_str_add(&dstr, "]}", 0);
	} else if (whence == 1) { /* end */
		xdebug_str_add(&str, ",\"atp\":1", 0);
		xdebug_str_add(&str, ",\"tme\":", 0);
		xdebug_str_add(&str, time, 1);
#if HAVE_PHP_MEMORY_USAGE
		xdebug_str_add(&str, ",\"mem\":", 0);
		xdebug_str_add(&str, mem, 1);
		//xdebug_str_add(&str, ",\"lne\":", 0);
		//xdebug_str_add(&str, xdebug_sprintf("%d", EG(current_execute_data)->opline->lineno), 1);
		xdebug_str_add(&str, "}", 0);
#else

#endif
	}

	return str.d;
}

#endif
