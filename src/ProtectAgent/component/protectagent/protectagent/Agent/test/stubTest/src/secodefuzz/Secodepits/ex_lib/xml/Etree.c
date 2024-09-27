/*
 * tree.c : implementation of access function for an XML tree.
 *
 * References:
 *   XHTML 1.0 W3C REC: http://www.w3.org/TR/2002/REC-xhtml1-20020801/
 *
 * See Copyright for the status of this software.
 *
 * daniel@veillard.com
 *
 */

#define IN_LIBXML



#include "Etree.h"
#include "Eparser.h"

int hw___xmlRegisterCallbacks = 0;

hw_xmlNsPtr xmlNewReconciliedNs(hw_xmlDocPtr doc, hw_xmlNodePtr tree, hw_xmlNsPtr ns);

/************************************************************************
 *									*
 * 		Tree memory error handler				*
 *									*
 ************************************************************************/
/**
 * xmlTreeErrMemory:
 * @extra:  extra informations
 *
 * Handle an out of memory condition
 */
static void
xmlTreeErrMemory(const char *extra)
{
    hw___xmlSimpleError(XML_FROM_TREE, XML_ERR_NO_MEMORY, NULL, NULL, extra);
}

/**
 * xmlTreeErr:
 * @code:  the error number
 * @extra:  extra informations
 *
 * Handle an out of memory condition
 */
static void
xmlTreeErr(int code, hw_xmlNodePtr node, const char *extra)
{
    const char *msg = NULL;

    switch(code) {
        case XML_TREE_INVALID_HEX:
	    msg = "invalid hexadecimal character value\n";
	    break;
	case XML_TREE_INVALID_DEC:
	    msg = "invalid decimal character value\n";
	    break;
	case XML_TREE_UNTERMINATED_ENTITY:
	    msg = "unterminated entity reference %15s\n";
	    break;
	default:
	    msg = "unexpected error number\n";
    }
    hw___xmlSimpleError(XML_FROM_TREE, code, node, msg, extra);
}

/************************************************************************
 *									*
 * 		A few static variables and macros			*
 *									*
 ************************************************************************/
/* #undef hw_xmlStringText */
const hw_xmlChar hw_xmlStringText[] = { 't', 'e', 'x', 't', 0 };
/* #undef hw_xmlStringTextNoenc */
const hw_xmlChar hw_xmlStringTextNoenc[] =
              { 't', 'e', 'x', 't', 'n', 'o', 'e', 'n', 'c', 0 };
/* #undef hw_xmlStringComment */
const hw_xmlChar hw_xmlStringComment[] = { 'c', 'o', 'm', 'm', 'e', 'n', 't', 0 };

static int xmlCheckDTD = 1;

#define UPDATE_LAST_CHILD_AND_PARENT(n) if ((n) != NULL) {		\
    hw_xmlNodePtr ulccur = (n)->children;					\
    if (ulccur == NULL) {						\
        (n)->last = NULL;						\
    } else {								\
        while (ulccur->next != NULL) {					\
	       	ulccur->parent = (n);					\
		ulccur = ulccur->next;					\
	}								\
	ulccur->parent = (n);						\
	(n)->last = ulccur;						\
}}

#define IS_STR_XML(str) ((str != NULL) && (str[0] == 'x') && \
  (str[1] == 'm') && (str[2] == 'l') && (str[3] == 0))

/* #define DEBUG_BUFFER */
/* #define DEBUG_TREE */

/************************************************************************
 *									*
 *		Functions to move to entities.c once the 		*
 *		API freeze is smoothen and they can be made public.	*
 *									*
 ************************************************************************/


/************************************************************************
 *									*
 *			QName handling helper				*
 *									*
 ************************************************************************/

/**
 * hw_xmlBuildQName:
 * @ncname:  the Name
 * @prefix:  the prefix
 * @memory:  preallocated memory
 * @len:  preallocated memory length
 *
 * Builds the QName @prefix:@ncname in @memory if there is enough space
 * and prefix is not NULL nor empty, otherwise allocate a new string.
 * If prefix is NULL or empty it returns ncname.
 *
 * Returns the new string which must be freed by the caller if different from
 *         @memory and @ncname or NULL in case of error
 */
hw_xmlChar *
hw_xmlBuildQName(const hw_xmlChar *ncname, const hw_xmlChar *prefix,
	      hw_xmlChar *memory, int len) {
    int lenn, lenp;
    hw_xmlChar *ret;

    if (ncname == NULL) return(NULL);
    if (prefix == NULL) return((hw_xmlChar *) ncname);

    lenn = strlen((char *) ncname);
    lenp = strlen((char *) prefix);

    if ((memory == NULL) || (len < lenn + lenp + 2)) {
	ret = (hw_xmlChar *) hw_xmlMallocAtomic(lenn + lenp + 2);
	if (ret == NULL) {
	    xmlTreeErrMemory("building QName");
	    return(NULL);
	}
    } else {
	ret = memory;
    }
    memcpy(&ret[0], prefix, lenp);
    ret[lenp] = ':';
    memcpy(&ret[lenp + 1], ncname, lenn);
    ret[lenn + lenp + 1] = 0;
    return(ret);
}

/**
 * hw_xmlSplitQName2:
 * @name:  the full QName
 * @prefix:  a hw_xmlChar ** 
 *
 * parse an XML qualified name string
 *
 * [NS 5] QName ::= (Prefix ':')? LocalPart
 *
 * [NS 6] Prefix ::= NCName
 *
 * [NS 7] LocalPart ::= NCName
 *
 * Returns NULL if not a QName, otherwise the local part, and prefix
 *   is updated to get the Prefix if any.
 */

hw_xmlChar *
hw_xmlSplitQName2(const hw_xmlChar *name, hw_xmlChar **prefix) {
    int len = 0;
    hw_xmlChar *ret = NULL;

    if (prefix == NULL) return(NULL);
    *prefix = NULL;
    if (name == NULL) return(NULL);

#ifndef hw_XML_XML_NAMESPACE
    /* xml: prefix is not really a namespace */
    if ((name[0] == 'x') && (name[1] == 'm') &&
        (name[2] == 'l') && (name[3] == ':'))
	return(NULL);
#endif

    /* nasty but valid */
    if (name[0] == ':')
	return(NULL);

    /*
     * we are not trying to validate but just to cut, and yes it will
     * work even if this is as set of UTF-8 encoded chars
     */
    while ((name[len] != 0) && (name[len] != ':')) 
	len++;
    
    if (name[len] == 0)
	return(NULL);

    *prefix = hw_xmlStrndup(name, len);
    if (*prefix == NULL) {
	xmlTreeErrMemory("QName split");
	return(NULL);
    }
    ret = hw_xmlStrdup(&name[len + 1]);
    if (ret == NULL) {
	xmlTreeErrMemory("QName split");
	if (*prefix != NULL) {
	    hw_xmlFree(*prefix);
	    *prefix = NULL;
	}
	return(NULL);
    }

    return(ret);
}

/**
 * hw_xmlSplitQName3:
 * @name:  the full QName
 * @len: an int *
 *
 * parse an XML qualified name string,i
 *
 * returns NULL if it is not a Qualified Name, otherwise, update len
 *         with the lenght in byte of the prefix and return a pointer
 *         to the start of the name without the prefix
 */

const hw_xmlChar *
hw_xmlSplitQName3(const hw_xmlChar *name, int *len) {
    int l = 0;

    if (name == NULL) return(NULL);
    if (len == NULL) return(NULL);

    /* nasty but valid */
    if (name[0] == ':')
	return(NULL);

    /*
     * we are not trying to validate but just to cut, and yes it will
     * work even if this is as set of UTF-8 encoded chars
     */
    while ((name[l] != 0) && (name[l] != ':')) 
	l++;
    
    if (name[l] == 0)
	return(NULL);

    *len = l;

    return(&name[l+1]);
}

/************************************************************************
 *									*
 *		Check Name, NCName and QName strings			*
 *									*
 ************************************************************************/
 
#define CUR_SCHAR(s, l) hw_xmlStringCurrentChar(NULL, s, &l)


/************************************************************************
 *									*
 *		Allocation and deallocation of basic structures		*
 *									*
 ************************************************************************/
 
/**
 * hw_xmlSetBufferAllocationScheme:
 * @scheme:  allocation method to use
 * 
 * Set the buffer allocation method.  Types are
 * XML_BUFFER_ALLOC_EXACT - use exact sizes, keeps memory usage down
 * XML_BUFFER_ALLOC_DOUBLEIT - double buffer when extra needed, 
 *                             improves performance
 */
void
hw_xmlSetBufferAllocationScheme(hw_xmlBufferAllocationScheme scheme) {
    hw_xmlBufferAllocScheme = scheme;
}

/**
 * hw_xmlGetBufferAllocationScheme:
 *
 * Types are
 * XML_BUFFER_ALLOC_EXACT - use exact sizes, keeps memory usage down
 * XML_BUFFER_ALLOC_DOUBLEIT - double buffer when extra needed, 
 *                             improves performance
 * 
 * Returns the current allocation scheme
 */
hw_xmlBufferAllocationScheme
hw_xmlGetBufferAllocationScheme(void) {
    return(hw_xmlBufferAllocScheme);
}

/**
 * hw_xmlNewNs:
 * @node:  the element carrying the namespace
 * @href:  the URI associated
 * @prefix:  the prefix for the namespace
 *
 * Creation of a new Namespace. This function will refuse to create
 * a namespace with a similar prefix than an existing one present on this
 * node.
 * We use href==NULL in the case of an element creation where the namespace
 * was not defined.
 * Returns a new namespace pointer or NULL
 */
hw_xmlNsPtr
hw_xmlNewNs(hw_xmlNodePtr node, const hw_xmlChar *href, const hw_xmlChar *prefix) {
    hw_xmlNsPtr cur;

    if ((node != NULL) && (node->type != XML_ELEMENT_NODE))
	return(NULL);

    if ((prefix != NULL) && (hw_xmlStrEqual(prefix, hw_BAD_CAST "xml")))
	return(NULL);

    /*
     * Allocate a new Namespace and fill the fields.
     */
    cur = (hw_xmlNsPtr) hw_xmlMalloc(sizeof(hw_xmlNs));
    if (cur == NULL) {
	xmlTreeErrMemory("building namespace");
	return(NULL);
    }
    memset(cur, 0, sizeof(hw_xmlNs));
    cur->type = hw_XML_LOCAL_NAMESPACE;

    if (href != NULL)
	cur->href = hw_xmlStrdup(href); 
    if (prefix != NULL)
	cur->prefix = hw_xmlStrdup(prefix); 

    /*
     * Add it at the end to preserve parsing order ...
     * and checks for existing use of the prefix
     */
    if (node != NULL) {
	if (node->nsDef == NULL) {
	    node->nsDef = cur;
	} else {
	    hw_xmlNsPtr prev = node->nsDef;

	    if (((prev->prefix == NULL) && (cur->prefix == NULL)) ||
		(hw_xmlStrEqual(prev->prefix, cur->prefix))) {
		hw_xmlFreeNs(cur);
		return(NULL);
	    }    
	    while (prev->next != NULL) {
	        prev = prev->next;
		if (((prev->prefix == NULL) && (cur->prefix == NULL)) ||
		    (hw_xmlStrEqual(prev->prefix, cur->prefix))) {
		    hw_xmlFreeNs(cur);
		    return(NULL);
		}    
	    }
	    prev->next = cur;
	}
    }
    return(cur);
}

/**
 * hw_xmlSetNs:
 * @node:  a node in the document
 * @ns:  a namespace pointer
 *
 * Associate a namespace to a node, a posteriori.
 */
void
hw_xmlSetNs(hw_xmlNodePtr node, hw_xmlNsPtr ns) {
    if (node == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlSetNs: node == NULL\n");
#endif
	return;
    }
    node->ns = ns;
}

/**
 * hw_xmlFreeNs:
 * @cur:  the namespace pointer
 *
 * Free up the structures associated to a namespace
 */
void
hw_xmlFreeNs(hw_xmlNsPtr cur) {
    if (cur == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlFreeNs : ns == NULL\n");
#endif
	return;
    }
    if (cur->href != NULL) hw_xmlFree((char *) cur->href);
    if (cur->prefix != NULL) hw_xmlFree((char *) cur->prefix);
    hw_xmlFree(cur);
}

/**
 * hw_xmlFreeNsList:
 * @cur:  the first namespace pointer
 *
 * Free up all the structures associated to the chained namespaces.
 */
void
hw_xmlFreeNsList(hw_xmlNsPtr cur) {
    hw_xmlNsPtr next;
    if (cur == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlFreeNsList : ns == NULL\n");
#endif
	return;
    }
    while (cur != NULL) {
        next = cur->next;
        hw_xmlFreeNs(cur);
	cur = next;
    }
}

/**
 * hw_xmlNewDtd:
 * @doc:  the document pointer
 * @name:  the DTD name
 * @ExternalID:  the external ID
 * @SystemID:  the system ID
 *
 * Creation of a new DTD for the external subset. To create an
 * internal subset, use hw_xmlCreateIntSubset().
 *
 * Returns a pointer to the new DTD structure
 */
hw_xmlDtdPtr
hw_xmlNewDtd(hw_xmlDocPtr doc, const hw_xmlChar *name,
                    const hw_xmlChar *ExternalID, const hw_xmlChar *SystemID) {
    hw_xmlDtdPtr cur;

    if ((doc != NULL) && (doc->extSubset != NULL)) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlNewDtd(%s): document %s already have a DTD %s\n",
	    /* !!! */ (char *) name, doc->name,
	    /* !!! */ (char *)doc->extSubset->name);
#endif
	return(NULL);
    }

    /*
     * Allocate a new DTD and fill the fields.
     */
    cur = (hw_xmlDtdPtr) hw_xmlMalloc(sizeof(hw_xmlDtd));
    if (cur == NULL) {
	xmlTreeErrMemory("building DTD");
	return(NULL);
    }
    memset(cur, 0 , sizeof(hw_xmlDtd));
    cur->type = XML_DTD_NODE;

    if (name != NULL)
	cur->name = hw_xmlStrdup(name); 
    if (ExternalID != NULL)
	cur->ExternalID = hw_xmlStrdup(ExternalID); 
    if (SystemID != NULL)
	cur->SystemID = hw_xmlStrdup(SystemID); 
    if (doc != NULL)
	doc->extSubset = cur;
    cur->doc = doc;

    return(cur);
}

/**
 * hw_xmlGetIntSubset:
 * @doc:  the document pointer
 *
 * Get the internal subset of a document
 * Returns a pointer to the DTD structure or NULL if not found
 */

hw_xmlDtdPtr
hw_xmlGetIntSubset(hw_xmlDocPtr doc) {
    hw_xmlNodePtr cur;

    if (doc == NULL)
	return(NULL);
    cur = doc->children;
    while (cur != NULL) {
	if (cur->type == XML_DTD_NODE)
	    return((hw_xmlDtdPtr) cur);
	cur = cur->next;
    }
    return((hw_xmlDtdPtr) doc->intSubset);
}

/**
 * hw_xmlCreateIntSubset:
 * @doc:  the document pointer
 * @name:  the DTD name
 * @ExternalID:  the external (PUBLIC) ID
 * @SystemID:  the system ID
 *
 * Create the internal subset of a document
 * Returns a pointer to the new DTD structure
 */
hw_xmlDtdPtr
hw_xmlCreateIntSubset(hw_xmlDocPtr doc, const hw_xmlChar *name,
                   const hw_xmlChar *ExternalID, const hw_xmlChar *SystemID) {
    hw_xmlDtdPtr cur;

    if ((doc != NULL) && (hw_xmlGetIntSubset(doc) != NULL)) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,

     "hw_xmlCreateIntSubset(): document %s already have an internal subset\n",
	    doc->name);
#endif
	return(NULL);
    }

    /*
     * Allocate a new DTD and fill the fields.
     */
    cur = (hw_xmlDtdPtr) hw_xmlMalloc(sizeof(hw_xmlDtd));
    if (cur == NULL) {
	xmlTreeErrMemory("building internal subset");
	return(NULL);
    }
    memset(cur, 0, sizeof(hw_xmlDtd));
    cur->type = XML_DTD_NODE;

    if (name != NULL) {
	cur->name = hw_xmlStrdup(name);
	if (cur->name == NULL) {
	    xmlTreeErrMemory("building internal subset");
	    hw_xmlFree(cur);
	    return(NULL);
	}
    }
    if (ExternalID != NULL) {
	cur->ExternalID = hw_xmlStrdup(ExternalID); 
	if (cur->ExternalID  == NULL) {
	    xmlTreeErrMemory("building internal subset");
	    if (cur->name != NULL)
	        hw_xmlFree((char *)cur->name);
	    hw_xmlFree(cur);
	    return(NULL);
	}
    }
    if (SystemID != NULL) {
	cur->SystemID = hw_xmlStrdup(SystemID); 
	if (cur->SystemID == NULL) {
	    xmlTreeErrMemory("building internal subset");
	    if (cur->name != NULL)
	        hw_xmlFree((char *)cur->name);
	    if (cur->ExternalID != NULL)
	        hw_xmlFree((char *)cur->ExternalID);
	    hw_xmlFree(cur);
	    return(NULL);
	}
    }
    if (doc != NULL) {
	doc->intSubset = cur;
	cur->parent = doc;
	cur->doc = doc;
	if (doc->children == NULL) {
	    doc->children = (hw_xmlNodePtr) cur;
	    doc->last = (hw_xmlNodePtr) cur;
	} else {
	    if (doc->type == XML_HTML_DOCUMENT_NODE) {
		hw_xmlNodePtr prev;

		prev = doc->children;
		prev->prev = (hw_xmlNodePtr) cur;
		cur->next = prev;
		doc->children = (hw_xmlNodePtr) cur;
	    } else {
		hw_xmlNodePtr next;

		next = doc->children;
		while ((next != NULL) && (next->type != XML_ELEMENT_NODE))
		    next = next->next;
		if (next == NULL) {
		    cur->prev = doc->last;
		    cur->prev->next = (hw_xmlNodePtr) cur;
		    cur->next = NULL;
		    doc->last = (hw_xmlNodePtr) cur;
		} else {
		    cur->next = next;
		    cur->prev = next->prev;
		    if (cur->prev == NULL)
			doc->children = (hw_xmlNodePtr) cur;
		    else
			cur->prev->next = (hw_xmlNodePtr) cur;
		    next->prev = (hw_xmlNodePtr) cur;
		}
	    }
	}
    }

    return(cur);
}

/**
 * hw_DICT_FREE:
 * @str:  a string
 *
 * Free a string if it is not owned by the "dict" dictionnary in the
 * current scope
 */
#define hw_DICT_FREE(str)						\
	if ((str) && ((!dict) || 				\
	    (hw_xmlDictOwns(dict, (const hw_xmlChar *)(str)) == 0)))	\
	    hw_xmlFree((char *)(str));

/**
 * hw_xmlFreeDtd:
 * @cur:  the DTD structure to free up
 *
 * Free a DTD structure.
 */
void
hw_xmlFreeDtd(hw_xmlDtdPtr cur) {
    hw_xmlDictPtr dict = NULL;

    if (cur == NULL) {
	return;
    }
    if (cur->doc != NULL) dict = cur->doc->dict;


    if (cur->children != NULL) {
	hw_xmlNodePtr next, c = cur->children;

	/*
	 * Cleanup all nodes which are not part of the specific lists
	 * of notations, elements, attributes and entities.
	 */
        while (c != NULL) {
	    next = c->next;
	    if ((c->type != XML_NOTATION_NODE) &&
	        (c->type != XML_ELEMENT_DECL) &&
		(c->type != XML_ATTRIBUTE_DECL) &&
		(c->type != XML_ENTITY_DECL)) {
		hw_xmlUnlinkNode(c);
		hw_xmlFreeNode(c);
	    }
	    c = next;
	}
    }
    hw_DICT_FREE(cur->name)
    hw_DICT_FREE(cur->SystemID)
    hw_DICT_FREE(cur->ExternalID)
    /* TODO !!! */
    
    if (cur->entities != NULL)
        hw_xmlFreeEntitiesTable((hw_xmlEntitiesTablePtr) cur->entities);
    if (cur->pentities != NULL)
        hw_xmlFreeEntitiesTable((hw_xmlEntitiesTablePtr) cur->pentities);

    hw_xmlFree(cur);
}

/**
 * hw_xmlNewDoc:
 * @version:  hw_xmlChar string giving the version of XML "1.0"
 *
 * Creates a new XML document
 *
 * Returns a new document
 */
hw_xmlDocPtr
hw_xmlNewDoc(const hw_xmlChar *version) {
    hw_xmlDocPtr cur;

    if (version == NULL)
	version = (const hw_xmlChar *) "1.0";

    /*
     * Allocate a new document and fill the fields.
     */
    cur = (hw_xmlDocPtr) hw_xmlMalloc(sizeof(hw_xmlDoc));
    if (cur == NULL) {
	xmlTreeErrMemory("building doc");
	return(NULL);
    }
    memset(cur, 0, sizeof(hw_xmlDoc));
    cur->type = XML_DOCUMENT_NODE;

    cur->version = hw_xmlStrdup(version); 
    if (cur->version == NULL) {
	xmlTreeErrMemory("building doc");
	hw_xmlFree(cur);
    	return(NULL);
    }
    cur->standalone = -1;
    cur->compression = -1; /* not initialized */
    cur->doc = cur;
    /*
     * The in memory encoding is always UTF8
     * This field will never change and would
     * be obsolete if not for binary compatibility.
     */
    cur->charset = XML_CHAR_ENCODING_UTF8;

    return(cur);
}

/**
 * hw_xmlFreeDoc:
 * @cur:  pointer to the document
 *
 * Free up all the structures used by a document, tree included.
 */
void
hw_xmlFreeDoc(hw_xmlDocPtr cur) {
    hw_xmlDtdPtr extSubset, intSubset;
    hw_xmlDictPtr dict = NULL;

    if (cur == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlFreeDoc : document == NULL\n");
#endif
	return;
    }
    if (cur != NULL) dict = cur->dict;


    /*
     * Do this before freeing the children list to avoid ID lookups
     */
    cur->ids = NULL;
    cur->refs = NULL;
    extSubset = cur->extSubset;
    intSubset = cur->intSubset;
    if (intSubset == extSubset)
	extSubset = NULL;
    if (extSubset != NULL) {
	hw_xmlUnlinkNode((hw_xmlNodePtr) cur->extSubset);
	cur->extSubset = NULL;
	hw_xmlFreeDtd(extSubset);
    }
    if (intSubset != NULL) {
	hw_xmlUnlinkNode((hw_xmlNodePtr) cur->intSubset);
	cur->intSubset = NULL;
	hw_xmlFreeDtd(intSubset);
    }

    if (cur->children != NULL) hw_xmlFreeNodeList(cur->children);
    if (cur->oldNs != NULL) hw_xmlFreeNsList(cur->oldNs);

    hw_DICT_FREE(cur->version)
    hw_DICT_FREE(cur->name)
    hw_DICT_FREE(cur->encoding)
    hw_DICT_FREE(cur->URL)
    hw_xmlFree(cur);
    if (dict) hw_xmlDictFree(dict);
}

/**
 * hw_xmlStringLenGetNodeList:
 * @doc:  the document
 * @value:  the value of the text
 * @len:  the length of the string value
 *
 * Parse the value string and build the node list associated. Should
 * produce a flat tree with only TEXTs and ENTITY_REFs.
 * Returns a pointer to the first child
 */
hw_xmlNodePtr
hw_xmlStringLenGetNodeList(hw_xmlDocPtr doc, const hw_xmlChar *value, int len) {
    hw_xmlNodePtr ret = NULL, last = NULL;
    hw_xmlNodePtr node;
    hw_xmlChar *val;
    const hw_xmlChar *cur = value, *end = cur + len;
    const hw_xmlChar *q;
    hw_xmlEntityPtr ent;

    if (value == NULL) return(NULL);

    q = cur;
    while ((cur < end) && (*cur != 0)) {
	if (cur[0] == '&') {
	    int charval = 0;
	    hw_xmlChar tmp;

	    /*
	     * Save the current text.
	     */
            if (cur != q) {
		if ((last != NULL) && (last->type == XML_TEXT_NODE)) {
		    hw_xmlNodeAddContentLen(last, q, cur - q);
		} else {
		    node = hw_xmlNewDocTextLen(doc, q, cur - q);
		    if (node == NULL) return(ret);
		    if (last == NULL)
			last = ret = node;
		    else {
			last->next = node;
			node->prev = last;
			last = node;
		    }
		}
	    }
	    q = cur;
	    if ((cur + 2 < end) && (cur[1] == '#') && (cur[2] == 'x')) {
		cur += 3;
		if (cur < end)
		    tmp = *cur;
		else
		    tmp = 0;
		while (tmp != ';') { /* Non input consuming loop */
		    if ((tmp >= '0') && (tmp <= '9')) 
			charval = charval * 16 + (tmp - '0');
		    else if ((tmp >= 'a') && (tmp <= 'f'))
			charval = charval * 16 + (tmp - 'a') + 10;
		    else if ((tmp >= 'A') && (tmp <= 'F'))
			charval = charval * 16 + (tmp - 'A') + 10;
		    else {
			xmlTreeErr(XML_TREE_INVALID_HEX, (hw_xmlNodePtr) doc,
			           NULL);
			charval = 0;
			break;
		    }
		    cur++;
		    if (cur < end)
			tmp = *cur;
		    else
			tmp = 0;
		}
		if (tmp == ';')
		    cur++;
		q = cur;
	    } else if ((cur + 1 < end) && (cur[1] == '#')) {
		cur += 2;
		if (cur < end)
		    tmp = *cur;
		else
		    tmp = 0;
		while (tmp != ';') { /* Non input consuming loops */
		    if ((tmp >= '0') && (tmp <= '9')) 
			charval = charval * 10 + (tmp - '0');
		    else {
			xmlTreeErr(XML_TREE_INVALID_DEC, (hw_xmlNodePtr) doc,
			           NULL);
			charval = 0;
			break;
		    }
		    cur++;
		    if (cur < end)
			tmp = *cur;
		    else
			tmp = 0;
		}
		if (tmp == ';')
		    cur++;
		q = cur;
	    } else {
		/*
		 * Read the entity string
		 */
		cur++;
		q = cur;
		while ((cur < end) && (*cur != 0) && (*cur != ';')) cur++;
		if ((cur >= end) || (*cur == 0)) {
		    xmlTreeErr(XML_TREE_UNTERMINATED_ENTITY, (hw_xmlNodePtr) doc,
		               (const char *) q);
		    return(ret);
		}
		if (cur != q) {
		    /*
		     * Predefined entities don't generate nodes
		     */
		    val = hw_xmlStrndup(q, cur - q);
		    ent = hw_xmlGetDocEntity(doc, val);
		    if ((ent != NULL) &&
			(ent->etype == XML_INTERNAL_PREDEFINED_ENTITY)) {
			if (last == NULL) {
			    node = hw_xmlNewDocText(doc, ent->content);
			    last = ret = node;
			} else if (last->type != XML_TEXT_NODE) {
			    node = hw_xmlNewDocText(doc, ent->content);
			    last = hw_xmlAddNextSibling(last, node);
			} else
			    hw_xmlNodeAddContent(last, ent->content);
			    
		    } else {
			/*
			 * Create a new REFERENCE_REF node
			 */
			node = hw_xmlNewReference(doc, val);
			if (node == NULL) {
			    if (val != NULL) hw_xmlFree(val);
			    return(ret);
			}
			else if ((ent != NULL) && (ent->children == NULL)) {
			    hw_xmlNodePtr temp;

			    ent->children = hw_xmlStringGetNodeList(doc,
				    (const hw_xmlChar*)node->content);
			    ent->owner = 1;
			    temp = ent->children;
			    while (temp) {
				temp->parent = (hw_xmlNodePtr)ent;
				ent->last = temp;
				temp = temp->next;
			    }
			}
			if (last == NULL) {
			    last = ret = node;
			} else {
			    last = hw_xmlAddNextSibling(last, node);
			}
		    }
		    hw_xmlFree(val);
		}
		cur++;
		q = cur;
	    }
	    if (charval != 0) {
		hw_xmlChar buf[10];
		int l;

		l = hw_xmlCopyCharMultiByte(buf, charval);
		buf[l] = 0;
		node = hw_xmlNewDocText(doc, buf);
		if (node != NULL) {
		    if (last == NULL) {
			last = ret = node;
		    } else {
			last = hw_xmlAddNextSibling(last, node);
		    }
		}
		charval = 0;
	    }
	} else
	    cur++;
    }
    if ((cur != q) || (ret == NULL)) {
        /*
	 * Handle the last piece of text.
	 */
	if ((last != NULL) && (last->type == XML_TEXT_NODE)) {
	    hw_xmlNodeAddContentLen(last, q, cur - q);
	} else {
	    node = hw_xmlNewDocTextLen(doc, q, cur - q);
	    if (node == NULL) return(ret);
	    if (last == NULL) {
		last = ret = node;
	    } else {
		last = hw_xmlAddNextSibling(last, node);
	    }
	}
    }
    return(ret);
}

/**
 * hw_xmlStringGetNodeList:
 * @doc:  the document
 * @value:  the value of the attribute
 *
 * Parse the value string and build the node list associated. Should
 * produce a flat tree with only TEXTs and ENTITY_REFs.
 * Returns a pointer to the first child
 */
hw_xmlNodePtr
hw_xmlStringGetNodeList(hw_xmlDocPtr doc, const hw_xmlChar *value) {
    hw_xmlNodePtr ret = NULL, last = NULL;
    hw_xmlNodePtr node;
    hw_xmlChar *val;
    const hw_xmlChar *cur = value;
    const hw_xmlChar *q;
    hw_xmlEntityPtr ent;

    if (value == NULL) return(NULL);

    q = cur;
    while (*cur != 0) {
	if (cur[0] == '&') {
	    int charval = 0;
	    hw_xmlChar tmp;

	    /*
	     * Save the current text.
	     */
            if (cur != q) {
		if ((last != NULL) && (last->type == XML_TEXT_NODE)) {
		    hw_xmlNodeAddContentLen(last, q, cur - q);
		} else {
		    node = hw_xmlNewDocTextLen(doc, q, cur - q);
		    if (node == NULL) return(ret);
		    if (last == NULL)
			last = ret = node;
		    else {
			last->next = node;
			node->prev = last;
			last = node;
		    }
		}
	    }
	    q = cur;
	    if ((cur[1] == '#') && (cur[2] == 'x')) {
		cur += 3;
		tmp = *cur;
		while (tmp != ';') { /* Non input consuming loop */
		    if ((tmp >= '0') && (tmp <= '9')) 
			charval = charval * 16 + (tmp - '0');
		    else if ((tmp >= 'a') && (tmp <= 'f'))
			charval = charval * 16 + (tmp - 'a') + 10;
		    else if ((tmp >= 'A') && (tmp <= 'F'))
			charval = charval * 16 + (tmp - 'A') + 10;
		    else {
			xmlTreeErr(XML_TREE_INVALID_HEX, (hw_xmlNodePtr) doc,
			           NULL);
			charval = 0;
			break;
		    }
		    cur++;
		    tmp = *cur;
		}
		if (tmp == ';')
		    cur++;
		q = cur;
	    } else if  (cur[1] == '#') {
		cur += 2;
		tmp = *cur;
		while (tmp != ';') { /* Non input consuming loops */
		    if ((tmp >= '0') && (tmp <= '9')) 
			charval = charval * 10 + (tmp - '0');
		    else {
			xmlTreeErr(XML_TREE_INVALID_DEC, (hw_xmlNodePtr) doc,
			           NULL);
			charval = 0;
			break;
		    }
		    cur++;
		    tmp = *cur;
		}
		if (tmp == ';')
		    cur++;
		q = cur;
	    } else {
		/*
		 * Read the entity string
		 */
		cur++;
		q = cur;
		while ((*cur != 0) && (*cur != ';')) cur++;
		if (*cur == 0) {
		    xmlTreeErr(XML_TREE_UNTERMINATED_ENTITY,
		               (hw_xmlNodePtr) doc, (const char *) q);
		    return(ret);
		}
		if (cur != q) {
		    /*
		     * Predefined entities don't generate nodes
		     */
		    val = hw_xmlStrndup(q, cur - q);
		    ent = hw_xmlGetDocEntity(doc, val);
		    if ((ent != NULL) &&
			(ent->etype == XML_INTERNAL_PREDEFINED_ENTITY)) {
			if (last == NULL) {
			    node = hw_xmlNewDocText(doc, ent->content);
			    last = ret = node;
			} else if (last->type != XML_TEXT_NODE) {
			    node = hw_xmlNewDocText(doc, ent->content);
			    last = hw_xmlAddNextSibling(last, node);
			} else
			    hw_xmlNodeAddContent(last, ent->content);
			    
		    } else {
			/*
			 * Create a new REFERENCE_REF node
			 */
			node = hw_xmlNewReference(doc, val);
			if (node == NULL) {
			    if (val != NULL) hw_xmlFree(val);
			    return(ret);
			}
			else if ((ent != NULL) && (ent->children == NULL)) {
			    hw_xmlNodePtr temp;

			    ent->children = hw_xmlStringGetNodeList(doc,
				    (const hw_xmlChar*)node->content);
			    ent->owner = 1;
			    temp = ent->children;
			    while (temp) {
				temp->parent = (hw_xmlNodePtr)ent;
				temp = temp->next;
			    }
			}
			if (last == NULL) {
			    last = ret = node;
			} else {
			    last = hw_xmlAddNextSibling(last, node);
			}
		    }
		    hw_xmlFree(val);
		}
		cur++;
		q = cur;
	    }
	    if (charval != 0) {
		hw_xmlChar buf[10];
		int len;

		len = hw_xmlCopyCharMultiByte(buf, charval);
		buf[len] = 0;
		node = hw_xmlNewDocText(doc, buf);
		if (node != NULL) {
		    if (last == NULL) {
			last = ret = node;
		    } else {
			last = hw_xmlAddNextSibling(last, node);
		    }
		}

		charval = 0;
	    }
	} else
	    cur++;
    }
    if ((cur != q) || (ret == NULL)) {
        /*
	 * Handle the last piece of text.
	 */
	if ((last != NULL) && (last->type == XML_TEXT_NODE)) {
	    hw_xmlNodeAddContentLen(last, q, cur - q);
	} else {
	    node = hw_xmlNewDocTextLen(doc, q, cur - q);
	    if (node == NULL) return(ret);
	    if (last == NULL) {
		last = ret = node;
	    } else {
		last = hw_xmlAddNextSibling(last, node);
	    }
	}
    }
    return(ret);
}

/**
 * hw_xmlNodeListGetString:
 * @doc:  the document
 * @list:  a Node list
 * @inLine:  should we replace entity contents or show their external form
 *
 * Build the string equivalent to the text contained in the Node list
 * made of TEXTs and ENTITY_REFs
 *
 * Returns a pointer to the string copy, the caller must free it with hw_xmlFree().
 */
hw_xmlChar *
hw_xmlNodeListGetString(hw_xmlDocPtr doc, hw_xmlNodePtr list, int inLine)
{
    hw_xmlNodePtr node = list;
    hw_xmlChar *ret = NULL;
    hw_xmlEntityPtr ent;

    if (list == NULL)
        return (NULL);

    while (node != NULL) {
        if ((node->type == XML_TEXT_NODE) ||
            (node->type == XML_CDATA_SECTION_NODE)) {
            if (inLine) {
                ret = hw_xmlStrcat(ret, node->content);
            } else {
                hw_xmlChar *buffer;

                buffer = hw_xmlEncodeEntitiesReentrant(doc, node->content);
                if (buffer != NULL) {
                    ret = hw_xmlStrcat(ret, buffer);
                    hw_xmlFree(buffer);
                }
            }
        } else if (node->type == XML_ENTITY_REF_NODE) {
            if (inLine) {
                ent = hw_xmlGetDocEntity(doc, node->name);
                if (ent != NULL) {
                    hw_xmlChar *buffer;

                    /* an entity content can be any "well balanced chunk",
                     * i.e. the result of the content [43] production:
                     * http://www.w3.org/TR/REC-xml#NT-content.
                     * So it can contain text, CDATA section or nested
                     * entity reference nodes (among others).
                     * -> we recursive  call hw_xmlNodeListGetString()
                     * which handles these types */
                    buffer = hw_xmlNodeListGetString(doc, ent->children, 1);
                    if (buffer != NULL) {
                        ret = hw_xmlStrcat(ret, buffer);
                        hw_xmlFree(buffer);
                    }
                } else {
                    ret = hw_xmlStrcat(ret, node->content);
                }
            } else {
                hw_xmlChar buf[2];

                buf[0] = '&';
                buf[1] = 0;
                ret = hw_xmlStrncat(ret, buf, 1);
                ret = hw_xmlStrcat(ret, node->name);
                buf[0] = ';';
                buf[1] = 0;
                ret = hw_xmlStrncat(ret, buf, 1);
            }
        }
#if 0
        else {
            hw_xmlGenericError(hw_xmlGenericErrorContext,
                            "xmlGetNodeListString : invalid node type %d\n",
                            node->type);
        }
#endif
        node = node->next;
    }
    return (ret);
}

static hw_xmlAttrPtr
xmlNewPropInternal(hw_xmlNodePtr node, hw_xmlNsPtr ns,
                   const hw_xmlChar * name, const hw_xmlChar * value,
                   int eatname)
{
    hw_xmlAttrPtr cur;
    hw_xmlDocPtr doc = NULL;

    if ((node != NULL) && (node->type != XML_ELEMENT_NODE)) {
        if (eatname == 1)
            hw_xmlFree((hw_xmlChar *) name);
        return (NULL);
    }

    /*
     * Allocate a new property and fill the fields.
     */
    cur = (hw_xmlAttrPtr) hw_xmlMalloc(sizeof(hw_xmlAttr));
    if (cur == NULL) {
        if (eatname == 1)
            hw_xmlFree((hw_xmlChar *) name);
        xmlTreeErrMemory("building attribute");
        return (NULL);
    }
    memset(cur, 0, sizeof(hw_xmlAttr));
    cur->type = XML_ATTRIBUTE_NODE;

    cur->parent = node;
    if (node != NULL) {
        doc = node->doc;
        cur->doc = doc;
    }
    cur->ns = ns;

    if (eatname == 0) {
        if ((doc != NULL) && (doc->dict != NULL))
            cur->name = (hw_xmlChar *) hw_xmlDictLookup(doc->dict, name, -1);
        else
            cur->name = hw_xmlStrdup(name);
    } else
        cur->name = name;

    if (value != NULL) {
        hw_xmlChar *buffer;
        hw_xmlNodePtr tmp;

        buffer = hw_xmlEncodeEntitiesReentrant(doc, value);
        cur->children = hw_xmlStringGetNodeList(doc, buffer);
        cur->last = NULL;
        tmp = cur->children;
        while (tmp != NULL) {
            tmp->parent = (hw_xmlNodePtr) cur;
            if (tmp->next == NULL)
                cur->last = tmp;
            tmp = tmp->next;
        }
        hw_xmlFree(buffer);
    }

    /*
     * Add it at the end to preserve parsing order ...
     */
    if (node != NULL) {
        if (node->properties == NULL) {
            node->properties = cur;
        } else {
            hw_xmlAttrPtr prev = node->properties;

            while (prev->next != NULL)
                prev = prev->next;
            prev->next = cur;
            cur->prev = prev;
        }
    }

    if (hw_xmlIsID((node == NULL) ? NULL : node->doc, node, cur) == 1)
        hw_xmlAddID(NULL, node->doc, value, cur);

    return (cur);
}

/**
 * hw_xmlNewNsProp:
 * @node:  the holding node
 * @ns:  the namespace
 * @name:  the name of the attribute
 * @value:  the value of the attribute
 *
 * Create a new property tagged with a namespace and carried by a node.
 * Returns a pointer to the attribute
 */
hw_xmlAttrPtr
hw_xmlNewNsProp(hw_xmlNodePtr node, hw_xmlNsPtr ns, const hw_xmlChar *name,
           const hw_xmlChar *value) {

    if (name == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlNewNsProp : name == NULL\n");
#endif
	return(NULL);
    }

    return xmlNewPropInternal(node, ns, name, value, 0);
}

/**
 * hw_xmlNewNsPropEatName:
 * @node:  the holding node
 * @ns:  the namespace
 * @name:  the name of the attribute
 * @value:  the value of the attribute
 *
 * Create a new property tagged with a namespace and carried by a node.
 * Returns a pointer to the attribute
 */
hw_xmlAttrPtr
hw_xmlNewNsPropEatName(hw_xmlNodePtr node, hw_xmlNsPtr ns, hw_xmlChar *name,
           const hw_xmlChar *value) {

    if (name == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlNewNsPropEatName : name == NULL\n");
#endif
	return(NULL);
    }

	return xmlNewPropInternal(node, ns, name, value, 1);
}

/**
 * hw_xmlNewDocProp:
 * @doc:  the document
 * @name:  the name of the attribute
 * @value:  the value of the attribute
 *
 * Create a new property carried by a document.
 * Returns a pointer to the attribute
 */
hw_xmlAttrPtr
hw_xmlNewDocProp(hw_xmlDocPtr doc, const hw_xmlChar *name, const hw_xmlChar *value) {
    hw_xmlAttrPtr cur;

    if (name == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlNewDocProp : name == NULL\n");
#endif
	return(NULL);
    }

    /*
     * Allocate a new property and fill the fields.
     */
    cur = (hw_xmlAttrPtr) hw_xmlMalloc(sizeof(hw_xmlAttr));
    if (cur == NULL) {
	xmlTreeErrMemory("building attribute");
	return(NULL);
    }
    memset(cur, 0, sizeof(hw_xmlAttr));
    cur->type = XML_ATTRIBUTE_NODE;

    if ((doc != NULL) && (doc->dict != NULL))
	cur->name = hw_xmlDictLookup(doc->dict, name, -1);
    else
	cur->name = hw_xmlStrdup(name);
    cur->doc = doc; 
    if (value != NULL) {
	hw_xmlNodePtr tmp;

	cur->children = hw_xmlStringGetNodeList(doc, value);
	cur->last = NULL;

	tmp = cur->children;
	while (tmp != NULL) {
	    tmp->parent = (hw_xmlNodePtr) cur;
	    if (tmp->next == NULL)
		cur->last = tmp;
	    tmp = tmp->next;
	}
    }

    return(cur);
}

/**
 * hw_xmlFreePropList:
 * @cur:  the first property in the list
 *
 * Free a property and all its siblings, all the children are freed too.
 */
void
hw_xmlFreePropList(hw_xmlAttrPtr cur) {
    hw_xmlAttrPtr next;
    if (cur == NULL) return;
    while (cur != NULL) {
        next = cur->next;
        hw_xmlFreeProp(cur);
	cur = next;
    }
}

/**
 * hw_xmlFreeProp:
 * @cur:  an attribute
 *
 * Free one attribute, all the content is freed too
 */
void
hw_xmlFreeProp(hw_xmlAttrPtr cur) {
    hw_xmlDictPtr dict = NULL;
    if (cur == NULL) return;

    if (cur->doc != NULL) dict = cur->doc->dict;

    /* Check for ID removal -> leading to invalid references ! */
    if ((cur->doc != NULL) && (cur->atype == XML_ATTRIBUTE_ID)) {
	    hw_xmlRemoveID(cur->doc, cur);
    }
    if (cur->children != NULL) hw_xmlFreeNodeList(cur->children);
    hw_DICT_FREE(cur->name)
    hw_xmlFree(cur);
}

/**
 * hw_xmlRemoveProp:
 * @cur:  an attribute
 *
 * Unlink and free one attribute, all the content is freed too
 * Note this doesn't work for namespace definition attributes
 *
 * Returns 0 if success and -1 in case of error.
 */
int
hw_xmlRemoveProp(hw_xmlAttrPtr cur) {
    hw_xmlAttrPtr tmp;
    if (cur == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlRemoveProp : cur == NULL\n");
#endif
	return(-1);
    }
    if (cur->parent == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlRemoveProp : cur->parent == NULL\n");
#endif
	return(-1);
    }
    tmp = cur->parent->properties;
    if (tmp == cur) {
        cur->parent->properties = cur->next;
		if (cur->next != NULL)
			cur->next->prev = NULL;
	hw_xmlFreeProp(cur);
	return(0);
    }
    while (tmp != NULL) {
	if (tmp->next == cur) {
	    tmp->next = cur->next;
	    if (tmp->next != NULL)
		tmp->next->prev = tmp;
	    hw_xmlFreeProp(cur);
	    return(0);
	}
        tmp = tmp->next;
    }
#ifdef DEBUG_TREE
    hw_xmlGenericError(hw_xmlGenericErrorContext,
	    "hw_xmlRemoveProp : attribute not owned by its node\n");
#endif
    return(-1);
}

/**
 * hw_xmlNewDocPI:
 * @doc:  the target document
 * @name:  the processing instruction name
 * @content:  the PI content
 *
 * Creation of a processing instruction element.
 * Returns a pointer to the new node object.
 */
hw_xmlNodePtr
hw_xmlNewDocPI(hw_xmlDocPtr doc, const hw_xmlChar *name, const hw_xmlChar *content) {
    hw_xmlNodePtr cur;

    if (name == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlNewPI : name == NULL\n");
#endif
	return(NULL);
    }

    /*
     * Allocate a new node and fill the fields.
     */
    cur = (hw_xmlNodePtr) hw_xmlMalloc(sizeof(hw_xmlNode));
    if (cur == NULL) {
	xmlTreeErrMemory("building PI");
	return(NULL);
    }
    memset(cur, 0, sizeof(hw_xmlNode));
    cur->type = XML_PI_NODE;

    if ((doc != NULL) && (doc->dict != NULL))
        cur->name = hw_xmlDictLookup(doc->dict, name, -1);
    else
	cur->name = hw_xmlStrdup(name);
    if (content != NULL) {
	cur->content = hw_xmlStrdup(content);
    }
    cur->doc = doc;

    return(cur);
}

/**
 * hw_xmlNewPI:
 * @name:  the processing instruction name
 * @content:  the PI content
 *
 * Creation of a processing instruction element.
 * Use xmlDocNewPI preferably to get string interning
 *
 * Returns a pointer to the new node object.
 */
hw_xmlNodePtr
hw_xmlNewPI(const hw_xmlChar *name, const hw_xmlChar *content) {
    return(hw_xmlNewDocPI(NULL, name, content));
}

/**
 * hw_xmlNewNode:
 * @ns:  namespace if any
 * @name:  the node name
 *
 * Creation of a new node element. @ns is optional (NULL).
 *
 * Returns a pointer to the new node object. Uses hw_xmlStrdup() to make
 * copy of @name.
 */
hw_xmlNodePtr
hw_xmlNewNode(hw_xmlNsPtr ns, const hw_xmlChar *name) {
    hw_xmlNodePtr cur;

    if (name == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlNewNode : name == NULL\n");
#endif
	return(NULL);
    }

    /*
     * Allocate a new node and fill the fields.
     */
    cur = (hw_xmlNodePtr) hw_xmlMalloc(sizeof(hw_xmlNode));
    if (cur == NULL) {
	xmlTreeErrMemory("building node");
	return(NULL);
    }
    memset(cur, 0, sizeof(hw_xmlNode));
    cur->type = XML_ELEMENT_NODE;
    
    cur->name = hw_xmlStrdup(name);
    cur->ns = ns;
    return(cur);
}

/**
 * hw_xmlNewNodeEatName:
 * @ns:  namespace if any
 * @name:  the node name
 *
 * Creation of a new node element. @ns is optional (NULL).
 *
 * Returns a pointer to the new node object, with pointer @name as
 * new node's name. Use hw_xmlNewNode() if a copy of @name string is
 * is needed as new node's name.
 */
hw_xmlNodePtr
hw_xmlNewNodeEatName(hw_xmlNsPtr ns, hw_xmlChar *name) {
    hw_xmlNodePtr cur;

    if (name == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlNewNode : name == NULL\n");
#endif
	return(NULL);
    }

    /*
     * Allocate a new node and fill the fields.
     */
    cur = (hw_xmlNodePtr) hw_xmlMalloc(sizeof(hw_xmlNode));
    if (cur == NULL) {
	hw_xmlFree(name);
	xmlTreeErrMemory("building node");
	return(NULL);
    }
    memset(cur, 0, sizeof(hw_xmlNode));
    cur->type = XML_ELEMENT_NODE;
    
    cur->name = name;
    cur->ns = ns;

    return(cur);
}

/**
 * hw_xmlNewDocNode:
 * @doc:  the document
 * @ns:  namespace if any
 * @name:  the node name
 * @content:  the XML text content if any
 *
 * Creation of a new node element within a document. @ns and @content
 * are optional (NULL).
 * NOTE: @content is supposed to be a piece of XML CDATA, so it allow entities
 *       references, but XML special chars need to be escaped first by using
 *       hw_xmlEncodeEntitiesReentrant(). Use xmlNewDocRawNode() if you don't
 *       need entities support.
 *
 * Returns a pointer to the new node object.
 */
hw_xmlNodePtr
hw_xmlNewDocNode(hw_xmlDocPtr doc, hw_xmlNsPtr ns,
              const hw_xmlChar *name, const hw_xmlChar *content) {
    hw_xmlNodePtr cur;

    if ((doc != NULL) && (doc->dict != NULL))
        cur = hw_xmlNewNodeEatName(ns, (hw_xmlChar *)
	                        hw_xmlDictLookup(doc->dict, name, -1));
    else
	cur = hw_xmlNewNode(ns, name);
    if (cur != NULL) {
        cur->doc = doc;
	if (content != NULL) {
	    cur->children = hw_xmlStringGetNodeList(doc, content);
	    UPDATE_LAST_CHILD_AND_PARENT(cur)
	}
    }

    return(cur);
}

/**
 * hw_xmlNewDocNodeEatName:
 * @doc:  the document
 * @ns:  namespace if any
 * @name:  the node name
 * @content:  the XML text content if any
 *
 * Creation of a new node element within a document. @ns and @content
 * are optional (NULL).
 * NOTE: @content is supposed to be a piece of XML CDATA, so it allow entities
 *       references, but XML special chars need to be escaped first by using
 *       hw_xmlEncodeEntitiesReentrant(). Use xmlNewDocRawNode() if you don't
 *       need entities support.
 *
 * Returns a pointer to the new node object.
 */
hw_xmlNodePtr
hw_xmlNewDocNodeEatName(hw_xmlDocPtr doc, hw_xmlNsPtr ns,
              hw_xmlChar *name, const hw_xmlChar *content) {
    hw_xmlNodePtr cur;

    cur = hw_xmlNewNodeEatName(ns, name);
    if (cur != NULL) {
        cur->doc = doc;
	if (content != NULL) {
	    cur->children = hw_xmlStringGetNodeList(doc, content);
	    UPDATE_LAST_CHILD_AND_PARENT(cur)
	}
    }
    return(cur);
}

/**
 * hw_xmlNewText:
 * @content:  the text content
 *
 * Creation of a new text node.
 * Returns a pointer to the new node object.
 */
hw_xmlNodePtr
hw_xmlNewText(const hw_xmlChar *content) {
    hw_xmlNodePtr cur;

    /*
     * Allocate a new node and fill the fields.
     */
    cur = (hw_xmlNodePtr) hw_xmlMalloc(sizeof(hw_xmlNode));
    if (cur == NULL) {
	xmlTreeErrMemory("building text");
	return(NULL);
    }
    memset(cur, 0, sizeof(hw_xmlNode));
    cur->type = XML_TEXT_NODE;

    cur->name = hw_xmlStringText;
    if (content != NULL) {
	cur->content = hw_xmlStrdup(content);
    }

    return(cur);
}

/**
 * hw_xmlNewCharRef:
 * @doc: the document
 * @name:  the char ref string, starting with # or "&# ... ;"
 *
 * Creation of a new character reference node.
 * Returns a pointer to the new node object.
 */
hw_xmlNodePtr
hw_xmlNewCharRef(hw_xmlDocPtr doc, const hw_xmlChar *name) {
    hw_xmlNodePtr cur;

    if (name == NULL)
        return(NULL);

    /*
     * Allocate a new node and fill the fields.
     */
    cur = (hw_xmlNodePtr) hw_xmlMalloc(sizeof(hw_xmlNode));
    if (cur == NULL) {
	xmlTreeErrMemory("building character reference");
	return(NULL);
    }
    memset(cur, 0, sizeof(hw_xmlNode));
    cur->type = XML_ENTITY_REF_NODE;

    cur->doc = doc;
    if (name[0] == '&') {
        int len;
        name++;
	len = hw_xmlStrlen(name);
	if (name[len - 1] == ';')
	    cur->name = hw_xmlStrndup(name, len - 1);
	else
	    cur->name = hw_xmlStrndup(name, len);
    } else
	cur->name = hw_xmlStrdup(name);

    return(cur);
}

/**
 * hw_xmlNewReference:
 * @doc: the document
 * @name:  the reference name, or the reference string with & and ;
 *
 * Creation of a new reference node.
 * Returns a pointer to the new node object.
 */
hw_xmlNodePtr
hw_xmlNewReference(hw_xmlDocPtr doc, const hw_xmlChar *name) {
    hw_xmlNodePtr cur;
    hw_xmlEntityPtr ent;

    if (name == NULL)
        return(NULL);

    /*
     * Allocate a new node and fill the fields.
     */
    cur = (hw_xmlNodePtr) hw_xmlMalloc(sizeof(hw_xmlNode));
    if (cur == NULL) {
	xmlTreeErrMemory("building reference");
	return(NULL);
    }
    memset(cur, 0, sizeof(hw_xmlNode));
    cur->type = XML_ENTITY_REF_NODE;

    cur->doc = doc;
    if (name[0] == '&') {
        int len;
        name++;
	len = hw_xmlStrlen(name);
	if (name[len - 1] == ';')
	    cur->name = hw_xmlStrndup(name, len - 1);
	else
	    cur->name = hw_xmlStrndup(name, len);
    } else
	cur->name = hw_xmlStrdup(name);

    ent = hw_xmlGetDocEntity(doc, cur->name);
    if (ent != NULL) {
	cur->content = ent->content;
	/*
	 * The parent pointer in entity is a DTD pointer and thus is NOT
	 * updated.  Not sure if this is 100% correct.
	 *  -George
	 */
	cur->children = (hw_xmlNodePtr) ent;
	cur->last = (hw_xmlNodePtr) ent;
    }

    return(cur);
}

/**
 * hw_xmlNewDocText:
 * @doc: the document
 * @content:  the text content
 *
 * Creation of a new text node within a document.
 * Returns a pointer to the new node object.
 */
hw_xmlNodePtr
hw_xmlNewDocText(hw_xmlDocPtr doc, const hw_xmlChar *content) {
    hw_xmlNodePtr cur;

    cur = hw_xmlNewText(content);
    if (cur != NULL) cur->doc = doc;
    return(cur);
}

/**
 * hw_xmlNewTextLen:
 * @content:  the text content
 * @len:  the text len.
 *
 * Creation of a new text node with an extra parameter for the content's length
 * Returns a pointer to the new node object.
 */
hw_xmlNodePtr
hw_xmlNewTextLen(const hw_xmlChar *content, int len) {
    hw_xmlNodePtr cur;

    /*
     * Allocate a new node and fill the fields.
     */
    cur = (hw_xmlNodePtr) hw_xmlMalloc(sizeof(hw_xmlNode));
    if (cur == NULL) {
	xmlTreeErrMemory("building text");
	return(NULL);
    }
    memset(cur, 0, sizeof(hw_xmlNode));
    cur->type = XML_TEXT_NODE;

    cur->name = hw_xmlStringText;
    if (content != NULL) {
	cur->content = hw_xmlStrndup(content, len);
    }

    return(cur);
}

/**
 * hw_xmlNewDocTextLen:
 * @doc: the document
 * @content:  the text content
 * @len:  the text len.
 *
 * Creation of a new text node with an extra content length parameter. The
 * text node pertain to a given document.
 * Returns a pointer to the new node object.
 */
hw_xmlNodePtr
hw_xmlNewDocTextLen(hw_xmlDocPtr doc, const hw_xmlChar *content, int len) {
    hw_xmlNodePtr cur;

    cur = hw_xmlNewTextLen(content, len);
    if (cur != NULL) cur->doc = doc;
    return(cur);
}

/**
 * hw_xmlNewComment:
 * @content:  the comment content
 *
 * Creation of a new node containing a comment.
 * Returns a pointer to the new node object.
 */
hw_xmlNodePtr
hw_xmlNewComment(const hw_xmlChar *content) {
    hw_xmlNodePtr cur;

    /*
     * Allocate a new node and fill the fields.
     */
    cur = (hw_xmlNodePtr) hw_xmlMalloc(sizeof(hw_xmlNode));
    if (cur == NULL) {
	xmlTreeErrMemory("building comment");
	return(NULL);
    }
    memset(cur, 0, sizeof(hw_xmlNode));
    cur->type = XML_COMMENT_NODE;

    cur->name = hw_xmlStringComment;
    if (content != NULL) {
	cur->content = hw_xmlStrdup(content);
    }
    return(cur);
}

/**
 * hw_xmlNewCDataBlock:
 * @doc:  the document
 * @content:  the CDATA block content content
 * @len:  the length of the block
 *
 * Creation of a new node containing a CDATA block.
 * Returns a pointer to the new node object.
 */
hw_xmlNodePtr
hw_xmlNewCDataBlock(hw_xmlDocPtr doc, const hw_xmlChar *content, int len) {
    hw_xmlNodePtr cur;

    /*
     * Allocate a new node and fill the fields.
     */
    cur = (hw_xmlNodePtr) hw_xmlMalloc(sizeof(hw_xmlNode));
    if (cur == NULL) {
	xmlTreeErrMemory("building CDATA");
	return(NULL);
    }
    memset(cur, 0, sizeof(hw_xmlNode));
    cur->type = XML_CDATA_SECTION_NODE;
    cur->doc = doc;

    if (content != NULL) {
	cur->content = hw_xmlStrndup(content, len);
    }

    return(cur);
}

/**
 * hw_xmlNewDocComment:
 * @doc:  the document
 * @content:  the comment content
 *
 * Creation of a new node containing a comment within a document.
 * Returns a pointer to the new node object.
 */
hw_xmlNodePtr
hw_xmlNewDocComment(hw_xmlDocPtr doc, const hw_xmlChar *content) {
    hw_xmlNodePtr cur;

    cur = hw_xmlNewComment(content);
    if (cur != NULL) cur->doc = doc;
    return(cur);
}

/**
 * hw_xmlSetTreeDoc:
 * @tree:  the top element
 * @doc:  the document
 *
 * update all nodes under the tree to point to the right document
 */
void
hw_xmlSetTreeDoc(hw_xmlNodePtr tree, hw_xmlDocPtr doc) {
    hw_xmlAttrPtr prop;

    if (tree == NULL)
	return;
    if (tree->doc != doc) {
	if(tree->type == XML_ELEMENT_NODE) {
	    prop = tree->properties;
	    while (prop != NULL) {
		prop->doc = doc;
		hw_xmlSetListDoc(prop->children, doc);
		prop = prop->next;
	    }
	}
	if (tree->children != NULL)
	    hw_xmlSetListDoc(tree->children, doc);
	tree->doc = doc;
    }
}

/**
 * hw_xmlSetListDoc:
 * @list:  the first element
 * @doc:  the document
 *
 * update all nodes in the list to point to the right document
 */
void
hw_xmlSetListDoc(hw_xmlNodePtr list, hw_xmlDocPtr doc) {
    hw_xmlNodePtr cur;

    if (list == NULL)
	return;
    cur = list;
    while (cur != NULL) {
	if (cur->doc != doc)
	    hw_xmlSetTreeDoc(cur, doc);
	cur = cur->next;
    }
}

/**
 * xmlAddPropSibling:
 * @prev:  the attribute to which @prop is added after 
 * @cur:   the base attribute passed to calling function
 * @prop:  the new attribute
 *
 * Add a new attribute after @prev using @cur as base attribute.
 * When inserting before @cur, @prev is passed as @cur->prev.
 * When inserting after @cur, @prev is passed as @cur.
 * If an existing attribute is found it is detroyed prior to adding @prop. 
 *
 * Returns the attribute being inserted or NULL in case of error.
 */
static hw_xmlNodePtr
xmlAddPropSibling(hw_xmlNodePtr prev, hw_xmlNodePtr cur, hw_xmlNodePtr prop) {
	hw_xmlAttrPtr attr;

	if (cur->type != XML_ATTRIBUTE_NODE)
		return(NULL);

	/* check if an attribute with the same name exists */
	if (prop->ns == NULL)
		attr = hw_xmlHasNsProp(cur->parent, prop->name, NULL);
	else
		attr = hw_xmlHasNsProp(cur->parent, prop->name, prop->ns->href);

	if (prop->doc != cur->doc) {
		hw_xmlSetTreeDoc(prop, cur->doc);
	}
	prop->parent = cur->parent;
	prop->prev = prev;
	if (prev != NULL) {
		prop->next = prev->next;
		prev->next = prop;
		if (prop->next)
			prop->next->prev = prop;
	} else {
		prop->next = cur;
		cur->prev = prop;
	}
	if (prop->prev == NULL && prop->parent != NULL)
		prop->parent->properties = (hw_xmlAttrPtr) prop;
	if ((attr != NULL) && (attr->type != XML_ATTRIBUTE_DECL)) {
		/* different instance, destroy it (attributes must be unique) */
		hw_xmlRemoveProp((hw_xmlAttrPtr) attr);
	}
	return prop;
}

/**
 * hw_xmlAddNextSibling:
 * @cur:  the child node
 * @elem:  the new node
 *
 * Add a new node @elem as the next sibling of @cur
 * If the new node was already inserted in a document it is
 * first unlinked from its existing context.
 * As a result of text merging @elem may be freed.
 * If the new node is ATTRIBUTE, it is added into properties instead of children.
 * If there is an attribute with equal name, it is first destroyed. 
 *
 * Returns the new node or NULL in case of error.
 */
hw_xmlNodePtr
hw_xmlAddNextSibling(hw_xmlNodePtr cur, hw_xmlNodePtr elem) {
    if (cur == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlAddNextSibling : cur == NULL\n");
#endif
	return(NULL);
    }
    if (elem == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlAddNextSibling : elem == NULL\n");
#endif
	return(NULL);
    }

    if (cur == elem) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlAddNextSibling : cur == elem\n");
#endif
	return(NULL);
    }

    hw_xmlUnlinkNode(elem);

    if (elem->type == XML_TEXT_NODE) {
	if (cur->type == XML_TEXT_NODE) {
	    hw_xmlNodeAddContent(cur, elem->content);
	    hw_xmlFreeNode(elem);
	    return(cur);
	}
	if ((cur->next != NULL) && (cur->next->type == XML_TEXT_NODE) &&
            (cur->name == cur->next->name)) {
	    hw_xmlChar *tmp;

	    tmp = hw_xmlStrdup(elem->content);
	    tmp = hw_xmlStrcat(tmp, cur->next->content);
	    hw_xmlNodeSetContent(cur->next, tmp);
	    hw_xmlFree(tmp);
	    hw_xmlFreeNode(elem);
	    return(cur->next);
	}
    } else if (elem->type == XML_ATTRIBUTE_NODE) {
		return xmlAddPropSibling(cur, cur, elem);
    }

    if (elem->doc != cur->doc) {
	hw_xmlSetTreeDoc(elem, cur->doc);
    }
    elem->parent = cur->parent;
    elem->prev = cur;
    elem->next = cur->next;
    cur->next = elem;
    if (elem->next != NULL)
	elem->next->prev = elem;
    if ((elem->parent != NULL) && (elem->parent->last == cur))
	elem->parent->last = elem;
    return(elem);
}

/**
 * hw_xmlAddSibling:
 * @cur:  the child node
 * @elem:  the new node
 *
 * Add a new element @elem to the list of siblings of @cur
 * merging adjacent TEXT nodes (@elem may be freed)
 * If the new element was already inserted in a document it is
 * first unlinked from its existing context.
 *
 * Returns the new element or NULL in case of error.
 */
hw_xmlNodePtr
hw_xmlAddSibling(hw_xmlNodePtr cur, hw_xmlNodePtr elem) {
    hw_xmlNodePtr parent;

    if (cur == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlAddSibling : cur == NULL\n");
#endif
	return(NULL);
    }

    if (elem == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlAddSibling : elem == NULL\n");
#endif
	return(NULL);
    }

    /*
     * Constant time is we can rely on the ->parent->last to find
     * the last sibling.
     */
    if ((cur->type != XML_ATTRIBUTE_NODE) && (cur->parent != NULL) && 
	(cur->parent->children != NULL) &&
	(cur->parent->last != NULL) &&
	(cur->parent->last->next == NULL)) {
	cur = cur->parent->last;
    } else {
	while (cur->next != NULL) cur = cur->next;
    }

    hw_xmlUnlinkNode(elem);

    if ((cur->type == XML_TEXT_NODE) && (elem->type == XML_TEXT_NODE) &&
        (cur->name == elem->name)) {
	hw_xmlNodeAddContent(cur, elem->content);
	hw_xmlFreeNode(elem);
	return(cur);
    } else if (elem->type == XML_ATTRIBUTE_NODE) {
		return xmlAddPropSibling(cur, cur, elem);
    }

    if (elem->doc != cur->doc) {
	hw_xmlSetTreeDoc(elem, cur->doc);
    }
    parent = cur->parent;
    elem->prev = cur;
    elem->next = NULL;
    elem->parent = parent;
    cur->next = elem;
    if (parent != NULL)
	parent->last = elem;

    return(elem);
}

/**
 * hw_xmlAddChildList:
 * @parent:  the parent node
 * @cur:  the first node in the list
 *
 * Add a list of node at the end of the child list of the parent
 * merging adjacent TEXT nodes (@cur may be freed)
 *
 * Returns the last child or NULL in case of error.
 */
hw_xmlNodePtr
hw_xmlAddChildList(hw_xmlNodePtr parent, hw_xmlNodePtr cur) {
    hw_xmlNodePtr prev;

    if (parent == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlAddChildList : parent == NULL\n");
#endif
	return(NULL);
    }

    if (cur == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlAddChildList : child == NULL\n");
#endif
	return(NULL);
    }

    if ((cur->doc != NULL) && (parent->doc != NULL) &&
        (cur->doc != parent->doc)) {
#ifdef DEBUG_TREE
	hw_xmlGenericError(hw_xmlGenericErrorContext,
		"Elements moved to a different document\n");
#endif
    }

    /*
     * add the first element at the end of the children list.
     */

    if (parent->children == NULL) {
        parent->children = cur;
    } else {
	/*
	 * If cur and parent->last both are TEXT nodes, then merge them.
	 */
	if ((cur->type == XML_TEXT_NODE) && 
	    (parent->last->type == XML_TEXT_NODE) &&
	    (cur->name == parent->last->name)) {
    	    hw_xmlNodeAddContent(parent->last, cur->content);
	    /*
	     * if it's the only child, nothing more to be done.
	     */
	    if (cur->next == NULL) {
		hw_xmlFreeNode(cur);
		return(parent->last);
	    }
	    prev = cur;
	    cur = cur->next;
	    hw_xmlFreeNode(prev);
	}
        prev = parent->last;
	prev->next = cur;
	cur->prev = prev;
    }
    while (cur->next != NULL) {
	cur->parent = parent;
	if (cur->doc != parent->doc) {
	    hw_xmlSetTreeDoc(cur, parent->doc);
	}
        cur = cur->next;
    }
    cur->parent = parent;
    cur->doc = parent->doc; /* the parent may not be linked to a doc ! */
    parent->last = cur;

    return(cur);
}

/**
 * hw_xmlAddChild:
 * @parent:  the parent node
 * @cur:  the child node
 *
 * Add a new node to @parent, at the end of the child (or property) list
 * merging adjacent TEXT nodes (in which case @cur is freed)
 * If the new node is ATTRIBUTE, it is added into properties instead of children.
 * If there is an attribute with equal name, it is first destroyed. 
 *
 * Returns the child or NULL in case of error.
 */
hw_xmlNodePtr
hw_xmlAddChild(hw_xmlNodePtr parent, hw_xmlNodePtr cur) {
    hw_xmlNodePtr prev;

    if (parent == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlAddChild : parent == NULL\n");
#endif
	return(NULL);
    }

    if (cur == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlAddChild : child == NULL\n");
#endif
	return(NULL);
    }

    if (parent == cur) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlAddChild : parent == cur\n");
#endif
	return(NULL);
    }
    /*
     * If cur is a TEXT node, merge its content with adjacent TEXT nodes
     * cur is then freed.
     */
    if (cur->type == XML_TEXT_NODE) {
	if ((parent->type == XML_TEXT_NODE) &&
	    (parent->content != NULL) &&
	    (parent->name == cur->name)) {
	    hw_xmlNodeAddContent(parent, cur->content);
	    hw_xmlFreeNode(cur);
	    return(parent);
	}
	if ((parent->last != NULL) && (parent->last->type == XML_TEXT_NODE) &&
	    (parent->last->name == cur->name) &&
	    (parent->last != cur)) {
	    hw_xmlNodeAddContent(parent->last, cur->content);
	    hw_xmlFreeNode(cur);
	    return(parent->last);
	}
    }

    /*
     * add the new element at the end of the children list.
     */
    prev = cur->parent;
    cur->parent = parent;
    if (cur->doc != parent->doc) {
	hw_xmlSetTreeDoc(cur, parent->doc);
    }
    /* this check prevents a loop on tree-traversions if a developer
     * tries to add a node to its parent multiple times
     */
    if (prev == parent)
	return(cur);

    /*
     * Coalescing
     */
    if ((parent->type == XML_TEXT_NODE) &&
	(parent->content != NULL) &&
	(parent != cur)) {
	hw_xmlNodeAddContent(parent, cur->content);
	hw_xmlFreeNode(cur);
	return(parent);
    }
    if (cur->type == XML_ATTRIBUTE_NODE) {
		if (parent->type != XML_ELEMENT_NODE)
			return(NULL);
	if (parent->properties == NULL) {
	    parent->properties = (hw_xmlAttrPtr) cur;
	} else {
	    /* check if an attribute with the same name exists */
	    hw_xmlAttrPtr lastattr;

	    if (cur->ns == NULL)
		lastattr = hw_xmlHasNsProp(parent, cur->name, NULL);
	    else
		lastattr = hw_xmlHasNsProp(parent, cur->name, cur->ns->href);
	    if ((lastattr != NULL) && (lastattr != (hw_xmlAttrPtr) cur) && (lastattr->type != XML_ATTRIBUTE_DECL)) {
		/* different instance, destroy it (attributes must be unique) */
			hw_xmlUnlinkNode((hw_xmlNodePtr) lastattr);
		hw_xmlFreeProp(lastattr);
	    }
		if (lastattr == (hw_xmlAttrPtr) cur)
			return(cur);
	    /* find the end */
	    lastattr = parent->properties;
	    while (lastattr->next != NULL) {
		lastattr = lastattr->next;
	    }
	    lastattr->next = (hw_xmlAttrPtr) cur;
	    ((hw_xmlAttrPtr) cur)->prev = lastattr;
	}
    } else {
	if (parent->children == NULL) {
	    parent->children = cur;
	    parent->last = cur;
	} else {
	    prev = parent->last;
	    prev->next = cur;
	    cur->prev = prev;
	    parent->last = cur;
	}
    }
    return(cur);
}

/**
 * hw_xmlGetLastChild:
 * @parent:  the parent node
 *
 * Search the last child of a node.
 * Returns the last child or NULL if none.
 */
hw_xmlNodePtr
hw_xmlGetLastChild(hw_xmlNodePtr parent) {
    if (parent == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlGetLastChild : parent == NULL\n");
#endif
	return(NULL);
    }
    return(parent->last);
}

/**
 * hw_xmlFreeNodeList:
 * @cur:  the first node in the list
 *
 * Free a node and all its siblings, this is a recursive behaviour, all
 * the children are freed too.
 */
void
hw_xmlFreeNodeList(hw_xmlNodePtr cur) {
    hw_xmlNodePtr next;
    hw_xmlDictPtr dict = NULL;

    if (cur == NULL) return;
    if (cur->type == XML_NAMESPACE_DECL) {
	hw_xmlFreeNsList((hw_xmlNsPtr) cur);
	return;
    }
    if ((cur->type == XML_DOCUMENT_NODE) ||
	(cur->type == XML_HTML_DOCUMENT_NODE)) {
	hw_xmlFreeDoc((hw_xmlDocPtr) cur);
	return;
    }
    if (cur->doc != NULL) dict = cur->doc->dict;
    while (cur != NULL) {
        next = cur->next;
	if (cur->type != XML_DTD_NODE) {

	    if ((cur->children != NULL) &&
		(cur->type != XML_ENTITY_REF_NODE))
		hw_xmlFreeNodeList(cur->children);
	    if (((cur->type == XML_ELEMENT_NODE) ||
		 (cur->type == XML_XINCLUDE_START) ||
		 (cur->type == XML_XINCLUDE_END)) &&
		(cur->properties != NULL))
		hw_xmlFreePropList(cur->properties);
	    if ((cur->type != XML_ELEMENT_NODE) &&
		(cur->type != XML_XINCLUDE_START) &&
		(cur->type != XML_XINCLUDE_END) &&
		(cur->type != XML_ENTITY_REF_NODE) &&
		(cur->content != (hw_xmlChar *) &(cur->properties))) {
		hw_DICT_FREE(cur->content)
	    }
	    if (((cur->type == XML_ELEMENT_NODE) ||
	         (cur->type == XML_XINCLUDE_START) ||
		 (cur->type == XML_XINCLUDE_END)) &&
		(cur->nsDef != NULL))
		hw_xmlFreeNsList(cur->nsDef);

	    /*
	     * When a node is a text node or a comment, it uses a global static
	     * variable for the name of the node.
	     * Otherwise the node name might come from the document's
	     * dictionnary
	     */
	    if ((cur->name != NULL) &&
		(cur->type != XML_TEXT_NODE) &&
		(cur->type != XML_COMMENT_NODE))
		hw_DICT_FREE(cur->name)
	    hw_xmlFree(cur);
	}
	cur = next;
    }
}

/**
 * hw_xmlFreeNode:
 * @cur:  the node
 *
 * Free a node, this is a recursive behaviour, all the children are freed too.
 * This doesn't unlink the child from the list, use hw_xmlUnlinkNode() first.
 */
void
hw_xmlFreeNode(hw_xmlNodePtr cur) {
    hw_xmlDictPtr dict = NULL;

    if (cur == NULL) return;

    /* use hw_xmlFreeDtd for DTD nodes */
    if (cur->type == XML_DTD_NODE) {
	hw_xmlFreeDtd((hw_xmlDtdPtr) cur);
	return;
    }
    if (cur->type == XML_NAMESPACE_DECL) {
	hw_xmlFreeNs((hw_xmlNsPtr) cur);
        return;
    }
    if (cur->type == XML_ATTRIBUTE_NODE) {
	hw_xmlFreeProp((hw_xmlAttrPtr) cur);
	return;
    }

    if (cur->doc != NULL) dict = cur->doc->dict;

    if ((cur->children != NULL) &&
	(cur->type != XML_ENTITY_REF_NODE))
	hw_xmlFreeNodeList(cur->children);
    if (((cur->type == XML_ELEMENT_NODE) ||
	 (cur->type == XML_XINCLUDE_START) ||
	 (cur->type == XML_XINCLUDE_END)) &&
	(cur->properties != NULL))
	hw_xmlFreePropList(cur->properties);
    if ((cur->type != XML_ELEMENT_NODE) &&
	(cur->content != NULL) &&
	(cur->type != XML_ENTITY_REF_NODE) &&
	(cur->type != XML_XINCLUDE_END) &&
	(cur->type != XML_XINCLUDE_START) &&
	(cur->content != (hw_xmlChar *) &(cur->properties))) {
	hw_DICT_FREE(cur->content)
    }

    /*
     * When a node is a text node or a comment, it uses a global static
     * variable for the name of the node.
     * Otherwise the node name might come from the document's dictionnary
     */
    if ((cur->name != NULL) &&
        (cur->type != XML_TEXT_NODE) &&
        (cur->type != XML_COMMENT_NODE))
	hw_DICT_FREE(cur->name)

    if (((cur->type == XML_ELEMENT_NODE) ||
	 (cur->type == XML_XINCLUDE_START) ||
	 (cur->type == XML_XINCLUDE_END)) &&
	(cur->nsDef != NULL))
	hw_xmlFreeNsList(cur->nsDef);
    hw_xmlFree(cur);
}

/**
 * hw_xmlUnlinkNode:
 * @cur:  the node
 *
 * Unlink a node from it's current context, the node is not freed
 */
void
hw_xmlUnlinkNode(hw_xmlNodePtr cur) {
    if (cur == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlUnlinkNode : node == NULL\n");
#endif
	return;
    }
    if (cur->type == XML_DTD_NODE) {
	hw_xmlDocPtr doc;
	doc = cur->doc;
	if (doc != NULL) {
	    if (doc->intSubset == (hw_xmlDtdPtr) cur)
		doc->intSubset = NULL;
	    if (doc->extSubset == (hw_xmlDtdPtr) cur)
		doc->extSubset = NULL;
	}
    }
    if (cur->parent != NULL) {
	hw_xmlNodePtr parent;
	parent = cur->parent;
	if (cur->type == XML_ATTRIBUTE_NODE) {
	    if (parent->properties == (hw_xmlAttrPtr) cur)
		parent->properties = ((hw_xmlAttrPtr) cur)->next;
	} else {
	    if (parent->children == cur)
		parent->children = cur->next;
	    if (parent->last == cur)
		parent->last = cur->prev;
	}
	cur->parent = NULL;
    }
    if (cur->next != NULL)
        cur->next->prev = cur->prev;
    if (cur->prev != NULL)
        cur->prev->next = cur->next;
    cur->next = cur->prev = NULL;
}


/************************************************************************
 *									*
 *		Copy operations						*
 *									*
 ************************************************************************/
 
/**
 * hw_xmlCopyNamespace:
 * @cur:  the namespace
 *
 * Do a copy of the namespace.
 *
 * Returns: a new #hw_xmlNsPtr, or NULL in case of error.
 */
hw_xmlNsPtr
hw_xmlCopyNamespace(hw_xmlNsPtr cur) {
    hw_xmlNsPtr ret;

    if (cur == NULL) return(NULL);
    switch (cur->type) {
	case hw_XML_LOCAL_NAMESPACE:
	    ret = hw_xmlNewNs(NULL, cur->href, cur->prefix);
	    break;
	default:
#ifdef DEBUG_TREE
	    hw_xmlGenericError(hw_xmlGenericErrorContext,
		    "hw_xmlCopyNamespace: invalid type %d\n", cur->type);
#endif
	    return(NULL);
    }
    return(ret);
}

/**
 * hw_xmlCopyNamespaceList:
 * @cur:  the first namespace
 *
 * Do a copy of an namespace list.
 *
 * Returns: a new #hw_xmlNsPtr, or NULL in case of error.
 */
hw_xmlNsPtr
hw_xmlCopyNamespaceList(hw_xmlNsPtr cur) {
    hw_xmlNsPtr ret = NULL;
    hw_xmlNsPtr p = NULL,q;

    while (cur != NULL) {
        q = hw_xmlCopyNamespace(cur);
	if (p == NULL) {
	    ret = p = q;
	} else {
	    p->next = q;
	    p = q;
	}
	cur = cur->next;
    }
    return(ret);
}

static hw_xmlNodePtr
xmlStaticCopyNodeList(hw_xmlNodePtr node, hw_xmlDocPtr doc, hw_xmlNodePtr parent);

static hw_xmlAttrPtr
xmlCopyPropInternal(hw_xmlDocPtr doc, hw_xmlNodePtr target, hw_xmlAttrPtr cur) {
    hw_xmlAttrPtr ret;

    if (cur == NULL) return(NULL);
    if (target != NULL)
	ret = hw_xmlNewDocProp(target->doc, cur->name, NULL);
    else if (doc != NULL)
	ret = hw_xmlNewDocProp(doc, cur->name, NULL);
    else if (cur->parent != NULL)
	ret = hw_xmlNewDocProp(cur->parent->doc, cur->name, NULL);
    else if (cur->children != NULL)
	ret = hw_xmlNewDocProp(cur->children->doc, cur->name, NULL);
    else
	ret = hw_xmlNewDocProp(NULL, cur->name, NULL);
    if (ret == NULL) return(NULL);
    ret->parent = target;

    if ((cur->ns != NULL) && (target != NULL)) {
      hw_xmlNsPtr ns;

      ns = hw_xmlSearchNs(target->doc, target, cur->ns->prefix);
      if (ns == NULL) {
        /*
         * Humm, we are copying an element whose namespace is defined
         * out of the new tree scope. Search it in the original tree
         * and add it at the top of the new tree
         */
        ns = hw_xmlSearchNs(cur->doc, cur->parent, cur->ns->prefix);
        if (ns != NULL) {
          hw_xmlNodePtr root = target;
          hw_xmlNodePtr pred = NULL;

          while (root->parent != NULL) {
            pred = root;
            root = root->parent;
          }
          if (root == (hw_xmlNodePtr) target->doc) {
            /* correct possibly cycling above the document elt */
            root = pred;
          }
          ret->ns = hw_xmlNewNs(root, ns->href, ns->prefix);
        }
      } else {
        /*
         * we have to find something appropriate here since
         * we cant be sure, that the namespce we found is identified
         * by the prefix
         */
        if (hw_xmlStrEqual(ns->href, cur->ns->href)) {
          /* this is the nice case */
          ret->ns = ns;
        } else {
          /*
           * we are in trouble: we need a new reconcilied namespace.
           * This is expensive
           */
          ret->ns = xmlNewReconciliedNs(target->doc, target, cur->ns);
        }
      }
 
    } else
        ret->ns = NULL;

    if (cur->children != NULL) {
	hw_xmlNodePtr tmp;

	ret->children = xmlStaticCopyNodeList(cur->children, ret->doc, (hw_xmlNodePtr) ret);
	ret->last = NULL;
	tmp = ret->children;
	while (tmp != NULL) {
	    /* tmp->parent = (hw_xmlNodePtr)ret; */
	    if (tmp->next == NULL)
	        ret->last = tmp;
	    tmp = tmp->next;
	}
    }
    /*
     * Try to handle IDs
     */
    if ((target!= NULL) && (cur!= NULL) &&
	(target->doc != NULL) && (cur->doc != NULL) &&
	(cur->doc->ids != NULL) && (cur->parent != NULL)) {
	if (hw_xmlIsID(cur->doc, cur->parent, cur)) {
	    hw_xmlChar *id;

	    id = hw_xmlNodeListGetString(cur->doc, cur->children, 1);
	    if (id != NULL) {
		hw_xmlAddID(NULL, target->doc, id, ret);
		hw_xmlFree(id);
	    }
	}
    }
    return(ret);
}

/**
 * hw_xmlCopyProp:
 * @target:  the element where the attribute will be grafted
 * @cur:  the attribute
 *
 * Do a copy of the attribute.
 *
 * Returns: a new #hw_xmlAttrPtr, or NULL in case of error.
 */
hw_xmlAttrPtr
hw_xmlCopyProp(hw_xmlNodePtr target, hw_xmlAttrPtr cur) {
	return xmlCopyPropInternal(NULL, target, cur);
}

/**
 * hw_xmlCopyPropList:
 * @target:  the element where the attributes will be grafted
 * @cur:  the first attribute
 *
 * Do a copy of an attribute list.
 *
 * Returns: a new #hw_xmlAttrPtr, or NULL in case of error.
 */
hw_xmlAttrPtr
hw_xmlCopyPropList(hw_xmlNodePtr target, hw_xmlAttrPtr cur) {
    hw_xmlAttrPtr ret = NULL;
    hw_xmlAttrPtr p = NULL,q;

    while (cur != NULL) {
        q = hw_xmlCopyProp(target, cur);
	if (q == NULL)
	    return(NULL);
	if (p == NULL) {
	    ret = p = q;
	} else {
	    p->next = q;
	    q->prev = p;
	    p = q;
	}
	cur = cur->next;
    }
    return(ret);
}

/*
 * NOTE about the CopyNode operations !
 *
 * They are split into external and internal parts for one
 * tricky reason: namespaces. Doing a direct copy of a node
 * say RPM:Copyright without changing the namespace pointer to
 * something else can produce stale links. One way to do it is
 * to keep a reference counter but this doesn't work as soon
 * as one move the element or the subtree out of the scope of
 * the existing namespace. The actual solution seems to add
 * a copy of the namespace at the top of the copied tree if
 * not available in the subtree.
 * Hence two functions, the public front-end call the inner ones
 * The argument "recursive" normally indicates a recursive copy
 * of the node with values 0 (no) and 1 (yes).  For XInclude,
 * however, we allow a value of 2 to indicate copy properties and
 * namespace info, but don't recurse on children.
 */

static hw_xmlNodePtr
xmlStaticCopyNode(const hw_xmlNodePtr node, hw_xmlDocPtr doc, hw_xmlNodePtr parent,
                  int extended) {
    hw_xmlNodePtr ret;

    if (node == NULL) return(NULL);
    switch (node->type) {
        case XML_TEXT_NODE:
        case XML_CDATA_SECTION_NODE:
        case XML_ELEMENT_NODE:
        case XML_DOCUMENT_FRAG_NODE:
        case XML_ENTITY_REF_NODE:
        case XML_ENTITY_NODE:
        case XML_PI_NODE:
        case XML_COMMENT_NODE:
        case XML_XINCLUDE_START:
        case XML_XINCLUDE_END:
	    break;
        case XML_ATTRIBUTE_NODE:
		return((hw_xmlNodePtr) xmlCopyPropInternal(doc, parent, (hw_xmlAttrPtr) node));
        case XML_NAMESPACE_DECL:
	    return((hw_xmlNodePtr) hw_xmlCopyNamespaceList((hw_xmlNsPtr) node));
	    
        case XML_DOCUMENT_NODE:
        case XML_HTML_DOCUMENT_NODE:
        case XML_DOCUMENT_TYPE_NODE:
        case XML_NOTATION_NODE:
        case XML_DTD_NODE:
        case XML_ELEMENT_DECL:
        case XML_ATTRIBUTE_DECL:
        case XML_ENTITY_DECL:
            return(NULL);
    }

    /*
     * Allocate a new node and fill the fields.
     */
    ret = (hw_xmlNodePtr) hw_xmlMalloc(sizeof(hw_xmlNode));
    if (ret == NULL) {
	xmlTreeErrMemory("copying node");
	return(NULL);
    }
    memset(ret, 0, sizeof(hw_xmlNode));
    ret->type = node->type;

    ret->doc = doc;
    ret->parent = parent; 
    if (node->name == hw_xmlStringText)
	ret->name = hw_xmlStringText;
    else if (node->name == hw_xmlStringTextNoenc)
	ret->name = hw_xmlStringTextNoenc;
    else if (node->name == hw_xmlStringComment)
	ret->name = hw_xmlStringComment;
    else if (node->name != NULL) {
        if ((doc != NULL) && (doc->dict != NULL))
	    ret->name = hw_xmlDictLookup(doc->dict, node->name, -1);
	else
	    ret->name = hw_xmlStrdup(node->name);
    }
    if ((node->type != XML_ELEMENT_NODE) &&
	(node->content != NULL) &&
	(node->type != XML_ENTITY_REF_NODE) &&
	(node->type != XML_XINCLUDE_END) &&
	(node->type != XML_XINCLUDE_START)) {
	ret->content = hw_xmlStrdup(node->content);
    }else{
      if (node->type == XML_ELEMENT_NODE)
        ret->line = node->line;
    }
    if (parent != NULL) {
	hw_xmlNodePtr tmp;

	/*
	 * this is a tricky part for the node register thing:
	 * in case ret does get coalesced in hw_xmlAddChild
	 * the deregister-node callback is called; so we register ret now already
	 */

        tmp = hw_xmlAddChild(parent, ret);
	/* node could have coalesced */
	if (tmp != ret)
	    return(tmp);
    }
    
    if (!extended)
	goto out;
    if ((node->type == XML_ELEMENT_NODE) && (node->nsDef != NULL))
        ret->nsDef = hw_xmlCopyNamespaceList(node->nsDef);

    if (node->ns != NULL) {
        hw_xmlNsPtr ns;

	ns = hw_xmlSearchNs(doc, ret, node->ns->prefix);
	if (ns == NULL) {
	    /*
	     * Humm, we are copying an element whose namespace is defined
	     * out of the new tree scope. Search it in the original tree
	     * and add it at the top of the new tree
	     */
	    ns = hw_xmlSearchNs(node->doc, node, node->ns->prefix);
	    if (ns != NULL) {
	        hw_xmlNodePtr root = ret;

		while (root->parent != NULL) root = root->parent;
		ret->ns = hw_xmlNewNs(root, ns->href, ns->prefix);
	    }
	} else {
	    /*
	     * reference the existing namespace definition in our own tree.
	     */
	    ret->ns = ns;
	}
    }
    if ((node->type == XML_ELEMENT_NODE) && (node->properties != NULL))
        ret->properties = hw_xmlCopyPropList(ret, node->properties);
    if (node->type == XML_ENTITY_REF_NODE) {
	if ((doc == NULL) || (node->doc != doc)) {
	    /*
	     * The copied node will go into a separate document, so
	     * to avoid dangling references to the ENTITY_DECL node
	     * we cannot keep the reference. Try to find it in the
	     * target document.
	     */
	    ret->children = (hw_xmlNodePtr) hw_xmlGetDocEntity(doc, ret->name);
	} else {
            ret->children = node->children;
	}
	ret->last = ret->children;
    } else if ((node->children != NULL) && (extended != 2)) {
        ret->children = xmlStaticCopyNodeList(node->children, doc, ret);
	UPDATE_LAST_CHILD_AND_PARENT(ret)
    }

out:
    return(ret);
}

static hw_xmlNodePtr
xmlStaticCopyNodeList(hw_xmlNodePtr node, hw_xmlDocPtr doc, hw_xmlNodePtr parent) {
    hw_xmlNodePtr ret = NULL;
    hw_xmlNodePtr p = NULL,q;

    while (node != NULL) {
	    q = xmlStaticCopyNode(node, doc, parent, 1);
	if (ret == NULL) {
	    q->prev = NULL;
	    ret = p = q;
	} else if (p != q) {
	/* the test is required if xmlStaticCopyNode coalesced 2 text nodes */
	    p->next = q;
	    q->prev = p;
	    p = q;
	}
	node = node->next;
    }
    return(ret);
}

/**
 * hw_xmlCopyNode:
 * @node:  the node
 * @extended:   if 1 do a recursive copy (properties, namespaces and children
 *			when applicable)
 *		if 2 copy properties and namespaces (when applicable)
 *
 * Do a copy of the node.
 *
 * Returns: a new #hw_xmlNodePtr, or NULL in case of error.
 */
hw_xmlNodePtr
hw_xmlCopyNode(const hw_xmlNodePtr node, int extended) {
    hw_xmlNodePtr ret;

    ret = xmlStaticCopyNode(node, NULL, NULL, extended);
    return(ret);
}

/**
 * hw_xmlDocCopyNode:
 * @node:  the node
 * @doc:  the document
 * @extended:   if 1 do a recursive copy (properties, namespaces and children
 *			when applicable)
 *		if 2 copy properties and namespaces (when applicable)
 *
 * Do a copy of the node to a given document.
 *
 * Returns: a new #hw_xmlNodePtr, or NULL in case of error.
 */
hw_xmlNodePtr
hw_xmlDocCopyNode(const hw_xmlNodePtr node, hw_xmlDocPtr doc, int extended) {
    hw_xmlNodePtr ret;

    ret = xmlStaticCopyNode(node, doc, NULL, extended);
    return(ret);
}

/**
 * hw_xmlDocCopyNodeList:
 * @doc: the target document
 * @node:  the first node in the list.
 *
 * Do a recursive copy of the node list.
 *
 * Returns: a new #hw_xmlNodePtr, or NULL in case of error.
 */
hw_xmlNodePtr hw_xmlDocCopyNodeList(hw_xmlDocPtr doc, const hw_xmlNodePtr node) {
    hw_xmlNodePtr ret = xmlStaticCopyNodeList(node, doc, NULL);
    return(ret);
}

/**
 * hw_xmlCopyNodeList:
 * @node:  the first node in the list.
 *
 * Do a recursive copy of the node list.
 * Use hw_xmlDocCopyNodeList() if possible to ensure string interning.
 *
 * Returns: a new #hw_xmlNodePtr, or NULL in case of error.
 */
hw_xmlNodePtr hw_xmlCopyNodeList(const hw_xmlNodePtr node) {
    hw_xmlNodePtr ret = xmlStaticCopyNodeList(node, NULL, NULL);
    return(ret);
}
/************************************************************************
 *									*
 *		Content access functions				*
 *									*
 ************************************************************************/
 
/**
 * hw_xmlGetLineNo:
 * @node: valid node
 *
 * Get line number of @node. This requires activation of this option
 * before invoking the parser by calling xmlLineNumbersDefault(1)
 *
 * Returns the line number if successful, -1 otherwise
 */
long
hw_xmlGetLineNo(hw_xmlNodePtr node)
{
    long result = -1;

    if (!node)
        return result;
    if ((node->type == XML_ELEMENT_NODE) ||
        (node->type == XML_TEXT_NODE) ||
	(node->type == XML_COMMENT_NODE) ||
	(node->type == XML_PI_NODE))
        result = (long) node->line;
    else if ((node->prev != NULL) &&
             ((node->prev->type == XML_ELEMENT_NODE) ||
	      (node->prev->type == XML_TEXT_NODE) ||
	      (node->prev->type == XML_COMMENT_NODE) ||
	      (node->prev->type == XML_PI_NODE)))
        result = hw_xmlGetLineNo(node->prev);
    else if ((node->parent != NULL) &&
             (node->parent->type == XML_ELEMENT_NODE))
        result = hw_xmlGetLineNo(node->parent);

    return result;
}

/**
 * hw_xmlDocGetRootElement:
 * @doc:  the document
 *
 * Get the root element of the document (doc->children is a list
 * containing possibly comments, PIs, etc ...).
 *
 * Returns the #hw_xmlNodePtr for the root or NULL
 */
hw_xmlNodePtr
hw_xmlDocGetRootElement(hw_xmlDocPtr doc) {
    hw_xmlNodePtr ret;

    if (doc == NULL) return(NULL);
    ret = doc->children;
    while (ret != NULL) {
	if (ret->type == XML_ELEMENT_NODE)
	    return(ret);
        ret = ret->next;
    }
    return(ret);
}
 
/**
 * hw_xmlNodeGetSpacePreserve:
 * @cur:  the node being checked
 *
 * Searches the space preserving behaviour of a node, i.e. the values
 * of the xml:space attribute or the one carried by the nearest
 * ancestor.
 *
 * Returns -1 if xml:space is not inherited, 0 if "default", 1 if "preserve"
 */
int
hw_xmlNodeGetSpacePreserve(hw_xmlNodePtr cur) {
    hw_xmlChar *space;

    while (cur != NULL) {
	space = hw_xmlGetNsProp(cur, hw_BAD_CAST "space", hw_XML_XML_NAMESPACE);
	if (space != NULL) {
	    if (hw_xmlStrEqual(space, hw_BAD_CAST "preserve")) {
		hw_xmlFree(space);
		return(1);
	    }
	    if (hw_xmlStrEqual(space, hw_BAD_CAST "default")) {
		hw_xmlFree(space);
		return(0);
	    }
	    hw_xmlFree(space);
	}
	cur = cur->parent;
    }
    return(-1);
}
 


/**
 * hw_xmlNodeGetBase:
 * @doc:  the document the node pertains to
 * @cur:  the node being checked
 *
 * Searches for the BASE URL. The code should work on both XML
 * and HTML document even if base mechanisms are completely different.
 * It returns the base as defined in RFC 2396 sections
 * 5.1.1. Base URI within Document Content
 * and
 * 5.1.2. Base URI from the Encapsulating Entity
 * However it does not return the document base (5.1.3), use
 * xmlDocumentGetBase() for this
 *
 * Returns a pointer to the base URL, or NULL if not found
 *     It's up to the caller to free the memory with hw_xmlFree().
 */
hw_xmlChar *
hw_xmlNodeGetBase(hw_xmlDocPtr doc, hw_xmlNodePtr cur) {
    hw_xmlChar *oldbase = NULL;
    hw_xmlChar *base, *newbase;

    if ((cur == NULL) && (doc == NULL)) 
        return(NULL);
    if (doc == NULL) doc = cur->doc;	
    if ((doc != NULL) && (doc->type == XML_HTML_DOCUMENT_NODE)) {
        cur = doc->children;
	while ((cur != NULL) && (cur->name != NULL)) {
	    if (cur->type != XML_ELEMENT_NODE) {
	        cur = cur->next;
		continue;
	    }
	    if (!hw_xmlStrcasecmp(cur->name, hw_BAD_CAST "html")) {
	        cur = cur->children;
		continue;
	    }
	    if (!hw_xmlStrcasecmp(cur->name, hw_BAD_CAST "head")) {
	        cur = cur->children;
		continue;
	    }
	    if (!hw_xmlStrcasecmp(cur->name, hw_BAD_CAST "base")) {
                return(hw_xmlGetProp(cur, hw_BAD_CAST "href"));
	    }
	    cur = cur->next;
	}
	return(NULL);
    }
    while (cur != NULL) {
	if (cur->type == XML_ENTITY_DECL) {
	    hw_xmlEntityPtr ent = (hw_xmlEntityPtr) cur;
	    return(hw_xmlStrdup(ent->URI));
	}
	if (cur->type == XML_ELEMENT_NODE) {
	    base = hw_xmlGetNsProp(cur, hw_BAD_CAST "base", hw_XML_XML_NAMESPACE);
	    if (base != NULL) {
		if (oldbase != NULL) {
		    newbase = hw_xmlBuildURI(oldbase, base);
		    if (newbase != NULL) {
			hw_xmlFree(oldbase);
			hw_xmlFree(base);
			oldbase = newbase;
		    } else {
			hw_xmlFree(oldbase);
			hw_xmlFree(base);
			return(NULL);
		    }
		} else {
		    oldbase = base;
		}
		if ((!hw_xmlStrncmp(oldbase, hw_BAD_CAST "http://", 7)) ||
		    (!hw_xmlStrncmp(oldbase, hw_BAD_CAST "ftp://", 6)) ||
		    (!hw_xmlStrncmp(oldbase, hw_BAD_CAST "urn:", 4)))
		    return(oldbase);
	    }
	}
	cur = cur->parent;
    }
    if ((doc != NULL) && (doc->URL != NULL)) {
	if (oldbase == NULL)
	    return(hw_xmlStrdup(doc->URL));
	newbase = hw_xmlBuildURI(oldbase, doc->URL);
	hw_xmlFree(oldbase);
	return(newbase);
    }
    return(oldbase);
}
 
/**
 * hw_xmlNodeSetContent:
 * @cur:  the node being modified
 * @content:  the new value of the content
 *
 * Replace the content of a node.
 */
void
hw_xmlNodeSetContent(hw_xmlNodePtr cur, const hw_xmlChar *content) {
    if (cur == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlNodeSetContent : node == NULL\n");
#endif
	return;
    }
    switch (cur->type) {
        case XML_DOCUMENT_FRAG_NODE:
        case XML_ELEMENT_NODE:
        case XML_ATTRIBUTE_NODE:
	    if (cur->children != NULL) hw_xmlFreeNodeList(cur->children);
	    cur->children = hw_xmlStringGetNodeList(cur->doc, content);
	    UPDATE_LAST_CHILD_AND_PARENT(cur)
	    break;
        case XML_TEXT_NODE:
        case XML_CDATA_SECTION_NODE:
        case XML_ENTITY_REF_NODE:
        case XML_ENTITY_NODE:
        case XML_PI_NODE:
        case XML_COMMENT_NODE:
	    if ((cur->content != NULL) &&
	        (cur->content != (hw_xmlChar *) &(cur->properties))) {
	        if (!((cur->doc != NULL) && (cur->doc->dict != NULL) &&
		    (hw_xmlDictOwns(cur->doc->dict, cur->content))))
		    hw_xmlFree(cur->content);
	    }	
	    if (cur->children != NULL) hw_xmlFreeNodeList(cur->children);
	    cur->last = cur->children = NULL;
	    if (content != NULL) {
		cur->content = hw_xmlStrdup(content);
	    } else 
		cur->content = NULL;
	    cur->properties = NULL;
	    cur->nsDef = NULL;
	    break;
        case XML_DOCUMENT_NODE:
        case XML_HTML_DOCUMENT_NODE:
        case XML_DOCUMENT_TYPE_NODE:
	case XML_XINCLUDE_START:
	case XML_XINCLUDE_END:
	    break;
        case XML_NOTATION_NODE:
	    break;
        case XML_DTD_NODE:
	    break;
	case XML_NAMESPACE_DECL:
	    break;
        case XML_ELEMENT_DECL:
	    /* TODO !!! */
	    break;
        case XML_ATTRIBUTE_DECL:
	    /* TODO !!! */
	    break;
        case XML_ENTITY_DECL:
	    /* TODO !!! */
	    break;
    }
}


/**
 * hw_xmlNodeAddContentLen:
 * @cur:  the node being modified
 * @content:  extra content
 * @len:  the size of @content
 * 
 * Append the extra substring to the node content.
 */
void
hw_xmlNodeAddContentLen(hw_xmlNodePtr cur, const hw_xmlChar *content, int len) {
    if (cur == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlNodeAddContentLen : node == NULL\n");
#endif
	return;
    }
    if (len <= 0) return;
    switch (cur->type) {
        case XML_DOCUMENT_FRAG_NODE:
        case XML_ELEMENT_NODE: {
	    hw_xmlNodePtr last, newNode, tmp;

	    last = cur->last;
	    newNode = hw_xmlNewTextLen(content, len);
	    if (newNode != NULL) {
		tmp = hw_xmlAddChild(cur, newNode);
		if (tmp != newNode)
		    return;
	        if ((last != NULL) && (last->next == newNode)) {
		    hw_xmlTextMerge(last, newNode);
		}
	    }
	    break;
	}
        case XML_ATTRIBUTE_NODE:
	    break;
        case XML_TEXT_NODE:
        case XML_CDATA_SECTION_NODE:
        case XML_ENTITY_REF_NODE:
        case XML_ENTITY_NODE:
        case XML_PI_NODE:
        case XML_COMMENT_NODE:
        case XML_NOTATION_NODE:
	    if (content != NULL) {
	        if ((cur->content == (hw_xmlChar *) &(cur->properties)) ||
		    ((cur->doc != NULL) && (cur->doc->dict != NULL) &&
			    hw_xmlDictOwns(cur->doc->dict, cur->content))) {
		    cur->content = hw_xmlStrncatNew(cur->content, content, len);
		    cur->properties = NULL;
		    cur->nsDef = NULL;
		    break;
		}
		cur->content = hw_xmlStrncat(cur->content, content, len);
            }
        case XML_DOCUMENT_NODE:
        case XML_DTD_NODE:
        case XML_HTML_DOCUMENT_NODE:
        case XML_DOCUMENT_TYPE_NODE:
	case XML_NAMESPACE_DECL:
	case XML_XINCLUDE_START:
	case XML_XINCLUDE_END:
	    break;
        case XML_ELEMENT_DECL:
        case XML_ATTRIBUTE_DECL:
        case XML_ENTITY_DECL:
	    break;
    }
}

/**
 * hw_xmlNodeAddContent:
 * @cur:  the node being modified
 * @content:  extra content
 * 
 * Append the extra substring to the node content.
 */
void
hw_xmlNodeAddContent(hw_xmlNodePtr cur, const hw_xmlChar *content) {
    int len;

    if (cur == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlNodeAddContent : node == NULL\n");
#endif
	return;
    }
    if (content == NULL) return;
    len = hw_xmlStrlen(content);
    hw_xmlNodeAddContentLen(cur, content, len);
}

/**
 * hw_xmlTextMerge:
 * @first:  the first text node
 * @second:  the second text node being merged
 * 
 * Merge two text nodes into one
 * Returns the first text node augmented
 */
hw_xmlNodePtr
hw_xmlTextMerge(hw_xmlNodePtr first, hw_xmlNodePtr second) {
    if (first == NULL) return(second);
    if (second == NULL) return(first);
    if (first->type != XML_TEXT_NODE) return(first);
    if (second->type != XML_TEXT_NODE) return(first);
    if (second->name != first->name)
	return(first);
    hw_xmlNodeAddContent(first, second->content);
    hw_xmlUnlinkNode(second);
    hw_xmlFreeNode(second);
    return(first);
}


/**
 * hw_xmlSearchNs:
 * @doc:  the document
 * @node:  the current node
 * @nameSpace:  the namespace prefix
 *
 * Search a Ns registered under a given name space for a document.
 * recurse on the parents until it finds the defined namespace
 * or return NULL otherwise.
 * @nameSpace can be NULL, this is a search for the default namespace.
 * We don't allow to cross entities boundaries. If you don't declare
 * the namespace within those you will be in troubles !!! A warning
 * is generated to cover this case.
 *
 * Returns the namespace pointer or NULL.
 */
hw_xmlNsPtr
hw_xmlSearchNs(hw_xmlDocPtr doc, hw_xmlNodePtr node, const hw_xmlChar *nameSpace) {
	
    hw_xmlNsPtr cur;
    hw_xmlNodePtr orig = node;

    if (node == NULL) return(NULL);
    if ((nameSpace != NULL) &&
	(hw_xmlStrEqual(nameSpace, (const hw_xmlChar *)"xml"))) {
	if ((doc == NULL) && (node->type == XML_ELEMENT_NODE)) {
	    /*
	     * The XML-1.0 namespace is normally held on the root
	     * element. In this case exceptionally create it on the
	     * node element.
	     */
	    cur = (hw_xmlNsPtr) hw_xmlMalloc(sizeof(hw_xmlNs));
	    if (cur == NULL) {
		xmlTreeErrMemory("searching namespace");
		return(NULL);
	    }
	    memset(cur, 0, sizeof(hw_xmlNs));
	    cur->type = hw_XML_LOCAL_NAMESPACE;
	    cur->href = hw_xmlStrdup(hw_XML_XML_NAMESPACE); 
	    cur->prefix = hw_xmlStrdup((const hw_xmlChar *)"xml"); 
	    cur->next = node->nsDef;
	    node->nsDef = cur;
	    return(cur);
	}
	if (doc == NULL) {
	    doc = node->doc;
	    if (doc == NULL)
		return(NULL);
	}
	if (doc->oldNs == NULL) {
	    /*
	     * Allocate a new Namespace and fill the fields.
	     */
	    doc->oldNs = (hw_xmlNsPtr) hw_xmlMalloc(sizeof(hw_xmlNs));
	    if (doc->oldNs == NULL) {
		xmlTreeErrMemory("searching namespace");
		return(NULL);
	    }
	    memset(doc->oldNs, 0, sizeof(hw_xmlNs));
	    doc->oldNs->type = hw_XML_LOCAL_NAMESPACE;

	    doc->oldNs->href = hw_xmlStrdup(hw_XML_XML_NAMESPACE); 
	    doc->oldNs->prefix = hw_xmlStrdup((const hw_xmlChar *)"xml"); 
	}
	return(doc->oldNs);
    }
    while (node != NULL) {
	if ((node->type == XML_ENTITY_REF_NODE) ||
	    (node->type == XML_ENTITY_NODE) ||
	    (node->type == XML_ENTITY_DECL))
	    return(NULL);
	if (node->type == XML_ELEMENT_NODE) {
	    cur = node->nsDef;
	    while (cur != NULL) {
		if ((cur->prefix == NULL) && (nameSpace == NULL) &&
		    (cur->href != NULL))
		    return(cur);
		if ((cur->prefix != NULL) && (nameSpace != NULL) &&
		    (cur->href != NULL) &&
		    (hw_xmlStrEqual(cur->prefix, nameSpace)))
		    return(cur);
		cur = cur->next;
	    }
	    if (orig != node) { 
	        cur = node->ns;
	        if (cur != NULL) {
		    if ((cur->prefix == NULL) && (nameSpace == NULL) &&
		        (cur->href != NULL))
		        return(cur);
		    if ((cur->prefix != NULL) && (nameSpace != NULL) &&
		        (cur->href != NULL) &&
		        (hw_xmlStrEqual(cur->prefix, nameSpace)))
		        return(cur);
	        }
	    }    
	}
	node = node->parent;
    }
    return(NULL);
}

/**
 * xmlNsInScope:
 * @doc:  the document
 * @node:  the current node
 * @ancestor:  the ancestor carrying the namespace
 * @prefix:  the namespace prefix
 *
 * Verify that the given namespace held on @ancestor is still in scope
 * on node.
 * 
 * Returns 1 if true, 0 if false and -1 in case of error.
 */
static int
xmlNsInScope(hw_xmlDocPtr doc ATTRIBUTE_UNUSED, hw_xmlNodePtr node,
             hw_xmlNodePtr ancestor, const hw_xmlChar * prefix)
{
    hw_xmlNsPtr tst;

    while ((node != NULL) && (node != ancestor)) {
        if ((node->type == XML_ENTITY_REF_NODE) ||
            (node->type == XML_ENTITY_NODE) ||
            (node->type == XML_ENTITY_DECL))
            return (-1);
        if (node->type == XML_ELEMENT_NODE) {
            tst = node->nsDef;
            while (tst != NULL) {
                if ((tst->prefix == NULL)
                    && (prefix == NULL))
                    return (0);
                if ((tst->prefix != NULL)
                    && (prefix != NULL)
                    && (hw_xmlStrEqual(tst->prefix, prefix)))
                    return (0);
                tst = tst->next;
            }
        }
        node = node->parent;
    }
    if (node != ancestor)
        return (-1);
    return (1);
}
                  
/**
 * hw_xmlSearchNsByHref:
 * @doc:  the document
 * @node:  the current node
 * @href:  the namespace value
 *
 * Search a Ns aliasing a given URI. Recurse on the parents until it finds
 * the defined namespace or return NULL otherwise.
 * Returns the namespace pointer or NULL.
 */
hw_xmlNsPtr
hw_xmlSearchNsByHref(hw_xmlDocPtr doc, hw_xmlNodePtr node, const hw_xmlChar * href)
{
    hw_xmlNsPtr cur;
    hw_xmlNodePtr orig = node;
    int is_attr;

    if ((node == NULL) || (href == NULL))
        return (NULL);
    if (hw_xmlStrEqual(href, hw_XML_XML_NAMESPACE)) {
        /*
         * Only the document can hold the XML spec namespace.
         */
        if ((doc == NULL) && (node->type == XML_ELEMENT_NODE)) {
            /*
             * The XML-1.0 namespace is normally held on the root
             * element. In this case exceptionally create it on the
             * node element.
             */
            cur = (hw_xmlNsPtr) hw_xmlMalloc(sizeof(hw_xmlNs));
            if (cur == NULL) {
		xmlTreeErrMemory("searching namespace");
                return (NULL);
            }
            memset(cur, 0, sizeof(hw_xmlNs));
            cur->type = hw_XML_LOCAL_NAMESPACE;
            cur->href = hw_xmlStrdup(hw_XML_XML_NAMESPACE);
            cur->prefix = hw_xmlStrdup((const hw_xmlChar *) "xml");
            cur->next = node->nsDef;
            node->nsDef = cur;
            return (cur);
        }
	if (doc == NULL) {
	    doc = node->doc;
	    if (doc == NULL)
		return(NULL);
	}
        if (doc->oldNs == NULL) {
            /*
             * Allocate a new Namespace and fill the fields.
             */
            doc->oldNs = (hw_xmlNsPtr) hw_xmlMalloc(sizeof(hw_xmlNs));
            if (doc->oldNs == NULL) {
		xmlTreeErrMemory("searching namespace");
                return (NULL);
            }
            memset(doc->oldNs, 0, sizeof(hw_xmlNs));
            doc->oldNs->type = hw_XML_LOCAL_NAMESPACE;

            doc->oldNs->href = hw_xmlStrdup(hw_XML_XML_NAMESPACE);
            doc->oldNs->prefix = hw_xmlStrdup((const hw_xmlChar *) "xml");
        }
        return (doc->oldNs);
    }
    is_attr = (node->type == XML_ATTRIBUTE_NODE);
    while (node != NULL) {
        if ((node->type == XML_ENTITY_REF_NODE) ||
            (node->type == XML_ENTITY_NODE) ||
            (node->type == XML_ENTITY_DECL))
            return (NULL);
        if (node->type == XML_ELEMENT_NODE) {
            cur = node->nsDef;
            while (cur != NULL) {
                if ((cur->href != NULL) && (href != NULL) &&
                    (hw_xmlStrEqual(cur->href, href))) {
		    if (((!is_attr) || (cur->prefix != NULL)) &&
		        (xmlNsInScope(doc, orig, node, cur->prefix) == 1))
			return (cur);
                }
                cur = cur->next;
            }
            if (orig != node) {
                cur = node->ns;
                if (cur != NULL) {
                    if ((cur->href != NULL) && (href != NULL) &&
                        (hw_xmlStrEqual(cur->href, href))) {
			if (((!is_attr) || (cur->prefix != NULL)) &&
		            (xmlNsInScope(doc, orig, node, cur->prefix) == 1))
			    return (cur);
                    }
                }
            }    
        }
        node = node->parent;
    }
    return (NULL);
}

/**
 * xmlNewReconciliedNs:
 * @doc:  the document
 * @tree:  a node expected to hold the new namespace
 * @ns:  the original namespace
 *
 * This function tries to locate a namespace definition in a tree
 * ancestors, or create a new namespace definition node similar to
 * @ns trying to reuse the same prefix. However if the given prefix is
 * null (default namespace) or reused within the subtree defined by
 * @tree or on one of its ancestors then a new prefix is generated.
 * Returns the (new) namespace definition or NULL in case of error
 */
hw_xmlNsPtr
xmlNewReconciliedNs(hw_xmlDocPtr doc, hw_xmlNodePtr tree, hw_xmlNsPtr ns) {
    hw_xmlNsPtr def;
    hw_xmlChar prefix[50];
    int counter = 1;

    if (tree == NULL) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"xmlNewReconciliedNs : tree == NULL\n");
#endif
	return(NULL);
    }
    if ((ns == NULL) || (ns->type != XML_NAMESPACE_DECL)) {
#ifdef DEBUG_TREE
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"xmlNewReconciliedNs : ns == NULL\n");
#endif
	return(NULL);
    }
    /*
     * Search an existing namespace definition inherited.
     */
    def = hw_xmlSearchNsByHref(doc, tree, ns->href);
    if (def != NULL)
        return(def);

    /*
     * Find a close prefix which is not already in use.
     * Let's strip namespace prefixes longer than 20 chars !
     */
    if (ns->prefix == NULL)
	snprintf((char *) prefix, sizeof(prefix), "default");
    else
	snprintf((char *) prefix, sizeof(prefix), "%.20s", (char *)ns->prefix);

    def = hw_xmlSearchNs(doc, tree, prefix);
    while (def != NULL) {
        if (counter > 1000) return(NULL);
	if (ns->prefix == NULL)
	    snprintf((char *) prefix, sizeof(prefix), "default%d", counter++);
	else
	    snprintf((char *) prefix, sizeof(prefix), "%.20s%d",
	    	(char *)ns->prefix, counter++);
	def = hw_xmlSearchNs(doc, tree, prefix);
    }

    /*
     * OK, now we are ready to create a new one.
     */
    def = hw_xmlNewNs(tree, ns->href, prefix);
    return(def);
}

static hw_xmlAttrPtr
xmlGetPropNodeInternal(hw_xmlNodePtr node, const hw_xmlChar *name,
		       const hw_xmlChar *nsName, int useDTD)
{
    hw_xmlAttrPtr prop;

    if ((node == NULL) || (node->type != XML_ELEMENT_NODE) || (name == NULL))
	return(NULL);

    if (node->properties != NULL) {
	prop = node->properties;
	if (nsName == NULL) {
	    /*
	    * We want the attr to be in no namespace.
	    */
	    do {
		if ((prop->ns == NULL) && hw_xmlStrEqual(prop->name, name)) {
		    return(prop);
		}
		prop = prop->next;
	    } while (prop != NULL);
	} else {
	    /*
	    * We want the attr to be in the specified namespace.
	    */
	    do {
		if ((prop->ns != NULL) && hw_xmlStrEqual(prop->name, name) &&
		    ((prop->ns->href == nsName) ||
		     hw_xmlStrEqual(prop->ns->href, nsName)))
		{
		    return(prop);
		}
		prop = prop->next;
	    } while (prop != NULL);
	}
    }

    return(NULL);
}

static hw_xmlChar*
xmlGetPropNodeValueInternal(hw_xmlAttrPtr prop)
{
    if (prop == NULL)
	return(NULL);
    if (prop->type == XML_ATTRIBUTE_NODE) {
	/*
	* Note that we return at least the empty string.
	*   TODO: Do we really always want that?
	*/
	if (prop->children != NULL) {
	    if ((prop->children == prop->last) &&
		((prop->children->type == XML_TEXT_NODE) ||
		(prop->children->type == XML_CDATA_SECTION_NODE)))
	    {
		/*
		* Optimization for the common case: only 1 text node.
		*/
		return(hw_xmlStrdup(prop->children->content));
	    } else {
		hw_xmlChar *ret;

		ret = hw_xmlNodeListGetString(prop->doc, prop->children, 1);
		if (ret != NULL)
		    return(ret);
	    }
	}
	return(hw_xmlStrdup((hw_xmlChar *)""));
    } else if (prop->type == XML_ATTRIBUTE_DECL) {
	return(hw_xmlStrdup(((hw_xmlAttributePtr)prop)->defaultValue));
    }
    return(NULL); 
}

/**
 * hw_xmlHasProp:
 * @node:  the node
 * @name:  the attribute name
 *
 * Search an attribute associated to a node
 * This function also looks in DTD attribute declaration for #FIXED or
 * default declaration values unless DTD use has been turned off.
 *
 * Returns the attribute or the attribute declaration or NULL if 
 *         neither was found.
 */
hw_xmlAttrPtr
hw_xmlHasProp(hw_xmlNodePtr node, const hw_xmlChar *name) {
    hw_xmlAttrPtr prop;
    hw_xmlDocPtr doc;

    if ((node == NULL) || (node->type != XML_ELEMENT_NODE) || (name == NULL))
        return(NULL);
    /*
     * Check on the properties attached to the node
     */
    prop = node->properties;
    while (prop != NULL) {
        if (hw_xmlStrEqual(prop->name, name))  {
	    return(prop);
        }
	prop = prop->next;
    }
    if (!xmlCheckDTD) return(NULL);

    /*
     * Check if there is a default declaration in the internal
     * or external subsets
     */
    doc =  node->doc;
    if (doc != NULL) {
        hw_xmlAttributePtr attrDecl;
        if (doc->intSubset != NULL) {
	    attrDecl = hw_xmlGetDtdAttrDesc(doc->intSubset, node->name, name);
	    if ((attrDecl == NULL) && (doc->extSubset != NULL))
		attrDecl = hw_xmlGetDtdAttrDesc(doc->extSubset, node->name, name);
            if ((attrDecl != NULL) && (attrDecl->defaultValue != NULL))
              /* return attribute declaration only if a default value is given
                 (that includes #FIXED declarations) */
		return((hw_xmlAttrPtr) attrDecl);
	}
    }
    return(NULL);
}

/**
 * hw_xmlHasNsProp:
 * @node:  the node
 * @name:  the attribute name
 * @nameSpace:  the URI of the namespace
 *
 * Search for an attribute associated to a node
 * This attribute has to be anchored in the namespace specified.
 * This does the entity substitution.
 * This function looks in DTD attribute declaration for #FIXED or
 * default declaration values unless DTD use has been turned off.
 * Note that a namespace of NULL indicates to use the default namespace.
 *
 * Returns the attribute or the attribute declaration or NULL
 *     if neither was found.
 */
hw_xmlAttrPtr
hw_xmlHasNsProp(hw_xmlNodePtr node, const hw_xmlChar *name, const hw_xmlChar *nameSpace) {

    return(xmlGetPropNodeInternal(node, name, nameSpace, xmlCheckDTD));
}

/**
 * hw_xmlGetProp:
 * @node:  the node
 * @name:  the attribute name
 *
 * Search and get the value of an attribute associated to a node
 * This does the entity substitution.
 * This function looks in DTD attribute declaration for #FIXED or
 * default declaration values unless DTD use has been turned off.
 * NOTE: this function acts independently of namespaces associated
 *       to the attribute. Use hw_xmlGetNsProp() or hw_xmlGetNoNsProp()
 *       for namespace aware processing.
 *
 * Returns the attribute value or NULL if not found.
 *     It's up to the caller to free the memory with hw_xmlFree().
 */
hw_xmlChar *
hw_xmlGetProp(hw_xmlNodePtr node, const hw_xmlChar *name) {
    hw_xmlAttrPtr prop;    

    prop = hw_xmlHasProp(node, name);
    if (prop == NULL)
	return(NULL);
    return(xmlGetPropNodeValueInternal(prop));     
}

/**
 * hw_xmlGetNoNsProp:
 * @node:  the node
 * @name:  the attribute name
 *
 * Search and get the value of an attribute associated to a node
 * This does the entity substitution.
 * This function looks in DTD attribute declaration for #FIXED or
 * default declaration values unless DTD use has been turned off.
 * This function is similar to hw_xmlGetProp except it will accept only
 * an attribute in no namespace.
 *
 * Returns the attribute value or NULL if not found.
 *     It's up to the caller to free the memory with hw_xmlFree().
 */
hw_xmlChar *
hw_xmlGetNoNsProp(hw_xmlNodePtr node, const hw_xmlChar *name) {
    hw_xmlAttrPtr prop;
    
    prop = xmlGetPropNodeInternal(node, name, NULL, xmlCheckDTD);
    if (prop == NULL)
	return(NULL);
    return(xmlGetPropNodeValueInternal(prop));
}

/**
 * hw_xmlGetNsProp:
 * @node:  the node
 * @name:  the attribute name
 * @nameSpace:  the URI of the namespace
 *
 * Search and get the value of an attribute associated to a node
 * This attribute has to be anchored in the namespace specified.
 * This does the entity substitution.
 * This function looks in DTD attribute declaration for #FIXED or
 * default declaration values unless DTD use has been turned off.
 *
 * Returns the attribute value or NULL if not found.
 *     It's up to the caller to free the memory with hw_xmlFree().
 */
hw_xmlChar *
hw_xmlGetNsProp(hw_xmlNodePtr node, const hw_xmlChar *name, const hw_xmlChar *nameSpace) {
    hw_xmlAttrPtr prop;

    prop = xmlGetPropNodeInternal(node, name, nameSpace, xmlCheckDTD);
    if (prop == NULL)
	return(NULL);
    return(xmlGetPropNodeValueInternal(prop));
}

/**
 * hw_xmlNodeIsText:
 * @node:  the node
 * 
 * Is this node a Text node ?
 * Returns 1 yes, 0 no
 */
int
hw_xmlNodeIsText(hw_xmlNodePtr node) {
    if (node == NULL) return(0);

    if (node->type == XML_TEXT_NODE) return(1);
    return(0);
}

/**
 * hw_xmlIsBlankNode:
 * @node:  the node
 * 
 * Checks whether this node is an empty or whitespace only
 * (and possibly ignorable) text-node.
 *
 * Returns 1 yes, 0 no
 */
int
hw_xmlIsBlankNode(hw_xmlNodePtr node) {
    const hw_xmlChar *cur;
    if (node == NULL) return(0);

    if ((node->type != XML_TEXT_NODE) &&
        (node->type != XML_CDATA_SECTION_NODE))
	return(0);
    if (node->content == NULL) return(1);
    cur = node->content;
    while (*cur != 0) {
	if (!hw_IS_BLANK_CH(*cur)) return(0);
	cur++;
    }

    return(1);
}

/**
 * hw_xmlTextConcat:
 * @node:  the node
 * @content:  the content
 * @len:  @content length
 * 
 * Concat the given string at the end of the existing node content
 *
 * Returns -1 in case of error, 0 otherwise
 */

int
hw_xmlTextConcat(hw_xmlNodePtr node, const hw_xmlChar *content, int len) {
    if (node == NULL) return(-1);

    if ((node->type != XML_TEXT_NODE) &&
        (node->type != XML_CDATA_SECTION_NODE)) {
#ifdef DEBUG_TREE
	hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlTextConcat: node is not text nor CDATA\n");
#endif
        return(-1);
    }
    /* need to check if content is currently in the dictionary */
    if ((node->content == (hw_xmlChar *) &(node->properties)) ||
        ((node->doc != NULL) && (node->doc->dict != NULL) &&
		hw_xmlDictOwns(node->doc->dict, node->content))) {
	node->content = hw_xmlStrncatNew(node->content, content, len);
    } else {
        node->content = hw_xmlStrncat(node->content, content, len);
    }
    node->properties = NULL;
    if (node->content == NULL)
        return(-1);
    return(0);
}

/************************************************************************
 *									*
 *			Output : to a FILE or in memory			*
 *									*
 ************************************************************************/

/**
 * hw_xmlBufferCreate:
 *
 * routine to create an XML buffer.
 * returns the new structure.
 */
hw_xmlBufferPtr
hw_xmlBufferCreate(void) {
    hw_xmlBufferPtr ret;

    ret = (hw_xmlBufferPtr) hw_xmlMalloc(sizeof(hw_xmlBuffer));
    if (ret == NULL) {
	xmlTreeErrMemory("creating buffer");
        return(NULL);
    }
    ret->use = 0;
    ret->size = hw_xmlDefaultBufferSize;
    ret->alloc = hw_xmlBufferAllocScheme;
    ret->content = (hw_xmlChar *) hw_xmlMallocAtomic(ret->size * sizeof(hw_xmlChar));
    if (ret->content == NULL) {
	xmlTreeErrMemory("creating buffer");
	hw_xmlFree(ret);
        return(NULL);
    }
    ret->content[0] = 0;
    return(ret);
}

/**
 * hw_xmlBufferCreateSize:
 * @size: initial size of buffer
 *
 * routine to create an XML buffer.
 * returns the new structure.
 */
hw_xmlBufferPtr
hw_xmlBufferCreateSize(size_t size) {
    hw_xmlBufferPtr ret;

    ret = (hw_xmlBufferPtr) hw_xmlMalloc(sizeof(hw_xmlBuffer));
    if (ret == NULL) {
	xmlTreeErrMemory("creating buffer");
        return(NULL);
    }
    ret->use = 0;
    ret->alloc = hw_xmlBufferAllocScheme;
    ret->size = (size ? size+2 : 0);         /* +1 for ending null */
    if (ret->size){
        ret->content = (hw_xmlChar *) hw_xmlMallocAtomic(ret->size * sizeof(hw_xmlChar));
        if (ret->content == NULL) {
	    xmlTreeErrMemory("creating buffer");
            hw_xmlFree(ret);
            return(NULL);
        }
        ret->content[0] = 0;
    } else
	ret->content = NULL;
    return(ret);
}

/**
 * hw_xmlBufferCreateStatic:
 * @mem: the memory area
 * @size:  the size in byte
 *
 * routine to create an XML buffer from an immutable memory area.
 * The area won't be modified nor copied, and is expected to be
 * present until the end of the buffer lifetime.
 *
 * returns the new structure.
 */
hw_xmlBufferPtr
hw_xmlBufferCreateStatic(void *mem, size_t size) {
    hw_xmlBufferPtr ret;

    if ((mem == NULL) || (size == 0))
        return(NULL);

    ret = (hw_xmlBufferPtr) hw_xmlMalloc(sizeof(hw_xmlBuffer));
    if (ret == NULL) {
	xmlTreeErrMemory("creating buffer");
        return(NULL);
    }
    ret->use = size;
    ret->size = size;
    ret->alloc = XML_BUFFER_ALLOC_IMMUTABLE;
    ret->content = (hw_xmlChar *) mem;
    return(ret);
}

/**
 * hw_xmlBufferSetAllocationScheme:
 * @buf:  the buffer to tune
 * @scheme:  allocation scheme to use
 *
 * Sets the allocation scheme for this buffer
 */
void
hw_xmlBufferSetAllocationScheme(hw_xmlBufferPtr buf, 
                             hw_xmlBufferAllocationScheme scheme) {
    if (buf == NULL) {
#ifdef DEBUG_BUFFER
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlBufferSetAllocationScheme: buf == NULL\n");
#endif
        return;
    }
    if (buf->alloc == XML_BUFFER_ALLOC_IMMUTABLE) return;

    buf->alloc = scheme;
}

/**
 * hw_xmlBufferFree:
 * @buf:  the buffer to free
 *
 * Frees an XML buffer. It frees both the content and the structure which
 * encapsulate it.
 */
void
hw_xmlBufferFree(hw_xmlBufferPtr buf) {
    if (buf == NULL) {
#ifdef DEBUG_BUFFER
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlBufferFree: buf == NULL\n");
#endif
	return;
    }

    if ((buf->content != NULL) &&
        (buf->alloc != XML_BUFFER_ALLOC_IMMUTABLE)) {
        hw_xmlFree(buf->content);
    }
    hw_xmlFree(buf);
}

/**
 * hw_xmlBufferEmpty:
 * @buf:  the buffer
 *
 * empty a buffer.
 */
void
hw_xmlBufferEmpty(hw_xmlBufferPtr buf) {
    if (buf == NULL) return;
    if (buf->content == NULL) return;
    buf->use = 0;
    if (buf->alloc == XML_BUFFER_ALLOC_IMMUTABLE) {
        buf->content = hw_BAD_CAST "";
    } else {
	memset(buf->content, 0, buf->size);
    }
}

/**
 * hw_xmlBufferShrink:
 * @buf:  the buffer to dump
 * @len:  the number of hw_xmlChar to remove
 *
 * Remove the beginning of an XML buffer.
 *
 * Returns the number of #hw_xmlChar removed, or -1 in case of failure.
 */
int
hw_xmlBufferShrink(hw_xmlBufferPtr buf, unsigned int len) {
    if (buf == NULL) return(-1);
    if (len == 0) return(0);
    if (len > buf->use) return(-1);

    buf->use -= len;
    if (buf->alloc == XML_BUFFER_ALLOC_IMMUTABLE) {
        buf->content += len;
    } else {
	memmove(buf->content, &buf->content[len], buf->use * sizeof(hw_xmlChar));
	buf->content[buf->use] = 0;
    }
    return(len);
}

/**
 * hw_xmlBufferGrow:
 * @buf:  the buffer
 * @len:  the minimum free size to allocate
 *
 * Grow the available space of an XML buffer.
 *
 * Returns the new available space or -1 in case of error
 */
int
hw_xmlBufferGrow(hw_xmlBufferPtr buf, unsigned int len) {
    int size;
    hw_xmlChar *newbuf;

    if (buf == NULL) return(-1);

    if (buf->alloc == XML_BUFFER_ALLOC_IMMUTABLE) return(0);
    if (len + buf->use < buf->size) return(0);

/*
 * Windows has a BIG problem on realloc timing, so we try to double
 * the buffer size (if that's enough) (bug 146697)
 */
#ifdef WIN32
    if (buf->size > len)
        size = buf->size * 2;
    else
        size = buf->use + len + 100;
#else
    size = buf->use + len + 100;
#endif

    newbuf = (hw_xmlChar *) hw_xmlRealloc(buf->content, size);
    if (newbuf == NULL) {
	xmlTreeErrMemory("growing buffer");
        return(-1);
    }
    buf->content = newbuf;
    buf->size = size;
    return(buf->size - buf->use);
}

/**
 * hw_xmlBufferLength:
 * @buf:  the buffer 
 *
 * Function to get the length of a buffer
 *
 * Returns the length of data in the internal content
 */

int
hw_xmlBufferLength(const hw_xmlBufferPtr buf)
{
    if(!buf)
        return 0;

    return buf->use;
}

/**
 * hw_xmlBufferResize:
 * @buf:  the buffer to resize
 * @size:  the desired size
 *
 * Resize a buffer to accommodate minimum size of @size.
 *
 * Returns  0 in case of problems, 1 otherwise
 */
int
hw_xmlBufferResize(hw_xmlBufferPtr buf, unsigned int size)
{
    unsigned int newSize;
    hw_xmlChar* rebuf = NULL;

    if (buf == NULL)
        return(0);

    if (buf->alloc == XML_BUFFER_ALLOC_IMMUTABLE) return(0);

    /* Don't resize if we don't have to */
    if (size < buf->size)
        return 1;

    /* figure out new size */
    switch (buf->alloc){
    case XML_BUFFER_ALLOC_DOUBLEIT:
	/*take care of empty case*/
        newSize = (buf->size ? buf->size*2 : size + 10);
        while (size > newSize) newSize *= 2;
        break;
    case XML_BUFFER_ALLOC_EXACT:
        newSize = size+10;
        break;
    default:
        newSize = size+10;
        break;
    }

    if (buf->content == NULL)
	rebuf = (hw_xmlChar *) hw_xmlMallocAtomic(newSize * sizeof(hw_xmlChar));
    else if (buf->size - buf->use < 100) {
	rebuf = (hw_xmlChar *) hw_xmlRealloc(buf->content, 
				       newSize * sizeof(hw_xmlChar));
   } else {
        /*
	 * if we are reallocating a buffer far from being full, it's
	 * better to make a new allocation and copy only the used range
	 * and free the old one.
	 */
	rebuf = (hw_xmlChar *) hw_xmlMallocAtomic(newSize * sizeof(hw_xmlChar));
	if (rebuf != NULL) {
	    memcpy(rebuf, buf->content, buf->use);
	    hw_xmlFree(buf->content);
	    rebuf[buf->use] = 0;
	}
    }
    if (rebuf == NULL) {
	xmlTreeErrMemory("growing buffer");
        return 0;
    }
    buf->content = rebuf;
    buf->size = newSize;

    return 1;
}

/**
 * hw_xmlBufferAdd:
 * @buf:  the buffer to dump
 * @str:  the #hw_xmlChar string
 * @len:  the number of #hw_xmlChar to add
 *
 * Add a string range to an XML buffer. if len == -1, the length of
 * str is recomputed.
 *
 * Returns 0 successful, a positive error code number otherwise
 *         and -1 in case of internal or API error.
 */
int
hw_xmlBufferAdd(hw_xmlBufferPtr buf, const hw_xmlChar *str, int len) {
    unsigned int needSize;

    if ((str == NULL) || (buf == NULL)) {
	return -1;
    }
    if (buf->alloc == XML_BUFFER_ALLOC_IMMUTABLE) return -1;
    if (len < -1) {
#ifdef DEBUG_BUFFER
        hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlBufferAdd: len < 0\n");
#endif
	return -1;
    }
    if (len == 0) return 0;

    if (len < 0)
        len = hw_xmlStrlen(str);

    if (len <= 0) return -1;

    needSize = buf->use + len + 2;
    if (needSize > buf->size){
        if (!hw_xmlBufferResize(buf, needSize)){
	    xmlTreeErrMemory("growing buffer");
            return XML_ERR_NO_MEMORY;
        }
    }

    memmove(&buf->content[buf->use], str, len*sizeof(hw_xmlChar));
    buf->use += len;
    buf->content[buf->use] = 0;
    return 0;
}


#if 1

/**
 * hw_xmlBufferAllocScheme:
 *
 * Global setting, default allocation policy for buffers, default is
 * XML_BUFFER_ALLOC_EXACT
 */
hw_xmlBufferAllocationScheme hw_xmlBufferAllocScheme = XML_BUFFER_ALLOC_EXACT;
/**
 * hw_xmlDefaultBufferSize:
 *
 * Global setting, default buffer size. Default value is hw_BASE_BUFFER_SIZE
 */
int hw_xmlDefaultBufferSize = hw_BASE_BUFFER_SIZE;


/*
 * Error handling
 */

/* hw_xmlGenericErrorFunc hw_xmlGenericError = xmlGenericErrorDefaultFunc; */
/* Must initialize hw_xmlGenericError in hw_xmlInitParser */
void hw_XMLCDECL xmlGenericErrorDefaultFunc	(void *ctx ATTRIBUTE_UNUSED,
				 const char *msg,
				 ...);
/**
 * hw_xmlGenericError:
 *
 * Global setting: function used for generic error callbacks
 */
hw_xmlGenericErrorFunc hw_xmlGenericError = xmlGenericErrorDefaultFunc;
/**
 * hw_xmlStructuredError:
 *
 * Global setting: function used for structured error callbacks
 */
hw_xmlStructuredErrorFunc hw_xmlStructuredError = NULL;
/**
 * hw_xmlGenericErrorContext:
 *
 * Global setting passed to generic error callbacks
 */
void *hw_xmlGenericErrorContext = NULL;
hw_xmlError hw_xmlLastError;
#endif

