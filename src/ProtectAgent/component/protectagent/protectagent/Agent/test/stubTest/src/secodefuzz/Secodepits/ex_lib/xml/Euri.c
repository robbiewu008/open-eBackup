/**
 * uri.c: set of generic URI related routines 
 *
 * Reference: RFCs 2396, 2732 and 2373
 *
 * See Copyright for the status of this software.
 *
 * daniel@veillard.com
 */

#define IN_LIBXML


#include "Etree.h"
/************************************************************************
 *									*
 *		Macros to differentiate various character type		*
 *			directly extracted from RFC 2396		*
 *									*
 ************************************************************************/

/*
 * alpha    = lowalpha | upalpha
 */
#define IS_ALPHA(x) (IS_LOWALPHA(x) || IS_UPALPHA(x))


/*
 * lowalpha = "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" | "j" |
 *            "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r" | "s" | "t" |
 *            "u" | "v" | "w" | "x" | "y" | "z"
 */

#define IS_LOWALPHA(x) (((x) >= 'a') && ((x) <= 'z'))

/*
 * upalpha = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J" |
 *           "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" | "S" | "T" |
 *           "U" | "V" | "W" | "X" | "Y" | "Z"
 */
#define IS_UPALPHA(x) (((x) >= 'A') && ((x) <= 'Z'))

//#ifdef hw_IS_DIGIT
//#undef hw_IS_DIGIT
//#endif
/*
 * digit = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
 */
#define IS_DIGIT(x) (((x) >= '0') && ((x) <= '9'))

/*
 * alphanum = alpha | digit
 */

#define IS_ALPHANUM(x) (IS_ALPHA(x) || hw_IS_DIGIT(x))

/*
 * hex = digit | "A" | "B" | "C" | "D" | "E" | "F" |
 *               "a" | "b" | "c" | "d" | "e" | "f"
 */

#define IS_HEX(x) ((hw_IS_DIGIT(x)) || (((x) >= 'a') && ((x) <= 'f')) || \
	    (((x) >= 'A') && ((x) <= 'F')))

/*
 * mark = "-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")"
 */

#define IS_MARK(x) (((x) == '-') || ((x) == '_') || ((x) == '.') ||	\
    ((x) == '!') || ((x) == '~') || ((x) == '*') || ((x) == '\'') ||	\
    ((x) == '(') || ((x) == ')'))


/*
 * reserved = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" | "$" | "," |
 * 	      "[" | "]"
 */

#define IS_RESERVED(x) (((x) == ';') || ((x) == '/') || ((x) == '?') ||	\
        ((x) == ':') || ((x) == '@') || ((x) == '&') || ((x) == '=') ||	\
	((x) == '+') || ((x) == '$') || ((x) == ',') || ((x) == '[') || \
	((x) == ']'))

/*
 * unreserved = alphanum | mark
 */

#define IS_UNRESERVED(x) (IS_ALPHANUM(x) || IS_MARK(x))

/*
 * escaped = "%" hex hex
 */

#define IS_ESCAPED(p) ((*(p) == '%') && (IS_HEX((p)[1])) &&		\
	    (IS_HEX((p)[2])))

/*
 * uric_no_slash = unreserved | escaped | ";" | "?" | ":" | "@" |
 *                        "&" | "=" | "+" | "$" | ","
 */
#define IS_URIC_NO_SLASH(p) ((IS_UNRESERVED(*(p))) || (IS_ESCAPED(p)) ||\
	        ((*(p) == ';')) || ((*(p) == '?')) || ((*(p) == ':')) ||\
	        ((*(p) == '@')) || ((*(p) == '&')) || ((*(p) == '=')) ||\
	        ((*(p) == '+')) || ((*(p) == '$')) || ((*(p) == ',')))

/*
 * pchar = unreserved | escaped | ":" | "@" | "&" | "=" | "+" | "$" | ","
 */
#define IS_PCHAR(p) ((IS_UNRESERVED(*(p))) || (IS_ESCAPED(p)) ||	\
	        ((*(p) == ':')) || ((*(p) == '@')) || ((*(p) == '&')) ||\
	        ((*(p) == '=')) || ((*(p) == '+')) || ((*(p) == '$')) ||\
	        ((*(p) == ',')))

/*
 * rel_segment   = 1*( unreserved | escaped |
 *                 ";" | "@" | "&" | "=" | "+" | "$" | "," )
 */

#define IS_SEGMENT(p) ((IS_UNRESERVED(*(p))) || (IS_ESCAPED(p)) ||	\
          ((*(p) == ';')) || ((*(p) == '@')) || ((*(p) == '&')) ||	\
	  ((*(p) == '=')) || ((*(p) == '+')) || ((*(p) == '$')) ||	\
	  ((*(p) == ',')))

/*
 * scheme = alpha *( alpha | digit | "+" | "-" | "." )
 */

#define IS_SCHEME(x) ((IS_ALPHA(x)) || (hw_IS_DIGIT(x)) ||			\
	              ((x) == '+') || ((x) == '-') || ((x) == '.'))

/*
 * reg_name = 1*( unreserved | escaped | "$" | "," |
 *                ";" | ":" | "@" | "&" | "=" | "+" )
 */

#define IS_REG_NAME(p) ((IS_UNRESERVED(*(p))) || (IS_ESCAPED(p)) ||	\
       ((*(p) == '$')) || ((*(p) == ',')) || ((*(p) == ';')) ||		\
       ((*(p) == ':')) || ((*(p) == '@')) || ((*(p) == '&')) ||		\
       ((*(p) == '=')) || ((*(p) == '+')))

/*
 * userinfo = *( unreserved | escaped | ";" | ":" | "&" | "=" |
 *                      "+" | "$" | "," )
 */
#define IS_USERINFO(p) ((IS_UNRESERVED(*(p))) || (IS_ESCAPED(p)) ||	\
       ((*(p) == ';')) || ((*(p) == ':')) || ((*(p) == '&')) ||		\
       ((*(p) == '=')) || ((*(p) == '+')) || ((*(p) == '$')) ||		\
       ((*(p) == ',')))

/*
 * uric = reserved | unreserved | escaped
 */

#define IS_URIC(p) ((IS_UNRESERVED(*(p))) || (IS_ESCAPED(p)) ||		\
	            (IS_RESERVED(*(p))))

/*                                                                              
* unwise = "{" | "}" | "|" | "\" | "^" | "`"
*/                                                                             

#define IS_UNWISE(p)                                                    \
      (((*(p) == '{')) || ((*(p) == '}')) || ((*(p) == '|')) ||         \
       ((*(p) == '\\')) || ((*(p) == '^')) || ((*(p) == '[')) ||        \
       ((*(p) == ']')) || ((*(p) == '`')))  

/*
 * Skip to next pointer char, handle escaped sequences
 */

#define NEXT(p) ((*p == '%')? p += 3 : p++)

/*
 * Productions from the spec.
 *
 *    authority     = server | reg_name
 *    reg_name      = 1*( unreserved | escaped | "$" | "," |
 *                        ";" | ":" | "@" | "&" | "=" | "+" )
 *
 * path          = [ abs_path | opaque_part ]
 */

#define STRNDUP(s, n) (char *) hw_xmlStrndup((const hw_xmlChar *)(s), (n))

/************************************************************************
 *									*
 *			Generic URI structure functions			*
 *									*
 ************************************************************************/

/**
 * hw_xmlCreateURI:
 *
 * Simply creates an empty hw_xmlURI
 *
 * Returns the new structure or NULL in case of error
 */
hw_xmlURIPtr
hw_xmlCreateURI(void) {
    hw_xmlURIPtr ret;

    ret = (hw_xmlURIPtr) hw_xmlMalloc(sizeof(hw_xmlURI));
    if (ret == NULL) {
	hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlCreateURI: out of memory\n");
	return(NULL);
    }
    memset(ret, 0, sizeof(hw_xmlURI));
    return(ret);
}

/**
 * hw_xmlSaveUri:
 * @uri:  pointer to an hw_xmlURI
 *
 * Save the URI as an escaped string
 *
 * Returns a new string (to be deallocated by caller)
 */
hw_xmlChar *
hw_xmlSaveUri(hw_xmlURIPtr uri) {
    hw_xmlChar *ret = NULL;
    const char *p;
    int len;
    int max;

    if (uri == NULL) return(NULL);


    max = 80;
    ret = (hw_xmlChar *) hw_xmlMallocAtomic((max + 1) * sizeof(hw_xmlChar));
    if (ret == NULL) {
	hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlSaveUri: out of memory\n");
	return(NULL);
    }
    len = 0;

    if (uri->scheme != NULL) {
	p = uri->scheme;
	while (*p != 0) {
	    if (len >= max) {
		max *= 2;
		ret = (hw_xmlChar *) hw_xmlRealloc(ret, (max + 1) * sizeof(hw_xmlChar));
		if (ret == NULL) {
		    hw_xmlGenericError(hw_xmlGenericErrorContext,
			    "hw_xmlSaveUri: out of memory\n");
		    return(NULL);
		}
	    }
	    ret[len++] = *p++;
	}
	if (len >= max) {
	    max *= 2;
	    ret = (hw_xmlChar *) hw_xmlRealloc(ret, (max + 1) * sizeof(hw_xmlChar));
	    if (ret == NULL) {
		hw_xmlGenericError(hw_xmlGenericErrorContext,
			"hw_xmlSaveUri: out of memory\n");
		return(NULL);
	    }
	}
	ret[len++] = ':';
    }
    if (uri->opaque != NULL) {
	p = uri->opaque;
	while (*p != 0) {
	    if (len + 3 >= max) {
		max *= 2;
		ret = (hw_xmlChar *) hw_xmlRealloc(ret, (max + 1) * sizeof(hw_xmlChar));
		if (ret == NULL) {
		    hw_xmlGenericError(hw_xmlGenericErrorContext,
			    "hw_xmlSaveUri: out of memory\n");
		    return(NULL);
		}
	    }
	    if (IS_RESERVED(*(p)) || IS_UNRESERVED(*(p)))
		ret[len++] = *p++;
	    else {
		int val = *(unsigned char *)p++;
		int hi = val / 0x10, lo = val % 0x10;
		ret[len++] = '%';
		ret[len++] = hi + (hi > 9? 'A'-10 : '0');
		ret[len++] = lo + (lo > 9? 'A'-10 : '0');
	    }
	}
    } else {
	if (uri->server != NULL) {
	    if (len + 3 >= max) {
		max *= 2;
		ret = (hw_xmlChar *) hw_xmlRealloc(ret, (max + 1) * sizeof(hw_xmlChar));
		if (ret == NULL) {
		    hw_xmlGenericError(hw_xmlGenericErrorContext,
			    "hw_xmlSaveUri: out of memory\n");
		    return(NULL);
		}
	    }
	    ret[len++] = '/';
	    ret[len++] = '/';
	    if (uri->user != NULL) {
		p = uri->user;
		while (*p != 0) {
		    if (len + 3 >= max) {
			max *= 2;
			ret = (hw_xmlChar *) hw_xmlRealloc(ret,
				(max + 1) * sizeof(hw_xmlChar));
			if (ret == NULL) {
			    hw_xmlGenericError(hw_xmlGenericErrorContext,
				    "hw_xmlSaveUri: out of memory\n");
			    return(NULL);
			}
		    }
		    if ((IS_UNRESERVED(*(p))) ||
			((*(p) == ';')) || ((*(p) == ':')) ||
			((*(p) == '&')) || ((*(p) == '=')) ||
			((*(p) == '+')) || ((*(p) == '$')) ||
			((*(p) == ',')))
			ret[len++] = *p++;
		    else {
			int val = *(unsigned char *)p++;
			int hi = val / 0x10, lo = val % 0x10;
			ret[len++] = '%';
			ret[len++] = hi + (hi > 9? 'A'-10 : '0');
			ret[len++] = lo + (lo > 9? 'A'-10 : '0');
		    }
		}
		if (len + 3 >= max) {
		    max *= 2;
		    ret = (hw_xmlChar *) hw_xmlRealloc(ret,
			    (max + 1) * sizeof(hw_xmlChar));
		    if (ret == NULL) {
			hw_xmlGenericError(hw_xmlGenericErrorContext,
				"hw_xmlSaveUri: out of memory\n");
			return(NULL);
		    }
		}
		ret[len++] = '@';
	    }
	    p = uri->server;
	    while (*p != 0) {
		if (len >= max) {
		    max *= 2;
		    ret = (hw_xmlChar *) hw_xmlRealloc(ret,
			    (max + 1) * sizeof(hw_xmlChar));
		    if (ret == NULL) {
			hw_xmlGenericError(hw_xmlGenericErrorContext,
				"hw_xmlSaveUri: out of memory\n");
			return(NULL);
		    }
		}
		ret[len++] = *p++;
	    }
	    if (uri->port > 0) {
		if (len + 10 >= max) {
		    max *= 2;
		    ret = (hw_xmlChar *) hw_xmlRealloc(ret,
			    (max + 1) * sizeof(hw_xmlChar));
		    if (ret == NULL) {
			hw_xmlGenericError(hw_xmlGenericErrorContext,
				"hw_xmlSaveUri: out of memory\n");
			return(NULL);
		    }
		}
		len += snprintf((char *) &ret[len], max - len, ":%d", uri->port);
	    }
	} else if (uri->authority != NULL) {
	    if (len + 3 >= max) {
		max *= 2;
		ret = (hw_xmlChar *) hw_xmlRealloc(ret,
			(max + 1) * sizeof(hw_xmlChar));
		if (ret == NULL) {
		    hw_xmlGenericError(hw_xmlGenericErrorContext,
			    "hw_xmlSaveUri: out of memory\n");
		    return(NULL);
		}
	    }
	    ret[len++] = '/';
	    ret[len++] = '/';
	    p = uri->authority;
	    while (*p != 0) {
		if (len + 3 >= max) {
		    max *= 2;
		    ret = (hw_xmlChar *) hw_xmlRealloc(ret,
			    (max + 1) * sizeof(hw_xmlChar));
		    if (ret == NULL) {
			hw_xmlGenericError(hw_xmlGenericErrorContext,
				"hw_xmlSaveUri: out of memory\n");
			return(NULL);
		    }
		}
		if ((IS_UNRESERVED(*(p))) ||
                    ((*(p) == '$')) || ((*(p) == ',')) || ((*(p) == ';')) ||
                    ((*(p) == ':')) || ((*(p) == '@')) || ((*(p) == '&')) ||
                    ((*(p) == '=')) || ((*(p) == '+')))
		    ret[len++] = *p++;
		else {
		    int val = *(unsigned char *)p++;
		    int hi = val / 0x10, lo = val % 0x10;
		    ret[len++] = '%';
		    ret[len++] = hi + (hi > 9? 'A'-10 : '0');
		    ret[len++] = lo + (lo > 9? 'A'-10 : '0');
		}
	    }
	} else if (uri->scheme != NULL) {
	    if (len + 3 >= max) {
		max *= 2;
		ret = (hw_xmlChar *) hw_xmlRealloc(ret,
			(max + 1) * sizeof(hw_xmlChar));
		if (ret == NULL) {
		    hw_xmlGenericError(hw_xmlGenericErrorContext,
			    "hw_xmlSaveUri: out of memory\n");
		    return(NULL);
		}
	    }
	    ret[len++] = '/';
	    ret[len++] = '/';
	}
	if (uri->path != NULL) {
	    p = uri->path;
	    while (*p != 0) {
		if (len + 3 >= max) {
		    max *= 2;
		    ret = (hw_xmlChar *) hw_xmlRealloc(ret,
			    (max + 1) * sizeof(hw_xmlChar));
		    if (ret == NULL) {
			hw_xmlGenericError(hw_xmlGenericErrorContext,
				"hw_xmlSaveUri: out of memory\n");
			return(NULL);
		    }
		}
		if ((IS_UNRESERVED(*(p))) || ((*(p) == '/')) ||
                    ((*(p) == ';')) || ((*(p) == '@')) || ((*(p) == '&')) ||
	            ((*(p) == '=')) || ((*(p) == '+')) || ((*(p) == '$')) ||
	            ((*(p) == ',')))
		    ret[len++] = *p++;
		else {
		    int val = *(unsigned char *)p++;
		    int hi = val / 0x10, lo = val % 0x10;
		    ret[len++] = '%';
		    ret[len++] = hi + (hi > 9? 'A'-10 : '0');
		    ret[len++] = lo + (lo > 9? 'A'-10 : '0');
		}
	    }
	}
	if (uri->query != NULL) {
	    if (len + 3 >= max) {
		max *= 2;
		ret = (hw_xmlChar *) hw_xmlRealloc(ret,
			(max + 1) * sizeof(hw_xmlChar));
		if (ret == NULL) {
		    hw_xmlGenericError(hw_xmlGenericErrorContext,
			    "hw_xmlSaveUri: out of memory\n");
		    return(NULL);
		}
	    }
	    ret[len++] = '?';
	    p = uri->query;
	    while (*p != 0) {
		if (len + 3 >= max) {
		    max *= 2;
		    ret = (hw_xmlChar *) hw_xmlRealloc(ret,
			    (max + 1) * sizeof(hw_xmlChar));
		    if (ret == NULL) {
			hw_xmlGenericError(hw_xmlGenericErrorContext,
				"hw_xmlSaveUri: out of memory\n");
			return(NULL);
		    }
		}
		if ((IS_UNRESERVED(*(p))) || (IS_RESERVED(*(p)))) 
		    ret[len++] = *p++;
		else {
		    int val = *(unsigned char *)p++;
		    int hi = val / 0x10, lo = val % 0x10;
		    ret[len++] = '%';
		    ret[len++] = hi + (hi > 9? 'A'-10 : '0');
		    ret[len++] = lo + (lo > 9? 'A'-10 : '0');
		}
	    }
	}
    }
    if (uri->fragment != NULL) {
	if (len + 3 >= max) {
	    max *= 2;
	    ret = (hw_xmlChar *) hw_xmlRealloc(ret,
		    (max + 1) * sizeof(hw_xmlChar));
	    if (ret == NULL) {
		hw_xmlGenericError(hw_xmlGenericErrorContext,
			"hw_xmlSaveUri: out of memory\n");
		return(NULL);
	    }
	}
	ret[len++] = '#';
	p = uri->fragment;
	while (*p != 0) {
	    if (len + 3 >= max) {
		max *= 2;
		ret = (hw_xmlChar *) hw_xmlRealloc(ret,
			(max + 1) * sizeof(hw_xmlChar));
		if (ret == NULL) {
		    hw_xmlGenericError(hw_xmlGenericErrorContext,
			    "hw_xmlSaveUri: out of memory\n");
		    return(NULL);
		}
	    }
	    if ((IS_UNRESERVED(*(p))) || (IS_RESERVED(*(p)))) 
		ret[len++] = *p++;
	    else {
		int val = *(unsigned char *)p++;
		int hi = val / 0x10, lo = val % 0x10;
		ret[len++] = '%';
		ret[len++] = hi + (hi > 9? 'A'-10 : '0');
		ret[len++] = lo + (lo > 9? 'A'-10 : '0');
	    }
	}
    }
    if (len >= max) {
	max *= 2;
	ret = (hw_xmlChar *) hw_xmlRealloc(ret, (max + 1) * sizeof(hw_xmlChar));
	if (ret == NULL) {
	    hw_xmlGenericError(hw_xmlGenericErrorContext,
		    "hw_xmlSaveUri: out of memory\n");
	    return(NULL);
	}
    }
    ret[len++] = 0;
    return(ret);
}

/**
 * xmlCleanURI:
 * @uri:  pointer to an hw_xmlURI
 *
 * Make sure the hw_xmlURI struct is free of content
 */
static void
xmlCleanURI(hw_xmlURIPtr uri) {
    if (uri == NULL) return;

    if (uri->scheme != NULL) hw_xmlFree(uri->scheme);
    uri->scheme = NULL;
    if (uri->server != NULL) hw_xmlFree(uri->server);
    uri->server = NULL;
    if (uri->user != NULL) hw_xmlFree(uri->user);
    uri->user = NULL;
    if (uri->path != NULL) hw_xmlFree(uri->path);
    uri->path = NULL;
    if (uri->fragment != NULL) hw_xmlFree(uri->fragment);
    uri->fragment = NULL;
    if (uri->opaque != NULL) hw_xmlFree(uri->opaque);
    uri->opaque = NULL;
    if (uri->authority != NULL) hw_xmlFree(uri->authority);
    uri->authority = NULL;
    if (uri->query != NULL) hw_xmlFree(uri->query);
    uri->query = NULL;
}

/**
 * hw_xmlFreeURI:
 * @uri:  pointer to an hw_xmlURI
 *
 * Free up the hw_xmlURI struct
 */
void
hw_xmlFreeURI(hw_xmlURIPtr uri) {
    if (uri == NULL) return;

    if (uri->scheme != NULL) hw_xmlFree(uri->scheme);
    if (uri->server != NULL) hw_xmlFree(uri->server);
    if (uri->user != NULL) hw_xmlFree(uri->user);
    if (uri->path != NULL) hw_xmlFree(uri->path);
    if (uri->fragment != NULL) hw_xmlFree(uri->fragment);
    if (uri->opaque != NULL) hw_xmlFree(uri->opaque);
    if (uri->authority != NULL) hw_xmlFree(uri->authority);
    if (uri->query != NULL) hw_xmlFree(uri->query);
    hw_xmlFree(uri);
}

/************************************************************************
 *									*
 *			Helper functions				*
 *									*
 ************************************************************************/

/**
 * hw_xmlNormalizeURIPath:
 * @path:  pointer to the path string
 *
 * Applies the 5 normalization steps to a path string--that is, RFC 2396
 * Section 5.2, steps 6.c through 6.g.
 *
 * Normalization occurs directly on the string, no new allocation is done
 *
 * Returns 0 or an error code
 */
int
hw_xmlNormalizeURIPath(char *path) {
    char *cur, *out;

    if (path == NULL)
	return(-1);

    /* Skip all initial "/" chars.  We want to get to the beginning of the
     * first non-empty segment.
     */
    cur = path;
    while (cur[0] == '/')
      ++cur;
    if (cur[0] == '\0')
      return(0);

    /* Keep everything we've seen so far.  */
    out = cur;

    /*
     * Analyze each segment in sequence for cases (c) and (d).
     */
    while (cur[0] != '\0') {
	/*
	 * c) All occurrences of "./", where "." is a complete path segment,
	 *    are removed from the buffer string.
	 */
	if ((cur[0] == '.') && (cur[1] == '/')) {
	    cur += 2;
	    /* '//' normalization should be done at this point too */
	    while (cur[0] == '/')
		cur++;
	    continue;
	}

	/*
	 * d) If the buffer string ends with "." as a complete path segment,
	 *    that "." is removed.
	 */
	if ((cur[0] == '.') && (cur[1] == '\0'))
	    break;

	/* Otherwise keep the segment.  */
	while (cur[0] != '/') {
            if (cur[0] == '\0')
              goto done_cd;
	    (out++)[0] = (cur++)[0];
	}
	/* nomalize // */
	while ((cur[0] == '/') && (cur[1] == '/'))
	    cur++;

        (out++)[0] = (cur++)[0];
    }
 done_cd:
    out[0] = '\0';

    /* Reset to the beginning of the first segment for the next sequence.  */
    cur = path;
    while (cur[0] == '/')
      ++cur;
    if (cur[0] == '\0')
	return(0);

    /*
     * Analyze each segment in sequence for cases (e) and (f).
     *
     * e) All occurrences of "<segment>/../", where <segment> is a
     *    complete path segment not equal to "..", are removed from the
     *    buffer string.  Removal of these path segments is performed
     *    iteratively, removing the leftmost matching pattern on each
     *    iteration, until no matching pattern remains.
     *
     * f) If the buffer string ends with "<segment>/..", where <segment>
     *    is a complete path segment not equal to "..", that
     *    "<segment>/.." is removed.
     *
     * To satisfy the "iterative" clause in (e), we need to collapse the
     * string every time we find something that needs to be removed.  Thus,
     * we don't need to keep two pointers into the string: we only need a
     * "current position" pointer.
     */
    while (1) {
        char *segp, *tmp;

        /* At the beginning of each iteration of this loop, "cur" points to
         * the first character of the segment we want to examine.
         */

        /* Find the end of the current segment.  */
        segp = cur;
        while ((segp[0] != '/') && (segp[0] != '\0'))
          ++segp;

        /* If this is the last segment, we're done (we need at least two
         * segments to meet the criteria for the (e) and (f) cases).
         */
        if (segp[0] == '\0')
          break;

        /* If the first segment is "..", or if the next segment _isn't_ "..",
         * keep this segment and try the next one.
         */
        ++segp;
        if (((cur[0] == '.') && (cur[1] == '.') && (segp == cur+3))
            || ((segp[0] != '.') || (segp[1] != '.')
                || ((segp[2] != '/') && (segp[2] != '\0')))) {
          cur = segp;
          continue;
        }

        /* If we get here, remove this segment and the next one and back up
         * to the previous segment (if there is one), to implement the
         * "iteratively" clause.  It's pretty much impossible to back up
         * while maintaining two pointers into the buffer, so just compact
         * the whole buffer now.
         */

        /* If this is the end of the buffer, we're done.  */
        if (segp[2] == '\0') {
          cur[0] = '\0';
          break;
        }
        /* Valgrind complained, strcpy(cur, segp + 3); */
	/* string will overlap, do not use strcpy */
	tmp = cur;
	segp += 3;
	while ((*tmp++ = *segp++) != 0);

        /* If there are no previous segments, then keep going from here.  */
        segp = cur;
        while ((segp > path) && ((--segp)[0] == '/'))
          ;
        if (segp == path)
          continue;

        /* "segp" is pointing to the end of a previous segment; find it's
         * start.  We need to back up to the previous segment and start
         * over with that to handle things like "foo/bar/../..".  If we
         * don't do this, then on the first pass we'll remove the "bar/..",
         * but be pointing at the second ".." so we won't realize we can also
         * remove the "foo/..".
         */
        cur = segp;
        while ((cur > path) && (cur[-1] != '/'))
          --cur;
    }
    out[0] = '\0';

    /*
     * g) If the resulting buffer string still begins with one or more
     *    complete path segments of "..", then the reference is
     *    considered to be in error. Implementations may handle this
     *    error by retaining these components in the resolved path (i.e.,
     *    treating them as part of the final URI), by removing them from
     *    the resolved path (i.e., discarding relative levels above the
     *    root), or by avoiding traversal of the reference.
     *
     * We discard them from the final path.
     */
    if (path[0] == '/') {
      cur = path;
      while ((cur[0] == '/') && (cur[1] == '.') && (cur[2] == '.')
             && ((cur[3] == '/') || (cur[3] == '\0')))
	cur += 3;

      if (cur != path) {
	out = path;
	while (cur[0] != '\0')
          (out++)[0] = (cur++)[0];
	out[0] = 0;
      }
    }

    return(0);
}

static int is_hex(char c) {
    if (((c >= '0') && (c <= '9')) ||
        ((c >= 'a') && (c <= 'f')) ||
        ((c >= 'A') && (c <= 'F')))
	return(1);
    return(0);
}

/**
 * hw_xmlURIUnescapeString:
 * @str:  the string to unescape
 * @len:   the length in bytes to unescape (or <= 0 to indicate full string)
 * @target:  optional destination buffer
 *
 * Unescaping routine, does not do validity checks !
 * Output is direct unsigned char translation of %XX values (no encoding)
 *
 * Returns an copy of the string, but unescaped
 */
char *
hw_xmlURIUnescapeString(const char *str, int len, char *target) {
    char *ret, *out;
    const char *in;

    if (str == NULL)
	return(NULL);
    if (len <= 0) len = strlen(str);
    if (len < 0) return(NULL);

    if (target == NULL) {
	ret = (char *) hw_xmlMallocAtomic(len + 1);
	if (ret == NULL) {
	    hw_xmlGenericError(hw_xmlGenericErrorContext,
		    "hw_xmlURIUnescapeString: out of memory\n");
	    return(NULL);
	}
    } else
	ret = target;
    in = str;
    out = ret;
    while(len > 0) {
	if ((len > 2) && (*in == '%') && (is_hex(in[1])) && (is_hex(in[2]))) {
	    in++;
	    if ((*in >= '0') && (*in <= '9')) 
	        *out = (*in - '0');
	    else if ((*in >= 'a') && (*in <= 'f'))
	        *out = (*in - 'a') + 10;
	    else if ((*in >= 'A') && (*in <= 'F'))
	        *out = (*in - 'A') + 10;
	    in++;
	    if ((*in >= '0') && (*in <= '9')) 
	        *out = *out * 16 + (*in - '0');
	    else if ((*in >= 'a') && (*in <= 'f'))
	        *out = *out * 16 + (*in - 'a') + 10;
	    else if ((*in >= 'A') && (*in <= 'F'))
	        *out = *out * 16 + (*in - 'A') + 10;
	    in++;
	    len -= 3;
	    out++;
	} else {
	    *out++ = *in++;
	    len--;
	}
    }
    *out = 0;
    return(ret);
}

/**
 * hw_xmlURIEscapeStr:
 * @str:  string to escape
 * @list: exception list string of chars not to escape
 *
 * This routine escapes a string to hex, ignoring reserved characters (a-z)
 * and the characters in the exception list.
 *
 * Returns a new escaped string or NULL in case of error.
 */
hw_xmlChar *
hw_xmlURIEscapeStr(const hw_xmlChar *str, const hw_xmlChar *list) {
    hw_xmlChar *ret, ch;
    const hw_xmlChar *in;

    unsigned int len, out;

    if (str == NULL)
	return(NULL);
    if (str[0] == 0)
	return(hw_xmlStrdup(str));
    len = hw_xmlStrlen(str);
    if (!(len > 0)) return(NULL);

    len += 20;
    ret = (hw_xmlChar *) hw_xmlMallocAtomic(len);
    if (ret == NULL) {
	hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlURIEscapeStr: out of memory\n");
	return(NULL);
    }
    in = (const hw_xmlChar *) str;
    out = 0;
    while(*in != 0) {
	if (len - out <= 3) {
	    len += 20;
	    ret = (hw_xmlChar *) hw_xmlRealloc(ret, len);
	    if (ret == NULL) {
		hw_xmlGenericError(hw_xmlGenericErrorContext,
			"hw_xmlURIEscapeStr: out of memory\n");
		return(NULL);
	    }
	}

	ch = *in;

	if ((ch != '@') && (!IS_UNRESERVED(ch)) && (!hw_xmlStrchr(list, ch))) {
	    unsigned char val;
	    ret[out++] = '%';
	    val = ch >> 4;
	    if (val <= 9)
		ret[out++] = '0' + val;
	    else
		ret[out++] = 'A' + val - 0xA;
	    val = ch & 0xF;
	    if (val <= 9)
		ret[out++] = '0' + val;
	    else
		ret[out++] = 'A' + val - 0xA;
	    in++;
	} else {
	    ret[out++] = *in++;
	}

    }
    ret[out] = 0;
    return(ret);
}


/************************************************************************
 *									*
 *			Escaped URI parsing				*
 *									*
 ************************************************************************/

/**
 * xmlParseURIFragment:
 * @uri:  pointer to an URI structure
 * @str:  pointer to the string to analyze
 *
 * Parse an URI fragment string and fills in the appropriate fields
 * of the @uri structure.
 * 
 * fragment = *uric
 *
 * Returns 0 or the error code
 */
static int
xmlParseURIFragment(hw_xmlURIPtr uri, const char **str)
{
    const char *cur;
    
    if (str == NULL)
        return (-1);

    cur = *str;

    while (IS_URIC(cur) || IS_UNWISE(cur))
        NEXT(cur);
    if (uri != NULL) {
        if (uri->fragment != NULL)
            hw_xmlFree(uri->fragment);
	if (uri->cleanup & 2)
	    uri->fragment = STRNDUP(*str, cur - *str);
	else
	    uri->fragment = hw_xmlURIUnescapeString(*str, cur - *str, NULL);
    }
    *str = cur;
    return (0);
}

/**
 * xmlParseURIQuery:
 * @uri:  pointer to an URI structure
 * @str:  pointer to the string to analyze
 *
 * Parse the query part of an URI
 * 
 * query = *uric
 *
 * Returns 0 or the error code
 */
static int
xmlParseURIQuery(hw_xmlURIPtr uri, const char **str)
{
    const char *cur;

    if (str == NULL)
        return (-1);

    cur = *str;

    while ((IS_URIC(cur)) ||
           ((uri != NULL) && (uri->cleanup & 1) && (IS_UNWISE(cur))))
        NEXT(cur);
    if (uri != NULL) {
        if (uri->query != NULL)
            hw_xmlFree(uri->query);
	if (uri->cleanup & 2)
	    uri->query = STRNDUP(*str, cur - *str);
	else
	    uri->query = hw_xmlURIUnescapeString(*str, cur - *str, NULL);
    }
    *str = cur;
    return (0);
}

/**
 * xmlParseURIScheme:
 * @uri:  pointer to an URI structure
 * @str:  pointer to the string to analyze
 *
 * Parse an URI scheme
 * 
 * scheme = alpha *( alpha | digit | "+" | "-" | "." )
 *
 * Returns 0 or the error code
 */
static int
xmlParseURIScheme(hw_xmlURIPtr uri, const char **str) {
    const char *cur;

    if (str == NULL)
	return(-1);
    
    cur = *str;
    if (!IS_ALPHA(*cur))
	return(2);
    cur++;
    while (IS_SCHEME(*cur)) cur++;
    if (uri != NULL) {
	if (uri->scheme != NULL) hw_xmlFree(uri->scheme);
	uri->scheme = STRNDUP(*str, cur - *str);
    }
    *str = cur;
    return(0);
}

/**
 * xmlParseURIOpaquePart:
 * @uri:  pointer to an URI structure
 * @str:  pointer to the string to analyze
 *
 * Parse an URI opaque part
 * 
 * opaque_part = uric_no_slash *uric
 *
 * Returns 0 or the error code
 */
static int
xmlParseURIOpaquePart(hw_xmlURIPtr uri, const char **str)
{
    const char *cur;

    if (str == NULL)
        return (-1);

    cur = *str;
    if (!((IS_URIC_NO_SLASH(cur)) ||
          ((uri != NULL) && (uri->cleanup & 1) && (IS_UNWISE(cur))))) {
        return (3);
    }
    NEXT(cur);
    while ((IS_URIC(cur)) ||
           ((uri != NULL) && (uri->cleanup & 1) && (IS_UNWISE(cur))))
        NEXT(cur);
    if (uri != NULL) {
        if (uri->opaque != NULL)
            hw_xmlFree(uri->opaque);
	if (uri->cleanup & 2)
	    uri->opaque = STRNDUP(*str, cur - *str);
	else
	    uri->opaque = hw_xmlURIUnescapeString(*str, cur - *str, NULL);
    }
    *str = cur;
    return (0);
}

/**
 * xmlParseURIServer:
 * @uri:  pointer to an URI structure
 * @str:  pointer to the string to analyze
 *
 * Parse a server subpart of an URI, it's a finer grain analysis
 * of the authority part.
 * 
 * server        = [ [ userinfo "@" ] hostport ]
 * userinfo      = *( unreserved | escaped |
 *                       ";" | ":" | "&" | "=" | "+" | "$" | "," )
 * hostport      = host [ ":" port ]
 * host          = hostname | IPv4address | IPv6reference
 * hostname      = *( domainlabel "." ) toplabel [ "." ]
 * domainlabel   = alphanum | alphanum *( alphanum | "-" ) alphanum
 * toplabel      = alpha | alpha *( alphanum | "-" ) alphanum
 * IPv6reference = "[" IPv6address "]"
 * IPv6address   = hexpart [ ":" IPv4address ]
 * IPv4address   = 1*3digit "." 1*3digit "." 1*3digit "." 1*3digit
 * hexpart       = hexseq | hexseq "::" [ hexseq ]| "::" [ hexseq ]
 * hexseq        = hex4 *( ":" hex4)
 * hex4          = 1*4hexdig
 * port          = *digit
 *
 * Returns 0 or the error code
 */
static int
xmlParseURIServer(hw_xmlURIPtr uri, const char **str) {
    const char *cur;
    const char *host, *tmp;
    const int IPV4max = 4;
    const int IPV6max = 8;
    int oct;

    if (str == NULL)
	return(-1);
    
    cur = *str;

    /*
     * is there a userinfo ?
     */
    while (IS_USERINFO(cur)) NEXT(cur);
    if (*cur == '@') {
	if (uri != NULL) {
	    if (uri->user != NULL) hw_xmlFree(uri->user);
	    if (uri->cleanup & 2)
		uri->path = STRNDUP(*str, cur - *str);
	    else
		uri->user = hw_xmlURIUnescapeString(*str, cur - *str, NULL);
	}
	cur++;
    } else {
	if (uri != NULL) {
	    if (uri->user != NULL) hw_xmlFree(uri->user);
	    uri->user = NULL;
	}
        cur = *str;
    }
    /*
     * This can be empty in the case where there is no server
     */
    host = cur;
    if (*cur == '/') {
	if (uri != NULL) {
	    if (uri->authority != NULL) hw_xmlFree(uri->authority);
	    uri->authority = NULL;
	    if (uri->server != NULL) hw_xmlFree(uri->server);
	    uri->server = NULL;
	    uri->port = 0;
	}
	return(0);
    }
    /*
     * host part of hostport can denote an IPV4 address, an IPV6 address
     * or an unresolved name. Check the IP first, its easier to detect
     * errors if wrong one.
     * An IPV6 address must start with a '[' and end with a ']'.
     */
    if (*cur == '[') {
	int compress=0;
	cur++;
	for (oct = 0; oct < IPV6max; ++oct) {
	    if (*cur == ':') {
		if (compress)
		    return(3);	/* multiple compression attempted */
		if (!oct) { 	/* initial char is compression */
		    if (*++cur != ':')
			return(3);
		}
		compress = 1;	/* set compression-encountered flag */
		cur++;		/* skip over the second ':' */
		continue;
	    }
	    while(IS_HEX(*cur)) cur++;
	    if (oct == (IPV6max-1))
		continue;
	    if (*cur != ':')
		break;
	    cur++;
	}
	if ((!compress) && (oct != IPV6max))
	    return(3);
	if (*cur != ']')
	    return(3);
	if (uri != NULL) {
	    if (uri->server != NULL) hw_xmlFree(uri->server);
	    uri->server = (char *)hw_xmlStrndup((hw_xmlChar *)host+1,
			(cur-host)-1);
	}
	cur++;
    } else {
	/*
	 * Not IPV6, maybe IPV4
	 */
	for (oct = 0; oct < IPV4max; ++oct) {
            if (*cur == '.') 
                return(3); /* e.g. http://.xml/ or http://18.29..30/ */
            while(hw_IS_DIGIT(*cur)) cur++;
            if (oct == (IPV4max-1))
                continue;
            if (*cur != '.')
	        break;
            cur++;
	}
    }
    if ((host[0] != '[') && (oct < IPV4max || (*cur == '.' && cur++) ||
			     IS_ALPHA(*cur))) {
        /* maybe host_name */
        if (!IS_ALPHANUM(*cur))
            return(4); /* e.g. http://xml.$oft */
        do {
            do ++cur; while (IS_ALPHANUM(*cur));
            if (*cur == '-') {
	        --cur;
                if (*cur == '.')
                    return(5); /* e.g. http://xml.-soft */
	        ++cur;
		continue;
            }
    	    if (*cur == '.') {
	        --cur;
                if (*cur == '-')
                    return(6); /* e.g. http://xml-.soft */
                if (*cur == '.')
                    return(7); /* e.g. http://xml..soft */
	        ++cur;
		continue;
            }
	    break;
        } while (1);
        tmp = cur;
        if (tmp[-1] == '.')
            --tmp; /* e.g. http://xml.$Oft/ */
        do --tmp; while (tmp >= host && IS_ALPHANUM(*tmp));
        if ((++tmp == host || tmp[-1] == '.') && !IS_ALPHA(*tmp))
            return(8); /* e.g. http://xmlsOft.0rg/ */
    }
    if (uri != NULL) {
	if (uri->authority != NULL) hw_xmlFree(uri->authority);
	uri->authority = NULL;
	if (host[0] != '[') {	/* it's not an IPV6 addr */
	    if (uri->server != NULL) hw_xmlFree(uri->server);
	    if (uri->cleanup & 2)
		uri->server = STRNDUP(host, cur - host);
	    else
		uri->server = hw_xmlURIUnescapeString(host, cur - host, NULL);
	}
    }
    /*
     * finish by checking for a port presence.
     */
    if (*cur == ':') {
        cur++;
	if (hw_IS_DIGIT(*cur)) {
	    if (uri != NULL)
	        uri->port = 0;
	    while (hw_IS_DIGIT(*cur)) {
	        if (uri != NULL)
		    uri->port = uri->port * 10 + (*cur - '0');
		cur++;
	    }
	}
    }
    *str = cur;
    return(0);
}	

/**
 * xmlParseURIRelSegment:
 * @uri:  pointer to an URI structure
 * @str:  pointer to the string to analyze
 *
 * Parse an URI relative segment
 * 
 * rel_segment = 1*( unreserved | escaped | ";" | "@" | "&" | "=" |
 *                          "+" | "$" | "," )
 *
 * Returns 0 or the error code
 */
static int
xmlParseURIRelSegment(hw_xmlURIPtr uri, const char **str)
{
    const char *cur;

    if (str == NULL)
        return (-1);

    cur = *str;
    if (!((IS_SEGMENT(cur)) ||
          ((uri != NULL) && (uri->cleanup & 1) && (IS_UNWISE(cur))))) {
        return (3);
    }
    NEXT(cur);
    while ((IS_SEGMENT(cur)) ||
           ((uri != NULL) && (uri->cleanup & 1) && (IS_UNWISE(cur))))
        NEXT(cur);
    if (uri != NULL) {
        if (uri->path != NULL)
            hw_xmlFree(uri->path);
	if (uri->cleanup & 2)
	    uri->path = STRNDUP(*str, cur - *str);
	else
	    uri->path = hw_xmlURIUnescapeString(*str, cur - *str, NULL);
    }
    *str = cur;
    return (0);
}

/**
 * xmlParseURIPathSegments:
 * @uri:  pointer to an URI structure
 * @str:  pointer to the string to analyze
 * @slash:  should we add a leading slash
 *
 * Parse an URI set of path segments
 * 
 * path_segments = segment *( "/" segment )
 * segment       = *pchar *( ";" param )
 * param         = *pchar
 *
 * Returns 0 or the error code
 */
static int
xmlParseURIPathSegments(hw_xmlURIPtr uri, const char **str, int slash)
{
    const char *cur;

    if (str == NULL)
        return (-1);

    cur = *str;

    do {
        while ((IS_PCHAR(cur)) ||
	       ((uri != NULL) && (uri->cleanup & 1) && (IS_UNWISE(cur))))
            NEXT(cur);
        while (*cur == ';') {
            cur++;
            while ((IS_PCHAR(cur)) ||
	           ((uri != NULL) && (uri->cleanup & 1) && (IS_UNWISE(cur))))
                NEXT(cur);
        }
        if (*cur != '/')
            break;
        cur++;
    } while (1);
    if (uri != NULL) {
        int len, len2 = 0;
        char *path;

        /*
         * Concat the set of path segments to the current path
         */
        len = cur - *str;
        if (slash)
            len++;

        if (uri->path != NULL) {
            len2 = strlen(uri->path);
            len += len2;
        }
        path = (char *) hw_xmlMallocAtomic(len + 1);
        if (path == NULL) {
	    hw_xmlGenericError(hw_xmlGenericErrorContext,
	    		    "xmlParseURIPathSegments: out of memory\n");
            *str = cur;
            return (-1);
        }
        if (uri->path != NULL)
            memcpy(path, uri->path, len2);
        if (slash) {
            path[len2] = '/';
            len2++;
        }
        path[len2] = 0;
        if (cur - *str > 0) {
	    if (uri->cleanup & 2) {
	        memcpy(&path[len2], *str, cur - *str);
		path[len2 + (cur - *str)] = 0;
	    } else
		hw_xmlURIUnescapeString(*str, cur - *str, &path[len2]);
	}
        if (uri->path != NULL)
            hw_xmlFree(uri->path);
        uri->path = path;
    }
    *str = cur;
    return (0);
}

/**
 * xmlParseURIAuthority:
 * @uri:  pointer to an URI structure
 * @str:  pointer to the string to analyze
 *
 * Parse the authority part of an URI.
 * 
 * authority = server | reg_name
 * server    = [ [ userinfo "@" ] hostport ]
 * reg_name  = 1*( unreserved | escaped | "$" | "," | ";" | ":" |
 *                        "@" | "&" | "=" | "+" )
 *
 * Note : this is completely ambiguous since reg_name is allowed to
 *        use the full set of chars in use by server:
 *
 *        3.2.1. Registry-based Naming Authority
 *
 *        The structure of a registry-based naming authority is specific
 *        to the URI scheme, but constrained to the allowed characters
 *        for an authority component.
 *
 * Returns 0 or the error code
 */
static int
xmlParseURIAuthority(hw_xmlURIPtr uri, const char **str) {
    const char *cur;
    int ret;

    if (str == NULL)
	return(-1);
    
    cur = *str;

    /*
     * try first to parse it as a server string.
     */
    ret = xmlParseURIServer(uri, str);
    if ((ret == 0) && (*str != NULL) &&
	((**str == 0) || (**str == '/') || (**str == '?')))
        return(0);
    *str = cur;

    /*
     * failed, fallback to reg_name
     */
    if (!IS_REG_NAME(cur)) {
	return(5);
    }
    NEXT(cur);
    while (IS_REG_NAME(cur)) NEXT(cur);
    if (uri != NULL) {
	if (uri->server != NULL) hw_xmlFree(uri->server);
	uri->server = NULL;
	if (uri->user != NULL) hw_xmlFree(uri->user);
	uri->user = NULL;
	if (uri->authority != NULL) hw_xmlFree(uri->authority);
	if (uri->cleanup & 2)
	    uri->authority = STRNDUP(*str, cur - *str);
	else
	    uri->authority = hw_xmlURIUnescapeString(*str, cur - *str, NULL);
    }
    *str = cur;
    return(0);
}

/**
 * xmlParseURIHierPart:
 * @uri:  pointer to an URI structure
 * @str:  pointer to the string to analyze
 *
 * Parse an URI hierarchical part
 * 
 * hier_part = ( net_path | abs_path ) [ "?" query ]
 * abs_path = "/"  path_segments
 * net_path = "//" authority [ abs_path ]
 *
 * Returns 0 or the error code
 */
static int
xmlParseURIHierPart(hw_xmlURIPtr uri, const char **str) {
    int ret;
    const char *cur;

    if (str == NULL)
	return(-1);
    
    cur = *str;

    if ((cur[0] == '/') && (cur[1] == '/')) {
	cur += 2;
	ret = xmlParseURIAuthority(uri, &cur);
	if (ret != 0)
	    return(ret);
	if (cur[0] == '/') {
	    cur++;
	    ret = xmlParseURIPathSegments(uri, &cur, 1);
	}
    } else if (cur[0] == '/') {
	cur++;
	ret = xmlParseURIPathSegments(uri, &cur, 1);
    } else {
	return(4);
    }
    if (ret != 0)
	return(ret);
    if (*cur == '?') {
	cur++;
	ret = xmlParseURIQuery(uri, &cur);
	if (ret != 0)
	    return(ret);
    }
    *str = cur;
    return(0);
}

/**
 * xmlParseAbsoluteURI:
 * @uri:  pointer to an URI structure
 * @str:  pointer to the string to analyze
 *
 * Parse an URI reference string and fills in the appropriate fields
 * of the @uri structure
 * 
 * absoluteURI   = scheme ":" ( hier_part | opaque_part )
 *
 * Returns 0 or the error code
 */
static int
xmlParseAbsoluteURI(hw_xmlURIPtr uri, const char **str) {
    int ret;
    const char *cur;

    if (str == NULL)
	return(-1);
    
    cur = *str;

    ret = xmlParseURIScheme(uri, str);
    if (ret != 0) return(ret);
    if (**str != ':') {
	*str = cur;
	return(1);
    }
    (*str)++;
    if (**str == '/')
	return(xmlParseURIHierPart(uri, str));
    return(xmlParseURIOpaquePart(uri, str));
}

/**
 * xmlParseRelativeURI:
 * @uri:  pointer to an URI structure
 * @str:  pointer to the string to analyze
 *
 * Parse an relative URI string and fills in the appropriate fields
 * of the @uri structure
 * 
 * relativeURI = ( net_path | abs_path | rel_path ) [ "?" query ]
 * abs_path = "/"  path_segments
 * net_path = "//" authority [ abs_path ]
 * rel_path = rel_segment [ abs_path ]
 *
 * Returns 0 or the error code
 */
static int
xmlParseRelativeURI(hw_xmlURIPtr uri, const char **str) {
    int ret = 0;
    const char *cur;

    if (str == NULL)
	return(-1);
    
    cur = *str;
    if ((cur[0] == '/') && (cur[1] == '/')) {
	cur += 2;
	ret = xmlParseURIAuthority(uri, &cur);
	if (ret != 0)
	    return(ret);
	if (cur[0] == '/') {
	    cur++;
	    ret = xmlParseURIPathSegments(uri, &cur, 1);
	}
    } else if (cur[0] == '/') {
	cur++;
	ret = xmlParseURIPathSegments(uri, &cur, 1);
    } else if (cur[0] != '#' && cur[0] != '?') {
	ret = xmlParseURIRelSegment(uri, &cur);
	if (ret != 0)
	    return(ret);
	if (cur[0] == '/') {
	    cur++;
	    ret = xmlParseURIPathSegments(uri, &cur, 1);
	}
    }
    if (ret != 0)
	return(ret);
    if (*cur == '?') {
	cur++;
	ret = xmlParseURIQuery(uri, &cur);
	if (ret != 0)
	    return(ret);
    }
    *str = cur;
    return(ret);
}

/**
 * hw_xmlParseURIReference:
 * @uri:  pointer to an URI structure
 * @str:  the string to analyze
 *
 * Parse an URI reference string and fills in the appropriate fields
 * of the @uri structure
 * 
 * URI-reference = [ absoluteURI | relativeURI ] [ "#" fragment ]
 *
 * Returns 0 or the error code
 */
int
hw_xmlParseURIReference(hw_xmlURIPtr uri, const char *str) {
    int ret;
    const char *tmp = str;

    if (str == NULL)
	return(-1);
    xmlCleanURI(uri);

    /*
     * Try first to parse absolute refs, then fallback to relative if
     * it fails.
     */
    ret = xmlParseAbsoluteURI(uri, &str);
    if (ret != 0) {
	xmlCleanURI(uri);
	str = tmp;
        ret = xmlParseRelativeURI(uri, &str);
    }
    if (ret != 0) {
	xmlCleanURI(uri);
	return(ret);
    }

    if (*str == '#') {
	str++;
	ret = xmlParseURIFragment(uri, &str);
	if (ret != 0) return(ret);
    }
    if (*str != 0) {
	xmlCleanURI(uri);
	return(1);
    }
    return(0);
}

/**
 * hw_xmlParseURI:
 * @str:  the URI string to analyze
 *
 * Parse an URI 
 * 
 * URI-reference = [ absoluteURI | relativeURI ] [ "#" fragment ]
 *
 * Returns a newly built hw_xmlURIPtr or NULL in case of error
 */
hw_xmlURIPtr
hw_xmlParseURI(const char *str) {
    hw_xmlURIPtr uri;
    int ret;

    if (str == NULL)
	return(NULL);
    uri = hw_xmlCreateURI();
    if (uri != NULL) {
	ret = hw_xmlParseURIReference(uri, str);
        if (ret) {
	    hw_xmlFreeURI(uri);
	    return(NULL);
	}
    }
    return(uri);
}

/************************************************************************
 *									*
 *			Public functions				*
 *									*
 ************************************************************************/

/**
 * hw_xmlBuildURI:
 * @URI:  the URI instance found in the document
 * @base:  the base value
 *
 * Computes he final URI of the reference done by checking that
 * the given URI is valid, and building the final URI using the
 * base URI. This is processed according to section 5.2 of the 
 * RFC 2396
 *
 * 5.2. Resolving Relative References to Absolute Form
 *
 * Returns a new URI string (to be freed by the caller) or NULL in case
 *         of error.
 */
hw_xmlChar *
hw_xmlBuildURI(const hw_xmlChar *URI, const hw_xmlChar *base) {
    hw_xmlChar *val = NULL;
    int ret, len, indx, cur, out;
    hw_xmlURIPtr ref = NULL;
    hw_xmlURIPtr bas = NULL;
    hw_xmlURIPtr res = NULL;

    /*
     * 1) The URI reference is parsed into the potential four components and
     *    fragment identifier, as described in Section 4.3.
     *
     *    NOTE that a completely empty URI is treated by modern browsers
     *    as a reference to "." rather than as a synonym for the current
     *    URI.  Should we do that here?
     */
    if (URI == NULL) 
	ret = -1;
    else {
	if (*URI) {
	    ref = hw_xmlCreateURI();
	    if (ref == NULL)
		goto done;
	    ret = hw_xmlParseURIReference(ref, (const char *) URI);
	}
	else
	    ret = 0;
    }
    if (ret != 0)
	goto done;
    if ((ref != NULL) && (ref->scheme != NULL)) {
	/*
	 * The URI is absolute don't modify.
	 */
	val = hw_xmlStrdup(URI);
	goto done;
    }
    if (base == NULL)
	ret = -1;
    else {
	bas = hw_xmlCreateURI();
	if (bas == NULL)
	    goto done;
	ret = hw_xmlParseURIReference(bas, (const char *) base);
    }
    if (ret != 0) {
	if (ref)
	    val = hw_xmlSaveUri(ref);
	goto done;
    }
    if (ref == NULL) {
	/*
	 * the base fragment must be ignored
	 */
	if (bas->fragment != NULL) {
	    hw_xmlFree(bas->fragment);
	    bas->fragment = NULL;
	}
	val = hw_xmlSaveUri(bas);
	goto done;
    }

    /*
     * 2) If the path component is empty and the scheme, authority, and
     *    query components are undefined, then it is a reference to the
     *    current document and we are done.  Otherwise, the reference URI's
     *    query and fragment components are defined as found (or not found)
     *    within the URI reference and not inherited from the base URI.
     *
     *    NOTE that in modern browsers, the parsing differs from the above
     *    in the following aspect:  the query component is allowed to be
     *    defined while still treating this as a reference to the current
     *    document.
     */
    res = hw_xmlCreateURI();
    if (res == NULL)
	goto done;
    if ((ref->scheme == NULL) && (ref->path == NULL) &&
	((ref->authority == NULL) && (ref->server == NULL))) {
	if (bas->scheme != NULL)
	    res->scheme = (char*)hw_xmlMemStrdup((hw_xmlChar *)bas->scheme);
	if (bas->authority != NULL)
	    res->authority = (char*)hw_xmlMemStrdup((hw_xmlChar *)bas->authority);
	else if (bas->server != NULL) {
	    res->server = (char*)hw_xmlMemStrdup((hw_xmlChar *)bas->server);
	    if (bas->user != NULL)
		res->user = (char*)hw_xmlMemStrdup((hw_xmlChar *)bas->user);
	    res->port = bas->port;		
	}
	if (bas->path != NULL)
	    res->path = (char*)hw_xmlMemStrdup((hw_xmlChar *)bas->path);
	if (ref->query != NULL)
	    res->query = (char*)hw_xmlMemStrdup((hw_xmlChar *)ref->query);
	else if (bas->query != NULL)
	    res->query = (char*)hw_xmlMemStrdup((hw_xmlChar *)bas->query);
	if (ref->fragment != NULL)
	    res->fragment = (char*)hw_xmlMemStrdup((hw_xmlChar *)ref->fragment);
	goto step_7;
    }

    /*
     * 3) If the scheme component is defined, indicating that the reference
     *    starts with a scheme name, then the reference is interpreted as an
     *    absolute URI and we are done.  Otherwise, the reference URI's
     *    scheme is inherited from the base URI's scheme component.
     */
    if (ref->scheme != NULL) {
	val = hw_xmlSaveUri(ref);
	goto done;
    }
    if (bas->scheme != NULL)
	res->scheme = (char*)hw_xmlMemStrdup((hw_xmlChar *)bas->scheme);
 
    if (ref->query != NULL)
	res->query = (char*)hw_xmlMemStrdup((hw_xmlChar *)ref->query);
    if (ref->fragment != NULL)
	res->fragment = (char*)hw_xmlMemStrdup((hw_xmlChar *)ref->fragment);

    /*
     * 4) If the authority component is defined, then the reference is a
     *    network-path and we skip to step 7.  Otherwise, the reference
     *    URI's authority is inherited from the base URI's authority
     *    component, which will also be undefined if the URI scheme does not
     *    use an authority component.
     */
    if ((ref->authority != NULL) || (ref->server != NULL)) {
	if (ref->authority != NULL)
	    res->authority = (char*)hw_xmlMemStrdup((hw_xmlChar *)ref->authority);
	else {
	    res->server = (char*)hw_xmlMemStrdup((hw_xmlChar *)ref->server);
	    if (ref->user != NULL)
		res->user = (char*)hw_xmlMemStrdup((hw_xmlChar *)ref->user);
            res->port = ref->port;		
	}
	if (ref->path != NULL)
	    res->path = (char*)hw_xmlMemStrdup((hw_xmlChar *)ref->path);
	goto step_7;
    }
    if (bas->authority != NULL)
	res->authority = (char*)hw_xmlMemStrdup((hw_xmlChar *)bas->authority);
    else if (bas->server != NULL) {
	res->server = (char*)hw_xmlMemStrdup((hw_xmlChar *)bas->server);
	if (bas->user != NULL)
	    res->user = (char*)hw_xmlMemStrdup((hw_xmlChar *)bas->user);
	res->port = bas->port;		
    }

    /*
     * 5) If the path component begins with a slash character ("/"), then
     *    the reference is an absolute-path and we skip to step 7.
     */
    if ((ref->path != NULL) && (ref->path[0] == '/')) {
	res->path = (char*)hw_xmlMemStrdup((hw_xmlChar *)ref->path);
	goto step_7;
    }


    /*
     * 6) If this step is reached, then we are resolving a relative-path
     *    reference.  The relative path needs to be merged with the base
     *    URI's path.  Although there are many ways to do this, we will
     *    describe a simple method using a separate string buffer.
     *
     * Allocate a buffer large enough for the result string.
     */
    len = 2; /* extra / and 0 */
    if (ref->path != NULL)
	len += strlen(ref->path);
    if (bas->path != NULL)
	len += strlen(bas->path);
    res->path = (char *) hw_xmlMallocAtomic(len);
    if (res->path == NULL) {
	hw_xmlGenericError(hw_xmlGenericErrorContext,
		"hw_xmlBuildURI: out of memory\n");
	goto done;
    }
    res->path[0] = 0;

    /*
     * a) All but the last segment of the base URI's path component is
     *    copied to the buffer.  In other words, any characters after the
     *    last (right-most) slash character, if any, are excluded.
     */
    cur = 0;
    out = 0;
    if (bas->path != NULL) {
	while (bas->path[cur] != 0) {
	    while ((bas->path[cur] != 0) && (bas->path[cur] != '/'))
		cur++;
	    if (bas->path[cur] == 0)
		break;

	    cur++;
	    while (out < cur) {
		res->path[out] = bas->path[out];
		out++;
	    }
	}
    }
    res->path[out] = 0;

    /*
     * b) The reference's path component is appended to the buffer
     *    string.
     */
    if (ref->path != NULL && ref->path[0] != 0) {
	indx = 0;
	/*
	 * Ensure the path includes a '/'
	 */
	if ((out == 0) && (bas->server != NULL))
	    res->path[out++] = '/';
	while (ref->path[indx] != 0) {
	    res->path[out++] = ref->path[indx++];
	}
    }
    res->path[out] = 0;

    /*
     * Steps c) to h) are really path normalization steps
     */
    hw_xmlNormalizeURIPath(res->path);

step_7:

    /*
     * 7) The resulting URI components, including any inherited from the
     *    base URI, are recombined to give the absolute form of the URI
     *    reference.
     */
    val = hw_xmlSaveUri(res);

done:
    if (ref != NULL)
	hw_xmlFreeURI(ref);
    if (bas != NULL)
	hw_xmlFreeURI(bas);
    if (res != NULL)
	hw_xmlFreeURI(res);
    return(val);
}

/**
 * hw_xmlCanonicPath:
 * @path:  the resource locator in a filesystem notation
 *
 * Constructs a canonic path from the specified path. 
 *
 * Returns a new canonic path, or a duplicate of the path parameter if the 
 * construction fails. The caller is responsible for freeing the memory occupied
 * by the returned string. If there is insufficient memory available, or the 
 * argument is NULL, the function returns NULL.
 */
#define IS_WINDOWS_PATH(p) 					\
	((p != NULL) &&						\
	 (((p[0] >= 'a') && (p[0] <= 'z')) ||			\
	  ((p[0] >= 'A') && (p[0] <= 'Z'))) &&			\
	 (p[1] == ':') && ((p[2] == '/') || (p[2] == '\\')))
hw_xmlChar*
hw_xmlCanonicPath(const hw_xmlChar *path)
{
#if defined(_WIN32) && !defined(__CYGWIN__)    
    int len = 0;
    int i = 0;
    hw_xmlChar *p = NULL;
#endif
    hw_xmlURIPtr uri;
    hw_xmlChar *ret;
    const hw_xmlChar *absuri;

    if (path == NULL)
	return(NULL);
    if ((uri = hw_xmlParseURI((const char *) path)) != NULL) {
	hw_xmlFreeURI(uri);
	return hw_xmlStrdup(path);
    }

    absuri = hw_xmlStrstr(path, hw_BAD_CAST "://");
    if (absuri != NULL) {
        int l, j;
	unsigned char c;
	hw_xmlChar *escURI;

        /*
	 * this looks like an URI where some parts have not been
	 * escaped leading to a parsing problem check that the first
	 * part matches a protocol.
	 */
	l = absuri - path;
	if ((l <= 0) || (l > 20))
	    goto path_processing;
	for (j = 0;j < l;j++) {
	    c = path[j];
	    if (!(((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'))))
	        goto path_processing;
	}

        escURI = hw_xmlURIEscapeStr(path, hw_BAD_CAST ":/?_.#&;=");
	if (escURI != NULL) {
	    uri = hw_xmlParseURI((const char *) escURI);
	    if (uri != NULL) {
	        hw_xmlFreeURI(uri);
		return escURI;
	    }
	    hw_xmlFreeURI(uri);
	}
    }

path_processing:
#if defined(_WIN32) && !defined(__CYGWIN__)    
    /*
     * This really need to be cleaned up by someone with a Windows box
     */
    uri = hw_xmlCreateURI();
    if (uri == NULL) {
        return(NULL);
    }

    len = hw_xmlStrlen(path);
    if ((len > 2) && IS_WINDOWS_PATH(path)) {
	uri->scheme = hw_xmlStrdup(hw_BAD_CAST "file");
	uri->path = hw_xmlMallocAtomic(len + 2);	/* FIXME - check alloc! */
	uri->path[0] = '/';
	p = uri->path + 1;
	strncpy(p, path, len + 1);
    } else {
	uri->path = hw_xmlStrdup(path);		/* FIXME - check alloc! */
	p = uri->path;
    }
    while (*p != '\0') {
	if (*p == '\\')
	    *p = '/';
	p++;
    }
    if (uri->path == NULL) {
        hw_xmlFreeURI(uri);
        return(NULL);
    }

	if (uri->scheme == NULL) {
		ret = hw_xmlStrdup((const hw_xmlChar *) path);
	} else {
		ret = hw_xmlSaveUri(uri);
	}

    hw_xmlFreeURI(uri);
#else
    ret = hw_xmlStrdup((const hw_xmlChar *) path);
#endif
    return(ret);
}

