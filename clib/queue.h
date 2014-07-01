/*
 * Copyright (c) International Business Machines Corp., 2014
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 *   File: queue.h
 * Author: Shaun Wetzstein <shaun@us.ibm.com>
 *  Descr: Queue container
 *   Note: This is an adaptor of the list container
 *   Date: 10/07/10
 */

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "list.h"
#include "type.h"

typedef list queue;
typedef list_node queue_node;

#define queue_init(q)		list_init((list *)(q))
#define queue_push(q,n)		list_add_tail((list *)(q),(n))
#define queue_pop(q)		list_remove_head((list *)(q))
#define stack_empty(q)		list_empty((list *)(q))
#define queue_dump(q,s)		list_dump((list *)(q),(s))

#define queue_entry(n, t, m)	list_entry((n),(t),(m))
#define queue_top(q)		list_head((list *)(q))
#define queue_bottom(q)		list_tail((list *)(q))

#define queue_for_each(q, i, m)				\
    for (i = container_of_var(q->node.next, i, m);	\
        &i->m != &(q)->node;				\
        i = container_of_var(i->m.next, i, m))

#define queue_for_each_safe(q, i, n, m)			\
    for (i = container_of_var((q)->node.next, i, m),	\
            n = container_of_var(i->m.next, i, m);	\
        &i->m != &(q)->node;				\
        i = n, n = container_of_var(i->m.next, i, m))

#endif				/* __QUEUE_H__ */
