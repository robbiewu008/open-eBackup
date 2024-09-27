/*
 * list.c: lists handling implementation
 *
 * Copyright (C) 2000 Gary Pennington and Daniel Veillard.
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
 * Author: Gary.Pennington@uk.sun.com
 */

#define IN_LIBXML

#include <stdlib.h>

#include "Etree.h"
#include "Eparser.h"
/*
 * Type definition are kept internal
 */

struct _xmlLink
{
    struct _xmlLink *next;
    struct _xmlLink *prev;
    void *data;
};

struct _xmlList
{
    hw_xmlLinkPtr sentinel;
    void (*linkDeallocator)(hw_xmlLinkPtr );
    int (*linkCompare)(const void *, const void*);
};

/************************************************************************
 *                                    *
 *                Interfaces                *
 *                                    *
 ************************************************************************/

/**
 * xmlLinkDeallocator:
 * @l:  a list
 * @lk:  a link
 *
 * Unlink and deallocate @lk from list @l
 */
static void
xmlLinkDeallocator(hw_xmlListPtr l, hw_xmlLinkPtr lk)
{
    (lk->prev)->next = lk->next;
    (lk->next)->prev = lk->prev;
    if(l->linkDeallocator)
        l->linkDeallocator(lk);
    hw_xmlFree(lk);
}

/**
 * xmlLinkCompare:
 * @data0:  first data
 * @data1:  second data
 *
 * Compares two arbitrary data
 *
 * Returns -1, 0 or 1 depending on whether data1 is greater equal or smaller
 *          than data0
 */
static int
xmlLinkCompare(const void *data0, const void *data1)
{
    if (data0 < data1)
        return (-1);
    else if (data0 == data1)
	return (0);
    return (1);
}

/**
 * xmlListLowerSearch:
 * @l:  a list
 * @data:  a data
 *
 * Search data in the ordered list walking from the beginning
 *
 * Returns the link containing the data or NULL
 */
static hw_xmlLinkPtr 
xmlListLowerSearch(hw_xmlListPtr l, void *data) 
{
    hw_xmlLinkPtr lk;

    if (l == NULL)
        return(NULL);
    for(lk = l->sentinel->next;lk != l->sentinel && l->linkCompare(lk->data, data) <0 ;lk = lk->next);
    return lk;    
}

/**
 * xmlListHigherSearch:
 * @l:  a list
 * @data:  a data
 *
 * Search data in the ordered list walking backward from the end
 *
 * Returns the link containing the data or NULL
 */
static hw_xmlLinkPtr 
xmlListHigherSearch(hw_xmlListPtr l, void *data) 
{
    hw_xmlLinkPtr lk;

    if (l == NULL)
        return(NULL);
    for(lk = l->sentinel->prev;lk != l->sentinel && l->linkCompare(lk->data, data) >0 ;lk = lk->prev);
    return lk;    
}

/**
 * @l:  a list
 * @data:  a data
 *
 * Search data in the list
 *
 * Returns the link containing the data or NULL
 */
static hw_xmlLinkPtr 
xmlListLinkSearch(hw_xmlListPtr l, void *data) 
{
    hw_xmlLinkPtr lk;
    if (l == NULL)
        return(NULL);
    lk = xmlListLowerSearch(l, data);
    if (lk == l->sentinel)
        return NULL;
    else {
        if (l->linkCompare(lk->data, data) ==0)
            return lk;
        return NULL;
    }
}


/**
 * hw_xmlListCreate:
 * @deallocator:  an optional deallocator function
 * @compare:  an optional comparison function
 *
 * Create a new list
 *
 * Returns the new list or NULL in case of error
 */
hw_xmlListPtr
hw_xmlListCreate(hw_xmlListDeallocator deallocator, hw_xmlListDataCompare compare)
{
    hw_xmlListPtr l;
    if (NULL == (l = (hw_xmlListPtr )hw_xmlMalloc( sizeof(hw_xmlList)))) {
        hw_xmlGenericError(hw_xmlGenericErrorContext, 
		        "Cannot initialize memory for list");
        return (NULL);
    }
    /* Initialize the list to NULL */
    memset(l, 0, sizeof(hw_xmlList));
    
    /* Add the sentinel */
    if (NULL ==(l->sentinel = (hw_xmlLinkPtr )hw_xmlMalloc(sizeof(hw_xmlLink)))) {
        hw_xmlGenericError(hw_xmlGenericErrorContext, 
		        "Cannot initialize memory for sentinel");
	hw_xmlFree(l);
        return (NULL);
    }
    l->sentinel->next = l->sentinel;
    l->sentinel->prev = l->sentinel;
    l->sentinel->data = NULL;
    
    /* If there is a link deallocator, use it */
    if (deallocator != NULL)
        l->linkDeallocator = deallocator;
    /* If there is a link comparator, use it */
    if (compare != NULL)
        l->linkCompare = compare;
    else /* Use our own */
        l->linkCompare = xmlLinkCompare;
    return l;
}
    

/**
 * hw_xmlListInsert:
 * @l:  a list
 * @data:  the data
 *
 * Insert data in the ordered list at the beginning for this value
 *
 * Returns 0 in case of success, 1 in case of failure
 */
int
hw_xmlListInsert(hw_xmlListPtr l, void *data) 
{
    hw_xmlLinkPtr lkPlace, lkNew;

    if (l == NULL)
        return(1);
    lkPlace = xmlListLowerSearch(l, data);
    /* Add the new link */
    lkNew = (hw_xmlLinkPtr) hw_xmlMalloc(sizeof(hw_xmlLink));
    if (lkNew == NULL) {
        hw_xmlGenericError(hw_xmlGenericErrorContext, 
		        "Cannot initialize memory for new link");
        return (1);
    }
    lkNew->data = data;
    lkPlace = lkPlace->prev;
    lkNew->next = lkPlace->next;
    (lkPlace->next)->prev = lkNew;
    lkPlace->next = lkNew;
    lkNew->prev = lkPlace;
    return 0;
}

/**
 * hw_xmlListAppend:
 * @l:  a list
 * @data:  the data
 *
 * Insert data in the ordered list at the end for this value
 *
 * Returns 0 in case of success, 1 in case of failure
 */
int hw_xmlListAppend(hw_xmlListPtr l, void *data) 
{
    hw_xmlLinkPtr lkPlace, lkNew;

    if (l == NULL)
        return(1);
    lkPlace = xmlListHigherSearch(l, data);
    /* Add the new link */
    lkNew = (hw_xmlLinkPtr) hw_xmlMalloc(sizeof(hw_xmlLink));
    if (lkNew == NULL) {
        hw_xmlGenericError(hw_xmlGenericErrorContext, 
		        "Cannot initialize memory for new link");
        return (0);
    }
    lkNew->data = data;
    lkNew->next = lkPlace->next;
    (lkPlace->next)->prev = lkNew;
    lkPlace->next = lkNew;
    lkNew->prev = lkPlace;
    return 1;
}

/**
 * hw_xmlListDelete:
 * @l:  a list
 *
 * Deletes the list and its associated data
 */
void hw_xmlListDelete(hw_xmlListPtr l)
{
    if (l == NULL)
        return;

    hw_xmlListClear(l);
    hw_xmlFree(l->sentinel);
    hw_xmlFree(l);
}

/**
 * hw_xmlListRemoveFirst:
 * @l:  a list
 * @data:  list data
 *
 * Remove the first instance associated to data in the list
 *
 * Returns 1 if a deallocation occured, or 0 if not found
 */
int
hw_xmlListRemoveFirst(hw_xmlListPtr l, void *data)
{
    hw_xmlLinkPtr lk;
    
    if (l == NULL)
        return(0);
    /*Find the first instance of this data */
    lk = xmlListLinkSearch(l, data);
    if (lk != NULL) {
        xmlLinkDeallocator(l, lk);
        return 1;
    }
    return 0;
}


/**
 * hw_xmlListClear:
 * @l:  a list
 *
 * Remove the all data in the list
 */
void
hw_xmlListClear(hw_xmlListPtr l)
{
    hw_xmlLinkPtr  lk;
    
    if (l == NULL)
        return;
    lk = l->sentinel->next;
    while(lk != l->sentinel) {
        hw_xmlLinkPtr next = lk->next;

        xmlLinkDeallocator(l, lk);
        lk = next;
    }
}

    

/**
 * hw_xmlLinkGetData:
 * @lk:  a link
 *
 * See Returns.
 *
 * Returns a pointer to the data referenced from this link
 */
void *
hw_xmlLinkGetData(hw_xmlLinkPtr lk)
{
    if (lk == NULL)
        return(NULL);
    return lk->data;
}


