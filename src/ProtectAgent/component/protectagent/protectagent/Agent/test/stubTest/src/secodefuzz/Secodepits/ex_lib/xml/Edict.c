/*
 * dict.c: dictionary of reusable strings, just used to avoid allocation
 *         and freeing operations.
 *
 * Copyright (C) 2003 Daniel Veillard.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE AUTHORS AND
 * CONTRIBUTORS ACCEPT NO RESPONSIBILITY IN ANY CONCEIVABLE MANNER.
 *
 * Author: daniel@veillard.com
 */

#define IN_LIBXML

#include "Etree.h"

#define MAX_HASH_LEN 4
#define MIN_DICT_SIZE 128
#define MAX_DICT_HASH 8 * 2048

/* #define ALLOW_REMOVAL */

/*
 * An entry in the dictionnary
 */
typedef struct _xmlDictEntry xmlDictEntry;
typedef xmlDictEntry *xmlDictEntryPtr;
struct _xmlDictEntry {
    struct _xmlDictEntry *next;
    const hw_xmlChar *name;
    int len;
    int valid;
};

typedef struct _xmlDictStrings xmlDictStrings;
typedef xmlDictStrings *xmlDictStringsPtr;
struct _xmlDictStrings {
    xmlDictStringsPtr next;
    hw_xmlChar *free;
    hw_xmlChar *end;
    int size;
    int nbStrings;
    hw_xmlChar array[1];
};
/*
 * The entire dictionnary
 */
struct _xmlDict {
    int ref_counter;

    struct _xmlDictEntry *dict;
    int size;
    int nbElems;
    xmlDictStringsPtr strings;

    struct _xmlDict *subdict;
};

/*
 * Whether the dictionary mutex was initialized.
 */
static int xmlDictInitialized = 0;

/**
 * xmlInitializeDict:
 *
 * Do the dictionary mutex initialization.
 * this function is not thread safe, initialization should
 * preferably be done once at startup
 */
static int xmlInitializeDict(void) {
    if (xmlDictInitialized)
        return(1);


    xmlDictInitialized = 1;
    return(1);
}

/**
 * hw_xmlDictCleanup:
 *
 * Free the dictionary mutex.
 */
void
hw_xmlDictCleanup(void) {
    if (!xmlDictInitialized)
        return;

    xmlDictInitialized = 0;
}

/*
 * xmlDictAddString:
 * @dict: the dictionnary
 * @name: the name of the userdata
 * @len: the length of the name, if -1 it is recomputed
 *
 * Add the string to the array[s]
 *
 * Returns the pointer of the local string, or NULL in case of error.
 */
static const hw_xmlChar *
xmlDictAddString(hw_xmlDictPtr dict, const hw_xmlChar *name, int namelen) {
    xmlDictStringsPtr pool;
    const hw_xmlChar *ret;
    int size = 0; /* + sizeof(_xmlDictStrings) == 1024 */

    pool = dict->strings;
    while (pool != NULL) {
	if (pool->end - pool->free > namelen)
	    goto found_pool;
	if (pool->size > size) size = pool->size;
	pool = pool->next;
    }
    /*
     * Not found, need to allocate
     */
    if (pool == NULL) {
        if (size == 0) size = 1000;
	else size *= 4; /* exponential growth */
        if (size < 4 * namelen) 
	    size = 4 * namelen; /* just in case ! */
	pool = (xmlDictStringsPtr) hw_xmlMalloc(sizeof(xmlDictStrings) + size);
	if (pool == NULL)
	    return(NULL);
	pool->size = size;
	pool->nbStrings = 0;
	pool->free = &pool->array[0];
	pool->end = &pool->array[size];
	pool->next = dict->strings;
	dict->strings = pool;
    }
found_pool:
    ret = pool->free;
    memcpy(pool->free, name, namelen);
    pool->free += namelen;
    *(pool->free++) = 0;
    return(ret);
}

/*
 * xmlDictComputeKey:
 * Calculate the hash key
 */
static unsigned long
xmlDictComputeKey(const hw_xmlChar *name, int namelen) {
    unsigned long value = 0L;
    
    if (name == NULL) return(0);
    value = *name;
    value <<= 5;
    if (namelen > 10) {
        value += name[namelen - 1];
        namelen = 10;
    }
    switch (namelen) {
        case 10: value += name[9];
        case 9: value += name[8];
        case 8: value += name[7];
        case 7: value += name[6];
        case 6: value += name[5];
        case 5: value += name[4];
        case 4: value += name[3];
        case 3: value += name[2];
        case 2: value += name[1];
        default: break;
    }
    return(value);
}

/**
 * hw_xmlDictCreate:
 *
 * Create a new dictionary
 *
 * Returns the newly created dictionnary, or NULL if an error occured.
 */
hw_xmlDictPtr
hw_xmlDictCreate(void) {
    hw_xmlDictPtr dict;

    if (!xmlDictInitialized)
        if (!xmlInitializeDict())
            return(NULL);
 
    dict = hw_xmlMalloc(sizeof(hw_xmlDict));
    if (dict) {
        dict->ref_counter = 1;

        dict->size = MIN_DICT_SIZE;
	dict->nbElems = 0;
        dict->dict = hw_xmlMalloc(MIN_DICT_SIZE * sizeof(xmlDictEntry));
	dict->strings = NULL;
	dict->subdict = NULL;
        if (dict->dict) {
                memset(dict->dict, 0, MIN_DICT_SIZE * sizeof(xmlDictEntry));
                return(dict);
        }
        hw_xmlFree(dict);
    }
    return(NULL);
}


/**
 * hw_xmlDictReference:
 * @dict: the dictionnary
 *
 * Increment the reference counter of a dictionary
 *
 * Returns 0 in case of success and -1 in case of error
 */
int
hw_xmlDictReference(hw_xmlDictPtr dict) {
    if (!xmlDictInitialized)
        if (!xmlInitializeDict())
            return(-1);

    if (dict == NULL) return -1;
    dict->ref_counter++;
    return(0);
}

/**
 * xmlDictGrow:
 * @dict: the dictionnary
 * @size: the new size of the dictionnary
 *
 * resize the dictionnary
 *
 * Returns 0 in case of success, -1 in case of failure
 */
static int
xmlDictGrow(hw_xmlDictPtr dict, int size) {
    unsigned long key;
    int oldsize, i;
    xmlDictEntryPtr iter, next;
    struct _xmlDictEntry *olddict;
  
    if (dict == NULL)
	return(-1);
    if (size < 8)
        return(-1);
    if (size > 8 * 2048)
	return(-1);

    oldsize = dict->size;
    olddict = dict->dict;
    if (olddict == NULL)
        return(-1);
  
    dict->dict = hw_xmlMalloc(size * sizeof(xmlDictEntry));
    if (dict->dict == NULL) {
	dict->dict = olddict;
	return(-1);
    }
    memset(dict->dict, 0, size * sizeof(xmlDictEntry));
    dict->size = size;

    /*	If the two loops are merged, there would be situations where
	a new entry needs to allocated and data copied into it from 
	the main dict. So instead, we run through the array twice, first
	copying all the elements in the main array (where we can't get
	conflicts) and then the rest, so we only free (and don't allocate)
    */
    for (i = 0; i < oldsize; i++) {
	if (olddict[i].valid == 0) 
	    continue;
	key = xmlDictComputeKey(olddict[i].name, olddict[i].len) % dict->size;
	memcpy(&(dict->dict[key]), &(olddict[i]), sizeof(xmlDictEntry));
	dict->dict[key].next = NULL;
    }

    for (i = 0; i < oldsize; i++) {
	iter = olddict[i].next;
	while (iter) {
	    next = iter->next;

	    /*
	     * put back the entry in the new dict
	     */

	    key = xmlDictComputeKey(iter->name, iter->len) % dict->size;
	    if (dict->dict[key].valid == 0) {
		memcpy(&(dict->dict[key]), iter, sizeof(xmlDictEntry));
		dict->dict[key].next = NULL;
		dict->dict[key].valid = 1;
		hw_xmlFree(iter);
	    } else {
	    	iter->next = dict->dict[key].next;
	    	dict->dict[key].next = iter;
	    }


	    iter = next;
	}
    }

    hw_xmlFree(olddict);


    return(0);
}

/**
 * hw_xmlDictFree:
 * @dict: the dictionnary
 *
 * Free the hash @dict and its contents. The userdata is
 * deallocated with @f if provided.
 */
void
hw_xmlDictFree(hw_xmlDictPtr dict) {
    int i;
    xmlDictEntryPtr iter;
    xmlDictEntryPtr next;
    int inside_dict = 0;
    xmlDictStringsPtr pool, nextp;

    if (dict == NULL)
	return;

    if (!xmlDictInitialized)
        if (!xmlInitializeDict())
            return;

    /* decrement the counter, it may be shared by a parser and docs */
    dict->ref_counter--;
    if (dict->ref_counter > 0) {
        return;
    }


    if (dict->subdict != NULL) {
        hw_xmlDictFree(dict->subdict);
    }

    if (dict->dict) {
	for(i = 0; ((i < dict->size) && (dict->nbElems > 0)); i++) {
	    iter = &(dict->dict[i]);
	    if (iter->valid == 0)
		continue;
	    inside_dict = 1;
	    while (iter) {
		next = iter->next;
		if (!inside_dict)
		    hw_xmlFree(iter);
		dict->nbElems--;
		inside_dict = 0;
		iter = next;
	    }
	    inside_dict = 0;
	}
	hw_xmlFree(dict->dict);
    }
    pool = dict->strings;
    while (pool != NULL) {
        nextp = pool->next;
	hw_xmlFree(pool);
	pool = nextp;
    }
    hw_xmlFree(dict);
}

/**
 * hw_xmlDictLookup:
 * @dict: the dictionnary
 * @name: the name of the userdata
 * @len: the length of the name, if -1 it is recomputed
 *
 * Add the @name to the dictionnary @dict if not present.
 *
 * Returns the internal copy of the name or NULL in case of internal error
 */
const hw_xmlChar *
hw_xmlDictLookup(hw_xmlDictPtr dict, const hw_xmlChar *name, int len) {
    unsigned long key, okey, nbi = 0;
    xmlDictEntryPtr entry;
    xmlDictEntryPtr insert;
    const hw_xmlChar *ret;

    if ((dict == NULL) || (name == NULL))
	return(NULL);

    if (len < 0)
        len = hw_xmlStrlen(name);

    /*
     * Check for duplicate and insertion location.
     */
    okey = xmlDictComputeKey(name, len);
    key = okey % dict->size;
    if (dict->dict[key].valid == 0) {
	insert = NULL;
    } else {
	for (insert = &(dict->dict[key]); insert->next != NULL;
	     insert = insert->next) {
#ifdef __GNUC__
	    if (insert->len == len) {
		if (!memcmp(insert->name, name, len))
		    return(insert->name);
	    }
#else
	    if ((insert->len == len) &&
	        (!hw_xmlStrncmp(insert->name, name, len)))
		return(insert->name);
#endif
	    nbi++;
	}
#ifdef __GNUC__
	if (insert->len == len) {
	    if (!memcmp(insert->name, name, len))
		return(insert->name);
	}
#else
	if ((insert->len == len) &&
	    (!hw_xmlStrncmp(insert->name, name, len)))
	    return(insert->name);
#endif
    }

    if (dict->subdict) {
	key = okey % dict->subdict->size;
	if (dict->subdict->dict[key].valid != 0) {
	    xmlDictEntryPtr tmp;

	    for (tmp = &(dict->subdict->dict[key]); tmp->next != NULL;
		 tmp = tmp->next) {
#ifdef __GNUC__
		if (tmp->len == len) {
		    if (!memcmp(tmp->name, name, len))
			return(tmp->name);
		}
#else
		if ((tmp->len == len) &&
		    (!hw_xmlStrncmp(tmp->name, name, len)))
		    return(tmp->name);
#endif
		nbi++;
	    }
#ifdef __GNUC__
	    if (tmp->len == len) {
		if (!memcmp(tmp->name, name, len))
		    return(tmp->name);
	    }
#else
	    if ((tmp->len == len) &&
		(!hw_xmlStrncmp(tmp->name, name, len)))
		return(tmp->name);
#endif
	}
	key = okey % dict->size;
    }

    ret = xmlDictAddString(dict, name, len);
    if (ret == NULL)
        return(NULL);
    if (insert == NULL) {
	entry = &(dict->dict[key]);
    } else {
	entry = hw_xmlMalloc(sizeof(xmlDictEntry));
	if (entry == NULL)
	     return(NULL);
    }
    entry->name = ret;
    entry->len = len;
    entry->next = NULL;
    entry->valid = 1;


    if (insert != NULL) 
	insert->next = entry;

    dict->nbElems++;

    if ((nbi > MAX_HASH_LEN) &&
        (dict->size <= ((MAX_DICT_HASH / 2) / MAX_HASH_LEN)))
	xmlDictGrow(dict, MAX_HASH_LEN * 2 * dict->size);
    /* Note that entry may have been freed at this point by xmlDictGrow */

    return(ret);
}


/**
 * hw_xmlDictOwns:
 * @dict: the dictionnary
 * @str: the string
 *
 * check if a string is owned by the disctionary
 *
 * Returns 1 if true, 0 if false and -1 in case of error
 * -1 in case of error
 */
int
hw_xmlDictOwns(hw_xmlDictPtr dict, const hw_xmlChar *str) {
    xmlDictStringsPtr pool;

    if ((dict == NULL) || (str == NULL))
	return(-1);
    pool = dict->strings;
    while (pool != NULL) {
        if ((str >= &pool->array[0]) && (str <= pool->free))
	    return(1);
	pool = pool->next;
    }
    if (dict->subdict)
        return(hw_xmlDictOwns(dict->subdict, str));
    return(0);
}

