/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: ARRAY implementation
 * Author: z00118096
 * Create: 2014-10-27
 * History: 2018-10-06 Zhang Jie (employee ID: 00316590) Rectification by UK
 */

#include "wsecv2_array.h"
#include "securec.h"
#include "wsecv2_mem.h"

/*
 * Ensure that the array space is available.
 * When an element needs to be added to an array, check the current number of elements (countUsed) and allocation.
 * Relationship between the lengths (countMax). If the space is insufficient, expand the space by using growNum.
 * Indicates the memory.
 */
static WsecBool ConfirmSpaceEnough(WsecArrayData *arrData)
{
    WsecAddr *newMem = NULL;
    WsecVoid *ptr = NULL;
    size_t sizeOld;
    size_t sizeNew;

    WSEC_ASSERT(arrData != NULL);
    /* Sufficient space */
    if (arrData->countMax > arrData->countUsed) {
        return WSEC_TRUE;
    }

    if (arrData->growNum < 1) {
        arrData->growNum = WSEC_ARR_GROW_BY;
    }

    sizeOld = (size_t)WSEC_ARR_MEM_SIZE((WsecUint32)arrData->countMax);
    sizeNew = (size_t)WSEC_ARR_MEM_SIZE((WsecUint32)(arrData->countMax + arrData->growNum));

    newMem = (WsecAddr *)WSEC_MALLOC(sizeNew);
    if (newMem == NULL) {
        return WSEC_FALSE;
    }

    /* Migrate the old memory to the new memory. */
    if (arrData->itemAddr != NULL) {
        /* If the reparenting fails, the new is released and the old is retained. */
        if (memcpy_s(newMem, sizeNew, arrData->itemAddr, sizeOld) != EOK) {
            ptr = (WsecVoid *)newMem;
            WSEC_FREE(ptr);
            return WSEC_FALSE;
        }

        /* The reparenting is successful and the old is released. */
        ptr = (WsecVoid *)(arrData->itemAddr);
        WSEC_FREE(ptr);
    }

    arrData->itemAddr = newMem;
    arrData->countMax += arrData->growNum;

    return WSEC_TRUE;
}

/*
 * Initializes an array. You need to enter the element size, number of elements,
 * array increase step, element comparison callback, and element deletion callback.
 */
WsecArray WsecArrInitialize(int elementNum, WsecUint32 elementSize,
    int growNum,
    CallbackCompare cmpElement, CallbackRemoveElement removeElement)
{
    WsecArrayData *data = NULL;

    data = (WsecArrayData *)WSEC_MALLOC(sizeof(WsecArrayData));
    if (data == NULL) {
        return (WsecArray)data;
    }

    data->growNum = growNum; /* You do not need to check this item. Perform this check when expanding the memory. */
    data->cmpElement = cmpElement;
    data->removeElement = removeElement;
    data->elementSize = elementSize;
    data->itemAddr = NULL;
    data->countUsed = 0;
    data->countMax = 0;

    if (elementNum <= 0) {
        return (WsecArray)data;
    }

    data->itemAddr = (WsecAddr *)WSEC_MALLOC(WSEC_ARR_MEM_SIZE((WsecUint32)elementNum));
    if (data->itemAddr != NULL) {
        data->countMax = elementNum;
    } else {
        WSEC_FREE(data);
    }

    return (WsecArray)data;
}

/* Deinitializes an array. */
WsecArray WsecArrFinalize(WsecArray arr)
{
    if (arr != NULL) {
        WsecArrRemoveAll(arr);
        WSEC_FREE(arr);
    }

    return NULL;
}

/* Obtains a location array element. */
WsecVoid *WsecArrGetAt(const WsecVoid *arr, int idx)
{
    const WsecArrayData *arrData = (const WsecArrayData *)arr;
    WsecVoid *itemValue = NULL;

    WSEC_ASSERT(arr != NULL);
    /* DTS2017042407037 The index is not checked. */
    if (idx >= 0 && idx < arrData->countUsed) {
        itemValue = *(arrData->itemAddr + idx);
    }

    return itemValue;
}

/* Adds an array element to the end of the array. */
int WsecArrAdd(WsecArray arr, WsecVoid *element)
{
    WsecArrayData *arrData = (WsecArrayData *)arr;
    int idx = -1;

    WSEC_ASSERT(arr != NULL);
    WSEC_ASSERT(element != NULL);
    if (ConfirmSpaceEnough(arrData) == WSEC_TRUE) {
        idx = arrData->countUsed;
        *(arrData->itemAddr + idx) = element;
        arrData->countUsed++;
    }
    return idx;
}

/* Add Array Element in Ascending Order */
int WsecArrAddOrderly(WsecArray arr, WsecVoid *element)
{
    WsecArrayData *arrData = (WsecArrayData *)arr;
    int i;
    int insertAt = -1;

    if (!(arr != NULL && element != NULL && arrData->cmpElement != NULL)) {
        return -1;
    }
    if (ConfirmSpaceEnough(arrData) == WSEC_FALSE) {
        return -1;
    }

    for (i = 0; i < arrData->countUsed; i++) {
        if (arrData->cmpElement(&element, arrData->itemAddr + i) < 0) {
            insertAt = i;
            break;
        }
    }

    if (insertAt < 0) {
        insertAt = arrData->countUsed;
    }

    if (WsecArrInsertAt(arr, insertAt, element) == WSEC_FALSE) {
        insertAt = -1;
    }

    return insertAt;
}

/* Cloning and Adding Elements in Sequence */
int WsecArrCloneAddOrderly(WsecArray arr, const WsecVoid *element, size_t elementSize)
{
    WsecVoid *cloneElement = WSEC_CLONE_BUFF(element, elementSize);
    if (cloneElement == NULL) {
        return -1;
    }
    return WsecArrAddOrderly(arr, cloneElement);
}

/* Adds an array element to a specified position. */
WsecBool WsecArrInsertAt(WsecArray arr, int idx, WsecVoid *element)
{
    WsecArrayData *arrData = (WsecArrayData *)arr;
    int i;

    WSEC_ASSERT(arrData != NULL);
    WSEC_ASSERT(element != NULL);
    /* A maximum of xxx can be inserted. */
    if ((idx < 0) || (idx > arrData->countUsed)) {
        return WSEC_FALSE;
    }
    if (ConfirmSpaceEnough(arrData) == WSEC_FALSE) {
        return WSEC_FALSE;
    }

    arrData->countUsed++;

    /* [idx, countUsed - 1] moves rightwards. */
    for (i = arrData->countUsed - 1; i > idx; i--) {    /* soter 554 */
        *(arrData->itemAddr + i) = *(arrData->itemAddr + i - 1);
    }

    /* Write to-be-inserted data */
    *(arrData->itemAddr + idx) = element;

    return WSEC_TRUE;
}

/* Deletes an array element at a specified location. */
WsecVoid WsecArrRemoveAt(WsecArray arr, int idx)
{
    WsecArrayData *arrData = (WsecArrayData *)arr;
    int i;

    WSEC_ASSERT(arr != NULL);
    /* DTS2017042407037 The index is not checked. */
    if (idx < 0 || idx >= arrData->countUsed) {
        return;
    }

    /* Delete the element. */
    if (arrData->removeElement != NULL) {
        arrData->removeElement(*(arrData->itemAddr + idx), arrData->elementSize);
    }

    /* [idx + 1, countUsed - 1] Move a position to the left. */
    for (i = idx + 1; i <= arrData->countUsed - 1; i++) {   /* soter 554 */
        *(arrData->itemAddr + i - 1) = *(arrData->itemAddr + i);
    }

    arrData->countUsed--;
}

/* Deleting All Elements */
WsecVoid WsecArrRemoveAll(WsecArray arr)
{
    WsecArrayData *arrData = (WsecArrayData *)arr;
    WsecVoid **elementPointer = NULL;
    int i;

    if (arrData == NULL) {
        return;
    }
    if (arrData->itemAddr == NULL) {
        return;
    }

    if (arrData->removeElement != NULL) {
        elementPointer = arrData->itemAddr;
        for (i = 0; i < arrData->countUsed; i++, elementPointer++) {    /* soter 573 */
            arrData->removeElement(*elementPointer, arrData->elementSize);
        }
    }

    arrData->countUsed = 0;
    arrData->countMax = 0;
    WSEC_FREE(arrData->itemAddr);
}

/* Obtains the number of elements in the current array. */
int WsecArrGetCount(const WsecArray arr)
{
    WsecArrayData *arrData = (WsecArrayData *)arr;
    return (arrData != NULL) ? arrData->countUsed : 0;
}

/*
 * Sorts arrays quickly.
 * Note: The application must provide a callback function for comparing the sizes of two elements
 * in the WsecArrInitialize() call.
 * The two parameters in the callback function are the addresses of the two elements
 * involved in the comparison, and the elements in the array.
 * The data address provided by the application is stored. Therefore,*a is the pointer pointing to the application data
 */
WsecVoid WsecArrQuickSort(WsecArray arr)
{
    WsecArrayData *arrData = (WsecArrayData *)arr;

    if (arrData != NULL && arrData->itemAddr != NULL && arrData->cmpElement != NULL) {
        qsort((WsecVoid *)arrData->itemAddr,
            (size_t)(unsigned int)arrData->countUsed,
            sizeof(WsecVoid *), arrData->cmpElement);
    }
}

/*
 * dichotomy lookup array
 * key: pointer to the keyword (level 1)
 * Returns the address of the data that matches the keyword.
 * Note
 * (1) Ensure that the array arr is sorted in ascending order.
 * (2) During the search process, the callback function for comparing the sizes of two elements needs to be invoked.
 * This function is the same as the sorting callback function.
 * It should be provided when WsecArrInitialize() is called.
 */
WsecVoid *WsecArrBinarySearch(const WsecArray arr, const WsecVoid *key)
{
    WsecArrayData *arrData = (WsecArrayData *)arr;
    WsecAddr *foundAddr = NULL;
    WsecVoid *foundData = NULL;

    if (WsecArrGetCount(arr) > 0) {
        WSEC_ASSERT(arrData != NULL);
        WSEC_ASSERT(key != NULL);
        WSEC_ASSERT(arrData->cmpElement != NULL);
        if (arrData->itemAddr == NULL) {
            return foundData;
        }

        foundAddr = (WsecAddr *)bsearch((const WsecVoid *)&key,
            (WsecVoid *)arrData->itemAddr,
            (size_t)(unsigned int)arrData->countUsed,
            sizeof(WsecVoid *),
            arrData->cmpElement);
        if (foundAddr != NULL) {
            foundData = *foundAddr;
        }
    }

    return foundData;
}

/*
 * dichotomy lookup array
 * The return value -1 indicates failure. The non-negative number is the subscript of the data that
 * matches the keyword in the array.
 * Note
 * (1) Ensure that the arrays are sorted in ascending order.
 * (2) During the search process, the callback function for comparing the sizes of two elements needs to be invoked.
 * This function is the same as the sorting callback function.
 * It should be provided when WsecArrInitialize() is called.
 */
int WsecArrBinarySearchAt(const WsecArray arr, const WsecVoid *key)
{
    WsecArrayData *arrData = (WsecArrayData *)arr;
    WsecAddr *foundAddr = NULL;
    int idx = -1;
    size_t idxItem;

    if (WsecArrGetCount(arr) > 0) {
        WSEC_ASSERT(arrData != NULL);
        WSEC_ASSERT(arrData->itemAddr != NULL);
        WSEC_ASSERT(key != NULL);
        WSEC_ASSERT(arrData->cmpElement != NULL);
        foundAddr = (WsecAddr *)bsearch((const WsecVoid *)&key,
            (WsecVoid *)arrData->itemAddr,
            (size_t)(unsigned int)arrData->countUsed,
            sizeof(WsecVoid *),
            arrData->cmpElement);
        if (foundAddr != NULL) {
            idxItem = (size_t)(foundAddr - arrData->itemAddr);
            idx = (int)idxItem;
        }
    }

    return idx;
}

/*
 * Standard deletion array node.
 * The array node stores the user's data address, which is used for dynamic application.
 * memory, so memory should be freed when deleting the element.
 */
WsecVoid WsecArrStdRemoveElement(WsecVoid *element, WsecUint32 elementSize)
{
    WSEC_CLEAR_FREE(element, elementSize);
}
