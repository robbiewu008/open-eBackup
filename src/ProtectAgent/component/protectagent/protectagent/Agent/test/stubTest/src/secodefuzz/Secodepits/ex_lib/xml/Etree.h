/*
 * Summary: interfaces for tree manipulation
 * Description: this module describes the structures found in an tree resulting
 *              from an XML or HTML parsing, as well as the API provided for
 *              various processing on that tree
 *
 * Copy: See Copyright for the status of this software.
 *
 * Author: Daniel Veillard
 */

#ifndef __XML_TREE_H__
#define __XML_TREE_H__

#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h> /* pull definition of size_t */
#include <stdarg.h>

#include <ctype.h>

/////////////////////////////////////////////////////////////////////////////////////

/** DOC_DISABLE */

/* Windows platform with MS compiler */
#if defined(_WIN32) && defined(_MSC_VER)
  #undef hw_XMLPUBFUN
  #undef hw_XMLPUBVAR
  #undef hw_XMLCALL
  #undef hw_XMLCDECL
  #if defined(IN_LIBXML) && !defined(LIBXML_STATIC)
    #define XMLPUBFUN __declspec(dllexport)
    #define XMLPUBVAR __declspec(dllexport)
  #else
    #define XMLPUBFUN
    #if !defined(LIBXML_STATIC)
      #define XMLPUBVAR __declspec(dllimport) extern
    #else
      #define XMLPUBVAR extern
    #endif
  #endif
  #if defined(LIBXML_FASTCALL)
    #define XMLCALL __fastcall
  #else
    #define XMLCALL __cdecl
  #endif
  #define XMLCDECL __cdecl
  #if !defined _REENTRANT
    #define _REENTRANT
  #endif
#endif

/* Windows platform with Borland compiler */
#if defined(_WIN32) && defined(__BORLANDC__)
  #undef hw_XMLPUBFUN
  #undef hw_XMLPUBVAR
  #undef hw_XMLCALL
  #undef hw_XMLCDECL
  #if defined(IN_LIBXML) && !defined(LIBXML_STATIC)
    #define XMLPUBFUN __declspec(dllexport)
    #define XMLPUBVAR __declspec(dllexport) extern
  #else
    #define XMLPUBFUN
    #if !defined(LIBXML_STATIC)
      #define XMLPUBVAR __declspec(dllimport) extern
    #else
      #define XMLPUBVAR extern
    #endif
  #endif
  #define XMLCALL __cdecl
  #define XMLCDECL __cdecl
  #if !defined _REENTRANT
    #define _REENTRANT
  #endif
#endif

/* Windows platform with GNU compiler (Mingw) */
#if defined(_WIN32) && defined(__MINGW32__)
  #undef hw_XMLPUBFUN
  #undef hw_XMLPUBVAR
  #undef hw_XMLCALL
  #undef hw_XMLCDECL
  #if defined(IN_LIBXML) && !defined(LIBXML_STATIC)
    #define XMLPUBFUN __declspec(dllexport)
    #define XMLPUBVAR __declspec(dllexport)
  #else
    #define XMLPUBFUN
    #if !defined(LIBXML_STATIC)
      #define XMLPUBVAR __declspec(dllimport) extern
    #else
      #define XMLPUBVAR extern
    #endif
  #endif
  #define XMLCALL __cdecl
  #define XMLCDECL __cdecl
  #if !defined _REENTRANT
    #define _REENTRANT
  #endif
#endif

/* Cygwin platform, GNU compiler */
#if defined(_WIN32) && defined(__CYGWIN__)
  #undef hw_XMLPUBFUN
  #undef hw_XMLPUBVAR
  #undef hw_XMLCALL
  #undef hw_XMLCDECL
  #if defined(IN_LIBXML) && !defined(LIBXML_STATIC)
    #define XMLPUBFUN __declspec(dllexport)
    #define XMLPUBVAR __declspec(dllexport)
  #else
    #define XMLPUBFUN
    #if !defined(LIBXML_STATIC)
      #define XMLPUBVAR __declspec(dllimport) extern
    #else
      #define XMLPUBVAR
    #endif
  #endif
  #define XMLCALL __cdecl
  #define XMLCDECL __cdecl
#endif

/* Compatibility */
#if !defined(LIBXML_DLL_IMPORT)
#define LIBXML_DLL_IMPORT hw_XMLPUBVAR
#endif


/**
 * ATTRIBUTE_UNUSED:
 *
 * Macro used to signal to GCC unused function parameters
 */
#ifdef __GNUC__
#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED __attribute__((unused))
#endif
#else
#define ATTRIBUTE_UNUSED
#endif

////////////////////////////////////////////////////////////////////////////////////

/**
 * hw_BAD_CAST:
 *
 * Macro to cast a string to an hw_xmlChar * when one know its safe.
 */
#define hw_BAD_CAST (hw_xmlChar *)


/**
 * hw_XMLPUBFUN, hw_XMLPUBVAR, hw_XMLCALL
 *
 * Macros which declare an exportable function, an exportable variable and
 * the calling convention used for functions.
 *
 * Please use an extra block for every platform/compiler combination when
 * modifying this, rather than overlong #ifdef lines. This helps
 * readability as well as the fact that different compilers on the same
 * platform might need different definitions.
 */

/**
 * hw_XMLPUBFUN:
 *
 * Macros which declare an exportable function
 */
#define hw_XMLPUBFUN
/**
 * hw_XMLPUBVAR:
 *
 * Macros which declare an exportable variable
 */
#define hw_XMLPUBVAR extern
/**
 * hw_XMLCALL:
 *
 * Macros which declare the called convention for exported functions
 */
#define hw_XMLCALL
/**
 * hw_XMLCDECL:
 *
 * Macro which declares the calling convention for exported functions that 
 * use '...'.
 */
#define hw_XMLCDECL

/**
 * hw_BASE_BUFFER_SIZE:
 *
 * default buffer size 4000.
 */
#define hw_BASE_BUFFER_SIZE 4096

/**
 * hw_XML_XML_NAMESPACE:
 *
 * This is the namespace for the special xml: prefix predefined in the
 * XML Namespace specification.
 */
#define hw_XML_XML_NAMESPACE \
    (const hw_xmlChar *) "http://www.w3.org/XML/1998/namespace"

#define hw_XML_LOCAL_NAMESPACE XML_NAMESPACE_DECL



/**
 * hw_xmlIsBaseChar_ch:
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define hw_xmlIsBaseChar_ch(c)	(((0x41 <= (c)) && ((c) <= 0x5a)) || \
				 ((0x61 <= (c)) && ((c) <= 0x7a)) || \
				 ((0xc0 <= (c)) && ((c) <= 0xd6)) || \
				 ((0xd8 <= (c)) && ((c) <= 0xf6)) || \
				  (0xf8 <= (c)))

/**
 * hw_xmlIsBaseCharQ:
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define hw_xmlIsBaseCharQ(c)	(((c) < 0x100) ? \
				 hw_xmlIsBaseChar_ch((c)) : \
				 1)



/**
 * hw_xmlIsBlank_ch:
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define hw_xmlIsBlank_ch(c)	(((c) == 0x20) || \
				 ((0x9 <= (c)) && ((c) <= 0xa)) || \
				 ((c) == 0xd))

/**
 * hw_xmlIsBlankQ:
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define hw_xmlIsBlankQ(c)		(((c) < 0x100) ? \
				 hw_xmlIsBlank_ch((c)) : 0)


/**
 * hw_xmlIsChar_ch:
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define hw_xmlIsChar_ch(c)		(((0x9 <= (c)) && ((c) <= 0xa)) || \
				 ((c) == 0xd) || \
				  (0x20 <= (c)))

/**
 * hw_xmlIsCharQ:
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define hw_xmlIsCharQ(c)		(((c) < 0x100) ? \
				 hw_xmlIsChar_ch((c)) :\
				(((0x100 <= (c)) && ((c) <= 0xd7ff)) || \
				 ((0xe000 <= (c)) && ((c) <= 0xfffd)) || \
				 ((0x10000 <= (c)) && ((c) <= 0x10ffff))))




/**
 * hw_xmlIsCombiningQ:
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define hw_xmlIsCombiningQ(c)	(((c) < 0x100) ? \
				 0 : \
				 1)

/**
 * hw_xmlIsDigit_ch:
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define hw_xmlIsDigit_ch(c)	(((0x30 <= (c)) && ((c) <= 0x39)))

/**
 * hw_xmlIsDigitQ:
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define hw_xmlIsDigitQ(c)		(((c) < 0x100) ? \
				 hw_xmlIsDigit_ch((c)) : \
				 1)

/**
 * hw_xmlIsExtender_ch:
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define hw_xmlIsExtender_ch(c)	(((c) == 0xb7))

/**
 * hw_xmlIsExtenderQ:
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define hw_xmlIsExtenderQ(c)	(((c) < 0x100) ? \
				 hw_xmlIsExtender_ch((c)) : \
				 1)

/**
 * hw_xmlIsIdeographicQ:
 * @c: char to validate
 *
 * Automatically generated by genChRanges.py
 */
#define hw_xmlIsIdeographicQ(c)	(((c) < 0x100) ? \
				 0 :\
				(((0x4e00 <= (c)) && ((c) <= 0x9fa5)) || \
				 ((c) == 0x3007) || \
				 ((0x3021 <= (c)) && ((c) <= 0x3029))))




/**
 * hw_XML_CTXT_FINISH_DTD_0:
 *
 * Special value for finishDtd field when embedded in an hw_xmlParserCtxt
 */
#define hw_XML_CTXT_FINISH_DTD_0 0xabcd1234
/**
 * hw_XML_CTXT_FINISH_DTD_1:
 *
 * Special value for finishDtd field when embedded in an hw_xmlParserCtxt
 */
#define hw_XML_CTXT_FINISH_DTD_1 0xabcd1235





 /**
  * hw_XML_MAX_NAMELEN:
  *
  * Identifiers can be longer, but this will be more costly
  * at runtime.
  */
#define hw_XML_MAX_NAMELEN 100

/**
 * hw_INPUT_CHUNK:
 *
 * The parser tries to always have that amount of input ready.
 * One of the point is providing context when reporting errors.
 */
#define hw_INPUT_CHUNK	250

/************************************************************************
 *									*
 * UNICODE version of the macros.      					*
 *									*
 ************************************************************************/
/**
 * hw_IS_BYTE_CHAR:
 * @c:  an byte value (int)
 *
 * Macro to check the following production in the XML spec:
 *
 * [2] Char ::= #x9 | #xA | #xD | [#x20...]
 * any byte character in the accepted range
 */
#define hw_IS_BYTE_CHAR(c)	 hw_xmlIsChar_ch(c)

/**
 * hw_IS_CHAR:
 * @c:  an UNICODE value (int)
 *
 * Macro to check the following production in the XML spec:
 *
 * [2] Char ::= #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD]
 *                  | [#x10000-#x10FFFF]
 * any Unicode character, excluding the surrogate blocks, FFFE, and FFFF.
 */
#define hw_IS_CHAR(c)   hw_xmlIsCharQ(c)


/**
 * hw_IS_BLANK:
 * @c:  an UNICODE value (int)
 *
 * Macro to check the following production in the XML spec:
 *
 * [3] S ::= (#x20 | #x9 | #xD | #xA)+
 */
#define hw_IS_BLANK(c)  hw_xmlIsBlankQ(c)

/**
 * hw_IS_BLANK_CH:
 * @c:  an hw_xmlChar value (normally unsigned char)
 *
 * Behaviour same as hw_IS_BLANK
 */
#define hw_IS_BLANK_CH(c)  hw_xmlIsBlank_ch(c)

/**
 * hw_IS_BASECHAR:
 * @c:  an UNICODE value (int)
 *
 * Macro to check the following production in the XML spec:
 *
 * [85] BaseChar ::= ... long list see REC ...
 */
#define hw_IS_BASECHAR(c) hw_xmlIsBaseCharQ(c)

/**
 * hw_IS_DIGIT:
 * @c:  an UNICODE value (int)
 *
 * Macro to check the following production in the XML spec:
 *
 * [88] Digit ::= ... long list see REC ...
 */
#define hw_IS_DIGIT(c) hw_xmlIsDigitQ(c)


/**
 * hw_IS_COMBINING:
 * @c:  an UNICODE value (int)
 *
 * Macro to check the following production in the XML spec:
 *
 * [87] CombiningChar ::= ... long list see REC ...
 */
#define hw_IS_COMBINING(c) hw_xmlIsCombiningQ(c)


/**
 * hw_IS_EXTENDER:
 * @c:  an UNICODE value (int)
 *
 * Macro to check the following production in the XML spec:
 *
 *
 * [89] Extender ::= #x00B7 | #x02D0 | #x02D1 | #x0387 | #x0640 |
 *                   #x0E46 | #x0EC6 | #x3005 | [#x3031-#x3035] |
 *                   [#x309D-#x309E] | [#x30FC-#x30FE]
 */
#define hw_IS_EXTENDER(c) hw_xmlIsExtenderQ(c)


/**
 * hw_IS_IDEOGRAPHIC:
 * @c:  an UNICODE value (int)
 *
 * Macro to check the following production in the XML spec:
 *
 *
 * [86] Ideographic ::= [#x4E00-#x9FA5] | #x3007 | [#x3021-#x3029]
 */
#define hw_IS_IDEOGRAPHIC(c) hw_xmlIsIdeographicQ(c)

/**
 * hw_IS_LETTER:
 * @c:  an UNICODE value (int)
 *
 * Macro to check the following production in the XML spec:
 *
 *
 * [84] Letter ::= BaseChar | Ideographic 
 */
#define hw_IS_LETTER(c) (hw_IS_BASECHAR(c) || hw_IS_IDEOGRAPHIC(c))




/**
 * hw_MOVETO_ENDTAG:
 * @p:  and UTF8 string pointer
 *
 * Skips to the next '>' char.
 */
#define hw_MOVETO_ENDTAG(p)						\
    while ((*p) && (*(p) != '>')) (p)++



#define hw_xmlMalloc malloc
#define hw_xmlFree free
#define hw_xmlMallocAtomic malloc
#define hw_xmlRealloc realloc
#define hw_xmlFree free
#define hw_xmlMemStrdup hw_xmlStrdup


/**
 * hw_XML_SUBSTITUTE_REF:
 *
 * Whether general entities need to be substituted.
 */
#define hw_XML_SUBSTITUTE_REF	1
/**
 * hw_XML_SUBSTITUTE_PEREF:
 *
 * Whether parameter entities need to be substituted.
 */
#define hw_XML_SUBSTITUTE_PEREF	2
/////////////////////////////////////////////////////////////////////////////////////

/*
 * hw_xmlCharEncoding:
 *
 * Predefined values for some standard encodings.
 * Libxml does not do beforehand translation on UTF8 and ISOLatinX.
 * It also supports ASCII, ISO-8859-1, and UTF16 (LE and BE) by default.
 *
 * Anything else would have to be translated to UTF8 before being
 * given to the parser itself. The BOM for UTF16 and the encoding
 * declaration are looked at and a converter is looked for at that
 * point. If not found the parser stops here as asked by the XML REC. A
 * converter can be registered by the user using hw_xmlRegisterCharEncodingHandler
 * but the current form doesn't allow stateful transcoding (a serious
 * problem agreed !). If iconv has been found it will be used
 * automatically and allow stateful transcoding, the simplest is then
 * to be sure to enable iconv and to provide iconv libs for the encoding
 * support needed.
 *
 * Note that the generic "UTF-16" is not a predefined value.  Instead, only
 * the specific UTF-16LE and UTF-16BE are present.
 */
typedef enum {
    XML_CHAR_ENCODING_ERROR=   -1, /* No char encoding detected */
    XML_CHAR_ENCODING_NONE=	0, /* No char encoding detected */
    XML_CHAR_ENCODING_UTF8=	1, /* UTF-8 */
    XML_CHAR_ENCODING_UTF16LE=	2, /* UTF-16 little endian */
    XML_CHAR_ENCODING_UTF16BE=	3, /* UTF-16 big endian */
    XML_CHAR_ENCODING_UCS4LE=	4, /* UCS-4 little endian */
    XML_CHAR_ENCODING_UCS4BE=	5, /* UCS-4 big endian */
    XML_CHAR_ENCODING_EBCDIC=	6, /* EBCDIC uh! */
    XML_CHAR_ENCODING_UCS4_2143=7, /* UCS-4 unusual ordering */
    XML_CHAR_ENCODING_UCS4_3412=8, /* UCS-4 unusual ordering */
    XML_CHAR_ENCODING_UCS2=	9, /* UCS-2 */
    XML_CHAR_ENCODING_8859_1=	10,/* ISO-8859-1 ISO Latin 1 */
    XML_CHAR_ENCODING_8859_2=	11,/* ISO-8859-2 ISO Latin 2 */
    XML_CHAR_ENCODING_8859_3=	12,/* ISO-8859-3 */
    XML_CHAR_ENCODING_8859_4=	13,/* ISO-8859-4 */
    XML_CHAR_ENCODING_8859_5=	14,/* ISO-8859-5 */
    XML_CHAR_ENCODING_8859_6=	15,/* ISO-8859-6 */
    XML_CHAR_ENCODING_8859_7=	16,/* ISO-8859-7 */
    XML_CHAR_ENCODING_8859_8=	17,/* ISO-8859-8 */
    XML_CHAR_ENCODING_8859_9=	18,/* ISO-8859-9 */
    XML_CHAR_ENCODING_2022_JP=  19,/* ISO-2022-JP */
    XML_CHAR_ENCODING_SHIFT_JIS=20,/* Shift_JIS */
    XML_CHAR_ENCODING_EUC_JP=   21,/* EUC-JP */
    XML_CHAR_ENCODING_ASCII=    22 /* pure ASCII */
} hw_xmlCharEncoding;




/**
 * hw_xmlBufferAllocationScheme:
 *
 * A buffer allocation scheme can be defined to either match exactly the
 * need or double it's allocated size each time it is found too small.
 */

typedef enum {
    XML_BUFFER_ALLOC_DOUBLEIT,
    XML_BUFFER_ALLOC_EXACT,
    XML_BUFFER_ALLOC_IMMUTABLE
} hw_xmlBufferAllocationScheme;

/*
 * The different element types carried by an XML tree.
 *
 * NOTE: This is synchronized with DOM Level1 values
 *       See http://www.w3.org/TR/REC-DOM-Level-1/
 *
 * Actually this had diverged a bit, and now XML_DOCUMENT_TYPE_NODE should
 * be deprecated to use an XML_DTD_NODE.
 */
typedef enum {
    XML_ELEMENT_NODE=		1,
    XML_ATTRIBUTE_NODE=		2,
    XML_TEXT_NODE=		3,
    XML_CDATA_SECTION_NODE=	4,
    XML_ENTITY_REF_NODE=	5,
    XML_ENTITY_NODE=		6,
    XML_PI_NODE=		7,
    XML_COMMENT_NODE=		8,
    XML_DOCUMENT_NODE=		9,
    XML_DOCUMENT_TYPE_NODE=	10,
    XML_DOCUMENT_FRAG_NODE=	11,
    XML_NOTATION_NODE=		12,
    XML_HTML_DOCUMENT_NODE=	13,
    XML_DTD_NODE=		14,
    XML_ELEMENT_DECL=		15,
    XML_ATTRIBUTE_DECL=		16,
    XML_ENTITY_DECL=		17,
    XML_NAMESPACE_DECL=		18,
    XML_XINCLUDE_START=		19,
    XML_XINCLUDE_END=		20
} hw_xmlElementType;


/**
 * hw_xmlAttributeType:
 *
 * A DTD Attribute type definition.
 */

typedef enum {
    XML_ATTRIBUTE_CDATA = 1,
    XML_ATTRIBUTE_ID,
    XML_ATTRIBUTE_IDREF	,
    XML_ATTRIBUTE_IDREFS,
    XML_ATTRIBUTE_ENTITY,
    XML_ATTRIBUTE_ENTITIES,
    XML_ATTRIBUTE_NMTOKEN,
    XML_ATTRIBUTE_NMTOKENS,
    XML_ATTRIBUTE_ENUMERATION,
    XML_ATTRIBUTE_NOTATION
} hw_xmlAttributeType;

/**
 * hw_xmlAttributeDefault:
 *
 * A DTD Attribute default definition.
 */

typedef enum {
    XML_ATTRIBUTE_NONE = 1,
    XML_ATTRIBUTE_REQUIRED,
    XML_ATTRIBUTE_IMPLIED,
    XML_ATTRIBUTE_FIXED
} hw_xmlAttributeDefault;


/**
 * hw_xmlElementContentType:
 *
 * Possible definitions of element content types.
 */
typedef enum {
    XML_ELEMENT_CONTENT_PCDATA = 1,
    XML_ELEMENT_CONTENT_ELEMENT,
    XML_ELEMENT_CONTENT_SEQ,
    XML_ELEMENT_CONTENT_OR
} hw_xmlElementContentType;

/**
 * hw_xmlElementContentOccur:
 *
 * Possible definitions of element content occurrences.
 */
typedef enum {
    XML_ELEMENT_CONTENT_ONCE = 1,
    XML_ELEMENT_CONTENT_OPT,
    XML_ELEMENT_CONTENT_MULT,
    XML_ELEMENT_CONTENT_PLUS
} hw_xmlElementContentOccur;



/**
 * hw_xmlElementTypeVal:
 *
 * The different possibilities for an element content type.
 */

typedef enum {
    XML_ELEMENT_TYPE_UNDEFINED = 0,
    XML_ELEMENT_TYPE_EMPTY = 1,
    XML_ELEMENT_TYPE_ANY,
    XML_ELEMENT_TYPE_MIXED,
    XML_ELEMENT_TYPE_ELEMENT
} hw_xmlElementTypeVal;


/*
 * The different valid entity types.
 */
typedef enum {
    XML_INTERNAL_GENERAL_ENTITY = 1,
    XML_EXTERNAL_GENERAL_PARSED_ENTITY = 2,
    XML_EXTERNAL_GENERAL_UNPARSED_ENTITY = 3,
    XML_INTERNAL_PARAMETER_ENTITY = 4,
    XML_EXTERNAL_PARAMETER_ENTITY = 5,
    XML_INTERNAL_PREDEFINED_ENTITY = 6
} hw_xmlEntityType;



/**
 * hw_xmlErrorLevel:
 *
 * Indicates the level of an error
 */
typedef enum {
    XML_ERR_NONE = 0,
    XML_ERR_WARNING = 1,	/* A simple warning */
    XML_ERR_ERROR = 2,		/* A recoverable error */
    XML_ERR_FATAL = 3		/* A fatal error */
} hw_xmlErrorLevel;

/**
 * hw_xmlErrorDomain:
 *
 * Indicates where an error may have come from
 */
typedef enum {
    XML_FROM_NONE = 0,
    XML_FROM_PARSER,	/* The XML parser */
    XML_FROM_TREE,	/* The tree module */
    XML_FROM_NAMESPACE,	/* The XML Namespace module */
    XML_FROM_DTD,	/* The XML DTD validation with parser context*/
    XML_FROM_HTML,	/* The HTML parser */
    XML_FROM_MEMORY,	/* The memory allocator */
    XML_FROM_OUTPUT,	/* The serialization code */
    XML_FROM_IO,	/* The Input/Output stack */
    XML_FROM_FTP,	/* The FTP module */
    XML_FROM_HTTP,	/* The HTTP module */
    XML_FROM_XINCLUDE,	/* The XInclude processing */
    XML_FROM_XPATH,	/* The XPath module */
    XML_FROM_XPOINTER,	/* The XPointer module */
    XML_FROM_REGEXP,	/* The regular expressions module */
    XML_FROM_DATATYPE,	/* The W3C XML Schemas Datatype module */
    XML_FROM_SCHEMASP,	/* The W3C XML Schemas parser module */
    XML_FROM_SCHEMASV,	/* The W3C XML Schemas validation module */
    XML_FROM_RELAXNGP,	/* The Relax-NG parser module */
    XML_FROM_RELAXNGV,	/* The Relax-NG validator module */
    XML_FROM_CATALOG,	/* The Catalog module */
    XML_FROM_C14N,	/* The Canonicalization module */
    XML_FROM_XSLT,	/* The XSLT engine from libxslt */
    XML_FROM_VALID,	/* The XML DTD validation with valid context */
    XML_FROM_CHECK,	/* The error checking module */
    XML_FROM_WRITER,	/* The xmlwriter module */
    XML_FROM_MODULE,	/* The dynamically loaded module module*/
    XML_FROM_I18N 	/* The module handling character conversion */
} hw_xmlErrorDomain;


/**
 * hw_xmlParserError:
 *
 * This is an error that the XML (or HTML) parser can generate
 */
typedef enum {
    XML_ERR_OK = 0,
    XML_ERR_INTERNAL_ERROR, /* 1 */
    XML_ERR_NO_MEMORY, /* 2 */
    XML_ERR_DOCUMENT_START, /* 3 */
    XML_ERR_DOCUMENT_EMPTY, /* 4 */
    XML_ERR_DOCUMENT_END, /* 5 */
    XML_ERR_INVALID_HEX_CHARREF, /* 6 */
    XML_ERR_INVALID_DEC_CHARREF, /* 7 */
    XML_ERR_INVALID_CHARREF, /* 8 */
    XML_ERR_INVALID_CHAR, /* 9 */
    XML_ERR_CHARREF_AT_EOF, /* 10 */
    XML_ERR_CHARREF_IN_PROLOG, /* 11 */
    XML_ERR_CHARREF_IN_EPILOG, /* 12 */
    XML_ERR_CHARREF_IN_DTD, /* 13 */
    XML_ERR_ENTITYREF_AT_EOF, /* 14 */
    XML_ERR_ENTITYREF_IN_PROLOG, /* 15 */
    XML_ERR_ENTITYREF_IN_EPILOG, /* 16 */
    XML_ERR_ENTITYREF_IN_DTD, /* 17 */
    XML_ERR_PEREF_AT_EOF, /* 18 */
    XML_ERR_PEREF_IN_PROLOG, /* 19 */
    XML_ERR_PEREF_IN_EPILOG, /* 20 */
    XML_ERR_PEREF_IN_INT_SUBSET, /* 21 */
    XML_ERR_ENTITYREF_NO_NAME, /* 22 */
    XML_ERR_ENTITYREF_SEMICOL_MISSING, /* 23 */
    XML_ERR_PEREF_NO_NAME, /* 24 */
    XML_ERR_PEREF_SEMICOL_MISSING, /* 25 */
    XML_ERR_UNDECLARED_ENTITY, /* 26 */
    XML_WAR_UNDECLARED_ENTITY, /* 27 */
    XML_ERR_UNPARSED_ENTITY, /* 28 */
    XML_ERR_ENTITY_IS_EXTERNAL, /* 29 */
    XML_ERR_ENTITY_IS_PARAMETER, /* 30 */
    XML_ERR_UNKNOWN_ENCODING, /* 31 */
    XML_ERR_UNSUPPORTED_ENCODING, /* 32 */
    XML_ERR_STRING_NOT_STARTED, /* 33 */
    XML_ERR_STRING_NOT_CLOSED, /* 34 */
    XML_ERR_NS_DECL_ERROR, /* 35 */
    XML_ERR_ENTITY_NOT_STARTED, /* 36 */
    XML_ERR_ENTITY_NOT_FINISHED, /* 37 */
    XML_ERR_LT_IN_ATTRIBUTE, /* 38 */
    XML_ERR_ATTRIBUTE_NOT_STARTED, /* 39 */
    XML_ERR_ATTRIBUTE_NOT_FINISHED, /* 40 */
    XML_ERR_ATTRIBUTE_WITHOUT_VALUE, /* 41 */
    XML_ERR_ATTRIBUTE_REDEFINED, /* 42 */
    XML_ERR_LITERAL_NOT_STARTED, /* 43 */
    XML_ERR_LITERAL_NOT_FINISHED, /* 44 */
    XML_ERR_COMMENT_NOT_FINISHED, /* 45 */
    XML_ERR_PI_NOT_STARTED, /* 46 */
    XML_ERR_PI_NOT_FINISHED, /* 47 */
    XML_ERR_NOTATION_NOT_STARTED, /* 48 */
    XML_ERR_NOTATION_NOT_FINISHED, /* 49 */
    XML_ERR_ATTLIST_NOT_STARTED, /* 50 */
    XML_ERR_ATTLIST_NOT_FINISHED, /* 51 */
    XML_ERR_MIXED_NOT_STARTED, /* 52 */
    XML_ERR_MIXED_NOT_FINISHED, /* 53 */
    XML_ERR_ELEMCONTENT_NOT_STARTED, /* 54 */
    XML_ERR_ELEMCONTENT_NOT_FINISHED, /* 55 */
    XML_ERR_XMLDECL_NOT_STARTED, /* 56 */
    XML_ERR_XMLDECL_NOT_FINISHED, /* 57 */
    XML_ERR_CONDSEC_NOT_STARTED, /* 58 */
    XML_ERR_CONDSEC_NOT_FINISHED, /* 59 */
    XML_ERR_EXT_SUBSET_NOT_FINISHED, /* 60 */
    XML_ERR_DOCTYPE_NOT_FINISHED, /* 61 */
    XML_ERR_MISPLACED_CDATA_END, /* 62 */
    XML_ERR_CDATA_NOT_FINISHED, /* 63 */
    XML_ERR_RESERVED_XML_NAME, /* 64 */
    XML_ERR_SPACE_REQUIRED, /* 65 */
    XML_ERR_SEPARATOR_REQUIRED, /* 66 */
    XML_ERR_NMTOKEN_REQUIRED, /* 67 */
    XML_ERR_NAME_REQUIRED, /* 68 */
    XML_ERR_PCDATA_REQUIRED, /* 69 */
    XML_ERR_URI_REQUIRED, /* 70 */
    XML_ERR_PUBID_REQUIRED, /* 71 */
    XML_ERR_LT_REQUIRED, /* 72 */
    XML_ERR_GT_REQUIRED, /* 73 */
    XML_ERR_LTSLASH_REQUIRED, /* 74 */
    XML_ERR_EQUAL_REQUIRED, /* 75 */
    XML_ERR_TAG_NAME_MISMATCH, /* 76 */
    XML_ERR_TAG_NOT_FINISHED, /* 77 */
    XML_ERR_STANDALONE_VALUE, /* 78 */
    XML_ERR_ENCODING_NAME, /* 79 */
    XML_ERR_HYPHEN_IN_COMMENT, /* 80 */
    XML_ERR_INVALID_ENCODING, /* 81 */
    XML_ERR_EXT_ENTITY_STANDALONE, /* 82 */
    XML_ERR_CONDSEC_INVALID, /* 83 */
    XML_ERR_VALUE_REQUIRED, /* 84 */
    XML_ERR_NOT_WELL_BALANCED, /* 85 */
    XML_ERR_EXTRA_CONTENT, /* 86 */
    XML_ERR_ENTITY_CHAR_ERROR, /* 87 */
    XML_ERR_ENTITY_PE_INTERNAL, /* 88 */
    XML_ERR_ENTITY_LOOP, /* 89 */
    XML_ERR_ENTITY_BOUNDARY, /* 90 */
    XML_ERR_INVALID_URI, /* 91 */
    XML_ERR_URI_FRAGMENT, /* 92 */
    XML_WAR_CATALOG_PI, /* 93 */
    XML_ERR_NO_DTD, /* 94 */
    XML_ERR_CONDSEC_INVALID_KEYWORD, /* 95 */
    XML_ERR_VERSION_MISSING, /* 96 */
    XML_WAR_UNKNOWN_VERSION, /* 97 */
    XML_WAR_LANG_VALUE, /* 98 */
    XML_WAR_NS_URI, /* 99 */
    XML_WAR_NS_URI_RELATIVE, /* 100 */
    XML_ERR_MISSING_ENCODING, /* 101 */
    XML_WAR_SPACE_VALUE, /* 102 */
    XML_ERR_NOT_STANDALONE, /* 103 */
    XML_ERR_ENTITY_PROCESSING, /* 104 */
    XML_ERR_NOTATION_PROCESSING, /* 105 */
    XML_WAR_NS_COLUMN, /* 106 */
    XML_WAR_ENTITY_REDEFINED, /* 107 */
    XML_NS_ERR_XML_NAMESPACE = 200,
    XML_NS_ERR_UNDEFINED_NAMESPACE, /* 201 */
    XML_NS_ERR_QNAME, /* 202 */
    XML_NS_ERR_ATTRIBUTE_REDEFINED, /* 203 */
    XML_NS_ERR_EMPTY, /* 204 */
    XML_DTD_ATTRIBUTE_DEFAULT = 500,
    XML_DTD_ATTRIBUTE_REDEFINED, /* 501 */
    XML_DTD_ATTRIBUTE_VALUE, /* 502 */
    XML_DTD_CONTENT_ERROR, /* 503 */
    XML_DTD_CONTENT_MODEL, /* 504 */
    XML_DTD_CONTENT_NOT_DETERMINIST, /* 505 */
    XML_DTD_DIFFERENT_PREFIX, /* 506 */
    XML_DTD_ELEM_DEFAULT_NAMESPACE, /* 507 */
    XML_DTD_ELEM_NAMESPACE, /* 508 */
    XML_DTD_ELEM_REDEFINED, /* 509 */
    XML_DTD_EMPTY_NOTATION, /* 510 */
    XML_DTD_ENTITY_TYPE, /* 511 */
    XML_DTD_ID_FIXED, /* 512 */
    XML_DTD_ID_REDEFINED, /* 513 */
    XML_DTD_ID_SUBSET, /* 514 */
    XML_DTD_INVALID_CHILD, /* 515 */
    XML_DTD_INVALID_DEFAULT, /* 516 */
    XML_DTD_LOAD_ERROR, /* 517 */
    XML_DTD_MISSING_ATTRIBUTE, /* 518 */
    XML_DTD_MIXED_CORRUPT, /* 519 */
    XML_DTD_MULTIPLE_ID, /* 520 */
    XML_DTD_NO_DOC, /* 521 */
    XML_DTD_NO_DTD, /* 522 */
    XML_DTD_NO_ELEM_NAME, /* 523 */
    XML_DTD_NO_PREFIX, /* 524 */
    XML_DTD_NO_ROOT, /* 525 */
    XML_DTD_NOTATION_REDEFINED, /* 526 */
    XML_DTD_NOTATION_VALUE, /* 527 */
    XML_DTD_NOT_EMPTY, /* 528 */
    XML_DTD_NOT_PCDATA, /* 529 */
    XML_DTD_NOT_STANDALONE, /* 530 */
    XML_DTD_ROOT_NAME, /* 531 */
    XML_DTD_STANDALONE_WHITE_SPACE, /* 532 */
    XML_DTD_UNKNOWN_ATTRIBUTE, /* 533 */
    XML_DTD_UNKNOWN_ELEM, /* 534 */
    XML_DTD_UNKNOWN_ENTITY, /* 535 */
    XML_DTD_UNKNOWN_ID, /* 536 */
    XML_DTD_UNKNOWN_NOTATION, /* 537 */
    XML_DTD_STANDALONE_DEFAULTED, /* 538 */
    XML_DTD_XMLID_VALUE, /* 539 */
    XML_DTD_XMLID_TYPE, /* 540 */
    XML_HTML_STRUCURE_ERROR = 800,
    XML_HTML_UNKNOWN_TAG, /* 801 */
    XML_RNGP_ANYNAME_ATTR_ANCESTOR = 1000,
    XML_RNGP_ATTR_CONFLICT, /* 1001 */
    XML_RNGP_ATTRIBUTE_CHILDREN, /* 1002 */
    XML_RNGP_ATTRIBUTE_CONTENT, /* 1003 */
    XML_RNGP_ATTRIBUTE_EMPTY, /* 1004 */
    XML_RNGP_ATTRIBUTE_NOOP, /* 1005 */
    XML_RNGP_CHOICE_CONTENT, /* 1006 */
    XML_RNGP_CHOICE_EMPTY, /* 1007 */
    XML_RNGP_CREATE_FAILURE, /* 1008 */
    XML_RNGP_DATA_CONTENT, /* 1009 */
    XML_RNGP_DEF_CHOICE_AND_INTERLEAVE, /* 1010 */
    XML_RNGP_DEFINE_CREATE_FAILED, /* 1011 */
    XML_RNGP_DEFINE_EMPTY, /* 1012 */
    XML_RNGP_DEFINE_MISSING, /* 1013 */
    XML_RNGP_DEFINE_NAME_MISSING, /* 1014 */
    XML_RNGP_ELEM_CONTENT_EMPTY, /* 1015 */
    XML_RNGP_ELEM_CONTENT_ERROR, /* 1016 */
    XML_RNGP_ELEMENT_EMPTY, /* 1017 */
    XML_RNGP_ELEMENT_CONTENT, /* 1018 */
    XML_RNGP_ELEMENT_NAME, /* 1019 */
    XML_RNGP_ELEMENT_NO_CONTENT, /* 1020 */
    XML_RNGP_ELEM_TEXT_CONFLICT, /* 1021 */
    XML_RNGP_EMPTY, /* 1022 */
    XML_RNGP_EMPTY_CONSTRUCT, /* 1023 */
    XML_RNGP_EMPTY_CONTENT, /* 1024 */
    XML_RNGP_EMPTY_NOT_EMPTY, /* 1025 */
    XML_RNGP_ERROR_TYPE_LIB, /* 1026 */
    XML_RNGP_EXCEPT_EMPTY, /* 1027 */
    XML_RNGP_EXCEPT_MISSING, /* 1028 */
    XML_RNGP_EXCEPT_MULTIPLE, /* 1029 */
    XML_RNGP_EXCEPT_NO_CONTENT, /* 1030 */
    XML_RNGP_EXTERNALREF_EMTPY, /* 1031 */
    XML_RNGP_EXTERNAL_REF_FAILURE, /* 1032 */
    XML_RNGP_EXTERNALREF_RECURSE, /* 1033 */
    XML_RNGP_FORBIDDEN_ATTRIBUTE, /* 1034 */
    XML_RNGP_FOREIGN_ELEMENT, /* 1035 */
    XML_RNGP_GRAMMAR_CONTENT, /* 1036 */
    XML_RNGP_GRAMMAR_EMPTY, /* 1037 */
    XML_RNGP_GRAMMAR_MISSING, /* 1038 */
    XML_RNGP_GRAMMAR_NO_START, /* 1039 */
    XML_RNGP_GROUP_ATTR_CONFLICT, /* 1040 */
    XML_RNGP_HREF_ERROR, /* 1041 */
    XML_RNGP_INCLUDE_EMPTY, /* 1042 */
    XML_RNGP_INCLUDE_FAILURE, /* 1043 */
    XML_RNGP_INCLUDE_RECURSE, /* 1044 */
    XML_RNGP_INTERLEAVE_ADD, /* 1045 */
    XML_RNGP_INTERLEAVE_CREATE_FAILED, /* 1046 */
    XML_RNGP_INTERLEAVE_EMPTY, /* 1047 */
    XML_RNGP_INTERLEAVE_NO_CONTENT, /* 1048 */
    XML_RNGP_INVALID_DEFINE_NAME, /* 1049 */
    XML_RNGP_INVALID_URI, /* 1050 */
    XML_RNGP_INVALID_VALUE, /* 1051 */
    XML_RNGP_MISSING_HREF, /* 1052 */
    XML_RNGP_NAME_MISSING, /* 1053 */
    XML_RNGP_NEED_COMBINE, /* 1054 */
    XML_RNGP_NOTALLOWED_NOT_EMPTY, /* 1055 */
    XML_RNGP_NSNAME_ATTR_ANCESTOR, /* 1056 */
    XML_RNGP_NSNAME_NO_NS, /* 1057 */
    XML_RNGP_PARAM_FORBIDDEN, /* 1058 */
    XML_RNGP_PARAM_NAME_MISSING, /* 1059 */
    XML_RNGP_PARENTREF_CREATE_FAILED, /* 1060 */
    XML_RNGP_PARENTREF_NAME_INVALID, /* 1061 */
    XML_RNGP_PARENTREF_NO_NAME, /* 1062 */
    XML_RNGP_PARENTREF_NO_PARENT, /* 1063 */
    XML_RNGP_PARENTREF_NOT_EMPTY, /* 1064 */
    XML_RNGP_PARSE_ERROR, /* 1065 */
    XML_RNGP_PAT_ANYNAME_EXCEPT_ANYNAME, /* 1066 */
    XML_RNGP_PAT_ATTR_ATTR, /* 1067 */
    XML_RNGP_PAT_ATTR_ELEM, /* 1068 */
    XML_RNGP_PAT_DATA_EXCEPT_ATTR, /* 1069 */
    XML_RNGP_PAT_DATA_EXCEPT_ELEM, /* 1070 */
    XML_RNGP_PAT_DATA_EXCEPT_EMPTY, /* 1071 */
    XML_RNGP_PAT_DATA_EXCEPT_GROUP, /* 1072 */
    XML_RNGP_PAT_DATA_EXCEPT_INTERLEAVE, /* 1073 */
    XML_RNGP_PAT_DATA_EXCEPT_LIST, /* 1074 */
    XML_RNGP_PAT_DATA_EXCEPT_ONEMORE, /* 1075 */
    XML_RNGP_PAT_DATA_EXCEPT_REF, /* 1076 */
    XML_RNGP_PAT_DATA_EXCEPT_TEXT, /* 1077 */
    XML_RNGP_PAT_LIST_ATTR, /* 1078 */
    XML_RNGP_PAT_LIST_ELEM, /* 1079 */
    XML_RNGP_PAT_LIST_INTERLEAVE, /* 1080 */
    XML_RNGP_PAT_LIST_LIST, /* 1081 */
    XML_RNGP_PAT_LIST_REF, /* 1082 */
    XML_RNGP_PAT_LIST_TEXT, /* 1083 */
    XML_RNGP_PAT_NSNAME_EXCEPT_ANYNAME, /* 1084 */
    XML_RNGP_PAT_NSNAME_EXCEPT_NSNAME, /* 1085 */
    XML_RNGP_PAT_ONEMORE_GROUP_ATTR, /* 1086 */
    XML_RNGP_PAT_ONEMORE_INTERLEAVE_ATTR, /* 1087 */
    XML_RNGP_PAT_START_ATTR, /* 1088 */
    XML_RNGP_PAT_START_DATA, /* 1089 */
    XML_RNGP_PAT_START_EMPTY, /* 1090 */
    XML_RNGP_PAT_START_GROUP, /* 1091 */
    XML_RNGP_PAT_START_INTERLEAVE, /* 1092 */
    XML_RNGP_PAT_START_LIST, /* 1093 */
    XML_RNGP_PAT_START_ONEMORE, /* 1094 */
    XML_RNGP_PAT_START_TEXT, /* 1095 */
    XML_RNGP_PAT_START_VALUE, /* 1096 */
    XML_RNGP_PREFIX_UNDEFINED, /* 1097 */
    XML_RNGP_REF_CREATE_FAILED, /* 1098 */
    XML_RNGP_REF_CYCLE, /* 1099 */
    XML_RNGP_REF_NAME_INVALID, /* 1100 */
    XML_RNGP_REF_NO_DEF, /* 1101 */
    XML_RNGP_REF_NO_NAME, /* 1102 */
    XML_RNGP_REF_NOT_EMPTY, /* 1103 */
    XML_RNGP_START_CHOICE_AND_INTERLEAVE, /* 1104 */
    XML_RNGP_START_CONTENT, /* 1105 */
    XML_RNGP_START_EMPTY, /* 1106 */
    XML_RNGP_START_MISSING, /* 1107 */
    XML_RNGP_TEXT_EXPECTED, /* 1108 */
    XML_RNGP_TEXT_HAS_CHILD, /* 1109 */
    XML_RNGP_TYPE_MISSING, /* 1110 */
    XML_RNGP_TYPE_NOT_FOUND, /* 1111 */
    XML_RNGP_TYPE_VALUE, /* 1112 */
    XML_RNGP_UNKNOWN_ATTRIBUTE, /* 1113 */
    XML_RNGP_UNKNOWN_COMBINE, /* 1114 */
    XML_RNGP_UNKNOWN_CONSTRUCT, /* 1115 */
    XML_RNGP_UNKNOWN_TYPE_LIB, /* 1116 */
    XML_RNGP_URI_FRAGMENT, /* 1117 */
    XML_RNGP_URI_NOT_ABSOLUTE, /* 1118 */
    XML_RNGP_VALUE_EMPTY, /* 1119 */
    XML_RNGP_VALUE_NO_CONTENT, /* 1120 */
    XML_RNGP_XMLNS_NAME, /* 1121 */
    XML_RNGP_XML_NS, /* 1122 */
    XML_XPATH_EXPRESSION_OK = 1200,
    XML_XPATH_NUMBER_ERROR, /* 1201 */
    XML_XPATH_UNFINISHED_LITERAL_ERROR, /* 1202 */
    XML_XPATH_START_LITERAL_ERROR, /* 1203 */
    XML_XPATH_VARIABLE_REF_ERROR, /* 1204 */
    XML_XPATH_UNDEF_VARIABLE_ERROR, /* 1205 */
    XML_XPATH_INVALID_PREDICATE_ERROR, /* 1206 */
    XML_XPATH_EXPR_ERROR, /* 1207 */
    XML_XPATH_UNCLOSED_ERROR, /* 1208 */
    XML_XPATH_UNKNOWN_FUNC_ERROR, /* 1209 */
    XML_XPATH_INVALID_OPERAND, /* 1210 */
    XML_XPATH_INVALID_TYPE, /* 1211 */
    XML_XPATH_INVALID_ARITY, /* 1212 */
    XML_XPATH_INVALID_CTXT_SIZE, /* 1213 */
    XML_XPATH_INVALID_CTXT_POSITION, /* 1214 */
    XML_XPATH_MEMORY_ERROR, /* 1215 */
    XML_XPTR_SYNTAX_ERROR, /* 1216 */
    XML_XPTR_RESOURCE_ERROR, /* 1217 */
    XML_XPTR_SUB_RESOURCE_ERROR, /* 1218 */
    XML_XPATH_UNDEF_PREFIX_ERROR, /* 1219 */
    XML_XPATH_ENCODING_ERROR, /* 1220 */
    XML_XPATH_INVALID_CHAR_ERROR, /* 1221 */
    XML_TREE_INVALID_HEX = 1300,
    XML_TREE_INVALID_DEC, /* 1301 */
    XML_TREE_UNTERMINATED_ENTITY, /* 1302 */
    XML_SAVE_NOT_UTF8 = 1400,
    XML_SAVE_CHAR_INVALID, /* 1401 */
    XML_SAVE_NO_DOCTYPE, /* 1402 */
    XML_SAVE_UNKNOWN_ENCODING, /* 1403 */
    XML_REGEXP_COMPILE_ERROR = 1450,
    XML_IO_UNKNOWN = 1500,
    XML_IO_EACCES, /* 1501 */
    XML_IO_EAGAIN, /* 1502 */
    XML_IO_EBADF, /* 1503 */
    XML_IO_EBADMSG, /* 1504 */
    XML_IO_EBUSY, /* 1505 */
    XML_IO_ECANCELED, /* 1506 */
    XML_IO_ECHILD, /* 1507 */
    XML_IO_EDEADLK, /* 1508 */
    XML_IO_EDOM, /* 1509 */
    XML_IO_EEXIST, /* 1510 */
    XML_IO_EFAULT, /* 1511 */
    XML_IO_EFBIG, /* 1512 */
    XML_IO_EINPROGRESS, /* 1513 */
    XML_IO_EINTR, /* 1514 */
    XML_IO_EINVAL, /* 1515 */
    XML_IO_EIO, /* 1516 */
    XML_IO_EISDIR, /* 1517 */
    XML_IO_EMFILE, /* 1518 */
    XML_IO_EMLINK, /* 1519 */
    XML_IO_EMSGSIZE, /* 1520 */
    XML_IO_ENAMETOOLONG, /* 1521 */
    XML_IO_ENFILE, /* 1522 */
    XML_IO_ENODEV, /* 1523 */
    XML_IO_ENOENT, /* 1524 */
    XML_IO_ENOEXEC, /* 1525 */
    XML_IO_ENOLCK, /* 1526 */
    XML_IO_ENOMEM, /* 1527 */
    XML_IO_ENOSPC, /* 1528 */
    XML_IO_ENOSYS, /* 1529 */
    XML_IO_ENOTDIR, /* 1530 */
    XML_IO_ENOTEMPTY, /* 1531 */
    XML_IO_ENOTSUP, /* 1532 */
    XML_IO_ENOTTY, /* 1533 */
    XML_IO_ENXIO, /* 1534 */
    XML_IO_EPERM, /* 1535 */
    XML_IO_EPIPE, /* 1536 */
    XML_IO_ERANGE, /* 1537 */
    XML_IO_EROFS, /* 1538 */
    XML_IO_ESPIPE, /* 1539 */
    XML_IO_ESRCH, /* 1540 */
    XML_IO_ETIMEDOUT, /* 1541 */
    XML_IO_EXDEV, /* 1542 */
    XML_IO_NETWORK_ATTEMPT, /* 1543 */
    XML_IO_ENCODER, /* 1544 */
    XML_IO_FLUSH, /* 1545 */
    XML_IO_WRITE, /* 1546 */
    XML_IO_NO_INPUT, /* 1547 */
    XML_IO_BUFFER_FULL, /* 1548 */
    XML_IO_LOAD_ERROR, /* 1549 */
    XML_IO_ENOTSOCK, /* 1550 */
    XML_IO_EISCONN, /* 1551 */
    XML_IO_ECONNREFUSED, /* 1552 */
    XML_IO_ENETUNREACH, /* 1553 */
    XML_IO_EADDRINUSE, /* 1554 */
    XML_IO_EALREADY, /* 1555 */
    XML_IO_EAFNOSUPPORT, /* 1556 */
    XML_XINCLUDE_RECURSION=1600,
    XML_XINCLUDE_PARSE_VALUE, /* 1601 */
    XML_XINCLUDE_ENTITY_DEF_MISMATCH, /* 1602 */
    XML_XINCLUDE_NO_HREF, /* 1603 */
    XML_XINCLUDE_NO_FALLBACK, /* 1604 */
    XML_XINCLUDE_HREF_URI, /* 1605 */
    XML_XINCLUDE_TEXT_FRAGMENT, /* 1606 */
    XML_XINCLUDE_TEXT_DOCUMENT, /* 1607 */
    XML_XINCLUDE_INVALID_CHAR, /* 1608 */
    XML_XINCLUDE_BUILD_FAILED, /* 1609 */
    XML_XINCLUDE_UNKNOWN_ENCODING, /* 1610 */
    XML_XINCLUDE_MULTIPLE_ROOT, /* 1611 */
    XML_XINCLUDE_XPTR_FAILED, /* 1612 */
    XML_XINCLUDE_XPTR_RESULT, /* 1613 */
    XML_XINCLUDE_INCLUDE_IN_INCLUDE, /* 1614 */
    XML_XINCLUDE_FALLBACKS_IN_INCLUDE, /* 1615 */
    XML_XINCLUDE_FALLBACK_NOT_IN_INCLUDE, /* 1616 */
    XML_XINCLUDE_DEPRECATED_NS, /* 1617 */
    XML_XINCLUDE_FRAGMENT_ID, /* 1618 */
    XML_CATALOG_MISSING_ATTR = 1650,
    XML_CATALOG_ENTRY_BROKEN, /* 1651 */
    XML_CATALOG_PREFER_VALUE, /* 1652 */
    XML_CATALOG_NOT_CATALOG, /* 1653 */
    XML_CATALOG_RECURSION, /* 1654 */
    XML_SCHEMAP_PREFIX_UNDEFINED = 1700,
    XML_SCHEMAP_ATTRFORMDEFAULT_VALUE, /* 1701 */
    XML_SCHEMAP_ATTRGRP_NONAME_NOREF, /* 1702 */
    XML_SCHEMAP_ATTR_NONAME_NOREF, /* 1703 */
    XML_SCHEMAP_COMPLEXTYPE_NONAME_NOREF, /* 1704 */
    XML_SCHEMAP_ELEMFORMDEFAULT_VALUE, /* 1705 */
    XML_SCHEMAP_ELEM_NONAME_NOREF, /* 1706 */
    XML_SCHEMAP_EXTENSION_NO_BASE, /* 1707 */
    XML_SCHEMAP_FACET_NO_VALUE, /* 1708 */
    XML_SCHEMAP_FAILED_BUILD_IMPORT, /* 1709 */
    XML_SCHEMAP_GROUP_NONAME_NOREF, /* 1710 */
    XML_SCHEMAP_IMPORT_NAMESPACE_NOT_URI, /* 1711 */
    XML_SCHEMAP_IMPORT_REDEFINE_NSNAME, /* 1712 */
    XML_SCHEMAP_IMPORT_SCHEMA_NOT_URI, /* 1713 */
    XML_SCHEMAP_INVALID_BOOLEAN, /* 1714 */
    XML_SCHEMAP_INVALID_ENUM, /* 1715 */
    XML_SCHEMAP_INVALID_FACET, /* 1716 */
    XML_SCHEMAP_INVALID_FACET_VALUE, /* 1717 */
    XML_SCHEMAP_INVALID_MAXOCCURS, /* 1718 */
    XML_SCHEMAP_INVALID_MINOCCURS, /* 1719 */
    XML_SCHEMAP_INVALID_REF_AND_SUBTYPE, /* 1720 */
    XML_SCHEMAP_INVALID_WHITE_SPACE, /* 1721 */
    XML_SCHEMAP_NOATTR_NOREF, /* 1722 */
    XML_SCHEMAP_NOTATION_NO_NAME, /* 1723 */
    XML_SCHEMAP_NOTYPE_NOREF, /* 1724 */
    XML_SCHEMAP_REF_AND_SUBTYPE, /* 1725 */
    XML_SCHEMAP_RESTRICTION_NONAME_NOREF, /* 1726 */
    XML_SCHEMAP_SIMPLETYPE_NONAME, /* 1727 */
    XML_SCHEMAP_TYPE_AND_SUBTYPE, /* 1728 */
    XML_SCHEMAP_UNKNOWN_ALL_CHILD, /* 1729 */
    XML_SCHEMAP_UNKNOWN_ANYATTRIBUTE_CHILD, /* 1730 */
    XML_SCHEMAP_UNKNOWN_ATTR_CHILD, /* 1731 */
    XML_SCHEMAP_UNKNOWN_ATTRGRP_CHILD, /* 1732 */
    XML_SCHEMAP_UNKNOWN_ATTRIBUTE_GROUP, /* 1733 */
    XML_SCHEMAP_UNKNOWN_BASE_TYPE, /* 1734 */
    XML_SCHEMAP_UNKNOWN_CHOICE_CHILD, /* 1735 */
    XML_SCHEMAP_UNKNOWN_COMPLEXCONTENT_CHILD, /* 1736 */
    XML_SCHEMAP_UNKNOWN_COMPLEXTYPE_CHILD, /* 1737 */
    XML_SCHEMAP_UNKNOWN_ELEM_CHILD, /* 1738 */
    XML_SCHEMAP_UNKNOWN_EXTENSION_CHILD, /* 1739 */
    XML_SCHEMAP_UNKNOWN_FACET_CHILD, /* 1740 */
    XML_SCHEMAP_UNKNOWN_FACET_TYPE, /* 1741 */
    XML_SCHEMAP_UNKNOWN_GROUP_CHILD, /* 1742 */
    XML_SCHEMAP_UNKNOWN_IMPORT_CHILD, /* 1743 */
    XML_SCHEMAP_UNKNOWN_LIST_CHILD, /* 1744 */
    XML_SCHEMAP_UNKNOWN_NOTATION_CHILD, /* 1745 */
    XML_SCHEMAP_UNKNOWN_PROCESSCONTENT_CHILD, /* 1746 */
    XML_SCHEMAP_UNKNOWN_REF, /* 1747 */
    XML_SCHEMAP_UNKNOWN_RESTRICTION_CHILD, /* 1748 */
    XML_SCHEMAP_UNKNOWN_SCHEMAS_CHILD, /* 1749 */
    XML_SCHEMAP_UNKNOWN_SEQUENCE_CHILD, /* 1750 */
    XML_SCHEMAP_UNKNOWN_SIMPLECONTENT_CHILD, /* 1751 */
    XML_SCHEMAP_UNKNOWN_SIMPLETYPE_CHILD, /* 1752 */
    XML_SCHEMAP_UNKNOWN_TYPE, /* 1753 */
    XML_SCHEMAP_UNKNOWN_UNION_CHILD, /* 1754 */
    XML_SCHEMAP_ELEM_DEFAULT_FIXED, /* 1755 */
    XML_SCHEMAP_REGEXP_INVALID, /* 1756 */
    XML_SCHEMAP_FAILED_LOAD, /* 1757 */
    XML_SCHEMAP_NOTHING_TO_PARSE, /* 1758 */
    XML_SCHEMAP_NOROOT, /* 1759 */
    XML_SCHEMAP_REDEFINED_GROUP, /* 1760 */
    XML_SCHEMAP_REDEFINED_TYPE, /* 1761 */
    XML_SCHEMAP_REDEFINED_ELEMENT, /* 1762 */
    XML_SCHEMAP_REDEFINED_ATTRGROUP, /* 1763 */
    XML_SCHEMAP_REDEFINED_ATTR, /* 1764 */
    XML_SCHEMAP_REDEFINED_NOTATION, /* 1765 */
    XML_SCHEMAP_FAILED_PARSE, /* 1766 */
    XML_SCHEMAP_UNKNOWN_PREFIX, /* 1767 */
    XML_SCHEMAP_DEF_AND_PREFIX, /* 1768 */
    XML_SCHEMAP_UNKNOWN_INCLUDE_CHILD, /* 1769 */
    XML_SCHEMAP_INCLUDE_SCHEMA_NOT_URI, /* 1770 */
    XML_SCHEMAP_INCLUDE_SCHEMA_NO_URI, /* 1771 */
    XML_SCHEMAP_NOT_SCHEMA, /* 1772 */
    XML_SCHEMAP_UNKNOWN_MEMBER_TYPE, /* 1773 */
    XML_SCHEMAP_INVALID_ATTR_USE, /* 1774 */
    XML_SCHEMAP_RECURSIVE, /* 1775 */
    XML_SCHEMAP_SUPERNUMEROUS_LIST_ITEM_TYPE, /* 1776 */
    XML_SCHEMAP_INVALID_ATTR_COMBINATION, /* 1777 */
    XML_SCHEMAP_INVALID_ATTR_INLINE_COMBINATION, /* 1778 */
    XML_SCHEMAP_MISSING_SIMPLETYPE_CHILD, /* 1779 */
    XML_SCHEMAP_INVALID_ATTR_NAME, /* 1780 */
    XML_SCHEMAP_REF_AND_CONTENT, /* 1781 */
    XML_SCHEMAP_CT_PROPS_CORRECT_1, /* 1782 */
    XML_SCHEMAP_CT_PROPS_CORRECT_2, /* 1783 */
    XML_SCHEMAP_CT_PROPS_CORRECT_3, /* 1784 */
    XML_SCHEMAP_CT_PROPS_CORRECT_4, /* 1785 */
    XML_SCHEMAP_CT_PROPS_CORRECT_5, /* 1786 */
    XML_SCHEMAP_DERIVATION_OK_RESTRICTION_1, /* 1787 */
    XML_SCHEMAP_DERIVATION_OK_RESTRICTION_2_1_1, /* 1788 */
    XML_SCHEMAP_DERIVATION_OK_RESTRICTION_2_1_2, /* 1789 */
    XML_SCHEMAP_DERIVATION_OK_RESTRICTION_2_2, /* 1790 */
    XML_SCHEMAP_DERIVATION_OK_RESTRICTION_3, /* 1791 */
    XML_SCHEMAP_WILDCARD_INVALID_NS_MEMBER, /* 1792 */
    XML_SCHEMAP_INTERSECTION_NOT_EXPRESSIBLE, /* 1793 */
    XML_SCHEMAP_UNION_NOT_EXPRESSIBLE, /* 1794 */
    XML_SCHEMAP_SRC_IMPORT_3_1, /* 1795 */
    XML_SCHEMAP_SRC_IMPORT_3_2, /* 1796 */
    XML_SCHEMAP_DERIVATION_OK_RESTRICTION_4_1, /* 1797 */
    XML_SCHEMAP_DERIVATION_OK_RESTRICTION_4_2, /* 1798 */
    XML_SCHEMAP_DERIVATION_OK_RESTRICTION_4_3, /* 1799 */
    XML_SCHEMAP_COS_CT_EXTENDS_1_3, /* 1800 */
    XML_SCHEMAV_NOROOT = 1801,
    XML_SCHEMAV_UNDECLAREDELEM, /* 1802 */
    XML_SCHEMAV_NOTTOPLEVEL, /* 1803 */
    XML_SCHEMAV_MISSING, /* 1804 */
    XML_SCHEMAV_WRONGELEM, /* 1805 */
    XML_SCHEMAV_NOTYPE, /* 1806 */
    XML_SCHEMAV_NOROLLBACK, /* 1807 */
    XML_SCHEMAV_ISABSTRACT, /* 1808 */
    XML_SCHEMAV_NOTEMPTY, /* 1809 */
    XML_SCHEMAV_ELEMCONT, /* 1810 */
    XML_SCHEMAV_HAVEDEFAULT, /* 1811 */
    XML_SCHEMAV_NOTNILLABLE, /* 1812 */
    XML_SCHEMAV_EXTRACONTENT, /* 1813 */
    XML_SCHEMAV_INVALIDATTR, /* 1814 */
    XML_SCHEMAV_INVALIDELEM, /* 1815 */
    XML_SCHEMAV_NOTDETERMINIST, /* 1816 */
    XML_SCHEMAV_CONSTRUCT, /* 1817 */
    XML_SCHEMAV_INTERNAL, /* 1818 */
    XML_SCHEMAV_NOTSIMPLE, /* 1819 */
    XML_SCHEMAV_ATTRUNKNOWN, /* 1820 */
    XML_SCHEMAV_ATTRINVALID, /* 1821 */
    XML_SCHEMAV_VALUE, /* 1822 */
    XML_SCHEMAV_FACET, /* 1823 */
    XML_SCHEMAV_CVC_DATATYPE_VALID_1_2_1, /* 1824 */
    XML_SCHEMAV_CVC_DATATYPE_VALID_1_2_2, /* 1825 */
    XML_SCHEMAV_CVC_DATATYPE_VALID_1_2_3, /* 1826 */
    XML_SCHEMAV_CVC_TYPE_3_1_1, /* 1827 */
    XML_SCHEMAV_CVC_TYPE_3_1_2, /* 1828 */
    XML_SCHEMAV_CVC_FACET_VALID, /* 1829 */
    XML_SCHEMAV_CVC_LENGTH_VALID, /* 1830 */
    XML_SCHEMAV_CVC_MINLENGTH_VALID, /* 1831 */
    XML_SCHEMAV_CVC_MAXLENGTH_VALID, /* 1832 */
    XML_SCHEMAV_CVC_MININCLUSIVE_VALID, /* 1833 */
    XML_SCHEMAV_CVC_MAXINCLUSIVE_VALID, /* 1834 */
    XML_SCHEMAV_CVC_MINEXCLUSIVE_VALID, /* 1835 */
    XML_SCHEMAV_CVC_MAXEXCLUSIVE_VALID, /* 1836 */
    XML_SCHEMAV_CVC_TOTALDIGITS_VALID, /* 1837 */
    XML_SCHEMAV_CVC_FRACTIONDIGITS_VALID, /* 1838 */
    XML_SCHEMAV_CVC_PATTERN_VALID, /* 1839 */
    XML_SCHEMAV_CVC_ENUMERATION_VALID, /* 1840 */
    XML_SCHEMAV_CVC_COMPLEX_TYPE_2_1, /* 1841 */
    XML_SCHEMAV_CVC_COMPLEX_TYPE_2_2, /* 1842 */
    XML_SCHEMAV_CVC_COMPLEX_TYPE_2_3, /* 1843 */
    XML_SCHEMAV_CVC_COMPLEX_TYPE_2_4, /* 1844 */
    XML_SCHEMAV_CVC_ELT_1, /* 1845 */
    XML_SCHEMAV_CVC_ELT_2, /* 1846 */
    XML_SCHEMAV_CVC_ELT_3_1, /* 1847 */
    XML_SCHEMAV_CVC_ELT_3_2_1, /* 1848 */
    XML_SCHEMAV_CVC_ELT_3_2_2, /* 1849 */
    XML_SCHEMAV_CVC_ELT_4_1, /* 1850 */
    XML_SCHEMAV_CVC_ELT_4_2, /* 1851 */
    XML_SCHEMAV_CVC_ELT_4_3, /* 1852 */
    XML_SCHEMAV_CVC_ELT_5_1_1, /* 1853 */
    XML_SCHEMAV_CVC_ELT_5_1_2, /* 1854 */
    XML_SCHEMAV_CVC_ELT_5_2_1, /* 1855 */  
    XML_SCHEMAV_CVC_ELT_5_2_2_1, /* 1856 */
    XML_SCHEMAV_CVC_ELT_5_2_2_2_1, /* 1857 */
    XML_SCHEMAV_CVC_ELT_5_2_2_2_2, /* 1858 */
    XML_SCHEMAV_CVC_ELT_6, /* 1859 */
    XML_SCHEMAV_CVC_ELT_7, /* 1860 */
    XML_SCHEMAV_CVC_ATTRIBUTE_1, /* 1861 */
    XML_SCHEMAV_CVC_ATTRIBUTE_2, /* 1862 */
    XML_SCHEMAV_CVC_ATTRIBUTE_3, /* 1863 */
    XML_SCHEMAV_CVC_ATTRIBUTE_4, /* 1864 */
    XML_SCHEMAV_CVC_COMPLEX_TYPE_3_1, /* 1865 */
    XML_SCHEMAV_CVC_COMPLEX_TYPE_3_2_1, /* 1866 */
    XML_SCHEMAV_CVC_COMPLEX_TYPE_3_2_2, /* 1867 */
    XML_SCHEMAV_CVC_COMPLEX_TYPE_4, /* 1868 */
    XML_SCHEMAV_CVC_COMPLEX_TYPE_5_1, /* 1869 */
    XML_SCHEMAV_CVC_COMPLEX_TYPE_5_2, /* 1870 */
    XML_SCHEMAV_ELEMENT_CONTENT, /* 1871 */
    XML_SCHEMAV_DOCUMENT_ELEMENT_MISSING, /* 1872 */
    XML_SCHEMAV_CVC_COMPLEX_TYPE_1, /* 1873 */
    XML_SCHEMAV_CVC_AU, /* 1874 */
    XML_SCHEMAV_CVC_TYPE_1, /* 1875 */
    XML_SCHEMAV_CVC_TYPE_2, /* 1876 */
    XML_SCHEMAV_CVC_IDC, /* 1877 */
    XML_SCHEMAV_CVC_WILDCARD, /* 1878 */
    XML_SCHEMAV_MISC, /* 1879 */
    XML_XPTR_UNKNOWN_SCHEME = 1900, 
    XML_XPTR_CHILDSEQ_START, /* 1901 */
    XML_XPTR_EVAL_FAILED, /* 1902 */
    XML_XPTR_EXTRA_OBJECTS, /* 1903 */
    XML_C14N_CREATE_CTXT = 1950,
    XML_C14N_REQUIRES_UTF8, /* 1951 */
    XML_C14N_CREATE_STACK, /* 1952 */
    XML_C14N_INVALID_NODE, /* 1953 */
    XML_C14N_UNKNOW_NODE, /* 1954 */
    XML_C14N_RELATIVE_NAMESPACE, /* 1955 */
    XML_FTP_PASV_ANSWER = 2000,
    XML_FTP_EPSV_ANSWER, /* 2001 */
    XML_FTP_ACCNT, /* 2002 */
    XML_FTP_URL_SYNTAX, /* 2003 */
    XML_HTTP_URL_SYNTAX = 2020,
    XML_HTTP_USE_IP, /* 2021 */
    XML_HTTP_UNKNOWN_HOST, /* 2022 */
    XML_SCHEMAP_SRC_SIMPLE_TYPE_1 = 3000,
    XML_SCHEMAP_SRC_SIMPLE_TYPE_2, /* 3001 */
    XML_SCHEMAP_SRC_SIMPLE_TYPE_3, /* 3002 */
    XML_SCHEMAP_SRC_SIMPLE_TYPE_4, /* 3003 */
    XML_SCHEMAP_SRC_RESOLVE, /* 3004 */ 
    XML_SCHEMAP_SRC_RESTRICTION_BASE_OR_SIMPLETYPE, /* 3005 */
    XML_SCHEMAP_SRC_LIST_ITEMTYPE_OR_SIMPLETYPE, /* 3006 */
    XML_SCHEMAP_SRC_UNION_MEMBERTYPES_OR_SIMPLETYPES, /* 3007 */
    XML_SCHEMAP_ST_PROPS_CORRECT_1, /* 3008 */
    XML_SCHEMAP_ST_PROPS_CORRECT_2, /* 3009 */
    XML_SCHEMAP_ST_PROPS_CORRECT_3, /* 3010 */     
    XML_SCHEMAP_COS_ST_RESTRICTS_1_1, /* 3011 */
    XML_SCHEMAP_COS_ST_RESTRICTS_1_2, /* 3012 */    
    XML_SCHEMAP_COS_ST_RESTRICTS_1_3_1, /* 3013 */
    XML_SCHEMAP_COS_ST_RESTRICTS_1_3_2, /* 3014 */
    XML_SCHEMAP_COS_ST_RESTRICTS_2_1, /* 3015 */
    XML_SCHEMAP_COS_ST_RESTRICTS_2_3_1_1, /* 3016 */
    XML_SCHEMAP_COS_ST_RESTRICTS_2_3_1_2, /* 3017 */
    XML_SCHEMAP_COS_ST_RESTRICTS_2_3_2_1, /* 3018 */
    XML_SCHEMAP_COS_ST_RESTRICTS_2_3_2_2, /* 3019 */
    XML_SCHEMAP_COS_ST_RESTRICTS_2_3_2_3, /* 3020 */
    XML_SCHEMAP_COS_ST_RESTRICTS_2_3_2_4, /* 3021 */
    XML_SCHEMAP_COS_ST_RESTRICTS_2_3_2_5, /* 3022 */
    XML_SCHEMAP_COS_ST_RESTRICTS_3_1, /* 3023 */
    XML_SCHEMAP_COS_ST_RESTRICTS_3_3_1, /* 3024 */
    XML_SCHEMAP_COS_ST_RESTRICTS_3_3_1_2, /* 3025 */
    XML_SCHEMAP_COS_ST_RESTRICTS_3_3_2_2, /* 3026 */
    XML_SCHEMAP_COS_ST_RESTRICTS_3_3_2_1, /* 3027 */
    XML_SCHEMAP_COS_ST_RESTRICTS_3_3_2_3, /* 3028 */
    XML_SCHEMAP_COS_ST_RESTRICTS_3_3_2_4, /* 3029 */
    XML_SCHEMAP_COS_ST_RESTRICTS_3_3_2_5, /* 3030 */
    XML_SCHEMAP_COS_ST_DERIVED_OK_2_1, /* 3031 */ 
    XML_SCHEMAP_COS_ST_DERIVED_OK_2_2, /* 3032 */
    XML_SCHEMAP_S4S_ELEM_NOT_ALLOWED, /* 3033 */
    XML_SCHEMAP_S4S_ELEM_MISSING, /* 3034 */
    XML_SCHEMAP_S4S_ATTR_NOT_ALLOWED, /* 3035 */
    XML_SCHEMAP_S4S_ATTR_MISSING, /* 3036 */
    XML_SCHEMAP_S4S_ATTR_INVALID_VALUE, /* 3037 */
    XML_SCHEMAP_SRC_ELEMENT_1, /* 3038 */
    XML_SCHEMAP_SRC_ELEMENT_2_1, /* 3039 */
    XML_SCHEMAP_SRC_ELEMENT_2_2, /* 3040 */
    XML_SCHEMAP_SRC_ELEMENT_3, /* 3041 */
    XML_SCHEMAP_P_PROPS_CORRECT_1, /* 3042 */
    XML_SCHEMAP_P_PROPS_CORRECT_2_1, /* 3043 */
    XML_SCHEMAP_P_PROPS_CORRECT_2_2, /* 3044 */
    XML_SCHEMAP_E_PROPS_CORRECT_2, /* 3045 */
    XML_SCHEMAP_E_PROPS_CORRECT_3, /* 3046 */
    XML_SCHEMAP_E_PROPS_CORRECT_4, /* 3047 */
    XML_SCHEMAP_E_PROPS_CORRECT_5, /* 3048 */
    XML_SCHEMAP_E_PROPS_CORRECT_6, /* 3049 */
    XML_SCHEMAP_SRC_INCLUDE, /* 3050 */    
    XML_SCHEMAP_SRC_ATTRIBUTE_1, /* 3051 */
    XML_SCHEMAP_SRC_ATTRIBUTE_2, /* 3052 */
    XML_SCHEMAP_SRC_ATTRIBUTE_3_1, /* 3053 */
    XML_SCHEMAP_SRC_ATTRIBUTE_3_2, /* 3054 */
    XML_SCHEMAP_SRC_ATTRIBUTE_4, /* 3055 */
    XML_SCHEMAP_NO_XMLNS, /* 3056 */
    XML_SCHEMAP_NO_XSI, /* 3057 */      
    XML_SCHEMAP_COS_VALID_DEFAULT_1, /* 3058 */
    XML_SCHEMAP_COS_VALID_DEFAULT_2_1, /* 3059 */
    XML_SCHEMAP_COS_VALID_DEFAULT_2_2_1, /* 3060 */
    XML_SCHEMAP_COS_VALID_DEFAULT_2_2_2, /* 3061 */
    XML_SCHEMAP_CVC_SIMPLE_TYPE, /* 3062 */
    XML_SCHEMAP_COS_CT_EXTENDS_1_1, /* 3063 */
    XML_SCHEMAP_SRC_IMPORT_1_1, /* 3064 */
    XML_SCHEMAP_SRC_IMPORT_1_2, /* 3065 */
    XML_SCHEMAP_SRC_IMPORT_2, /* 3066 */
    XML_SCHEMAP_SRC_IMPORT_2_1, /* 3067 */
    XML_SCHEMAP_SRC_IMPORT_2_2, /* 3068 */
    XML_SCHEMAP_INTERNAL, /* 3069 non-W3C */
    XML_SCHEMAP_NOT_DETERMINISTIC, /* 3070 non-W3C */
    XML_SCHEMAP_SRC_ATTRIBUTE_GROUP_1, /* 3071 */
    XML_SCHEMAP_SRC_ATTRIBUTE_GROUP_2, /* 3072 */
    XML_SCHEMAP_SRC_ATTRIBUTE_GROUP_3, /* 3073 */
    XML_SCHEMAP_MG_PROPS_CORRECT_1, /* 3074 */
    XML_SCHEMAP_MG_PROPS_CORRECT_2, /* 3075 */
    XML_SCHEMAP_SRC_CT_1, /* 3076 */
    XML_SCHEMAP_DERIVATION_OK_RESTRICTION_2_1_3, /* 3077 */
    XML_SCHEMAP_AU_PROPS_CORRECT_2, /* 3078 */
    XML_SCHEMAP_A_PROPS_CORRECT_2, /* 3079 */
    XML_SCHEMAP_C_PROPS_CORRECT, /* 3080 */
    XML_SCHEMAP_SRC_REDEFINE, /* 3081 */
    XML_SCHEMAP_SRC_IMPORT, /* 3082 */
    XML_SCHEMAP_WARN_SKIP_SCHEMA, /* 3083 */
    XML_SCHEMAP_WARN_UNLOCATED_SCHEMA, /* 3084 */
    XML_SCHEMAP_WARN_ATTR_REDECL_PROH, /* 3085 */
    XML_SCHEMAP_WARN_ATTR_POINTLESS_PROH, /* 3085 */
    XML_SCHEMAP_AG_PROPS_CORRECT, /* 3086 */
    XML_SCHEMAP_COS_CT_EXTENDS_1_2, /* 3087 */
    XML_SCHEMAP_AU_PROPS_CORRECT, /* 3088 */
    XML_SCHEMAP_A_PROPS_CORRECT_3, /* 3089 */
    XML_SCHEMAP_COS_ALL_LIMITED, /* 3090 */
    XML_MODULE_OPEN = 4900, /* 4900 */
    XML_MODULE_CLOSE, /* 4901 */
    XML_CHECK_FOUND_ELEMENT = 5000,
    XML_CHECK_FOUND_ATTRIBUTE, /* 5001 */
    XML_CHECK_FOUND_TEXT, /* 5002 */
    XML_CHECK_FOUND_CDATA, /* 5003 */
    XML_CHECK_FOUND_ENTITYREF, /* 5004 */
    XML_CHECK_FOUND_ENTITY, /* 5005 */
    XML_CHECK_FOUND_PI, /* 5006 */
    XML_CHECK_FOUND_COMMENT, /* 5007 */
    XML_CHECK_FOUND_DOCTYPE, /* 5008 */
    XML_CHECK_FOUND_FRAGMENT, /* 5009 */
    XML_CHECK_FOUND_NOTATION, /* 5010 */
    XML_CHECK_UNKNOWN_NODE, /* 5011 */
    XML_CHECK_ENTITY_TYPE, /* 5012 */
    XML_CHECK_NO_PARENT, /* 5013 */
    XML_CHECK_NO_DOC, /* 5014 */
    XML_CHECK_NO_NAME, /* 5015 */
    XML_CHECK_NO_ELEM, /* 5016 */
    XML_CHECK_WRONG_DOC, /* 5017 */
    XML_CHECK_NO_PREV, /* 5018 */
    XML_CHECK_WRONG_PREV, /* 5019 */
    XML_CHECK_NO_NEXT, /* 5020 */
    XML_CHECK_WRONG_NEXT, /* 5021 */
    XML_CHECK_NOT_DTD, /* 5022 */
    XML_CHECK_NOT_ATTR, /* 5023 */
    XML_CHECK_NOT_ATTR_DECL, /* 5024 */
    XML_CHECK_NOT_ELEM_DECL, /* 5025 */
    XML_CHECK_NOT_ENTITY_DECL, /* 5026 */
    XML_CHECK_NOT_NS_DECL, /* 5027 */
    XML_CHECK_NO_HREF, /* 5028 */
    XML_CHECK_WRONG_PARENT,/* 5029 */
    XML_CHECK_NS_SCOPE, /* 5030 */
    XML_CHECK_NS_ANCESTOR, /* 5031 */
    XML_CHECK_NOT_UTF8, /* 5032 */
    XML_CHECK_NO_DICT, /* 5033 */
    XML_CHECK_NOT_NCNAME, /* 5034 */
    XML_CHECK_OUTSIDE_DICT, /* 5035 */
    XML_CHECK_WRONG_NAME, /* 5036 */
    XML_CHECK_NAME_NOT_NULL, /* 5037 */
    XML_I18N_NO_NAME = 6000,
    XML_I18N_NO_HANDLER, /* 6001 */
    XML_I18N_EXCESS_HANDLER, /* 6002 */
    XML_I18N_CONV_FAILED, /* 6003 */
    XML_I18N_NO_OUTPUT /* 6004 */
} hw_xmlParserErrors;


/////////////////////////////////////////////////////////////////////////////////////




/**
 * hw_xmlCharEncodingInputFunc:
 * @out:  a pointer to an array of bytes to store the UTF-8 result
 * @outlen:  the length of @out
 * @in:  a pointer to an array of chars in the original encoding
 * @inlen:  the length of @in
 *
 * Take a block of chars in the original encoding and try to convert
 * it to an UTF-8 block of chars out.
 *
 * Returns the number of bytes written, -1 if lack of space, or -2
 *     if the transcoding failed.
 * The value of @inlen after return is the number of octets consumed
 *     if the return value is positive, else unpredictiable.
 * The value of @outlen after return is the number of octets consumed.
 */
typedef int (* hw_xmlCharEncodingInputFunc)(unsigned char *out, int *outlen,
                                         const unsigned char *in, int *inlen);


/**
 * hw_xmlCharEncodingOutputFunc:
 * @out:  a pointer to an array of bytes to store the result
 * @outlen:  the length of @out
 * @in:  a pointer to an array of UTF-8 chars
 * @inlen:  the length of @in
 *
 * Take a block of UTF-8 chars in and try to convert it to another
 * encoding.
 * Note: a first call designed to produce heading info is called with
 * in = NULL. If stateful this should also initialize the encoder state.
 *
 * Returns the number of bytes written, -1 if lack of space, or -2
 *     if the transcoding failed.
 * The value of @inlen after return is the number of octets consumed
 *     if the return value is positive, else unpredictiable.
 * The value of @outlen after return is the number of octets produced.
 */
typedef int (* hw_xmlCharEncodingOutputFunc)(unsigned char *out, int *outlen,
                                          const unsigned char *in, int *inlen);


/*
 * Block defining the handlers for non UTF-8 encodings.
 * If iconv is supported, there are two extra fields.
 */

typedef struct hw__xmlCharEncodingHandler hw_xmlCharEncodingHandler;
typedef hw_xmlCharEncodingHandler *hw_xmlCharEncodingHandlerPtr;


struct hw__xmlCharEncodingHandler {
    char                       *name;
    hw_xmlCharEncodingInputFunc   input;
    hw_xmlCharEncodingOutputFunc  output;
};


/**
 * hw_xmlChar:
 *
 * This is a basic byte in an UTF-8 encoded string.
 * It's unsigned allowing to pinpoint case where char * are assigned
 * to hw_xmlChar * (possibly making serialization back impossible).
 */
typedef unsigned char hw_xmlChar;


/*
 * Some of the basic types pointer to structures:
 */
/* xmlIO.h */
typedef struct hw__xmlParserInputBuffer hw_xmlParserInputBuffer;
typedef hw_xmlParserInputBuffer *hw_xmlParserInputBufferPtr;

typedef struct _xmlOutputBuffer hw_xmlOutputBuffer;
typedef hw_xmlOutputBuffer *hw_xmlOutputBufferPtr;

/* parser.h */
typedef struct hw__xmlParserInput hw_xmlParserInput;
typedef hw_xmlParserInput *hw_xmlParserInputPtr;

typedef struct hw__xmlParserCtxt hw_xmlParserCtxt;
typedef hw_xmlParserCtxt *hw_xmlParserCtxtPtr;

typedef struct hw__xmlSAXLocator hw_xmlSAXLocator;
typedef hw_xmlSAXLocator *hw_xmlSAXLocatorPtr;

typedef struct hw__xmlSAXHandler hw_xmlSAXHandler;
typedef hw_xmlSAXHandler *hw_xmlSAXHandlerPtr;

/* entities.h */
typedef struct hw__xmlEntity hw_xmlEntity;
typedef hw_xmlEntity *hw_xmlEntityPtr;




/**
 * hw_xmlBuffer:
 *
 * A buffer structure.
 */
typedef struct hw__xmlBuffer hw_xmlBuffer;
typedef hw_xmlBuffer *hw_xmlBufferPtr;
struct hw__xmlBuffer {
    hw_xmlChar *content;		/* The buffer content UTF8 */
    unsigned int use;		/* The buffer size used */
    unsigned int size;		/* The buffer size */
    hw_xmlBufferAllocationScheme alloc; /* The realloc method */
};

/**
 * hw_xmlNotation:
 *
 * A DTD Notation definition.
 */

typedef struct hw__xmlNotation hw_xmlNotation;
typedef hw_xmlNotation *hw_xmlNotationPtr;
struct hw__xmlNotation {
    const hw_xmlChar               *name;	        /* Notation name */
    const hw_xmlChar               *PublicID;	/* Public identifier, if any */
    const hw_xmlChar               *SystemID;	/* System identifier, if any */
};




/**
 * hw_xmlEnumeration:
 *
 * List structure used when there is an enumeration in DTDs.
 */

typedef struct hw__xmlEnumeration hw_xmlEnumeration;
typedef hw_xmlEnumeration *hw_xmlEnumerationPtr;
struct hw__xmlEnumeration {
    struct hw__xmlEnumeration    *next;	/* next one */
    const hw_xmlChar            *name;	/* Enumeration name */
};

/**
 * hw_xmlAttribute:
 *
 * An Attribute declaration in a DTD.
 */

typedef struct hw__xmlAttribute hw_xmlAttribute;
typedef hw_xmlAttribute *hw_xmlAttributePtr;
struct hw__xmlAttribute {
    void           *_private;	        /* application data */
    hw_xmlElementType          type;       /* XML_ATTRIBUTE_DECL, must be second ! */
    const hw_xmlChar          *name;	/* Attribute name */
    struct hw__xmlNode    *children;	/* NULL */
    struct hw__xmlNode        *last;	/* NULL */
    struct hw__xmlDtd       *parent;	/* -> DTD */
    struct hw__xmlNode        *next;	/* next sibling link  */
    struct hw__xmlNode        *prev;	/* previous sibling link  */
    struct hw__xmlDoc          *doc;       /* the containing document */

    struct hw__xmlAttribute  *nexth;	/* next in hash table */
    hw_xmlAttributeType       atype;	/* The attribute type */
    hw_xmlAttributeDefault      def;	/* the default */
    const hw_xmlChar  *defaultValue;	/* or the default value */
    hw_xmlEnumerationPtr       tree;       /* or the enumeration tree if any */
    const hw_xmlChar        *prefix;	/* the namespace prefix if any */
    const hw_xmlChar          *elem;	/* Element holding the attribute */
};

/**
 * hw_xmlElementContent:
 *
 * An XML Element content as stored after parsing an element definition
 * in a DTD.
 */

typedef struct hw__xmlElementContent hw_xmlElementContent;
typedef hw_xmlElementContent *hw_xmlElementContentPtr;
struct hw__xmlElementContent {
    hw_xmlElementContentType     type;	/* PCDATA, ELEMENT, SEQ or OR */
    hw_xmlElementContentOccur    ocur;	/* ONCE, OPT, MULT or PLUS */
    const hw_xmlChar             *name;	/* Element name */
    struct hw__xmlElementContent *c1;	/* first child */
    struct hw__xmlElementContent *c2;	/* second child */
    struct hw__xmlElementContent *parent;	/* parent */
    const hw_xmlChar             *prefix;	/* Namespace prefix */
};

/**
 * hw_xmlElement:
 *
 * An XML Element declaration from a DTD.
 */

typedef struct hw__xmlElement hw_xmlElement;
typedef hw_xmlElement *hw_xmlElementPtr;
struct hw__xmlElement {
    void           *_private;	        /* application data */
    hw_xmlElementType          type;       /* XML_ELEMENT_DECL, must be second ! */
    const hw_xmlChar          *name;	/* Element name */
    struct hw__xmlNode    *children;	/* NULL */
    struct hw__xmlNode        *last;	/* NULL */
    struct hw__xmlDtd       *parent;	/* -> DTD */
    struct hw__xmlNode        *next;	/* next sibling link  */
    struct hw__xmlNode        *prev;	/* previous sibling link  */
    struct hw__xmlDoc          *doc;       /* the containing document */

    hw_xmlElementTypeVal      etype;	/* The type */
    hw_xmlElementContentPtr content;	/* the allowed element content */
    hw_xmlAttributePtr   attributes;	/* List of the declared attributes */
    const hw_xmlChar        *prefix;	/* the namespace prefix if any */

    void	      *contModel;

};


/**
 *
 * A namespace declaration node.
 */

typedef hw_xmlElementType hw_xmlNsType;

/**
 * hw_xmlNs:
 *
 * An XML namespace.
 * Note that prefix == NULL is valid, it defines the default namespace
 * within the subtree (until overridden).
 *
 * hw_xmlNsType is unified with hw_xmlElementType.
 */

typedef struct hw__xmlNs hw_xmlNs;
typedef hw_xmlNs *hw_xmlNsPtr;
struct hw__xmlNs {
    struct hw__xmlNs  *next;	/* next Ns link for this node  */
    hw_xmlNsType      type;	/* global or local */
    const hw_xmlChar *href;	/* URL for the namespace */
    const hw_xmlChar *prefix;	/* prefix for the namespace */
    void           *_private;   /* application data */
};

/**
 * hw_xmlDtd:
 *
 * An XML DTD, as defined by <!DOCTYPE ... There is actually one for
 * the internal subset and for the external subset.
 */
typedef struct hw__xmlDtd hw_xmlDtd;
typedef hw_xmlDtd *hw_xmlDtdPtr;
struct hw__xmlDtd {
    void           *_private;	/* application data */
    hw_xmlElementType  type;       /* XML_DTD_NODE, must be second ! */
    const hw_xmlChar *name;	/* Name of the DTD */
    struct hw__xmlNode *children;	/* the value of the property link */
    struct hw__xmlNode *last;	/* last child link */
    struct hw__xmlDoc  *parent;	/* child->parent link */
    struct hw__xmlNode *next;	/* next sibling link  */
    struct hw__xmlNode *prev;	/* previous sibling link  */
    struct hw__xmlDoc  *doc;	/* the containing document */

    /* End of common part */
    void          *notations;   /* Hash table for notations if any */
    void          *elements;    /* Hash table for elements if any */
    void          *attributes;  /* Hash table for attributes if any */
    void          *entities;    /* Hash table for entities if any */
    const hw_xmlChar *ExternalID;	/* External identifier for PUBLIC DTD */
    const hw_xmlChar *SystemID;	/* URI for a SYSTEM or PUBLIC DTD */
    void          *pentities;   /* Hash table for param entities if any */
};

/**
 * hw_xmlAttr:
 *
 * An attribute on an XML node.
 */
typedef struct hw__xmlAttr hw_xmlAttr;
typedef hw_xmlAttr *hw_xmlAttrPtr;
struct hw__xmlAttr {
    void           *_private;	/* application data */
    hw_xmlElementType   type;      /* XML_ATTRIBUTE_NODE, must be second ! */
    const hw_xmlChar   *name;      /* the name of the property */
    struct hw__xmlNode *children;	/* the value of the property */
    struct hw__xmlNode *last;	/* NULL */
    struct hw__xmlNode *parent;	/* child->parent link */
    struct hw__xmlAttr *next;	/* next sibling link  */
    struct hw__xmlAttr *prev;	/* previous sibling link  */
    struct hw__xmlDoc  *doc;	/* the containing document */
    hw_xmlNs           *ns;        /* pointer to the associated namespace */
    hw_xmlAttributeType atype;     /* the attribute type if validating */
    void            *psvi;	/* for type/PSVI informations */
};

/**
 * hw_xmlID:
 *
 * An XML ID instance.
 */

typedef struct hw__xmlID hw_xmlID;
typedef hw_xmlID *hw_xmlIDPtr;
struct hw__xmlID {
    struct hw__xmlID    *next;	/* next ID */
    const hw_xmlChar    *value;	/* The ID name */
    hw_xmlAttrPtr        attr;	/* The attribute holding it */
    const hw_xmlChar    *name;	/* The attribute if attr is not available */
    int               lineno;	/* The line number if attr is not available */
    struct hw__xmlDoc   *doc;	/* The document holding the ID */
};

/**
 * hw_xmlRef:
 *
 * An XML IDREF instance.
 */

typedef struct hw__xmlRef hw_xmlRef;
typedef hw_xmlRef *hw_xmlRefPtr;
struct hw__xmlRef {
    struct hw__xmlRef    *next;	/* next Ref */
    const hw_xmlChar     *value;	/* The Ref name */
    hw_xmlAttrPtr        attr;	/* The attribute holding it */
    const hw_xmlChar    *name;	/* The attribute if attr is not available */
    int               lineno;	/* The line number if attr is not available */
};

/**
 * hw_xmlNode:
 *
 * A node in an XML tree.
 */
typedef struct hw__xmlNode hw_xmlNode;
typedef hw_xmlNode *hw_xmlNodePtr;
struct hw__xmlNode {
    void           *_private;	/* application data */
    hw_xmlElementType   type;	/* type number, must be second ! */
    const hw_xmlChar   *name;      /* the name of the node, or the entity */
    struct hw__xmlNode *children;	/* parent->childs link */
    struct hw__xmlNode *last;	/* last child link */
    struct hw__xmlNode *parent;	/* child->parent link */
    struct hw__xmlNode *next;	/* next sibling link  */
    struct hw__xmlNode *prev;	/* previous sibling link  */
    struct hw__xmlDoc  *doc;	/* the containing document */

    /* End of common part */
    hw_xmlNs           *ns;        /* pointer to the associated namespace */
    hw_xmlChar         *content;   /* the content */
    struct hw__xmlAttr *properties;/* properties list */
    hw_xmlNs           *nsDef;     /* namespace definitions on this node */
    void            *psvi;	/* for type/PSVI informations */
    unsigned short   line;	/* line number */
    unsigned short   extra;	/* extra data for XPath/XSLT */
  //  void * XML_Element_ptr;
};

/**
 * hw_xmlDoc:
 *
 * An XML document.
 */
typedef struct hw__xmlDoc hw_xmlDoc;
typedef hw_xmlDoc *hw_xmlDocPtr;
struct hw__xmlDoc {
    void           *_private;	/* application data */
    hw_xmlElementType  type;       /* XML_DOCUMENT_NODE, must be second ! */
    char           *name;	/* name/filename/URI of the document */
    struct hw__xmlNode *children;	/* the document tree */
    struct hw__xmlNode *last;	/* last child link */
    struct hw__xmlNode *parent;	/* child->parent link */
    struct hw__xmlNode *next;	/* next sibling link  */
    struct hw__xmlNode *prev;	/* previous sibling link  */
    struct hw__xmlDoc  *doc;	/* autoreference to itself */

    /* End of common part */
    int             compression;/* level of zlib compression */
    int             standalone; /* standalone document (no external refs) */
    struct hw__xmlDtd  *intSubset;	/* the document internal subset */
    struct hw__xmlDtd  *extSubset;	/* the document external subset */
    struct hw__xmlNs   *oldNs;	/* Global namespace, the old way */
    const hw_xmlChar  *version;	/* the XML version string */
    const hw_xmlChar  *encoding;   /* external initial encoding, if any */
    void           *ids;        /* Hash table for ID attributes if any */
    void           *refs;       /* Hash table for IDREFs attributes if any */
    const hw_xmlChar  *URL;	/* The URI for that document */
    int             charset;    /* encoding of the in-memory content
				   actually an hw_xmlCharEncoding */
    struct _xmlDict *dict;      /* dict used to allocate names or NULL */
    void           *psvi;	/* for type/PSVI informations */
};


/*
 * The dictionnary.
 */
typedef struct _xmlDict hw_xmlDict;
typedef hw_xmlDict *hw_xmlDictPtr;





/**
 * hw_xmlURI:
 *
 * A parsed URI reference. This is a struct containing the various fields
 * as described in RFC 2396 but separated for further processing.
 */
typedef struct hw__xmlURI hw_xmlURI;
typedef hw_xmlURI *hw_xmlURIPtr;
struct hw__xmlURI {
    char *scheme;	/* the URI scheme */
    char *opaque;	/* opaque part */
    char *authority;	/* the authority part */
    char *server;	/* the server part */
    char *user;		/* the user part */
    int port;		/* the port number */
    char *path;		/* the path string */
    char *query;	/* the query string */
    char *fragment;	/* the fragment identifier */
    int  cleanup;	/* parsing potentially unclean URI */
};




typedef struct _xmlLink hw_xmlLink;
typedef hw_xmlLink *hw_xmlLinkPtr;

typedef struct _xmlList hw_xmlList;
typedef hw_xmlList *hw_xmlListPtr;

/**
 * hw_xmlListDeallocator:
 * @lk:  the data to deallocate
 *
 * Callback function used to free data from a list.
 */
typedef void (*hw_xmlListDeallocator) (hw_xmlLinkPtr lk);
/**
 * hw_xmlListDataCompare:
 * @data0: the first data
 * @data1: the second data
 *
 * Callback function used to compare 2 data.
 *
 * Returns 0 is equality, -1 or 1 otherwise depending on the ordering.
 */
typedef int  (*hw_xmlListDataCompare) (const void *data0, const void *data1);



/*
 * Validation state added for non-determinist content model.
 */
typedef struct _xmlValidState hw_xmlValidState;

/**
 * hw_xmlValidityErrorFunc:
 * @ctx:  usually an hw_xmlValidCtxtPtr to a validity error context,
 *        but comes from ctxt->userData (which normally contains such
 *        a pointer); ctxt->userData can be changed by the user.
 * @msg:  the string to format *printf like vararg
 * @...:  remaining arguments to the format
 *
 * Callback called when a validity error is found. This is a message
 * oriented function similar to an *printf function.
 */
typedef void (hw_XMLCDECL *hw_xmlValidityErrorFunc) (void *ctx,
			     const char *msg,
			     ...);

/**
 * hw_xmlValidityWarningFunc:
 * @ctx:  usually an hw_xmlValidCtxtPtr to a validity error context,
 *        but comes from ctxt->userData (which normally contains such
 *        a pointer); ctxt->userData can be changed by the user.
 * @msg:  the string to format *printf like vararg
 * @...:  remaining arguments to the format
 *
 * Callback called when a validity warning is found. This is a message
 * oriented function similar to an *printf function.
 */
typedef void (hw_XMLCDECL *hw_xmlValidityWarningFunc) (void *ctx,
			       const char *msg,
			       ...);


/*
 * hw_xmlValidCtxt:
 * An hw_xmlValidCtxt is used for error reporting when validating.
 */
typedef struct hw__xmlValidCtxt hw_xmlValidCtxt;
typedef hw_xmlValidCtxt *hw_xmlValidCtxtPtr;
struct hw__xmlValidCtxt {
    void *userData;			/* user specific data block */
    hw_xmlValidityErrorFunc error;		/* the callback in case of errors */
    hw_xmlValidityWarningFunc warning;	/* the callback in case of warning */

    /* Node analysis stack used when validating within entities */
    hw_xmlNodePtr         node;          /* Current parsed Node */
    int                nodeNr;        /* Depth of the parsing stack */
    int                nodeMax;       /* Max depth of the parsing stack */
    hw_xmlNodePtr        *nodeTab;       /* array of nodes */

    unsigned int     finishDtd;       /* finished validating the Dtd ? */
    hw_xmlDocPtr              doc;       /* the document */
    int                  valid;       /* temporary validity check result */

    /* state state used for non-determinist content validation */
    hw_xmlValidState     *vstate;        /* current state */
    int                vstateNr;      /* Depth of the validation stack */
    int                vstateMax;     /* Max depth of the validation stack */
    hw_xmlValidState     *vstateTab;     /* array of validation states */


    void                     *am;
    void                  *state;

};

/*
 * ALL notation declarations are stored in a table.
 * There is one table per DTD.
 */

typedef struct _xmlHashTable hw_xmlNotationTable;
typedef hw_xmlNotationTable *hw_xmlNotationTablePtr;

/*
 * ALL element declarations are stored in a table.
 * There is one table per DTD.
 */

typedef struct _xmlHashTable hw_xmlElementTable;
typedef hw_xmlElementTable *hw_xmlElementTablePtr;

/*
 * ALL attribute declarations are stored in a table.
 * There is one table per DTD.
 */

typedef struct _xmlHashTable hw_xmlAttributeTable;
typedef hw_xmlAttributeTable *hw_xmlAttributeTablePtr;

/*
 * ALL IDs attributes are stored in a table.
 * There is one table per document.
 */

typedef struct _xmlHashTable hw_xmlIDTable;
typedef hw_xmlIDTable *hw_xmlIDTablePtr;

/*
 * ALL Refs attributes are stored in a table.
 * There is one table per document.
 */

typedef struct _xmlHashTable hw_xmlRefTable;
typedef hw_xmlRefTable *hw_xmlRefTablePtr;





/*
 * An unit of storage for an entity, contains the string, the value
 * and the linkind data needed for the linking in the hash table.
 */

struct hw__xmlEntity {
    void           *_private;	        /* application data */
    hw_xmlElementType          type;       /* XML_ENTITY_DECL, must be second ! */
    const hw_xmlChar          *name;	/* Entity name */
    struct hw__xmlNode    *children;	/* First child link */
    struct hw__xmlNode        *last;	/* Last child link */
    struct hw__xmlDtd       *parent;	/* -> DTD */
    struct hw__xmlNode        *next;	/* next sibling link  */
    struct hw__xmlNode        *prev;	/* previous sibling link  */
    struct hw__xmlDoc          *doc;       /* the containing document */

    hw_xmlChar                *orig;	/* content without ref substitution */
    hw_xmlChar             *content;	/* content or ndata if unparsed */
    int                   length;	/* the content length */
    hw_xmlEntityType          etype;	/* The entity type */
    const hw_xmlChar    *ExternalID;	/* External identifier for PUBLIC */
    const hw_xmlChar      *SystemID;	/* URI for a SYSTEM or PUBLIC Entity */

    struct hw__xmlEntity     *nexte;	/* unused */
    const hw_xmlChar           *URI;	/* the full URI as computed */
    int                    owner;	/* does the entity own the childrens */
};

/*
 * All entities are stored in an hash table.
 * There is 2 separate hash tables for global and parameter entities.
 */

typedef struct _xmlHashTable hw_xmlEntitiesTable;
typedef hw_xmlEntitiesTable *hw_xmlEntitiesTablePtr;


/**
 * hw_xmlError:
 *
 * An XML Error instance.
 */

typedef struct hw__xmlError hw_xmlError;
typedef hw_xmlError *hw_xmlErrorPtr;
struct hw__xmlError {
    int		domain;	/* What part of the library raised this error */
    int		code;	/* The error code, e.g. an hw_xmlParserError */
    char       *message;/* human-readable informative error message */
    hw_xmlErrorLevel level;/* how consequent is the error */
    char       *file;	/* the filename */
    int		line;	/* the line number if available */
    char       *str1;	/* extra string information */
    char       *str2;	/* extra string information */
    char       *str3;	/* extra string information */
    int		int1;	/* extra number information */
    int		int2;	/* column number of the error or 0 if N/A (todo: rename this field when we would break ABI) */
    void       *ctxt;   /* the parser context if available */
    void       *node;   /* the node in the tree */
};


/**
 * hw_xmlGenericErrorFunc:
 * @ctx:  a parsing context
 * @msg:  the message
 * @...:  the extra arguments of the varags to format the message
 *
 * Signature of the function to use when there is an error and
 * no parsing or validity context available .
 */
typedef void (hw_XMLCDECL *hw_xmlGenericErrorFunc) (void *ctx,const char *msg, ...);
/**
 * hw_xmlStructuredErrorFunc:
 * @userData:  user provided data for the error callback
 * @error:  the error being raised.
 *
 * Signature of the function to use when there is an error and
 * the module handles the new error reporting mechanism.
 */
typedef void (hw_XMLCALL *hw_xmlStructuredErrorFunc) (void *userData, hw_xmlErrorPtr error);



hw_XMLPUBVAR const unsigned char hw_xmlIsPubidChar_tab[256];

/*
 * The hash table.
 */
typedef struct _xmlHashTable hw_xmlHashTable;
typedef hw_xmlHashTable *hw_xmlHashTablePtr;





/**
 * hw_xmlParserMaxDepth:
 *
 * arbitrary depth limit for the XML documents that we allow to 
 * process. This is not a limitation of the parser but a safety 
 * boundary feature.
 */
hw_XMLPUBVAR unsigned int hw_xmlParserMaxDepth;

/**
 * Global variables used for predefined strings.
 */
hw_XMLPUBVAR const hw_xmlChar hw_xmlStringText[];
hw_XMLPUBVAR const hw_xmlChar hw_xmlStringTextNoenc[];
hw_XMLPUBVAR const hw_xmlChar hw_xmlStringComment[];





///////////////////////////////////////////


/*
 * Externally global symbols which need to be protected for backwards
 * compatibility support.
 */





hw_XMLPUBVAR hw_xmlError hw_xmlLastError;

/*
 * Everything starting from the line below is
 * Automatically generated by build_glob.py.
 * Do not modify the previous line.
 */

hw_XMLPUBVAR hw_xmlBufferAllocationScheme hw_xmlBufferAllocScheme;
hw_XMLPUBVAR int hw_xmlDefaultBufferSize;
hw_XMLPUBVAR hw_xmlGenericErrorFunc hw_xmlGenericError;
hw_XMLPUBVAR hw_xmlStructuredErrorFunc hw_xmlStructuredError;
hw_XMLPUBVAR void * hw_xmlGenericErrorContext;





/*
 * Those are the functions and datatypes for the parser input
 * I/O structures.
 */

/**
 * hw_xmlInputMatchCallback:
 * @filename: the filename or URI
 *
 * Callback used in the I/O Input API to detect if the current handler 
 * can provide input fonctionnalities for this resource.
 *
 * Returns 1 if yes and 0 if another Input module should be used
 */
typedef int (hw_XMLCALL *hw_xmlInputMatchCallback) (char const *filename);
/**
 * hw_xmlInputOpenCallback:
 * @filename: the filename or URI
 *
 * Callback used in the I/O Input API to open the resource
 *
 * Returns an Input context or NULL in case or error
 */
typedef void * (hw_XMLCALL *hw_xmlInputOpenCallback) (char const *filename);
/**
 * hw_xmlInputReadCallback:
 * @context:  an Input context
 * @buffer:  the buffer to store data read
 * @len:  the length of the buffer in bytes
 *
 * Callback used in the I/O Input API to read the resource
 *
 * Returns the number of bytes read or -1 in case of error
 */
typedef int (hw_XMLCALL *hw_xmlInputReadCallback) (void * context, char * buffer, int len);
/**
 * hw_xmlInputCloseCallback:
 * @context:  an Input context
 *
 * Callback used in the I/O Input API to close the resource
 *
 * Returns 0 or -1 in case of error
 */
typedef int (hw_XMLCALL *hw_xmlInputCloseCallback) (void * context);








struct hw__xmlParserInputBuffer {
    void*                  context;
    hw_xmlInputReadCallback   readcallback;
    hw_xmlInputCloseCallback  closecallback;
    
    hw_xmlCharEncodingHandlerPtr encoder; /* I18N conversions to UTF-8 */
    
    hw_xmlBufferPtr buffer;    /* Local buffer encoded in UTF-8 */
    hw_xmlBufferPtr raw;       /* if encoder != NULL buffer for raw input */
    int	compressed;	    /* -1=unknown, 0=not compressed, 1=compressed */
    int error;
    unsigned long rawconsumed;/* amount consumed from raw */
};
/*
 * External functions:
 */
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////






/*
 * Variables.
 */

/*
 * Some helper functions
 */


hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL	
		hw_xmlBuildQName		(const hw_xmlChar *ncname,
					 const hw_xmlChar *prefix,
					 hw_xmlChar *memory,
					 int len);
hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL	
		hw_xmlSplitQName2		(const hw_xmlChar *name,
					 hw_xmlChar **prefix);
hw_XMLPUBFUN const hw_xmlChar * hw_XMLCALL	
		hw_xmlSplitQName3		(const hw_xmlChar *name,
					 int *len);

/*
 * Handling Buffers.
 */

hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlSetBufferAllocationScheme(hw_xmlBufferAllocationScheme scheme);
hw_XMLPUBFUN hw_xmlBufferAllocationScheme hw_XMLCALL	 
		hw_xmlGetBufferAllocationScheme(void);

hw_XMLPUBFUN hw_xmlBufferPtr hw_XMLCALL	
		hw_xmlBufferCreate		(void);
hw_XMLPUBFUN hw_xmlBufferPtr hw_XMLCALL	
		hw_xmlBufferCreateSize	(size_t size);
hw_XMLPUBFUN hw_xmlBufferPtr hw_XMLCALL	
		hw_xmlBufferCreateStatic	(void *mem,
					 size_t size);
hw_XMLPUBFUN int hw_XMLCALL		
		hw_xmlBufferResize		(hw_xmlBufferPtr buf,
					 unsigned int size);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlBufferFree		(hw_xmlBufferPtr buf);
hw_XMLPUBFUN int hw_XMLCALL		
		hw_xmlBufferAdd		(hw_xmlBufferPtr buf,
					 const hw_xmlChar *str,
					 int len);
hw_XMLPUBFUN int hw_XMLCALL		
		hw_xmlBufferShrink		(hw_xmlBufferPtr buf,
					 unsigned int len);
hw_XMLPUBFUN int hw_XMLCALL		
		hw_xmlBufferGrow		(hw_xmlBufferPtr buf,
					 unsigned int len);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlBufferEmpty		(hw_xmlBufferPtr buf);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlBufferSetAllocationScheme(hw_xmlBufferPtr buf,
					 hw_xmlBufferAllocationScheme scheme);
hw_XMLPUBFUN int hw_XMLCALL		
		hw_xmlBufferLength		(const hw_xmlBufferPtr buf);

/*
 * Creating/freeing new structures.
 */
hw_XMLPUBFUN hw_xmlDtdPtr hw_XMLCALL	
		hw_xmlCreateIntSubset	(hw_xmlDocPtr doc,
					 const hw_xmlChar *name,
					 const hw_xmlChar *ExternalID,
					 const hw_xmlChar *SystemID);
hw_XMLPUBFUN hw_xmlDtdPtr hw_XMLCALL	
		hw_xmlNewDtd		(hw_xmlDocPtr doc,
					 const hw_xmlChar *name,
					 const hw_xmlChar *ExternalID,
					 const hw_xmlChar *SystemID);
hw_XMLPUBFUN hw_xmlDtdPtr hw_XMLCALL	
		hw_xmlGetIntSubset		(hw_xmlDocPtr doc);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlFreeDtd		(hw_xmlDtdPtr cur);
hw_XMLPUBFUN hw_xmlNsPtr hw_XMLCALL	
		hw_xmlNewNs		(hw_xmlNodePtr node,
					 const hw_xmlChar *href,
					 const hw_xmlChar *prefix);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlFreeNs		(hw_xmlNsPtr cur);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlFreeNsList		(hw_xmlNsPtr cur);
hw_XMLPUBFUN hw_xmlDocPtr hw_XMLCALL 	
		hw_xmlNewDoc		(const hw_xmlChar *version);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlFreeDoc		(hw_xmlDocPtr cur);
hw_XMLPUBFUN hw_xmlAttrPtr hw_XMLCALL	
		hw_xmlNewDocProp		(hw_xmlDocPtr doc,
					 const hw_xmlChar *name,
					 const hw_xmlChar *value);

hw_XMLPUBFUN hw_xmlAttrPtr hw_XMLCALL	
		hw_xmlNewNsProp		(hw_xmlNodePtr node,
					 hw_xmlNsPtr ns,
					 const hw_xmlChar *name,
					 const hw_xmlChar *value);
hw_XMLPUBFUN hw_xmlAttrPtr hw_XMLCALL	
		hw_xmlNewNsPropEatName	(hw_xmlNodePtr node,
					 hw_xmlNsPtr ns,
					 hw_xmlChar *name,
					 const hw_xmlChar *value);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlFreePropList		(hw_xmlAttrPtr cur);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlFreeProp		(hw_xmlAttrPtr cur);
hw_XMLPUBFUN hw_xmlAttrPtr hw_XMLCALL	
		hw_xmlCopyProp		(hw_xmlNodePtr target,
					 hw_xmlAttrPtr cur);
hw_XMLPUBFUN hw_xmlAttrPtr hw_XMLCALL	
		hw_xmlCopyPropList		(hw_xmlNodePtr target,
					 hw_xmlAttrPtr cur);
/*
 * Creating new nodes.
 */
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlNewDocNode		(hw_xmlDocPtr doc,
					 hw_xmlNsPtr ns,
					 const hw_xmlChar *name,
					 const hw_xmlChar *content);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlNewDocNodeEatName	(hw_xmlDocPtr doc,
					 hw_xmlNsPtr ns,
					 hw_xmlChar *name,
					 const hw_xmlChar *content);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlNewNode		(hw_xmlNsPtr ns,
					 const hw_xmlChar *name);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlNewNodeEatName	(hw_xmlNsPtr ns,
					 hw_xmlChar *name);

hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlNewDocText		(hw_xmlDocPtr doc,
					 const hw_xmlChar *content);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlNewText		(const hw_xmlChar *content);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlNewDocPI		(hw_xmlDocPtr doc,
					 const hw_xmlChar *name,
					 const hw_xmlChar *content);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlNewPI		(const hw_xmlChar *name,
					 const hw_xmlChar *content);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlNewDocTextLen	(hw_xmlDocPtr doc,
					 const hw_xmlChar *content,
					 int len);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlNewTextLen		(const hw_xmlChar *content,
					 int len);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlNewDocComment	(hw_xmlDocPtr doc,
					 const hw_xmlChar *content);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlNewComment		(const hw_xmlChar *content);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlNewCDataBlock	(hw_xmlDocPtr doc,
					 const hw_xmlChar *content,
					 int len);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlNewCharRef		(hw_xmlDocPtr doc,
					 const hw_xmlChar *name);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlNewReference		(hw_xmlDocPtr doc,
					 const hw_xmlChar *name);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlCopyNode		(const hw_xmlNodePtr node,
					 int recursive);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlDocCopyNode		(const hw_xmlNodePtr node,
					 hw_xmlDocPtr doc,
					 int recursive);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlDocCopyNodeList	(hw_xmlDocPtr doc,
					 const hw_xmlNodePtr node);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlCopyNodeList		(const hw_xmlNodePtr node);

/*
 * Navigating.
 */
hw_XMLPUBFUN long hw_XMLCALL		
		hw_xmlGetLineNo		(hw_xmlNodePtr node);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlDocGetRootElement	(hw_xmlDocPtr doc);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlGetLastChild		(hw_xmlNodePtr parent);
hw_XMLPUBFUN int hw_XMLCALL		
		hw_xmlNodeIsText		(hw_xmlNodePtr node);
hw_XMLPUBFUN int hw_XMLCALL		
		hw_xmlIsBlankNode		(hw_xmlNodePtr node);

/*
 * Changing the structure.
 */
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlAddChild		(hw_xmlNodePtr parent,
					 hw_xmlNodePtr cur);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlAddChildList		(hw_xmlNodePtr parent,
					 hw_xmlNodePtr cur);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlAddSibling		(hw_xmlNodePtr cur,
					 hw_xmlNodePtr elem);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlAddNextSibling	(hw_xmlNodePtr cur,
					 hw_xmlNodePtr elem);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlUnlinkNode		(hw_xmlNodePtr cur);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlTextMerge		(hw_xmlNodePtr first,
					 hw_xmlNodePtr second);
hw_XMLPUBFUN int hw_XMLCALL		
		hw_xmlTextConcat		(hw_xmlNodePtr node,
					 const hw_xmlChar *content,
					 int len);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlFreeNodeList		(hw_xmlNodePtr cur);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlFreeNode		(hw_xmlNodePtr cur);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlSetTreeDoc		(hw_xmlNodePtr tree,
					 hw_xmlDocPtr doc);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlSetListDoc		(hw_xmlNodePtr list,
					 hw_xmlDocPtr doc);
/*
 * Namespaces.
 */
hw_XMLPUBFUN hw_xmlNsPtr hw_XMLCALL	
		hw_xmlSearchNs		(hw_xmlDocPtr doc,
					 hw_xmlNodePtr node,
					 const hw_xmlChar *nameSpace);
hw_XMLPUBFUN hw_xmlNsPtr hw_XMLCALL	
		hw_xmlSearchNsByHref	(hw_xmlDocPtr doc,
					 hw_xmlNodePtr node,
					 const hw_xmlChar *href);

hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlSetNs		(hw_xmlNodePtr node,
					 hw_xmlNsPtr ns);
hw_XMLPUBFUN hw_xmlNsPtr hw_XMLCALL	
		hw_xmlCopyNamespace	(hw_xmlNsPtr cur);
hw_XMLPUBFUN hw_xmlNsPtr hw_XMLCALL	
		hw_xmlCopyNamespaceList	(hw_xmlNsPtr cur);

/*
 * Changing the content.
 */
hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL	
		hw_xmlGetNoNsProp		(hw_xmlNodePtr node,
					 const hw_xmlChar *name);
hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL	
		hw_xmlGetProp		(hw_xmlNodePtr node,
					 const hw_xmlChar *name);
hw_XMLPUBFUN hw_xmlAttrPtr hw_XMLCALL	
		hw_xmlHasProp		(hw_xmlNodePtr node,
					 const hw_xmlChar *name);
hw_XMLPUBFUN hw_xmlAttrPtr hw_XMLCALL	
		hw_xmlHasNsProp		(hw_xmlNodePtr node,
					 const hw_xmlChar *name,
					 const hw_xmlChar *nameSpace);
hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL	
		hw_xmlGetNsProp		(hw_xmlNodePtr node,
					 const hw_xmlChar *name,
					 const hw_xmlChar *nameSpace);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlStringGetNodeList	(hw_xmlDocPtr doc,
					 const hw_xmlChar *value);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL	
		hw_xmlStringLenGetNodeList	(hw_xmlDocPtr doc,
					 const hw_xmlChar *value,
					 int len);
hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL	
		hw_xmlNodeListGetString	(hw_xmlDocPtr doc,
					 hw_xmlNodePtr list,
					 int inLine);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlNodeSetContent	(hw_xmlNodePtr cur,
					 const hw_xmlChar *content);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlNodeAddContent	(hw_xmlNodePtr cur,
					 const hw_xmlChar *content);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlNodeAddContentLen	(hw_xmlNodePtr cur,
					 const hw_xmlChar *content,
					 int len);
hw_XMLPUBFUN int hw_XMLCALL		
		hw_xmlNodeGetSpacePreserve	(hw_xmlNodePtr cur);
hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL	
		hw_xmlNodeGetBase		(hw_xmlDocPtr doc,
					 hw_xmlNodePtr cur);

/*
 * Removing content.
 */
hw_XMLPUBFUN int hw_XMLCALL		
		hw_xmlRemoveProp		(hw_xmlAttrPtr cur);





/*
 * Constructor and destructor.
 */
hw_XMLPUBFUN hw_xmlDictPtr hw_XMLCALL
			hw_xmlDictCreate	(void);
hw_XMLPUBFUN int hw_XMLCALL
			hw_xmlDictReference(hw_xmlDictPtr dict);
hw_XMLPUBFUN void hw_XMLCALL			
			hw_xmlDictFree	(hw_xmlDictPtr dict);

/*
 * Lookup of entry in the dictionnary.
 */
hw_XMLPUBFUN const hw_xmlChar * hw_XMLCALL		
			hw_xmlDictLookup	(hw_xmlDictPtr dict,
		                         const hw_xmlChar *name,
		                         int len);

hw_XMLPUBFUN int hw_XMLCALL
			hw_xmlDictOwns	(hw_xmlDictPtr dict,
					 const hw_xmlChar *str);

/*
 * Cleanup function
 */
hw_XMLPUBFUN void hw_XMLCALL
                        hw_xmlDictCleanup  (void);




/*
 * This function is in tree.h:
 * hw_xmlChar *	hw_xmlNodeGetBase	(hw_xmlDocPtr doc,
 *                               hw_xmlNodePtr cur);
 */
hw_XMLPUBFUN hw_xmlURIPtr hw_XMLCALL	
		hw_xmlCreateURI		(void);
hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL	
		hw_xmlBuildURI		(const hw_xmlChar *URI,
	                         	 const hw_xmlChar *base);
hw_XMLPUBFUN hw_xmlURIPtr hw_XMLCALL	
		hw_xmlParseURI		(const char *str);
hw_XMLPUBFUN int hw_XMLCALL		
		hw_xmlParseURIReference	(hw_xmlURIPtr uri,
					 const char *str);
hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL	
		hw_xmlSaveUri		(hw_xmlURIPtr uri);
hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL       
		hw_xmlURIEscapeStr         (const hw_xmlChar *str,
 					 const hw_xmlChar *list);
hw_XMLPUBFUN char * hw_XMLCALL		
		hw_xmlURIUnescapeString	(const char *str,
					 int len,
					 char *target);
hw_XMLPUBFUN int hw_XMLCALL		
		hw_xmlNormalizeURIPath	(char *path);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlFreeURI		(hw_xmlURIPtr uri);
hw_XMLPUBFUN hw_xmlChar* hw_XMLCALL	
		hw_xmlCanonicPath		(const hw_xmlChar *path);




hw_XMLPUBFUN hw_xmlEntityPtr hw_XMLCALL	
		hw_xmlSAX2GetEntity		(void *ctx,
						 const hw_xmlChar *name);
hw_XMLPUBFUN hw_xmlEntityPtr hw_XMLCALL	
		hw_xmlSAX2GetParameterEntity	(void *ctx,
						 const hw_xmlChar *name);


hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlSAX2StartDocument		(void *ctx);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlSAX2EndDocument		(void *ctx);
hw_XMLPUBFUN void hw_XMLCALL
		hw_xmlSAX2StartElementNs		(void *ctx,
						 const hw_xmlChar *localname,
						 const hw_xmlChar *prefix,
						 const hw_xmlChar *URI,
						 int nb_namespaces,
						 const hw_xmlChar **namespaces,
						 int nb_attributes,
						 int nb_defaulted,
						 const hw_xmlChar **attributes);
hw_XMLPUBFUN void hw_XMLCALL
		hw_xmlSAX2EndElementNs		(void *ctx,
						 const hw_xmlChar *localname,
						 const hw_xmlChar *prefix,
						 const hw_xmlChar *URI);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlSAX2Reference		(void *ctx,
						 const hw_xmlChar *name);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlSAX2Characters		(void *ctx,
						 const hw_xmlChar *ch,
						 int len);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlSAX2ProcessingInstruction	(void *ctx,
						 const hw_xmlChar *target,
						 const hw_xmlChar *data);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlSAX2Comment			(void *ctx,
						 const hw_xmlChar *value);

hw_XMLPUBFUN int hw_XMLCALL
		hw_xmlSAXVersion			(hw_xmlSAXHandler *hdlr,
						 int version);




/* Creation/Deletion */
hw_XMLPUBFUN hw_xmlListPtr hw_XMLCALL
		hw_xmlListCreate		(hw_xmlListDeallocator deallocator,
	                                 hw_xmlListDataCompare compare);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlListDelete		(hw_xmlListPtr l);

/* Basic Operators */
hw_XMLPUBFUN int hw_XMLCALL		
		hw_xmlListInsert		(hw_xmlListPtr l,
					 void *data) ;
hw_XMLPUBFUN int hw_XMLCALL		
		hw_xmlListAppend		(hw_xmlListPtr l,
					 void *data) ;
hw_XMLPUBFUN int hw_XMLCALL		
		hw_xmlListRemoveFirst	(hw_xmlListPtr l,
					 void *data);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlListClear		(hw_xmlListPtr l);


/* Link operators */
hw_XMLPUBFUN void * hw_XMLCALL          
		hw_xmlLinkGetData          (hw_xmlLinkPtr lk);


/* IDs */
hw_XMLPUBFUN hw_xmlIDPtr hw_XMLCALL	
		hw_xmlAddID	       (hw_xmlValidCtxtPtr ctxt,
					hw_xmlDocPtr doc,
					const hw_xmlChar *value,
					hw_xmlAttrPtr attr);		


hw_XMLPUBFUN int hw_XMLCALL		
		hw_xmlIsID		       (hw_xmlDocPtr doc,
					hw_xmlNodePtr elem,
					hw_xmlAttrPtr attr);
hw_XMLPUBFUN int hw_XMLCALL		
		hw_xmlRemoveID	       (hw_xmlDocPtr doc, 
					hw_xmlAttrPtr attr);

/**
 * The public function calls related to validity checking.
 */


hw_XMLPUBFUN int hw_XMLCALL		
		hw_xmlIsMixedElement	(hw_xmlDocPtr doc,
					 const hw_xmlChar *name);
hw_XMLPUBFUN hw_xmlAttributePtr hw_XMLCALL	
		hw_xmlGetDtdAttrDesc	(hw_xmlDtdPtr dtd,
					 const hw_xmlChar *elem,
					 const hw_xmlChar *name);

hw_XMLPUBFUN hw_xmlElementPtr hw_XMLCALL	
		hw_xmlGetDtdElementDesc	(hw_xmlDtdPtr dtd,
					 const hw_xmlChar *name);




/*
 * hw_xmlChar handling
 */
hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL
                hw_xmlStrdup                (const hw_xmlChar *cur);
hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL
                hw_xmlStrndup               (const hw_xmlChar *cur,
                                         int len);
hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL
                hw_xmlCharStrndup           (const char *cur,
                                         int len);
hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL
                hw_xmlCharStrdup            (const char *cur);
hw_XMLPUBFUN const hw_xmlChar * hw_XMLCALL
                hw_xmlStrchr                (const hw_xmlChar *str,
                                         hw_xmlChar val);
hw_XMLPUBFUN const hw_xmlChar * hw_XMLCALL
                hw_xmlStrstr                (const hw_xmlChar *str,
                                         const hw_xmlChar *val);
hw_XMLPUBFUN int hw_XMLCALL
                hw_xmlStrcmp                (const hw_xmlChar *str1,
                                         const hw_xmlChar *str2);
hw_XMLPUBFUN int hw_XMLCALL
                hw_xmlStrncmp               (const hw_xmlChar *str1,
                                         const hw_xmlChar *str2,
                                         int len);
hw_XMLPUBFUN int hw_XMLCALL
                hw_xmlStrcasecmp            (const hw_xmlChar *str1,
                                         const hw_xmlChar *str2);
hw_XMLPUBFUN int hw_XMLCALL
                hw_xmlStrncasecmp           (const hw_xmlChar *str1,
                                         const hw_xmlChar *str2,
                                         int len);
hw_XMLPUBFUN int hw_XMLCALL
                hw_xmlStrEqual              (const hw_xmlChar *str1,
                                         const hw_xmlChar *str2);
hw_XMLPUBFUN int hw_XMLCALL
                hw_xmlStrlen                (const hw_xmlChar *str);
hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL
                hw_xmlStrcat                (hw_xmlChar *cur,
                                         const hw_xmlChar *add);
hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL
                hw_xmlStrncat               (hw_xmlChar *cur,
                                         const hw_xmlChar *add,
                                         int len);
hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL
                hw_xmlStrncatNew            (const hw_xmlChar *str1,
                                         const hw_xmlChar *str2,
                                         int len);



////////////////////////////////////////////////////////////////////////////////////////////////




hw_XMLPUBFUN hw_xmlEntityPtr hw_XMLCALL		
			hw_xmlGetPredefinedEntity	(const hw_xmlChar *name);
hw_XMLPUBFUN hw_xmlEntityPtr hw_XMLCALL		
			hw_xmlGetDocEntity		(hw_xmlDocPtr doc,
						 const hw_xmlChar *name);
hw_XMLPUBFUN hw_xmlEntityPtr hw_XMLCALL		
			hw_xmlGetParameterEntity	(hw_xmlDocPtr doc,
						 const hw_xmlChar *name);
hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL		
			hw_xmlEncodeEntitiesReentrant(hw_xmlDocPtr doc,
						 const hw_xmlChar *input);
hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL		
			hw_xmlEncodeSpecialChars	(hw_xmlDocPtr doc,
						 const hw_xmlChar *input);

hw_XMLPUBFUN void hw_XMLCALL			
			hw_xmlFreeEntitiesTable	(hw_xmlEntitiesTablePtr table);




/*
 * Interfaces for encoding handlers.
 */
hw_XMLPUBFUN void hw_XMLCALL	
	hw_xmlInitCharEncodingHandlers	(void);
hw_XMLPUBFUN void hw_XMLCALL	
	hw_xmlCleanupCharEncodingHandlers	(void);
hw_XMLPUBFUN void hw_XMLCALL	
	hw_xmlRegisterCharEncodingHandler	(hw_xmlCharEncodingHandlerPtr handler);
hw_XMLPUBFUN hw_xmlCharEncodingHandlerPtr hw_XMLCALL
	hw_xmlGetCharEncodingHandler	(hw_xmlCharEncoding enc);
hw_XMLPUBFUN hw_xmlCharEncodingHandlerPtr hw_XMLCALL
	hw_xmlFindCharEncodingHandler	(const char *name);
hw_XMLPUBFUN hw_xmlCharEncodingHandlerPtr hw_XMLCALL
	hw_xmlNewCharEncodingHandler	(const char *name, 
                          		 hw_xmlCharEncodingInputFunc input,
                          		 hw_xmlCharEncodingOutputFunc output);

/*
 * Interfaces for encoding names and aliases.
 */
hw_XMLPUBFUN const char * hw_XMLCALL
	hw_xmlGetEncodingAlias		(const char *alias);
hw_XMLPUBFUN void hw_XMLCALL	
	hw_xmlCleanupEncodingAliases	(void);
hw_XMLPUBFUN hw_xmlCharEncoding hw_XMLCALL
	hw_xmlParseCharEncoding		(const char *name);
hw_XMLPUBFUN const char * hw_XMLCALL
	hw_xmlGetCharEncodingName		(hw_xmlCharEncoding enc);

/*
 * Interfaces directly used by the parsers.
 */
hw_XMLPUBFUN hw_xmlCharEncoding hw_XMLCALL
	hw_xmlDetectCharEncoding		(const unsigned char *in,
					 int len);


hw_XMLPUBFUN int hw_XMLCALL	
	hw_xmlCharEncInFunc		(hw_xmlCharEncodingHandler *handler,
					 hw_xmlBufferPtr out,
					 hw_xmlBufferPtr in);
hw_XMLPUBFUN int hw_XMLCALL
	hw_xmlCharEncFirstLine		(hw_xmlCharEncodingHandler *handler,
					 hw_xmlBufferPtr out,
					 hw_xmlBufferPtr in);
hw_XMLPUBFUN int hw_XMLCALL	
	hw_xmlCharEncCloseFunc		(hw_xmlCharEncodingHandler *handler);

/*
 * Export a few useful functions
 */
hw_XMLPUBFUN int hw_XMLCALL	
	hw_isolat1ToUTF8			(unsigned char *out,
					 int *outlen,
					 const unsigned char *in,
					 int *inlen);






/*
 * Use the following function to reset the two global variables
 * hw_xmlGenericError and hw_xmlGenericErrorContext.
 */

hw_XMLPUBFUN void hw_XMLCALL	
    hw_initGenericErrorDefaultFunc	(hw_xmlGenericErrorFunc *handler);

/*
 * Default message routines used by SAX and Valid context for error
 * and warning reporting.
 */
hw_XMLPUBFUN void hw_XMLCDECL	
    hw_xmlParserError		(void *ctx,
				 const char *msg,
				 ...);
hw_XMLPUBFUN void hw_XMLCDECL	
    hw_xmlParserWarning		(void *ctx,
				 const char *msg,
				 ...);
hw_XMLPUBFUN void hw_XMLCDECL	
    hw_xmlParserValidityError	(void *ctx,
				 const char *msg,
				 ...);
hw_XMLPUBFUN void hw_XMLCDECL	
    hw_xmlParserValidityWarning	(void *ctx,
				 const char *msg,
				 ...);
hw_XMLPUBFUN void hw_XMLCALL	
    hw_xmlParserPrintFileInfo	(hw_xmlParserInputPtr input);
hw_XMLPUBFUN void hw_XMLCALL	
    hw_xmlParserPrintFileContext	(hw_xmlParserInputPtr input);

/*
 * Extended error information routines
 */

hw_XMLPUBFUN void hw_XMLCALL
    hw_xmlResetLastError		(void);

hw_XMLPUBFUN void hw_XMLCALL
    hw_xmlResetError		(hw_xmlErrorPtr err);
hw_XMLPUBFUN int hw_XMLCALL
    hw_xmlCopyError		(hw_xmlErrorPtr from,
    				 hw_xmlErrorPtr to);

//#ifdef IN_LIBXML
/*
 * Internal callback reporting routine
 */
hw_XMLPUBFUN void hw_XMLCALL 
    hw___xmlRaiseError		(hw_xmlStructuredErrorFunc schannel,
    				 hw_xmlGenericErrorFunc channel,
    				 void *data,
                                 void *ctx,
    				 void *node,
    				 int domain,
				 int code,
				 hw_xmlErrorLevel level,
				 const char *file,
				 int line,
				 const char *str1,
				 const char *str2,
				 const char *str3,
				 int int1,
				 int col,
				 const char *msg,
				 ...);
hw_XMLPUBFUN void hw_XMLCALL 
    hw___xmlSimpleError		(int domain,
    				 int code,
				 hw_xmlNodePtr node,
				 const char *msg,
				 const char *extra);


/*
 * internal error reporting routines, shared but not partof the API.
 */
void hw___xmlIOErr(int domain, int code, const char *extra);
void hw___xmlLoaderErr(void *ctx, const char *msg, const char *filename);


/**
 * Parser context.
 */
hw_XMLPUBFUN hw_xmlParserCtxtPtr hw_XMLCALL	
			hw_xmlCreateURLParserCtxt	(const char *filename,
						 int options);
hw_XMLPUBFUN hw_xmlParserCtxtPtr hw_XMLCALL	
			hw_xmlCreateMemoryParserCtxt(const char *buffer,
						 int size);
hw_XMLPUBFUN hw_xmlParserCtxtPtr hw_XMLCALL	
			hw_xmlCreateEntityParserCtxt(const hw_xmlChar *URL,
						 const hw_xmlChar *ID,
						 const hw_xmlChar *base);
hw_XMLPUBFUN int hw_XMLCALL			
			hw_xmlSwitchEncoding	(hw_xmlParserCtxtPtr ctxt,
						 hw_xmlCharEncoding enc);
hw_XMLPUBFUN int hw_XMLCALL			
			hw_xmlSwitchToEncoding	(hw_xmlParserCtxtPtr ctxt,
					 hw_xmlCharEncodingHandlerPtr handler);
hw_XMLPUBFUN int hw_XMLCALL			
			hw_xmlSwitchInputEncoding	(hw_xmlParserCtxtPtr ctxt,
						 hw_xmlParserInputPtr input,
					 hw_xmlCharEncodingHandlerPtr handler);

#ifdef IN_LIBXML
/* internal error reporting */
hw_XMLPUBFUN void hw_XMLCALL
			hw___xmlErrEncoding	(hw_xmlParserCtxtPtr ctxt,
						 hw_xmlParserErrors xmlerr,
						 const char *msg,
						 const hw_xmlChar * str1,
						 const hw_xmlChar * str2);
#endif

/**
 * Input Streams.
 */
hw_XMLPUBFUN hw_xmlParserInputPtr hw_XMLCALL	
			hw_xmlNewEntityInputStream	(hw_xmlParserCtxtPtr ctxt,
						 hw_xmlEntityPtr entity);
hw_XMLPUBFUN void hw_XMLCALL			
			hw_xmlPushInput		(hw_xmlParserCtxtPtr ctxt,
						 hw_xmlParserInputPtr input);
hw_XMLPUBFUN hw_xmlChar hw_XMLCALL			
			hw_xmlPopInput		(hw_xmlParserCtxtPtr ctxt);
hw_XMLPUBFUN void hw_XMLCALL			
			hw_xmlFreeInputStream	(hw_xmlParserInputPtr input);
hw_XMLPUBFUN hw_xmlParserInputPtr hw_XMLCALL	
			hw_xmlNewInputFromFile	(hw_xmlParserCtxtPtr ctxt,
						 const char *filename);
hw_XMLPUBFUN hw_xmlParserInputPtr hw_XMLCALL	
			hw_xmlNewInputStream	(hw_xmlParserCtxtPtr ctxt);


/**
 * Generic production rules.
 */
hw_XMLPUBFUN const hw_xmlChar * hw_XMLCALL		
			hw_xmlParseName		(hw_xmlParserCtxtPtr ctxt);
hw_XMLPUBFUN void hw_XMLCALL			
			hw_xmlParseCharData	(hw_xmlParserCtxtPtr ctxt,
						 int cdata);
hw_XMLPUBFUN void hw_XMLCALL			
			hw_xmlParseComment		(hw_xmlParserCtxtPtr ctxt);
hw_XMLPUBFUN hw_xmlEntityPtr hw_XMLCALL		
			hw_xmlParseEntityRef	(hw_xmlParserCtxtPtr ctxt);
hw_XMLPUBFUN void hw_XMLCALL			
			hw_xmlParseContent		(hw_xmlParserCtxtPtr ctxt);
hw_XMLPUBFUN void hw_XMLCALL			
			hw_xmlParseElement		(hw_xmlParserCtxtPtr ctxt);
hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL		
			hw_xmlParseVersionNum	(hw_xmlParserCtxtPtr ctxt);
hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL		
			hw_xmlParseVersionInfo	(hw_xmlParserCtxtPtr ctxt);
hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL		
			hw_xmlParseEncName		(hw_xmlParserCtxtPtr ctxt);
hw_XMLPUBFUN const hw_xmlChar * hw_XMLCALL		
			hw_xmlParseEncodingDecl	(hw_xmlParserCtxtPtr ctxt);
hw_XMLPUBFUN void hw_XMLCALL			
			hw_xmlParseXMLDecl		(hw_xmlParserCtxtPtr ctxt);
hw_XMLPUBFUN void hw_XMLCALL			
			hw_xmlParseTextDecl	(hw_xmlParserCtxtPtr ctxt);
hw_XMLPUBFUN void hw_XMLCALL			
			hw_xmlParseMisc		(hw_xmlParserCtxtPtr ctxt);




hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL
		hw_xmlStringDecodeEntities		(hw_xmlParserCtxtPtr ctxt,
						 const hw_xmlChar *str,
						 int what,
						 hw_xmlChar end,
						 hw_xmlChar  end2,
						 hw_xmlChar end3);
hw_XMLPUBFUN hw_xmlChar * hw_XMLCALL
		hw_xmlStringLenDecodeEntities	(hw_xmlParserCtxtPtr ctxt,
						 const hw_xmlChar *str,
						 int len,
						 int what,
						 hw_xmlChar end,
						 hw_xmlChar  end2,
						 hw_xmlChar end3);

/*
 * Generated by MACROS on top of parser.c c.f. PUSH_AND_POP.
 */
hw_XMLPUBFUN int hw_XMLCALL			hw_nodePush		(hw_xmlParserCtxtPtr ctxt, hw_xmlNodePtr value);
hw_XMLPUBFUN hw_xmlNodePtr hw_XMLCALL		hw_nodePop			(hw_xmlParserCtxtPtr ctxt);
hw_XMLPUBFUN int hw_XMLCALL			hw_inputPush		(hw_xmlParserCtxtPtr ctxt, hw_xmlParserInputPtr value);
hw_XMLPUBFUN hw_xmlParserInputPtr hw_XMLCALL	hw_inputPop		(hw_xmlParserCtxtPtr ctxt);
hw_XMLPUBFUN const hw_xmlChar * hw_XMLCALL	hw_namePop			(hw_xmlParserCtxtPtr ctxt);
hw_XMLPUBFUN int hw_XMLCALL			hw_namePush		(hw_xmlParserCtxtPtr ctxt,const hw_xmlChar *value);

/*
 * other commodities shared between parser.c and parserInternals.
 */
hw_XMLPUBFUN int hw_XMLCALL			hw_xmlSkipBlankChars	(hw_xmlParserCtxtPtr ctxt);
hw_XMLPUBFUN int hw_XMLCALL			hw_xmlStringCurrentChar	(hw_xmlParserCtxtPtr ctxt, const hw_xmlChar *cur, int *len);
hw_XMLPUBFUN void hw_XMLCALL			hw_xmlParserHandlePEReference(hw_xmlParserCtxtPtr ctxt);
hw_XMLPUBFUN int hw_XMLCALL			hw_xmlCheckLanguageID	(const hw_xmlChar *lang);

/*
 * Really core function shared with HTML parser.
 */
hw_XMLPUBFUN int hw_XMLCALL			hw_xmlCurrentChar		(hw_xmlParserCtxtPtr ctxt, int *len);
hw_XMLPUBFUN int hw_XMLCALL			hw_xmlCopyCharMultiByte	(hw_xmlChar *out,int val);
hw_XMLPUBFUN int hw_XMLCALL			hw_xmlCopyChar		(int len,hw_xmlChar *out, int val);
hw_XMLPUBFUN void hw_XMLCALL			hw_xmlNextChar		(hw_xmlParserCtxtPtr ctxt);
hw_XMLPUBFUN void hw_XMLCALL			hw_xmlParserInputShrink	(hw_xmlParserInputPtr in);


/*
 * Specific function to keep track of entities references
 * and used by the XSLT debugger.
 */

/*
 * internal only
 */
hw_XMLPUBFUN void hw_XMLCALL
	hw_xmlErrMemory		(hw_xmlParserCtxtPtr ctxt, const char *extra);


/*
 * Interfaces for input
 */
hw_XMLPUBFUN void hw_XMLCALL	hw_xmlCleanupInputCallbacks		(void);
hw_XMLPUBFUN void hw_XMLCALL	hw_xmlRegisterDefaultInputCallbacks	(void);
hw_XMLPUBFUN hw_xmlParserInputBufferPtr hw_XMLCALL	hw_xmlAllocParserInputBuffer		(hw_xmlCharEncoding enc);
hw_XMLPUBFUN hw_xmlParserInputBufferPtr hw_XMLCALL	hw_xmlParserInputBufferCreateFilename	(const char *URI,hw_xmlCharEncoding enc);
hw_XMLPUBFUN hw_xmlParserInputBufferPtr hw_XMLCALL	hw_xmlParserInputBufferCreateMem		(const char *mem, int size,hw_xmlCharEncoding enc);
hw_XMLPUBFUN hw_xmlParserInputBufferPtr hw_XMLCALL 	hw_xmlParserInputBufferCreateStatic	(const char *mem, int size,hw_xmlCharEncoding enc);
hw_XMLPUBFUN int hw_XMLCALL	hw_xmlParserInputBufferRead		(hw_xmlParserInputBufferPtr in,int len);
hw_XMLPUBFUN int hw_XMLCALL	hw_xmlParserInputBufferGrow		(hw_xmlParserInputBufferPtr in, int len);
hw_XMLPUBFUN int hw_XMLCALL	hw_xmlParserInputBufferPush		(hw_xmlParserInputBufferPtr in, int len, const char *buf);
hw_XMLPUBFUN void hw_XMLCALL	hw_xmlFreeParserInputBuffer		(hw_xmlParserInputBufferPtr in);
hw_XMLPUBFUN char * hw_XMLCALL	hw_xmlParserGetDirectory			(const char *filename);
hw_XMLPUBFUN int hw_XMLCALL     	hw_xmlRegisterInputCallbacks		(hw_xmlInputMatchCallback matchFunc,
						 hw_xmlInputOpenCallback openFunc,
						 hw_xmlInputReadCallback readFunc,
						 hw_xmlInputCloseCallback closeFunc);

 hw_xmlParserInputBufferPtr hw___xmlParserInputBufferCreateFilename(const char *URI,hw_xmlCharEncoding enc);



/* 
 * xmlNormalizeWindowsPath is obsolete, don't use it. 
 * Check hw_xmlCanonicPath in uri.h for a better alternative.
 */

hw_XMLPUBFUN int hw_XMLCALL	hw_xmlCheckFilename		(const char *path);
/**
 * Default 'file://' protocol callbacks 
 */
hw_XMLPUBFUN int hw_XMLCALL	hw_xmlFileMatch 			(const char *filename);
hw_XMLPUBFUN void * hw_XMLCALL	hw_xmlFileOpen 			(const char *filename);
hw_XMLPUBFUN int hw_XMLCALL	hw_xmlFileRead 			(void * context, char * buffer, int len);
hw_XMLPUBFUN int hw_XMLCALL	hw_xmlFileClose 			(void * context);

#ifdef __cplusplus
}
#endif

#endif /* __XML_TREE_H__ */

