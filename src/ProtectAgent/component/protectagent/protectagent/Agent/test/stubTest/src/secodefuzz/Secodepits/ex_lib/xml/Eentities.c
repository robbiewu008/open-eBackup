/*
 * entities.c : implementation for the XML entities handling
 *
 * See Copyright for the status of this software.
 *
 * daniel@veillard.com
 */

#define IN_LIBXML

#include "Eparser.h"
#include "Etree.h"

/*
 * The XML predefined entities.
 */

static hw_xmlEntity xmlEntityLt = {
    NULL, XML_ENTITY_DECL, hw_BAD_CAST "lt",
    NULL, NULL, NULL, NULL, NULL, NULL, 
    hw_BAD_CAST "<", hw_BAD_CAST "<", 1,
    XML_INTERNAL_PREDEFINED_ENTITY,
    NULL, NULL, NULL, NULL, 0
};
static hw_xmlEntity xmlEntityGt = {
    NULL, XML_ENTITY_DECL, hw_BAD_CAST "gt",
    NULL, NULL, NULL, NULL, NULL, NULL, 
    hw_BAD_CAST ">", hw_BAD_CAST ">", 1,
    XML_INTERNAL_PREDEFINED_ENTITY,
    NULL, NULL, NULL, NULL, 0
};
static hw_xmlEntity xmlEntityAmp = {
    NULL, XML_ENTITY_DECL, hw_BAD_CAST "amp",
    NULL, NULL, NULL, NULL, NULL, NULL, 
    hw_BAD_CAST "&", hw_BAD_CAST "&", 1,
    XML_INTERNAL_PREDEFINED_ENTITY,
    NULL, NULL, NULL, NULL, 0
};
static hw_xmlEntity xmlEntityQuot = {
    NULL, XML_ENTITY_DECL, hw_BAD_CAST "quot",
    NULL, NULL, NULL, NULL, NULL, NULL, 
    hw_BAD_CAST "\"", hw_BAD_CAST "\"", 1,
    XML_INTERNAL_PREDEFINED_ENTITY,
    NULL, NULL, NULL, NULL, 0
};
static hw_xmlEntity xmlEntityApos = {
    NULL, XML_ENTITY_DECL, hw_BAD_CAST "apos",
    NULL, NULL, NULL, NULL, NULL, NULL, 
    hw_BAD_CAST "'", hw_BAD_CAST "'", 1,
    XML_INTERNAL_PREDEFINED_ENTITY,
    NULL, NULL, NULL, NULL, 0
};

/**
 * xmlEntitiesErrMemory:
 * @extra:  extra informations
 *
 * Handle an out of memory condition
 */
static void
xmlEntitiesErrMemory(const char *extra)
{
    hw___xmlSimpleError(XML_FROM_TREE, XML_ERR_NO_MEMORY, NULL, NULL, extra);
}

/**
 * xmlEntitiesErr:
 * @code:  the error code
 * @msg:  the message
 *
 * Handle an out of memory condition
 */
static void
xmlEntitiesErr(hw_xmlParserErrors code, const char *msg)
{
    hw___xmlSimpleError(XML_FROM_TREE, code, NULL, msg, NULL);
}
/**
 * hw_xmlGetPredefinedEntity:
 * @name:  the entity name
 *
 * Check whether this name is an predefined entity.
 *
 * Returns NULL if not, otherwise the entity
 */
hw_xmlEntityPtr
hw_xmlGetPredefinedEntity(const hw_xmlChar *name) {
    if (name == NULL) return(NULL);
    switch (name[0]) {
        case 'l':
	    if (hw_xmlStrEqual(name, hw_BAD_CAST "lt"))
	        return(&xmlEntityLt);
	    break;
        case 'g':
	    if (hw_xmlStrEqual(name, hw_BAD_CAST "gt"))
	        return(&xmlEntityGt);
	    break;
        case 'a':
	    if (hw_xmlStrEqual(name, hw_BAD_CAST "amp"))
	        return(&xmlEntityAmp);
	    if (hw_xmlStrEqual(name, hw_BAD_CAST "apos"))
	        return(&xmlEntityApos);
	    break;
        case 'q':
	    if (hw_xmlStrEqual(name, hw_BAD_CAST "quot"))
	        return(&xmlEntityQuot);
	    break;
	default:
	    break;
    }
    return(NULL);
}

/**
 * xmlGetEntityFromTable:
 * @table:  an entity table
 * @name:  the entity name
 * @parameter:  look for parameter entities
 *
 * Do an entity lookup in the table.
 * returns the corresponding parameter entity, if found.
 * 
 * Returns A pointer to the entity structure or NULL if not found.
 */
static hw_xmlEntityPtr
xmlGetEntityFromTable(hw_xmlEntitiesTablePtr table, const hw_xmlChar *name) {
    return 0;
}

/**
 * hw_xmlGetParameterEntity:
 * @doc:  the document referencing the entity
 * @name:  the entity name
 *
 * Do an entity lookup in the internal and external subsets and
 * returns the corresponding parameter entity, if found.
 * 
 * Returns A pointer to the entity structure or NULL if not found.
 */
hw_xmlEntityPtr
hw_xmlGetParameterEntity(hw_xmlDocPtr doc, const hw_xmlChar *name) {
    hw_xmlEntitiesTablePtr table;
    hw_xmlEntityPtr ret;

    if (doc == NULL)
	return(NULL);
    if ((doc->intSubset != NULL) && (doc->intSubset->pentities != NULL)) {
	table = (hw_xmlEntitiesTablePtr) doc->intSubset->pentities;
	ret = xmlGetEntityFromTable(table, name);
	if (ret != NULL)
	    return(ret);
    }
    if ((doc->extSubset != NULL) && (doc->extSubset->pentities != NULL)) {
	table = (hw_xmlEntitiesTablePtr) doc->extSubset->pentities;
	return(xmlGetEntityFromTable(table, name));
    }
    return(NULL);
}

/**
 * hw_xmlGetDocEntity:
 * @doc:  the document referencing the entity
 * @name:  the entity name
 *
 * Do an entity lookup in the document entity hash table and
 * returns the corresponding entity, otherwise a lookup is done
 * in the predefined entities too.
 * 
 * Returns A pointer to the entity structure or NULL if not found.
 */
hw_xmlEntityPtr
hw_xmlGetDocEntity(hw_xmlDocPtr doc, const hw_xmlChar *name) {
    hw_xmlEntityPtr cur;
    hw_xmlEntitiesTablePtr table;

    if (doc != NULL) {
	if ((doc->intSubset != NULL) && (doc->intSubset->entities != NULL)) {
	    table = (hw_xmlEntitiesTablePtr) doc->intSubset->entities;
	    cur = xmlGetEntityFromTable(table, name);
	    if (cur != NULL)
		return(cur);
	}
	if (doc->standalone != 1) {
	    if ((doc->extSubset != NULL) &&
		(doc->extSubset->entities != NULL)) {
		table = (hw_xmlEntitiesTablePtr) doc->extSubset->entities;
		cur = xmlGetEntityFromTable(table, name);
		if (cur != NULL)
		    return(cur);
	    }
	}
    }
    return(hw_xmlGetPredefinedEntity(name));
}

/*
 * Macro used to grow the current buffer.
 */
#define growBufferReentrant() {						\
    buffer_size *= 2;							\
    buffer = (hw_xmlChar *)						\
    		hw_xmlRealloc(buffer, buffer_size * sizeof(hw_xmlChar));	\
    if (buffer == NULL) {						\
        xmlEntitiesErrMemory("hw_xmlEncodeEntitiesReentrant: realloc failed");\
	return(NULL);							\
    }									\
}


/**
 * hw_xmlEncodeEntitiesReentrant:
 * @doc:  the document containing the string
 * @input:  A string to convert to XML.
 *
 * Do a global encoding of a string, replacing the predefined entities
 * and non ASCII values with their entities and CharRef counterparts.
 * Contrary to xmlEncodeEntities, this routine is reentrant, and result
 * must be deallocated.
 *
 * Returns A newly allocated string with the substitution done.
 */
hw_xmlChar *
hw_xmlEncodeEntitiesReentrant(hw_xmlDocPtr doc, const hw_xmlChar *input) {
    const hw_xmlChar *cur = input;
    hw_xmlChar *buffer = NULL;
    hw_xmlChar *out = NULL;
    int buffer_size = 0;
    int html = 0;

    if (input == NULL) return(NULL);
    if (doc != NULL)
        html = (doc->type == XML_HTML_DOCUMENT_NODE);

    /*
     * allocate an translation buffer.
     */
    buffer_size = 1000;
    buffer = (hw_xmlChar *) hw_xmlMalloc(buffer_size * sizeof(hw_xmlChar));
    if (buffer == NULL) {
        xmlEntitiesErrMemory("hw_xmlEncodeEntitiesReentrant: malloc failed");
	return(NULL);
    }
    out = buffer;

    while (*cur != '\0') {
        if (out - buffer > buffer_size - 100) {
	    int indx = out - buffer;

	    growBufferReentrant();
	    out = &buffer[indx];
	}

	/*
	 * By default one have to encode at least '<', '>', '"' and '&' !
	 */
	if (*cur == '<') {
	    *out++ = '&';
	    *out++ = 'l';
	    *out++ = 't';
	    *out++ = ';';
	} else if (*cur == '>') {
	    *out++ = '&';
	    *out++ = 'g';
	    *out++ = 't';
	    *out++ = ';';
	} else if (*cur == '&') {
	    *out++ = '&';
	    *out++ = 'a';
	    *out++ = 'm';
	    *out++ = 'p';
	    *out++ = ';';
	} else if (((*cur >= 0x20) && (*cur < 0x80)) ||
	    (*cur == '\n') || (*cur == '\t') || ((html) && (*cur == '\r'))) {
	    /*
	     * default case, just copy !
	     */
	    *out++ = *cur;
	} else if (*cur >= 0x80) {
	    if (((doc != NULL) && (doc->encoding != NULL)) || (html)) {
		/*
		 * Bjørn Reese <br@sseusa.com> provided the patch
	        hw_xmlChar xc;
	        xc = (*cur & 0x3F) << 6;
	        if (cur[1] != 0) {
		    xc += *(++cur) & 0x3F;
		    *out++ = xc;
	        } else
		 */
		*out++ = *cur;
	    } else {
		/*
		 * We assume we have UTF-8 input.
		 */
		char buf[11], *ptr;
		int val = 0, l = 1;

		if (*cur < 0xC0) {
		    xmlEntitiesErr(XML_CHECK_NOT_UTF8,
			    "hw_xmlEncodeEntitiesReentrant : input not UTF-8");
		    if (doc != NULL)
			doc->encoding = hw_xmlStrdup(hw_BAD_CAST "ISO-8859-1");
		    snprintf(buf, sizeof(buf), "&#%d;", *cur);
		    buf[sizeof(buf) - 1] = 0;
		    ptr = buf;
		    while (*ptr != 0) *out++ = *ptr++;
		    cur++;
		    continue;
		} else if (*cur < 0xE0) {
                    val = (cur[0]) & 0x1F;
		    val <<= 6;
		    val |= (cur[1]) & 0x3F;
		    l = 2;
		} else if (*cur < 0xF0) {
                    val = (cur[0]) & 0x0F;
		    val <<= 6;
		    val |= (cur[1]) & 0x3F;
		    val <<= 6;
		    val |= (cur[2]) & 0x3F;
		    l = 3;
		} else if (*cur < 0xF8) {
                    val = (cur[0]) & 0x07;
		    val <<= 6;
		    val |= (cur[1]) & 0x3F;
		    val <<= 6;
		    val |= (cur[2]) & 0x3F;
		    val <<= 6;
		    val |= (cur[3]) & 0x3F;
		    l = 4;
		}
		if ((l == 1) || (!hw_IS_CHAR(val))) {
		    xmlEntitiesErr(XML_ERR_INVALID_CHAR,
			"hw_xmlEncodeEntitiesReentrant : char out of range\n");
		    if (doc != NULL)
			doc->encoding = hw_xmlStrdup(hw_BAD_CAST "ISO-8859-1");
		    snprintf(buf, sizeof(buf), "&#%d;", *cur);
		    buf[sizeof(buf) - 1] = 0;
		    ptr = buf;
		    while (*ptr != 0) *out++ = *ptr++;
		    cur++;
		    continue;
		}
		/*
		 * We could do multiple things here. Just save as a char ref
		 */
		snprintf(buf, sizeof(buf), "&#x%X;", val);
		buf[sizeof(buf) - 1] = 0;
		ptr = buf;
		while (*ptr != 0) *out++ = *ptr++;
		cur += l;
		continue;
	    }
	} else if (hw_IS_BYTE_CHAR(*cur)) {
	    char buf[11], *ptr;

	    snprintf(buf, sizeof(buf), "&#%d;", *cur);
	    buf[sizeof(buf) - 1] = 0;
            ptr = buf;
	    while (*ptr != 0) *out++ = *ptr++;
	}
	cur++;
    }
    *out++ = 0;
    return(buffer);
}

/**
 * hw_xmlEncodeSpecialChars:
 * @doc:  the document containing the string
 * @input:  A string to convert to XML.
 *
 * Do a global encoding of a string, replacing the predefined entities
 * this routine is reentrant, and result must be deallocated.
 *
 * Returns A newly allocated string with the substitution done.
 */
hw_xmlChar *
hw_xmlEncodeSpecialChars(hw_xmlDocPtr doc ATTRIBUTE_UNUSED, const hw_xmlChar *input) {
    const hw_xmlChar *cur = input;
    hw_xmlChar *buffer = NULL;
    hw_xmlChar *out = NULL;
    int buffer_size = 0;
    if (input == NULL) return(NULL);

    /*
     * allocate an translation buffer.
     */
    buffer_size = 1000;
    buffer = (hw_xmlChar *) hw_xmlMalloc(buffer_size * sizeof(hw_xmlChar));
    if (buffer == NULL) {
        xmlEntitiesErrMemory("hw_xmlEncodeSpecialChars: malloc failed");
	return(NULL);
    }
    out = buffer;

    while (*cur != '\0') {
        if (out - buffer > buffer_size - 10) {
	    int indx = out - buffer;

	    growBufferReentrant();
	    out = &buffer[indx];
	}

	/*
	 * By default one have to encode at least '<', '>', '"' and '&' !
	 */
	if (*cur == '<') {
	    *out++ = '&';
	    *out++ = 'l';
	    *out++ = 't';
	    *out++ = ';';
	} else if (*cur == '>') {
	    *out++ = '&';
	    *out++ = 'g';
	    *out++ = 't';
	    *out++ = ';';
	} else if (*cur == '&') {
	    *out++ = '&';
	    *out++ = 'a';
	    *out++ = 'm';
	    *out++ = 'p';
	    *out++ = ';';
	} else if (*cur == '"') {
	    *out++ = '&';
	    *out++ = 'q';
	    *out++ = 'u';
	    *out++ = 'o';
	    *out++ = 't';
	    *out++ = ';';
	} else if (*cur == '\r') {
	    *out++ = '&';
	    *out++ = '#';
	    *out++ = '1';
	    *out++ = '3';
	    *out++ = ';';
	} else {
	    /*
	     * Works because on UTF-8, all extended sequences cannot
	     * result in bytes in the ASCII range.
	     */
	    *out++ = *cur;
	}
	cur++;
    }
    *out++ = 0;
    return(buffer);
}


/**
 * hw_xmlFreeEntitiesTable:
 * @table:  An entity table
 *
 * Deallocate the memory used by an entities hash table.
 */
void
hw_xmlFreeEntitiesTable(hw_xmlEntitiesTablePtr table) {
}


