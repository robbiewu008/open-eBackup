/*
 * SAX2.c : Default SAX2 handler to build a tree.
 *
 * See Copyright for the status of this software.
 *
 * Daniel Veillard <daniel@veillard.com>
 */


#define IN_LIBXML
#include <stdlib.h>
#include "Etree.h"
#include "Eparser.h"

/* #define DEBUG_SAX2 */
/* #define DEBUG_SAX2_TREE */

/**
 * TODO:
 *
 * macro to flag unimplemented blocks
 * XML_CATALOG_PREFER user env to select between system/public prefered
 * option. C.f. Richard Tobin <richard@cogsci.ed.ac.uk>
 *> Just FYI, I am using an environment variable XML_CATALOG_PREFER with
 *> values "system" and "public".  I have made the default be "system" to
 *> match yours.
 */
#define TODO 								\
    hw_xmlGenericError(hw_xmlGenericErrorContext,				\
	    "Unimplemented block at %s:%d\n",				\
            __FILE__, __LINE__);

/*
 * xmlSAX2ErrMemory:
 * @ctxt:  an XML validation parser context
 * @msg:   a string to accompany the error message
 */
static void
xmlSAX2ErrMemory(hw_xmlParserCtxtPtr ctxt, const char *msg) {
    if (ctxt != NULL) {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt->userData, "%s: out of memory\n", msg);
	ctxt->errNo = XML_ERR_NO_MEMORY;
	ctxt->instate = XML_PARSER_EOF;
	ctxt->disableSAX = 1;
    }
}


/**
 * xmlFatalErrMsg:
 * @ctxt:  an XML parser context
 * @error:  the error number
 * @msg:  the error message
 * @str1:  an error string
 * @str2:  an error string
 *
 * Handle a fatal parser error, i.e. violating Well-Formedness constraints
 */
static void
xmlFatalErrMsg(hw_xmlParserCtxtPtr ctxt, hw_xmlParserErrors error,
               const char *msg, const hw_xmlChar *str1, const hw_xmlChar *str2)
{
    if ((ctxt != NULL) && (ctxt->disableSAX != 0) &&
        (ctxt->instate == XML_PARSER_EOF))
	return;
    if (ctxt != NULL)
	ctxt->errNo = error;
    hw___xmlRaiseError(NULL, NULL, NULL, ctxt, NULL, XML_FROM_PARSER, error,
                    XML_ERR_FATAL, NULL, 0, 
		    (const char *) str1, (const char *) str2,
		    NULL, 0, 0, msg, str1, str2);
    if (ctxt != NULL) {
	ctxt->wellFormed = 0;
	ctxt->valid = 0;
	if (ctxt->recovery == 0)
	    ctxt->disableSAX = 1;
    }
}


/**
 * hw_xmlSAX2GetEntity:
 * @ctx: the user data (XML parser context)
 * @name: The entity name
 *
 * Get an entity by name
 *
 * Returns the hw_xmlEntityPtr if found.
 */
hw_xmlEntityPtr
hw_xmlSAX2GetEntity(void *ctx, const hw_xmlChar *name)
{
    hw_xmlParserCtxtPtr ctxt = (hw_xmlParserCtxtPtr) ctx;
    hw_xmlEntityPtr ret = NULL;

    if (ctx == NULL) return(NULL);
#ifdef DEBUG_SAX
    hw_xmlGenericError(hw_xmlGenericErrorContext,
	    "SAX.xmlSAX2GetEntity(%s)\n", name);
#endif

    if (ctxt->inSubset == 0) {
	ret = hw_xmlGetPredefinedEntity(name);
	if (ret != NULL)
	    return(ret);
    }
    if ((ctxt->myDoc != NULL) && (ctxt->myDoc->standalone == 1)) {
	if (ctxt->inSubset == 2) {
	    ctxt->myDoc->standalone = 0;
	    ret = hw_xmlGetDocEntity(ctxt->myDoc, name);
	    ctxt->myDoc->standalone = 1;
	} else {
	    ret = hw_xmlGetDocEntity(ctxt->myDoc, name);
	    if (ret == NULL) {
		ctxt->myDoc->standalone = 0;
		ret = hw_xmlGetDocEntity(ctxt->myDoc, name);
		if (ret != NULL) {
		    xmlFatalErrMsg(ctxt, XML_ERR_NOT_STANDALONE,
	 "Entity(%s) document marked standalone but requires external subset\n",
				   name, NULL);
		}
		ctxt->myDoc->standalone = 1;
	    }
	}
    } else {
	ret = hw_xmlGetDocEntity(ctxt->myDoc, name);
    }
    if ((ret != NULL) &&
	((ctxt->replaceEntities)) &&
	(ret->children == NULL) &&
	(ret->etype == XML_EXTERNAL_GENERAL_PARSED_ENTITY)) {
	int val;

	/*
	 * for validation purposes we really need to fetch and
	 * parse the external entity
	 */
	hw_xmlNodePtr children;

        val = hw_xmlParseCtxtExternalEntity(ctxt, ret->URI,
		                         ret->ExternalID, &children);
	if (val == 0) {
	    hw_xmlAddChildList((hw_xmlNodePtr) ret, children);
	} else {
	    xmlFatalErrMsg(ctxt, XML_ERR_ENTITY_PROCESSING,
		           "Failure to process entity %s\n", name, NULL);
	    return(NULL);
	}
	ret->owner = 1;
    }
    return(ret);
}

/**
 * hw_xmlSAX2GetParameterEntity:
 * @ctx: the user data (XML parser context)
 * @name: The entity name
 *
 * Get a parameter entity by name
 *
 * Returns the hw_xmlEntityPtr if found.
 */
hw_xmlEntityPtr
hw_xmlSAX2GetParameterEntity(void *ctx, const hw_xmlChar *name)
{
    hw_xmlParserCtxtPtr ctxt = (hw_xmlParserCtxtPtr) ctx;
    hw_xmlEntityPtr ret;

    if (ctx == NULL) return(NULL);
#ifdef DEBUG_SAX
    hw_xmlGenericError(hw_xmlGenericErrorContext,
	    "SAX.xmlSAX2GetParameterEntity(%s)\n", name);
#endif

    ret = hw_xmlGetParameterEntity(ctxt->myDoc, name);
    return(ret);
}


/**
 * hw_xmlSAX2StartDocument:
 * @ctx: the user data (XML parser context)
 *
 * called when the document start being processed.
 */
void
hw_xmlSAX2StartDocument(void *ctx)
{
    hw_xmlParserCtxtPtr ctxt = (hw_xmlParserCtxtPtr) ctx;
    hw_xmlDocPtr doc;

    if (ctx == NULL) return;

#ifdef DEBUG_SAX
    hw_xmlGenericError(hw_xmlGenericErrorContext,
	    "SAX.xmlSAX2StartDocument()\n");
#endif
    if (ctxt->html) {
hw_xmlGenericError(hw_xmlGenericErrorContext,
		"libxml2 built without HTML support\n");
	ctxt->errNo = XML_ERR_INTERNAL_ERROR;
	ctxt->instate = XML_PARSER_EOF;
	ctxt->disableSAX = 1;
	return;

    } else {
	doc = ctxt->myDoc = hw_xmlNewDoc(ctxt->version);
	if (doc != NULL) {
	    if (ctxt->encoding != NULL)
		doc->encoding = hw_xmlStrdup(ctxt->encoding);
	    else
		doc->encoding = NULL;
	    doc->standalone = ctxt->standalone;
	} else {
	    xmlSAX2ErrMemory(ctxt, "hw_xmlSAX2StartDocument");
	    return;
	}
	if ((ctxt->dictNames) && (doc != NULL)) {
	    doc->dict = ctxt->dict;
	    hw_xmlDictReference(doc->dict);
	}
    }
    if ((ctxt->myDoc != NULL) && (ctxt->myDoc->URL == NULL) &&
	(ctxt->input != NULL) && (ctxt->input->filename != NULL)) {
	ctxt->myDoc->URL = hw_xmlCanonicPath((const hw_xmlChar *) ctxt->input->filename);
	if (ctxt->myDoc->URL == NULL)
	    xmlSAX2ErrMemory(ctxt, "hw_xmlSAX2StartDocument");
    }
}

/**
 * hw_xmlSAX2EndDocument:
 * @ctx: the user data (XML parser context)
 *
 * called when the document end has been detected.
 */
void
hw_xmlSAX2EndDocument(void *ctx)
{
    hw_xmlParserCtxtPtr ctxt = (hw_xmlParserCtxtPtr) ctx;
#ifdef DEBUG_SAX
    hw_xmlGenericError(hw_xmlGenericErrorContext,
	    "SAX.xmlSAX2EndDocument()\n");
#endif
    if (ctx == NULL) return;

    /*
     * Grab the encoding if it was added on-the-fly
     */
    if ((ctxt->encoding != NULL) && (ctxt->myDoc != NULL) &&
	(ctxt->myDoc->encoding == NULL)) {
	ctxt->myDoc->encoding = ctxt->encoding;
	ctxt->encoding = NULL;
    }
    if ((ctxt->inputTab != NULL) &&
        (ctxt->inputNr > 0) && (ctxt->inputTab[0] != NULL) &&
        (ctxt->inputTab[0]->encoding != NULL) && (ctxt->myDoc != NULL) &&
	(ctxt->myDoc->encoding == NULL)) {
	ctxt->myDoc->encoding = hw_xmlStrdup(ctxt->inputTab[0]->encoding);
    }
    if ((ctxt->charset != XML_CHAR_ENCODING_NONE) && (ctxt->myDoc != NULL) &&
	(ctxt->myDoc->charset == XML_CHAR_ENCODING_NONE)) {
	ctxt->myDoc->charset = ctxt->charset;
    }
}


/*
 * xmlSAX2TextNode:
 * @ctxt:  the parser context
 * @str:  the input string
 * @len: the string length
 * 
 * Remove the entities from an attribute value
 *
 * Returns the newly allocated string or NULL if not needed or error
 */
static hw_xmlNodePtr
xmlSAX2TextNode(hw_xmlParserCtxtPtr ctxt, const hw_xmlChar *str, int len) {
    hw_xmlNodePtr ret;
    const hw_xmlChar *intern = NULL;

    /*
     * Allocate
     */
    if (ctxt->freeElems != NULL) {
	ret = ctxt->freeElems;
	ctxt->freeElems = ret->next;
	ctxt->freeElemsNr--;
    } else {
	ret = (hw_xmlNodePtr) hw_xmlMalloc(sizeof(hw_xmlNode));
    }
    if (ret == NULL) {
        hw_xmlErrMemory(ctxt, "hw_xmlSAX2Characters");
	return(NULL);
    }
    memset(ret, 0, sizeof(hw_xmlNode));
    /*
     * intern the formatting blanks found between tags, or the
     * very short strings
     */
    if (ctxt->dictNames) {
        hw_xmlChar cur = str[len];

	if ((len <= 3) && ((cur == '"') || (cur == '\'') ||
	    ((cur == '<') && (str[len + 1] != '!')))) {
	    intern = hw_xmlDictLookup(ctxt->dict, str, len);
	} else if (hw_IS_BLANK_CH(*str) && (len < 60) && (cur == '<') &&
	           (str[len + 1] != '!')) {
	    int i;

	    for (i = 1;i < len;i++) {
		if (!hw_IS_BLANK_CH(str[i])) goto skip;
	    }
	    intern = hw_xmlDictLookup(ctxt->dict, str, len);
	}
    }
skip:
    ret->type = XML_TEXT_NODE;

    ret->name = hw_xmlStringText;
    if (intern == NULL) {
	ret->content = hw_xmlStrndup(str, len);
	if (ret->content == NULL) {
	    xmlSAX2ErrMemory(ctxt, "xmlSAX2TextNode");
	    hw_xmlFree(ret);
	    return(NULL);
	}
    } else
	ret->content = (hw_xmlChar *) intern;

    return(ret);
}


/**
 * xmlSAX2AttributeNs:
 * @ctx: the user data (XML parser context)
 * @localname:  the local name of the attribute
 * @prefix:  the attribute namespace prefix if available
 * @URI:  the attribute namespace name if available
 * @value:  Start of the attribute value
 * @valueend: end of the attribute value
 *
 * Handle an attribute that has been read by the parser.
 * The default handling is to convert the attribute into an
 * DOM subtree and past it in a new hw_xmlAttr element added to
 * the element.
 */
static void
xmlSAX2AttributeNs(hw_xmlParserCtxtPtr ctxt,
                   const hw_xmlChar * localname,
                   const hw_xmlChar * prefix,
		   const hw_xmlChar * value,
		   const hw_xmlChar * valueend)
{
    hw_xmlAttrPtr ret;
    hw_xmlNsPtr namespace = NULL;
    hw_xmlChar *dup = NULL;

    /*
     * Note: if prefix == NULL, the attribute is not in the default namespace
     */
    if (prefix != NULL)
	namespace = hw_xmlSearchNs(ctxt->myDoc, ctxt->node, prefix);

    /*
     * allocate the node
     */
    if (ctxt->freeAttrs != NULL) {
        ret = ctxt->freeAttrs;
	ctxt->freeAttrs = ret->next;
	ctxt->freeAttrsNr--;
	memset(ret, 0, sizeof(hw_xmlAttr));
	ret->type = XML_ATTRIBUTE_NODE;

	ret->parent = ctxt->node; 
	ret->doc = ctxt->myDoc;
	ret->ns = namespace;

	if (ctxt->dictNames)
	    ret->name = localname;
	else
	    ret->name = hw_xmlStrdup(localname);

        /* link at the end to preserv order, TODO speed up with a last */
	if (ctxt->node->properties == NULL) {
	    ctxt->node->properties = ret;
	} else {
	    hw_xmlAttrPtr prev = ctxt->node->properties;

	    while (prev->next != NULL) prev = prev->next;
	    prev->next = ret;
	    ret->prev = prev;
	}

    } else {
	if (ctxt->dictNames)
	    ret = hw_xmlNewNsPropEatName(ctxt->node, namespace, 
	                              (hw_xmlChar *) localname, NULL);
	else
	    ret = hw_xmlNewNsProp(ctxt->node, namespace, localname, NULL);
	if (ret == NULL) {
	    hw_xmlErrMemory(ctxt, "xmlSAX2AttributeNs");
	    return;
	}
    }

    if ((ctxt->replaceEntities == 0) && (!ctxt->html)) {
	hw_xmlNodePtr tmp;

	/*
	 * We know that if there is an entity reference, then
	 * the string has been dup'ed and terminates with 0
	 * otherwise with ' or "
	 */
	if (*valueend != 0) {
	    tmp = xmlSAX2TextNode(ctxt, value, valueend - value);
	    ret->children = tmp;
	    ret->last = tmp;
	    if (tmp != NULL) {
		tmp->doc = ret->doc;
		tmp->parent = (hw_xmlNodePtr) ret;
	    }
	} else {
	    ret->children = hw_xmlStringLenGetNodeList(ctxt->myDoc, value,
						    valueend - value);
	    tmp = ret->children;
	    while (tmp != NULL) {
	        tmp->doc = ret->doc;
		tmp->parent = (hw_xmlNodePtr) ret;
		if (tmp->next == NULL)
		    ret->last = tmp;
		tmp = tmp->next;
	    }
	}
    } else if (value != NULL) {
	hw_xmlNodePtr tmp;

	tmp = xmlSAX2TextNode(ctxt, value, valueend - value);
	ret->children = tmp;
	ret->last = tmp;
	if (tmp != NULL) {
	    tmp->doc = ret->doc;
	    tmp->parent = (hw_xmlNodePtr) ret;
	}
    }


    if (dup != NULL)
	hw_xmlFree(dup);
}

/**
 * hw_xmlSAX2StartElementNs:
 * @ctx:  the user data (XML parser context)
 * @localname:  the local name of the element
 * @prefix:  the element namespace prefix if available
 * @URI:  the element namespace name if available
 * @nb_namespaces:  number of namespace definitions on that node
 * @namespaces:  pointer to the array of prefix/URI pairs namespace definitions
 * @nb_attributes:  the number of attributes on that node
 * @nb_defaulted:  the number of defaulted attributes.
 * @attributes:  pointer to the array of (localname/prefix/URI/value/end)
 *               attribute values.
 *
 * SAX2 callback when an element start has been detected by the parser.
 * It provides the namespace informations for the element, as well as
 * the new namespace declarations on the element.
 */
void
hw_xmlSAX2StartElementNs(void *ctx,
                      const hw_xmlChar *localname,
		      const hw_xmlChar *prefix,
		      const hw_xmlChar *URI,
		      int nb_namespaces,
		      const hw_xmlChar **namespaces,
		      int nb_attributes,
		      int nb_defaulted,
		      const hw_xmlChar **attributes)
{
    hw_xmlParserCtxtPtr ctxt = (hw_xmlParserCtxtPtr) ctx;
    hw_xmlNodePtr ret;
    hw_xmlNodePtr parent;
    hw_xmlNsPtr last = NULL, ns;
    const hw_xmlChar *uri, *pref;
    int i, j;

    if (ctx == NULL) return;
    parent = ctxt->node;
    /*
     * First check on validity:
     */


    /*
     * allocate the node
     */
    if (ctxt->freeElems != NULL) {
        ret = ctxt->freeElems;
	ctxt->freeElems = ret->next;
	ctxt->freeElemsNr--;
	memset(ret, 0, sizeof(hw_xmlNode));
	ret->type = XML_ELEMENT_NODE;

	if (ctxt->dictNames)
	    ret->name = localname;
	else {
	    ret->name = hw_xmlStrdup(localname);
	    if (ret->name == NULL) {
	        xmlSAX2ErrMemory(ctxt, "hw_xmlSAX2StartElementNs");
		return;
	    }
	}
    } else {
	if (ctxt->dictNames)
	    ret = hw_xmlNewDocNodeEatName(ctxt->myDoc, NULL, 
	                               (hw_xmlChar *) localname, NULL);
	else
	    ret = hw_xmlNewDocNode(ctxt->myDoc, NULL, localname, NULL);
	if (ret == NULL) {
	    xmlSAX2ErrMemory(ctxt, "hw_xmlSAX2StartElementNs");
	    return;
	}
    }
    if (ctxt->linenumbers) {
	if (ctxt->input != NULL) {
	    if (ctxt->input->line < 65535)
		ret->line = (short) ctxt->input->line;
	    else
	        ret->line = 65535;
	}
    }

    if ((ctxt->myDoc->children == NULL) || (parent == NULL)) {
        hw_xmlAddChild((hw_xmlNodePtr) ctxt->myDoc, (hw_xmlNodePtr) ret);
    }
    /*
     * Build the namespace list
     */
    for (i = 0,j = 0;j < nb_namespaces;j++) {
        pref = namespaces[i++];
	uri = namespaces[i++];
	ns = hw_xmlNewNs(NULL, uri, pref);
	if (ns != NULL) {
	    if (last == NULL) {
	        ret->nsDef = last = ns;
	    } else {
	        last->next = ns;
		last = ns;
	    }
	    if ((URI != NULL) && (prefix == pref))
		ret->ns = ns;
	} else {
	    xmlSAX2ErrMemory(ctxt, "hw_xmlSAX2StartElementNs");
	    return;
	}
    }
    ctxt->nodemem = -1;

    /*
     * We are parsing a new node.
     */
    hw_nodePush(ctxt, ret);

    /*
     * Link the child element
     */
    if (parent != NULL) {
        if (parent->type == XML_ELEMENT_NODE) {
	    hw_xmlAddChild(parent, ret);
	} else {
	    hw_xmlAddSibling(parent, ret);
	}
    }


    /*
     * Search the namespace if it wasn't already found
     * Note that, if prefix is NULL, this searches for the default Ns
     */
    if ((URI != NULL) && (ret->ns == NULL)) {
        ret->ns = hw_xmlSearchNs(ctxt->myDoc, parent, prefix);
	if ((ret->ns == NULL) && (hw_xmlStrEqual(prefix, hw_BAD_CAST "xml"))) {
	    ret->ns = hw_xmlSearchNs(ctxt->myDoc, ret, prefix);
	}
	if (ret->ns == NULL) {
	    ns = hw_xmlNewNs(ret, NULL, prefix);
	    if (ns == NULL) {

	        xmlSAX2ErrMemory(ctxt, "hw_xmlSAX2StartElementNs");
		return;
	    }
	    if ((ctxt->sax != NULL) && (ctxt->sax->warning != NULL))
		ctxt->sax->warning(ctxt->userData, 
		     "Namespace prefix %s was not found\n", prefix);
	}
    }

    /*
     * process all the other attributes
     */
    if (nb_attributes > 0) {
        for (j = 0,i = 0;i < nb_attributes;i++,j+=5) {
	    xmlSAX2AttributeNs(ctxt, attributes[j], attributes[j+1],
	                       attributes[j+3], attributes[j+4]);
	}
    }

}

/**
 * hw_xmlSAX2EndElementNs:
 * @ctx:  the user data (XML parser context)
 * @localname:  the local name of the element
 * @prefix:  the element namespace prefix if available
 * @URI:  the element namespace name if available
 *
 * SAX2 callback when an element end has been detected by the parser.
 * It provides the namespace informations for the element.
 */
void
hw_xmlSAX2EndElementNs(void *ctx,
                    const hw_xmlChar * localname ATTRIBUTE_UNUSED,
                    const hw_xmlChar * prefix ATTRIBUTE_UNUSED,
		    const hw_xmlChar * URI ATTRIBUTE_UNUSED)
{
    hw_xmlParserCtxtPtr ctxt = (hw_xmlParserCtxtPtr) ctx;
    hw_xmlParserNodeInfo node_info;
    hw_xmlNodePtr cur;

    if (ctx == NULL) return;
    cur = ctxt->node;
    /* Capture end position and add node */
    if ((ctxt->record_info) && (cur != NULL)) {
        node_info.end_pos = ctxt->input->cur - ctxt->input->base;
        node_info.end_line = ctxt->input->line;
        node_info.node = cur;
        hw_xmlParserAddNodeInfo(ctxt, &node_info);
    }
    ctxt->nodemem = -1;


    /*
     * end of parsing of this node.
     */
    hw_nodePop(ctxt);
}

/**
 * hw_xmlSAX2Reference:
 * @ctx: the user data (XML parser context)
 * @name:  The entity name
 *
 * called when an entity hw_xmlSAX2Reference is detected. 
 */
void
hw_xmlSAX2Reference(void *ctx, const hw_xmlChar *name)
{
    hw_xmlParserCtxtPtr ctxt = (hw_xmlParserCtxtPtr) ctx;
    hw_xmlNodePtr ret;

    if (ctx == NULL) return;
#ifdef DEBUG_SAX
    hw_xmlGenericError(hw_xmlGenericErrorContext,
	    "SAX.xmlSAX2Reference(%s)\n", name);
#endif
    if (name[0] == '#')
	ret = hw_xmlNewCharRef(ctxt->myDoc, name);
    else
	ret = hw_xmlNewReference(ctxt->myDoc, name);
#ifdef DEBUG_SAX_TREE
    hw_xmlGenericError(hw_xmlGenericErrorContext,
	    "add hw_xmlSAX2Reference %s to %s \n", name, ctxt->node->name);
#endif
    hw_xmlAddChild(ctxt->node, ret);
}

/**
 * hw_xmlSAX2Characters:
 * @ctx: the user data (XML parser context)
 * @ch:  a hw_xmlChar string
 * @len: the number of hw_xmlChar
 *
 * receiving some chars from the parser.
 */
void
hw_xmlSAX2Characters(void *ctx, const hw_xmlChar *ch, int len)
{
    hw_xmlParserCtxtPtr ctxt = (hw_xmlParserCtxtPtr) ctx;
    hw_xmlNodePtr lastChild;

    if (ctx == NULL) return;
#ifdef DEBUG_SAX
    hw_xmlGenericError(hw_xmlGenericErrorContext,
	    "SAX.xmlSAX2Characters(%.30s, %d)\n", ch, len);
#endif
    /*
     * Handle the data if any. If there is no child
     * add it as content, otherwise if the last child is text,
     * concatenate it, else create a new node of type text.
     */

    if (ctxt->node == NULL) {
#ifdef DEBUG_SAX_TREE
	hw_xmlGenericError(hw_xmlGenericErrorContext,
		"add chars: ctxt->node == NULL !\n");
#endif
        return;
    }
    lastChild = ctxt->node->last;
#ifdef DEBUG_SAX_TREE
    hw_xmlGenericError(hw_xmlGenericErrorContext,
	    "add chars to %s \n", ctxt->node->name);
#endif

    /*
     * Here we needed an accelerator mechanism in case of very large
     * elements. Use an attribute in the structure !!!
     */
    if (lastChild == NULL) {
        lastChild = xmlSAX2TextNode(ctxt, ch, len);
	if (lastChild != NULL) {
	    ctxt->node->children = lastChild;
	    ctxt->node->last = lastChild;
	    lastChild->parent = ctxt->node;
	    lastChild->doc = ctxt->node->doc;
	    ctxt->nodelen = len;
	    ctxt->nodemem = len + 1;
	} else {
	    xmlSAX2ErrMemory(ctxt, "hw_xmlSAX2Characters");
	    return;
	}
    } else {
	int coalesceText = (lastChild != NULL) &&
	    (lastChild->type == XML_TEXT_NODE) &&
	    (lastChild->name == hw_xmlStringText);
	if ((coalesceText) && (ctxt->nodemem != 0)) {
	    /*
	     * The whole point of maintaining nodelen and nodemem,
	     * hw_xmlTextConcat is too costly, i.e. compute length,
	     * reallocate a new buffer, move data, append ch. Here
	     * We try to minimaze realloc() uses and avoid copying
	     * and recomputing length over and over.
	     */
	    if (lastChild->content == (hw_xmlChar *)&(lastChild->properties)) {
		lastChild->content = hw_xmlStrdup(lastChild->content);
		lastChild->properties = NULL;
	    } else if ((ctxt->nodemem == ctxt->nodelen + 1) &&
	               (hw_xmlDictOwns(ctxt->dict, lastChild->content))) {
		lastChild->content = hw_xmlStrdup(lastChild->content);
	    }
	    if (ctxt->nodelen + len >= ctxt->nodemem) {
		hw_xmlChar *newbuf;
		int size;

		size = ctxt->nodemem + len;
		size *= 2;
                newbuf = (hw_xmlChar *) hw_xmlRealloc(lastChild->content,size);
		if (newbuf == NULL) {
		    xmlSAX2ErrMemory(ctxt, "hw_xmlSAX2Characters");
		    return;
		}
		ctxt->nodemem = size;
		lastChild->content = newbuf;
	    }
	    memcpy(&lastChild->content[ctxt->nodelen], ch, len);
	    ctxt->nodelen += len;
	    lastChild->content[ctxt->nodelen] = 0;
	} else if (coalesceText) {
	    if (hw_xmlTextConcat(lastChild, ch, len)) {
		xmlSAX2ErrMemory(ctxt, "hw_xmlSAX2Characters");
	    }
	    if (ctxt->node->children != NULL) {
		ctxt->nodelen = hw_xmlStrlen(lastChild->content);
		ctxt->nodemem = ctxt->nodelen + 1;
	    }
	} else {
	    /* Mixed content, first time */
	    lastChild = xmlSAX2TextNode(ctxt, ch, len);
	    if (lastChild != NULL) {
		hw_xmlAddChild(ctxt->node, lastChild);
		if (ctxt->node->children != NULL) {
		    ctxt->nodelen = len;
		    ctxt->nodemem = len + 1;
		}
	    }
	}
    }
}


/**
 * hw_xmlSAX2ProcessingInstruction:
 * @ctx: the user data (XML parser context)
 * @target:  the target name
 * @data: the PI data's
 *
 * A processing instruction has been parsed.
 */
void
hw_xmlSAX2ProcessingInstruction(void *ctx, const hw_xmlChar *target,
                      const hw_xmlChar *data)
{
    hw_xmlParserCtxtPtr ctxt = (hw_xmlParserCtxtPtr) ctx;
    hw_xmlNodePtr ret;
    hw_xmlNodePtr parent;

    if (ctx == NULL) return;
    parent = ctxt->node;
#ifdef DEBUG_SAX
    hw_xmlGenericError(hw_xmlGenericErrorContext,
	    "SAX.xmlSAX2ProcessingInstruction(%s, %s)\n", target, data);
#endif

    ret = hw_xmlNewDocPI(ctxt->myDoc, target, data);
    if (ret == NULL) return;
    parent = ctxt->node;

    if (ctxt->linenumbers) {
	if (ctxt->input != NULL) {
	    if (ctxt->input->line < 65535)
		ret->line = (short) ctxt->input->line;
	    else
	        ret->line = 65535;
	}
    }
    if (ctxt->inSubset == 1) {
	hw_xmlAddChild((hw_xmlNodePtr) ctxt->myDoc->intSubset, ret);
	return;
    } else if (ctxt->inSubset == 2) {
	hw_xmlAddChild((hw_xmlNodePtr) ctxt->myDoc->extSubset, ret);
	return;
    }
    if ((ctxt->myDoc->children == NULL) || (parent == NULL)) {
#ifdef DEBUG_SAX_TREE
	    hw_xmlGenericError(hw_xmlGenericErrorContext,
		    "Setting PI %s as root\n", target);
#endif
        hw_xmlAddChild((hw_xmlNodePtr) ctxt->myDoc, (hw_xmlNodePtr) ret);
	return;
    }
    if (parent->type == XML_ELEMENT_NODE) {
#ifdef DEBUG_SAX_TREE
	hw_xmlGenericError(hw_xmlGenericErrorContext,
		"adding PI %s child to %s\n", target, parent->name);
#endif
	hw_xmlAddChild(parent, ret);
    } else {
#ifdef DEBUG_SAX_TREE
	hw_xmlGenericError(hw_xmlGenericErrorContext,
		"adding PI %s sibling to ", target);
	xmlDebugDumpOneNode(stderr, parent, 0);
#endif
	hw_xmlAddSibling(parent, ret);
    }
}

/**
 * hw_xmlSAX2Comment:
 * @ctx: the user data (XML parser context)
 * @value:  the hw_xmlSAX2Comment content
 *
 * A hw_xmlSAX2Comment has been parsed.
 */
void
hw_xmlSAX2Comment(void *ctx, const hw_xmlChar *value)
{
    hw_xmlParserCtxtPtr ctxt = (hw_xmlParserCtxtPtr) ctx;
    hw_xmlNodePtr ret;
    hw_xmlNodePtr parent;

    if (ctx == NULL) return;
    parent = ctxt->node;
#ifdef DEBUG_SAX
    hw_xmlGenericError(hw_xmlGenericErrorContext, "SAX.xmlSAX2Comment(%s)\n", value);
#endif
    ret = hw_xmlNewDocComment(ctxt->myDoc, value);
    if (ret == NULL) return;
    if (ctxt->linenumbers) {
	if (ctxt->input != NULL) {
	    if (ctxt->input->line < 65535)
		ret->line = (short) ctxt->input->line;
	    else
	        ret->line = 65535;
	}
    }

    if (ctxt->inSubset == 1) {
	hw_xmlAddChild((hw_xmlNodePtr) ctxt->myDoc->intSubset, ret);
	return;
    } else if (ctxt->inSubset == 2) {
	hw_xmlAddChild((hw_xmlNodePtr) ctxt->myDoc->extSubset, ret);
	return;
    }
    if ((ctxt->myDoc->children == NULL) || (parent == NULL)) {
#ifdef DEBUG_SAX_TREE
	    hw_xmlGenericError(hw_xmlGenericErrorContext,
		    "Setting hw_xmlSAX2Comment as root\n");
#endif
        hw_xmlAddChild((hw_xmlNodePtr) ctxt->myDoc, (hw_xmlNodePtr) ret);
	return;
    }
    if (parent->type == XML_ELEMENT_NODE) {
#ifdef DEBUG_SAX_TREE
	hw_xmlGenericError(hw_xmlGenericErrorContext,
		"adding hw_xmlSAX2Comment child to %s\n", parent->name);
#endif
	hw_xmlAddChild(parent, ret);
    } else {
#ifdef DEBUG_SAX_TREE
	hw_xmlGenericError(hw_xmlGenericErrorContext,
		"adding hw_xmlSAX2Comment sibling to ");
	xmlDebugDumpOneNode(stderr, parent, 0);
#endif
	hw_xmlAddSibling(parent, ret);
    }
}


//static int xmlSAX2DefaultVersionValue = 2;


/**
 * hw_xmlSAXVersion:
 * @hdlr:  the SAX handler
 * @version:  the version, 1 or 2
 *
 * Initialize the default XML SAX handler according to the version
 *
 * Returns 0 in case of success and -1 in case of error.
 */
int
hw_xmlSAXVersion(hw_xmlSAXHandler *hdlr, int version)
{
    if (hdlr == NULL) return(-1);
    if (version == 2) {
	hdlr->startElement = NULL;
	hdlr->endElement = NULL;
	hdlr->startElementNs = hw_xmlSAX2StartElementNs;
	hdlr->endElementNs = hw_xmlSAX2EndElementNs;
	hdlr->serror = NULL;
	hdlr->initialized = hw_XML_SAX2_MAGIC;
    } else
        return(-1);
    hdlr->getEntity = hw_xmlSAX2GetEntity;
    hdlr->getParameterEntity = hw_xmlSAX2GetParameterEntity;
    hdlr->startDocument = hw_xmlSAX2StartDocument;
    hdlr->endDocument = hw_xmlSAX2EndDocument;
    hdlr->reference = hw_xmlSAX2Reference;
    hdlr->characters = hw_xmlSAX2Characters;
    hdlr->ignorableWhitespace = hw_xmlSAX2Characters;
    hdlr->processingInstruction = hw_xmlSAX2ProcessingInstruction;
    hdlr->comment = hw_xmlSAX2Comment;
    hdlr->warning = hw_xmlParserWarning;
    hdlr->error = hw_xmlParserError;
    hdlr->fatalError = hw_xmlParserError;

    return(0);
}



