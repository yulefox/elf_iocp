/**
 * Copyright (C) 2010 - 2011 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 *
 * @file hash.h
 * @author Fox (yulefox at gmail.com)
 * @date 2010-12-21
 *
 * @brief Generic hash table implemented by using separate chaining algorithm
 * like SGI STL.
 *
 * 本身不支持多线程, 如果需要, 可以在应用层添加互斥.
 */

#ifndef PUBLIC_HASH_H
#define PUBLIC_HASH_H

#include "def.h"
#include "def/type.h"

BEGIN_C_DECLS

/**
 * @struct hash_ops_t
 * Contain operational functions for a hash table instance.
 * @a hash hash the key of an item into an integer. 
 * @a compare compare keys of items.
 * @a key_dup duplicate the key when create and insert a new item.
 * @a key_free release the key when remove an item.
 * @a data_dup duplicate the data when create and insert a new item.
 * @a data_free release the data when remove an item.
 * @note For keys with type of const string within a single thread, @a key_dup
 * and @a key_free are not needed. If dup_func is given, release_func must be
 * given, too.
 * @see hash_create().
 */

struct hash_ops_t {
    hash_func hash;
    compare_func compare;
    dup_func key_dup;
    release_func key_free;
    dup_func data_dup;
    release_func data_free;
};

/**
 * Create a new hash table.
 * @param size initial table size. If 0, then HASH_DEFAULT_SIZE(10) is used.
 * @param ops operational functions. If NULL, then string keys are assumed and
 * no memory (de)allocation is performed for keys and data. Thus, if you wanna
 * set the key in other type (such as uuid_t), a custom `hash_ops_t` is needed.
 * @return pointer to a dynamically allocated hash table.
 */
ELF_API struct hash_t *hash_create(size_t size, const struct hash_ops_t *ops);

/**
 * Destroy a hash table.
 * @param table pointer to the hash table to be released.
 * @return 0 if successful.
 */
ELF_API int hash_destroy(struct hash_t *t);

/**
 * Get length of a hash table.
 * @param table pointer to the hash table to be measured.
 * @return length of the hash table.
 */
ELF_API size_t hash_len(struct hash_t *t);

/**
 * Find the data associated to the given key in the hash table.
 * @param table pointer to the hash table.
 * @param key key associated to the data.
 * @return pointer to the found data or NULL.
 */
ELF_API void *hash_find(struct hash_t *t, const void *key);
ELF_API void *hash_find_n(struct hash_t *t, int key);

/**
 * Insert data associated with the given key into the hash table. If the
 * key already exists, the old data is freed according to the memory management
 * operations passed in hash_create().
 * @param table pointer to the hash table.
 * @param key key associated to the data.
 * @param data data associated to the key.
 * @return 0 if successful.
 */
ELF_API int hash_insert(struct hash_t *t, const void *key, void *data);
ELF_API int hash_insert_n(struct hash_t *t, int key, void *data);

/**
 * Remove an iterator from the hash table.
 * @param table pointer to the hash table.
 * @param key the key associated to the iterator.
 */
ELF_API int hash_remove(struct hash_t *t, const void *key);
ELF_API int hash_remove_n(struct hash_t *t, int key);
ELF_API int hash_remove_i(struct hash_t *t, struct hash_iterator_t *itr);

ELF_API int hash_begin(struct hash_t *t, struct hash_iterator_t **itr);
ELF_API int hash_next(struct hash_t *t, struct hash_iterator_t **itr);
ELF_API int hash_prev(struct hash_t *t, struct hash_iterator_t **itr);
ELF_API int hash_key(struct hash_iterator_t *itr, void **key);
ELF_API int hash_key_n(struct hash_iterator_t *itr, int *key);
ELF_API int hash_val(struct hash_iterator_t *itr, void **data);

END_C_DECLS

#endif /* !ELF_HASH_H */
