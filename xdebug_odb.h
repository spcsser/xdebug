#ifndef XDEBUG_ODB_H
#define XDEBUG_ODB_H

#include <sqlite3.h>

ZEND_EXTERN_MODULE_GLOBALS( xdebug)

static char* return_trace_stack_frame_json(function_stack_entry* i, int fnr, int whence TSRMLS_DC);

#define return_trace_stack_frame_begin_json(i,f)  return_trace_stack_frame_json((i), (f), 0 TSRMLS_CC)
#define return_trace_stack_frame_end_json(i,f)    return_trace_stack_frame_json((i), (f), 1 TSRMLS_CC)

static char* return_trace_stack_frame_json(function_stack_entry* i, int fnr, int whence TSRMLS_DC)
{
	char *tmp_name;
	char *tmp_val;
	xdebug_str str = {0, 0, NULL};
	xdebug_str dstr = {0, 0, NULL};

	xdebug_str_add(&str, xdebug_sprintf("\n{\"lvl\":%d", i->level), 1);
	xdebug_str_add(&str, xdebug_sprintf(",\"aid\":%d", fnr), 1);

	if (whence == 0) { /* start */
		tmp_name = xdebug_show_fname(i->function, 0, 0 TSRMLS_CC);

		xdebug_str_add(&str, ",\"atp\":0", 0);
		xdebug_str_add(&str, xdebug_sprintf(",\"tme\":%f", i->time - XG(start_time)), 1);
#if HAVE_PHP_MEMORY_USAGE
		xdebug_str_add(&str, xdebug_sprintf(",\"mem\":%lu", i->memory), 1);
#else
		//xdebug_str_add(&str, "\t", 0);
#endif
		if(strstr(tmp_name,"->__construct")){
			i->function.type=XFUNC_NEW;
		}
		xdebug_str_add(&str, xdebug_sprintf(",\"nme\":\"%s\"", tmp_name), 1);
		xdebug_str_add(&str, xdebug_sprintf(",\"ext\":%d,\"ftp\":%d", i->user_defined == XDEBUG_EXTERNAL ? 1 : 0, i->function.type), 1);
		xdfree(tmp_name);

		if (i->include_filename) {
			xdebug_str_add(&str, xdebug_sprintf(",\"inc\":\"%s\"",i->include_filename), 1);
		}

		/* Filename and Lineno (9, 10) */
		xdebug_str_add(&str, xdebug_sprintf(",\"fle\":\"%s\",\"lne\":%d}", i->filename, i->lineno), 1);

		/* Nr of arguments (11) */
		//xdebug_str_add(&str, xdebug_sprintf(",\"argcount\":%d", i->varc), 1);

		/* Time to collect the data into seperate string and then file*/
		xdebug_str_add(&dstr, xdebug_sprintf("\n{\"aid\":%d,\"atp\":0,\"obj\":",fnr),0);

		/* First the object scope*/
		char *tmp_obj;
		tmp_obj = xdebug_get_zval_json_value(EG(This), i->function.type==XFUNC_NEW, NULL);
		if(tmp_obj) {
			xdebug_str_add(&dstr, tmp_obj, 1);
		} else {
			xdebug_str_add(&dstr, "{\"typ\":\"NULL\"}", 0);
		}
		xdebug_str_add(&dstr, ",\"arg\":[", 0);

		if (XG(collect_params) > 0 && i->varc > 0) {
			int j = 0; /* Counter */

			/* Arguments (12-...) */
			for (j = 0; j < i->varc; j++) {
				char *tmp_value;
				if (i->var[j].name) {
					xdebug_str_add(&dstr, xdebug_sprintf("{\"nme\":\"$%s\"", i->var[j].name), 1);
				} else {
					xdebug_str_add(&dstr, xdebug_sprintf("{\"nme\":\"%d\"",j), 0);
				}

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
		xdebug_str_add(&str, xdebug_sprintf(",\"tme\":%f", xdebug_get_utime() - XG(start_time)), 1);
#if HAVE_PHP_MEMORY_USAGE
		xdebug_str_add(&str, xdebug_sprintf(",\"mem\":%lu}", XG_MEMORY_USAGE()), 1);
#else

#endif
	}

	return str.d;
}

#endif
