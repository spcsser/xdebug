/* $Id: xdebug_llist.c,v 1.5 2007-01-02 16:02:37 derick Exp $ */

/* The contents of this file are subject to the Vulcan Logic Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.vl-srm.net/vlpl/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is vl-srm.net code.
 *
 * The Initial Developer of the Original Code is the Vulcan Logic 
 * Group.  Portions created by Vulcan Logic Group are Copyright (C) 
 * 2000, 2001, 2002 Vulcan Logic Group. All Rights Reserved.
 *
 * Author(s): Sterling Hughes <sterling@php.net> 
 */

#include <stdlib.h>
#include <string.h>

#include "xdebug_tree.h"

xdebug_tree *xdebug_tree_alloc(xdebug_tree_dtor dtor)
{
	xdebug_tree *l;

	l = malloc(sizeof(xdebug_tree));
	xdebug_tree_init(l, dtor);

	return l;
}

void xdebug_tree_init(xdebug_tree *l, xdebug_tree_dtor dtor)
{
	l->size = 0;
	l->dtor = dtor;
	l->root = NULL;
	l->last_child = NULL;
}

int xdebug_tree_insert_next(xdebug_tree *l, xdebug_tree_element *parent, xdebug_tree_element *e, const void *p)
{
	xdebug_tree_element  *ne;

	if (!e) {
		e = XDEBUG_TREE_LAST_CHILD(l);
	}

	if(!parent) {
		parent = XDEBUG_TREE_ROOT(l);
	}

	ne = (xdebug_tree_element *) malloc(sizeof(xdebug_tree_element));
	ne->ptr = (void *) p;
	if (l->size == 0) {
		l->head = ne;
		l->head->prev = NULL;
		l->head->next = NULL;
		l->tail = ne;
	} else {
		ne->next = e->next;
		ne->prev = e;
		if (e->next) {
			e->next->prev = ne;
		} else {
			l->tail = ne;
		}
		e->next = ne;
	}

	++l->size;

	return 1;
}

int xdebug_tree_insert_prev(xdebug_tree *l, xdebug_tree_element *e, const void *p)
{
	xdebug_tree_element *ne;

	if (!e) {
		e = XDEBUG_LLIST_HEAD(l);
	}

	ne = (xdebug_tree_element *) malloc(sizeof(xdebug_tree_element));
	ne->ptr = (void *) p;
	if (l->size == 0) {
		l->head = ne;
		l->head->prev = NULL;
		l->head->next = NULL;
		l->tail = ne;
	} else {
		ne->next = e;
		ne->prev = e->prev;
		if (e->prev)
			e->prev->next = ne;
		else
			l->head = ne;
		e->prev = ne;
	}

	++l->size;

	return 0;
}

int xdebug_tree_remove(xdebug_tree *l, xdebug_tree_element *e, void *user)
{
	if (e == NULL || l->size == 0)
		return 0;

	if (e == l->head) {
		l->head = e->next;

		if (l->head == NULL)
			l->tail = NULL;
		else
			e->next->prev = NULL;
	} else {
		e->prev->next = e->next;
		if (!e->next)
			l->tail = e->prev;
		else
			e->next->prev = e->prev;
	}

	if (l->dtor) {
		l->dtor(user, e->ptr);
	}
	free(e);
	--l->size;

	return 0;
}

int xdebug_tree_remove_next(xdebug_tree *l, xdebug_tree_element *e, void *user)
{
	return xdebug_tree_remove(l, e->next, user);
}

int xdebug_tree_remove_prev(xdebug_tree *l, xdebug_tree_element *e, void *user)
{
	return xdebug_tree_remove(l, e->prev, user);
}

xdebug_tree_element *xdebug_tree_jump(xdebug_tree *l, int where, int pos)
{
    xdebug_tree_element *e=NULL;
    int i;

    if (where == LIST_HEAD) {
        e = XDEBUG_LLIST_HEAD(l);
        for (i = 0; i < pos; ++i) {
            e = XDEBUG_LLIST_NEXT(e);
        }
    }
    else if (where == LIST_TAIL) {
        e = XDEBUG_LLIST_TAIL(l);
        for (i = 0; i < pos; ++i) {
            e = XDEBUG_LLIST_PREV(e);
        }
    }

    return e;
}

size_t xdebug_tree_count(xdebug_tree *l)
{
	return l->size;
}

void xdebug_tree_empty(xdebug_tree *l, void *user)
{
	while (xdebug_tree_count(l) > 0) {
		xdebug_tree_remove(l, XDEBUG_LLIST_TAIL(l), user);
	}
}

void xdebug_tree_destroy(xdebug_tree *l, void *user)
{
	xdebug_tree_empty(l, user);

	free (l);
}

/*
 * Local Variables:
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
