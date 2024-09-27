/*
 * parser.c : an XML 1.0 parser, namespaces and validity support are mostly
 *            implemented on top of the SAX interfaces
 *
 * References:
 *   The XML specification:
 *     http://www.w3.org/TR/REC-xml
 *   Original 1.0 version:
 *     http://www.w3.org/TR/1998/REC-xml-19980210
 *   XML second edition working draft
 *     http://www.w3.org/TR/2000/WD-xml-2e-20000814
 *
 * Okay this is a big file, the parser core is around 7000 lines, then it
 * is followed by the progressive parser top routines, then the various
 * high level APIs to call the parser and a few miscellaneous functions.
 * A number of helper functions and deprecated ones have been moved to
 * parserInternals.c to reduce this file size.
 * As much as possible the functions are associated with their relative
 * production in the XML specification. A few productions defining the
 * different ranges of character are actually implanted either in 
 * parserInternals.h or parserInternals.c
 * The DOM tree build is realized from the default SAX callbacks in
 * the module SAX.c.
 * The routines doing the validation checks are in valid.c and called either
 * from the SAX callbacks or as standalone functions using a preparsed
 * document.
 *
 * See Copyright for the status of this software.
 *
 * daniel@veillard.com
 */

#define IN_LIBXML

#if defined(WIN32) && !defined (__CYGWIN__)
#define XML_DIR_SEP '\\'
#else
#define XML_DIR_SEP '/'
#endif

#include <stdlib.h>
#include <stdarg.h>
#include "Etree.h"
#include "Eparser.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/**
 * hw_xmlParserMaxDepth:
 *
 * arbitrary depth limit for the XML documents that we allow to 
 * process. This is not a limitation of the parser but a safety 
 * boundary feature.
 */
unsigned int hw_xmlParserMaxDepth = 1024;

#define SAX2 1

#define XML_PARSER_BIG_BUFFER_SIZE 300
#define XML_PARSER_BUFFER_SIZE 100

#define SAX_COMPAT_MODE hw_BAD_CAST "SAX compatibility mode document"


/* DEPR void xmlParserHandleReference(hw_xmlParserCtxtPtr ctxt); */
hw_xmlEntityPtr xmlParseStringPEReference(hw_xmlParserCtxtPtr ctxt,
                                       const hw_xmlChar **str);

/************************************************************************
 *									*
 * 		Some factorized error routines				*
 *									*
 ************************************************************************/

/**
 * xmlErrAttributeDup:
 * @ctxt:  an XML parser context
 * @prefix:  the attribute prefix
 * @localname:  the attribute localname
 *
 * Handle a redefinition of attribute error
 */
static void
xmlErrAttributeDup(hw_xmlParserCtxtPtr ctxt, const hw_xmlChar * prefix,
                   const hw_xmlChar * localname)
{
    if ((ctxt != NULL) && (ctxt->disableSAX != 0) &&
        (ctxt->instate == XML_PARSER_EOF))
	return;
    if (ctxt != NULL)
	ctxt->errNo = XML_ERR_ATTRIBUTE_REDEFINED;
    if (prefix == NULL)
        hw___xmlRaiseError(NULL, NULL, NULL, ctxt, NULL, XML_FROM_PARSER,
                        ctxt->errNo, XML_ERR_FATAL, NULL, 0,
                        (const char *) localname, NULL, NULL, 0, 0,
                        "Attribute %s redefined\n", localname);
    else
        hw___xmlRaiseError(NULL, NULL, NULL, ctxt, NULL, XML_FROM_PARSER,
                        ctxt->errNo, XML_ERR_FATAL, NULL, 0,
                        (const char *) prefix, (const char *) localname,
                        NULL, 0, 0, "Attribute %s:%s redefined\n", prefix,
                        localname);
    if (ctxt != NULL) {
	ctxt->wellFormed = 0;
	if (ctxt->recovery == 0)
	    ctxt->disableSAX = 1;
    }
}

/**
 * xmlFatalErr:
 * @ctxt:  an XML parser context
 * @error:  the error number
 * @extra:  extra information string
 *
 * Handle a fatal parser error, i.e. violating Well-Formedness constraints
 */
static void
xmlFatalErr(hw_xmlParserCtxtPtr ctxt, hw_xmlParserErrors error, const char *info)
{
    const char *errmsg;

    if ((ctxt != NULL) && (ctxt->disableSAX != 0) &&
        (ctxt->instate == XML_PARSER_EOF))
	return;
    switch (error) {
        case XML_ERR_INVALID_HEX_CHARREF:
            errmsg = "CharRef: invalid hexadecimal value\n";
            break;
        case XML_ERR_INVALID_DEC_CHARREF:
            errmsg = "CharRef: invalid decimal value\n";
            break;
        case XML_ERR_INVALID_CHARREF:
            errmsg = "CharRef: invalid value\n";
            break;
        case XML_ERR_INTERNAL_ERROR:
            errmsg = "internal error";
            break;
        case XML_ERR_PEREF_AT_EOF:
            errmsg = "PEReference at end of document\n";
            break;
        case XML_ERR_PEREF_IN_PROLOG:
            errmsg = "PEReference in prolog\n";
            break;
        case XML_ERR_PEREF_IN_EPILOG:
            errmsg = "PEReference in epilog\n";
            break;
        case XML_ERR_PEREF_NO_NAME:
            errmsg = "PEReference: no name\n";
            break;
        case XML_ERR_PEREF_SEMICOL_MISSING:
            errmsg = "PEReference: expecting ';'\n";
            break;
        case XML_ERR_ENTITY_LOOP:
            errmsg = "Detected an entity reference loop\n";
            break;
        case XML_ERR_ENTITY_NOT_STARTED:
            errmsg = "EntityValue: \" or ' expected\n";
            break;
        case XML_ERR_ENTITY_PE_INTERNAL:
            errmsg = "PEReferences forbidden in internal subset\n";
            break;
        case XML_ERR_ENTITY_NOT_FINISHED:
            errmsg = "EntityValue: \" or ' expected\n";
            break;
        case XML_ERR_ATTRIBUTE_NOT_STARTED:
            errmsg = "AttValue: \" or ' expected\n";
            break;
        case XML_ERR_LT_IN_ATTRIBUTE:
            errmsg = "Unescaped '<' not allowed in attributes values\n";
            break;
        case XML_ERR_LITERAL_NOT_STARTED:
            errmsg = "SystemLiteral \" or ' expected\n";
            break;
        case XML_ERR_LITERAL_NOT_FINISHED:
            errmsg = "Unfinished System or Public ID \" or ' expected\n";
            break;
        case XML_ERR_MISPLACED_CDATA_END:
            errmsg = "Sequence ']]>' not allowed in content\n";
            break;
        case XML_ERR_URI_REQUIRED:
            errmsg = "SYSTEM or PUBLIC, the URI is missing\n";
            break;
        case XML_ERR_PUBID_REQUIRED:
            errmsg = "PUBLIC, the Public Identifier is missing\n";
            break;
        case XML_ERR_HYPHEN_IN_COMMENT:
            errmsg = "Comment must not contain '--' (double-hyphen)\n";
            break;
        case XML_ERR_RESERVED_XML_NAME:
            errmsg = "Invalid PI name\n";
            break;
        case XML_ERR_NOTATION_NOT_STARTED:
            errmsg = "NOTATION: Name expected here\n";
            break;
        case XML_ERR_NOTATION_NOT_FINISHED:
            errmsg = "'>' required to close NOTATION declaration\n";
            break;
        case XML_ERR_VALUE_REQUIRED:
            errmsg = "Entity value required\n";
            break;
        case XML_ERR_URI_FRAGMENT:
            errmsg = "Fragment not allowed";
            break;
        case XML_ERR_ATTLIST_NOT_STARTED:
            errmsg = "'(' required to start ATTLIST enumeration\n";
            break;
        case XML_ERR_NMTOKEN_REQUIRED:
            errmsg = "NmToken expected in ATTLIST enumeration\n";
            break;
        case XML_ERR_ATTLIST_NOT_FINISHED:
            errmsg = "')' required to finish ATTLIST enumeration\n";
            break;
        case XML_ERR_MIXED_NOT_STARTED:
            errmsg = "MixedContentDecl : '|' or ')*' expected\n";
            break;
        case XML_ERR_PCDATA_REQUIRED:
            errmsg = "MixedContentDecl : '#PCDATA' expected\n";
            break;
        case XML_ERR_ELEMCONTENT_NOT_STARTED:
            errmsg = "ContentDecl : Name or '(' expected\n";
            break;
        case XML_ERR_ELEMCONTENT_NOT_FINISHED:
            errmsg = "ContentDecl : ',' '|' or ')' expected\n";
            break;
        case XML_ERR_PEREF_IN_INT_SUBSET:
            errmsg =
                "PEReference: forbidden within markup decl in internal subset\n";
            break;
        case XML_ERR_GT_REQUIRED:
            errmsg = "expected '>'\n";
            break;
        case XML_ERR_CONDSEC_INVALID:
            errmsg = "XML conditional section '[' expected\n";
            break;
        case XML_ERR_EXT_SUBSET_NOT_FINISHED:
            errmsg = "Content error in the external subset\n";
            break;
        case XML_ERR_CONDSEC_INVALID_KEYWORD:
            errmsg =
                "conditional section INCLUDE or IGNORE keyword expected\n";
            break;
        case XML_ERR_CONDSEC_NOT_FINISHED:
            errmsg = "XML conditional section not closed\n";
            break;
        case XML_ERR_XMLDECL_NOT_STARTED:
            errmsg = "Text declaration '<?xml' required\n";
            break;
        case XML_ERR_XMLDECL_NOT_FINISHED:
            errmsg = "parsing XML declaration: '?>' expected\n";
            break;
        case XML_ERR_EXT_ENTITY_STANDALONE:
            errmsg = "external parsed entities cannot be standalone\n";
            break;
        case XML_ERR_ENTITYREF_SEMICOL_MISSING:
            errmsg = "EntityRef: expecting ';'\n";
            break;
        case XML_ERR_DOCTYPE_NOT_FINISHED:
            errmsg = "DOCTYPE improperly terminated\n";
            break;
        case XML_ERR_LTSLASH_REQUIRED:
            errmsg = "EndTag: '</' not found\n";
            break;
        case XML_ERR_EQUAL_REQUIRED:
            errmsg = "expected '='\n";
            break;
        case XML_ERR_STRING_NOT_CLOSED:
            errmsg = "String not closed expecting \" or '\n";
            break;
        case XML_ERR_STRING_NOT_STARTED:
            errmsg = "String not started expecting ' or \"\n";
            break;
        case XML_ERR_ENCODING_NAME:
            errmsg = "Invalid XML encoding name\n";
            break;
        case XML_ERR_STANDALONE_VALUE:
            errmsg = "standalone accepts only 'yes' or 'no'\n";
            break;
        case XML_ERR_DOCUMENT_EMPTY:
            errmsg = "Document is empty\n";
            break;
        case XML_ERR_DOCUMENT_END:
            errmsg = "Extra content at the end of the document\n";
            break;
        case XML_ERR_NOT_WELL_BALANCED:
            errmsg = "chunk is not well balanced\n";
            break;
        case XML_ERR_EXTRA_CONTENT:
            errmsg = "extra content at the end of well balanced chunk\n";
            break;
        case XML_ERR_VERSION_MISSING:
            errmsg = "Malformed declaration expecting version\n";
            break;
#if 0
        case:
            errmsg = "\n";
            break;
#endif
        default:
            errmsg = "Unregistered error message\n";
    }
    if (ctxt != NULL)
	ctxt->errNo = error;
    hw___xmlRaiseError(NULL, NULL, NULL, ctxt, NULL, XML_FROM_PARSER, error,
                    XML_ERR_FATAL, NULL, 0, info, NULL, NULL, 0, 0, errmsg,
                    info);
    if (ctxt != NULL) {
	ctxt->wellFormed = 0;
	if (ctxt->recovery == 0)
	    ctxt->disableSAX = 1;
    }
}

/**
 * xmlFatalErrMsg:
 * @ctxt:  an XML parser context
 * @error:  the error number
 * @msg:  the error message
 *
 * Handle a fatal parser error, i.e. violating Well-Formedness constraints
 */
static void
xmlFatalErrMsg(hw_xmlParserCtxtPtr ctxt, hw_xmlParserErrors error,
               const char *msg)
{
    if ((ctxt != NULL) && (ctxt->disableSAX != 0) &&
        (ctxt->instate == XML_PARSER_EOF))
	return;
    if (ctxt != NULL)
	ctxt->errNo = error;
    hw___xmlRaiseError(NULL, NULL, NULL, ctxt, NULL, XML_FROM_PARSER, error,
                    XML_ERR_FATAL, NULL, 0, NULL, NULL, NULL, 0, 0, msg);
    if (ctxt != NULL) {
	ctxt->wellFormed = 0;
	if (ctxt->recovery == 0)
	    ctxt->disableSAX = 1;
    }
}

/**
 * xmlWarningMsg:
 * @ctxt:  an XML parser context
 * @error:  the error number
 * @msg:  the error message
 * @str1:  extra data
 * @str2:  extra data
 *
 * Handle a warning.
 */
static void
xmlWarningMsg(hw_xmlParserCtxtPtr ctxt, hw_xmlParserErrors error,
              const char *msg, const hw_xmlChar *str1, const hw_xmlChar *str2)
{
    hw_xmlStructuredErrorFunc schannel = NULL;
    
    if ((ctxt != NULL) && (ctxt->disableSAX != 0) &&
        (ctxt->instate == XML_PARSER_EOF))
	return;
    if ((ctxt != NULL) && (ctxt->sax != NULL) &&
        (ctxt->sax->initialized == hw_XML_SAX2_MAGIC))
        schannel = ctxt->sax->serror;
    hw___xmlRaiseError(schannel,
                    (ctxt->sax) ? ctxt->sax->warning : NULL,
                    ctxt->userData,
                    ctxt, NULL, XML_FROM_PARSER, error,
                    XML_ERR_WARNING, NULL, 0,
		    (const char *) str1, (const char *) str2, NULL, 0, 0,
		    msg, (const char *) str1, (const char *) str2);
}

/**
 * xmlFatalErrMsgInt:
 * @ctxt:  an XML parser context
 * @error:  the error number
 * @msg:  the error message
 * @val:  an integer value
 *
 * Handle a fatal parser error, i.e. violating Well-Formedness constraints
 */
static void
xmlFatalErrMsgInt(hw_xmlParserCtxtPtr ctxt, hw_xmlParserErrors error,
                  const char *msg, int val)
{
    if ((ctxt != NULL) && (ctxt->disableSAX != 0) &&
        (ctxt->instate == XML_PARSER_EOF))
	return;
    if (ctxt != NULL)
	ctxt->errNo = error;
    hw___xmlRaiseError(NULL, NULL, NULL,
                    ctxt, NULL, XML_FROM_PARSER, error, XML_ERR_FATAL,
                    NULL, 0, NULL, NULL, NULL, val, 0, msg, val);
    if (ctxt != NULL) {
	ctxt->wellFormed = 0;
	if (ctxt->recovery == 0)
	    ctxt->disableSAX = 1;
    }
}

/**
 * xmlFatalErrMsgStrIntStr:
 * @ctxt:  an XML parser context
 * @error:  the error number
 * @msg:  the error message
 * @str1:  an string info
 * @val:  an integer value
 * @str2:  an string info
 *
 * Handle a fatal parser error, i.e. violating Well-Formedness constraints
 */
static void
xmlFatalErrMsgStrIntStr(hw_xmlParserCtxtPtr ctxt, hw_xmlParserErrors error,
                  const char *msg, const hw_xmlChar *str1, int val, 
		  const hw_xmlChar *str2)
{
    if ((ctxt != NULL) && (ctxt->disableSAX != 0) &&
        (ctxt->instate == XML_PARSER_EOF))
	return;
    if (ctxt != NULL)
	ctxt->errNo = error;
    hw___xmlRaiseError(NULL, NULL, NULL,
                    ctxt, NULL, XML_FROM_PARSER, error, XML_ERR_FATAL,
                    NULL, 0, (const char *) str1, (const char *) str2,
		    NULL, val, 0, msg, str1, val, str2);
    if (ctxt != NULL) {
	ctxt->wellFormed = 0;
	if (ctxt->recovery == 0)
	    ctxt->disableSAX = 1;
    }
}

/**
 * xmlFatalErrMsgStr:
 * @ctxt:  an XML parser context
 * @error:  the error number
 * @msg:  the error message
 * @val:  a string value
 *
 * Handle a fatal parser error, i.e. violating Well-Formedness constraints
 */
static void
xmlFatalErrMsgStr(hw_xmlParserCtxtPtr ctxt, hw_xmlParserErrors error,
                  const char *msg, const hw_xmlChar * val)
{
    if ((ctxt != NULL) && (ctxt->disableSAX != 0) &&
        (ctxt->instate == XML_PARSER_EOF))
	return;
    if (ctxt != NULL)
	ctxt->errNo = error;
    hw___xmlRaiseError(NULL, NULL, NULL, ctxt, NULL,
                    XML_FROM_PARSER, error, XML_ERR_FATAL,
                    NULL, 0, (const char *) val, NULL, NULL, 0, 0, msg,
                    val);
    if (ctxt != NULL) {
	ctxt->wellFormed = 0;
	if (ctxt->recovery == 0)
	    ctxt->disableSAX = 1;
    }
}

/**
 * xmlErrMsgStr:
 * @ctxt:  an XML parser context
 * @error:  the error number
 * @msg:  the error message
 * @val:  a string value
 *
 * Handle a non fatal parser error
 */
static void
xmlErrMsgStr(hw_xmlParserCtxtPtr ctxt, hw_xmlParserErrors error,
                  const char *msg, const hw_xmlChar * val)
{
    if ((ctxt != NULL) && (ctxt->disableSAX != 0) &&
        (ctxt->instate == XML_PARSER_EOF))
	return;
    if (ctxt != NULL)
	ctxt->errNo = error;
    hw___xmlRaiseError(NULL, NULL, NULL, ctxt, NULL,
                    XML_FROM_PARSER, error, XML_ERR_ERROR,
                    NULL, 0, (const char *) val, NULL, NULL, 0, 0, msg,
                    val);
}

/**
 * xmlNsErr:
 * @ctxt:  an XML parser context
 * @error:  the error number
 * @msg:  the message
 * @info1:  extra information string
 * @info2:  extra information string
 *
 * Handle a fatal parser error, i.e. violating Well-Formedness constraints
 */
static void
xmlNsErr(hw_xmlParserCtxtPtr ctxt, hw_xmlParserErrors error,
         const char *msg,
         const hw_xmlChar * info1, const hw_xmlChar * info2,
         const hw_xmlChar * info3)
{
    if ((ctxt != NULL) && (ctxt->disableSAX != 0) &&
        (ctxt->instate == XML_PARSER_EOF))
	return;
    if (ctxt != NULL)
	ctxt->errNo = error;
    hw___xmlRaiseError(NULL, NULL, NULL, ctxt, NULL, XML_FROM_NAMESPACE, error,
                    XML_ERR_ERROR, NULL, 0, (const char *) info1,
                    (const char *) info2, (const char *) info3, 0, 0, msg,
                    info1, info2, info3);
    if (ctxt != NULL)
	ctxt->nsWellFormed = 0;
}

/************************************************************************
 *									*
 * 		SAX2 defaulted attributes handling			*
 *									*
 ************************************************************************/

/**
 * xmlDetectSAX2:
 * @ctxt:  an XML parser context
 *
 * Do the SAX2 detection and specific intialization
 */
static void
xmlDetectSAX2(hw_xmlParserCtxtPtr ctxt) {
    if (ctxt == NULL) return;

    ctxt->sax2 = 1;

    ctxt->str_xml = hw_xmlDictLookup(ctxt->dict, hw_BAD_CAST "xml", 3);
    ctxt->str_xmlns = hw_xmlDictLookup(ctxt->dict, hw_BAD_CAST "xmlns", 5);
    ctxt->str_xml_ns = hw_xmlDictLookup(ctxt->dict, hw_XML_XML_NAMESPACE, 36);
    if ((ctxt->str_xml==NULL) || (ctxt->str_xmlns==NULL) || 
    		(ctxt->str_xml_ns == NULL)) {
        hw_xmlErrMemory(ctxt, NULL);
    }
}

typedef struct _xmlDefAttrs xmlDefAttrs;
typedef xmlDefAttrs *xmlDefAttrsPtr;
struct _xmlDefAttrs {
    int nbAttrs;	/* number of defaulted attributes on that element */
    int maxAttrs;       /* the size of the array */
    const hw_xmlChar *values[4]; /* array of localname/prefix/values */
};


/**
 * hw_xmlCheckLanguageID:
 * @lang:  pointer to the string value
 *
 * Checks that the value conforms to the LanguageID production:
 *
 * NOTE: this is somewhat deprecated, those productions were removed from
 *       the XML Second edition.
 *
 * [33] LanguageID ::= Langcode ('-' Subcode)*
 * [34] Langcode ::= ISO639Code |  IanaCode |  UserCode
 * [35] ISO639Code ::= ([a-z] | [A-Z]) ([a-z] | [A-Z])
 * [36] IanaCode ::= ('i' | 'I') '-' ([a-z] | [A-Z])+
 * [37] UserCode ::= ('x' | 'X') '-' ([a-z] | [A-Z])+
 * [38] Subcode ::= ([a-z] | [A-Z])+
 *
 * Returns 1 if correct 0 otherwise
 **/
int
hw_xmlCheckLanguageID(const hw_xmlChar * lang)
{
    const hw_xmlChar *cur = lang;

    if (cur == NULL)
        return (0);
    if (((cur[0] == 'i') && (cur[1] == '-')) ||
        ((cur[0] == 'I') && (cur[1] == '-'))) {
        /*
         * IANA code
         */
        cur += 2;
        while (((cur[0] >= 'A') && (cur[0] <= 'Z')) ||  /* non input consuming */
               ((cur[0] >= 'a') && (cur[0] <= 'z')))
            cur++;
    } else if (((cur[0] == 'x') && (cur[1] == '-')) ||
               ((cur[0] == 'X') && (cur[1] == '-'))) {
        /*
         * User code
         */
        cur += 2;
        while (((cur[0] >= 'A') && (cur[0] <= 'Z')) ||  /* non input consuming */
               ((cur[0] >= 'a') && (cur[0] <= 'z')))
            cur++;
    } else if (((cur[0] >= 'A') && (cur[0] <= 'Z')) ||
               ((cur[0] >= 'a') && (cur[0] <= 'z'))) {
        /*
         * ISO639
         */
        cur++;
        if (((cur[0] >= 'A') && (cur[0] <= 'Z')) ||
            ((cur[0] >= 'a') && (cur[0] <= 'z')))
            cur++;
        else
            return (0);
    } else
        return (0);
    while (cur[0] != 0) {       /* non input consuming */
        if (cur[0] != '-')
            return (0);
        cur++;
        if (((cur[0] >= 'A') && (cur[0] <= 'Z')) ||
            ((cur[0] >= 'a') && (cur[0] <= 'z')))
            cur++;
        else
            return (0);
        while (((cur[0] >= 'A') && (cur[0] <= 'Z')) ||  /* non input consuming */
               ((cur[0] >= 'a') && (cur[0] <= 'z')))
            cur++;
    }
    return (1);
}

/************************************************************************
 *									*
 * 		Parser stacks related functions and macros		*
 *									*
 ************************************************************************/

hw_xmlEntityPtr xmlParseStringEntityRef(hw_xmlParserCtxtPtr ctxt,
                                     const hw_xmlChar ** str);

#ifdef SAX2
/**
 * nsPush:
 * @ctxt:  an XML parser context
 * @prefix:  the namespace prefix or NULL
 * @URL:  the namespace name
 *
 * Pushes a new parser namespace on top of the ns stack
 *
 * Returns -1 in case of error, -2 if the namespace should be discarded
 *	   and the index in the stack otherwise.
 */
static int
nsPush(hw_xmlParserCtxtPtr ctxt, const hw_xmlChar *prefix, const hw_xmlChar *URL)
{

    if ((ctxt->nsMax == 0) || (ctxt->nsTab == NULL)) {
	ctxt->nsMax = 10;
	ctxt->nsNr = 0;
	ctxt->nsTab = (const hw_xmlChar **)
	              hw_xmlMalloc(ctxt->nsMax * sizeof(hw_xmlChar *));
	if (ctxt->nsTab == NULL) {
	    hw_xmlErrMemory(ctxt, NULL);
	    ctxt->nsMax = 0;
            return (-1);
	}
    } else if (ctxt->nsNr >= ctxt->nsMax) {
        ctxt->nsMax *= 2;
        ctxt->nsTab = (const hw_xmlChar **)
	              hw_xmlRealloc((char *) ctxt->nsTab,
				 ctxt->nsMax * sizeof(ctxt->nsTab[0]));
        if (ctxt->nsTab == NULL) {
            hw_xmlErrMemory(ctxt, NULL);
	    ctxt->nsMax /= 2;
            return (-1);
        }
    }
    ctxt->nsTab[ctxt->nsNr++] = prefix;
    ctxt->nsTab[ctxt->nsNr++] = URL;
    return (ctxt->nsNr);
}
/**
 * nsPop:
 * @ctxt: an XML parser context
 * @nr:  the number to pop
 *
 * Pops the top @nr parser prefix/namespace from the ns stack
 *
 * Returns the number of namespaces removed
 */
static int
nsPop(hw_xmlParserCtxtPtr ctxt, int nr)
{
    int i;

    if (ctxt->nsTab == NULL) return(0);
    if (ctxt->nsNr < nr) {
        hw_xmlGenericError(hw_xmlGenericErrorContext, "Pbm popping %d NS\n", nr);
        nr = ctxt->nsNr;
    }
    if (ctxt->nsNr <= 0)
        return (0);
    
    for (i = 0;i < nr;i++) {
         ctxt->nsNr--;
	 ctxt->nsTab[ctxt->nsNr] = NULL;
    }
    return(nr);
}
#endif

static int
xmlCtxtGrowAttrs(hw_xmlParserCtxtPtr ctxt, int nr) {
    const hw_xmlChar **atts;
    int *attallocs;
    int maxatts;

    if (ctxt->atts == NULL) {
	maxatts = 55; /* allow for 10 attrs by default */
	atts = (const hw_xmlChar **)
	       hw_xmlMalloc(maxatts * sizeof(hw_xmlChar *));
	if (atts == NULL) goto mem_error;
	ctxt->atts = atts;
	attallocs = (int *) hw_xmlMalloc((maxatts / 5) * sizeof(int));
	if (attallocs == NULL) goto mem_error;
	ctxt->attallocs = attallocs;
	ctxt->maxatts = maxatts;
    } else if (nr + 5 > ctxt->maxatts) {
	maxatts = (nr + 5) * 2;
	atts = (const hw_xmlChar **) hw_xmlRealloc((void *) ctxt->atts,
				     maxatts * sizeof(const hw_xmlChar *));
	if (atts == NULL) goto mem_error;
	ctxt->atts = atts;
	attallocs = (int *) hw_xmlRealloc((void *) ctxt->attallocs,
	                             (maxatts / 5) * sizeof(int));
	if (attallocs == NULL) goto mem_error;
	ctxt->attallocs = attallocs;
	ctxt->maxatts = maxatts;
    }
    return(ctxt->maxatts);
mem_error:
    hw_xmlErrMemory(ctxt, NULL);
    return(-1);
}

/**
 * hw_inputPush:
 * @ctxt:  an XML parser context
 * @value:  the parser input
 *
 * Pushes a new parser input on top of the input stack
 *
 * Returns 0 in case of error, the index in the stack otherwise
 */
int
hw_inputPush(hw_xmlParserCtxtPtr ctxt, hw_xmlParserInputPtr value)
{
    if ((ctxt == NULL) || (value == NULL))
        return(0);
    if (ctxt->inputNr >= ctxt->inputMax) {
        ctxt->inputMax *= 2;
        ctxt->inputTab =
            (hw_xmlParserInputPtr *) hw_xmlRealloc(ctxt->inputTab,
                                             ctxt->inputMax *
                                             sizeof(ctxt->inputTab[0]));
        if (ctxt->inputTab == NULL) {
            hw_xmlErrMemory(ctxt, NULL);
            return (0);
        }
    }
    ctxt->inputTab[ctxt->inputNr] = value;
    ctxt->input = value;
    return (ctxt->inputNr++);
}
/**
 * hw_inputPop:
 * @ctxt: an XML parser context
 *
 * Pops the top parser input from the input stack
 *
 * Returns the input just removed
 */
hw_xmlParserInputPtr
hw_inputPop(hw_xmlParserCtxtPtr ctxt)
{
    hw_xmlParserInputPtr ret;

    if (ctxt == NULL)
        return(NULL);
    if (ctxt->inputNr <= 0)
        return (NULL);
    ctxt->inputNr--;
    if (ctxt->inputNr > 0)
        ctxt->input = ctxt->inputTab[ctxt->inputNr - 1];
    else
        ctxt->input = NULL;
    ret = ctxt->inputTab[ctxt->inputNr];
    ctxt->inputTab[ctxt->inputNr] = NULL;
    return (ret);
}
/**
 * hw_nodePush:
 * @ctxt:  an XML parser context
 * @value:  the element node
 *
 * Pushes a new element node on top of the node stack
 *
 * Returns 0 in case of error, the index in the stack otherwise
 */
int
hw_nodePush(hw_xmlParserCtxtPtr ctxt, hw_xmlNodePtr value)
{
    if (ctxt == NULL) return(0);
    if (ctxt->nodeNr >= ctxt->nodeMax) {
        hw_xmlNodePtr *tmp;

	tmp = (hw_xmlNodePtr *) hw_xmlRealloc(ctxt->nodeTab,
                                      ctxt->nodeMax * 2 *
                                      sizeof(ctxt->nodeTab[0]));
        if (tmp == NULL) {
            hw_xmlErrMemory(ctxt, NULL);
            return (0);
        }
        ctxt->nodeTab = tmp;
	ctxt->nodeMax *= 2;
    }
    if (((unsigned int) ctxt->nodeNr) > hw_xmlParserMaxDepth) {
	xmlFatalErrMsgInt(ctxt, XML_ERR_INTERNAL_ERROR,
		 "Excessive depth in document: change hw_xmlParserMaxDepth = %d\n",
			  hw_xmlParserMaxDepth);
	ctxt->instate = XML_PARSER_EOF;
	return(0);
    }
    ctxt->nodeTab[ctxt->nodeNr] = value;
    ctxt->node = value;
    return (ctxt->nodeNr++);
}
/**
 * hw_nodePop:
 * @ctxt: an XML parser context
 *
 * Pops the top element node from the node stack
 *
 * Returns the node just removed
 */
hw_xmlNodePtr
hw_nodePop(hw_xmlParserCtxtPtr ctxt)
{
    hw_xmlNodePtr ret;

    if (ctxt == NULL) return(NULL);
    if (ctxt->nodeNr <= 0)
        return (NULL);
    ctxt->nodeNr--;
    if (ctxt->nodeNr > 0)
        ctxt->node = ctxt->nodeTab[ctxt->nodeNr - 1];
    else
        ctxt->node = NULL;
    ret = ctxt->nodeTab[ctxt->nodeNr];
    ctxt->nodeTab[ctxt->nodeNr] = NULL;
    return (ret);
}


/**
 * hw_namePush:
 * @ctxt:  an XML parser context
 * @value:  the element name
 *
 * Pushes a new element name on top of the name stack
 *
 * Returns -1 in case of error, the index in the stack otherwise
 */
int
hw_namePush(hw_xmlParserCtxtPtr ctxt, const hw_xmlChar * value)
{
    if (ctxt == NULL) return (-1);

    if (ctxt->nameNr >= ctxt->nameMax) {
        const hw_xmlChar * *tmp;
        ctxt->nameMax *= 2;
        tmp = (const hw_xmlChar * *) hw_xmlRealloc((hw_xmlChar * *)ctxt->nameTab,
                                    ctxt->nameMax *
                                    sizeof(ctxt->nameTab[0]));
        if (tmp == NULL) {
	    ctxt->nameMax /= 2;
	    goto mem_error;
        }
	ctxt->nameTab = tmp;
    }
    ctxt->nameTab[ctxt->nameNr] = value;
    ctxt->name = value;
    return (ctxt->nameNr++);
mem_error:
    hw_xmlErrMemory(ctxt, NULL);
    return (-1);
}
/**
 * hw_namePop:
 * @ctxt: an XML parser context
 *
 * Pops the top element name from the name stack
 *
 * Returns the name just removed
 */
const hw_xmlChar *
hw_namePop(hw_xmlParserCtxtPtr ctxt)
{
    const hw_xmlChar *ret;

    if ((ctxt == NULL) || (ctxt->nameNr <= 0))
        return (NULL);
    ctxt->nameNr--;
    if (ctxt->nameNr > 0)
        ctxt->name = ctxt->nameTab[ctxt->nameNr - 1];
    else
        ctxt->name = NULL;
    ret = ctxt->nameTab[ctxt->nameNr];
    ctxt->nameTab[ctxt->nameNr] = NULL;
    return (ret);
}

static int spacePush(hw_xmlParserCtxtPtr ctxt, int val) {
    if (ctxt->spaceNr >= ctxt->spaceMax) {
	ctxt->spaceMax *= 2;
        ctxt->spaceTab = (int *) hw_xmlRealloc(ctxt->spaceTab,
	             ctxt->spaceMax * sizeof(ctxt->spaceTab[0]));
        if (ctxt->spaceTab == NULL) {
	    hw_xmlErrMemory(ctxt, NULL);
	    return(0);
	}
    }
    ctxt->spaceTab[ctxt->spaceNr] = val;
    ctxt->space = &ctxt->spaceTab[ctxt->spaceNr];
    return(ctxt->spaceNr++);
}

static int spacePop(hw_xmlParserCtxtPtr ctxt) {
    int ret;
    if (ctxt->spaceNr <= 0) return(0);
    ctxt->spaceNr--;
    if (ctxt->spaceNr > 0)
	ctxt->space = &ctxt->spaceTab[ctxt->spaceNr - 1];
    else
        ctxt->space = NULL;
    ret = ctxt->spaceTab[ctxt->spaceNr];
    ctxt->spaceTab[ctxt->spaceNr] = -1;
    return(ret);
}

/*
 * Macros for accessing the content. Those should be used only by the parser,
 * and not exported.
 *
 * Dirty macros, i.e. one often need to make assumption on the context to
 * use them
 *
 *   CUR_PTR return the current pointer to the hw_xmlChar to be parsed.
 *           To be used with extreme caution since operations consuming
 *           characters may move the input buffer to a different location !
 *   CUR     returns the current hw_xmlChar value, i.e. a 8 bit value if compiled
 *           This should be used internally by the parser
 *           only to compare to ASCII values otherwise it would break when
 *           running with UTF-8 encoding.
 *   RAW     same as CUR but in the input buffer, bypass any token
 *           extraction that may have been done
 *   NXT(n)  returns the n'th next hw_xmlChar. Same as CUR is should be used only
 *           to compare on ASCII based substring.
 *   SKIP(n) Skip n hw_xmlChar, and must also be used only to skip ASCII defined
 *           strings without newlines within the parser.
 *   NEXT1(l) Skip 1 hw_xmlChar, and must also be used only to skip 1 non-newline ASCII 
 *           defined char within the parser.
 * Clean macros, not dependent of an ASCII context, expect UTF-8 encoding
 *
 *   NEXT    Skip to the next character, this does the proper decoding
 *           in UTF-8 mode. It also pop-up unfinished entities on the fly.
 *   NEXTL(l) Skip the current unicode character of l xmlChars long.
 *   CUR_CHAR(l) returns the current unicode character (int), set l
 *           to the number of xmlChars used for the encoding [0-5].
 *   CUR_SCHAR  same but operate on a string instead of the context
 *   COPY_BUF  copy the current unicode char to the target buffer, increment
 *            the index
 *   GROW, SHRINK  handling of input buffers
 */

#define RAW (*ctxt->input->cur)
#define CUR (*ctxt->input->cur)
#define NXT(val) ctxt->input->cur[(val)]
#define CUR_PTR ctxt->input->cur

#define CMP4( s, c1, c2, c3, c4 ) \
  ( ((unsigned char *) s)[ 0 ] == c1 && ((unsigned char *) s)[ 1 ] == c2 && \
    ((unsigned char *) s)[ 2 ] == c3 && ((unsigned char *) s)[ 3 ] == c4 )
#define CMP5( s, c1, c2, c3, c4, c5 ) \
  ( CMP4( s, c1, c2, c3, c4 ) && ((unsigned char *) s)[ 4 ] == c5 )
#define CMP6( s, c1, c2, c3, c4, c5, c6 ) \
  ( CMP5( s, c1, c2, c3, c4, c5 ) && ((unsigned char *) s)[ 5 ] == c6 )
#define CMP7( s, c1, c2, c3, c4, c5, c6, c7 ) \
  ( CMP6( s, c1, c2, c3, c4, c5, c6 ) && ((unsigned char *) s)[ 6 ] == c7 )
#define CMP8( s, c1, c2, c3, c4, c5, c6, c7, c8 ) \
  ( CMP7( s, c1, c2, c3, c4, c5, c6, c7 ) && ((unsigned char *) s)[ 7 ] == c8 )
#define CMP9( s, c1, c2, c3, c4, c5, c6, c7, c8, c9 ) \
  ( CMP8( s, c1, c2, c3, c4, c5, c6, c7, c8 ) && \
    ((unsigned char *) s)[ 8 ] == c9 )
#define CMP10( s, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10 ) \
  ( CMP9( s, c1, c2, c3, c4, c5, c6, c7, c8, c9 ) && \
    ((unsigned char *) s)[ 9 ] == c10 )

#define SKIP(val) do {							\
    ctxt->nbChars += (val),ctxt->input->cur += (val),ctxt->input->col+=(val);			\
    if (*ctxt->input->cur == '%') hw_xmlParserHandlePEReference(ctxt);	\
    if ((*ctxt->input->cur == 0) &&					\
        (hw_xmlParserInputGrow(ctxt->input, hw_INPUT_CHUNK) <= 0))		\
	    hw_xmlPopInput(ctxt);						\
  } while (0)

#define SKIPL(val) do {							\
    int skipl;								\
    for(skipl=0; skipl<val; skipl++) {					\
    	if (*(ctxt->input->cur) == '\n') {				\
	ctxt->input->line++; ctxt->input->col = 1;			\
    	} else ctxt->input->col++;					\
    	ctxt->nbChars++;						\
	ctxt->input->cur++;						\
    }									\
    if (*ctxt->input->cur == '%') hw_xmlParserHandlePEReference(ctxt);	\
    if ((*ctxt->input->cur == 0) &&					\
        (hw_xmlParserInputGrow(ctxt->input, hw_INPUT_CHUNK) <= 0))		\
	    hw_xmlPopInput(ctxt);						\
  } while (0)

#define SHRINK if ((ctxt->progressive == 0) &&				\
		   (ctxt->input->cur - ctxt->input->base > 2 * hw_INPUT_CHUNK) && \
		   (ctxt->input->end - ctxt->input->cur < 2 * hw_INPUT_CHUNK)) \
	xmlSHRINK (ctxt);

static void xmlSHRINK (hw_xmlParserCtxtPtr ctxt) {
    hw_xmlParserInputShrink(ctxt->input);
    if ((*ctxt->input->cur == 0) &&
        (hw_xmlParserInputGrow(ctxt->input, hw_INPUT_CHUNK) <= 0))
	    hw_xmlPopInput(ctxt);
  }

#define GROW if ((ctxt->progressive == 0) &&				\
		 (ctxt->input->end - ctxt->input->cur < hw_INPUT_CHUNK))	\
	xmlGROW (ctxt);

static void xmlGROW (hw_xmlParserCtxtPtr ctxt) {
    hw_xmlParserInputGrow(ctxt->input, hw_INPUT_CHUNK);
    if ((*ctxt->input->cur == 0) &&
        (hw_xmlParserInputGrow(ctxt->input, hw_INPUT_CHUNK) <= 0))
	    hw_xmlPopInput(ctxt);
}

#define SKIP_BLANKS hw_xmlSkipBlankChars(ctxt)

#define NEXT hw_xmlNextChar(ctxt)

#define NEXT1 {								\
	ctxt->input->col++;						\
	ctxt->input->cur++;						\
	ctxt->nbChars++;						\
	if (*ctxt->input->cur == 0)					\
	    hw_xmlParserInputGrow(ctxt->input, hw_INPUT_CHUNK);		\
    }

#define NEXTL(l) do {							\
    if (*(ctxt->input->cur) == '\n') {					\
	ctxt->input->line++; ctxt->input->col = 1;			\
    } else ctxt->input->col++;						\
    ctxt->input->cur += l;				\
    if (*ctxt->input->cur == '%') hw_xmlParserHandlePEReference(ctxt);	\
  } while (0)

#define CUR_CHAR(l) hw_xmlCurrentChar(ctxt, &l)
#define CUR_SCHAR(s, l) hw_xmlStringCurrentChar(ctxt, s, &l)

#define COPY_BUF(l,b,i,v)						\
    if (l == 1) b[i++] = (hw_xmlChar) v;					\
    else i += hw_xmlCopyCharMultiByte(&b[i],v)

/**
 * hw_xmlSkipBlankChars:
 * @ctxt:  the XML parser context
 *
 * skip all blanks character found at that point in the input streams.
 * It pops up finished entities in the process if allowable at that point.
 *
 * Returns the number of space chars skipped
 */

int
hw_xmlSkipBlankChars(hw_xmlParserCtxtPtr ctxt) {
    int res = 0;

    /*
     * It's Okay to use CUR/NEXT here since all the blanks are on
     * the ASCII range.
     */
    if ((ctxt->inputNr == 1) && (ctxt->instate != XML_PARSER_DTD)) {
	const hw_xmlChar *cur;
	/*
	 * if we are in the document content, go really fast
	 */
	cur = ctxt->input->cur;
	while (hw_IS_BLANK_CH(*cur)) {
	    if (*cur == '\n') {
		ctxt->input->line++; ctxt->input->col = 1;
	    }
	    cur++;
	    res++;
	    if (*cur == 0) {
		ctxt->input->cur = cur;
		hw_xmlParserInputGrow(ctxt->input, hw_INPUT_CHUNK);
		cur = ctxt->input->cur;
	    }
	}
	ctxt->input->cur = cur;
    } else {
	int cur;
	do {
	    cur = CUR;
	    while (hw_IS_BLANK_CH(cur)) { /* CHECKED tstblanks.xml */
		NEXT;
		cur = CUR;
		res++;
	    }
	    while ((cur == 0) && (ctxt->inputNr > 1) &&
		   (ctxt->instate != XML_PARSER_COMMENT)) {
		hw_xmlPopInput(ctxt);
		cur = CUR;
	    }
	    /*
	     * Need to handle support of entities branching here
	     */
	    if (*ctxt->input->cur == '%') hw_xmlParserHandlePEReference(ctxt);
	} while (hw_IS_BLANK(cur)); /* CHECKED tstblanks.xml */
    }
    return(res);
}

/************************************************************************
 *									*
 *		Commodity functions to handle entities			*
 *									*
 ************************************************************************/

/**
 * hw_xmlPopInput:
 * @ctxt:  an XML parser context
 *
 * hw_xmlPopInput: the current input pointed by ctxt->input came to an end
 *          pop it and return the next char.
 *
 * Returns the current hw_xmlChar in the parser context
 */
hw_xmlChar
hw_xmlPopInput(hw_xmlParserCtxtPtr ctxt) {
    if ((ctxt == NULL) || (ctxt->inputNr <= 1)) return(0);

    hw_xmlFreeInputStream(hw_inputPop(ctxt));
    if ((*ctxt->input->cur == 0) &&
        (hw_xmlParserInputGrow(ctxt->input, hw_INPUT_CHUNK) <= 0))
	    return(hw_xmlPopInput(ctxt));
    return(CUR);
}

/**
 * hw_xmlPushInput:
 * @ctxt:  an XML parser context
 * @input:  an XML parser input fragment (entity, XML fragment ...).
 *
 * hw_xmlPushInput: switch to a new input stream which is stacked on top
 *               of the previous one(s).
 */
void
hw_xmlPushInput(hw_xmlParserCtxtPtr ctxt, hw_xmlParserInputPtr input) {
    if (input == NULL) return;

    hw_inputPush(ctxt, input);
    GROW;
}


/**
 * xmlParseStringCharRef:
 * @ctxt:  an XML parser context
 * @str:  a pointer to an index in the string
 *
 * parse Reference declarations, variant parsing from a string rather
 * than an an input flow.
 *
 * [66] CharRef ::= '&#' [0-9]+ ';' |
 *                  '&#x' [0-9a-fA-F]+ ';'
 *
 * [ WFC: Legal Character ]
 * Characters referred to using character references must match the
 * production for Char. 
 *
 * Returns the value parsed (as an int), 0 in case of error, str will be
 *         updated to the current value of the index
 */
static int
xmlParseStringCharRef(hw_xmlParserCtxtPtr ctxt, const hw_xmlChar **str) {
    const hw_xmlChar *ptr;
    hw_xmlChar cur;
    unsigned int val = 0;
    unsigned int outofrange = 0;

    if ((str == NULL) || (*str == NULL)) return(0);
    ptr = *str;
    cur = *ptr;
    if ((cur == '&') && (ptr[1] == '#') && (ptr[2] == 'x')) {
	ptr += 3;
	cur = *ptr;
	while (cur != ';') { /* Non input consuming loop */
	    if ((cur >= '0') && (cur <= '9')) 
	        val = val * 16 + (cur - '0');
	    else if ((cur >= 'a') && (cur <= 'f'))
	        val = val * 16 + (cur - 'a') + 10;
	    else if ((cur >= 'A') && (cur <= 'F'))
	        val = val * 16 + (cur - 'A') + 10;
	    else {
		xmlFatalErr(ctxt, XML_ERR_INVALID_HEX_CHARREF, NULL);
		val = 0;
		break;
	    }
	    if (val > 0x10FFFF)
	        outofrange = val;

	    ptr++;
	    cur = *ptr;
	}
	if (cur == ';')
	    ptr++;
    } else if  ((cur == '&') && (ptr[1] == '#')){
	ptr += 2;
	cur = *ptr;
	while (cur != ';') { /* Non input consuming loops */
	    if ((cur >= '0') && (cur <= '9')) 
	        val = val * 10 + (cur - '0');
	    else {
		xmlFatalErr(ctxt, XML_ERR_INVALID_DEC_CHARREF, NULL);
		val = 0;
		break;
	    }
	    if (val > 0x10FFFF)
	        outofrange = val;

	    ptr++;
	    cur = *ptr;
	}
	if (cur == ';')
	    ptr++;
    } else {
	xmlFatalErr(ctxt, XML_ERR_INVALID_CHARREF, NULL);
	return(0);
    }
    *str = ptr;

    /*
     * [ WFC: Legal Character ]
     * Characters referred to using character references must match the
     * production for Char. 
     */
    if ((hw_IS_CHAR(val) && (outofrange == 0))) {
        return(val);
    } else {
        xmlFatalErrMsgInt(ctxt, XML_ERR_INVALID_CHAR,
			  "xmlParseStringCharRef: invalid hw_xmlChar value %d\n",
			  val);
    }
    return(0);
}

/**
 * xmlNewBlanksWrapperInputStream:
 * @ctxt:  an XML parser context
 * @entity:  an Entity pointer
 *
 * Create a new input stream for wrapping
 * blanks around a PEReference
 *
 * Returns the new input stream or NULL
 */
 
static void deallocblankswrapper (hw_xmlChar *str) {hw_xmlFree(str);}
 
static hw_xmlParserInputPtr
xmlNewBlanksWrapperInputStream(hw_xmlParserCtxtPtr ctxt, hw_xmlEntityPtr entity) {
    hw_xmlParserInputPtr input;
    hw_xmlChar *buffer;
    size_t length;
    if (entity == NULL) {
	xmlFatalErr(ctxt, XML_ERR_INTERNAL_ERROR,
	            "xmlNewBlanksWrapperInputStream entity\n");
	return(NULL);
    }

    input = hw_xmlNewInputStream(ctxt);
    if (input == NULL) {
	return(NULL);
    }
    length = hw_xmlStrlen(entity->name) + 5;
    buffer = hw_xmlMallocAtomic(length);
    if (buffer == NULL) {
	hw_xmlErrMemory(ctxt, NULL);
    	return(NULL);
    }
    buffer [0] = ' ';
    buffer [1] = '%';
    buffer [length-3] = ';';
    buffer [length-2] = ' ';
    buffer [length-1] = 0;
    memcpy(buffer + 2, entity->name, length - 5);
    input->free = deallocblankswrapper;
    input->base = buffer;
    input->cur = buffer;
    input->length = length;
    input->end = &buffer[length];
    return(input);
}

/**
 * hw_xmlParserHandlePEReference:
 * @ctxt:  the parser context
 * 
 * [69] PEReference ::= '%' Name ';'
 *
 * [ WFC: No Recursion ]
 * A parsed entity must not contain a recursive
 * reference to itself, either directly or indirectly. 
 *
 * [ WFC: Entity Declared ]
 * In a document without any DTD, a document with only an internal DTD
 * subset which contains no parameter entity references, or a document
 * with "standalone='yes'", ...  ... The declaration of a parameter
 * entity must precede any reference to it...
 *
 * [ VC: Entity Declared ]
 * In a document with an external subset or external parameter entities
 * with "standalone='no'", ...  ... The declaration of a parameter entity
 * must precede any reference to it...
 *
 * [ WFC: In DTD ]
 * Parameter-entity references may only appear in the DTD.
 * NOTE: misleading but this is handled.
 *
 * A PEReference may have been detected in the current input stream
 * the handling is done accordingly to 
 *      http://www.w3.org/TR/REC-xml#entproc
 * i.e. 
 *   - Included in literal in entity values
 *   - Included as Parameter Entity reference within DTDs
 */
void
hw_xmlParserHandlePEReference(hw_xmlParserCtxtPtr ctxt) {
    const hw_xmlChar *name;
    hw_xmlEntityPtr entity = NULL;
    hw_xmlParserInputPtr input;

    if (RAW != '%') return;
    switch(ctxt->instate) {
	case XML_PARSER_CDATA_SECTION:
	    return;
        case XML_PARSER_COMMENT:
	    return;
	case XML_PARSER_START_TAG:
	    return;
	case XML_PARSER_END_TAG:
	    return;
        case XML_PARSER_EOF:
	    xmlFatalErr(ctxt, XML_ERR_PEREF_AT_EOF, NULL);
	    return;
        case XML_PARSER_PROLOG:
	case XML_PARSER_START:
	case XML_PARSER_MISC:
	    xmlFatalErr(ctxt, XML_ERR_PEREF_IN_PROLOG, NULL);
	    return;
	case XML_PARSER_ENTITY_DECL:
        case XML_PARSER_CONTENT:
        case XML_PARSER_ATTRIBUTE_VALUE:
        case XML_PARSER_PI:
	case XML_PARSER_SYSTEM_LITERAL:
	case XML_PARSER_PUBLIC_LITERAL:
	    /* we just ignore it there */
	    return;
        case XML_PARSER_EPILOG:
	    xmlFatalErr(ctxt, XML_ERR_PEREF_IN_EPILOG, NULL);
	    return;
	case XML_PARSER_ENTITY_VALUE:
	    /*
	     * NOTE: in the case of entity values, we don't do the
	     *       substitution here since we need the literal
	     *       entity value to be able to save the internal
	     *       subset of the document.
	     *       This will be handled by hw_xmlStringDecodeEntities
	     */
	    return;
        case XML_PARSER_DTD:
	    /*
	     * [WFC: Well-Formedness Constraint: PEs in Internal Subset]
	     * In the internal DTD subset, parameter-entity references
	     * can occur only where markup declarations can occur, not
	     * within markup declarations.
	     * In that case this is handled in xmlParseMarkupDecl
	     */
	    if ((ctxt->external == 0) && (ctxt->inputNr == 1))
		return;
	    if (hw_IS_BLANK_CH(NXT(1)) || NXT(1) == 0)
		return;
            break;
        case XML_PARSER_IGNORE:
            return;
    }

    NEXT;
    name = hw_xmlParseName(ctxt);

    if (name == NULL) {
	xmlFatalErr(ctxt, XML_ERR_PEREF_NO_NAME, NULL);
    } else {
	if (RAW == ';') {
	    NEXT;
	    if ((ctxt->sax != NULL) && (ctxt->sax->getParameterEntity != NULL))
		entity = ctxt->sax->getParameterEntity(ctxt->userData, name);
	    if (entity == NULL) {
	        
		/*
		 * [ WFC: Entity Declared ]
		 * In a document without any DTD, a document with only an
		 * internal DTD subset which contains no parameter entity
		 * references, or a document with "standalone='yes'", ...
		 * ... The declaration of a parameter entity must precede
		 * any reference to it...
		 */
		if ((ctxt->standalone == 1) ||
		    ((ctxt->hasExternalSubset == 0) &&
		     (ctxt->hasPErefs == 0))) {
		    xmlFatalErrMsgStr(ctxt, XML_ERR_UNDECLARED_ENTITY,
			 "PEReference: %%%s; not found\n", name);
	        } else {
		    /*
		     * [ VC: Entity Declared ]
		     * In a document with an external subset or external
		     * parameter entities with "standalone='no'", ...
		     * ... The declaration of a parameter entity must precede
		     * any reference to it...
		     */
		 
		        xmlWarningMsg(ctxt, XML_WAR_UNDECLARED_ENTITY,
			              "PEReference: %%%s; not found\n",
				      name, NULL);
		    ctxt->valid = 0;
		}
	    } else if (ctxt->input->free != deallocblankswrapper) {
		    input = xmlNewBlanksWrapperInputStream(ctxt, entity);
		    hw_xmlPushInput(ctxt, input);
	    } else {
	        if ((entity->etype == XML_INTERNAL_PARAMETER_ENTITY) ||
		    (entity->etype == XML_EXTERNAL_PARAMETER_ENTITY)) {
		    hw_xmlChar start[4];
		    hw_xmlCharEncoding enc;

		    /*
		     * handle the extra spaces added before and after
		     * c.f. http://www.w3.org/TR/REC-xml#as-PE
		     * this is done independently.
		     */
		    input = hw_xmlNewEntityInputStream(ctxt, entity);
		    hw_xmlPushInput(ctxt, input);

		    /* 
		     * Get the 4 first bytes and decode the charset
		     * if enc != XML_CHAR_ENCODING_NONE
		     * plug some encoding conversion routines.
		     * Note that, since we may have some non-UTF8
		     * encoding (like UTF16, bug 135229), the 'length'
		     * is not known, but we can calculate based upon
		     * the amount of data in the buffer.
		     */
		    GROW
		    if ((ctxt->input->end - ctxt->input->cur)>=4) {
			start[0] = RAW;
			start[1] = NXT(1);
			start[2] = NXT(2);
			start[3] = NXT(3);
			enc = hw_xmlDetectCharEncoding(start, 4);
			if (enc != XML_CHAR_ENCODING_NONE) {
			    hw_xmlSwitchEncoding(ctxt, enc);
			}
		    }

		    if ((entity->etype == XML_EXTERNAL_PARAMETER_ENTITY) &&
			(CMP5(CUR_PTR, '<', '?', 'x', 'm', 'l' )) &&
			(hw_IS_BLANK_CH(NXT(5)))) {
			hw_xmlParseTextDecl(ctxt);
		    }
		} else {
		    xmlFatalErrMsgStr(ctxt, XML_ERR_ENTITY_IS_PARAMETER,
			     "PEReference: %s is not a parameter entity\n",
				      name);
		}
	    }
	} else {
	    xmlFatalErr(ctxt, XML_ERR_PEREF_SEMICOL_MISSING, NULL);
	}
    }
}

/*
 * Macro used to grow the current buffer.
 */
#define growBuffer(buffer) {						\
    hw_xmlChar *tmp;							\
    buffer##_size *= 2;							\
    tmp = (hw_xmlChar *)							\
    		hw_xmlRealloc(buffer, buffer##_size * sizeof(hw_xmlChar));	\
    if (tmp == NULL) goto mem_error;					\
    buffer = tmp;							\
}

/**
 * hw_xmlStringLenDecodeEntities:
 * @ctxt:  the parser context
 * @str:  the input string
 * @len: the string length
 * @what:  combination of hw_XML_SUBSTITUTE_REF and hw_XML_SUBSTITUTE_PEREF
 * @end:  an end marker hw_xmlChar, 0 if none
 * @end2:  an end marker hw_xmlChar, 0 if none
 * @end3:  an end marker hw_xmlChar, 0 if none
 * 
 * Takes a entity string content and process to do the adequate substitutions.
 *
 * [67] Reference ::= EntityRef | CharRef
 *
 * [69] PEReference ::= '%' Name ';'
 *
 * Returns A newly allocated string with the substitution done. The caller
 *      must deallocate it !
 */
hw_xmlChar *
hw_xmlStringLenDecodeEntities(hw_xmlParserCtxtPtr ctxt, const hw_xmlChar *str, int len,
		      int what, hw_xmlChar end, hw_xmlChar  end2, hw_xmlChar end3) {
    hw_xmlChar *buffer = NULL;
    int buffer_size = 0;

    hw_xmlChar *current = NULL;
    const hw_xmlChar *last;
    hw_xmlEntityPtr ent;
    int c,l;
    int nbchars = 0;

    if ((ctxt == NULL) || (str == NULL) || (len < 0))
	return(NULL);
    last = str + len;

    if (ctxt->depth > 40) {
	xmlFatalErr(ctxt, XML_ERR_ENTITY_LOOP, NULL);
	return(NULL);
    }

    /*
     * allocate a translation buffer.
     */
    buffer_size = XML_PARSER_BIG_BUFFER_SIZE;
    buffer = (hw_xmlChar *) hw_xmlMallocAtomic(buffer_size * sizeof(hw_xmlChar));
    if (buffer == NULL) goto mem_error;

    /*
     * OK loop until we reach one of the ending char or a size limit.
     * we are operating on already parsed values.
     */
    if (str < last)
	c = CUR_SCHAR(str, l);
    else
        c = 0;
    while ((c != 0) && (c != end) && /* non input consuming loop */
	   (c != end2) && (c != end3)) {

	if (c == 0) break;
        if ((c == '&') && (str[1] == '#')) {
	    int val = xmlParseStringCharRef(ctxt, &str);
	    if (val != 0) {
		COPY_BUF(0,buffer,nbchars,val);
	    }
	    if (nbchars > buffer_size - XML_PARSER_BUFFER_SIZE) {
	        growBuffer(buffer);
	    }
	} else if ((c == '&') && (what & hw_XML_SUBSTITUTE_REF)) {

	    ent = xmlParseStringEntityRef(ctxt, &str);
	    if ((ent != NULL) &&
		(ent->etype == XML_INTERNAL_PREDEFINED_ENTITY)) {
		if (ent->content != NULL) {
		    COPY_BUF(0,buffer,nbchars,ent->content[0]);
		    if (nbchars > buffer_size - XML_PARSER_BUFFER_SIZE) {
			growBuffer(buffer);
		    }
		} else {
		    xmlFatalErrMsg(ctxt, XML_ERR_INTERNAL_ERROR,
			    "predefined entity has no content\n");
		}
	    } else if ((ent != NULL) && (ent->content != NULL)) {
		hw_xmlChar *rep;

		ctxt->depth++;
		rep = hw_xmlStringDecodeEntities(ctxt, ent->content, what,
			                      0, 0, 0);
		ctxt->depth--;
		if (rep != NULL) {
		    current = rep;
		    while (*current != 0) { /* non input consuming loop */
			buffer[nbchars++] = *current++;
			if (nbchars >
		            buffer_size - XML_PARSER_BUFFER_SIZE) {
			    growBuffer(buffer);
			}
		    }
		    hw_xmlFree(rep);
		}
	    } else if (ent != NULL) {
		int i = hw_xmlStrlen(ent->name);
		const hw_xmlChar *cur = ent->name;

		buffer[nbchars++] = '&';
		if (nbchars > buffer_size - i - XML_PARSER_BUFFER_SIZE) {
		    growBuffer(buffer);
		}
		for (;i > 0;i--)
		    buffer[nbchars++] = *cur++;
		buffer[nbchars++] = ';';
	    }
	} else if (c == '%' && (what & hw_XML_SUBSTITUTE_PEREF)) {

	    ent = xmlParseStringPEReference(ctxt, &str);
	    if (ent != NULL) {
		hw_xmlChar *rep;

		ctxt->depth++;
		rep = hw_xmlStringDecodeEntities(ctxt, ent->content, what,
			                      0, 0, 0);
		ctxt->depth--;
		if (rep != NULL) {
		    current = rep;
		    while (*current != 0) { /* non input consuming loop */
			buffer[nbchars++] = *current++;
			if (nbchars >
		            buffer_size - XML_PARSER_BUFFER_SIZE) {
			    growBuffer(buffer);
			}
		    }
		    hw_xmlFree(rep);
		}
	    }
	} else {
	    COPY_BUF(l,buffer,nbchars,c);
	    str += l;
	    if (nbchars > buffer_size - XML_PARSER_BUFFER_SIZE) {
	      growBuffer(buffer);
	    }
	}
	if (str < last)
	    c = CUR_SCHAR(str, l);
	else
	    c = 0;
    }
    buffer[nbchars++] = 0;
    return(buffer);

mem_error:
    hw_xmlErrMemory(ctxt, NULL);
    return(NULL);
}

/**
 * hw_xmlStringDecodeEntities:
 * @ctxt:  the parser context
 * @str:  the input string
 * @what:  combination of hw_XML_SUBSTITUTE_REF and hw_XML_SUBSTITUTE_PEREF
 * @end:  an end marker hw_xmlChar, 0 if none
 * @end2:  an end marker hw_xmlChar, 0 if none
 * @end3:  an end marker hw_xmlChar, 0 if none
 * 
 * Takes a entity string content and process to do the adequate substitutions.
 *
 * [67] Reference ::= EntityRef | CharRef
 *
 * [69] PEReference ::= '%' Name ';'
 *
 * Returns A newly allocated string with the substitution done. The caller
 *      must deallocate it !
 */
hw_xmlChar *
hw_xmlStringDecodeEntities(hw_xmlParserCtxtPtr ctxt, const hw_xmlChar *str, int what,
		        hw_xmlChar end, hw_xmlChar  end2, hw_xmlChar end3) {
    if ((ctxt == NULL) || (str == NULL)) return(NULL);
    return(hw_xmlStringLenDecodeEntities(ctxt, str, hw_xmlStrlen(str), what,
           end, end2, end3));
}

/************************************************************************
 *									*
 *		Commodity functions, cleanup needed ?			*
 *									*
 ************************************************************************/

/**
 * areBlanks:
 * @ctxt:  an XML parser context
 * @str:  a hw_xmlChar *
 * @len:  the size of @str
 * @blank_chars: we know the chars are blanks
 *
 * Is this a sequence of blank chars that one can ignore ?
 *
 * Returns 1 if ignorable 0 otherwise.
 */

static int areBlanks(hw_xmlParserCtxtPtr ctxt, const hw_xmlChar *str, int len,
                     int blank_chars) {
    int i, ret;
    hw_xmlNodePtr lastChild;

    /*
     * Don't spend time trying to differentiate them, the same callback is
     * used !
     */
    if (ctxt->sax->ignorableWhitespace == ctxt->sax->characters)
	return(0);

    /*
     * Check for xml:space value.
     */
    if (*(ctxt->space) == 1)
	return(0);

    /*
     * Check that the string is made of blanks
     */
    if (blank_chars == 0) {
	for (i = 0;i < len;i++)
	    if (!(hw_IS_BLANK_CH(str[i]))) return(0);
    }

    /*
     * Look if the element is mixed content in the DTD if available
     */
    if (ctxt->node == NULL) return(0);
    if (ctxt->myDoc != NULL) {
	ret = hw_xmlIsMixedElement(ctxt->myDoc, ctxt->node->name);
        if (ret == 0) return(1);
        if (ret == 1) return(0);
    }

    /*
     * Otherwise, heuristic :-\
     */
    if ((RAW != '<') && (RAW != 0xD)) return(0);
    if ((ctxt->node->children == NULL) &&
	(RAW == '<') && (NXT(1) == '/')) return(0);

    lastChild = hw_xmlGetLastChild(ctxt->node);
    if (lastChild == NULL) {
        if ((ctxt->node->type != XML_ELEMENT_NODE) &&
            (ctxt->node->content != NULL)) return(0);
    } else if (hw_xmlNodeIsText(lastChild))
        return(0);
    else if ((ctxt->node->children != NULL) &&
             (hw_xmlNodeIsText(ctxt->node->children)))
        return(0);
    return(1);
}


/************************************************************************
 *									*
 *			The parser itself				*
 *	Relates to http://www.w3.org/TR/REC-xml				*
 *									*
 ************************************************************************/

static const hw_xmlChar * xmlParseNameComplex(hw_xmlParserCtxtPtr ctxt);
static hw_xmlChar * xmlParseAttValueInternal(hw_xmlParserCtxtPtr ctxt,
                                          int *len, int *alloc, int normalize);

/**
 * hw_xmlParseName:
 * @ctxt:  an XML parser context
 *
 * parse an XML name.
 *
 * [4] NameChar ::= Letter | Digit | '.' | '-' | '_' | ':' |
 *                  CombiningChar | Extender
 *
 * [5] Name ::= (Letter | '_' | ':') (NameChar)*
 *
 * [6] Names ::= Name (#x20 Name)*
 *
 * Returns the Name parsed or NULL
 */

const hw_xmlChar *
hw_xmlParseName(hw_xmlParserCtxtPtr ctxt) {
    const hw_xmlChar *in;
    const hw_xmlChar *ret;
    int count = 0;

    GROW;

    /*
     * Accelerator for simple ASCII names
     */
    in = ctxt->input->cur;
    if (((*in >= 0x61) && (*in <= 0x7A)) ||
	((*in >= 0x41) && (*in <= 0x5A)) ||
	(*in == '_') || (*in == ':')) {
	in++;
	while (((*in >= 0x61) && (*in <= 0x7A)) ||
	       ((*in >= 0x41) && (*in <= 0x5A)) ||
	       ((*in >= 0x30) && (*in <= 0x39)) ||
	       (*in == '_') || (*in == '-') ||
	       (*in == ':') || (*in == '.'))
	    in++;
	if ((*in > 0) && (*in < 0x80)) {
	    count = in - ctxt->input->cur;
	    ret = hw_xmlDictLookup(ctxt->dict, ctxt->input->cur, count);
	    ctxt->input->cur = in;
	    ctxt->nbChars += count;
	    ctxt->input->col += count;
	    if (ret == NULL)
	        hw_xmlErrMemory(ctxt, NULL);
	    return(ret);
	}
    }
    return(xmlParseNameComplex(ctxt));
}

/**
 * xmlParseNameAndCompare:
 * @ctxt:  an XML parser context
 *
 * parse an XML name and compares for match
 * (specialized for endtag parsing)
 *
 * Returns NULL for an illegal name, (hw_xmlChar*) 1 for success
 * and the name for mismatch
 */

static const hw_xmlChar *
xmlParseNameAndCompare(hw_xmlParserCtxtPtr ctxt, hw_xmlChar const *other) {
    register const hw_xmlChar *cmp = other;
    register const hw_xmlChar *in;
    const hw_xmlChar *ret;

    GROW;
    
    in = ctxt->input->cur;
    while (*in != 0 && *in == *cmp) {
    	++in;
	++cmp;
	ctxt->input->col++;
    }
    if (*cmp == 0 && (*in == '>' || hw_IS_BLANK_CH (*in))) {
    	/* success */
	ctxt->input->cur = in;
	return (const hw_xmlChar*) 1;
    }
    /* failure (or end of input buffer), check with full function */
    ret = hw_xmlParseName (ctxt);
    /* strings coming from the dictionnary direct compare possible */
    if (ret == other) {
	return (const hw_xmlChar*) 1;
    }
    return ret;
}

static const hw_xmlChar *
xmlParseNameComplex(hw_xmlParserCtxtPtr ctxt) {
    int len = 0, l;
    int c;
    int count = 0;

    /*
     * Handler for more complex cases
     */
    GROW;
    c = CUR_CHAR(l);
    if ((c == ' ') || (c == '>') || (c == '/') || /* accelerators */
	(!hw_IS_LETTER(c) && (c != '_') &&
         (c != ':'))) {
	return(NULL);
    }

    while ((c != ' ') && (c != '>') && (c != '/') && /* test bigname.xml */
	   ((hw_IS_LETTER(c)) || (hw_IS_DIGIT(c)) ||
            (c == '.') || (c == '-') ||
	    (c == '_') || (c == ':') || 
	    (hw_IS_COMBINING(c)) ||
	    (hw_IS_EXTENDER(c)))) {
	if (count++ > 100) {
	    count = 0;
	    GROW;
	}
	len += l;
	NEXTL(l);
	c = CUR_CHAR(l);
    }
    if ((*ctxt->input->cur == '\n') && (ctxt->input->cur[-1] == '\r'))
        return(hw_xmlDictLookup(ctxt->dict, ctxt->input->cur - (len + 1), len));
    return(hw_xmlDictLookup(ctxt->dict, ctxt->input->cur - len, len));
}

/**
 * xmlParseStringName:
 * @ctxt:  an XML parser context
 * @str:  a pointer to the string pointer (IN/OUT)
 *
 * parse an XML name.
 *
 * [4] NameChar ::= Letter | Digit | '.' | '-' | '_' | ':' |
 *                  CombiningChar | Extender
 *
 * [5] Name ::= (Letter | '_' | ':') (NameChar)*
 *
 * [6] Names ::= Name (#x20 Name)*
 *
 * Returns the Name parsed or NULL. The @str pointer 
 * is updated to the current location in the string.
 */

static hw_xmlChar *
xmlParseStringName(hw_xmlParserCtxtPtr ctxt, const hw_xmlChar** str) {
    hw_xmlChar buf[hw_XML_MAX_NAMELEN + 5];
    const hw_xmlChar *cur = *str;
    int len = 0, l;
    int c;

    c = CUR_SCHAR(cur, l);
    if (!hw_IS_LETTER(c) && (c != '_') &&
        (c != ':')) {
	return(NULL);
    }

    while ((hw_IS_LETTER(c)) || (hw_IS_DIGIT(c)) || /* test bigentname.xml */
           (c == '.') || (c == '-') ||
	   (c == '_') || (c == ':') || 
	   (hw_IS_COMBINING(c)) ||
	   (hw_IS_EXTENDER(c))) {
	COPY_BUF(l,buf,len,c);
	cur += l;
	c = CUR_SCHAR(cur, l);
	if (len >= hw_XML_MAX_NAMELEN) { /* test bigentname.xml */
	    /*
	     * Okay someone managed to make a huge name, so he's ready to pay
	     * for the processing speed.
	     */
	    hw_xmlChar *buffer;
	    int max = len * 2;
	    
	    buffer = (hw_xmlChar *) hw_xmlMallocAtomic(max * sizeof(hw_xmlChar));
	    if (buffer == NULL) {
	        hw_xmlErrMemory(ctxt, NULL);
		return(NULL);
	    }
	    memcpy(buffer, buf, len);
	    while ((hw_IS_LETTER(c)) || (hw_IS_DIGIT(c)) ||
	             /* test bigentname.xml */
		   (c == '.') || (c == '-') ||
		   (c == '_') || (c == ':') || 
		   (hw_IS_COMBINING(c)) ||
		   (hw_IS_EXTENDER(c))) {
		if (len + 10 > max) {
		    hw_xmlChar *tmp;
		    max *= 2;
		    tmp = (hw_xmlChar *) hw_xmlRealloc(buffer,
			                            max * sizeof(hw_xmlChar));
		    if (tmp == NULL) {
			hw_xmlErrMemory(ctxt, NULL);
			hw_xmlFree(buffer);
			return(NULL);
		    }
		    buffer = tmp;
		}
		COPY_BUF(l,buffer,len,c);
		cur += l;
		c = CUR_SCHAR(cur, l);
	    }
	    buffer[len] = 0;
	    *str = cur;
	    return(buffer);
	}
    }
    *str = cur;
    return(hw_xmlStrndup(buf, len));
}

/**
 * xmlParseAttValueComplex:
 * @ctxt:  an XML parser context
 * @len:   the resulting attribute len
 * @normalize:  wether to apply the inner normalization
 *
 * parse a value for an attribute, this is the fallback function
 * of non-ASCII characters, or normalization compaction.
 *
 * Returns the AttValue parsed or NULL. The value has to be freed by the caller.
 */
static hw_xmlChar *
xmlParseAttValueComplex(hw_xmlParserCtxtPtr ctxt, int *attlen, int normalize) {
    hw_xmlChar limit = 0;
    hw_xmlChar *buf = NULL;
    int len = 0;
    int buf_size = 0;
    int c, l, in_space = 0;
    hw_xmlChar *current = NULL;
    hw_xmlEntityPtr ent;

    if (NXT(0) == '"') {
	ctxt->instate = XML_PARSER_ATTRIBUTE_VALUE;
	limit = '"';
        NEXT;
    } else if (NXT(0) == '\'') {
	limit = '\'';
	ctxt->instate = XML_PARSER_ATTRIBUTE_VALUE;
        NEXT;
    } else {
	xmlFatalErr(ctxt, XML_ERR_ATTRIBUTE_NOT_STARTED, NULL);
	return(NULL);
    }
    
    /*
     * allocate a translation buffer.
     */
    buf_size = XML_PARSER_BUFFER_SIZE;
    buf = (hw_xmlChar *) hw_xmlMallocAtomic(buf_size * sizeof(hw_xmlChar));
    if (buf == NULL) goto mem_error;

    /*
     * OK loop until we reach one of the ending char or a size limit.
     */
    c = CUR_CHAR(l);
    while ((NXT(0) != limit) && /* checked */
	   (c != '<')) {
	if (c == 0) break;
	if (c == '&') {
	    in_space = 0;
	    {
		ent = hw_xmlParseEntityRef(ctxt);
		if ((ent != NULL) &&
		    (ent->etype == XML_INTERNAL_PREDEFINED_ENTITY)) {
		    if (len > buf_size - 10) {
			growBuffer(buf);
		    }
		    if ((ctxt->replaceEntities == 0) &&
		        (ent->content[0] == '&')) {
			buf[len++] = '&';
			buf[len++] = '#';
			buf[len++] = '3';
			buf[len++] = '8';
			buf[len++] = ';';
		    } else {
			buf[len++] = ent->content[0];
		    }
		} else if ((ent != NULL) && 
		           (ctxt->replaceEntities != 0)) {
		    hw_xmlChar *rep;

		    if (ent->etype != XML_INTERNAL_PREDEFINED_ENTITY) {
			rep = hw_xmlStringDecodeEntities(ctxt, ent->content,
						      hw_XML_SUBSTITUTE_REF,
						      0, 0, 0);
			if (rep != NULL) {
			    current = rep;
			    while (*current != 0) { /* non input consuming */
				buf[len++] = *current++;
				if (len > buf_size - 10) {
				    growBuffer(buf);
				}
			    }
			    hw_xmlFree(rep);
			}
		    } else {
			if (len > buf_size - 10) {
			    growBuffer(buf);
			}
			if (ent->content != NULL)
			    buf[len++] = ent->content[0];
		    }
		} else if (ent != NULL) {
		    int i = hw_xmlStrlen(ent->name);
		    const hw_xmlChar *cur = ent->name;

		    /*
		     * This may look absurd but is needed to detect
		     * entities problems
		     */
		    if ((ent->etype != XML_INTERNAL_PREDEFINED_ENTITY) &&
			(ent->content != NULL)) {
			hw_xmlChar *rep;
			rep = hw_xmlStringDecodeEntities(ctxt, ent->content,
						      hw_XML_SUBSTITUTE_REF, 0, 0, 0);
			if (rep != NULL)
			    hw_xmlFree(rep);
		    }

		    /*
		     * Just output the reference
		     */
		    buf[len++] = '&';
		    if (len > buf_size - i - 10) {
			growBuffer(buf);
		    }
		    for (;i > 0;i--)
			buf[len++] = *cur++;
		    buf[len++] = ';';
		}
	    }
	} else {
	    if ((c == 0x20) || (c == 0xD) || (c == 0xA) || (c == 0x9)) {
	        if ((len != 0) || (!normalize)) {
		    if ((!normalize) || (!in_space)) {
			COPY_BUF(l,buf,len,0x20);
			if (len > buf_size - 10) {
			    growBuffer(buf);
			}
		    }
		    in_space = 1;
		}
	    } else {
	        in_space = 0;
		COPY_BUF(l,buf,len,c);
		if (len > buf_size - 10) {
		    growBuffer(buf);
		}
	    }
	    NEXTL(l);
	}
	GROW;
	c = CUR_CHAR(l);
    }
    if ((in_space) && (normalize)) {
        while (buf[len - 1] == 0x20) len--;
    }
    buf[len] = 0;
    if (RAW == '<') {
	xmlFatalErr(ctxt, XML_ERR_LT_IN_ATTRIBUTE, NULL);
    } else if (RAW != limit) {
        xmlFatalErrMsg(ctxt, XML_ERR_ATTRIBUTE_NOT_FINISHED,
	               "AttValue: ' expected\n");
    } else
	NEXT;
    if (attlen != NULL) *attlen = len;
    return(buf);

mem_error:
    hw_xmlErrMemory(ctxt, NULL);
    return(NULL);
}

void xmlParseCharDataComplex(hw_xmlParserCtxtPtr ctxt, int cdata);

/*
 * used for the test in the inner loop of the char data testing
 */
static const unsigned char test_char_data[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x9, CR/LF separated */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x00, 0x27, /* & */
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x00, 0x3D, 0x3E, 0x3F, /* < */
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x00, 0x5E, 0x5F, /* ] */
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* non-ascii */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/**
 * hw_xmlParseCharData:
 * @ctxt:  an XML parser context
 * @cdata:  int indicating whether we are within a CDATA section
 *
 * parse a CharData section.
 * if we are within a CDATA section ']]>' marks an end of section.
 *
 * The right angle bracket (>) may be represented using the string "&gt;",
 * and must, for compatibility, be escaped using "&gt;" or a character
 * reference when it appears in the string "]]>" in content, when that
 * string is not marking the end of a CDATA section. 
 *
 * [14] CharData ::= [^<&]* - ([^<&]* ']]>' [^<&]*)
 */

void
hw_xmlParseCharData(hw_xmlParserCtxtPtr ctxt, int cdata) {
    const hw_xmlChar *in;
    int nbchar = 0;
    int line = ctxt->input->line;
    int col = ctxt->input->col;
    int ccol;

    SHRINK;
    GROW;
    /*
     * Accelerated common case where input don't need to be
     * modified before passing it to the handler.
     */
    if (!cdata) {
	in = ctxt->input->cur;
	do {
get_more_space:
	    while (*in == 0x20) in++;
	    if (*in == 0xA) {
		do {
		    ctxt->input->line++; ctxt->input->col = 1;
		    in++;
		} while (*in == 0xA);
		goto get_more_space;
	    }
	    if (*in == '<') {
		nbchar = in - ctxt->input->cur;
		if (nbchar > 0) {
		    const hw_xmlChar *tmp = ctxt->input->cur;
		    ctxt->input->cur = in;

		    if ((ctxt->sax != NULL) &&
		        (ctxt->sax->ignorableWhitespace !=
		         ctxt->sax->characters)) {
			if (areBlanks(ctxt, tmp, nbchar, 1)) {
			    if (ctxt->sax->ignorableWhitespace != NULL)
				ctxt->sax->ignorableWhitespace(ctxt->userData,
						       tmp, nbchar);
			} else if (ctxt->sax->characters != NULL)
			    ctxt->sax->characters(ctxt->userData,
						  tmp, nbchar);
		    } else if ((ctxt->sax != NULL) &&
		               (ctxt->sax->characters != NULL)) {
			ctxt->sax->characters(ctxt->userData,
					      tmp, nbchar);
		    }
		}
		return;
	    }

get_more:
            ccol = ctxt->input->col;
	    while (test_char_data[*in]) {
		in++;
		ccol++;
	    }
	    ctxt->input->col = ccol;
	    if (*in == 0xA) {
		do {
		    ctxt->input->line++; ctxt->input->col = 1;
		    in++;
		} while (*in == 0xA);
		goto get_more;
	    }
	    if (*in == ']') {
		if ((in[1] == ']') && (in[2] == '>')) {
		    xmlFatalErr(ctxt, XML_ERR_MISPLACED_CDATA_END, NULL);
		    ctxt->input->cur = in;
		    return;
		}
		in++;
		ctxt->input->col++;
		goto get_more;
	    }
	    nbchar = in - ctxt->input->cur;
	    if (nbchar > 0) {
		if ((ctxt->sax != NULL) &&
		    (ctxt->sax->ignorableWhitespace !=
		     ctxt->sax->characters) &&
		    (hw_IS_BLANK_CH(*ctxt->input->cur))) {
		    const hw_xmlChar *tmp = ctxt->input->cur;
		    ctxt->input->cur = in;

		    if (areBlanks(ctxt, tmp, nbchar, 0)) {
		        if (ctxt->sax->ignorableWhitespace != NULL)
			    ctxt->sax->ignorableWhitespace(ctxt->userData,
							   tmp, nbchar);
		    } else if (ctxt->sax->characters != NULL)
			ctxt->sax->characters(ctxt->userData,
					      tmp, nbchar);
                    line = ctxt->input->line;
                    col = ctxt->input->col;
		} else if (ctxt->sax != NULL) {
		    if (ctxt->sax->characters != NULL)
			ctxt->sax->characters(ctxt->userData,
					      ctxt->input->cur, nbchar);
                    line = ctxt->input->line;
                    col = ctxt->input->col;
		}
	    }
	    ctxt->input->cur = in;
	    if (*in == 0xD) {
		in++;
		if (*in == 0xA) {
		    ctxt->input->cur = in;
		    in++;
		    ctxt->input->line++; ctxt->input->col = 1;
		    continue; /* while */
		}
		in--;
	    }
	    if (*in == '<') {
		return;
	    }
	    if (*in == '&') {
		return;
	    }
	    SHRINK;
	    GROW;
	    in = ctxt->input->cur;
	} while (((*in >= 0x20) && (*in <= 0x7F)) || (*in == 0x09));
	nbchar = 0;
    }
    ctxt->input->line = line;
    ctxt->input->col = col;
    xmlParseCharDataComplex(ctxt, cdata);
}

/**
 * xmlParseCharDataComplex:
 * @ctxt:  an XML parser context
 * @cdata:  int indicating whether we are within a CDATA section
 *
 * parse a CharData section.this is the fallback function
 * of hw_xmlParseCharData() when the parsing requires handling
 * of non-ASCII characters.
 */
void
xmlParseCharDataComplex(hw_xmlParserCtxtPtr ctxt, int cdata) {
    hw_xmlChar buf[XML_PARSER_BIG_BUFFER_SIZE + 5];
    int nbchar = 0;
    int cur, l;
    int count = 0;

    SHRINK;
    GROW;
    cur = CUR_CHAR(l);
    while ((cur != '<') && /* checked */
           (cur != '&') && 
	   (hw_IS_CHAR(cur))) /* test also done in hw_xmlCurrentChar() */ {
	if ((cur == ']') && (NXT(1) == ']') &&
	    (NXT(2) == '>')) {
	    if (cdata) break;
	    else {
		xmlFatalErr(ctxt, XML_ERR_MISPLACED_CDATA_END, NULL);
	    }
	}
	COPY_BUF(l,buf,nbchar,cur);
	if (nbchar >= XML_PARSER_BIG_BUFFER_SIZE) {
	    buf[nbchar] = 0;

	    /*
	     * OK the segment is to be consumed as chars.
	     */
	    if ((ctxt->sax != NULL) && (!ctxt->disableSAX)) {
		if (areBlanks(ctxt, buf, nbchar, 0)) {
		    if (ctxt->sax->ignorableWhitespace != NULL)
			ctxt->sax->ignorableWhitespace(ctxt->userData,
			                               buf, nbchar);
		} else {
		    if (ctxt->sax->characters != NULL)
			ctxt->sax->characters(ctxt->userData, buf, nbchar);
		}
	    }
	    nbchar = 0;
	}
	count++;
	if (count > 50) {
	    GROW;
	    count = 0;
	}
	NEXTL(l);
	cur = CUR_CHAR(l);
    }
    if (nbchar != 0) {
        buf[nbchar] = 0;
	/*
	 * OK the segment is to be consumed as chars.
	 */
	if ((ctxt->sax != NULL) && (!ctxt->disableSAX)) {
	    if (areBlanks(ctxt, buf, nbchar, 0)) {
		if (ctxt->sax->ignorableWhitespace != NULL)
		    ctxt->sax->ignorableWhitespace(ctxt->userData, buf, nbchar);
	    } else {
		if (ctxt->sax->characters != NULL)
		    ctxt->sax->characters(ctxt->userData, buf, nbchar);
	    }
	}
    }
    if ((cur != 0) && (!hw_IS_CHAR(cur))) {
	/* Generate the error and skip the offending character */
        xmlFatalErrMsgInt(ctxt, XML_ERR_INVALID_CHAR,
                          "PCDATA invalid Char value %d\n",
	                  cur);
	NEXTL(l);
    }
}
/**
 * xmlParseCommentComplex:
 * @ctxt:  an XML parser context
 * @buf:  the already parsed part of the buffer
 * @len:  number of bytes filles in the buffer
 * @size:  allocated size of the buffer
 *
 * Skip an XML (SGML) comment <!-- .... -->
 *  The spec says that "For compatibility, the string "--" (double-hyphen)
 *  must not occur within comments. "
 * This is the slow routine in case the accelerator for ascii didn't work
 *
 * [15] Comment ::= '<!--' ((Char - '-') | ('-' (Char - '-')))* '-->'
 */
static void
xmlParseCommentComplex(hw_xmlParserCtxtPtr ctxt, hw_xmlChar *buf, int len, int size) {
    int q, ql;
    int r, rl;
    int cur, l;
    hw_xmlParserInputPtr input = ctxt->input;
    int count = 0;

    if (buf == NULL) {
        len = 0;
	size = XML_PARSER_BUFFER_SIZE;
	buf = (hw_xmlChar *) hw_xmlMallocAtomic(size * sizeof(hw_xmlChar));
	if (buf == NULL) {
	    hw_xmlErrMemory(ctxt, NULL);
	    return;
	}
    }
    q = CUR_CHAR(ql);
    if (q == 0)
        goto not_terminated;
    NEXTL(ql);
    r = CUR_CHAR(rl);
    if (r == 0)
        goto not_terminated;
    NEXTL(rl);
    cur = CUR_CHAR(l);
    if (cur == 0)
        goto not_terminated;
    while (hw_IS_CHAR(cur) && /* checked */
           ((cur != '>') ||
	    (r != '-') || (q != '-'))) {
	if ((r == '-') && (q == '-')) {
	    xmlFatalErr(ctxt, XML_ERR_HYPHEN_IN_COMMENT, NULL);
	}
	if (len + 5 >= size) {
	    hw_xmlChar *new_buf;
	    size *= 2;
	    new_buf = (hw_xmlChar *) hw_xmlRealloc(buf, size * sizeof(hw_xmlChar));
	    if (new_buf == NULL) {
		hw_xmlFree (buf);
		hw_xmlErrMemory(ctxt, NULL);
		return;
	    }
	    buf = new_buf;
	}
	COPY_BUF(ql,buf,len,q);
	q = r;
	ql = rl;
	r = cur;
	rl = l;

	count++;
	if (count > 50) {
	    GROW;
	    count = 0;
	}
	NEXTL(l);
	cur = CUR_CHAR(l);
	if (cur == 0) {
	    SHRINK;
	    GROW;
	    cur = CUR_CHAR(l);
	}
    }
    buf[len] = 0;
    if (!hw_IS_CHAR(cur)) {
	xmlFatalErrMsgStr(ctxt, XML_ERR_COMMENT_NOT_FINISHED,
	                     "Comment not terminated \n<!--%.50s\n", buf);
	hw_xmlFree(buf);
    } else {
	if (input != ctxt->input) {
	    xmlFatalErrMsg(ctxt, XML_ERR_ENTITY_BOUNDARY,
		"Comment doesn't start and stop in the same entity\n");
	}
        NEXT;
	if ((ctxt->sax != NULL) && (ctxt->sax->comment != NULL) &&
	    (!ctxt->disableSAX))
	    ctxt->sax->comment(ctxt->userData, buf);
	hw_xmlFree(buf);
    }
    return;
not_terminated:
    xmlFatalErrMsgStr(ctxt, XML_ERR_COMMENT_NOT_FINISHED,
			 "Comment not terminated\n", NULL);
    hw_xmlFree(buf);
}
/**
 * hw_xmlParseComment:
 * @ctxt:  an XML parser context
 *
 * Skip an XML (SGML) comment <!-- .... -->
 *  The spec says that "For compatibility, the string "--" (double-hyphen)
 *  must not occur within comments. "
 *
 * [15] Comment ::= '<!--' ((Char - '-') | ('-' (Char - '-')))* '-->'
 */
void
hw_xmlParseComment(hw_xmlParserCtxtPtr ctxt) {
    hw_xmlChar *buf = NULL;
    int size = XML_PARSER_BUFFER_SIZE;
    int len = 0;
    hw_xmlParserInputState state;
    const hw_xmlChar *in;
    int nbchar = 0, ccol;

    /*
     * Check that there is a comment right here.
     */
    if ((RAW != '<') || (NXT(1) != '!') ||
        (NXT(2) != '-') || (NXT(3) != '-')) return;

    state = ctxt->instate;
    ctxt->instate = XML_PARSER_COMMENT;
    SKIP(4);
    SHRINK;
    GROW;

    /*
     * Accelerated common case where input don't need to be
     * modified before passing it to the handler.
     */
    in = ctxt->input->cur;
    do {
	if (*in == 0xA) {
	    do {
		ctxt->input->line++; ctxt->input->col = 1;
		in++;
	    } while (*in == 0xA);
	}
get_more:
        ccol = ctxt->input->col;
	while (((*in > '-') && (*in <= 0x7F)) ||
	       ((*in >= 0x20) && (*in < '-')) ||
	       (*in == 0x09)) {
		    in++;
		    ccol++;
	}
	ctxt->input->col = ccol;
	if (*in == 0xA) {
	    do {
		ctxt->input->line++; ctxt->input->col = 1;
		in++;
	    } while (*in == 0xA);
	    goto get_more;
	}
	nbchar = in - ctxt->input->cur;
	/*
	 * save current set of data
	 */
	if (nbchar > 0) {
	    if ((ctxt->sax != NULL) &&
		(ctxt->sax->comment != NULL)) {
		if (buf == NULL) {
		    if ((*in == '-') && (in[1] == '-'))
		        size = nbchar + 1;
		    else
		        size = XML_PARSER_BUFFER_SIZE + nbchar;
		    buf = (hw_xmlChar *) hw_xmlMallocAtomic(size * sizeof(hw_xmlChar));
		    if (buf == NULL) {
		        hw_xmlErrMemory(ctxt, NULL);
			ctxt->instate = state;
			return;
		    }
		    len = 0;
		} else if (len + nbchar + 1 >= size) {
		    hw_xmlChar *new_buf;
		    size  += len + nbchar + XML_PARSER_BUFFER_SIZE;
		    new_buf = (hw_xmlChar *) hw_xmlRealloc(buf,
		                                     size * sizeof(hw_xmlChar));
		    if (new_buf == NULL) {
		        hw_xmlFree (buf);
			hw_xmlErrMemory(ctxt, NULL);
			ctxt->instate = state;
			return;
		    }
		    buf = new_buf;
		}
		memcpy(&buf[len], ctxt->input->cur, nbchar);
		len += nbchar;
		buf[len] = 0;
	    }
	}
	ctxt->input->cur = in;
	if (*in == 0xA) {
	    in++;
	    ctxt->input->line++; ctxt->input->col = 1;
	}
	if (*in == 0xD) {
	    in++;
	    if (*in == 0xA) {
		ctxt->input->cur = in;
		in++;
		ctxt->input->line++; ctxt->input->col = 1;
		continue; /* while */
	    }
	    in--;
	}
	SHRINK;
	GROW;
	in = ctxt->input->cur;
	if (*in == '-') {
	    if (in[1] == '-') {
	        if (in[2] == '>') {
		    SKIP(3);
		    if ((ctxt->sax != NULL) && (ctxt->sax->comment != NULL) &&
		        (!ctxt->disableSAX)) {
			if (buf != NULL)
			    ctxt->sax->comment(ctxt->userData, buf);
			else
			    ctxt->sax->comment(ctxt->userData, hw_BAD_CAST "");
		    }
		    if (buf != NULL)
		        hw_xmlFree(buf);
		    ctxt->instate = state;
		    return;
		}
		if (buf != NULL)
		    xmlFatalErrMsgStr(ctxt, XML_ERR_COMMENT_NOT_FINISHED,
		                      "Comment not terminated \n<!--%.50s\n",
				      buf);
		else
		    xmlFatalErrMsgStr(ctxt, XML_ERR_COMMENT_NOT_FINISHED,
		                      "Comment not terminated \n", NULL);
		in++;
		ctxt->input->col++;
	    }
	    in++;
	    ctxt->input->col++;
	    goto get_more;
	}
    } while (((*in >= 0x20) && (*in <= 0x7F)) || (*in == 0x09));
    xmlParseCommentComplex(ctxt, buf, len, size);
    ctxt->instate = state;
    return;
}

/**
 * hw_xmlParseTextDecl:
 * @ctxt:  an XML parser context
 * 
 * parse an XML declaration header for external entities
 *
 * [77] TextDecl ::= '<?xml' VersionInfo? EncodingDecl S? '?>'
 *
 * Question: Seems that EncodingDecl is mandatory ? Is that a typo ?
 */

void
hw_xmlParseTextDecl(hw_xmlParserCtxtPtr ctxt) {
    hw_xmlChar *version;
    const hw_xmlChar *encoding;

    /*
     * We know that '<?xml' is here.
     */
    if ((CMP5(CUR_PTR, '<', '?', 'x', 'm', 'l')) && (hw_IS_BLANK_CH(NXT(5)))) {
	SKIP(5);
    } else {
	xmlFatalErr(ctxt, XML_ERR_XMLDECL_NOT_STARTED, NULL);
	return;
    }

    if (!hw_IS_BLANK_CH(CUR)) {
	xmlFatalErrMsg(ctxt, XML_ERR_SPACE_REQUIRED,
		       "Space needed after '<?xml'\n");
    }
    SKIP_BLANKS;

    /*
     * We may have the VersionInfo here.
     */
    version = hw_xmlParseVersionInfo(ctxt);
    if (version == NULL)
	version = hw_xmlCharStrdup(hw_XML_DEFAULT_VERSION);
    else {
	if (!hw_IS_BLANK_CH(CUR)) {
	    xmlFatalErrMsg(ctxt, XML_ERR_SPACE_REQUIRED,
		           "Space needed here\n");
	}
    }
    ctxt->input->version = version;

    /*
     * We must have the encoding declaration
     */
    encoding = hw_xmlParseEncodingDecl(ctxt);
    if (ctxt->errNo == XML_ERR_UNSUPPORTED_ENCODING) {
	/*
	 * The XML REC instructs us to stop parsing right here
	 */
        return;
    }
    if ((encoding == NULL) && (ctxt->errNo == XML_ERR_OK)) {
	xmlFatalErrMsg(ctxt, XML_ERR_MISSING_ENCODING,
		       "Missing encoding in text declaration\n");
    }

    SKIP_BLANKS;
    if ((RAW == '?') && (NXT(1) == '>')) {
        SKIP(2);
    } else if (RAW == '>') {
        /* Deprecated old WD ... */
	xmlFatalErr(ctxt, XML_ERR_XMLDECL_NOT_FINISHED, NULL);
	NEXT;
    } else {
	xmlFatalErr(ctxt, XML_ERR_XMLDECL_NOT_FINISHED, NULL);
	hw_MOVETO_ENDTAG(CUR_PTR);
	NEXT;
    }
}

/**
 * hw_xmlParseEntityRef:
 * @ctxt:  an XML parser context
 *
 * parse ENTITY references declarations
 *
 * [68] EntityRef ::= '&' Name ';'
 *
 * [ WFC: Entity Declared ]
 * In a document without any DTD, a document with only an internal DTD
 * subset which contains no parameter entity references, or a document
 * with "standalone='yes'", the Name given in the entity reference
 * must match that in an entity declaration, except that well-formed
 * documents need not declare any of the following entities: amp, lt,
 * gt, apos, quot.  The declaration of a parameter entity must precede
 * any reference to it.  Similarly, the declaration of a general entity
 * must precede any reference to it which appears in a default value in an
 * attribute-list declaration. Note that if entities are declared in the
 * external subset or in external parameter entities, a non-validating
 * processor is not obligated to read and process their declarations;
 * for such documents, the rule that an entity must be declared is a
 * well-formedness constraint only if standalone='yes'.
 *
 * [ WFC: Parsed Entity ]
 * An entity reference must not contain the name of an unparsed entity
 *
 * Returns the hw_xmlEntityPtr if found, or NULL otherwise.
 */
hw_xmlEntityPtr
hw_xmlParseEntityRef(hw_xmlParserCtxtPtr ctxt) {
    const hw_xmlChar *name;
    hw_xmlEntityPtr ent = NULL;

    GROW;
    
    if (RAW == '&') {
        NEXT;
        name = hw_xmlParseName(ctxt);
	if (name == NULL) {
	    xmlFatalErrMsg(ctxt, XML_ERR_NAME_REQUIRED,
			   "hw_xmlParseEntityRef: no name\n");
	} else {
	    if (RAW == ';') {
	        NEXT;
		/*
		 * Ask first SAX for entity resolution, otherwise try the
		 * predefined set.
		 */
		if (ctxt->sax != NULL) {
		    if (ctxt->sax->getEntity != NULL)
			ent = ctxt->sax->getEntity(ctxt->userData, name);
		    if ((ctxt->wellFormed == 1 ) && (ent == NULL))
		        ent = hw_xmlGetPredefinedEntity(name);
		    if ((ctxt->wellFormed == 1 ) && (ent == NULL) &&
			(ctxt->userData==ctxt)) {
			ent = hw_xmlSAX2GetEntity(ctxt, name);
		    }
		}
		/*
		 * [ WFC: Entity Declared ]
		 * In a document without any DTD, a document with only an
		 * internal DTD subset which contains no parameter entity
		 * references, or a document with "standalone='yes'", the
		 * Name given in the entity reference must match that in an
		 * entity declaration, except that well-formed documents
		 * need not declare any of the following entities: amp, lt,
		 * gt, apos, quot.
		 * The declaration of a parameter entity must precede any
		 * reference to it.
		 * Similarly, the declaration of a general entity must
		 * precede any reference to it which appears in a default
		 * value in an attribute-list declaration. Note that if
		 * entities are declared in the external subset or in
		 * external parameter entities, a non-validating processor
		 * is not obligated to read and process their declarations;
		 * for such documents, the rule that an entity must be
		 * declared is a well-formedness constraint only if
		 * standalone='yes'. 
		 */
		if (ent == NULL) {
		    if ((ctxt->standalone == 1) ||
		        ((ctxt->hasExternalSubset == 0) &&
			 (ctxt->hasPErefs == 0))) {
			xmlFatalErrMsgStr(ctxt, XML_ERR_UNDECLARED_ENTITY,
				 "Entity '%s' not defined\n", name);
		    } else {
		        xmlErrMsgStr(ctxt, XML_WAR_UNDECLARED_ENTITY,
				 "Entity '%s' not defined\n", name);
			if ((ctxt->inSubset == 0) &&
		            (ctxt->sax != NULL) &&
		            (ctxt->sax->reference != NULL)) {
			    ctxt->sax->reference(ctxt, name);
			}
		    }
		    ctxt->valid = 0;
		}

		/*
		 * [ WFC: Parsed Entity ]
		 * An entity reference must not contain the name of an
		 * unparsed entity
		 */
		else if (ent->etype == XML_EXTERNAL_GENERAL_UNPARSED_ENTITY) {
		    xmlFatalErrMsgStr(ctxt, XML_ERR_UNPARSED_ENTITY,
			     "Entity reference to unparsed entity %s\n", name);
		}

		/*
		 * [ WFC: No External Entity References ]
		 * Attribute values cannot contain direct or indirect
		 * entity references to external entities.
		 */
		else if ((ctxt->instate == XML_PARSER_ATTRIBUTE_VALUE) &&
		         (ent->etype == XML_EXTERNAL_GENERAL_PARSED_ENTITY)) {
		    xmlFatalErrMsgStr(ctxt, XML_ERR_ENTITY_IS_EXTERNAL,
			 "Attribute references external entity '%s'\n", name);
		}
		/*
		 * [ WFC: No < in Attribute Values ]
		 * The replacement text of any entity referred to directly or
		 * indirectly in an attribute value (other than "&lt;") must
		 * not contain a <. 
		 */
		else if ((ctxt->instate == XML_PARSER_ATTRIBUTE_VALUE) &&
		         (ent != NULL) &&
			 (!hw_xmlStrEqual(ent->name, hw_BAD_CAST "lt")) &&
		         (ent->content != NULL) &&
			 (hw_xmlStrchr(ent->content, '<'))) {
		    xmlFatalErrMsgStr(ctxt, XML_ERR_LT_IN_ATTRIBUTE,
	 "'<' in entity '%s' is not allowed in attributes values\n", name);
		}

		/*
		 * Internal check, no parameter entities here ...
		 */
		else {
		    switch (ent->etype) {
			case XML_INTERNAL_PARAMETER_ENTITY:
			case XML_EXTERNAL_PARAMETER_ENTITY:
			xmlFatalErrMsgStr(ctxt, XML_ERR_ENTITY_IS_PARAMETER,
			 "Attempt to reference the parameter entity '%s'\n",
		                          name);
			break;
			default:
			break;
		    }
		}

		/*
		 * [ WFC: No Recursion ]
		 * A parsed entity must not contain a recursive reference
		 * to itself, either directly or indirectly. 
		 * Done somewhere else
		 */

	    } else {
		xmlFatalErr(ctxt, XML_ERR_ENTITYREF_SEMICOL_MISSING, NULL);
	    }
	}
    }
    return(ent);
}

/**
 * xmlParseStringEntityRef:
 * @ctxt:  an XML parser context
 * @str:  a pointer to an index in the string
 *
 * parse ENTITY references declarations, but this version parses it from
 * a string value.
 *
 * [68] EntityRef ::= '&' Name ';'
 *
 * [ WFC: Entity Declared ]
 * In a document without any DTD, a document with only an internal DTD
 * subset which contains no parameter entity references, or a document
 * with "standalone='yes'", the Name given in the entity reference
 * must match that in an entity declaration, except that well-formed
 * documents need not declare any of the following entities: amp, lt,
 * gt, apos, quot.  The declaration of a parameter entity must precede
 * any reference to it.  Similarly, the declaration of a general entity
 * must precede any reference to it which appears in a default value in an
 * attribute-list declaration. Note that if entities are declared in the
 * external subset or in external parameter entities, a non-validating
 * processor is not obligated to read and process their declarations;
 * for such documents, the rule that an entity must be declared is a
 * well-formedness constraint only if standalone='yes'.
 *
 * [ WFC: Parsed Entity ]
 * An entity reference must not contain the name of an unparsed entity
 *
 * Returns the hw_xmlEntityPtr if found, or NULL otherwise. The str pointer
 * is updated to the current location in the string.
 */
hw_xmlEntityPtr
xmlParseStringEntityRef(hw_xmlParserCtxtPtr ctxt, const hw_xmlChar ** str) {
    hw_xmlChar *name;
    const hw_xmlChar *ptr;
    hw_xmlChar cur;
    hw_xmlEntityPtr ent = NULL;

    if ((str == NULL) || (*str == NULL))
        return(NULL);
    ptr = *str;
    cur = *ptr;
    if (cur == '&') {
        ptr++;
	cur = *ptr;
        name = xmlParseStringName(ctxt, &ptr);
	if (name == NULL) {
	    xmlFatalErrMsg(ctxt, XML_ERR_NAME_REQUIRED,
			   "xmlParseStringEntityRef: no name\n");
	} else {
	    if (*ptr == ';') {
	        ptr++;
		/*
		 * Ask first SAX for entity resolution, otherwise try the
		 * predefined set.
		 */
		if (ctxt->sax != NULL) {
		    if (ctxt->sax->getEntity != NULL)
			ent = ctxt->sax->getEntity(ctxt->userData, name);
		    if (ent == NULL)
		        ent = hw_xmlGetPredefinedEntity(name);
		    if ((ent == NULL) && (ctxt->userData==ctxt)) {
			ent = hw_xmlSAX2GetEntity(ctxt, name);
		    }
		}
		/*
		 * [ WFC: Entity Declared ]
		 * In a document without any DTD, a document with only an
		 * internal DTD subset which contains no parameter entity
		 * references, or a document with "standalone='yes'", the
		 * Name given in the entity reference must match that in an
		 * entity declaration, except that well-formed documents
		 * need not declare any of the following entities: amp, lt,
		 * gt, apos, quot.
		 * The declaration of a parameter entity must precede any
		 * reference to it.
		 * Similarly, the declaration of a general entity must
		 * precede any reference to it which appears in a default
		 * value in an attribute-list declaration. Note that if
		 * entities are declared in the external subset or in
		 * external parameter entities, a non-validating processor
		 * is not obligated to read and process their declarations;
		 * for such documents, the rule that an entity must be
		 * declared is a well-formedness constraint only if
		 * standalone='yes'. 
		 */
		if (ent == NULL) {
		    if ((ctxt->standalone == 1) ||
		        ((ctxt->hasExternalSubset == 0) &&
			 (ctxt->hasPErefs == 0))) {
			xmlFatalErrMsgStr(ctxt, XML_ERR_UNDECLARED_ENTITY,
				 "Entity '%s' not defined\n", name);
		    } else {
			xmlErrMsgStr(ctxt, XML_WAR_UNDECLARED_ENTITY,
				      "Entity '%s' not defined\n",
				      name);
		    }
		    /* TODO ? check regressions ctxt->valid = 0; */
		}

		/*
		 * [ WFC: Parsed Entity ]
		 * An entity reference must not contain the name of an
		 * unparsed entity
		 */
		else if (ent->etype == XML_EXTERNAL_GENERAL_UNPARSED_ENTITY) {
		    xmlFatalErrMsgStr(ctxt, XML_ERR_UNPARSED_ENTITY,
			     "Entity reference to unparsed entity %s\n", name);
		}

		/*
		 * [ WFC: No External Entity References ]
		 * Attribute values cannot contain direct or indirect
		 * entity references to external entities.
		 */
		else if ((ctxt->instate == XML_PARSER_ATTRIBUTE_VALUE) &&
		         (ent->etype == XML_EXTERNAL_GENERAL_PARSED_ENTITY)) {
		    xmlFatalErrMsgStr(ctxt, XML_ERR_ENTITY_IS_EXTERNAL,
		     "Attribute references external entity '%s'\n", name);
		}
		/*
		 * [ WFC: No < in Attribute Values ]
		 * The replacement text of any entity referred to directly or
		 * indirectly in an attribute value (other than "&lt;") must
		 * not contain a <. 
		 */
		else if ((ctxt->instate == XML_PARSER_ATTRIBUTE_VALUE) &&
		         (ent != NULL) &&
			 (!hw_xmlStrEqual(ent->name, hw_BAD_CAST "lt")) &&
		         (ent->content != NULL) &&
			 (hw_xmlStrchr(ent->content, '<'))) {
		    xmlFatalErrMsgStr(ctxt, XML_ERR_LT_IN_ATTRIBUTE,
		 "'<' in entity '%s' is not allowed in attributes values\n",
	                              name);
		}

		/*
		 * Internal check, no parameter entities here ...
		 */
		else {
		    switch (ent->etype) {
			case XML_INTERNAL_PARAMETER_ENTITY:
			case XML_EXTERNAL_PARAMETER_ENTITY:
			    xmlFatalErrMsgStr(ctxt, XML_ERR_ENTITY_IS_PARAMETER,
			 "Attempt to reference the parameter entity '%s'\n",
			                      name);
			break;
			default:
			break;
		    }
		}

		/*
		 * [ WFC: No Recursion ]
		 * A parsed entity must not contain a recursive reference
		 * to itself, either directly or indirectly. 
		 * Done somewhere else
		 */

	    } else {
		xmlFatalErr(ctxt, XML_ERR_ENTITYREF_SEMICOL_MISSING, NULL);
	    }
	    hw_xmlFree(name);
	}
    }
    *str = ptr;
    return(ent);
}


/**
 * xmlParseStringPEReference:
 * @ctxt:  an XML parser context
 * @str:  a pointer to an index in the string
 *
 * parse PEReference declarations
 *
 * [69] PEReference ::= '%' Name ';'
 *
 * [ WFC: No Recursion ]
 * A parsed entity must not contain a recursive
 * reference to itself, either directly or indirectly. 
 *
 * [ WFC: Entity Declared ]
 * In a document without any DTD, a document with only an internal DTD
 * subset which contains no parameter entity references, or a document
 * with "standalone='yes'", ...  ... The declaration of a parameter
 * entity must precede any reference to it...
 *
 * [ VC: Entity Declared ]
 * In a document with an external subset or external parameter entities
 * with "standalone='no'", ...  ... The declaration of a parameter entity
 * must precede any reference to it...
 *
 * [ WFC: In DTD ]
 * Parameter-entity references may only appear in the DTD.
 * NOTE: misleading but this is handled.
 *
 * Returns the string of the entity content.
 *         str is updated to the current value of the index
 */
hw_xmlEntityPtr
xmlParseStringPEReference(hw_xmlParserCtxtPtr ctxt, const hw_xmlChar **str) {
    const hw_xmlChar *ptr;
    hw_xmlChar cur;
    hw_xmlChar *name;
    hw_xmlEntityPtr entity = NULL;

    if ((str == NULL) || (*str == NULL)) return(NULL);
    ptr = *str;
    cur = *ptr;
    if (cur == '%') {
        ptr++;
	cur = *ptr;
        name = xmlParseStringName(ctxt, &ptr);
	if (name == NULL) {
	    xmlFatalErrMsg(ctxt, XML_ERR_NAME_REQUIRED,
			   "xmlParseStringPEReference: no name\n");
	} else {
	    cur = *ptr;
	    if (cur == ';') {
		ptr++;
		cur = *ptr;
		if ((ctxt->sax != NULL) &&
		    (ctxt->sax->getParameterEntity != NULL))
		    entity = ctxt->sax->getParameterEntity(ctxt->userData,
		                                           name);
		if (entity == NULL) {
		    /*
		     * [ WFC: Entity Declared ]
		     * In a document without any DTD, a document with only an
		     * internal DTD subset which contains no parameter entity
		     * references, or a document with "standalone='yes'", ...
		     * ... The declaration of a parameter entity must precede
		     * any reference to it...
		     */
		    if ((ctxt->standalone == 1) ||
			((ctxt->hasExternalSubset == 0) &&
			 (ctxt->hasPErefs == 0))) {
			xmlFatalErrMsgStr(ctxt, XML_ERR_UNDECLARED_ENTITY,
			     "PEReference: %%%s; not found\n", name);
		    } else {
			/*
			 * [ VC: Entity Declared ]
			 * In a document with an external subset or external
			 * parameter entities with "standalone='no'", ...
			 * ... The declaration of a parameter entity must
			 * precede any reference to it...
			 */
			xmlWarningMsg(ctxt, XML_WAR_UNDECLARED_ENTITY,
				      "PEReference: %%%s; not found\n",
			              name, NULL);
			ctxt->valid = 0;
		    }
		} else {
		    /*
		     * Internal checking in case the entity quest barfed
		     */
		    if ((entity->etype != XML_INTERNAL_PARAMETER_ENTITY) &&
		        (entity->etype != XML_EXTERNAL_PARAMETER_ENTITY)) {
			xmlWarningMsg(ctxt, XML_WAR_UNDECLARED_ENTITY,
			              "%%%s; is not a parameter entity\n",
				      name, NULL);
		    }
		}
		ctxt->hasPErefs = 1;
	    } else {
		xmlFatalErr(ctxt, XML_ERR_ENTITYREF_SEMICOL_MISSING, NULL);
	    }
	    hw_xmlFree(name);
	}
    }
    *str = ptr;
    return(entity);
}


/************************************************************************
 *									*
 *		      SAX 2 specific operations				*
 *									*
 ************************************************************************/

static const hw_xmlChar *
xmlParseNCNameComplex(hw_xmlParserCtxtPtr ctxt) {
    int len = 0, l;
    int c;
    int count = 0;

    /*
     * Handler for more complex cases
     */
    GROW;
    c = CUR_CHAR(l);
    if ((c == ' ') || (c == '>') || (c == '/') || /* accelerators */
	(!hw_IS_LETTER(c) && (c != '_'))) {
	return(NULL);
    }

    while ((c != ' ') && (c != '>') && (c != '/') && /* test bigname.xml */
	   ((hw_IS_LETTER(c)) || (hw_IS_DIGIT(c)) ||
            (c == '.') || (c == '-') || (c == '_') ||
	    (hw_IS_COMBINING(c)) ||
	    (hw_IS_EXTENDER(c)))) {
	if (count++ > 100) {
	    count = 0;
	    GROW;
	}
	len += l;
	NEXTL(l);
	c = CUR_CHAR(l);
    }
    return(hw_xmlDictLookup(ctxt->dict, ctxt->input->cur - len, len));
}

/*
 * xmlGetNamespace:
 * @ctxt:  an XML parser context
 * @prefix:  the prefix to lookup
 *
 * Lookup the namespace name for the @prefix (which ca be NULL)
 * The prefix must come from the @ctxt->dict dictionnary
 *
 * Returns the namespace name or NULL if not bound
 */
static const hw_xmlChar *
xmlGetNamespace(hw_xmlParserCtxtPtr ctxt, const hw_xmlChar *prefix) {
    int i;

    if (prefix == ctxt->str_xml) return(ctxt->str_xml_ns);
    for (i = ctxt->nsNr - 2;i >= 0;i-=2)
        if (ctxt->nsTab[i] == prefix) {
	    if ((prefix == NULL) && (*ctxt->nsTab[i + 1] == 0))
	        return(NULL);
	    return(ctxt->nsTab[i + 1]);
	}
    return(NULL);
}

/**
 * xmlParseNCName:
 * @ctxt:  an XML parser context
 * @len:  lenght of the string parsed
 *
 * parse an XML name.
 *
 * [4NS] NCNameChar ::= Letter | Digit | '.' | '-' | '_' |
 *                      CombiningChar | Extender
 *
 * [5NS] NCName ::= (Letter | '_') (NCNameChar)*
 *
 * Returns the Name parsed or NULL
 */

static const hw_xmlChar *
xmlParseNCName(hw_xmlParserCtxtPtr ctxt) {
    const hw_xmlChar *in;
    const hw_xmlChar *ret;
    int count = 0;

    /*
     * Accelerator for simple ASCII names
     */
    in = ctxt->input->cur;
    if (((*in >= 0x61) && (*in <= 0x7A)) ||
	((*in >= 0x41) && (*in <= 0x5A)) ||
	(*in == '_')) {
	in++;
	while (((*in >= 0x61) && (*in <= 0x7A)) ||
	       ((*in >= 0x41) && (*in <= 0x5A)) ||
	       ((*in >= 0x30) && (*in <= 0x39)) ||
	       (*in == '_') || (*in == '-') ||
	       (*in == '.'))
	    in++;
	if ((*in > 0) && (*in < 0x80)) {
	    count = in - ctxt->input->cur;
	    ret = hw_xmlDictLookup(ctxt->dict, ctxt->input->cur, count);
	    ctxt->input->cur = in;
	    ctxt->nbChars += count;
	    ctxt->input->col += count;
	    if (ret == NULL) {
	        hw_xmlErrMemory(ctxt, NULL);
	    }
	    return(ret);
	}
    }
    return(xmlParseNCNameComplex(ctxt));
}

/**
 * xmlParseQName:
 * @ctxt:  an XML parser context
 * @prefix:  pointer to store the prefix part
 *
 * parse an XML Namespace QName
 *
 * [6]  QName  ::= (Prefix ':')? LocalPart
 * [7]  Prefix  ::= NCName
 * [8]  LocalPart  ::= NCName
 *
 * Returns the Name parsed or NULL
 */

static const hw_xmlChar *
xmlParseQName(hw_xmlParserCtxtPtr ctxt, const hw_xmlChar **prefix) {
    const hw_xmlChar *l, *p;

    GROW;

    l = xmlParseNCName(ctxt);
    if (l == NULL) {
        if (CUR == ':') {
	    l = hw_xmlParseName(ctxt);
	    if (l != NULL) {
	        xmlNsErr(ctxt, XML_NS_ERR_QNAME, 
		         "Failed to parse QName '%s'\n", l, NULL, NULL);
		*prefix = NULL;
		return(l);
	    }
	}
        return(NULL);
    }
    if (CUR == ':') {
        NEXT;
	p = l;
	l = xmlParseNCName(ctxt);
	if (l == NULL) {
	    hw_xmlChar *tmp;

            xmlNsErr(ctxt, XML_NS_ERR_QNAME,
	             "Failed to parse QName '%s:'\n", p, NULL, NULL);
	    tmp = hw_xmlBuildQName(hw_BAD_CAST "", p, NULL, 0);
	    p = hw_xmlDictLookup(ctxt->dict, tmp, -1);
	    if (tmp != NULL) hw_xmlFree(tmp);
	    *prefix = NULL;
	    return(p);
	}
	if (CUR == ':') {
	    hw_xmlChar *tmp;

            xmlNsErr(ctxt, XML_NS_ERR_QNAME,
	             "Failed to parse QName '%s:%s:'\n", p, l, NULL);
	    NEXT;
	    tmp = (hw_xmlChar *) hw_xmlParseName(ctxt);
	    if (tmp != NULL) {
	        tmp = hw_xmlBuildQName(tmp, l, NULL, 0);
		l = hw_xmlDictLookup(ctxt->dict, tmp, -1);
		if (tmp != NULL) hw_xmlFree(tmp);
		*prefix = p;
		return(l);
	    }
	    tmp = hw_xmlBuildQName(hw_BAD_CAST "", l, NULL, 0);
	    l = hw_xmlDictLookup(ctxt->dict, tmp, -1);
	    if (tmp != NULL) hw_xmlFree(tmp);
	    *prefix = p;
	    return(l);
	}
	*prefix = p;
    } else
        *prefix = NULL;
    return(l);
}

/**
 * xmlParseQNameAndCompare:
 * @ctxt:  an XML parser context
 * @name:  the localname
 * @prefix:  the prefix, if any.
 *
 * parse an XML name and compares for match
 * (specialized for endtag parsing)
 *
 * Returns NULL for an illegal name, (hw_xmlChar*) 1 for success
 * and the name for mismatch
 */

static const hw_xmlChar *
xmlParseQNameAndCompare(hw_xmlParserCtxtPtr ctxt, hw_xmlChar const *name,
                        hw_xmlChar const *prefix) {
    const hw_xmlChar *cmp = name;
    const hw_xmlChar *in;
    const hw_xmlChar *ret;
    const hw_xmlChar *prefix2;

    if (prefix == NULL) return(xmlParseNameAndCompare(ctxt, name));

    GROW;
    in = ctxt->input->cur;
    
    cmp = prefix;
    while (*in != 0 && *in == *cmp) {
    	++in;
	++cmp;
    }
    if ((*cmp == 0) && (*in == ':')) {
        in++;
	cmp = name;
	while (*in != 0 && *in == *cmp) {
	    ++in;
	    ++cmp;
	}
	if (*cmp == 0 && (*in == '>' || hw_IS_BLANK_CH (*in))) {
	    /* success */
	    ctxt->input->cur = in;
	    return((const hw_xmlChar*) 1);
	}
    }
    /*
     * all strings coms from the dictionary, equality can be done directly
     */
    ret = xmlParseQName (ctxt, &prefix2);
    if ((ret == name) && (prefix == prefix2))
	return((const hw_xmlChar*) 1);
    return ret;
}

/**
 * xmlParseAttValueInternal:
 * @ctxt:  an XML parser context
 * @len:  attribute len result
 * @alloc:  whether the attribute was reallocated as a new string
 * @normalize:  if 1 then further non-CDATA normalization must be done
 *
 * parse a value for an attribute.
 * NOTE: if no normalization is needed, the routine will return pointers
 *       directly from the data buffer.
 *
 * 3.3.3 Attribute-Value Normalization:
 * Before the value of an attribute is passed to the application or
 * checked for validity, the XML processor must normalize it as follows: 
 * - a character reference is processed by appending the referenced
 *   character to the attribute value
 * - an entity reference is processed by recursively processing the
 *   replacement text of the entity 
 * - a whitespace character (#x20, #xD, #xA, #x9) is processed by
 *   appending #x20 to the normalized value, except that only a single
 *   #x20 is appended for a "#xD#xA" sequence that is part of an external
 *   parsed entity or the literal entity value of an internal parsed entity 
 * - other characters are processed by appending them to the normalized value 
 * If the declared value is not CDATA, then the XML processor must further
 * process the normalized attribute value by discarding any leading and
 * trailing space (#x20) characters, and by replacing sequences of space
 * (#x20) characters by a single space (#x20) character.  
 * All attributes for which no declaration has been read should be treated
 * by a non-validating parser as if declared CDATA.
 *
 * Returns the AttValue parsed or NULL. The value has to be freed by the
 *     caller if it was copied, this can be detected by val[*len] == 0.
 */

static hw_xmlChar *
xmlParseAttValueInternal(hw_xmlParserCtxtPtr ctxt, int *len, int *alloc,
                         int normalize)
{
    hw_xmlChar limit = 0;
    const hw_xmlChar *in = NULL, *start, *end, *last;
    hw_xmlChar *ret = NULL;

    GROW;
    in = (hw_xmlChar *) CUR_PTR;
    if (*in != '"' && *in != '\'') {
        xmlFatalErr(ctxt, XML_ERR_ATTRIBUTE_NOT_STARTED, NULL);
        return (NULL);
    }
    ctxt->instate = XML_PARSER_ATTRIBUTE_VALUE;

    /*
     * try to handle in this routine the most common case where no
     * allocation of a new string is required and where content is
     * pure ASCII.
     */
    limit = *in++;
    end = ctxt->input->end;
    start = in;
    if (in >= end) {
        const hw_xmlChar *oldbase = ctxt->input->base;
	GROW;
	if (oldbase != ctxt->input->base) {
	    long delta = ctxt->input->base - oldbase;
	    start = start + delta;
	    in = in + delta;
	}
	end = ctxt->input->end;
    }
    if (normalize) {
        /*
	 * Skip any leading spaces
	 */
	while ((in < end) && (*in != limit) && 
	       ((*in == 0x20) || (*in == 0x9) ||
	        (*in == 0xA) || (*in == 0xD))) {
	    in++;
	    start = in;
	    if (in >= end) {
		const hw_xmlChar *oldbase = ctxt->input->base;
		GROW;
		if (oldbase != ctxt->input->base) {
		    long delta = ctxt->input->base - oldbase;
		    start = start + delta;
		    in = in + delta;
		}
		end = ctxt->input->end;
	    }
	}
	while ((in < end) && (*in != limit) && (*in >= 0x20) &&
	       (*in <= 0x7f) && (*in != '&') && (*in != '<')) {
	    if ((*in++ == 0x20) && (*in == 0x20)) break;
	    if (in >= end) {
		const hw_xmlChar *oldbase = ctxt->input->base;
		GROW;
		if (oldbase != ctxt->input->base) {
		    long delta = ctxt->input->base - oldbase;
		    start = start + delta;
		    in = in + delta;
		}
		end = ctxt->input->end;
	    }
	}
	last = in;
	/*
	 * skip the trailing blanks
	 */
	while ((last[-1] == 0x20) && (last > start)) last--;
	while ((in < end) && (*in != limit) && 
	       ((*in == 0x20) || (*in == 0x9) ||
	        (*in == 0xA) || (*in == 0xD))) {
	    in++;
	    if (in >= end) {
		const hw_xmlChar *oldbase = ctxt->input->base;
		GROW;
		if (oldbase != ctxt->input->base) {
		    long delta = ctxt->input->base - oldbase;
		    start = start + delta;
		    in = in + delta;
		    last = last + delta;
		}
		end = ctxt->input->end;
	    }
	}
	if (*in != limit) goto need_complex;
    } else {
	while ((in < end) && (*in != limit) && (*in >= 0x20) &&
	       (*in <= 0x7f) && (*in != '&') && (*in != '<')) {
	    in++;
	    if (in >= end) {
		const hw_xmlChar *oldbase = ctxt->input->base;
		GROW;
		if (oldbase != ctxt->input->base) {
		    long delta = ctxt->input->base - oldbase;
		    start = start + delta;
		    in = in + delta;
		}
		end = ctxt->input->end;
	    }
	}
	last = in;
	if (*in != limit) goto need_complex;
    }
    in++;
    if (len != NULL) {
        *len = last - start;
        ret = (hw_xmlChar *) start;
    } else {
        if (alloc) *alloc = 1;
        ret = hw_xmlStrndup(start, last - start);
    }
    CUR_PTR = in;
    if (alloc) *alloc = 0;
    return ret;
need_complex:
    if (alloc) *alloc = 1;
    return xmlParseAttValueComplex(ctxt, len, normalize);
}

/**
 * xmlParseAttribute2:
 * @ctxt:  an XML parser context
 * @pref:  the element prefix
 * @elem:  the element name
 * @prefix:  a hw_xmlChar ** used to store the value of the attribute prefix
 * @value:  a hw_xmlChar ** used to store the value of the attribute
 * @len:  an int * to save the length of the attribute
 * @alloc:  an int * to indicate if the attribute was allocated
 *
 * parse an attribute in the new SAX2 framework.
 *
 * Returns the attribute name, and the value in *value, .
 */

static const hw_xmlChar *
xmlParseAttribute2(hw_xmlParserCtxtPtr ctxt,
                   const hw_xmlChar *pref, const hw_xmlChar *elem,
                   const hw_xmlChar **prefix, hw_xmlChar **value,
		   int *len, int *alloc) {
    const hw_xmlChar *name;
    hw_xmlChar *val, *internal_val = NULL;
    int normalize = 0;

    *value = NULL;
    GROW;
    name = xmlParseQName(ctxt, prefix);
    if (name == NULL) {
	xmlFatalErrMsg(ctxt, XML_ERR_NAME_REQUIRED,
	               "error parsing attribute name\n");
        return(NULL);
    }


    /*
     * read the value
     */
    SKIP_BLANKS;
    if (RAW == '=') {
        NEXT;
	SKIP_BLANKS;
	val = xmlParseAttValueInternal(ctxt, len, alloc, normalize);
	ctxt->instate = XML_PARSER_CONTENT;
    } else {
	xmlFatalErrMsgStr(ctxt, XML_ERR_ATTRIBUTE_WITHOUT_VALUE,
	       "Specification mandate value for attribute %s\n", name);
	return(NULL);
    }

	if (*prefix == ctxt->str_xml) {
		/*
		 * Check that xml:lang conforms to the specification
		 * No more registered as an error, just generate a warning now
		 * since this was deprecated in XML second edition
		 */

		/*
		 * Check that xml:space conforms to the specification
		 */
		if (hw_xmlStrEqual(name, hw_BAD_CAST "space")) {
			internal_val = hw_xmlStrndup(val, *len);
			if (hw_xmlStrEqual(internal_val, hw_BAD_CAST "default"))
				*(ctxt->space) = 0;
			else if (hw_xmlStrEqual(internal_val, hw_BAD_CAST "preserve"))
				*(ctxt->space) = 1;
			else {
				xmlWarningMsg(ctxt, XML_WAR_SPACE_VALUE,
"Invalid value \"%s\" for xml:space : \"default\" or \"preserve\" expected\n",
                                 internal_val, NULL);
			}
		}
		if (internal_val) {
			hw_xmlFree(internal_val);
		}
	}

    *value = val;
    return(name);
}

/**
 * xmlParseStartTag2:
 * @ctxt:  an XML parser context
 * 
 * parse a start of tag either for rule element or
 * EmptyElement. In both case we don't parse the tag closing chars.
 * This routine is called when running SAX2 parsing
 *
 * [40] STag ::= '<' Name (S Attribute)* S? '>'
 *
 * [ WFC: Unique Att Spec ]
 * No attribute name may appear more than once in the same start-tag or
 * empty-element tag. 
 *
 * [44] EmptyElemTag ::= '<' Name (S Attribute)* S? '/>'
 *
 * [ WFC: Unique Att Spec ]
 * No attribute name may appear more than once in the same start-tag or
 * empty-element tag. 
 *
 * With namespace:
 *
 * [NS 8] STag ::= '<' QName (S Attribute)* S? '>'
 *
 * [NS 10] EmptyElement ::= '<' QName (S Attribute)* S? '/>'
 *
 * Returns the element name parsed
 */

static const hw_xmlChar *
xmlParseStartTag2(hw_xmlParserCtxtPtr ctxt, const hw_xmlChar **pref,
                  const hw_xmlChar **URI, int *tlen) {
    const hw_xmlChar *localname;
    const hw_xmlChar *prefix;
    const hw_xmlChar *attname;
    const hw_xmlChar *aprefix;
    const hw_xmlChar *nsname;
    hw_xmlChar *attvalue;
    const hw_xmlChar **atts = ctxt->atts;
    int maxatts = ctxt->maxatts;
    int nratts, nbatts, nbdef;
    int i, j, nbNs, attval;
    const hw_xmlChar *base;
    unsigned long cur;
    int nsNr = ctxt->nsNr;

    if (RAW != '<') return(NULL);
    NEXT1;

    /*
     * NOTE: it is crucial with the SAX2 API to never call SHRINK beyond that
     *       point since the attribute values may be stored as pointers to
     *       the buffer and calling SHRINK would destroy them !
     *       The Shrinking is only possible once the full set of attribute
     *       callbacks have been done.
     */
reparse:
    SHRINK;
    base = ctxt->input->base;
    cur = ctxt->input->cur - ctxt->input->base;
    nbatts = 0;
    nratts = 0;
    nbdef = 0;
    nbNs = 0;
    attval = 0;
    /* Forget any namespaces added during an earlier parse of this element. */
    ctxt->nsNr = nsNr;

    localname = xmlParseQName(ctxt, &prefix);
    if (localname == NULL) {
	xmlFatalErrMsg(ctxt, XML_ERR_NAME_REQUIRED,
		       "StartTag: invalid element name\n");
        return(NULL);
    }
    *tlen = ctxt->input->cur - ctxt->input->base - cur;

    /*
     * Now parse the attributes, it ends up with the ending
     *
     * (S Attribute)* S?
     */
    SKIP_BLANKS;
    GROW;
    if (ctxt->input->base != base) goto base_changed;

    while ((RAW != '>') && 
	   ((RAW != '/') || (NXT(1) != '>')) &&
	   (hw_IS_BYTE_CHAR(RAW))) {
	const hw_xmlChar *q = CUR_PTR;
	unsigned int cons = ctxt->input->consumed;
	int len = -1, alloc = 0;

	attname = xmlParseAttribute2(ctxt, prefix, localname,
	                             &aprefix, &attvalue, &len, &alloc);
        if ((attname != NULL) && (attvalue != NULL)) {
	    if (len < 0) len = hw_xmlStrlen(attvalue);
            if ((attname == ctxt->str_xmlns) && (aprefix == NULL)) {
	        const hw_xmlChar *URL = hw_xmlDictLookup(ctxt->dict, attvalue, len);
		hw_xmlURIPtr uri;

                if (*URL != 0) {
		    uri = hw_xmlParseURI((const char *) URL);
		    if (uri == NULL) {
			xmlWarningMsg(ctxt, XML_WAR_NS_URI,
				      "xmlns: %s not a valid URI\n",
				      URL, NULL);
		    } else {
			if (uri->scheme == NULL) {
			    xmlWarningMsg(ctxt, XML_WAR_NS_URI_RELATIVE,
				   "xmlns: URI %s is not absolute\n",
				          URL, NULL);
			}
			hw_xmlFreeURI(uri);
		    }
		}
		/*
		 * check that it's not a defined namespace
		 */
		for (j = 1;j <= nbNs;j++)
		    if (ctxt->nsTab[ctxt->nsNr - 2 * j] == NULL)
			break;
		if (j <= nbNs)
		    xmlErrAttributeDup(ctxt, NULL, attname);
		else
		    if (nsPush(ctxt, NULL, URL) > 0) nbNs++;
		if (alloc != 0) hw_xmlFree(attvalue);
		SKIP_BLANKS;
		continue;
	    }
            if (aprefix == ctxt->str_xmlns) {
	        const hw_xmlChar *URL = hw_xmlDictLookup(ctxt->dict, attvalue, len);
		hw_xmlURIPtr uri;

                if (attname == ctxt->str_xml) {
		    if (URL != ctxt->str_xml_ns) {
		        xmlNsErr(ctxt, XML_NS_ERR_XML_NAMESPACE,
			         "xml namespace prefix mapped to wrong URI\n",
			         NULL, NULL, NULL);
		    }
		    /*
		     * Do not keep a namespace definition node
		     */
		    if (alloc != 0) hw_xmlFree(attvalue);
		    SKIP_BLANKS;
		    continue;
		}
		uri = hw_xmlParseURI((const char *) URL);
		if (uri == NULL) {
		    xmlWarningMsg(ctxt, XML_WAR_NS_URI,
			 "xmlns:%s: '%s' is not a valid URI\n",
				       attname, URL);
		} else {
		   
		    hw_xmlFreeURI(uri);
		}

		/*
		 * check that it's not a defined namespace
		 */
		for (j = 1;j <= nbNs;j++)
		    if (ctxt->nsTab[ctxt->nsNr - 2 * j] == attname)
			break;
		if (j <= nbNs)
		    xmlErrAttributeDup(ctxt, aprefix, attname);
		else
		    if (nsPush(ctxt, attname, URL) > 0) nbNs++;
		if (alloc != 0) hw_xmlFree(attvalue);
		SKIP_BLANKS;
		if (ctxt->input->base != base) goto base_changed;
		continue;
	    }

	    /*
	     * Add the pair to atts
	     */
	    if ((atts == NULL) || (nbatts + 5 > maxatts)) {
	        if (xmlCtxtGrowAttrs(ctxt, nbatts + 5) < 0) {
		    if (attvalue[len] == 0)
			hw_xmlFree(attvalue);
		    goto failed;
		}
	        maxatts = ctxt->maxatts;
		atts = ctxt->atts;
	    }
	    ctxt->attallocs[nratts++] = alloc;
	    atts[nbatts++] = attname;
	    atts[nbatts++] = aprefix;
	    atts[nbatts++] = NULL; /* the URI will be fetched later */
	    atts[nbatts++] = attvalue;
	    attvalue += len;
	    atts[nbatts++] = attvalue;
	    /*
	     * tag if some deallocation is needed
	     */
	    if (alloc != 0) attval = 1;
	} else {
	    if ((attvalue != NULL) && (attvalue[len] == 0))
		hw_xmlFree(attvalue);
	}

failed:     

	GROW
	if (ctxt->input->base != base) goto base_changed;
	if ((RAW == '>') || (((RAW == '/') && (NXT(1) == '>'))))
	    break;
	if (!hw_IS_BLANK_CH(RAW)) {
	    xmlFatalErrMsg(ctxt, XML_ERR_SPACE_REQUIRED,
			   "attributes construct error\n");
	    break;
	}
	SKIP_BLANKS;
        if ((cons == ctxt->input->consumed) && (q == CUR_PTR) &&
            (attname == NULL) && (attvalue == NULL)) {
	    xmlFatalErr(ctxt, XML_ERR_INTERNAL_ERROR,
	         "xmlParseStartTag: problem parsing attributes\n");
	    break;
	}
        GROW;
	if (ctxt->input->base != base) goto base_changed;
    }

    /*
     * The attributes checkings
     */
    for (i = 0; i < nbatts;i += 5) {
        /*
	* The default namespace does not apply to attribute names.
	*/
	if (atts[i + 1] != NULL) {
	    nsname = xmlGetNamespace(ctxt, atts[i + 1]);
	    if (nsname == NULL) {
		xmlNsErr(ctxt, XML_NS_ERR_UNDEFINED_NAMESPACE,
		    "Namespace prefix %s for %s on %s is not defined\n",
		    atts[i + 1], atts[i], localname);
	    }
	    atts[i + 2] = nsname;
	} else
	    nsname = NULL;
	/*
	 * [ WFC: Unique Att Spec ]
	 * No attribute name may appear more than once in the same
	 * start-tag or empty-element tag. 
	 * As extended by the Namespace in XML REC.
	 */
        for (j = 0; j < i;j += 5) {
	    if (atts[i] == atts[j]) {
	        if (atts[i+1] == atts[j+1]) {
		    xmlErrAttributeDup(ctxt, atts[i+1], atts[i]);
		    break;
		}
		if ((nsname != NULL) && (atts[j + 2] == nsname)) {
		    xmlNsErr(ctxt, XML_NS_ERR_ATTRIBUTE_REDEFINED,
			     "Namespaced Attribute %s in '%s' redefined\n",
			     atts[i], nsname, NULL);
		    break;
		}
	    }
	}
    }

    nsname = xmlGetNamespace(ctxt, prefix);
    if ((prefix != NULL) && (nsname == NULL)) {
	xmlNsErr(ctxt, XML_NS_ERR_UNDEFINED_NAMESPACE,
	         "Namespace prefix %s on %s is not defined\n",
		 prefix, localname, NULL);
    }
    *pref = prefix;
    *URI = nsname;

    /*
     * SAX: Start of Element !
     */
    if ((ctxt->sax != NULL) && (ctxt->sax->startElementNs != NULL) &&
	(!ctxt->disableSAX)) {
	if (nbNs > 0)
	    ctxt->sax->startElementNs(ctxt->userData, localname, prefix,
			  nsname, nbNs, &ctxt->nsTab[ctxt->nsNr - 2 * nbNs],
			  nbatts / 5, nbdef, atts);
	else
	    ctxt->sax->startElementNs(ctxt->userData, localname, prefix,
	                  nsname, 0, NULL, nbatts / 5, nbdef, atts);
    }

    /*
     * Free up attribute allocated strings if needed
     */
    if (attval != 0) {
	for (i = 3,j = 0; j < nratts;i += 5,j++)
	    if ((ctxt->attallocs[j] != 0) && (atts[i] != NULL))
	        hw_xmlFree((hw_xmlChar *) atts[i]);
    }

    return(localname);

base_changed:
    /*
     * the attribute strings are valid iif the base didn't changed
     */
    if (attval != 0) {
	for (i = 3,j = 0; j < nratts;i += 5,j++)
	    if ((ctxt->attallocs[j] != 0) && (atts[i] != NULL))
	        hw_xmlFree((hw_xmlChar *) atts[i]);
    }
    ctxt->input->cur = ctxt->input->base + cur;
    if (ctxt->wellFormed == 1) {
	goto reparse;
    }
    return(NULL);
}

/**
 * xmlParseEndTag2:
 * @ctxt:  an XML parser context
 * @line:  line of the start tag
 * @nsNr:  number of namespaces on the start tag
 *
 * parse an end of tag
 *
 * [42] ETag ::= '</' Name S? '>'
 *
 * With namespace
 *
 * [NS 9] ETag ::= '</' QName S? '>'
 */

static void
xmlParseEndTag2(hw_xmlParserCtxtPtr ctxt, const hw_xmlChar *prefix,
                const hw_xmlChar *URI, int line, int nsNr, int tlen) {
    const hw_xmlChar *name;

    GROW;
    if ((RAW != '<') || (NXT(1) != '/')) {
	xmlFatalErr(ctxt, XML_ERR_LTSLASH_REQUIRED, NULL);
	return;
    }
    SKIP(2);

    if ((tlen > 0) && (hw_xmlStrncmp(ctxt->input->cur, ctxt->name, tlen) == 0)) {
        if (ctxt->input->cur[tlen] == '>') {
	    ctxt->input->cur += tlen + 1;
	    goto done;
	}
	ctxt->input->cur += tlen;
	name = (hw_xmlChar*)1;
    } else {
	if (prefix == NULL)
	    name = xmlParseNameAndCompare(ctxt, ctxt->name);
	else
	    name = xmlParseQNameAndCompare(ctxt, ctxt->name, prefix);
    }

    /*
     * We should definitely be at the ending "S? '>'" part
     */
    GROW;
    SKIP_BLANKS;
    if ((!hw_IS_BYTE_CHAR(RAW)) || (RAW != '>')) {
	xmlFatalErr(ctxt, XML_ERR_GT_REQUIRED, NULL);
    } else
	NEXT1;

    /*
     * [ WFC: Element Type Match ]
     * The Name in an element's end-tag must match the element type in the
     * start-tag. 
     *
     */
    if (name != (hw_xmlChar*)1) {
        if (name == NULL) name = hw_BAD_CAST "unparseable";
        xmlFatalErrMsgStrIntStr(ctxt, XML_ERR_TAG_NAME_MISMATCH,
		     "Opening and ending tag mismatch: %s line %d and %s\n",
		                ctxt->name, line, name);
    }

    /*
     * SAX: End of Tag
     */
done:
    if ((ctxt->sax != NULL) && (ctxt->sax->endElementNs != NULL) &&
	(!ctxt->disableSAX))
	ctxt->sax->endElementNs(ctxt->userData, ctxt->name, prefix, URI);

    spacePop(ctxt);
    if (nsNr != 0)
	nsPop(ctxt, nsNr);
    return;
}

/**
 * hw_xmlParseContent:
 * @ctxt:  an XML parser context
 *
 * Parse a content:
 *
 * [43] content ::= (element | CharData | Reference | CDSect | PI | Comment)*
 */

void
hw_xmlParseContent(hw_xmlParserCtxtPtr ctxt) {
    GROW;
    while ((RAW != 0) &&
	   ((RAW != '<') || (NXT(1) != '/'))) {
	const hw_xmlChar *test = CUR_PTR;
	unsigned int cons = ctxt->input->consumed;
	const hw_xmlChar *cur = ctxt->input->cur;

      if ((*cur == '<') && (NXT(1) == '!') &&
		 (NXT(2) == '-') && (NXT(3) == '-')) {
	    hw_xmlParseComment(ctxt);
	    ctxt->instate = XML_PARSER_CONTENT;
	}

	/*
	 * Fourth case :  a sub-element.
	 */
	else if (*cur == '<') {
	    hw_xmlParseElement(ctxt);
	}


	/*
	 * Last case, text. Note that References are handled directly.
	 */
	else {
	    hw_xmlParseCharData(ctxt, 0);
	}

	GROW;
	/*
	 * Pop-up of finished entities.
	 */
	while ((RAW == 0) && (ctxt->inputNr > 1))
	    hw_xmlPopInput(ctxt);
	SHRINK;

	if ((cons == ctxt->input->consumed) && (test == CUR_PTR)) {
	    xmlFatalErr(ctxt, XML_ERR_INTERNAL_ERROR,
	                "detected an error in element content\n");
	    ctxt->instate = XML_PARSER_EOF;
            break;
	}
    }
}

/**
 * hw_xmlParseElement:
 * @ctxt:  an XML parser context
 *
 * parse an XML element, this is highly recursive
 *
 * [39] element ::= EmptyElemTag | STag content ETag
 *
 * [ WFC: Element Type Match ]
 * The Name in an element's end-tag must match the element type in the
 * start-tag. 
 *
 */

void
hw_xmlParseElement(hw_xmlParserCtxtPtr ctxt) {
    const hw_xmlChar *name;
    const hw_xmlChar *prefix;
    const hw_xmlChar *URI;
    hw_xmlParserNodeInfo node_info;
    int line, tlen;
    hw_xmlNodePtr ret;
    int nsNr = ctxt->nsNr;

    /* Capture start position */
    if (ctxt->record_info) {
        node_info.begin_pos = ctxt->input->consumed +
                          (CUR_PTR - ctxt->input->base);
	node_info.begin_line = ctxt->input->line;
    }

    if (ctxt->spaceNr == 0)
	spacePush(ctxt, -1);
    else
	spacePush(ctxt, *ctxt->space);

    line = ctxt->input->line;
        name = xmlParseStartTag2(ctxt, &prefix, &URI, &tlen);

    if (name == NULL) {
	spacePop(ctxt);
        return;
    }
    hw_namePush(ctxt, name);
    ret = ctxt->node;


    /*
     * Check for an Empty Element.
     */
    if ((RAW == '/') && (NXT(1) == '>')) {
        SKIP(2);
	if (ctxt->sax2) {
	    if ((ctxt->sax != NULL) && (ctxt->sax->endElementNs != NULL) &&
		(!ctxt->disableSAX))
		ctxt->sax->endElementNs(ctxt->userData, name, prefix, URI);
	}
	hw_namePop(ctxt);
	spacePop(ctxt);
	if (nsNr != ctxt->nsNr)
	    nsPop(ctxt, ctxt->nsNr - nsNr);
	if ( ret != NULL && ctxt->record_info ) {
	   node_info.end_pos = ctxt->input->consumed +
			      (CUR_PTR - ctxt->input->base);
	   node_info.end_line = ctxt->input->line;
	   node_info.node = ret;
	   hw_xmlParserAddNodeInfo(ctxt, &node_info);
	}
	return;
    }
    if (RAW == '>') {
        NEXT1;
    } else {
        xmlFatalErrMsgStrIntStr(ctxt, XML_ERR_GT_REQUIRED,
		     "Couldn't find end of Start Tag %s line %d\n",
		                name, line, NULL);

	/*
	 * end of parsing of this node.
	 */
	hw_nodePop(ctxt);
	hw_namePop(ctxt);
	spacePop(ctxt);
	if (nsNr != ctxt->nsNr)
	    nsPop(ctxt, ctxt->nsNr - nsNr);

	/*
	 * Capture end position and add node
	 */
	if ( ret != NULL && ctxt->record_info ) {
	   node_info.end_pos = ctxt->input->consumed +
			      (CUR_PTR - ctxt->input->base);
	   node_info.end_line = ctxt->input->line;
	   node_info.node = ret;
	   hw_xmlParserAddNodeInfo(ctxt, &node_info);
	}
	return;
    }

    /*
     * Parse the content of the element:
     */
    hw_xmlParseContent(ctxt);
    if (!hw_IS_BYTE_CHAR(RAW)) {
        xmlFatalErrMsgStrIntStr(ctxt, XML_ERR_TAG_NOT_FINISHED,
	 "Premature end of data in tag %s line %d\n",
		                name, line, NULL);

	/*
	 * end of parsing of this node.
	 */
	hw_nodePop(ctxt);
	hw_namePop(ctxt);
	spacePop(ctxt);
	if (nsNr != ctxt->nsNr)
	    nsPop(ctxt, ctxt->nsNr - nsNr);
	return;
    }

    /*
     * parse the end of tag: '</' should be here.
     */
    if (ctxt->sax2) {
	xmlParseEndTag2(ctxt, prefix, URI, line, ctxt->nsNr - nsNr, tlen);
	hw_namePop(ctxt);
    }
    /*
     * Capture end position and add node
     */
    if ( ret != NULL && ctxt->record_info ) {
       node_info.end_pos = ctxt->input->consumed +
                          (CUR_PTR - ctxt->input->base);
       node_info.end_line = ctxt->input->line;
       node_info.node = ret;
       hw_xmlParserAddNodeInfo(ctxt, &node_info);
    }
}

/**
 * hw_xmlParseVersionNum:
 * @ctxt:  an XML parser context
 *
 * parse the XML version value.
 *
 * [26] VersionNum ::= ([a-zA-Z0-9_.:] | '-')+
 *
 * Returns the string giving the XML version number, or NULL
 */
hw_xmlChar *
hw_xmlParseVersionNum(hw_xmlParserCtxtPtr ctxt) {
    hw_xmlChar *buf = NULL;
    int len = 0;
    int size = 10;
    hw_xmlChar cur;

    buf = (hw_xmlChar *) hw_xmlMallocAtomic(size * sizeof(hw_xmlChar));
    if (buf == NULL) {
	hw_xmlErrMemory(ctxt, NULL);
	return(NULL);
    }
    cur = CUR;
    while (((cur >= 'a') && (cur <= 'z')) ||
           ((cur >= 'A') && (cur <= 'Z')) ||
           ((cur >= '0') && (cur <= '9')) ||
           (cur == '_') || (cur == '.') ||
	   (cur == ':') || (cur == '-')) {
	if (len + 1 >= size) {
	    hw_xmlChar *tmp;

	    size *= 2;
	    tmp = (hw_xmlChar *) hw_xmlRealloc(buf, size * sizeof(hw_xmlChar));
	    if (tmp == NULL) {
		hw_xmlErrMemory(ctxt, NULL);
		return(NULL);
	    }
	    buf = tmp;
	}
	buf[len++] = cur;
	NEXT;
	cur=CUR;
    }
    buf[len] = 0;
    return(buf);
}

/**
 * hw_xmlParseVersionInfo:
 * @ctxt:  an XML parser context
 * 
 * parse the XML version.
 *
 * [24] VersionInfo ::= S 'version' Eq (' VersionNum ' | " VersionNum ")
 * 
 * [25] Eq ::= S? '=' S?
 *
 * Returns the version string, e.g. "1.0"
 */

hw_xmlChar *
hw_xmlParseVersionInfo(hw_xmlParserCtxtPtr ctxt) {
    hw_xmlChar *version = NULL;

    if (CMP7(CUR_PTR, 'v', 'e', 'r', 's', 'i', 'o', 'n')) {
	SKIP(7);
	SKIP_BLANKS;
	if (RAW != '=') {
	    xmlFatalErr(ctxt, XML_ERR_EQUAL_REQUIRED, NULL);
	    return(NULL);
        }
	NEXT;
	SKIP_BLANKS;
	if (RAW == '"') {
	    NEXT;
	    version = hw_xmlParseVersionNum(ctxt);
	    if (RAW != '"') {
		xmlFatalErr(ctxt, XML_ERR_STRING_NOT_CLOSED, NULL);
	    } else
	        NEXT;
	} else if (RAW == '\''){
	    NEXT;
	    version = hw_xmlParseVersionNum(ctxt);
	    if (RAW != '\'') {
		xmlFatalErr(ctxt, XML_ERR_STRING_NOT_CLOSED, NULL);
	    } else
	        NEXT;
	} else {
	    xmlFatalErr(ctxt, XML_ERR_STRING_NOT_STARTED, NULL);
	}
    }
    return(version);
}

/**
 * hw_xmlParseEncName:
 * @ctxt:  an XML parser context
 *
 * parse the XML encoding name
 *
 * [81] EncName ::= [A-Za-z] ([A-Za-z0-9._] | '-')*
 *
 * Returns the encoding name value or NULL
 */
hw_xmlChar *
hw_xmlParseEncName(hw_xmlParserCtxtPtr ctxt) {
    hw_xmlChar *buf = NULL;
    int len = 0;
    int size = 10;
    hw_xmlChar cur;

    cur = CUR;
    if (((cur >= 'a') && (cur <= 'z')) ||
        ((cur >= 'A') && (cur <= 'Z'))) {
	buf = (hw_xmlChar *) hw_xmlMallocAtomic(size * sizeof(hw_xmlChar));
	if (buf == NULL) {
	    hw_xmlErrMemory(ctxt, NULL);
	    return(NULL);
	}
	
	buf[len++] = cur;
	NEXT;
	cur = CUR;
	while (((cur >= 'a') && (cur <= 'z')) ||
	       ((cur >= 'A') && (cur <= 'Z')) ||
	       ((cur >= '0') && (cur <= '9')) ||
	       (cur == '.') || (cur == '_') ||
	       (cur == '-')) {
	    if (len + 1 >= size) {
	        hw_xmlChar *tmp;

		size *= 2;
		tmp = (hw_xmlChar *) hw_xmlRealloc(buf, size * sizeof(hw_xmlChar));
		if (tmp == NULL) {
		    hw_xmlErrMemory(ctxt, NULL);
		    hw_xmlFree(buf);
		    return(NULL);
		}
		buf = tmp;
	    }
	    buf[len++] = cur;
	    NEXT;
	    cur = CUR;
	    if (cur == 0) {
	        SHRINK;
		GROW;
		cur = CUR;
	    }
        }
	buf[len] = 0;
    } else {
	xmlFatalErr(ctxt, XML_ERR_ENCODING_NAME, NULL);
    }
    return(buf);
}

/**
 * hw_xmlParseEncodingDecl:
 * @ctxt:  an XML parser context
 * 
 * parse the XML encoding declaration
 *
 * [80] EncodingDecl ::= S 'encoding' Eq ('"' EncName '"' |  "'" EncName "'")
 *
 * this setups the conversion filters.
 *
 * Returns the encoding value or NULL
 */

const hw_xmlChar *
hw_xmlParseEncodingDecl(hw_xmlParserCtxtPtr ctxt) {
    hw_xmlChar *encoding = NULL;

    SKIP_BLANKS;
    if (CMP8(CUR_PTR, 'e', 'n', 'c', 'o', 'd', 'i', 'n', 'g')) {
	SKIP(8);
	SKIP_BLANKS;
	if (RAW != '=') {
	    xmlFatalErr(ctxt, XML_ERR_EQUAL_REQUIRED, NULL);
	    return(NULL);
        }
	NEXT;
	SKIP_BLANKS;
	if (RAW == '"') {
	    NEXT;
	    encoding = hw_xmlParseEncName(ctxt);
	    if (RAW != '"') {
		xmlFatalErr(ctxt, XML_ERR_STRING_NOT_CLOSED, NULL);
	    } else
	        NEXT;
	} else if (RAW == '\''){
	    NEXT;
	    encoding = hw_xmlParseEncName(ctxt);
	    if (RAW != '\'') {
		xmlFatalErr(ctxt, XML_ERR_STRING_NOT_CLOSED, NULL);
	    } else
	        NEXT;
	} else {
	    xmlFatalErr(ctxt, XML_ERR_STRING_NOT_STARTED, NULL);
	}
	/*
	 * UTF-16 encoding stwich has already taken place at this stage,
	 * more over the little-endian/big-endian selection is already done
	 */
        if ((encoding != NULL) &&
	    ((!hw_xmlStrcasecmp(encoding, hw_BAD_CAST "UTF-16")) ||
	     (!hw_xmlStrcasecmp(encoding, hw_BAD_CAST "UTF16")))) {
	    if (ctxt->encoding != NULL)
		hw_xmlFree((hw_xmlChar *) ctxt->encoding);
	    ctxt->encoding = encoding;
	}
	/*
	 * UTF-8 encoding is handled natively
	 */
        else if ((encoding != NULL) &&
	    ((!hw_xmlStrcasecmp(encoding, hw_BAD_CAST "UTF-8")) ||
	     (!hw_xmlStrcasecmp(encoding, hw_BAD_CAST "UTF8")))) {
	    if (ctxt->encoding != NULL)
		hw_xmlFree((hw_xmlChar *) ctxt->encoding);
	    ctxt->encoding = encoding;
	}
	else if (encoding != NULL) {
	    hw_xmlCharEncodingHandlerPtr handler;

	    if (ctxt->input->encoding != NULL)
		hw_xmlFree((hw_xmlChar *) ctxt->input->encoding);
	    ctxt->input->encoding = encoding;

            handler = hw_xmlFindCharEncodingHandler((const char *) encoding);
	    if (handler != NULL) {
		hw_xmlSwitchToEncoding(ctxt, handler);
	    } else {
		xmlFatalErrMsgStr(ctxt, XML_ERR_UNSUPPORTED_ENCODING,
			"Unsupported encoding %s\n", encoding);
		return(NULL);
	    }
	}
    }
    return(encoding);
}
/**
 * hw_xmlParseXMLDecl:
 * @ctxt:  an XML parser context
 * 
 * parse an XML declaration header
 *
 * [23] XMLDecl ::= '<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>'
 */

void
hw_xmlParseXMLDecl(hw_xmlParserCtxtPtr ctxt) {
    hw_xmlChar *version;

    /*
     * This value for standalone indicates that the document has an
     * XML declaration but it does not have a standalone attribute.
     * It will be overwritten later if a standalone attribute is found.
     */

    /*
     * We know that '<?xml' is here.
     */
    SKIP(5);

    if (!hw_IS_BLANK_CH(RAW)) {
	xmlFatalErrMsg(ctxt, XML_ERR_SPACE_REQUIRED,
	               "Blank needed after '<?xml'\n");
    }
    SKIP_BLANKS;

    /*
     * We must have the VersionInfo here.
     */
    version = hw_xmlParseVersionInfo(ctxt);
    if (version == NULL) {
	xmlFatalErr(ctxt, XML_ERR_VERSION_MISSING, NULL);
    } else {
	if (!hw_xmlStrEqual(version, (const hw_xmlChar *) hw_XML_DEFAULT_VERSION)) {
	    /*
	     * TODO: Blueberry should be detected here
	     */
	    xmlWarningMsg(ctxt, XML_WAR_UNKNOWN_VERSION,
		          "Unsupported version '%s'\n",
			  version, NULL);
	}
	if (ctxt->version != NULL)
	    hw_xmlFree((void *) ctxt->version);
	ctxt->version = version;
    }

    /*
     * We may have the encoding declaration
     */
    if (!hw_IS_BLANK_CH(RAW)) {
        if ((RAW == '?') && (NXT(1) == '>')) {
	    SKIP(2);
	    return;
	}
	xmlFatalErrMsg(ctxt, XML_ERR_SPACE_REQUIRED, "Blank needed here\n");
    }
    hw_xmlParseEncodingDecl(ctxt);
    if (ctxt->errNo == XML_ERR_UNSUPPORTED_ENCODING) {
	/*
	 * The XML REC instructs us to stop parsing right here
	 */
        return;
    }

    /*
     * We may have the standalone status.
     */
    if ((ctxt->input->encoding != NULL) && (!hw_IS_BLANK_CH(RAW))) {
        if ((RAW == '?') && (NXT(1) == '>')) {
	    SKIP(2);
	    return;
	}
	xmlFatalErrMsg(ctxt, XML_ERR_SPACE_REQUIRED, "Blank needed here\n");
    }

    SKIP_BLANKS;
    if ((RAW == '?') && (NXT(1) == '>')) {
        SKIP(2);
    } else if (RAW == '>') {
        /* Deprecated old WD ... */
	xmlFatalErr(ctxt, XML_ERR_XMLDECL_NOT_FINISHED, NULL);
	NEXT;
    } else {
	xmlFatalErr(ctxt, XML_ERR_XMLDECL_NOT_FINISHED, NULL);
	hw_MOVETO_ENDTAG(CUR_PTR);
	NEXT;
    }
}

/**
 * hw_xmlParseMisc:
 * @ctxt:  an XML parser context
 * 
 * parse an XML Misc* optional field.
 *
 * [27] Misc ::= Comment | PI |  S
 */

void
hw_xmlParseMisc(hw_xmlParserCtxtPtr ctxt) {
    while (((RAW == '<') && (NXT(1) == '?')) ||
           (CMP4(CUR_PTR, '<', '!', '-', '-')) ||
           hw_IS_BLANK_CH(CUR)) {
        if (hw_IS_BLANK_CH(CUR)) {
	    NEXT;
	} else
	    hw_xmlParseComment(ctxt);
    }
}

/**
 * hw_xmlParseDocument:
 * @ctxt:  an XML parser context
 * 
 * parse an XML document (and build a tree if using the standard SAX
 * interface).
 *
 * [1] document ::= prolog element Misc*
 *
 * [22] prolog ::= XMLDecl? Misc* (doctypedecl Misc*)?
 *
 * Returns 0, -1 in case of error. the parser context is augmented
 *                as a result of the parsing.
 */

int
hw_xmlParseDocument(hw_xmlParserCtxtPtr ctxt) {
    hw_xmlChar start[4];
    hw_xmlCharEncoding enc;

    hw_xmlInitParser();

    if ((ctxt == NULL) || (ctxt->input == NULL))
        return(-1);

    GROW;

    /*
     * SAX: detecting the level.
     */
    xmlDetectSAX2(ctxt);

    /*
     * SAX: beginning of the document processing.
     */

    if ((ctxt->encoding == (const hw_xmlChar *)XML_CHAR_ENCODING_NONE) &&
        ((ctxt->input->end - ctxt->input->cur) >= 4)) {
	/* 
	 * Get the 4 first bytes and decode the charset
	 * if enc != XML_CHAR_ENCODING_NONE
	 * plug some encoding conversion routines.
	 */
	start[0] = RAW;
	start[1] = NXT(1);
	start[2] = NXT(2);
	start[3] = NXT(3);
	enc = hw_xmlDetectCharEncoding(&start[0], 4);
	if (enc != XML_CHAR_ENCODING_NONE) {
	    hw_xmlSwitchEncoding(ctxt, enc);
	}
    }


    if (CUR == 0) {
	xmlFatalErr(ctxt, XML_ERR_DOCUMENT_EMPTY, NULL);
    }

    /*
     * Check for the XMLDecl in the Prolog.
     */
    GROW;
    if ((CMP5(CUR_PTR, '<', '?', 'x', 'm', 'l')) && (hw_IS_BLANK_CH(NXT(5)))) {

	/*
	 * Note that we will switch encoding on the fly.
	 */
	hw_xmlParseXMLDecl(ctxt);
	if (ctxt->errNo == XML_ERR_UNSUPPORTED_ENCODING) {
	    /*
	     * The XML REC instructs us to stop parsing right here
	     */
	    return(-1);
	}
	SKIP_BLANKS;
    } else {
	ctxt->version = hw_xmlCharStrdup(hw_XML_DEFAULT_VERSION);
    }
    if ((ctxt->sax) && (ctxt->sax->startDocument) && (!ctxt->disableSAX))
        ctxt->sax->startDocument(ctxt->userData);

    /*
     * The Misc part of the Prolog
     */
    GROW;
    hw_xmlParseMisc(ctxt);

    /*
     * Time to start parsing the tree itself
     */
    GROW;
    if (RAW != '<') {
	xmlFatalErrMsg(ctxt, XML_ERR_DOCUMENT_EMPTY,
		       "Start tag expected, '<' not found\n");
    } else {
	ctxt->instate = XML_PARSER_CONTENT;
	hw_xmlParseElement(ctxt);
	ctxt->instate = XML_PARSER_EPILOG;


	/*
	 * The Misc part at the end
	 */
	hw_xmlParseMisc(ctxt);

	if (RAW != 0) {
	    xmlFatalErr(ctxt, XML_ERR_DOCUMENT_END, NULL);
	}
	ctxt->instate = XML_PARSER_EOF;
    }

    /*
     * SAX: end of the document processing.
     */
    if ((ctxt->sax) && (ctxt->sax->endDocument != NULL))
        ctxt->sax->endDocument(ctxt->userData);

    /*
     * Remove locally kept entity definitions if the tree was not built
     */
    if ((ctxt->myDoc != NULL) &&
	(hw_xmlStrEqual(ctxt->myDoc->version, SAX_COMPAT_MODE))) {
	hw_xmlFreeDoc(ctxt->myDoc);
	ctxt->myDoc = NULL;
    }

    if (! ctxt->wellFormed) {
	ctxt->valid = 0;
	return(-1);
    }
    return(0);
}


/************************************************************************
 *									*
 * 		Front ends when parsing an Entity			*
 *									*
 ************************************************************************/

/**
 * hw_xmlParseCtxtExternalEntity:
 * @ctx:  the existing parsing context
 * @URL:  the URL for the entity to load
 * @ID:  the System ID for the entity to load
 * @lst:  the return value for the set of parsed nodes
 *
 * Parse an external general entity within an existing parsing context
 * An external general parsed entity is well-formed if it matches the
 * production labeled extParsedEnt.
 *
 * [78] extParsedEnt ::= TextDecl? content
 *
 * Returns 0 if the entity is well formed, -1 in case of args problem and
 *    the parser error code otherwise
 */

int
hw_xmlParseCtxtExternalEntity(hw_xmlParserCtxtPtr ctx, const hw_xmlChar *URL,
	               const hw_xmlChar *ID, hw_xmlNodePtr *lst) {
    hw_xmlParserCtxtPtr ctxt;
    hw_xmlDocPtr newDoc;
    hw_xmlNodePtr newRoot;
    hw_xmlSAXHandlerPtr oldsax = NULL;
    int ret = 0;
    hw_xmlChar start[4];
    hw_xmlCharEncoding enc;

    if (ctx == NULL) return(-1);

    if (ctx->depth > 40) {
	return(XML_ERR_ENTITY_LOOP);
    }

    if (lst != NULL)
        *lst = NULL;
    if ((URL == NULL) && (ID == NULL))
	return(-1);
    if (ctx->myDoc == NULL) /* @@ relax but check for dereferences */
	return(-1);


    ctxt = hw_xmlCreateEntityParserCtxt(URL, ID, NULL);
    if (ctxt == NULL) return(-1);
    ctxt->userData = ctxt;
    ctxt->_private = ctx->_private;
    oldsax = ctxt->sax;
    ctxt->sax = ctx->sax;
    xmlDetectSAX2(ctxt);
    newDoc = hw_xmlNewDoc(hw_BAD_CAST "1.0");
    if (newDoc == NULL) {
	hw_xmlFreeParserCtxt(ctxt);
	return(-1);
    }
    if (ctx->myDoc->dict) {
	newDoc->dict = ctx->myDoc->dict;
	hw_xmlDictReference(newDoc->dict);
    }
    if (ctx->myDoc != NULL) {
	newDoc->intSubset = ctx->myDoc->intSubset;
	newDoc->extSubset = ctx->myDoc->extSubset;
    }
    if (ctx->myDoc->URL != NULL) {
	newDoc->URL = hw_xmlStrdup(ctx->myDoc->URL);
    }
    newRoot = hw_xmlNewDocNode(newDoc, NULL, hw_BAD_CAST "pseudoroot", NULL);
    if (newRoot == NULL) {
	ctxt->sax = oldsax;
	hw_xmlFreeParserCtxt(ctxt);
	newDoc->intSubset = NULL;
	newDoc->extSubset = NULL;
        hw_xmlFreeDoc(newDoc);
	return(-1);
    }
    hw_xmlAddChild((hw_xmlNodePtr) newDoc, newRoot);
    hw_nodePush(ctxt, newDoc->children);
    if (ctx->myDoc == NULL) {
	ctxt->myDoc = newDoc;
    } else {
	ctxt->myDoc = ctx->myDoc;
	newDoc->children->doc = ctx->myDoc;
    }

    /* 
     * Get the 4 first bytes and decode the charset
     * if enc != XML_CHAR_ENCODING_NONE
     * plug some encoding conversion routines.
     */
    GROW
    if ((ctxt->input->end - ctxt->input->cur) >= 4) {
	start[0] = RAW;
	start[1] = NXT(1);
	start[2] = NXT(2);
	start[3] = NXT(3);
	enc = hw_xmlDetectCharEncoding(start, 4);
	if (enc != XML_CHAR_ENCODING_NONE) {
	    hw_xmlSwitchEncoding(ctxt, enc);
	}
    }

    /*
     * Parse a possible text declaration first
     */
    if ((CMP5(CUR_PTR, '<', '?', 'x', 'm', 'l')) && (hw_IS_BLANK_CH(NXT(5)))) {
	hw_xmlParseTextDecl(ctxt);
    }

    /*
     * Doing validity checking on chunk doesn't make sense
     */
    ctxt->instate = XML_PARSER_CONTENT;
    ctxt->valid = ctx->valid;
    ctxt->depth = ctx->depth + 1;
    ctxt->replaceEntities = ctx->replaceEntities;
    {
	ctxt->vctxt.error = NULL;
	ctxt->vctxt.warning = NULL;
    }
    ctxt->vctxt.nodeTab = NULL;
    ctxt->vctxt.nodeNr = 0;
    ctxt->vctxt.nodeMax = 0;
    ctxt->vctxt.node = NULL;
    if (ctxt->dict != NULL) hw_xmlDictFree(ctxt->dict);
    ctxt->dict = ctx->dict;
    ctxt->str_xml = hw_xmlDictLookup(ctxt->dict, hw_BAD_CAST "xml", 3);
    ctxt->str_xmlns = hw_xmlDictLookup(ctxt->dict, hw_BAD_CAST "xmlns", 5);
    ctxt->str_xml_ns = hw_xmlDictLookup(ctxt->dict, hw_XML_XML_NAMESPACE, 36);
    ctxt->dictNames = ctx->dictNames;
    ctxt->attsDefault = ctx->attsDefault;
    ctxt->attsSpecial = ctx->attsSpecial;
    ctxt->linenumbers = ctx->linenumbers;

    hw_xmlParseContent(ctxt);
   
    ctx->valid = ctxt->valid;
    if ((RAW == '<') && (NXT(1) == '/')) {
	xmlFatalErr(ctxt, XML_ERR_NOT_WELL_BALANCED, NULL);
    } else if (RAW != 0) {
	xmlFatalErr(ctxt, XML_ERR_EXTRA_CONTENT, NULL);
    }
    if (ctxt->node != newDoc->children) {
	xmlFatalErr(ctxt, XML_ERR_NOT_WELL_BALANCED, NULL);
    }

    if (!ctxt->wellFormed) {
        if (ctxt->errNo == 0)
	    ret = 1;
	else
	    ret = ctxt->errNo;
    } else {
	if (lst != NULL) {
	    hw_xmlNodePtr cur;

	    /*
	     * Return the newly created nodeset after unlinking it from
	     * they pseudo parent.
	     */
	    cur = newDoc->children->children;
	    *lst = cur;
	    while (cur != NULL) {
		cur->parent = NULL;
		cur = cur->next;
	    }
            newDoc->children->children = NULL;
	}
	ret = 0;
    }
    ctxt->sax = oldsax;
    ctxt->dict = NULL;
    ctxt->attsDefault = NULL;
    ctxt->attsSpecial = NULL;
    hw_xmlFreeParserCtxt(ctxt);
    newDoc->intSubset = NULL;
    newDoc->extSubset = NULL;
    hw_xmlFreeDoc(newDoc);
    
    return(ret);
}

/**
 * hw_xmlCreateEntityParserCtxt:
 * @URL:  the entity URL
 * @ID:  the entity PUBLIC ID
 * @base:  a possible base for the target URI
 *
 * Create a parser context for an external entity
 * Automatic support for ZLIB/Compress compressed document is provided
 * by default if found at compile-time.
 *
 * Returns the new parser context or NULL
 */
hw_xmlParserCtxtPtr
hw_xmlCreateEntityParserCtxt(const hw_xmlChar *URL, const hw_xmlChar *ID,
	                  const hw_xmlChar *base) {
    hw_xmlParserCtxtPtr ctxt;
    hw_xmlParserInputPtr inputStream;
    char *directory = NULL;
    hw_xmlChar *uri;
    
    ctxt = hw_xmlNewParserCtxt();
    if (ctxt == NULL) {
	return(NULL);
    }

    uri = hw_xmlBuildURI(URL, base);

    if (uri == NULL) {
	inputStream = hw_xmlLoadExternalEntity((char *)URL, (char *)ID, ctxt);
	if (inputStream == NULL) {
	    hw_xmlFreeParserCtxt(ctxt);
	    return(NULL);
	}

	hw_inputPush(ctxt, inputStream);

	if ((ctxt->directory == NULL) && (directory == NULL))
	    directory = hw_xmlParserGetDirectory((char *)URL);
	if ((ctxt->directory == NULL) && (directory != NULL))
	    ctxt->directory = directory;
    } else {
	inputStream = hw_xmlLoadExternalEntity((char *)uri, (char *)ID, ctxt);
	if (inputStream == NULL) {
	    hw_xmlFree(uri);
	    hw_xmlFreeParserCtxt(ctxt);
	    return(NULL);
	}

	hw_inputPush(ctxt, inputStream);

	if ((ctxt->directory == NULL) && (directory == NULL))
	    directory = hw_xmlParserGetDirectory((char *)uri);
	if ((ctxt->directory == NULL) && (directory != NULL))
	    ctxt->directory = directory;
	hw_xmlFree(uri);
    }
    return(ctxt);
}

/************************************************************************
 *									*
 * 		Front ends when parsing from a file			*
 *									*
 ************************************************************************/

/**
 * xmlCreateURLParserCtxt:
 * @filename:  the filename or URL
 * @options:  a combination of hw_xmlParserOption
 *
 * Create a parser context for a file or URL content. 
 * Automatic support for ZLIB/Compress compressed document is provided
 * by default if found at compile-time and for file accesses
 *
 * Returns the new parser context or NULL
 */
hw_xmlParserCtxtPtr
hw_xmlCreateURLParserCtxt(const char *filename, int options)
{
    hw_xmlParserCtxtPtr ctxt;
    hw_xmlParserInputPtr inputStream;
    char *directory = NULL;

    ctxt = hw_xmlNewParserCtxt();
    if (ctxt == NULL) {
	hw_xmlErrMemory(NULL, "cannot allocate parser context");
	return(NULL);
    }

    if (options)
	hw_xmlCtxtUseOptions(ctxt, options);
    ctxt->linenumbers = 1;
    
    inputStream = hw_xmlLoadExternalEntity(filename, NULL, ctxt);
    if (inputStream == NULL) {
	hw_xmlFreeParserCtxt(ctxt);
	return(NULL);
    }

    hw_inputPush(ctxt, inputStream);
    if ((ctxt->directory == NULL) && (directory == NULL))
        directory = hw_xmlParserGetDirectory(filename);

    if ((ctxt->directory == NULL) && (directory != NULL))
        ctxt->directory = directory;

    return(ctxt);
}


/************************************************************************
 *									*
 * 		Front ends when parsing from memory			*
 *									*
 ************************************************************************/

/**
 * hw_xmlCreateMemoryParserCtxt:
 * @buffer:  a pointer to a char array
 * @size:  the size of the array
 *
 * Create a parser context for an XML in-memory document.
 *
 * Returns the new parser context or NULL
 */
hw_xmlParserCtxtPtr
hw_xmlCreateMemoryParserCtxt(const char *buffer, int size) {
    hw_xmlParserCtxtPtr ctxt;
    hw_xmlParserInputPtr input;
    hw_xmlParserInputBufferPtr buf;

    if (buffer == NULL)
	return(NULL);
    if (size <= 0)
	return(NULL);

    ctxt = hw_xmlNewParserCtxt();
    if (ctxt == NULL)
	return(NULL);

    /* TODO: hw_xmlParserInputBufferCreateStatic, requires some serious changes */
    buf = hw_xmlParserInputBufferCreateMem(buffer, size, XML_CHAR_ENCODING_NONE);
    if (buf == NULL) {
	hw_xmlFreeParserCtxt(ctxt);
	return(NULL);
    }

    input = hw_xmlNewInputStream(ctxt);
    if (input == NULL) {
	hw_xmlFreeParserInputBuffer(buf);
	hw_xmlFreeParserCtxt(ctxt);
	return(NULL);
    }

    input->filename = NULL;
    input->buf = buf;
    input->base = input->buf->buffer->content;
    input->cur = input->buf->buffer->content;
    input->end = &input->buf->buffer->content[input->buf->buffer->use];

    hw_inputPush(ctxt, input);
    return(ctxt);
}


/************************************************************************
 *									*
 * 				Miscellaneous				*
 *									*
 ************************************************************************/


extern void hw_XMLCDECL xmlGenericErrorDefaultFunc(void *ctx, const char *msg, ...);
static int xmlParserInitialized = 0;

/**
 * hw_xmlInitParser:
 *
 * Initialization function for the XML parser.
 * This is not reentrant. Call once before processing in case of
 * use in multithreaded programs.
 */

void
hw_xmlInitParser(void) {
    if (xmlParserInitialized != 0)
	return;

    if ((hw_xmlGenericError == xmlGenericErrorDefaultFunc) ||
	(hw_xmlGenericError == NULL))
	hw_initGenericErrorDefaultFunc(NULL);
    hw_xmlInitCharEncodingHandlers();
    hw_xmlRegisterDefaultInputCallbacks();
    xmlParserInitialized = 1;
}

/**
 * hw_xmlCleanupParser:
 *
 * Cleanup function for the XML library. It tries to reclaim all
 * parsing related global memory allocated for the library processing.
 * It doesn't deallocate any document related memory. Calling this
 * function should not prevent reusing the library but one should
 * call hw_xmlCleanupParser() only when the process has
 * finished using the library or XML document built with it.
 */

void
hw_xmlCleanupParser(void) {
    if (!xmlParserInitialized)
	return;

    hw_xmlCleanupCharEncodingHandlers();
    hw_xmlDictCleanup();
    hw_xmlCleanupInputCallbacks();
    hw_xmlResetLastError();
    xmlParserInitialized = 0;
}

/************************************************************************
 *									*
 *	New set (2.6.0) of simpler and more flexible APIs		*
 *									*
 ************************************************************************/

/**
 * hw_DICT_FREE:
 * @str:  a string
 *
 * Free a string if it is not owned by the "dict" dictionnary in the
 * current scope
 */
#define DICT_FREE(str)						\
	if ((str) && ((!dict) || 				\
	    (hw_xmlDictOwns(dict, (const hw_xmlChar *)(str)) == 0)))	\
	    hw_xmlFree((char *)(str));


/**
 * hw_xmlCtxtUseOptions:
 * @ctxt: an XML parser context
 * @options:  a combination of hw_xmlParserOption
 *
 * Applies the options to the parser context
 *
 * Returns 0 in case of success, the set of unknown or unimplemented options
 *         in case of error.
 */
int
hw_xmlCtxtUseOptions(hw_xmlParserCtxtPtr ctxt, int options)
{
    if (ctxt == NULL)
        return(-1);
    if (options & XML_PARSE_RECOVER) {
        ctxt->recovery = 1;
        options -= XML_PARSE_RECOVER;
    } else
        ctxt->recovery = 0;

    if (options & XML_PARSE_NOENT) {
        ctxt->replaceEntities = 1;
        options -= XML_PARSE_NOENT;
    } else
    {
        ctxt->replaceEntities = 0;
    }

    ctxt->keepBlanks = 1;
 
    if (options & XML_PARSE_NOWARNING) {
        ctxt->sax->warning = NULL;
        options -= XML_PARSE_NOWARNING;
    }
    if (options & XML_PARSE_NOERROR) {
        ctxt->sax->error = NULL;
        ctxt->sax->fatalError = NULL;
        options -= XML_PARSE_NOERROR;
    }
    if (options & XML_PARSE_NODICT) {
        ctxt->dictNames = 0;
        options -= XML_PARSE_NODICT;
    } else {
        ctxt->dictNames = 1;
    }
    ctxt->linenumbers = 1;
    return (options);
}

/**
 * hw_xmlDoRead:
 * @ctxt:  an XML parser context
 * @URL:  the base URL to use for the document
 * @encoding:  the document encoding, or NULL
 * @options:  a combination of hw_xmlParserOption
 * @reuse:  keep the context for reuse
 *
 * Common front-end for the xmlRead functions
 * 
 * Returns the resulting document tree or NULL
 */
static hw_xmlDocPtr
hw_xmlDoRead(hw_xmlParserCtxtPtr ctxt, const char *URL, const char *encoding,
          int options, int reuse)
{
    hw_xmlDocPtr ret;
    
    hw_xmlCtxtUseOptions(ctxt, options);
    if (encoding != NULL) {
        hw_xmlCharEncodingHandlerPtr hdlr;

	hdlr = hw_xmlFindCharEncodingHandler(encoding);
	if (hdlr != NULL)
	    hw_xmlSwitchToEncoding(ctxt, hdlr);
    }

    if ((URL != NULL) && (ctxt->input != NULL) &&
        (ctxt->input->filename == NULL))
        ctxt->input->filename = (char *) hw_xmlStrdup((const hw_xmlChar *) URL);
    hw_xmlParseDocument(ctxt);
    if ((ctxt->wellFormed) || ctxt->recovery)
        ret = ctxt->myDoc;
    else {
        ret = NULL;
	if (ctxt->myDoc != NULL) {
	    hw_xmlFreeDoc(ctxt->myDoc);
	}
    }
    ctxt->myDoc = NULL;
    if (!reuse) {
	hw_xmlFreeParserCtxt(ctxt);
    }

    return (ret);
}

/**
 * hw_xmlReadFile:
 * @filename:  a file or URL
 * @encoding:  the document encoding, or NULL
 * @options:  a combination of hw_xmlParserOption
 *
 * parse an XML file from the filesystem or the network.
 * 
 * Returns the resulting document tree
 */
hw_xmlDocPtr
hw_xmlReadFile(const char *filename, const char *encoding, int options)
{
    hw_xmlParserCtxtPtr ctxt;
    ctxt = hw_xmlCreateURLParserCtxt(filename, options);
    if (ctxt == NULL)
        return (NULL);
    return (hw_xmlDoRead(ctxt, NULL, encoding, options, 0));
}

