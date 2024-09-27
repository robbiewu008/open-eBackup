/*
 * Summary: the core parser module
 * Description: Interfaces, constants and types related to the XML parser
 *
 * Copy: See Copyright for the status of this software.
 *
 * Author: Daniel Veillard
 */

#ifndef __XML_PARSER_H__
#define __XML_PARSER_H__

#include <stdarg.h>

#include "Etree.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * hw_XML_DEFAULT_VERSION:
 *
 * The default version of XML used: 1.0
 */
#define hw_XML_DEFAULT_VERSION	"1.0"

/**
 * hw_xmlParserInput:
 *
 * An hw_xmlParserInput is an input flow for the XML processor.
 * Each entity parsed is associated an hw_xmlParserInput (except the
 * few predefined ones). This is the case both for internal entities
 * - in which case the flow is already completely in memory - or
 * external entities - in which case we use the buf structure for
 * progressive reading and I18N conversions to the internal UTF-8 format.
 */

/**
 * hw_\xmlParserInputDeallocate:
 * @str:  the string to deallocate
 *
 * Callback for freeing some parser input allocations.
 */
typedef void (* hw_xmlParserInputDeallocate)(hw_xmlChar *str);

struct hw__xmlParserInput {
    /* Input buffer */
    hw_xmlParserInputBufferPtr buf;      /* UTF-8 encoded buffer */

    const char *filename;             /* The file analyzed, if any */
    const char *directory;            /* the directory/base of the file */
    const hw_xmlChar *base;              /* Base of the array to parse */
    const hw_xmlChar *cur;               /* Current char being parsed */
    const hw_xmlChar *end;               /* end of the array to parse */
    int length;                       /* length if known */
    int line;                         /* Current line */
    int col;                          /* Current column */
    /*
     * NOTE: consumed is only tested for equality in the parser code,
     *       so even if there is an overflow this should not give troubles
     *       for parsing very large instances.
     */
    unsigned long consumed;           /* How many xmlChars already consumed */
    hw_xmlParserInputDeallocate free;    /* function to deallocate the base */
    const hw_xmlChar *encoding;          /* the encoding string for entity */
    const hw_xmlChar *version;           /* the version string for entity */
    int id;                           /* an unique identifier for the entity */
};

/**
 * hw_xmlParserNodeInfo:
 *
 * The parser can be asked to collect Node informations, i.e. at what
 * place in the file they were detected. 
 * NOTE: This is off by default and not very well tested.
 */
typedef struct hw__xmlParserNodeInfo hw_xmlParserNodeInfo;
typedef hw_xmlParserNodeInfo *hw_xmlParserNodeInfoPtr;

struct hw__xmlParserNodeInfo {
  const struct hw__xmlNode* node;
  /* Position & line # that text that created the node begins & ends on */
  unsigned long begin_pos;
  unsigned long begin_line;
  unsigned long end_pos;
  unsigned long end_line;
};

typedef struct hw__xmlParserNodeInfoSeq hw_xmlParserNodeInfoSeq;
typedef hw_xmlParserNodeInfoSeq *hw_xmlParserNodeInfoSeqPtr;
struct hw__xmlParserNodeInfoSeq {
  unsigned long maximum;
  unsigned long length;
  hw_xmlParserNodeInfo* buffer;
};

/**
 * hw_xmlParserInputState:
 *
 * The parser is now working also as a state based parser.
 * The recursive one use the state info for entities processing.
 */
typedef enum {
    XML_PARSER_EOF = -1,	/* nothing is to be parsed */
    XML_PARSER_START = 0,	/* nothing has been parsed */
    XML_PARSER_MISC,		/* Misc* before int subset */
    XML_PARSER_PI,		/* Within a processing instruction */
    XML_PARSER_DTD,		/* within some DTD content */
    XML_PARSER_PROLOG,		/* Misc* after internal subset */
    XML_PARSER_COMMENT,		/* within a comment */
    XML_PARSER_START_TAG,	/* within a start tag */
    XML_PARSER_CONTENT,		/* within the content */
    XML_PARSER_CDATA_SECTION,	/* within a CDATA section */
    XML_PARSER_END_TAG,		/* within a closing tag */
    XML_PARSER_ENTITY_DECL,	/* within an entity declaration */
    XML_PARSER_ENTITY_VALUE,	/* within an entity value in a decl */
    XML_PARSER_ATTRIBUTE_VALUE,	/* within an attribute value */
    XML_PARSER_SYSTEM_LITERAL,	/* within a SYSTEM value */
    XML_PARSER_EPILOG, 		/* the Misc* after the last end tag */
    XML_PARSER_IGNORE,		/* within an IGNORED section */
    XML_PARSER_PUBLIC_LITERAL 	/* within a PUBLIC value */
} hw_xmlParserInputState;


/**
 * hw_xmlParserMode:
 *
 * A parser can operate in various modes
 */
typedef enum {
    XML_PARSE_UNKNOWN = 0,
    XML_PARSE_DOM = 1,
    XML_PARSE_SAX = 2,
    XML_PARSE_PUSH_DOM = 3,
    XML_PARSE_PUSH_SAX = 4,
    XML_PARSE_READER = 5
} hw_xmlParserMode;

/**
 * hw_xmlParserCtxt:
 *
 * The parser context.
 * NOTE This doesn't completely define the parser state, the (current ?)
 *      design of the parser uses recursive function calls since this allow
 *      and easy mapping from the production rules of the specification
 *      to the actual code. The drawback is that the actual function call
 *      also reflect the parser state. However most of the parsing routines
 *      takes as the only argument the parser context pointer, so migrating
 *      to a state based parser for progressive parsing shouldn't be too hard.
 */
struct hw__xmlParserCtxt {
    struct hw__xmlSAXHandler *sax;       /* The SAX handler */
    void            *userData;        /* For SAX interface only, used by DOM build */
    hw_xmlDocPtr           myDoc;        /* the document being built */
    int            wellFormed;        /* is the document well formed */
    int       replaceEntities;        /* shall we replace entities ? */
    const hw_xmlChar    *version;        /* the XML version string */
    const hw_xmlChar   *encoding;        /* the declared encoding, if any */
    int            standalone;        /* standalone document */
    int                  html;        /* an HTML(1)/Docbook(2) document */

    /* Input stream stack */
    hw_xmlParserInputPtr  input;         /* Current input stream */
    int                inputNr;       /* Number of current input streams */
    int                inputMax;      /* Max number of input streams */
    hw_xmlParserInputPtr *inputTab;      /* stack of inputs */

    /* Node analysis stack only used for DOM building */
    hw_xmlNodePtr         node;          /* Current parsed Node */
    int                nodeNr;        /* Depth of the parsing stack */
    int                nodeMax;       /* Max depth of the parsing stack */
    hw_xmlNodePtr        *nodeTab;       /* array of nodes */

    int record_info;                  /* Whether node info should be kept */
    hw_xmlParserNodeInfoSeq node_seq;    /* info about each node parsed */

    int errNo;                        /* error code */

    int     hasExternalSubset;        /* reference and external subset */
    int             hasPErefs;        /* the internal subset has PE refs */
    int              external;        /* are we parsing an external entity */

    int                 valid;        /* is the document valid */
    hw_xmlValidCtxt        vctxt;        /* The validity context */

    hw_xmlParserInputState instate;      /* current type of input */
    int                 token;        /* next char look-ahead */    

    char           *directory;        /* the data directory */

    /* Node name stack */
    const hw_xmlChar     *name;          /* Current parsed Node */
    int                nameNr;        /* Depth of the parsing stack */
    int                nameMax;       /* Max depth of the parsing stack */
    const hw_xmlChar *   *nameTab;       /* array of nodes */

    long               nbChars;       /* number of hw_xmlChar processed */
    long            checkIndex;       /* used by progressive parsing lookup */
    int             keepBlanks;       /* ugly but ... */
    int             disableSAX;       /* SAX callbacks are disabled */
    int               inSubset;       /* Parsing is in int 1/ext 2 subset */
    const hw_xmlChar *    intSubName;    /* name of subset */
    /* xml:space values */
    int *              space;         /* Should the parser preserve spaces */
    int                spaceNr;       /* Depth of the parsing stack */
    int                spaceMax;      /* Max depth of the parsing stack */
    int *              spaceTab;      /* array of space infos */

    int                depth;         /* to prevent entity substitution loops */
    hw_xmlParserInputPtr  entity;        /* used to check entities boundaries */
    int                charset;       /* encoding of the in-memory content
				         actually an hw_xmlCharEncoding */
    int                nodelen;       /* Those two fields are there to */
    int                nodemem;       /* Speed up large node parsing */
    void              *_private;      /* For user data, libxml won't touch it */

    int                linenumbers;   /* set line number in element content */
    void              *catalogs;      /* document's own catalog */
    int                recovery;      /* run in recovery mode */
    int                progressive;   /* is this a progressive parsing */
    hw_xmlDictPtr         dict;          /* dictionnary for the parser */
    const hw_xmlChar *   *atts;          /* array for the attributes callbacks */
    int                maxatts;       /* the size of the array */
    int                docdict;       /* use strings from dict to build tree */

    /*
     * pre-interned strings
     */
    const hw_xmlChar *str_xml;
    const hw_xmlChar *str_xmlns;
    const hw_xmlChar *str_xml_ns;

    /*
     * Everything below is used only by the new SAX mode
     */
    int                sax2;          /* operating in the new SAX mode */
    int                nsNr;          /* the number of inherited namespaces */
    int                nsMax;         /* the size of the arrays */
    const hw_xmlChar *   *nsTab;         /* the array of prefix/namespace name */
    int               *attallocs;     /* which attribute were allocated */
    void *            *pushTab;       /* array of data for push */
    hw_xmlHashTablePtr    attsDefault;   /* defaulted attributes if any */
    hw_xmlHashTablePtr    attsSpecial;   /* non-CDATA attributes if any */
    int                nsWellFormed;  /* is the document XML Nanespace okay */

    /*
     * Those fields are needed only for treaming parsing so far
     */
    int               dictNames;    /* Use dictionary names for the tree */
    int               freeElemsNr;  /* number of freed element nodes */
    hw_xmlNodePtr        freeElems;    /* List of freed element nodes */
    int               freeAttrsNr;  /* number of freed attributes nodes */
    hw_xmlAttrPtr        freeAttrs;    /* List of freed attributes nodes */

    /*
     * the complete error informations for the last error.
     */
    hw_xmlError          lastError;
    hw_xmlParserMode     parseMode;    /* the parser mode */
};

/**
 * hw_xmlSAXLocator:
 *
 * A SAX Locator.
 */
struct hw__xmlSAXLocator {
    const hw_xmlChar *(*getPublicId)(void *ctx);
    const hw_xmlChar *(*getSystemId)(void *ctx);
    int (*getLineNumber)(void *ctx);
    int (*getColumnNumber)(void *ctx);
};

/**
 * hw_xmlSAXHandler:
 *
 * A SAX handler is bunch of callbacks called by the parser when processing
 * of the input generate data or structure informations.
 */

/**
 * hw_resolveEntitySAXFunc:
 * @ctx:  the user data (XML parser context)
 * @publicId: The public ID of the entity
 * @systemId: The system ID of the entity
 *
 * Callback:
 * The entity loader, to control the loading of external entities,
 * the application can either:
 *    - override this resolveEntity() callback in the SAX block
 *    - or better use the hw_xmlSetExternalEntityLoader() function to
 *      set up it's own entity resolution routine
 *
 * Returns the hw_xmlParserInputPtr if inlined or NULL for DOM behaviour.
 */
typedef hw_xmlParserInputPtr (*hw_resolveEntitySAXFunc) (void *ctx,
				const hw_xmlChar *publicId,
				const hw_xmlChar *systemId);
/**
 * hw_internalSubsetSAXFunc:
 * @ctx:  the user data (XML parser context)
 * @name:  the root element name
 * @ExternalID:  the external ID
 * @SystemID:  the SYSTEM ID (e.g. filename or URL)
 *
 * Callback on internal subset declaration.
 */
typedef void (*hw_internalSubsetSAXFunc) (void *ctx,
				const hw_xmlChar *name,
				const hw_xmlChar *ExternalID,
				const hw_xmlChar *SystemID);
/**
 * hw_externalSubsetSAXFunc:
 * @ctx:  the user data (XML parser context)
 * @name:  the root element name
 * @ExternalID:  the external ID
 * @SystemID:  the SYSTEM ID (e.g. filename or URL)
 *
 * Callback on external subset declaration.
 */
typedef void (*hw_externalSubsetSAXFunc) (void *ctx,
				const hw_xmlChar *name,
				const hw_xmlChar *ExternalID,
				const hw_xmlChar *SystemID);
/**
 * hw_getEntitySAXFunc:
 * @ctx:  the user data (XML parser context)
 * @name: The entity name
 *
 * Get an entity by name.
 *
 * Returns the hw_xmlEntityPtr if found.
 */
typedef hw_xmlEntityPtr (*hw_getEntitySAXFunc) (void *ctx,
				const hw_xmlChar *name);
/**
 * hw_getParameterEntitySAXFunc:
 * @ctx:  the user data (XML parser context)
 * @name: The entity name
 *
 * Get a parameter entity by name.
 *
 * Returns the hw_xmlEntityPtr if found.
 */
typedef hw_xmlEntityPtr (*hw_getParameterEntitySAXFunc) (void *ctx,
				const hw_xmlChar *name);
/**
 * hw_entityDeclSAXFunc:
 * @ctx:  the user data (XML parser context)
 * @name:  the entity name 
 * @type:  the entity type 
 * @publicId: The public ID of the entity
 * @systemId: The system ID of the entity
 * @content: the entity value (without processing).
 *
 * An entity definition has been parsed.
 */
typedef void (*hw_entityDeclSAXFunc) (void *ctx,
				const hw_xmlChar *name,
				int type,
				const hw_xmlChar *publicId,
				const hw_xmlChar *systemId,
				hw_xmlChar *content);
/**
 * hw_notationDeclSAXFunc:
 * @ctx:  the user data (XML parser context)
 * @name: The name of the notation
 * @publicId: The public ID of the entity
 * @systemId: The system ID of the entity
 *
 * What to do when a notation declaration has been parsed.
 */
typedef void (*hw_notationDeclSAXFunc)(void *ctx,
				const hw_xmlChar *name,
				const hw_xmlChar *publicId,
				const hw_xmlChar *systemId);
/**
 * hw_attributeDeclSAXFunc:
 * @ctx:  the user data (XML parser context)
 * @elem:  the name of the element
 * @fullname:  the attribute name 
 * @type:  the attribute type 
 * @def:  the type of default value
 * @defaultValue: the attribute default value
 * @tree:  the tree of enumerated value set
 *
 * An attribute definition has been parsed.
 */
typedef void (*hw_attributeDeclSAXFunc)(void *ctx,
				const hw_xmlChar *elem,
				const hw_xmlChar *fullname,
				int type,
				int def,
				const hw_xmlChar *defaultValue,
				hw_xmlEnumerationPtr tree);
/**
 * hw_elementDeclSAXFunc:
 * @ctx:  the user data (XML parser context)
 * @name:  the element name 
 * @type:  the element type 
 * @content: the element value tree
 *
 * An element definition has been parsed.
 */
typedef void (*hw_elementDeclSAXFunc)(void *ctx,
				const hw_xmlChar *name,
				int type,
				hw_xmlElementContentPtr content);
/**
 * hw_unparsedEntityDeclSAXFunc:
 * @ctx:  the user data (XML parser context)
 * @name: The name of the entity
 * @publicId: The public ID of the entity
 * @systemId: The system ID of the entity
 * @notationName: the name of the notation
 *
 * What to do when an unparsed entity declaration is parsed.
 */
typedef void (*hw_unparsedEntityDeclSAXFunc)(void *ctx,
				const hw_xmlChar *name,
				const hw_xmlChar *publicId,
				const hw_xmlChar *systemId,
				const hw_xmlChar *notationName);
/**
 * hw_startDocumentSAXFunc:
 * @ctx:  the user data (XML parser context)
 *
 * Called when the document start being processed.
 */
typedef void (*hw_startDocumentSAXFunc) (void *ctx);
/**
 * hw_endDocumentSAXFunc:
 * @ctx:  the user data (XML parser context)
 *
 * Called when the document end has been detected.
 */
typedef void (*hw_endDocumentSAXFunc) (void *ctx);
/**
 * hw_startElementSAXFunc:
 * @ctx:  the user data (XML parser context)
 * @name:  The element name, including namespace prefix
 * @atts:  An array of name/value attributes pairs, NULL terminated
 *
 * Called when an opening tag has been processed.
 */
typedef void (*hw_startElementSAXFunc) (void *ctx,
				const hw_xmlChar *name,
				const hw_xmlChar **atts);
/**
 * hw_endElementSAXFunc:
 * @ctx:  the user data (XML parser context)
 * @name:  The element name
 *
 * Called when the end of an element has been detected.
 */
typedef void (*hw_endElementSAXFunc) (void *ctx,
				const hw_xmlChar *name);
/**
 * hw_referenceSAXFunc:
 * @ctx:  the user data (XML parser context)
 * @name:  The entity name
 *
 * Called when an entity reference is detected. 
 */
typedef void (*hw_referenceSAXFunc) (void *ctx,
				const hw_xmlChar *name);
/**
 * hw_charactersSAXFunc:
 * @ctx:  the user data (XML parser context)
 * @ch:  a hw_xmlChar string
 * @len: the number of hw_xmlChar
 *
 * Receiving some chars from the parser.
 */
typedef void (*hw_charactersSAXFunc) (void *ctx,
				const hw_xmlChar *ch,
				int len);
/**
 * hw_ignorableWhitespaceSAXFunc:
 * @ctx:  the user data (XML parser context)
 * @ch:  a hw_xmlChar string
 * @len: the number of hw_xmlChar
 *
 * Receiving some ignorable whitespaces from the parser.
 * UNUSED: by default the DOM building will use characters.
 */
typedef void (*hw_ignorableWhitespaceSAXFunc) (void *ctx,
				const hw_xmlChar *ch,
				int len);
/**
 * hw_processingInstructionSAXFunc:
 * @ctx:  the user data (XML parser context)
 * @target:  the target name
 * @data: the PI data's
 *
 * A processing instruction has been parsed.
 */
typedef void (*hw_processingInstructionSAXFunc) (void *ctx,
				const hw_xmlChar *target,
				const hw_xmlChar *data);
/**
 * hw_commentSAXFunc:
 * @ctx:  the user data (XML parser context)
 * @value:  the comment content
 *
 * A comment has been parsed.
 */
typedef void (*hw_commentSAXFunc) (void *ctx,
				const hw_xmlChar *value);
/**
 * hw_cdataBlockSAXFunc:
 * @ctx:  the user data (XML parser context)
 * @value:  The pcdata content
 * @len:  the block length
 *
 * Called when a pcdata block has been parsed.
 */
typedef void (*hw_cdataBlockSAXFunc) (
	                        void *ctx,
				const hw_xmlChar *value,
				int len);
/**
 * hw_warningSAXFunc:
 * @ctx:  an XML parser context
 * @msg:  the message to display/transmit
 * @...:  extra parameters for the message display
 * 
 * Display and format a warning messages, callback.
 */
typedef void (hw_XMLCDECL *hw_warningSAXFunc) (void *ctx,
				const char *msg, ...);
/**
 * hw_errorSAXFunc:
 * @ctx:  an XML parser context
 * @msg:  the message to display/transmit
 * @...:  extra parameters for the message display
 * 
 * Display and format an error messages, callback.
 */
typedef void (hw_XMLCDECL *hw_errorSAXFunc) (void *ctx,
				const char *msg, ...);
/**
 * hw_fatalErrorSAXFunc:
 * @ctx:  an XML parser context
 * @msg:  the message to display/transmit
 * @...:  extra parameters for the message display
 * 
 * Display and format fatal error messages, callback.
 * Note: so far fatalError() SAX callbacks are not used, error()
 *       get all the callbacks for errors.
 */
typedef void (hw_XMLCDECL *hw_fatalErrorSAXFunc) (void *ctx,
				const char *msg, ...);
/**
 * hw_isStandaloneSAXFunc:
 * @ctx:  the user data (XML parser context)
 *
 * Is this document tagged standalone?
 *
 * Returns 1 if true
 */
typedef int (*hw_isStandaloneSAXFunc) (void *ctx);
/**
 * hw_hasInternalSubsetSAXFunc:
 * @ctx:  the user data (XML parser context)
 *
 * Does this document has an internal subset.
 *
 * Returns 1 if true
 */
typedef int (*hw_hasInternalSubsetSAXFunc) (void *ctx);

/**
 * hasExternalSubsetSAXFunc:
 * @ctx:  the user data (XML parser context)
 *
 * Does this document has an external subset?
 *
 * Returns 1 if true
 */
typedef int (*hasExternalSubsetSAXFunc) (void *ctx);

/************************************************************************
 *									*
 *			The SAX version 2 API extensions		*
 *									*
 ************************************************************************/
/**
 * hw_XML_SAX2_MAGIC:
 *
 * Special constant found in SAX2 blocks initialized fields
 */
#define hw_XML_SAX2_MAGIC 0xDEEDBEAF

/**
 * hw_startElementNsSAX2Func:
 * @ctx:  the user data (XML parser context)
 * @localname:  the local name of the element
 * @prefix:  the element namespace prefix if available
 * @URI:  the element namespace name if available
 * @nb_namespaces:  number of namespace definitions on that node
 * @namespaces:  pointer to the array of prefix/URI pairs namespace definitions
 * @nb_attributes:  the number of attributes on that node
 * @nb_defaulted:  the number of defaulted attributes. The defaulted
 *                  ones are at the end of the array
 * @attributes:  pointer to the array of (localname/prefix/URI/value/end)
 *               attribute values.
 *
 * SAX2 callback when an element start has been detected by the parser.
 * It provides the namespace informations for the element, as well as
 * the new namespace declarations on the element.
 */

typedef void (*hw_startElementNsSAX2Func) (void *ctx,
					const hw_xmlChar *localname,
					const hw_xmlChar *prefix,
					const hw_xmlChar *URI,
					int nb_namespaces,
					const hw_xmlChar **namespaces,
					int nb_attributes,
					int nb_defaulted,
					const hw_xmlChar **attributes);
 
/**
 * hw_endElementNsSAX2Func:
 * @ctx:  the user data (XML parser context)
 * @localname:  the local name of the element
 * @prefix:  the element namespace prefix if available
 * @URI:  the element namespace name if available
 *
 * SAX2 callback when an element end has been detected by the parser.
 * It provides the namespace informations for the element.
 */

typedef void (*hw_endElementNsSAX2Func)   (void *ctx,
					const hw_xmlChar *localname,
					const hw_xmlChar *prefix,
					const hw_xmlChar *URI);


struct hw__xmlSAXHandler {
    hw_getEntitySAXFunc getEntity;
    hw_startDocumentSAXFunc startDocument;
    hw_endDocumentSAXFunc endDocument;
    hw_startElementSAXFunc startElement;
    hw_endElementSAXFunc endElement;
    hw_referenceSAXFunc reference;
    hw_charactersSAXFunc characters;
    hw_ignorableWhitespaceSAXFunc ignorableWhitespace;
    hw_processingInstructionSAXFunc processingInstruction;
    hw_commentSAXFunc comment;
    hw_warningSAXFunc warning;
    hw_errorSAXFunc error;
    hw_fatalErrorSAXFunc fatalError; /* unused error() get all the errors */
    hw_getParameterEntitySAXFunc getParameterEntity;
    unsigned int initialized;
    /* The following fields are extensions available only on version 2 */
    void *_private;
    hw_startElementNsSAX2Func startElementNs;
    hw_endElementNsSAX2Func endElementNs;
    hw_xmlStructuredErrorFunc serror;
};

/*
 * SAX Version 1
 */
typedef struct hw__xmlSAXHandlerV1 hw_xmlSAXHandlerV1;
typedef hw_xmlSAXHandlerV1 *hw_xmlSAXHandlerV1Ptr;
struct hw__xmlSAXHandlerV1 {
    hw_internalSubsetSAXFunc internalSubset;
    hw_isStandaloneSAXFunc isStandalone;
    hw_hasInternalSubsetSAXFunc hasInternalSubset;
    hasExternalSubsetSAXFunc hasExternalSubset;
    hw_resolveEntitySAXFunc resolveEntity;
    hw_getEntitySAXFunc getEntity;
    hw_entityDeclSAXFunc entityDecl;
    hw_notationDeclSAXFunc notationDecl;
    hw_attributeDeclSAXFunc attributeDecl;
    hw_elementDeclSAXFunc elementDecl;
    hw_unparsedEntityDeclSAXFunc unparsedEntityDecl;
    hw_startDocumentSAXFunc startDocument;
    hw_endDocumentSAXFunc endDocument;
    hw_startElementSAXFunc startElement;
    hw_endElementSAXFunc endElement;
    hw_referenceSAXFunc reference;
    hw_charactersSAXFunc characters;
    hw_ignorableWhitespaceSAXFunc ignorableWhitespace;
    hw_processingInstructionSAXFunc processingInstruction;
    hw_commentSAXFunc comment;
    hw_warningSAXFunc warning;
    hw_errorSAXFunc error;
    hw_fatalErrorSAXFunc fatalError; /* unused error() get all the errors */
    hw_getParameterEntitySAXFunc getParameterEntity;
    hw_cdataBlockSAXFunc cdataBlock;
    hw_externalSubsetSAXFunc externalSubset;
    unsigned int initialized;
};


/**
 * hw_xmlExternalEntityLoader:
 * @URL: The System ID of the resource requested
 * @ID: The Public ID of the resource requested
 * @context: the XML parser context 
 *
 * External entity loaders types.
 *
 * Returns the entity input parser.
 */
typedef hw_xmlParserInputPtr (*hw_xmlExternalEntityLoader) (const char *URL,
					 const char *ID,
					 hw_xmlParserCtxtPtr context);

/*
 * Init/Cleanup
 */
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlInitParser		(void);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlCleanupParser	(void);

/*
 * Input functions
 */
hw_XMLPUBFUN int hw_XMLCALL		
		hw_xmlParserInputGrow	(hw_xmlParserInputPtr in,
					 int len);


/*
 * Less common routines and SAX interfaces
 */
hw_XMLPUBFUN int hw_XMLCALL		
		hw_xmlParseDocument	(hw_xmlParserCtxtPtr ctxt);

hw_XMLPUBFUN int hw_XMLCALL		
		hw_xmlParseCtxtExternalEntity(hw_xmlParserCtxtPtr ctx,
					 const hw_xmlChar *URL,
					 const hw_xmlChar *ID,
					 hw_xmlNodePtr *lst);

/*
 * Parser contexts handling.
 */
hw_XMLPUBFUN hw_xmlParserCtxtPtr hw_XMLCALL	
		hw_xmlNewParserCtxt	(void);
hw_XMLPUBFUN int hw_XMLCALL		
		hw_xmlInitParserCtxt	(hw_xmlParserCtxtPtr ctxt);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlFreeParserCtxt	(hw_xmlParserCtxtPtr ctxt);


/*
 * Node infos.
 */

hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlInitNodeInfoSeq	(hw_xmlParserNodeInfoSeqPtr seq);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlClearNodeInfoSeq	(hw_xmlParserNodeInfoSeqPtr seq);
hw_XMLPUBFUN unsigned long hw_XMLCALL 
		hw_xmlParserFindNodeInfoIndex(const hw_xmlParserNodeInfoSeqPtr seq,
                                         const hw_xmlNodePtr node);
hw_XMLPUBFUN void hw_XMLCALL		
		hw_xmlParserAddNodeInfo	(hw_xmlParserCtxtPtr ctxt,
					 const hw_xmlParserNodeInfoPtr info);

/*
 * External entities handling actually implemented in xmlIO.
 */


hw_XMLPUBFUN hw_xmlParserInputPtr hw_XMLCALL
		hw_xmlLoadExternalEntity	(const char *URL,
					 const char *ID,
					 hw_xmlParserCtxtPtr ctxt);

/*
 * New set of simpler/more flexible APIs
 */
/**
 * hw_xmlParserOption:
 *
 * This is the set of XML parser options that can be passed down
 * to the xmlReadDoc() and similar calls.
 */
typedef enum {
    XML_PARSE_RECOVER	= 1<<0,	/* recover on errors */
    XML_PARSE_NOENT	= 1<<1,	/* substitute entities */
    XML_PARSE_NOERROR	= 1<<5,	/* suppress error reports */
    XML_PARSE_NOWARNING	= 1<<6,	/* suppress warning reports */
    XML_PARSE_NOBLANKS	= 1<<8,	/* remove blank nodes */
    XML_PARSE_SAX1	= 1<<9,	/* use the SAX1 interface internally */
    XML_PARSE_XINCLUDE	= 1<<10,/* Implement XInclude substitition  */
    XML_PARSE_NODICT	= 1<<12,/* Do not reuse the context dictionnary */
    XML_PARSE_NOCDATA	= 1<<14,/* merge CDATA as text nodes */
    XML_PARSE_NOXINCNODE= 1<<15,/* do not generate XINCLUDE START/END nodes */
} hw_xmlParserOption;

hw_XMLPUBFUN int hw_XMLCALL hw_xmlCtxtUseOptions	(hw_xmlParserCtxtPtr ctxt,int options);
hw_XMLPUBFUN hw_xmlDocPtr hw_XMLCALL hw_xmlReadFile		(const char *URL,const char *encoding, int options);


#ifdef __cplusplus
}
#endif
#endif /* __XML_PARSER_H__ */

