/*
 * Copyright (c) 2017, EURECOM (www.eurecom.fr)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of the FreeBSD Project.
 */
/*! \file hashtable_uint64.c
  \brief
  \author Lionel Gauthier
  \company Eurecom
  \email: lionel.gauthier@eurecom.fr
*/
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <pthread.h>

#include "lte/gateway/c/core/oai/lib/bstr/bstrlib.h"
#include "lte/gateway/c/core/oai/common/dynamic_memory_check.h"
#include "lte/gateway/c/core/oai/lib/hashtable/hashtable.h"

#if TRACE_HASHTABLE
#define PRINT_HASHTABLE(hTbLe, ...)                                \
  do {                                                             \
    if (hTbLe->log_enabled) OAILOG_TRACE(LOG_UTIL, ##__VA_ARGS__); \
  } while (0)
#else
#define PRINT_HASHTABLE(...)
#endif

//------------------------------------------------------------------------------
/*
   Default hash function
   def_hashfunc() is the default used by hashtable_uint64_create() when the user
   didn't specify one. This is a simple/naive hash function which adds the key's
   ASCII char values. It will probably generate lots of collisions on large hash
   tables.
*/
static inline hash_size_t def_hashfunc(const uint64_t keyP) {
  return (hash_size_t)keyP;
}

//------------------------------------------------------------------------------
/*
   Initialization
   hashtable_uint64_init() set up the initial structure of the hash table. The
   user specified size will be allocated and initialized to NULL. The user can
   also specify a hash function. If the hashfunc argument is NULL, a default
   hash function is used. If an error occurred, NULL is returned. All other
   values in the returned hash_table_uint64_t pointer should be released with
   hashtable_uint64_destroy().
*/
hash_table_uint64_t* hashtable_uint64_init(
    hash_table_uint64_t* const hashtblP, const hash_size_t sizeP,
    hash_size_t (*hashfuncP)(const hash_key_t), bstring display_name_pP) {
  hash_size_t size = sizeP;
  // upper power of two:
  // http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2Float
  //  By Sean Eron Anderson
  // seander@cs.stanford.edu
  // Individually, the code snippets here are in the public domain (unless
  // otherwise noted) — feel free to use them however you please. The aggregate
  // collection and descriptions are © 1997-2005 Sean Eron Anderson. The code
  // and descriptions are distributed in the hope that they will be useful, but
  // WITHOUT ANY WARRANTY and without even the implied warranty of
  // merchantability or fitness for a particular purpose. As of May 5, 2005, all
  // the code has been tested thoroughly. Thousands of people have read it.
  // Moreover, Professor Randal Bryant, the Dean of Computer Science at Carnegie
  // Mellon University, has personally tested almost everything with his Uclid
  // code verification system. What he hasn't tested, I have checked against all
  // possible inputs on a 32-bit machine. To the first person to inform me of a
  // legitimate bug in the code, I'll pay a bounty of US$10 (by check or
  // Paypal). If directed to a charity, I'll pay US$20.
  size--;
  size |= size >> 1;
  size |= size >> 2;
  size |= size >> 4;
  size |= size >> 8;
  size |= size >> 16;
  size++;

  if (!(hashtblP->nodes = calloc(size, sizeof(hash_node_uint64_t*)))) {
    free_wrapper((void**)&hashtblP);
    return NULL;
  }
  hashtblP->log_enabled = true;

  PRINT_HASHTABLE(hashtblP, "allocated nodes\n");
  hashtblP->size = size;

  if (hashfuncP)
    hashtblP->hashfunc = hashfuncP;
  else
    hashtblP->hashfunc = def_hashfunc;

  if (display_name_pP) {
    bassign(hashtblP->name, display_name_pP);
  } else {
    hashtblP->name = bformat("hashtable%u@%p", size, hashtblP);
  }
  hashtblP->is_allocated_by_malloc = false;
  return hashtblP;
}

//------------------------------------------------------------------------------
/*
   Initialization
   hashtable_uint64_ts_init() sets up the initial structure of the thread safe
   hash table. The user specified size will be allocated and initialized to
   NULL. The user can also specify a hash function. If the hashfunc argument is
   NULL, a default hash function is used. If an error occurred, NULL is
   returned. All other values in the returned hash_table_uint64_t pointer should
   be released with hashtable_uint64_destroy().
*/
hash_table_uint64_ts_t* hashtable_uint64_ts_init(
    hash_table_uint64_ts_t* const hashtblP, const hash_size_t sizeP,
    hash_size_t (*hashfuncP)(const hash_key_t), bstring display_name_pP) {
  hash_size_t size = sizeP;
  // upper power of two:
  // http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2Float
  //  By Sean Eron Anderson
  // seander@cs.stanford.edu
  // Individually, the code snippets here are in the public domain (unless
  // otherwise noted) — feel free to use them however you please. The aggregate
  // collection and descriptions are © 1997-2005 Sean Eron Anderson. The code
  // and descriptions are distributed in the hope that they will be useful, but
  // WITHOUT ANY WARRANTY and without even the implied warranty of
  // merchantability or fitness for a particular purpose. As of May 5, 2005, all
  // the code has been tested thoroughly. Thousands of people have read it.
  // Moreover, Professor Randal Bryant, the Dean of Computer Science at Carnegie
  // Mellon University, has personally tested almost everything with his Uclid
  // code verification system. What he hasn't tested, I have checked against all
  // possible inputs on a 32-bit machine. To the first person to inform me of a
  // legitimate bug in the code, I'll pay a bounty of US$10 (by check or
  // Paypal). If directed to a charity, I'll pay US$20.
  size--;
  size |= size >> 1;
  size |= size >> 2;
  size |= size >> 4;
  size |= size >> 8;
  size |= size >> 16;
  size++;

  memset(hashtblP, 0, sizeof(*hashtblP));

  if (!(hashtblP->nodes = calloc(size, sizeof(hash_node_uint64_t*)))) {
    free_wrapper((void**)&hashtblP);
    return NULL;
  }

  if (!(hashtblP->lock_nodes = calloc(size, sizeof(pthread_mutex_t)))) {
    free_wrapper((void**)&hashtblP->nodes);
    free_wrapper((void**)&hashtblP->name);
    free_wrapper((void**)&hashtblP);
    return NULL;
  }

  pthread_mutex_init(&hashtblP->mutex, NULL);
  for (int i = 0; i < size; i++) {
    pthread_mutex_init(&hashtblP->lock_nodes[i], NULL);
  }

  hashtblP->size = size;

  if (hashfuncP)
    hashtblP->hashfunc = hashfuncP;
  else
    hashtblP->hashfunc = def_hashfunc;

  if (display_name_pP) {
    hashtblP->name = bstrcpy(display_name_pP);
  } else {
    hashtblP->name = bformat("hashtable@%p", hashtblP);
  }
  hashtblP->is_allocated_by_malloc = false;
  hashtblP->log_enabled = true;
  return hashtblP;
}
//------------------------------------------------------------------------------
/*
   Initialization
   hashtable_uint64_ts_create() allocate and sets up the initial structure of
   the thread safe hash table. The user specified size will be allocated and
   initialized to NULL. The user can also specify a hash function. If the
   hashfunc argument is NULL, a default hash function is used. If an error
   occurred, NULL is returned. All other values in the returned
   hash_table_uint64_t pointer should be released with
   hashtable_uint64_destroy().
*/
hash_table_uint64_ts_t* hashtable_uint64_ts_create(
    const hash_size_t sizeP, hash_size_t (*hashfuncP)(const hash_key_t),
    bstring display_name_pP) {
  hash_table_uint64_ts_t* hashtbl = NULL;

  if (!(hashtbl = calloc(1, sizeof(hash_table_uint64_ts_t)))) {
    return NULL;
  }
  hashtbl =
      hashtable_uint64_ts_init(hashtbl, sizeP, hashfuncP, display_name_pP);
  hashtbl->is_allocated_by_malloc = true;
  return hashtbl;
}

//------------------------------------------------------------------------------
/*
   Cleanup
   The hashtable_uint64_destroy() walks through the linked lists for each
   possible hash value, and releases the elements. It also releases the nodes
   array and the hash_table_uint64_t.
*/
hashtable_rc_t hashtable_uint64_ts_destroy(hash_table_uint64_ts_t* hashtblP) {
  hash_size_t n = 0;
  hash_node_uint64_t *node = NULL, *oldnode = NULL;

  if (!hashtblP) {
    return HASH_TABLE_BAD_PARAMETER_HASHTABLE;
  }

  for (n = 0; n < hashtblP->size; ++n) {
    pthread_mutex_lock(&hashtblP->lock_nodes[n]);
    node = hashtblP->nodes[n];

    while (node) {
      oldnode = node;
      node = node->next;
      free_wrapper((void**)&oldnode);
    }

    pthread_mutex_unlock(&hashtblP->lock_nodes[n]);
    pthread_mutex_destroy(&hashtblP->lock_nodes[n]);
  }

  free_wrapper((void**)&hashtblP->nodes);
  bdestroy_wrapper(&hashtblP->name);
  free_wrapper((void**)&hashtblP->lock_nodes);
  hashtblP->size = 0;
  if (hashtblP->is_allocated_by_malloc) {
    free_wrapper((void**)&hashtblP);
  }
  return HASH_TABLE_OK;
}

//------------------------------------------------------------------------------
hashtable_rc_t hashtable_uint64_ts_is_key_exists(
    const hash_table_uint64_ts_t* const hashtblP, const hash_key_t keyP) {
  hash_node_uint64_t* node = NULL;
  hash_size_t hash = 0;

  if (!hashtblP) {
    return HASH_TABLE_BAD_PARAMETER_HASHTABLE;
  }

  hash = hashtblP->hashfunc(keyP) % hashtblP->size;
  pthread_mutex_lock(&hashtblP->lock_nodes[hash]);
  node = hashtblP->nodes[hash];

  while (node) {
    if (node->key == keyP) {
      pthread_mutex_unlock(&hashtblP->lock_nodes[hash]);
      PRINT_HASHTABLE(hashtblP, "%s(%s,key 0x%" PRIx64 ") return OK\n",
                      __FUNCTION__, bdata(hashtblP->name), keyP);
      return HASH_TABLE_OK;
    }

    node = node->next;
  }
  pthread_mutex_unlock(&hashtblP->lock_nodes[hash]);
  PRINT_HASHTABLE(hashtblP, "%s(%s,key 0x%" PRIx64 ") return KEY_NOT_EXISTS\n",
                  __FUNCTION__, bdata(hashtblP->name), keyP);
  return HASH_TABLE_KEY_NOT_EXISTS;
}

//------------------------------------------------------------------------------
// may cost a lot CPU...
hashtable_key_array_t* hashtable_uint64_ts_get_keys(
    hash_table_uint64_ts_t* const hashtblP) {
  hash_node_uint64_t* node = NULL;
  unsigned int i = 0;
  hashtable_key_array_t* ka = NULL;

  if ((!hashtblP) || !(hashtblP->num_elements)) {
    return NULL;
  }
  ka = calloc(1, sizeof(hashtable_key_array_t));
  ka->keys = calloc(hashtblP->num_elements, sizeof(hash_key_t));

  while ((ka->num_keys < hashtblP->num_elements) && (i < hashtblP->size)) {
    pthread_mutex_lock(&hashtblP->lock_nodes[i]);
    if (hashtblP->nodes[i] != NULL) {
      node = hashtblP->nodes[i];
      while (node) {
        ka->keys[ka->num_keys++] = node->key;
        node = node->next;
      }
    }
    pthread_mutex_unlock(&hashtblP->lock_nodes[i]);
    i++;
  }
  return ka;
}

//------------------------------------------------------------------------------
// may cost a lot CPU...
// Also useful if we want to find an element in the collection based on compare
// criteria different than the single key The compare criteria in implemented in
// the funct_cb function
hashtable_rc_t hashtable_uint64_ts_apply_callback_on_elements(
    hash_table_uint64_ts_t* const hashtblP,
    bool funct_cb(const hash_key_t keyP, const uint64_t dataP, void* parameterP,
                  void** resultP),
    void* parameterP, void** resultP) {
  hash_node_uint64_t* node = NULL;
  unsigned int i = 0;
  unsigned int num_elements = 0;

  if (!hashtblP) {
    return HASH_TABLE_BAD_PARAMETER_HASHTABLE;
  }

  while ((num_elements < hashtblP->num_elements) && (i < hashtblP->size)) {
    pthread_mutex_lock(&hashtblP->lock_nodes[i]);
    if (hashtblP->nodes[i] != NULL) {
      node = hashtblP->nodes[i];

      while (node) {
        num_elements++;
        if (funct_cb(node->key, node->data, parameterP, resultP)) {
          pthread_mutex_unlock(&hashtblP->lock_nodes[i]);
          return HASH_TABLE_OK;
        }
        node = node->next;
      }
    }
    pthread_mutex_unlock(&hashtblP->lock_nodes[i]);
    i++;
  }

  return HASH_TABLE_OK;
}

//------------------------------------------------------------------------------
hashtable_rc_t hashtable_uint64_ts_dump_content(
    const hash_table_uint64_ts_t* const hashtblP, bstring str) {
  hash_node_uint64_t* node = NULL;
  unsigned int i = 0;

  if (!hashtblP) {
    bcatcstr(str, "HASH_TABLE_BAD_PARAMETER_HASHTABLE");
    return HASH_TABLE_BAD_PARAMETER_HASHTABLE;
  }

  while (i < hashtblP->size) {
    if (hashtblP->nodes[i] != NULL) {
      pthread_mutex_lock(&hashtblP->lock_nodes[i]);
      node = hashtblP->nodes[i];

      while (node) {
        bstring b0 =
            bformat("Key 0x%" PRIx64 " Element %" PRIx64 " Node %p Next %p\n",
                    node->key, node->data, node, node->next);
        if (!b0) {
          PRINT_HASHTABLE(hashtblP, "Error while dumping hashtable content");
        } else {
          bconcat(str, b0);
          bdestroy_wrapper(&b0);
        }
        node = node->next;
      }
      pthread_mutex_unlock(&hashtblP->lock_nodes[i]);
    }
    i += 1;
  }
  return HASH_TABLE_OK;
}

//------------------------------------------------------------------------------
/*
   Adding a new element
   To make sure the hash value is not bigger than size, the result of the user
   provided hash function is used modulo size.
*/
hashtable_rc_t hashtable_uint64_insert(hash_table_uint64_t* const hashtblP,
                                       const hash_key_t keyP,
                                       const uint64_t dataP) {
  hash_node_uint64_t* node = NULL;
  hash_size_t hash = 0;

  if (!hashtblP) {
    return HASH_TABLE_BAD_PARAMETER_HASHTABLE;
  }

  hash = hashtblP->hashfunc(keyP) % hashtblP->size;
  node = hashtblP->nodes[hash];

  while (node) {
    if (node->key == keyP) {
      if (node->data != dataP) {
        node->data = dataP;
        PRINT_HASHTABLE(hashtblP,
                        "%s(%s,key 0x%" PRIx64 " data %" PRIx64
                        ") return INSERT_OVERWRITTEN_DATA\n",
                        __FUNCTION__, bdata(hashtblP->name), keyP, dataP);
        return HASH_TABLE_INSERT_OVERWRITTEN_DATA;
      }
      node->data = dataP;
      PRINT_HASHTABLE(hashtblP,
                      "%s(%s,key 0x%" PRIx64 " data %" PRIx64 ") return OK\n",
                      __FUNCTION__, bdata(hashtblP->name), keyP, dataP);
      return HASH_TABLE_OK;
    }

    node = node->next;
  }

  if (!(node = malloc(sizeof(hash_node_uint64_t)))) return -1;

  node->key = keyP;
  node->data = dataP;

  if (hashtblP->nodes[hash]) {
    node->next = hashtblP->nodes[hash];
  } else {
    node->next = NULL;
  }

  hashtblP->nodes[hash] = node;
  hashtblP->num_elements += 1;

  PRINT_HASHTABLE(hashtblP,
                  "%s(%s,key 0x%" PRIx64 " data %" PRIx64 ") return OK\n",
                  __FUNCTION__, bdata(hashtblP->name), keyP, dataP);
  return HASH_TABLE_OK;
}

//------------------------------------------------------------------------------
/*
   Adding a new element
   To make sure the hash value is not bigger than size, the result of the user
   provided hash function is used modulo size.
*/
hashtable_rc_t hashtable_uint64_ts_insert(
    hash_table_uint64_ts_t* const hashtblP, const hash_key_t keyP,
    const uint64_t dataP) {
  hash_node_uint64_t* node = NULL;
  hash_size_t hash = 0;

  if (!hashtblP) {
    return HASH_TABLE_BAD_PARAMETER_HASHTABLE;
  }

  hash = hashtblP->hashfunc(keyP) % hashtblP->size;
  pthread_mutex_lock(&hashtblP->lock_nodes[hash]);
  node = hashtblP->nodes[hash];

  while (node) {
    if (node->key == keyP) {
      if (node->data != dataP) {
        node->data = dataP;
        pthread_mutex_unlock(&hashtblP->lock_nodes[hash]);
        PRINT_HASHTABLE(hashtblP,
                        "%s(%s,key 0x%" PRIx64 " data %" PRIx64
                        ") return INSERT_OVERWRITTEN_DATA\n",
                        __FUNCTION__, bdata(hashtblP->name), keyP, dataP);
        return HASH_TABLE_INSERT_OVERWRITTEN_DATA;
      }
      pthread_mutex_unlock(&hashtblP->lock_nodes[hash]);
      PRINT_HASHTABLE(hashtblP,
                      "%s(%s,key 0x%" PRIx64 " data %" PRIx64 ") return OK\n",
                      __FUNCTION__, bdata(hashtblP->name), keyP, dataP);
      return HASH_TABLE_SAME_KEY_VALUE_EXISTS;
    }

    node = node->next;
  }

  if (!(node = malloc(sizeof(hash_node_uint64_t)))) return -1;

  node->key = keyP;
  node->data = dataP;

  if (hashtblP->nodes[hash]) {
    node->next = hashtblP->nodes[hash];
  } else {
    node->next = NULL;
  }

  hashtblP->nodes[hash] = node;
  __sync_fetch_and_add(&hashtblP->num_elements, 1);
  pthread_mutex_unlock(&hashtblP->lock_nodes[hash]);
  PRINT_HASHTABLE(hashtblP,
                  "%s(%s,key 0x%" PRIx64 " data %p) next %p return OK\n",
                  __FUNCTION__, bdata(hashtblP->name), keyP, dataP, node->next);
  return HASH_TABLE_OK;
}

//------------------------------------------------------------------------------
/*
   To remove an element from the hash table, we just search for it in the linked
   list for that hash value, and remove it if it is found. If it was not found,
   it is an error and -1 is returned.
*/
hashtable_rc_t hashtable_uint64_remove(hash_table_uint64_t* const hashtblP,
                                       const hash_key_t keyP) {
  hash_node_uint64_t *node, *prevnode = NULL;
  hash_size_t hash = 0;

  if (!hashtblP) {
    return HASH_TABLE_BAD_PARAMETER_HASHTABLE;
  }

  hash = hashtblP->hashfunc(keyP) % hashtblP->size;
  node = hashtblP->nodes[hash];

  while (node) {
    if (node->key == keyP) {
      if (prevnode)
        prevnode->next = node->next;
      else
        hashtblP->nodes[hash] = node->next;

      free_wrapper((void**)&node);
      __sync_fetch_and_sub(&hashtblP->num_elements, 1);
      PRINT_HASHTABLE(hashtblP, "%s(%s,key 0x%" PRIx64 ") return OK\n",
                      __FUNCTION__, bdata(hashtblP->name), keyP);
      return HASH_TABLE_OK;
    }

    prevnode = node;
    node = node->next;
  }

  PRINT_HASHTABLE(hashtblP, "%s(%s,key 0x%" PRIx64 ") return KEY_NOT_EXISTS\n",
                  __FUNCTION__, bdata(hashtblP->name), keyP);
  return HASH_TABLE_KEY_NOT_EXISTS;
}

//------------------------------------------------------------------------------
/*
   To remove an element from the hash table, we just search for it in the linked
   list for that hash value, and remove it if it is found. If it was not found,
   it is an error and -1 is returned.
*/
hashtable_rc_t hashtable_uint64_ts_remove(
    hash_table_uint64_ts_t* const hashtblP, const hash_key_t keyP) {
  hash_node_uint64_t *node, *prevnode = NULL;
  hash_size_t hash = 0;

  if (!hashtblP) {
    return HASH_TABLE_BAD_PARAMETER_HASHTABLE;
  }

  hash = hashtblP->hashfunc(keyP) % hashtblP->size;
  pthread_mutex_lock(&hashtblP->lock_nodes[hash]);
  node = hashtblP->nodes[hash];

  while (node) {
    if (node->key == keyP) {
      if (prevnode)
        prevnode->next = node->next;
      else
        hashtblP->nodes[hash] = node->next;

      free_wrapper((void**)&node);
      __sync_fetch_and_sub(&hashtblP->num_elements, 1);
      pthread_mutex_unlock(&hashtblP->lock_nodes[hash]);
      PRINT_HASHTABLE(hashtblP, "%s(%s,key 0x%" PRIx64 ") return OK\n",
                      __FUNCTION__, bdata(hashtblP->name), keyP);
      return HASH_TABLE_OK;
    }

    prevnode = node;
    node = node->next;
  }
  pthread_mutex_unlock(&hashtblP->lock_nodes[hash]);

  PRINT_HASHTABLE(hashtblP, "%s(%s,key 0x%" PRIx64 ") return KEY_NOT_EXISTS\n",
                  __FUNCTION__, bdata(hashtblP->name), keyP);
  return HASH_TABLE_KEY_NOT_EXISTS;
}

//------------------------------------------------------------------------------
/*
   Searching for an element is easy. We just search through the linked list for
   the corresponding hash value. NULL is returned if we didn't find it.
*/
hashtable_rc_t hashtable_uint64_ts_get(
    const hash_table_uint64_ts_t* const hashtblP, const hash_key_t keyP,
    uint64_t* const dataP) {
  hash_node_uint64_t* node = NULL;
  hash_size_t hash = 0;

  if (!hashtblP) {
    return HASH_TABLE_BAD_PARAMETER_HASHTABLE;
  }

  hash = hashtblP->hashfunc(keyP) % hashtblP->size;

  pthread_mutex_lock(&hashtblP->lock_nodes[hash]);
  node = hashtblP->nodes[hash];

  while (node) {
    if (node->key == keyP) {
      *dataP = node->data;
      pthread_mutex_unlock(&hashtblP->lock_nodes[hash]);
      PRINT_HASHTABLE(hashtblP, "%s(%s,key 0x%" PRIx64 " data %p) return OK\n",
                      __FUNCTION__, bdata(hashtblP->name), keyP, *dataP);
      return HASH_TABLE_OK;
    }

    node = node->next;
  }
  pthread_mutex_unlock(&hashtblP->lock_nodes[hash]);
  PRINT_HASHTABLE(hashtblP, "%s(%s,key 0x%" PRIx64 ") return KEY_NOT_EXISTS\n",
                  __FUNCTION__, bdata(hashtblP->name), keyP);
  return HASH_TABLE_KEY_NOT_EXISTS;
}
