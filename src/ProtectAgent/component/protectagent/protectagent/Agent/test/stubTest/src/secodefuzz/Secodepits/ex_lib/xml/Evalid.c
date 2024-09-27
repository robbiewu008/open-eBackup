/*
 * valid.c : part of the code use to do the DTD handling and the validity
 *           checking
 *
 * See Copyright for the status of this software.
 *
 * daniel@veillard.com
 */

#define IN_LIBXML



#include "Eparser.h"
#include "Etree.h"

/* #define DEBUG_VALID_ALGO */
/* #define DEBUG_REGEXP_ALGO */

#define TODO 								\
    hw_xmlGenericError(hw_xmlGenericErrorContext,				\
	    "Unimplemented block at %s:%d\n",				\
            __FILE__, __LINE__);

/************************************************************************
 *									*
 *			Error handling routines				*
 *									*
 ************************************************************************/

/**
 * hw_xmlVErrMemory:
 * @ctxt:  an XML validation parser context
 * @extra:  extra informations
 *
 * Handle an out of memory error
 */
static void
hw_xmlVErrMemory(hw_xmlValidCtxtPtr ctxt, const char *extra)
{
    hw_xmlGenericErrorFunc channel = NULL;
    hw_xmlParserCtxtPtr pctxt = NULL;
    void *data = NULL;

    if (ctxt != NULL) {
        channel = ctxt->error;
        data = ctxt->userData;
	/* Use the special values to detect if it is part of a parsing
	   context */
	if ((ctxt->finishDtd == hw_XML_CTXT_FINISH_DTD_0) ||
	    (ctxt->finishDtd == hw_XML_CTXT_FINISH_DTD_1)) {
	    pctxt = ctxt->userData;
	}
    }
    if (extra)
        hw___xmlRaiseError(NULL, channel, data,
                        pctxt, NULL, XML_FROM_VALID, XML_ERR_NO_MEMORY,
                        XML_ERR_FATAL, NULL, 0, extra, NULL, NULL, 0, 0,
                        "Memory allocation failed : %s\n", extra);
    else
        hw___xmlRaiseError(NULL, channel, data,
                        pctxt, NULL, XML_FROM_VALID, XML_ERR_NO_MEMORY,
                        XML_ERR_FATAL, NULL, 0, NULL, NULL, NULL, 0, 0,
                        "Memory allocation failed\n");
}


/************************************************************************
 *									*
 *				IDs					*
 *									*
 ************************************************************************/
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
 * hw_xmlAddID:
 * @ctxt:  the validation context
 * @doc:  pointer to the document
 * @value:  the value name
 * @attr:  the attribute holding the ID
 *
 * Register a new id declaration
 *
 * Returns NULL if not, otherwise the new hw_xmlIDPtr
 */
hw_xmlIDPtr 
hw_xmlAddID(hw_xmlValidCtxtPtr ctxt, hw_xmlDocPtr doc, const hw_xmlChar *value,
         hw_xmlAttrPtr attr) {
    hw_xmlIDPtr ret;
    hw_xmlIDTablePtr table;

    if (doc == NULL) {
	return(NULL);
    }
    if (value == NULL) {
	return(NULL);
    }
    if (attr == NULL) {
	return(NULL);
    }

    /*
     * Create the ID table if needed.
     */
    table = (hw_xmlIDTablePtr) doc->ids;
    if (table == NULL) {
	hw_xmlVErrMemory(ctxt,
		"hw_xmlAddID: Table creation failed!\n");
        return(NULL);
    }

    ret = (hw_xmlIDPtr) hw_xmlMalloc(sizeof(hw_xmlID));
    if (ret == NULL) {
	hw_xmlVErrMemory(ctxt, "malloc failed");
	return(NULL);
    }

    /*
     * fill the structure.
     */
    ret->value = hw_xmlStrdup(value);
    ret->doc = doc;
    if ((ctxt != NULL) && (ctxt->vstateNr != 0)) {
	/*
	 * Operating in streaming mode, attr is gonna disapear
	 */
	if (doc->dict != NULL)
	    ret->name = hw_xmlDictLookup(doc->dict, attr->name, -1);
	else
	    ret->name = hw_xmlStrdup(attr->name);
	ret->attr = NULL;
    } else {
	ret->attr = attr;
	ret->name = NULL;
    }
    ret->lineno = hw_xmlGetLineNo(attr->parent);

    if (attr != NULL)
	attr->atype = XML_ATTRIBUTE_ID;
    return(ret);
}


/**
 * hw_xmlIsID:
 * @doc:  the document
 * @elem:  the element carrying the attribute
 * @attr:  the attribute
 *
 * Determine whether an attribute is of type ID. In case we have DTD(s)
 * then this is done if DTD loading has been requested. In the case
 * of HTML documents parsed with the HTML parser, then ID detection is
 * done systematically.
 *
 * Returns 0 or 1 depending on the lookup result
 */
int
hw_xmlIsID(hw_xmlDocPtr doc, hw_xmlNodePtr elem, hw_xmlAttrPtr attr) {
    if ((attr == NULL) || (attr->name == NULL)) return(0);
    if ((attr->ns != NULL) && (attr->ns->prefix != NULL) &&
        (!strcmp((char *) attr->name, "id")) &&
        (!strcmp((char *) attr->ns->prefix, "xml")))
	return(1);
    if (doc == NULL) return(0);
    if ((doc->intSubset == NULL) && (doc->extSubset == NULL)) {
	return(0);
    } else if (doc->type == XML_HTML_DOCUMENT_NODE) {
        if ((hw_xmlStrEqual(hw_BAD_CAST "id", attr->name)) ||
	    ((hw_xmlStrEqual(hw_BAD_CAST "name", attr->name)) &&
	    ((elem == NULL) || (!hw_xmlStrEqual(elem->name, hw_BAD_CAST "input")))))
	    return(1);
	return(0);    
    } else if (elem == NULL) {
	return(0);
    } else {
	hw_xmlAttributePtr attrDecl = NULL;

	hw_xmlChar felem[50], fattr[50];
	hw_xmlChar *fullelemname, *fullattrname;

	fullelemname = (elem->ns != NULL && elem->ns->prefix != NULL) ?
	    hw_xmlBuildQName(elem->name, elem->ns->prefix, felem, 50) :
	    (hw_xmlChar *)elem->name;

	fullattrname = (attr->ns != NULL && attr->ns->prefix != NULL) ?
	    hw_xmlBuildQName(attr->name, attr->ns->prefix, fattr, 50) :
	    (hw_xmlChar *)attr->name;

	if (fullelemname != NULL && fullattrname != NULL) {
	    attrDecl = hw_xmlGetDtdAttrDesc(doc->intSubset, fullelemname,
		                         fullattrname);
	    if ((attrDecl == NULL) && (doc->extSubset != NULL))
		attrDecl = hw_xmlGetDtdAttrDesc(doc->extSubset, fullelemname,
					     fullattrname);
	}

	if ((fullattrname != fattr) && (fullattrname != attr->name))
	    hw_xmlFree(fullattrname);
	if ((fullelemname != felem) && (fullelemname != elem->name))
	    hw_xmlFree(fullelemname);

        if ((attrDecl != NULL) && (attrDecl->atype == XML_ATTRIBUTE_ID))
	    return(1);
    }
    return(0);
}

/**
 * hw_xmlRemoveID:
 * @doc:  the document
 * @attr:  the attribute
 *
 * Remove the given attribute from the ID table maintained internally.
 *
 * Returns -1 if the lookup failed and 0 otherwise
 */
int
hw_xmlRemoveID(hw_xmlDocPtr doc, hw_xmlAttrPtr attr) {
    hw_xmlIDTablePtr table;
    hw_xmlIDPtr id;
    hw_xmlChar *ID;

    if (doc == NULL) return(-1);
    if (attr == NULL) return(-1);
    table = (hw_xmlIDTablePtr) doc->ids;
    if (table == NULL) 
        return(-1);

    if (attr == NULL)
	return(-1);
    ID = hw_xmlNodeListGetString(doc, attr->children, 1);
    if (ID == NULL)
	return(-1);
    id = 0;
    if (id == NULL || id->attr != attr) {
	hw_xmlFree(ID);
	return(-1);
    }
    hw_xmlFree(ID);
	attr->atype = 0;
    return(0);
}


/************************************************************************
 *									*
 *		Routines for validity checking				*
 *									*
 ************************************************************************/

/**
 * hw_xmlGetDtdElementDesc:
 * @dtd:  a pointer to the DtD to search
 * @name:  the element name
 *
 * Search the DTD for the description of this element
 *
 * returns the hw_xmlElementPtr if found or NULL
 */

hw_xmlElementPtr
hw_xmlGetDtdElementDesc(hw_xmlDtdPtr dtd, const hw_xmlChar *name) {
    //hw_xmlElementTablePtr table;
    hw_xmlElementPtr cur;
    hw_xmlChar *uqname = NULL, *prefix = NULL;

    if ((dtd == NULL) || (name == NULL)) return(NULL);
    if (dtd->elements == NULL)
	return(NULL);
    //table = (hw_xmlElementTablePtr) dtd->elements;

    uqname = hw_xmlSplitQName2(name, &prefix);
    if (uqname != NULL)
        name = uqname;
    cur = 0;
    if (prefix != NULL) hw_xmlFree(prefix);
    if (uqname != NULL) hw_xmlFree(uqname);
    return(cur);
}

/**
 * hw_xmlGetDtdAttrDesc:
 * @dtd:  a pointer to the DtD to search
 * @elem:  the element name
 * @name:  the attribute name
 *
 * Search the DTD for the description of this attribute on
 * this element.
 *
 * returns the hw_xmlAttributePtr if found or NULL
 */

hw_xmlAttributePtr
hw_xmlGetDtdAttrDesc(hw_xmlDtdPtr dtd, const hw_xmlChar *elem, const hw_xmlChar *name) {
    hw_xmlAttributeTablePtr table;
    hw_xmlAttributePtr cur;
    hw_xmlChar *uqname = NULL, *prefix = NULL;

    if (dtd == NULL) return(NULL);
    if (dtd->attributes == NULL) return(NULL);

    table = (hw_xmlAttributeTablePtr) dtd->attributes;
    if (table == NULL)
	return(NULL);

    uqname = hw_xmlSplitQName2(name, &prefix);

    if (uqname != NULL) {
	cur = 0;
	if (prefix != NULL) hw_xmlFree(prefix);
	if (uqname != NULL) hw_xmlFree(uqname);
    } else
	cur = 0;
    return(cur);
}

/**
 * hw_xmlIsMixedElement:
 * @doc:  the document
 * @name:  the element name
 *
 * Search in the DtDs whether an element accept Mixed content (or ANY)
 * basically if it is supposed to accept text childs
 *
 * returns 0 if no, 1 if yes, and -1 if no element description is available
 */

int
hw_xmlIsMixedElement(hw_xmlDocPtr doc, const hw_xmlChar *name) {
    hw_xmlElementPtr elemDecl;

    if ((doc == NULL) || (doc->intSubset == NULL)) return(-1);

    elemDecl = hw_xmlGetDtdElementDesc(doc->intSubset, name);
    if ((elemDecl == NULL) && (doc->extSubset != NULL))
	elemDecl = hw_xmlGetDtdElementDesc(doc->extSubset, name);
    if (elemDecl == NULL) return(-1);
    switch (elemDecl->etype) {
	case XML_ELEMENT_TYPE_UNDEFINED:
	    return(-1);
	case XML_ELEMENT_TYPE_ELEMENT:
	    return(0);
        case XML_ELEMENT_TYPE_EMPTY:
	    /*
	     * return 1 for EMPTY since we want VC error to pop up
	     * on <empty>     </empty> for example
	     */
	case XML_ELEMENT_TYPE_ANY:
	case XML_ELEMENT_TYPE_MIXED:
	    return(1);
    }
    return(1);
}


