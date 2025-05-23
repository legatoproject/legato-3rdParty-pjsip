/* 
 * Copyright (C) 2008-2011 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2003-2008 Benny Prijono <benny@prijono.org>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */
#include <pj/pool.h>
#include <pj/assert.h>
#include <pj/string.h>

#if PJ_HAS_POOL_ALT_API

#if PJ_HAS_MALLOC_H
#   include <malloc.h>
#endif


#if PJ_HAS_STDLIB_H
#   include <stdlib.h>
#endif


#if ((defined(PJ_WIN32) && PJ_WIN32!=0) || \
     (defined(PJ_WIN64) && PJ_WIN64 != 0)) && \
     defined(PJ_DEBUG) && PJ_DEBUG!=0 && !PJ_NATIVE_STRING_IS_UNICODE
#   include <windows.h>
#   define TRACE_(msg)  OutputDebugString(msg)
#endif

/* Uncomment this to enable TRACE_ */
//#undef TRACE_

#define IS_POWER_OF_TWO(val)    (((val)>0) && ((val) & ((val)-1))==0)

PJ_DEF_DATA(int) PJ_NO_MEMORY_EXCEPTION;


PJ_DEF(int) pj_NO_MEMORY_EXCEPTION()
{
    return PJ_NO_MEMORY_EXCEPTION;
}

/* Create pool */
PJ_DEF(pj_pool_t*) pj_pool_create_imp( const char *file, int line,
                                       void *factory,
                                       const char *name,
                                       pj_size_t initial_size,
                                       pj_size_t increment_size,
                                       pj_size_t alignment,
                                       pj_pool_callback *callback)
{
    pj_pool_t *pool;

    PJ_UNUSED_ARG(file);
    PJ_UNUSED_ARG(line);
    PJ_UNUSED_ARG(factory);
    PJ_UNUSED_ARG(initial_size);
    PJ_UNUSED_ARG(increment_size);

    if (!alignment)
        alignment = PJ_POOL_ALIGNMENT;

    PJ_ASSERT_RETURN(IS_POWER_OF_TWO(alignment), NULL);

    pool = malloc(sizeof(struct pj_pool_t));
    if (!pool)
        return NULL;

    if (name) {
        char *p = pj_ansi_strchr(name, '%');
        if (p && *(p+1)=='p' && *(p+2)=='\0') {
            /* Special name with "%p" suffix */
            pj_ansi_snprintf(pool->obj_name, sizeof(pool->obj_name),
                             name, pool);
        } else {
            pj_ansi_strxcpy(pool->obj_name, name, PJ_MAX_OBJ_NAME);
        }
    } else {
        pj_ansi_strxcpy(pool->obj_name, "altpool", sizeof(pool->obj_name));
    }

    pool->factory = NULL;
    pool->first_mem = NULL;
    pool->used_size = 0;
    pool->alignment = alignment;
    pool->cb = callback;

    return pool;
}


/* Release pool */
PJ_DEF(void) pj_pool_release_imp(pj_pool_t *pool)
{
    pj_pool_reset(pool);
    free(pool);
}

/* Safe release pool */
PJ_DEF(void) pj_pool_safe_release_imp( pj_pool_t **ppool )
{
    pj_pool_t *pool = *ppool;
    *ppool = NULL;
    if (pool)
        pj_pool_release(pool);
}

/* Secure release pool */
PJ_DEF(void) pj_pool_secure_release_imp( pj_pool_t **ppool )
{
    /* Secure release is not implemented, so we just call
     * safe release.
     */
    pj_pool_safe_release_imp(ppool);
}

/* Get pool name */
PJ_DEF(const char*) pj_pool_getobjname_imp(pj_pool_t *pool)
{
    PJ_UNUSED_ARG(pool);
    return "pooldbg";
}

/* Reset pool */
PJ_DEF(void) pj_pool_reset_imp(pj_pool_t *pool)
{
    struct pj_pool_mem *mem;

    mem = pool->first_mem;
    while (mem) {
        struct pj_pool_mem *next = mem->next;
        free(mem);
        mem = next;
    }

    pool->first_mem = NULL;
}

/* Get capacity */
PJ_DEF(pj_size_t) pj_pool_get_capacity_imp(pj_pool_t *pool)
{
    PJ_UNUSED_ARG(pool);

    /* Unlimited capacity */
    return 0x7FFFFFFFUL;
}

/* Get total used size */
PJ_DEF(pj_size_t) pj_pool_get_used_size_imp(pj_pool_t *pool)
{
    return pool->used_size;
}

/* Allocate memory from the pool */
PJ_DEF(void*) pj_pool_alloc_imp( const char *file, int line, 
                                 pj_pool_t *pool, pj_size_t alignment,
                                 pj_size_t sz)
{
    struct pj_pool_mem *mem;
    char               *buf;

    PJ_UNUSED_ARG(file);
    PJ_UNUSED_ARG(line);

    if (!alignment)
        alignment = pool->alignment;
        
    PJ_ASSERT_RETURN(IS_POWER_OF_TWO(alignment), NULL);

    /* obey alignment request from user */
    if (sz & (alignment - 1)) {
        sz = (sz + alignment) & ~(alignment - 1);
    }
    mem = malloc(sz +                /* allocation size, already aligned      */
                 alignment-1 +       /*gap [0:alignment-1] to align allocation*/
                 sizeof(struct pj_pool_mem)); /*block header, may be unaligned*/
    if (!mem) {
        if (pool->cb)
            (*pool->cb)(pool, sz);
        return NULL;
    }

    mem->next = pool->first_mem;
    pool->first_mem = mem;

#ifdef TRACE_
    {
        char msg[120];
        pj_ansi_snprintf(msg, sizeof(msg),
                        "Mem %X (%u+%u bytes) allocated by %s:%d\r\n",
                        (unsigned)(intptr_t)mem, (unsigned)sz,
                        (unsigned)sizeof(struct pj_pool_mem),
                        file, line);
        TRACE_(msg);
    }
#endif
    buf = ((char*)mem) + sizeof(struct pj_pool_mem);
    return (buf + (-(pj_ssize_t)buf & (alignment-1)));
}

/* Allocate memory from the pool and zero the memory */
PJ_DEF(void*) pj_pool_calloc_imp( const char *file, int line,
                                  pj_pool_t *pool, unsigned cnt,
                                  unsigned elemsz)
{
    void *mem;

    mem = pj_pool_alloc_imp(file, line, pool, 0, cnt*elemsz);
    if (!mem)
        return NULL;

    pj_bzero(mem, cnt*elemsz);
    return mem;
}

/* Allocate memory from the pool and zero the memory */
PJ_DEF(void*) pj_pool_zalloc_imp( const char *file, int line, 
                                  pj_pool_t *pool, pj_size_t sz)
{
    return pj_pool_calloc_imp(file, line, pool, 1, (unsigned)sz); 
}



#endif  /* PJ_HAS_POOL_ALT_API */
