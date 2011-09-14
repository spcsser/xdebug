/* $Id: xdebug_llist.h,v 1.4 2003-02-20 14:30:54 derick Exp $ */

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
 * Contributor(s): Sterling Hughes <sterling@php.net> 
 * Daniel R. Kalowsky <dank@deadmime.org>
 */

#ifndef __XDEBUG_TREE_H__
#define __XDEBUG_TREE_H__

#include <stddef.h>

typedef void (*xdebug_tree_dtor)(void *, void *);

typedef struct _xdebug_tree_element {
	void *ptr;

	struct _xdebug_tree_element *parent;
	struct _xdebug_tree_element **children;
	struct _xdebug_tree_element *next;
} xdebug_tree_element;

typedef struct _xdebug_tree {
	xdebug_tree_element *root;
	xdebug_tree_element *last_child;

	xdebug_tree_dtor dtor;

	size_t size;
} xdebug_tree;

xdebug_tree *xdebug_tree_alloc(xdebug_tree_dtor dtor);
void xdebug_tree_init(xdebug_tree *l, xdebug_tree_dtor dtor);

int xdebug_tree_insert_next(xdebug_tree *l, xdebug_tree_element *parent, xdebug_tree_element *e, const void *p);

int xdebug_tree_insert_prev(xdebug_tree *l, xdebug_tree_element *e, const void *p);
int xdebug_tree_remove(xdebug_tree *l, xdebug_tree_element *e, void *user);
int xdebug_tree_remove_next(xdebug_tree *l, xdebug_tree_element *e, void *user);
xdebug_tree_element *xdebug_tree_jump(xdebug_tree *l, int where, int pos);
size_t xdebug_tree_count(xdebug_tree *l);
void xdebug_tree_empty(xdebug_tree *l, void *user);
void xdebug_tree_destroy(xdebug_tree *l, void *user);

#if !defined(TREE_ROOT)
#define TREE_ROOT 0
#endif

#if !defined(LIST_TAIL)
#define LIST_TAIL 1
#endif

#define XDEBUG_TREE_ROOT(__l) ((__l)->root)
#define XDEBUG_TREE_LAST_CHILD(__l) ((__l)->last_child)

#define XDEBUG_LLIST_NEXT(__e) ((__e)->next)
#define XDEBUG_LLIST_PREV(__e) ((__e)->prev)
#define XDEBUG_LLIST_VALP(__e) ((__e)->ptr)
#define XDEBUG_LLIST_IS_TAIL(__e) ((__e)->next ? 0 : 1)
#define XDEBUG_LLIST_IS_HEAD(__e) ((__e)->prev ? 0 : 1)
#define XDEBUG_LLIST_COUNT(__l) ((__l)->size)

#endif /* __XDEBUG_LLIST_H__ */
