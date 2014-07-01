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

/*!
 * @file stack.h
 * @brief stack container
 * @details
 * A stack is a data structure (container) used for collecting a sequence of elements.
 * stack allow for efficient insertion, removal and retreival of elements.
 *
 * @details For example,
 * @code
 * #include <clib/stack.h>
 * #include <clib/stack_iter.h>
 *
 * int main(const int argc, const char * argv[]) {
 *     typedef struct {
 *          stack_node_t node;
 *          int i;
 *          float f;
 *     } data_t;
 *
 *     slab_t s;
 *     slab_init(&s, sizeof(data_t), 0);
 *
 *     stack_t a;
 *     stack_init(&a);
 *
 *     int i;
 *     for (i=0; i<10; i++) {
 *         data_t * d = (data_t *)slab_alloc(&s);
 *
 *         d->i = i;
 *         d->f = (float)i;
 *
 *         stack_add_tail(&l, &d->node);
 *     }
 *
 *     data_t * d;
 *     stack_for_each(&l, d, node) {
 *         printf("i: %d f: %f\n", d->i, d->f);
 *     }
 *
 *     stack_dump(&l, stdout);
 *     slab_delete(&s);
 *
 *     return 0;
 * }
 * @endcode
 * @author Shaun Wetzstein <shaun@us.ibm.com>
 * @date 2010-2011
 */

#ifndef __STACK_H__
#define __STACK_H__

#include "list.h"
#include "type.h"

typedef list stack;
typedef list_node stack_node;

#define stack_init(s)		list_init((list *)(s))
#define stack_push(s,n)		list_add_tail((list *)(s),(n))
#define stack_pop(s)		list_remove_tail((list *)(s))
#define stack_empty(s)		list_empty((list *)(s))
#define stack_dump(s,o)		list_dump((list *)(s),(o))

#define stack_entry(n, t, m)	list_entry((n),(t),(m))
#define stack_top(s)		list_head((list *)(s))
#define stack_bottom(s)		list_tail((list *)(s))

#define stack_for_each(s, i, m)				\
    for (i = container_of_var(s->node.next, i, m);	\
        &i->m != &(s)->node;				\
        i = container_of_var(i->m.next, i, m))

#define stack_for_each_safe(s, i, n, m)			\
    for (i = container_of_var((s)->node.next, i, m),	\
            n = container_of_var(i->m.next, i, m);	\
        &i->m != &(s)->node;				\
        i = n, n = container_of_var(i->m.next, i, m))

#endif				/* __STACK_H__ */
