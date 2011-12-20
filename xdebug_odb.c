#include "php_xdebug.h"
#include "xdebug_private.h"
#include "xdebug_str.h"
#include "xdebug_tracing.h"
#include "xdebug_var.h"
#include "xdebug_odb.h"
#include "ext/spl/php_spl.h"

//ZEND_EXTERN_MODULE_GLOBALS(xdebug)

static char *xdebug_print_hash_value(zval *struc){

	if(!struc){
		return xdstrdup("0");
	}
	char *hash;
	//hash = xdmalloc(33);
	//if(Z_TYPE_P(struc) == IS_OBJECT){
		//php_spl_object_hash(struc, hash TSRMLS_CC);
	//}else{
	hash=xdebug_sprintf("%lu", (unsigned long int) &struc->value);
	//}
	return hash;
}

static int xdebug_object_element_export_json(zval **zv XDEBUG_ZEND_HASH_APPLY_TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key);

static void odb_export_class_vars(char *class_name, xdebug_str *str, int level, xdebug_var_export_options *options){
 	zend_class_entry **zce;
        zend_lookup_class(class_name, strlen(class_name), &zce TSRMLS_CC);
	if(!zce || !(*zce)){
		return;
	}

	char *id;
	zval **tmpVal=0;

	id=xdebug_sprintf("%lu", (unsigned long int) *zce);
	// only add id if not known and no forcing
	if (zend_hash_find(&XG(known_values), class_name, strlen(class_name)+1, (void**)&tmpVal) != FAILURE) {
		xdebug_str_add(str, "{\"cid\":",0);
		xdebug_str_add(str, id, 1);
		xdebug_str_add(str, "}",0);
	} else {
		zend_hash_add(&XG(known_values), class_name, strlen(class_name)+1, (void**)&tmpVal, sizeof(zval*), NULL);

        	xdebug_str_add(str, "{\"typ\":\"class\",\"nme\":\"",0);
	        xdebug_str_add(str, class_name, 0);
        	xdebug_str_add(str, "\",\"cid\":", 0);
	        xdebug_str_add(str, id, 1);
        	xdebug_str_add(str, ",\"val\":[", 0);

	        if (level <= options->max_depth) {
        	        options->runtime[level].current_element_nr = 0;
                	options->runtime[level].start_element_nr = 0;
	                options->runtime[level].end_element_nr = options->max_children;
        	        zend_hash_apply_with_arguments((*zce)->static_members XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) xdebug_object_element_export_json, 6, level, str, 0, options, class_name, " static");
                	zend_hash_apply_with_arguments(&((*zce)->constants_table) XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) xdebug_object_element_export_json, 6, level, str, 0, options, class_name, " const");
	                /* Remove the ", " at the end of the string */
        	        if((*zce)->static_members->nNumOfElements > 0 || (*zce)->constants_table.nNumOfElements > 0){
                        	xdebug_str_chop(str, 1);
                	}
       		}
	        xdebug_str_add(str, "]}", 0);
	}	
}

static void printout_to_file(xdebug_str *dstr, int free, int data){
	FILE *tracedata_file= (data==0 ? XG(trace_file) : XG(tracedata_file));
	if (fprintf(tracedata_file, "%s", dstr->d) < 0) {
               fclose(tracedata_file);
               tracedata_file = NULL;
        } else {
               fflush(tracedata_file);
        }
	if(dstr && free){
	        xdebug_str_free(dstr);
	}

}

#define T(offset) (*(temp_variable *)((char *) Ts + offset))

zend_class_entry *xdebug_odb_get_class_entry(zend_execute_data *zdata, int node_type, XDEBUG_ZNODE *node, temp_variable *Ts, int *is_var){
	switch(node_type){
		case IS_CONST:
			fprintf(stderr, "\nIS_CONST\n");
			break;
		case IS_TMP_VAR:
			fprintf(stderr, "\nIS_TMP_VAR\n");
			break;
		case IS_VAR:
			*is_var = 1;
#if PHP_VERSION_ID >= 50399
			if (T(node->var).class_entry) {
				return T(node->var).class_entry;
#else
			if (T(node->u.var).class_entry){
				return T(node->u.var).class_entry;
#endif
			} else {
				fprintf(stderr, "\nIS_VAR\n");
			}
			break;
		case IS_CV:
			fprintf(stderr, "\nIS_CV\n");
			break;
		case IS_UNUSED:
			fprintf(stderr, "\nIS_UNUSED\n");
			break;
		default:
			fprintf(stderr, "\ndefault %d\n", node_type);
			break;
	}
	return NULL;
}

void xdebug_odb_call_entry_dtor(void *elem)
{
	zval *struc = (zval*)elem;
	Z_DELREF_P(struc);
}

static void print_trace_basics(xdebug_str *str, int fnr, function_stack_entry *i,  int incr_level){
	char *time;
        char *mem;
	
	if(i){
	        time=xdebug_sprintf("%f", i->time - XG(start_time));
        	mem=xdebug_sprintf("%lu", i->memory);
	}else{
		time=xdebug_sprintf("%f", xdebug_get_utime() - XG(start_time));
		mem=xdebug_sprintf("%lu", XG_MEMORY_USAGE());
	}

	xdebug_str_add(str, "\n{\"lvl\":", 0);
        xdebug_str_add(str, xdebug_sprintf("%d", incr_level ? ++XG(level) : XG(level)), 1);
        xdebug_str_add(str, ",\"aid\":", 0);
        xdebug_str_add(str, xdebug_sprintf("%d", fnr), 1);		

        xdebug_str_add(str, ",\"tme\":", 0);
        xdebug_str_add(str, time, 1);
#if HAVE_PHP_MEMORY_USAGE
        xdebug_str_add(str, ",\"mem\":", 0);
        xdebug_str_add(str, mem, 1);
#else
        //xdebug_str_add(&str, "\t", 0);
#endif

}

char* return_trace_stack_frame_json(function_stack_entry* i, int fnr, int whence TSRMLS_DC)
{
	char *tmp_name;
	xdebug_str str = {0,0,NULL};
	xdebug_str dstr = {0,0,NULL};
	/*
	char *time;
	char *mem;
	time=xdebug_sprintf("%f", i->time - XG(start_time));
	mem=xdebug_sprintf("%lu", i->memory);

	xdebug_str str = {0, 0, NULL};
	xdebug_str dstr = {0, 0, NULL};

	xdebug_str_add(&str, "\n{\"lvl\":", 0);
	xdebug_str_add(&str, xdebug_sprintf("%d", XG(level)), 1);
	xdebug_str_add(&str, ",\"aid\":", 0);
	xdebug_str_add(&str, xdebug_sprintf("%d", fnr), 1);
	*/
	print_trace_basics(&str,i->fn_nr,i,0);
	if (whence == 0) { /* start */
		tmp_name = xdebug_show_fname(i->function, 0, 0 TSRMLS_CC);

		xdebug_str_add(&str, ",\"atp\":0", 0);
/*		xdebug_str_add(&str, ",\"tme\":", 0);
		xdebug_str_add(&str, time, 1);
#if HAVE_PHP_MEMORY_USAGE
		xdebug_str_add(&str, ",\"mem\":", 0);
		xdebug_str_add(&str, mem, 1);
#else
		//xdebug_str_add(&str, "\t", 0);
#endif
*/
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

		/* Time to collect the data into seperate string and then file*/
		xdebug_str_add(&dstr, "\n{\"aid\":",0);
		xdebug_str_add(&dstr, xdebug_sprintf("%d",fnr),1);
		xdebug_str_add(&dstr, ",\"atp\":0,\"obj\":",0);

		/* First the object scope*/
		xdebug_str tmp_obj={0,0,NULL};
		if(EG(This)){
			xdebug_str_add(&tmp_obj, xdebug_get_zval_json_value(EG(This), strstr(tmp_name,"->__construct")!=NULL, NULL), 1);
		} else if(i->function.class){
			xdebug_var_export_options *options;
			options = xdebug_var_export_options_from_ini(TSRMLS_C);
			options->max_children = 1048576;
			options->max_data = 1073741824;
			options->max_depth = 4096;
			options->runtime = (xdebug_var_runtime_page*) xdmalloc((options->max_depth + 1) * sizeof(xdebug_var_runtime_page));
			odb_export_class_vars(i->function.class,&tmp_obj,i->level,options);
			xdfree(options->runtime);
			xdfree(options);
		}

		if(tmp_obj.d && tmp_obj.l>0) {
			xdebug_str_add(&dstr, tmp_obj.d, 1);
		} else {
			xdebug_str_add(&dstr, "{\"typ\":\"NULL\",\"id\":0}", 0);
		}
		xdebug_str_add(&dstr, ",\"arg\":[", 0);

		if (i->varc > 0) {
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

		printout_to_file(&dstr,1,1);
		/*
		if (fprintf(XG(tracedata_file), "%s", dstr.d) < 0) {
			fclose(XG(tracedata_file));
			XG(tracedata_file) = NULL;
		} else {
			fflush(XG(tracedata_file));
		}
		xdebug_str_free(&dstr);
		*/

	} else if (whence == 1) { /* end */

		xdebug_str_add(&str, ",\"atp\":1}", 0);
/*		xdebug_str_add(&str, ",\"tme\":", 0);
		xdebug_str_add(&str, time, 1);
#if HAVE_PHP_MEMORY_USAGE
		xdebug_str_add(&str, ",\"mem\":", 0);
		xdebug_str_add(&str, mem, 1);
#else

#endif
*/
		/*TODO check how it is possible to include the return line*/
		//xdebug_str_add(&str, ",\"lne\":", 0);
		//xdebug_str_add(&str, xdebug_sprintf("%d", EG(current_execute_data)->opline->lineno), 1);
		//xdebug_str_add(&str, "}", 0);
	}

	return str.d;
}

void xdebug_odb_handle_statement(function_stack_entry *i, char *file, int lineno) {
	xdebug_str str = { 0, 0, NULL };
	xdebug_str dstr = { 0, 0, NULL };
	char *tmp_name;

	xdebug_str_add(&str, "\n{\"lvl\":", 0);
	xdebug_str_add(&str, xdebug_sprintf("%d", XG(level)), 1);
	xdebug_str_add(&str, ",\"aid\":", 0);
	xdebug_str_add(&str, xdebug_sprintf("%d", ++XG(function_count)), 1);
	xdebug_str_add(&str, ",\"atp\":4", 0);
	xdebug_str_add(&str, ",\"tme\":", 0);
	xdebug_str_add(&str, xdebug_sprintf("%f", xdebug_get_utime() - XG(start_time)), 1);
	xdebug_str_add(&str, ",\"mem\":", 0);
	xdebug_str_add(&str, xdebug_sprintf("%lu", XG_MEMORY_USAGE()), 1);

	tmp_name = xdebug_show_fname(i->function, 0, 0 TSRMLS_CC);
	xdebug_str_add(&str, xdebug_sprintf(",\"nme\":\""), 0);
	xdebug_str_add(&str, tmp_name, 0);
	xdfree(tmp_name);

	xdebug_str_add(&str, ",\"ftp\":", 0);
	xdebug_str_add(&str, xdebug_sprintf("%d", i->function.type), 1);

	xdebug_str_add(&str, ",\"fle\":\"", 0);
	xdebug_str_add(&str, file, 0);
	xdebug_str_add(&str, "\",\"lne\":", 0);
	xdebug_str_add(&str, xdebug_sprintf("%d", lineno), 1);
	xdebug_str_add(&str, "}", 0);

	xdebug_str_add(&dstr, "\n{\"aid\":", 0);
	xdebug_str_add(&dstr, xdebug_sprintf("%d", XG(function_count)), 1);
	xdebug_str_add(&dstr, ",\"atp\":4", 0);
	xdebug_str_add(&dstr, ",\"arg\":[", 0);

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
			xdebug_str_add(&dstr, tmp_value, 0);
		} else {
			xdebug_str_add(&dstr, "\"???\"", 0);
		}
		xdebug_str_add(&dstr,"},",0);
	}
	xdebug_str_chop(&dstr, 1); //remove trailing colon
	xdebug_str_add(&dstr, "]}", 0);

	printout_to_file(&str, 1, 0);
	printout_to_file(&dstr, 1, 1);
	/*
	if (fprintf(XG(trace_file), "%s", str.d) < 0) {
		fclose(XG(trace_file));
		XG( trace_file) = NULL;
	} else {
		fflush(XG(trace_file));
	}

	if (fprintf(XG(tracedata_file), "%s", dstr.d) < 0) {
		fclose(XG(tracedata_file));
		XG( tracedata_file) = NULL;
	} else {
		fflush(XG(tracedata_file));
	}*/
}

void xdebug_odb_handle_error(int type, const char *error_filename, const uint error_lineno, error_handling_t error_handling, char *error_type_str, char *error_message, zend_class_entry *exception_class) {
	xdebug_str str = { 0, 0, NULL};
	xdebug_str dstr = { 0, 0, NULL};

	char *fn_nr=xdebug_sprintf("%d", ++XG(function_count));

        xdebug_str_add(&str, "\n{\"lvl\":", 0);
        xdebug_str_add(&str, xdebug_sprintf("%d", XG(level)), 1);
        xdebug_str_add(&str, ",\"aid\":", 0);
        xdebug_str_add(&str, fn_nr, 0);
        xdebug_str_add(&str, ",\"atp\":7", 0);
        xdebug_str_add(&str, ",\"tme\":", 0);
        xdebug_str_add(&str, xdebug_sprintf("%f", xdebug_get_utime() - XG(
                        start_time)), 1);
        xdebug_str_add(&str, ",\"mem\":", 0);
        xdebug_str_add(&str, xdebug_sprintf("%lu", XG_MEMORY_USAGE()), 1);
        xdebug_str_add(&str, ",\"nme\":\"", 0);
	xdebug_str_add(&str, error_type_str, 0);

	xdebug_str_add(&str, "\",\"fle\":\"", 0);
	xdebug_str_add(&str, xdebug_sprintf("%s",error_filename), 1);
	xdebug_str_add(&str, "\",\"lne\":", 0);
	xdebug_str_add(&str, xdebug_sprintf("%d",error_lineno), 1);
	xdebug_str_add(&str, "}", 0);

        xdebug_str_add(&dstr, "\n{\"aid\":", 0);
        xdebug_str_add(&dstr, fn_nr, 1);
        xdebug_str_add(&dstr, ",\"atp\":7,\"msg\":\"", 0);
	xdebug_str_add(&dstr, error_message, 0);
	xdebug_str_add(&dstr, "\"}", 0);
	
	switch(type){
		case E_CORE_ERROR:
		case E_COMPILE_ERROR:
		case E_PARSE:
		case E_ERROR:
			XG(level)=0;
			break;
		default:
			break;
	}
	printout_to_file(&str, 1, 0);
	printout_to_file(&dstr, 1, 1);
	/*
        if (fprintf(XG(tracedata_file), "%s", dstr.d) < 0) {
                fclose(XG(tracedata_file));
                XG(tracedata_file) = NULL;
        } else {
                fflush(XG(tracedata_file));
        }

        if (fprintf(XG(trace_file), "%s", str.d) < 0) {
                fclose(XG(trace_file));
                XG(trace_file) = NULL;
        } else {
                fflush(XG(trace_file));
        }*/

}

void xdebug_odb_handle_exception(zval *exception TSRMLS_DC) {
	xdebug_str str = { 0, 0, NULL };
	xdebug_str dstr = { 0, 0, NULL };	
	zval *message, *file, *line;
	zend_class_entry *default_ce;
	if(!exception){
		return;
	}
	default_ce = zend_exception_get_default(TSRMLS_C);
	char *fn_nr=xdebug_sprintf("%d", ++XG(function_count));

	message = zend_read_property(default_ce, exception, "message", sizeof("message")-1, 0 TSRMLS_CC);
	file =    zend_read_property(default_ce, exception, "file",    sizeof("file")-1,    0 TSRMLS_CC);
	line =    zend_read_property(default_ce, exception, "line",    sizeof("line")-1,    0 TSRMLS_CC);

	xdebug_str_add(&str, "\n{\"lvl\":", 0);
	xdebug_str_add(&str, xdebug_sprintf("%d", XG(level)), 1);
	xdebug_str_add(&str, ",\"aid\":", 0);
	xdebug_str_add(&str, fn_nr, 0);
	xdebug_str_add(&str, ",\"atp\":6", 0);
	xdebug_str_add(&str, ",\"tme\":", 0);
	xdebug_str_add(&str, xdebug_sprintf("%f", xdebug_get_utime() - XG(
			start_time)), 1);
	xdebug_str_add(&str, ",\"fle\":\"", 0);
	xdebug_str_add(&str, Z_STRVAL_P(file), 0);
	xdebug_str_add(&str, "\",\"lne\":", 0);
	xdebug_str_add(&str, xdebug_sprintf("%d", Z_LVAL_P(line)), 0);
	xdebug_str_add(&str, ",\"mem\":", 0);
	xdebug_str_add(&str, xdebug_sprintf("%lu", XG_MEMORY_USAGE()), 1);
	xdebug_str_add(&str, ",\"nme\":\"Exception\"}", 0);

	xdebug_str_add(&dstr, "\n{\"aid\":", 0);
	xdebug_str_add(&dstr, fn_nr, 1);
	xdebug_str_add(&dstr, ",\"atp\":6,\"val\":", 0);
	
	fn_nr = xdebug_get_zval_json_value(exception, 0, NULL);
	if(fn_nr) {
		xdebug_str_add(&dstr, fn_nr, 1);
	}else{
		fn_nr = xdebug_print_hash_value(exception);
		xdebug_str_add(&dstr, "{\"type\":\"NULL\",\"id\":", 0);
		xdebug_str_add(&dstr, fn_nr, 1);
		xdebug_str_add(&dstr, "}", 0);
	}
	xdebug_str_add(&dstr, ",\"msg\":\"", 0);
	xdebug_str_add(&dstr, Z_STRVAL_P(message), 0);
	xdebug_str_add(&dstr, "\"}", 0);

	printout_to_file(&str, 1, 0);
	printout_to_file(&dstr, 1, 1);
	/*
	if (fprintf(XG(trace_file), "%s", str.d) < 0) {
		fclose(XG(trace_file));
		XG( trace_file) = NULL;
	} else {
		fflush(XG(trace_file));
	}

	if (fprintf(XG(tracedata_file), "%s", dstr.d) < 0) {
		fclose(XG(tracedata_file));
		XG( tracedata_file) = NULL;
	} else {
		fflush(XG(tracedata_file));
	}
	*/
}

char* xdebug_return_trace_assignment_json(function_stack_entry *i, char *varname, zval *retval, zval *mid, char *op, char *filename, int lineno TSRMLS_DC)
{
	int j = 0;
	xdebug_str str = {0, 0, NULL};
	xdebug_str dstr = {0, 0, NULL};
	char *tmp_value=0;

	xdebug_str_add(&str, "\n{\"lvl\":", 0);
	xdebug_str_add(&str, xdebug_sprintf("%d", i->level), 1);
	xdebug_str_add(&str, ",\"aid\":", 0);
	xdebug_str_add(&str, xdebug_sprintf("%d", ++XG(function_count)), 1);

	xdebug_str_add(&str, ",\"atp\":2", 0);
	xdebug_str_add(&str, ",\"tme\":", 0);
	xdebug_str_add(&str, xdebug_sprintf("%f", xdebug_get_utime() - XG(start_time)), 1);
#if HAVE_PHP_MEMORY_USAGE
	xdebug_str_add(&str, ",\"mem\":", 0);
	xdebug_str_add(&str, xdebug_sprintf("%lu", XG_MEMORY_USAGE()), 1);
#else

#endif
	zend_op *cur_opcode = *EG(opline_ptr), *opcode_ptr;

	xdebug_str_add(&dstr, "\n{\"aid\":",  0);
	xdebug_str_add(&dstr, xdebug_sprintf("%d", XG(function_count)), 1);
	xdebug_str_add(&dstr, ",\"atp\":2", 0);

	xdebug_str_add(&dstr, ",\"id\":", 0);
	xdebug_str_add(&dstr, xdebug_print_hash_value(mid), 1);

	xdebug_str_add(&str, ",\"nme\":\"", 0);
	xdebug_str_add(&str, varname, 0);
	xdebug_str_add(&str, "\"", 0);
	xdebug_str_add(&dstr, ",\"nme\":\"", 0);
	xdebug_str_add(&dstr, varname, 0);
	xdebug_str_add(&dstr, "\"", 0);


	xdebug_str_add(&str, ",\"fle\":\"",  0);
	xdebug_str_add(&str, filename, 0);
	xdebug_str_add(&str, "\",\"lne\":", 0);
	xdebug_str_add(&str, xdebug_sprintf("%d", lineno), 1);
	xdebug_str_add(&str, "}", 0);

	if(retval && retval !=NULL){
		tmp_value = xdebug_get_zval_json_value(retval, 1, NULL);
	}

	if (tmp_value) {
		xdebug_str_add(&dstr, ",\"val\":", 0);
		xdebug_str_add(&dstr, tmp_value, 1);
	} else {
		xdebug_str_add(&dstr, ",\"val\":{\"typ\":\"NULL\",\"id\":0}", 0);
	}

	xdebug_str_addl(&dstr, "}", 1, 0);

        printout_to_file(&str, 1, 0);
        printout_to_file(&dstr, 1, 1);

/*
	if (fprintf(XG(tracedata_file), "%s", dstr.d) < 0) {
		fclose(XG(tracedata_file));
		XG(tracedata_file) = NULL;
	} else {
		fflush(XG(tracedata_file));
	}

	if (fprintf(XG(trace_file), "%s", str.d) < 0) {
		fclose(XG(trace_file));
		XG(trace_file) = NULL;
	} else {
		fflush(XG(trace_file));
	}

	xdebug_str_free(&str);
	xdebug_str_free(&dstr);
*/
	return xdstrdup("");
}

char* xdebug_get_zval_json_value(zval *val, int debug_zval, xdebug_var_export_options *options) {
	xdebug_str str = { 0, 0, NULL };
	int default_options = 0;
	TSRMLS_FETCH();

	if (!options) {
		options = xdebug_var_export_options_from_ini(TSRMLS_C);
		options->max_children = 1048576;
		options->max_data = 1073741824;
		options->max_depth = 4096;
		xdfree(options->runtime);
		options->runtime=(xdebug_var_runtime_page*) xdmalloc((options->max_depth + 1) * sizeof(xdebug_var_runtime_page));
		default_options = 1;
	}

	xdebug_var_export_json(&val, (xdebug_str*) &str, 1, debug_zval, options TSRMLS_CC);


	if (default_options) {
		if(options->runtime){
			xdfree(options->runtime);
		}
		xdfree(options);
	}

	return str.d;
}

/*****************************************************************************
 ** JSON variable printing routines
 */
static int xdebug_array_element_export_json(zval **zv XDEBUG_ZEND_HASH_APPLY_TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level, debug_zval;
	xdebug_str *str;
	xdebug_var_export_options *options;
#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300
	TSRMLS_FETCH();
#endif

	level = va_arg(args, int);
	str = va_arg(args, struct xdebug_str*);
	debug_zval = va_arg(args, int);
	options = va_arg(args, xdebug_var_export_options*);

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
			options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		if (hash_key->nKeyLength==0) { /* numeric key */
			xdebug_str_add(str, "{\"nme\":\"", 0);
			xdebug_str_add(str, xdebug_sprintf("%ld", hash_key->h), 1);
			xdebug_str_add(str, "\",\"val\":", 0);
		} else { /* string key */
			int newlen = 0;
			char *tmp, *tmp2;

			tmp = php_str_to_str(hash_key->arKey, hash_key->nKeyLength, "'", 1, "\\'", 2, &newlen);
			tmp2 = php_str_to_str(tmp, newlen - 1, "\0", 1, "\\0", 2, &newlen);
			if (tmp) {
				efree(tmp);
			}
			xdebug_str_add(str, "{\"nme\":\"'", 0);
			if (tmp2) {
				xdebug_str_addl(str, tmp2, newlen, 0);
				efree(tmp2);
			}
			xdebug_str_add(str, "'\",\"val\":", 0);
		}
		xdebug_var_export_json(zv, str, level + 2, debug_zval, options TSRMLS_CC);
		xdebug_str_add(str, "},", 0);
	}
	if (options->runtime[level].current_element_nr == options->runtime[level].end_element_nr) {
		xdebug_str_add(str, "{\"typ\":\"...\"},", 0);
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

static int xdebug_object_element_export_json(zval **zv XDEBUG_ZEND_HASH_APPLY_TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level, debug_zval;
	xdebug_str *str;
	xdebug_var_export_options *options;
	char *prop_name, *class_name, *modifier, *prop_class_name, *spec_mod;
#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300
	TSRMLS_FETCH();
#endif

	level = va_arg(args, int);
	str = va_arg(args, struct xdebug_str*);
	debug_zval = va_arg(args, int);
	options = va_arg(args, xdebug_var_export_options*);
	class_name = va_arg(args, char *);
	spec_mod = va_arg(args, char *);

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
			options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		if (hash_key->nKeyLength != 0) {
			modifier = xdebug_get_property_info(hash_key->arKey, hash_key->nKeyLength, &prop_name, &prop_class_name);

			if (strcmp(modifier, "private") != 0 || strcmp(class_name, prop_class_name) == 0) {
				xdebug_str_add(str, "{\"mod\":\"", 0);
				xdebug_str_add(str, modifier, 0);
				xdebug_str_add(str, spec_mod, 0);
				xdebug_str_add(str, "\",\"nme\":\"", 0);
				xdebug_str_add(str, prop_name, 0);
				xdebug_str_add(str, "\",\"val\":", 0);
			} else {
				xdebug_str_add(str, "{\"mod\":\"", 0);
				xdebug_str_add(str, modifier, 0);
				xdebug_str_add(str, spec_mod, 0);
				xdebug_str_add(str, "\",\"nme\":\"{", 0);
				xdebug_str_add(str, prop_class_name, 0);
				xdebug_str_add(str, "}:", 0);
				xdebug_str_add(str, prop_name, 0);
				xdebug_str_add(str, "\",\"val\":", 0);
			}
			xdebug_var_export_json(zv, str, level + 2, debug_zval, options TSRMLS_CC);
			xdebug_str_add(str, "},", 0);

		}
	}
	if (options->runtime[level].current_element_nr == options->runtime[level].end_element_nr) {
		xdebug_str_add(str, "{\"typ\":\"...\"},", 0);
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

void xdebug_var_export_json(zval **struc, xdebug_str *str, int level, int forceOutput, xdebug_var_export_options *options TSRMLS_DC)
{
	HashTable *myht;
	zend_class_entry *zce;
	char* tmp_str;
	char* id;
	int tmp_len;

	zval **tmpVal=0;

	if (!struc || !(*struc)) {
		return;
	}
	//TODO if value unknown OR forceOutput, then print out the value, otherwise only put out reference

	//id = xdebug_sprintf("%lu",(*struc)->value);
	id=xdebug_print_hash_value(*struc);
	if((*struc)->XDEBUG_REFCOUNT && (*struc)->XDEBUG_REFCOUNT < 2){ //if only one time referenced then probably only short term var, changing
		forceOutput = 1;
	}
	if(level > 1){
		forceOutput=0;
	}
	// only add id if not known and no forcing
	if(forceOutput == 0 && (zend_hash_find(&XG(known_values),
							id, strlen(id)+1,
							(void**)&tmpVal) == SUCCESS) ) {//&& cmpzvals(tmpVal, struc)
		xdebug_str_add(str, "{\"id\":",0);
		xdebug_str_add(str, id, 0);
		xdebug_str_add(str, "}",0);
	} else {
		if(XG(do_trace) && XG(trace_file) && XG(tracedata_file) && forceOutput == 0) {
			zend_hash_add(&XG(known_values), id, strlen(id)+1, (void**)&tmpVal, sizeof(zval*), NULL);
		}// else if(tmpVal && (*tmpVal) && cmpzvals(tmpVal, struc)) {
			//zend_hash_update(&XG(known_values), id, sizeof(id), (void**)&struc, sizeof(zval*), NULL);
		//}

		//if(level>1){
		//	forceOutput=0;
		//}

		switch (Z_TYPE_PP(struc)) {
			case IS_BOOL:
			xdebug_str_add(str, "{\"typ\":\"bool\",\"id\":", 0);
			xdebug_str_add(str, id, 0);
			xdebug_str_add(str, ",\"val\":\"", 0);
			xdebug_str_add(str, Z_LVAL_PP(struc) ? "TRUE" : "FALSE", 0);
			xdebug_str_add(str, "\"}", 0);
			break;

			case IS_NULL:
			xdebug_str_add(str, "{\"typ\":\"NULL\",\"id\":", 0);
			xdebug_str_add(str,  id, 0);
			xdebug_str_add(str, "}",  0);
			break;

			case IS_LONG:
			xdebug_str_add(str, "{\"typ\":\"int\",\"id\":", 0);
			xdebug_str_add(str, id, 0);
			xdebug_str_add(str, ",\"val\":", 0);
			xdebug_str_add(str, xdebug_sprintf("%ld", Z_LVAL_PP(struc)), 1);
			xdebug_str_add(str, "}", 0);
			break;

			case IS_DOUBLE:
			xdebug_str_add(str, "{\"typ\":\"double\",\"id\":", 0);
			xdebug_str_add(str, id, 0);
			xdebug_str_add(str, ",\"val\":", 0);
			xdebug_str_add(str, xdebug_sprintf("%.*G", (int) EG(precision), Z_DVAL_PP(struc)), 1);
			xdebug_str_add(str, "}",  0);
			break;

			case IS_STRING:
			tmp_str = php_addcslashes(Z_STRVAL_PP(struc), Z_STRLEN_PP(struc), &tmp_len, 0, "\"\\\0..\37", 6 TSRMLS_CC);
			if (options->max_data == 0 || Z_STRLEN_PP(struc) <= options->max_data) {
				xdebug_str_add(str, "{\"typ\":\"string\",\"id\":", 0);
				xdebug_str_add(str, id, 0);
				xdebug_str_add(str, ",\"val\":\"'", 0);
				xdebug_str_addl(str, tmp_str, tmp_len, 0);
				xdebug_str_add(str, "'\"}", 0);
			} else {
				xdebug_str_add(str, "{\"typ\":\"string\",\"id\":", 0);
				xdebug_str_add(str, id, 0);
				xdebug_str_add(str, ",\"val\":\"'", 0);
				xdebug_str_addl(str, tmp_str, options->max_data, 0);
				xdebug_str_add(str, "...'\"}", 0);
			}
			efree(tmp_str);
			break;

			case IS_ARRAY:
			myht = Z_ARRVAL_PP(struc);
			if (myht->nApplyCount < 1) {
				xdebug_str_add(str, "{\"typ\":\"array\",\"id\":", 0);
				xdebug_str_add(str, id, 0);
				xdebug_str_add(str, ",\"val\":[", 0);
				if (level <= options->max_depth) {
					options->runtime[level].current_element_nr = 0;
					options->runtime[level].start_element_nr = 0;
					options->runtime[level].end_element_nr = options->max_children;

					zend_hash_apply_with_arguments(myht XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) xdebug_array_element_export_json, 4, level, str, forceOutput, options);
					/* Remove the ", " at the end of the string */
					if (myht->nNumOfElements > 0) {
						xdebug_str_chop(str, 1);
					}
				} else {
					xdebug_str_add(str, "{\"nme\":\"0\",\"typ\":\"...\",\"id\":", 0);
					xdebug_str_add(str, id, 0);
					xdebug_str_add(str, "}", 0);
				}
				xdebug_str_add(str, "]}", 0);
			} else {
				xdebug_str_add(str, "{\"nme\":\"0\",\"typ\":\"...\",\"id\":", 0);
				xdebug_str_add(str, id, 0);
				xdebug_str_add(str, "}", 0);
			}
			break;

			case IS_OBJECT:
			myht = Z_OBJPROP_PP(struc);
			zce = Z_OBJCE_PP(struc);
			if (myht->nApplyCount < 1) {
				char *class_name;
				int class_name_len;
				zend_class_entry *zce = Z_OBJCE_PP(struc);

				zend_get_object_classname(*struc, &class_name, &class_name_len TSRMLS_CC);
				xdebug_str_add(str, "{\"typ\":\"object\",\"nme\":\"",0);
				xdebug_str_add(str, class_name, 0);
				xdebug_str_add(str, "\",\"id\":", 0);
				xdebug_str_add(str, id, 0);
				xdebug_str_add(str, ",\"cid\":", 0);
				xdebug_str_add(str, xdebug_sprintf("%lu", zce), 1);
				xdebug_str_add(str, ",\"val\":[", 0);


				if (level <= options->max_depth) {
					options->runtime[level].current_element_nr = 0;
					options->runtime[level].start_element_nr = 0;
					options->runtime[level].end_element_nr = options->max_children;
					zend_hash_apply_with_arguments(zce->static_members XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) xdebug_object_element_export_json, 6, level, str, 0, options, class_name, " static");
					zend_hash_apply_with_arguments(&(zce->constants_table) XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) xdebug_object_element_export_json, 6, level, str, 0, options, class_name, " const");
					zend_hash_apply_with_arguments(myht XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) xdebug_object_element_export_json, 6, level, str, forceOutput, options, class_name, "");
					/* Remove the ", " at the end of the string */
					if(myht->nNumOfElements > 0 || zce->static_members->nNumOfElements > 0 || zce->constants_table.nNumOfElements > 0){
						xdebug_str_chop(str, 1);
					}
				} else {
					xdebug_str_add(str, "", 0);
				}
				xdebug_str_add(str, "]}", 0);
				efree(class_name);
			} else {
				xdebug_str_add(str, "{\"typ\":\"object\",\"nme\":\"???\",\"id\":", 0);
				xdebug_str_add(str, id, 0);
				xdebug_str_add(str, ",\"val\":[]}", 0);
			}
			break;

			case IS_RESOURCE: {
				char *type_name;

				type_name = zend_rsrc_list_get_rsrc_type(Z_LVAL_PP(struc) TSRMLS_CC);
				xdebug_str_add(str, "{\"typ\":\"resource\",\"nme\":\"", 0);
				xdebug_str_add(str, type_name ? type_name : "Unknown", 0);
				xdebug_str_add(str, "\",\"id\":", 0);
				xdebug_str_add(str, id, 0);
				xdebug_str_add(str, ",\"val\":\"", 0);
				xdebug_str_add(str, xdebug_sprintf("%ld", Z_LVAL_PP(struc)), 1);
				xdebug_str_add(str, "\"}", 0);
				break;
			}

			default:
				xdebug_str_add(str, "{\"typ\":\"void\",\"nme\":\"", 0);
				xdebug_str_add(str, xdebug_sprintf("%d", Z_TYPE_PP(struc)), 1);
				xdebug_str_add(str, "\",\"id\":", 0);
				xdebug_str_add(str, id, 0);
				xdebug_str_add(str, "}", 0);
			break;
		}
	}

	if(id){
		xdfree(id);
	}
}
