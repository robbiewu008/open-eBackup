/*
 * encoding.c : implements the encoding conversion functions needed for XML
 *
 * Related specs: 
 * rfc2044        (UTF-8 and UTF-16) F. Yergeau Alis Technologies
 * rfc2781        UTF-16, an encoding of ISO 10646, P. Hoffman, F. Yergeau
 * [ISO-10646]    UTF-8 and UTF-16 in Annexes
 * [ISO-8859-1]   ISO Latin-1 characters codes.
 * [UNICODE]      The Unicode Consortium, "The Unicode Standard --
 *                Worldwide Character Encoding -- Version 1.0", Addison-
 *                Wesley, Volume 1, 1991, Volume 2, 1992.  UTF-8 is
 *                described in Unicode Technical Report #4.
 * [US-ASCII]     Coded Character Set--7-bit American Standard Code for
 *                Information Interchange, ANSI X3.4-1986.
 *
 * See Copyright for the status of this software.
 *
 * daniel@veillard.com
 *
 * Original code for IsoLatin1 and UTF-16 by "Martin J. Duerst" <duerst@w3.org>
 */

#define IN_LIBXML



#include "Etree.h"

static hw_xmlCharEncodingHandlerPtr xmlUTF16LEHandler = NULL;
static hw_xmlCharEncodingHandlerPtr xmlUTF16BEHandler = NULL;

typedef struct _xmlCharEncodingAlias xmlCharEncodingAlias;
typedef xmlCharEncodingAlias *xmlCharEncodingAliasPtr;
struct _xmlCharEncodingAlias {
    const char *name;
    const char *alias;
};

static xmlCharEncodingAliasPtr xmlCharEncodingAliases = NULL;
static int xmlCharEncodingAliasesNb = 0;
static int xmlCharEncodingAliasesMax = 0;



static int xmlLittleEndian = 1;

/**
 * xmlEncodingErrMemory:
 * @extra:  extra informations
 *
 * Handle an out of memory condition
 */
static void
xmlEncodingErrMemory(const char *extra)
{
    hw___xmlSimpleError(XML_FROM_I18N, XML_ERR_NO_MEMORY, NULL, NULL, extra);
}

/**
 * xmlErrEncoding:
 * @error:  the error number
 * @msg:  the error message
 *
 * n encoding error
 */
static void
xmlEncodingErr(hw_xmlParserErrors error, const char *msg, const char *val)
{
    hw___xmlRaiseError(NULL, NULL, NULL, NULL, NULL,
                    XML_FROM_I18N, error, XML_ERR_FATAL,
                    NULL, 0, val, NULL, NULL, 0, 0, msg, val);
}

/************************************************************************
 *									*
 *		Conversions To/From UTF8 encoding			*
 *									*
 ************************************************************************/

/**
 * asciiToUTF8:
 * @out:  a pointer to an array of bytes to store the result
 * @outlen:  the length of @out
 * @in:  a pointer to an array of ASCII chars
 * @inlen:  the length of @in
 *
 * Take a block of ASCII chars in and try to convert it to an UTF-8
 * block of chars out.
 * Returns 0 if success, or -1 otherwise
 * The value of @inlen after return is the number of octets consumed
 *     if the return value is positive, else unpredictable.
 * The value of @outlen after return is the number of octets consumed.
 */
static int
asciiToUTF8(unsigned char* out, int *outlen,
              const unsigned char* in, int *inlen) {
    unsigned char* outstart = out;
    const unsigned char* base = in;
    const unsigned char* processed = in;
    unsigned char* outend = out + *outlen;
    const unsigned char* inend;
    unsigned int c;

    inend = in + (*inlen);
    while ((in < inend) && (out - outstart + 5 < *outlen)) {
	c= *in++;

        if (out >= outend)
	    break;
        if (c < 0x80) {
	    *out++ = c;
	} else { 
	    *outlen = out - outstart;
	    *inlen = processed - base;
	    return(-1);
	}
 
	processed = (const unsigned char*) in;
    }
    *outlen = out - outstart;
    *inlen = processed - base;
    return(*outlen);
}


/**
 * hw_isolat1ToUTF8:
 * @out:  a pointer to an array of bytes to store the result
 * @outlen:  the length of @out
 * @in:  a pointer to an array of ISO Latin 1 chars
 * @inlen:  the length of @in
 *
 * Take a block of ISO Latin 1 chars in and try to convert it to an UTF-8
 * block of chars out.
 * Returns the number of bytes written if success, or -1 otherwise
 * The value of @inlen after return is the number of octets consumed
 *     if the return value is positive, else unpredictable.
 * The value of @outlen after return is the number of octets consumed.
 */
int
hw_isolat1ToUTF8(unsigned char* out, int *outlen,
              const unsigned char* in, int *inlen) {
    unsigned char* outstart = out;
    const unsigned char* base = in;
    unsigned char* outend;
    const unsigned char* inend;
    const unsigned char* instop;

    if ((out == NULL) || (in == NULL) || (outlen == NULL) || (inlen == NULL))
	return(-1);

    outend = out + *outlen;
    inend = in + (*inlen);
    instop = inend;
    
    while (in < inend && out < outend - 1) {
    	if (*in >= 0x80) {
	    *out++ = (((*in) >>  6) & 0x1F) | 0xC0;
        *out++ = ((*in) & 0x3F) | 0x80;
	    ++in;
	}
	if (instop - in > outend - out) instop = in + (outend - out); 
	while (in < instop && *in < 0x80) {
	    *out++ = *in++;
	}
    }	
    if (in < inend && out < outend && *in < 0x80) {
        *out++ = *in++;
    }
    *outlen = out - outstart;
    *inlen = in - base;
    return(*outlen);
}

/**
 * UTF8ToUTF8:
 * @out:  a pointer to an array of bytes to store the result
 * @outlen:  the length of @out
 * @inb:  a pointer to an array of UTF-8 chars
 * @inlenb:  the length of @in in UTF-8 chars
 *
 * No op copy operation for UTF8 handling.
 *
 * Returns the number of bytes written, or -1 if lack of space.
 *     The value of *inlen after return is the number of octets consumed
 *     if the return value is positive, else unpredictable.
 */
static int
UTF8ToUTF8(unsigned char* out, int *outlen,
           const unsigned char* inb, int *inlenb)
{
    int len;

    if ((out == NULL) || (inb == NULL) || (outlen == NULL) || (inlenb == NULL))
	return(-1);
    if (*outlen > *inlenb) {
	len = *inlenb;
    } else {
	len = *outlen;
    }
    if (len < 0)
	return(-1);

    memcpy(out, inb, len);

    *outlen = len;
    *inlenb = len;
    return(*outlen);
}



/**
 * UTF16LEToUTF8:
 * @out:  a pointer to an array of bytes to store the result
 * @outlen:  the length of @out
 * @inb:  a pointer to an array of UTF-16LE passwd as a byte array
 * @inlenb:  the length of @in in UTF-16LE chars
 *
 * Take a block of UTF-16LE ushorts in and try to convert it to an UTF-8
 * block of chars out. This function assumes the endian property
 * is the same between the native type of this machine and the
 * inputed one.
 *
 * Returns the number of bytes written, or -1 if lack of space, or -2
 *     if the transcoding fails (if *in is not a valid utf16 string)
 *     The value of *inlen after return is the number of octets consumed
 *     if the return value is positive, else unpredictable.
 */
static int
UTF16LEToUTF8(unsigned char* out, int *outlen,
            const unsigned char* inb, int *inlenb)
{
    unsigned char* outstart = out;
    const unsigned char* processed = inb;
    unsigned char* outend = out + *outlen;
    unsigned short* in = (unsigned short*) inb;
    unsigned short* inend;
    unsigned int c, d, inlen;
    unsigned char *tmp;
    int bits;

    if ((*inlenb % 2) == 1)
        (*inlenb)--;
    inlen = *inlenb / 2;
    inend = in + inlen;
    while ((in < inend) && (out - outstart + 5 < *outlen)) {
        if (xmlLittleEndian) {
	    c= *in++;
	} else {
	    tmp = (unsigned char *) in;
	    c = *tmp++;
	    c = c | (((unsigned int)*tmp) << 8);
	    in++;
	}
        if ((c & 0xFC00) == 0xD800) {    /* surrogates */
	    if (in >= inend) {           /* (in > inend) shouldn't happens */
		break;
	    }
	    if (xmlLittleEndian) {
		d = *in++;
	    } else {
		tmp = (unsigned char *) in;
		d = *tmp++;
		d = d | (((unsigned int)*tmp) << 8);
		in++;
	    }
            if ((d & 0xFC00) == 0xDC00) {
                c &= 0x03FF;
                c <<= 10;
                c |= d & 0x03FF;
                c += 0x10000;
            }
            else {
		*outlen = out - outstart;
		*inlenb = processed - inb;
	        return(-2);
	    }
        }

	/* assertion: c is a single UTF-4 value */
        if (out >= outend)
	    break;
        if      (c <    0x80) {  *out++=  c;                bits= -6; }
        else if (c <   0x800) {  *out++= ((c >>  6) & 0x1F) | 0xC0;  bits=  0; }
        else if (c < 0x10000) {  *out++= ((c >> 12) & 0x0F) | 0xE0;  bits=  6; }
        else                  {  *out++= ((c >> 18) & 0x07) | 0xF0;  bits= 12; }
 
        for ( ; bits >= 0; bits-= 6) {
            if (out >= outend)
	        break;
            *out++= ((c >> bits) & 0x3F) | 0x80;
        }
	processed = (const unsigned char*) in;
    }
    *outlen = out - outstart;
    *inlenb = processed - inb;
    return(*outlen);
}


/**
 * UTF16BEToUTF8:
 * @out:  a pointer to an array of bytes to store the result
 * @outlen:  the length of @out
 * @inb:  a pointer to an array of UTF-16 passed as a byte array
 * @inlenb:  the length of @in in UTF-16 chars
 *
 * Take a block of UTF-16 ushorts in and try to convert it to an UTF-8
 * block of chars out. This function assumes the endian property
 * is the same between the native type of this machine and the
 * inputed one.
 *
 * Returns the number of bytes written, or -1 if lack of space, or -2
 *     if the transcoding fails (if *in is not a valid utf16 string)
 * The value of *inlen after return is the number of octets consumed
 *     if the return value is positive, else unpredictable.
 */
static int
UTF16BEToUTF8(unsigned char* out, int *outlen,
            const unsigned char* inb, int *inlenb)
{
    unsigned char* outstart = out;
    const unsigned char* processed = inb;
    unsigned char* outend = out + *outlen;
    unsigned short* in = (unsigned short*) inb;
    unsigned short* inend;
    unsigned int c, d, inlen;
    unsigned char *tmp;
    int bits;

    if ((*inlenb % 2) == 1)
        (*inlenb)--;
    inlen = *inlenb / 2;
    inend= in + inlen;
    while (in < inend) {
	if (xmlLittleEndian) {
	    tmp = (unsigned char *) in;
	    c = *tmp++;
	    c = c << 8;
	    c = c | (unsigned int) *tmp;
	    in++;
	} else {
	    c= *in++;
	} 
        if ((c & 0xFC00) == 0xD800) {    /* surrogates */
	    if (in >= inend) {           /* (in > inend) shouldn't happens */
		*outlen = out - outstart;
		*inlenb = processed - inb;
	        return(-2);
	    }
	    if (xmlLittleEndian) {
		tmp = (unsigned char *) in;
		d = *tmp++;
		d = d << 8;
		d = d | (unsigned int) *tmp;
		in++;
	    } else {
		d= *in++;
	    }
            if ((d & 0xFC00) == 0xDC00) {
                c &= 0x03FF;
                c <<= 10;
                c |= d & 0x03FF;
                c += 0x10000;
            }
            else {
		*outlen = out - outstart;
		*inlenb = processed - inb;
	        return(-2);
	    }
        }

	/* assertion: c is a single UTF-4 value */
        if (out >= outend) 
	    break;
        if      (c <    0x80) {  *out++=  c;                bits= -6; }
        else if (c <   0x800) {  *out++= ((c >>  6) & 0x1F) | 0xC0;  bits=  0; }
        else if (c < 0x10000) {  *out++= ((c >> 12) & 0x0F) | 0xE0;  bits=  6; }
        else                  {  *out++= ((c >> 18) & 0x07) | 0xF0;  bits= 12; }
 
        for ( ; bits >= 0; bits-= 6) {
            if (out >= outend) 
	        break;
            *out++= ((c >> bits) & 0x3F) | 0x80;
        }
	processed = (const unsigned char*) in;
    }
    *outlen = out - outstart;
    *inlenb = processed - inb;
    return(*outlen);
}


/************************************************************************
 *									*
 *		Generic encoding handling routines			*
 *									*
 ************************************************************************/

/**
 * hw_xmlDetectCharEncoding:
 * @in:  a pointer to the first bytes of the XML entity, must be at least
 *       2 bytes long (at least 4 if encoding is UTF4 variant).
 * @len:  pointer to the length of the buffer
 *
 * Guess the encoding of the entity using the first bytes of the entity content
 * according to the non-normative appendix F of the XML-1.0 recommendation.
 * 
 * Returns one of the XML_CHAR_ENCODING_... values.
 */
hw_xmlCharEncoding
hw_xmlDetectCharEncoding(const unsigned char* in, int len)
{
    if (in == NULL) 
        return(XML_CHAR_ENCODING_NONE);
    if (len >= 4) {
	if ((in[0] == 0x00) && (in[1] == 0x00) &&
	    (in[2] == 0x00) && (in[3] == 0x3C))
	    return(XML_CHAR_ENCODING_UCS4BE);
	if ((in[0] == 0x3C) && (in[1] == 0x00) &&
	    (in[2] == 0x00) && (in[3] == 0x00))
	    return(XML_CHAR_ENCODING_UCS4LE);
	if ((in[0] == 0x00) && (in[1] == 0x00) &&
	    (in[2] == 0x3C) && (in[3] == 0x00))
	    return(XML_CHAR_ENCODING_UCS4_2143);
	if ((in[0] == 0x00) && (in[1] == 0x3C) &&
	    (in[2] == 0x00) && (in[3] == 0x00))
	    return(XML_CHAR_ENCODING_UCS4_3412);
	if ((in[0] == 0x4C) && (in[1] == 0x6F) &&
	    (in[2] == 0xA7) && (in[3] == 0x94))
	    return(XML_CHAR_ENCODING_EBCDIC);
	if ((in[0] == 0x3C) && (in[1] == 0x3F) &&
	    (in[2] == 0x78) && (in[3] == 0x6D))
	    return(XML_CHAR_ENCODING_UTF8);
	/*
	 * Although not part of the recommendation, we also
	 * attempt an "auto-recognition" of UTF-16LE and
	 * UTF-16BE encodings.
	 */
	if ((in[0] == 0x3C) && (in[1] == 0x00) &&
	    (in[2] == 0x3F) && (in[3] == 0x00))
	    return(XML_CHAR_ENCODING_UTF16LE);
	if ((in[0] == 0x00) && (in[1] == 0x3C) &&
	    (in[2] == 0x00) && (in[3] == 0x3F))
	    return(XML_CHAR_ENCODING_UTF16BE);
    }
    if (len >= 3) {
	/*
	 * Errata on XML-1.0 June 20 2001
	 * We now allow an UTF8 encoded BOM
	 */
	if ((in[0] == 0xEF) && (in[1] == 0xBB) &&
	    (in[2] == 0xBF))
	    return(XML_CHAR_ENCODING_UTF8);
    }
    /* For UTF-16 we can recognize by the BOM */
    if (len >= 2) {
	if ((in[0] == 0xFE) && (in[1] == 0xFF))
	    return(XML_CHAR_ENCODING_UTF16BE);
	if ((in[0] == 0xFF) && (in[1] == 0xFE))
	    return(XML_CHAR_ENCODING_UTF16LE);
    }
    return(XML_CHAR_ENCODING_NONE);
}

/**
 * hw_xmlCleanupEncodingAliases:
 *
 * Unregisters all aliases
 */
void
hw_xmlCleanupEncodingAliases(void) {
    int i;

    if (xmlCharEncodingAliases == NULL)
	return;

    for (i = 0;i < xmlCharEncodingAliasesNb;i++) {
	if (xmlCharEncodingAliases[i].name != NULL)
	    hw_xmlFree((char *) xmlCharEncodingAliases[i].name);
	if (xmlCharEncodingAliases[i].alias != NULL)
	    hw_xmlFree((char *) xmlCharEncodingAliases[i].alias);
    }
    xmlCharEncodingAliasesNb = 0;
    xmlCharEncodingAliasesMax = 0;
    hw_xmlFree(xmlCharEncodingAliases);
    xmlCharEncodingAliases = NULL;
}

/**
 * hw_xmlGetEncodingAlias:
 * @alias:  the alias name as parsed, in UTF-8 format (ASCII actually)
 *
 * Lookup an encoding name for the given alias.
 * 
 * Returns NULL if not found, otherwise the original name
 */
const char *
hw_xmlGetEncodingAlias(const char *alias) {
    int i;
    char upper[100];

    if (alias == NULL)
	return(NULL);

    if (xmlCharEncodingAliases == NULL)
	return(NULL);

    for (i = 0;i < 99;i++) {
        upper[i] = toupper(alias[i]);
	if (upper[i] == 0) break;
    }
    upper[i] = 0;

    /*
     * Walk down the list looking for a definition of the alias
     */
    for (i = 0;i < xmlCharEncodingAliasesNb;i++) {
	if (!strcmp(xmlCharEncodingAliases[i].alias, upper)) {
	    return(xmlCharEncodingAliases[i].name);
	}
    }
    return(NULL);
}


/**
 * hw_xmlParseCharEncoding:
 * @name:  the encoding name as parsed, in UTF-8 format (ASCII actually)
 *
 * Compare the string to the encoding schemes already known. Note
 * that the comparison is case insensitive accordingly to the section
 * [XML] 4.3.3 Character Encoding in Entities.
 * 
 * Returns one of the XML_CHAR_ENCODING_... values or XML_CHAR_ENCODING_NONE
 * if not recognized.
 */
hw_xmlCharEncoding
hw_xmlParseCharEncoding(const char* name)
{
    const char *alias;
    char upper[500];
    int i;

    if (name == NULL)
	return(XML_CHAR_ENCODING_NONE);

    /*
     * Do the alias resolution
     */
    alias = hw_xmlGetEncodingAlias(name);
    if (alias != NULL)
	name = alias;

    for (i = 0;i < 499;i++) {
        upper[i] = toupper(name[i]);
	if (upper[i] == 0) break;
    }
    upper[i] = 0;

    if (!strcmp(upper, "")) return(XML_CHAR_ENCODING_NONE);
    if (!strcmp(upper, "UTF-8")) return(XML_CHAR_ENCODING_UTF8);
    if (!strcmp(upper, "UTF8")) return(XML_CHAR_ENCODING_UTF8);

    /*
     * NOTE: if we were able to parse this, the endianness of UTF16 is
     *       already found and in use
     */
    if (!strcmp(upper, "UTF-16")) return(XML_CHAR_ENCODING_UTF16LE);
    if (!strcmp(upper, "UTF16")) return(XML_CHAR_ENCODING_UTF16LE);
    
    if (!strcmp(upper, "ISO-10646-UCS-2")) return(XML_CHAR_ENCODING_UCS2);
    if (!strcmp(upper, "UCS-2")) return(XML_CHAR_ENCODING_UCS2);
    if (!strcmp(upper, "UCS2")) return(XML_CHAR_ENCODING_UCS2);

    /*
     * NOTE: if we were able to parse this, the endianness of UCS4 is
     *       already found and in use
     */
    if (!strcmp(upper, "ISO-10646-UCS-4")) return(XML_CHAR_ENCODING_UCS4LE);
    if (!strcmp(upper, "UCS-4")) return(XML_CHAR_ENCODING_UCS4LE);
    if (!strcmp(upper, "UCS4")) return(XML_CHAR_ENCODING_UCS4LE);

    
    if (!strcmp(upper,  "ISO-8859-1")) return(XML_CHAR_ENCODING_8859_1);
    if (!strcmp(upper,  "ISO-LATIN-1")) return(XML_CHAR_ENCODING_8859_1);
    if (!strcmp(upper,  "ISO LATIN 1")) return(XML_CHAR_ENCODING_8859_1);

    if (!strcmp(upper,  "ISO-8859-2")) return(XML_CHAR_ENCODING_8859_2);
    if (!strcmp(upper,  "ISO-LATIN-2")) return(XML_CHAR_ENCODING_8859_2);
    if (!strcmp(upper,  "ISO LATIN 2")) return(XML_CHAR_ENCODING_8859_2);

    if (!strcmp(upper,  "ISO-8859-3")) return(XML_CHAR_ENCODING_8859_3);
    if (!strcmp(upper,  "ISO-8859-4")) return(XML_CHAR_ENCODING_8859_4);
    if (!strcmp(upper,  "ISO-8859-5")) return(XML_CHAR_ENCODING_8859_5);
    if (!strcmp(upper,  "ISO-8859-6")) return(XML_CHAR_ENCODING_8859_6);
    if (!strcmp(upper,  "ISO-8859-7")) return(XML_CHAR_ENCODING_8859_7);
    if (!strcmp(upper,  "ISO-8859-8")) return(XML_CHAR_ENCODING_8859_8);
    if (!strcmp(upper,  "ISO-8859-9")) return(XML_CHAR_ENCODING_8859_9);

    if (!strcmp(upper, "ISO-2022-JP")) return(XML_CHAR_ENCODING_2022_JP);
    if (!strcmp(upper, "SHIFT_JIS")) return(XML_CHAR_ENCODING_SHIFT_JIS);
    if (!strcmp(upper, "EUC-JP")) return(XML_CHAR_ENCODING_EUC_JP);

    return(XML_CHAR_ENCODING_ERROR);
}

/**
 * hw_xmlGetCharEncodingName:
 * @enc:  the encoding
 *
 * The "canonical" name for XML encoding.
 * C.f. http://www.w3.org/TR/REC-xml#charencoding
 * Section 4.3.3  Character Encoding in Entities
 *
 * Returns the canonical name for the given encoding
 */

const char*
hw_xmlGetCharEncodingName(hw_xmlCharEncoding enc) {
    switch (enc) {
        case XML_CHAR_ENCODING_ERROR:
	    return(NULL);
        case XML_CHAR_ENCODING_NONE:
	    return(NULL);
        case XML_CHAR_ENCODING_UTF8:
	    return("UTF-8");
        case XML_CHAR_ENCODING_UTF16LE:
	    return("UTF-16");
        case XML_CHAR_ENCODING_UTF16BE:
	    return("UTF-16");
        case XML_CHAR_ENCODING_EBCDIC:
            return("EBCDIC");
        case XML_CHAR_ENCODING_UCS4LE:
            return("ISO-10646-UCS-4");
        case XML_CHAR_ENCODING_UCS4BE:
            return("ISO-10646-UCS-4");
        case XML_CHAR_ENCODING_UCS4_2143:
            return("ISO-10646-UCS-4");
        case XML_CHAR_ENCODING_UCS4_3412:
            return("ISO-10646-UCS-4");
        case XML_CHAR_ENCODING_UCS2:
            return("ISO-10646-UCS-2");
        case XML_CHAR_ENCODING_8859_1:
	    return("ISO-8859-1");
        case XML_CHAR_ENCODING_8859_2:
	    return("ISO-8859-2");
        case XML_CHAR_ENCODING_8859_3:
	    return("ISO-8859-3");
        case XML_CHAR_ENCODING_8859_4:
	    return("ISO-8859-4");
        case XML_CHAR_ENCODING_8859_5:
	    return("ISO-8859-5");
        case XML_CHAR_ENCODING_8859_6:
	    return("ISO-8859-6");
        case XML_CHAR_ENCODING_8859_7:
	    return("ISO-8859-7");
        case XML_CHAR_ENCODING_8859_8:
	    return("ISO-8859-8");
        case XML_CHAR_ENCODING_8859_9:
	    return("ISO-8859-9");
        case XML_CHAR_ENCODING_2022_JP:
            return("ISO-2022-JP");
        case XML_CHAR_ENCODING_SHIFT_JIS:
            return("Shift-JIS");
        case XML_CHAR_ENCODING_EUC_JP:
            return("EUC-JP");
	case XML_CHAR_ENCODING_ASCII:
	    return(NULL);
    }
    return(NULL);
}

/************************************************************************
 *									*
 *			Char encoding handlers				*
 *									*
 ************************************************************************/


/* the size should be growable, but it's not a big deal ... */
#define MAX_ENCODING_HANDLERS 50
static hw_xmlCharEncodingHandlerPtr *handlers = NULL;
static int nbCharEncodingHandler = 0;

/*
 * The default is UTF-8 for XML, that's also the default used for the
 * parser internals, so the default encoding handler is NULL
 */

static hw_xmlCharEncodingHandlerPtr xmlDefaultCharEncodingHandler = NULL;

/**
 * hw_xmlNewCharEncodingHandler:
 * @name:  the encoding name, in UTF-8 format (ASCII actually)
 * @input:  the hw_xmlCharEncodingInputFunc to read that encoding
 * @output:  the hw_xmlCharEncodingOutputFunc to write that encoding
 *
 * Create and registers an hw_xmlCharEncodingHandler.
 *
 * Returns the hw_xmlCharEncodingHandlerPtr created (or NULL in case of error).
 */
hw_xmlCharEncodingHandlerPtr
hw_xmlNewCharEncodingHandler(const char *name, 
                          hw_xmlCharEncodingInputFunc input,
                          hw_xmlCharEncodingOutputFunc output) {
    hw_xmlCharEncodingHandlerPtr handler;
    const char *alias;
    char upper[500];
    int i;
    char *up = NULL;

    /*
     * Do the alias resolution
     */
    alias = hw_xmlGetEncodingAlias(name);
    if (alias != NULL)
	name = alias;

    /*
     * Keep only the uppercase version of the encoding.
     */
    if (name == NULL) {
        xmlEncodingErr(XML_I18N_NO_NAME,
		       "hw_xmlNewCharEncodingHandler : no name !\n", NULL);
	return(NULL);
    }
    for (i = 0;i < 499;i++) {
        upper[i] = toupper(name[i]);
	if (upper[i] == 0) break;
    }
    upper[i] = 0;
    up = (char*)hw_xmlMemStrdup((hw_xmlChar *)upper);
    if (up == NULL) {
        xmlEncodingErrMemory("hw_xmlNewCharEncodingHandler : out of memory !\n");
	return(NULL);
    }

    /*
     * allocate and fill-up an handler block.
     */
    handler = (hw_xmlCharEncodingHandlerPtr)
              hw_xmlMalloc(sizeof(hw_xmlCharEncodingHandler));
    if (handler == NULL) {
        hw_xmlFree(up);
        xmlEncodingErrMemory("hw_xmlNewCharEncodingHandler : out of memory !\n");
	return(NULL);
    }
    handler->input = input;
    handler->output = output;
    handler->name = up;


    /*
     * registers and returns the handler.
     */
    hw_xmlRegisterCharEncodingHandler(handler);
    return(handler);
}

/**
 * hw_xmlInitCharEncodingHandlers:
 *
 * Initialize the char encoding support, it registers the default
 * encoding supported.
 * NOTE: while public, this function usually doesn't need to be called
 *       in normal processing.
 */
void
hw_xmlInitCharEncodingHandlers(void) {
    unsigned short int tst = 0x1234;
    unsigned char *ptr = (unsigned char *) &tst; 

    if (handlers != NULL) return;

    handlers = (hw_xmlCharEncodingHandlerPtr *)
        hw_xmlMalloc(MAX_ENCODING_HANDLERS * sizeof(hw_xmlCharEncodingHandlerPtr));

    if (*ptr == 0x12) xmlLittleEndian = 0;
    else if (*ptr == 0x34) xmlLittleEndian = 1;
    else {
        xmlEncodingErr(XML_ERR_INTERNAL_ERROR,
	               "Odd problem at endianness detection\n", NULL);
    }

    if (handlers == NULL) {
        xmlEncodingErrMemory("hw_xmlInitCharEncodingHandlers : out of memory !\n");
	return;
    }
    hw_xmlNewCharEncodingHandler("UTF-8", UTF8ToUTF8, UTF8ToUTF8);

    xmlUTF16LEHandler = 
          hw_xmlNewCharEncodingHandler("UTF-16LE", UTF16LEToUTF8, NULL);
    xmlUTF16BEHandler = 
          hw_xmlNewCharEncodingHandler("UTF-16BE", UTF16BEToUTF8, NULL);
    hw_xmlNewCharEncodingHandler("UTF-16", UTF16LEToUTF8, NULL);
    hw_xmlNewCharEncodingHandler("ISO-8859-1", hw_isolat1ToUTF8, NULL);
    hw_xmlNewCharEncodingHandler("ASCII", asciiToUTF8, NULL);
    hw_xmlNewCharEncodingHandler("US-ASCII", asciiToUTF8, NULL);


}

/**
 * hw_xmlCleanupCharEncodingHandlers:
 *
 * Cleanup the memory allocated for the char encoding support, it
 * unregisters all the encoding handlers and the aliases.
 */
void
hw_xmlCleanupCharEncodingHandlers(void) {
    hw_xmlCleanupEncodingAliases();

    if (handlers == NULL) return;

    for (;nbCharEncodingHandler > 0;) {
        nbCharEncodingHandler--;
	if (handlers[nbCharEncodingHandler] != NULL) {
	    if (handlers[nbCharEncodingHandler]->name != NULL)
		hw_xmlFree(handlers[nbCharEncodingHandler]->name);
	    hw_xmlFree(handlers[nbCharEncodingHandler]);
	}
    }
    hw_xmlFree(handlers);
    handlers = NULL;
    nbCharEncodingHandler = 0;
    xmlDefaultCharEncodingHandler = NULL;
}

/**
 * hw_xmlRegisterCharEncodingHandler:
 * @handler:  the hw_xmlCharEncodingHandlerPtr handler block
 *
 * Register the char encoding handler, surprising, isn't it ?
 */
void
hw_xmlRegisterCharEncodingHandler(hw_xmlCharEncodingHandlerPtr handler) {
    if (handlers == NULL) hw_xmlInitCharEncodingHandlers();
    if (handler == NULL) {
        xmlEncodingErr(XML_I18N_NO_HANDLER,
		"hw_xmlRegisterCharEncodingHandler: NULL handler !\n", NULL);
	return;
    }

    if (nbCharEncodingHandler >= MAX_ENCODING_HANDLERS) {
        xmlEncodingErr(XML_I18N_EXCESS_HANDLER,
	"hw_xmlRegisterCharEncodingHandler: Too many handler registered, see %s\n",
	               "MAX_ENCODING_HANDLERS");
	return;
    }
    handlers[nbCharEncodingHandler++] = handler;
}

/**
 * hw_xmlGetCharEncodingHandler:
 * @enc:  an hw_xmlCharEncoding value.
 *
 * Search in the registered set the handler able to read/write that encoding.
 *
 * Returns the handler or NULL if not found
 */
hw_xmlCharEncodingHandlerPtr
hw_xmlGetCharEncodingHandler(hw_xmlCharEncoding enc) {
    hw_xmlCharEncodingHandlerPtr handler;

    if (handlers == NULL) hw_xmlInitCharEncodingHandlers();
    switch (enc) {
        case XML_CHAR_ENCODING_ERROR:
	    return(NULL);
        case XML_CHAR_ENCODING_NONE:
	    return(NULL);
        case XML_CHAR_ENCODING_UTF8:
	    return(NULL);
        case XML_CHAR_ENCODING_UTF16LE:
	    return(xmlUTF16LEHandler);
        case XML_CHAR_ENCODING_UTF16BE:
	    return(xmlUTF16BEHandler);
        case XML_CHAR_ENCODING_EBCDIC:
            handler = hw_xmlFindCharEncodingHandler("EBCDIC");
            if (handler != NULL) return(handler);
            handler = hw_xmlFindCharEncodingHandler("ebcdic");
            if (handler != NULL) return(handler);
	    break;
        case XML_CHAR_ENCODING_UCS4BE:
            handler = hw_xmlFindCharEncodingHandler("ISO-10646-UCS-4");
            if (handler != NULL) return(handler);
            handler = hw_xmlFindCharEncodingHandler("UCS-4");
            if (handler != NULL) return(handler);
            handler = hw_xmlFindCharEncodingHandler("UCS4");
            if (handler != NULL) return(handler);
	    break;
        case XML_CHAR_ENCODING_UCS4LE:
            handler = hw_xmlFindCharEncodingHandler("ISO-10646-UCS-4");
            if (handler != NULL) return(handler);
            handler = hw_xmlFindCharEncodingHandler("UCS-4");
            if (handler != NULL) return(handler);
            handler = hw_xmlFindCharEncodingHandler("UCS4");
            if (handler != NULL) return(handler);
	    break;
        case XML_CHAR_ENCODING_UCS4_2143:
	    break;
        case XML_CHAR_ENCODING_UCS4_3412:
	    break;
        case XML_CHAR_ENCODING_UCS2:
            handler = hw_xmlFindCharEncodingHandler("ISO-10646-UCS-2");
            if (handler != NULL) return(handler);
            handler = hw_xmlFindCharEncodingHandler("UCS-2");
            if (handler != NULL) return(handler);
            handler = hw_xmlFindCharEncodingHandler("UCS2");
            if (handler != NULL) return(handler);
	    break;

	    /*
	     * We used to keep ISO Latin encodings native in the
	     * generated data. This led to so many problems that
	     * this has been removed. One can still change this
	     * back by registering no-ops encoders for those
	     */
        case XML_CHAR_ENCODING_8859_1:
	    handler = hw_xmlFindCharEncodingHandler("ISO-8859-1");
	    if (handler != NULL) return(handler);
	    break;
        case XML_CHAR_ENCODING_8859_2:
	    handler = hw_xmlFindCharEncodingHandler("ISO-8859-2");
	    if (handler != NULL) return(handler);
	    break;
        case XML_CHAR_ENCODING_8859_3:
	    handler = hw_xmlFindCharEncodingHandler("ISO-8859-3");
	    if (handler != NULL) return(handler);
	    break;
        case XML_CHAR_ENCODING_8859_4:
	    handler = hw_xmlFindCharEncodingHandler("ISO-8859-4");
	    if (handler != NULL) return(handler);
	    break;
        case XML_CHAR_ENCODING_8859_5:
	    handler = hw_xmlFindCharEncodingHandler("ISO-8859-5");
	    if (handler != NULL) return(handler);
	    break;
        case XML_CHAR_ENCODING_8859_6:
	    handler = hw_xmlFindCharEncodingHandler("ISO-8859-6");
	    if (handler != NULL) return(handler);
	    break;
        case XML_CHAR_ENCODING_8859_7:
	    handler = hw_xmlFindCharEncodingHandler("ISO-8859-7");
	    if (handler != NULL) return(handler);
	    break;
        case XML_CHAR_ENCODING_8859_8:
	    handler = hw_xmlFindCharEncodingHandler("ISO-8859-8");
	    if (handler != NULL) return(handler);
	    break;
        case XML_CHAR_ENCODING_8859_9:
	    handler = hw_xmlFindCharEncodingHandler("ISO-8859-9");
	    if (handler != NULL) return(handler);
	    break;


        case XML_CHAR_ENCODING_2022_JP:
            handler = hw_xmlFindCharEncodingHandler("ISO-2022-JP");
            if (handler != NULL) return(handler);
	    break;
        case XML_CHAR_ENCODING_SHIFT_JIS:
            handler = hw_xmlFindCharEncodingHandler("SHIFT-JIS");
            if (handler != NULL) return(handler);
            handler = hw_xmlFindCharEncodingHandler("SHIFT_JIS");
            if (handler != NULL) return(handler);
            handler = hw_xmlFindCharEncodingHandler("Shift_JIS");
            if (handler != NULL) return(handler);
	    break;
        case XML_CHAR_ENCODING_EUC_JP:
            handler = hw_xmlFindCharEncodingHandler("EUC-JP");
            if (handler != NULL) return(handler);
	    break;
	default: 
	    break;
    }
    
    return(NULL);
}

/**
 * hw_xmlFindCharEncodingHandler:
 * @name:  a string describing the char encoding.
 *
 * Search in the registered set the handler able to read/write that encoding.
 *
 * Returns the handler or NULL if not found
 */
hw_xmlCharEncodingHandlerPtr
hw_xmlFindCharEncodingHandler(const char *name) {
    const char *nalias;
    const char *norig;
    hw_xmlCharEncoding alias;

    char upper[100];
    int i;

    if (handlers == NULL) hw_xmlInitCharEncodingHandlers();
    if (name == NULL) return(xmlDefaultCharEncodingHandler);
    if (name[0] == 0) return(xmlDefaultCharEncodingHandler);

    /*
     * Do the alias resolution
     */
    norig = name;
    nalias = hw_xmlGetEncodingAlias(name);
    if (nalias != NULL)
	name = nalias;

    /*
     * Check first for directly registered encoding names
     */
    for (i = 0;i < 99;i++) {
        upper[i] = toupper(name[i]);
	if (upper[i] == 0) break;
    }
    upper[i] = 0;

    for (i = 0;i < nbCharEncodingHandler; i++)
        if (!strcmp(upper, handlers[i]->name)) {
	    return(handlers[i]);
	}




    /*
     * Fallback using the canonical names
     */
    alias = hw_xmlParseCharEncoding(norig);
    if (alias != XML_CHAR_ENCODING_ERROR) {
        const char* canon;
        canon = hw_xmlGetCharEncodingName(alias);
        if ((canon != NULL) && (strcmp(name, canon))) {
	    return(hw_xmlFindCharEncodingHandler(canon));
        }
    }

    /* If "none of the above", give up */
    return(NULL);
}

/************************************************************************
 *									*
 *		ICONV based generic conversion functions		*
 *									*
 ************************************************************************/



/************************************************************************
 *									*
 *		The real API used by libxml for on-the-fly conversion	*
 *									*
 ************************************************************************/

/**
 * hw_xmlCharEncFirstLine:
 * @handler:	char enconding transformation data structure
 * @out:  an hw_xmlBuffer for the output.
 * @in:  an hw_xmlBuffer for the input
 *     
 * Front-end for the encoding handler input function, but handle only
 * the very first line, i.e. limit itself to 45 chars.
 *     
 * Returns the number of byte written if success, or 
 *     -1 general error
 *     -2 if the transcoding fails (for *in is not valid utf8 string or
 *        the result of transformation can't fit into the encoding we want), or
 */
int
hw_xmlCharEncFirstLine(hw_xmlCharEncodingHandler *handler, hw_xmlBufferPtr out,
                 hw_xmlBufferPtr in) {
    int ret = -2;
    int written;
    int toconv;

    if (handler == NULL) return(-1);
    if (out == NULL) return(-1);
    if (in == NULL) return(-1);

    written = out->size - out->use;
    toconv = in->use;
    if (toconv * 2 >= written) {
        hw_xmlBufferGrow(out, toconv);
	written = out->size - out->use - 1;
    }

    /*
     * echo '<?xml version="1.0" encoding="UCS4"?>' | wc -c => 38
     * 45 chars should be sufficient to reach the end of the encoding
     * declaration without going too far inside the document content.
     */
    written = 45;

    if (handler->input != NULL) {
	ret = handler->input(&out->content[out->use], &written,
	                     in->content, &toconv);
	hw_xmlBufferShrink(in, toconv);
	out->use += written;
	out->content[out->use] = 0;
    }

    /*
     * Ignore when input buffer is not on a boundary
     */
    if (ret == -3) ret = 0;
    if (ret == -1) ret = 0;
    return(ret);
}

/**
 * hw_xmlCharEncInFunc:
 * @handler:	char encoding transformation data structure
 * @out:  an hw_xmlBuffer for the output.
 * @in:  an hw_xmlBuffer for the input
 *     
 * Generic front-end for the encoding handler input function
 *     
 * Returns the number of byte written if success, or 
 *     -1 general error
 *     -2 if the transcoding fails (for *in is not valid utf8 string or
 *        the result of transformation can't fit into the encoding we want), or
 */
int
hw_xmlCharEncInFunc(hw_xmlCharEncodingHandler * handler, hw_xmlBufferPtr out,
                 hw_xmlBufferPtr in)
{
    int ret = -2;
    int written;
    int toconv;

    if (handler == NULL)
        return (-1);
    if (out == NULL)
        return (-1);
    if (in == NULL)
        return (-1);

    toconv = in->use;
    if (toconv == 0)
        return (0);
    written = out->size - out->use;
    if (toconv * 2 >= written) {
        hw_xmlBufferGrow(out, out->size + toconv * 2);
        written = out->size - out->use - 1;
    }
    if (handler->input != NULL) {
        ret = handler->input(&out->content[out->use], &written,
                             in->content, &toconv);
        hw_xmlBufferShrink(in, toconv);
        out->use += written;
        out->content[out->use] = 0;
    }

    switch (ret) {
        case 0:

            break;
        case -1:

            break;
        case -3:

            break;
        case -2: {
            char buf[50];

	    snprintf(&buf[0], 49, "0x%02X 0x%02X 0x%02X 0x%02X", 
		     in->content[0], in->content[1],
		     in->content[2], in->content[3]);
	    buf[49] = 0;
	    xmlEncodingErr(XML_I18N_CONV_FAILED,
		    "input conversion failed due to input error, bytes %s\n",
		           buf);
        }
    }
    /*
     * Ignore when input buffer is not on a boundary
     */
    if (ret == -3)
        ret = 0;
    return (written? written : ret);
}

/**
 * hw_xmlCharEncCloseFunc:
 * @handler:	char enconding transformation data structure
 *     
 * Generic front-end for encoding handler close function
 *
 * Returns 0 if success, or -1 in case of error
 */
int
hw_xmlCharEncCloseFunc(hw_xmlCharEncodingHandler *handler) {
    int ret = 0;
    if (handler == NULL) return(-1);
    if (handler->name == NULL) return(-1);


    return(ret);
}

