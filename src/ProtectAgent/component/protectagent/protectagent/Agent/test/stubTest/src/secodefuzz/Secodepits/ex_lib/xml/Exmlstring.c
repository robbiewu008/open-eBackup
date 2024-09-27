/*
 * string.c : an XML string utilities module
 *
 * This module provides various utility functions for manipulating
 * the hw_xmlChar* type. All functions named xmlStr* have been moved here
 * from the parser.c file (their original home). 
 *
 * See Copyright for the status of this software.
 *
 * UTF8 string routines from:
 * William Brack <wbrack@mmm.com.hk>
 *
 * daniel@veillard.com
 */

#define IN_LIBXML

#include <stdlib.h>
#include "Etree.h"

/************************************************************************
 *                                                                      *
 *                Commodity functions to handle xmlChars                *
 *                                                                      *
 ************************************************************************/

/**
 * hw_xmlStrndup:
 * @cur:  the input hw_xmlChar *
 * @len:  the len of @cur
 *
 * a strndup for array of hw_xmlChar's
 *
 * Returns a new hw_xmlChar * or NULL
 */
hw_xmlChar *
hw_xmlStrndup(const hw_xmlChar *cur, int len) {
    hw_xmlChar *ret;
    
    if ((cur == NULL) || (len < 0)) return(NULL);
    ret = (hw_xmlChar *) hw_xmlMallocAtomic((len + 1) * sizeof(hw_xmlChar));
    if (ret == NULL) {
        hw_xmlErrMemory(NULL, NULL);
        return(NULL);
    }
    memcpy(ret, cur, len * sizeof(hw_xmlChar));
    ret[len] = 0;
    return(ret);
}

/**
 * hw_xmlStrdup:
 * @cur:  the input hw_xmlChar *
 *
 * a strdup for array of hw_xmlChar's. Since they are supposed to be
 * encoded in UTF-8 or an encoding with 8bit based chars, we assume
 * a termination mark of '0'.
 *
 * Returns a new hw_xmlChar * or NULL
 */
hw_xmlChar *
hw_xmlStrdup(const hw_xmlChar *cur) {
    const hw_xmlChar *p = cur;

    if (cur == NULL) return(NULL);
    while (*p != 0) p++; /* non input consuming */
    return(hw_xmlStrndup(cur, p - cur));
}

/**
 * hw_xmlCharStrndup:
 * @cur:  the input char *
 * @len:  the len of @cur
 *
 * a strndup for char's to hw_xmlChar's
 *
 * Returns a new hw_xmlChar * or NULL
 */

hw_xmlChar *
hw_xmlCharStrndup(const char *cur, int len) {
    int i;
    hw_xmlChar *ret;
    
    if ((cur == NULL) || (len < 0)) return(NULL);
    ret = (hw_xmlChar *) hw_xmlMallocAtomic((len + 1) * sizeof(hw_xmlChar));
    if (ret == NULL) {
        hw_xmlErrMemory(NULL, NULL);
        return(NULL);
    }
    for (i = 0;i < len;i++) {
        ret[i] = (hw_xmlChar) cur[i];
        if (ret[i] == 0) return(ret);
    }
    ret[len] = 0;
    return(ret);
}

/**
 * hw_xmlCharStrdup:
 * @cur:  the input char *
 *
 * a strdup for char's to hw_xmlChar's
 *
 * Returns a new hw_xmlChar * or NULL
 */

hw_xmlChar *
hw_xmlCharStrdup(const char *cur) {
    const char *p = cur;

    if (cur == NULL) return(NULL);
    while (*p != '\0') p++; /* non input consuming */
    return(hw_xmlCharStrndup(cur, p - cur));
}

/**
 * hw_xmlStrcmp:
 * @str1:  the first hw_xmlChar *
 * @str2:  the second hw_xmlChar *
 *
 * a strcmp for hw_xmlChar's
 *
 * Returns the integer result of the comparison
 */

int
hw_xmlStrcmp(const hw_xmlChar *str1, const hw_xmlChar *str2) {
    register int tmp;

    if (str1 == str2) return(0);
    if (str1 == NULL) return(-1);
    if (str2 == NULL) return(1);
    do {
        tmp = *str1++ - *str2;
        if (tmp != 0) return(tmp);
    } while (*str2++ != 0);
    return 0;
}

/**
 * hw_xmlStrEqual:
 * @str1:  the first hw_xmlChar *
 * @str2:  the second hw_xmlChar *
 *
 * Check if both strings are equal of have same content.
 * Should be a bit more readable and faster than hw_xmlStrcmp()
 *
 * Returns 1 if they are equal, 0 if they are different
 */

int
hw_xmlStrEqual(const hw_xmlChar *str1, const hw_xmlChar *str2) {
    if (str1 == str2) return(1);
    if (str1 == NULL) return(0);
    if (str2 == NULL) return(0);
    do {
        if (*str1++ != *str2) return(0);
    } while (*str2++);
    return(1);
}

/**
 * hw_xmlStrncmp:
 * @str1:  the first hw_xmlChar *
 * @str2:  the second hw_xmlChar *
 * @len:  the max comparison length
 *
 * a strncmp for hw_xmlChar's
 *
 * Returns the integer result of the comparison
 */

int
hw_xmlStrncmp(const hw_xmlChar *str1, const hw_xmlChar *str2, int len) {
    register int tmp;

    if (len <= 0) return(0);
    if (str1 == str2) return(0);
    if (str1 == NULL) return(-1);
    if (str2 == NULL) return(1);
#ifdef __GNUC__
    tmp = strncmp((const char *)str1, (const char *)str2, len);
    return tmp;
#else
    do {
        tmp = *str1++ - *str2;
        if (tmp != 0 || --len == 0) return(tmp);
    } while (*str2++ != 0);
    return 0;
#endif
}

static const hw_xmlChar hw_casemap[256] = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
    0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
    0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
    0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
    0x40,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
    0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
    0x78,0x79,0x7A,0x7B,0x5C,0x5D,0x5E,0x5F,
    0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
    0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
    0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
    0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,
    0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
    0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,
    0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
    0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,
    0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
    0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,
    0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,
    0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
    0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,
    0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
    0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,
    0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
    0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,
    0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
};

/**
 * hw_xmlStrcasecmp:
 * @str1:  the first hw_xmlChar *
 * @str2:  the second hw_xmlChar *
 *
 * a strcasecmp for hw_xmlChar's
 *
 * Returns the integer result of the comparison
 */

int
hw_xmlStrcasecmp(const hw_xmlChar *str1, const hw_xmlChar *str2) {
    register int tmp;

    if (str1 == str2) return(0);
    if (str1 == NULL) return(-1);
    if (str2 == NULL) return(1);
    do {
        tmp = hw_casemap[*str1++] - hw_casemap[*str2];
        if (tmp != 0) return(tmp);
    } while (*str2++ != 0);
    return 0;
}

/**
 * hw_xmlStrncasecmp:
 * @str1:  the first hw_xmlChar *
 * @str2:  the second hw_xmlChar *
 * @len:  the max comparison length
 *
 * a strncasecmp for hw_xmlChar's
 *
 * Returns the integer result of the comparison
 */

int
hw_xmlStrncasecmp(const hw_xmlChar *str1, const hw_xmlChar *str2, int len) {
    register int tmp;

    if (len <= 0) return(0);
    if (str1 == str2) return(0);
    if (str1 == NULL) return(-1);
    if (str2 == NULL) return(1);
    do {
        tmp = hw_casemap[*str1++] - hw_casemap[*str2];
        if (tmp != 0 || --len == 0) return(tmp);
    } while (*str2++ != 0);
    return 0;
}

/**
 * hw_xmlStrchr:
 * @str:  the hw_xmlChar * array
 * @val:  the hw_xmlChar to search
 *
 * a strchr for hw_xmlChar's
 *
 * Returns the hw_xmlChar * for the first occurrence or NULL.
 */

const hw_xmlChar *
hw_xmlStrchr(const hw_xmlChar *str, hw_xmlChar val) {
    if (str == NULL) return(NULL);
    while (*str != 0) { /* non input consuming */
        if (*str == val) return((hw_xmlChar *) str);
        str++;
    }
    return(NULL);
}

/**
 * hw_xmlStrstr:
 * @str:  the hw_xmlChar * array (haystack)
 * @val:  the hw_xmlChar to search (needle)
 *
 * a strstr for hw_xmlChar's
 *
 * Returns the hw_xmlChar * for the first occurrence or NULL.
 */

const hw_xmlChar *
hw_xmlStrstr(const hw_xmlChar *str, const hw_xmlChar *val) {
    int n;
    
    if (str == NULL) return(NULL);
    if (val == NULL) return(NULL);
    n = hw_xmlStrlen(val);

    if (n == 0) return(str);
    while (*str != 0) { /* non input consuming */
        if (*str == *val) {
            if (!hw_xmlStrncmp(str, val, n)) return((const hw_xmlChar *) str);
        }
        str++;
    }
    return(NULL);
}

/**
 * hw_xmlStrlen:
 * @str:  the hw_xmlChar * array
 *
 * length of a hw_xmlChar's string
 *
 * Returns the number of hw_xmlChar contained in the ARRAY.
 */

int
hw_xmlStrlen(const hw_xmlChar *str) {
    int len = 0;

    if (str == NULL) return(0);
    while (*str != 0) { /* non input consuming */
        str++;
        len++;
    }
    return(len);
}

/**
 * hw_xmlStrncat:
 * @cur:  the original hw_xmlChar * array
 * @add:  the hw_xmlChar * array added
 * @len:  the length of @add
 *
 * a strncat for array of hw_xmlChar's, it will extend @cur with the len
 * first bytes of @add. Note that if @len < 0 then this is an API error
 * and NULL will be returned.
 *
 * Returns a new hw_xmlChar *, the original @cur is reallocated if needed
 * and should not be freed
 */

hw_xmlChar *
hw_xmlStrncat(hw_xmlChar *cur, const hw_xmlChar *add, int len) {
    int size;
    hw_xmlChar *ret;

    if ((add == NULL) || (len == 0))
        return(cur);
    if (len < 0)
	return(NULL);
    if (cur == NULL)
        return(hw_xmlStrndup(add, len));

    size = hw_xmlStrlen(cur);
    ret = (hw_xmlChar *) hw_xmlRealloc(cur, (size + len + 1) * sizeof(hw_xmlChar));
    if (ret == NULL) {
        hw_xmlErrMemory(NULL, NULL);
        return(cur);
    }
    memcpy(&ret[size], add, len * sizeof(hw_xmlChar));
    ret[size + len] = 0;
    return(ret);
}

/**
 * hw_xmlStrncatNew:
 * @str1:  first hw_xmlChar string
 * @str2:  second hw_xmlChar string
 * @len:  the len of @str2 or < 0
 *
 * same as hw_xmlStrncat, but creates a new string.  The original
 * two strings are not freed. If @len is < 0 then the length
 * will be calculated automatically.
 *
 * Returns a new hw_xmlChar * or NULL
 */
hw_xmlChar *
hw_xmlStrncatNew(const hw_xmlChar *str1, const hw_xmlChar *str2, int len) {
    int size;
    hw_xmlChar *ret;

    if (len < 0)
        len = hw_xmlStrlen(str2);
    if ((str2 == NULL) || (len == 0))
        return(hw_xmlStrdup(str1));
    if (str1 == NULL)
        return(hw_xmlStrndup(str2, len));

    size = hw_xmlStrlen(str1);
    ret = (hw_xmlChar *) hw_xmlMalloc((size + len + 1) * sizeof(hw_xmlChar));
    if (ret == NULL) {
        hw_xmlErrMemory(NULL, NULL);
        return(hw_xmlStrndup(str1, size));
    }
    memcpy(ret, str1, size * sizeof(hw_xmlChar));
    memcpy(&ret[size], str2, len * sizeof(hw_xmlChar));
    ret[size + len] = 0;
    return(ret);
}

/**
 * hw_xmlStrcat:
 * @cur:  the original hw_xmlChar * array
 * @add:  the hw_xmlChar * array added
 *
 * a strcat for array of hw_xmlChar's. Since they are supposed to be
 * encoded in UTF-8 or an encoding with 8bit based chars, we assume
 * a termination mark of '0'.
 *
 * Returns a new hw_xmlChar * containing the concatenated string.
 */
hw_xmlChar *
hw_xmlStrcat(hw_xmlChar *cur, const hw_xmlChar *add) {
    const hw_xmlChar *p = add;

    if (add == NULL) return(cur);
    if (cur == NULL) 
        return(hw_xmlStrdup(add));

    while (*p != 0) p++; /* non input consuming */
    return(hw_xmlStrncat(cur, add, p - add));
}



