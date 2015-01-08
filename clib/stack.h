/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/stack.h $                                                */
/*                                                                        */
/* OpenPOWER FFS Project                                                  */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2015                        */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

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
