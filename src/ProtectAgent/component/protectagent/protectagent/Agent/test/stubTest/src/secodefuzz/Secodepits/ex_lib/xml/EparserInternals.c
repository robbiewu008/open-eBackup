/*
 * parserInternals.c : Internal routines (and obsolete ones) needed for the
 *                     XML and HTML parsers.
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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "Etree.h"
#include "Eparser.h"

/************************************************************************
 *									*
 * 		Some factorized error routines				*
 *									*
 ************************************************************************/


/**
 * hw_xmlErrMemory:
 * @ctxt:  an XML parser context
 * @extra:  extra informations
 *
 * Handle a redefinition of attribute error
 */
void
hw_xmlErrMemory(hw_xmlParserCtxtPtr ctxt, const char *extra)
{
    if ((ctxt != NULL) && (ctxt->disableSAX != 0) &&
        (ctxt->instate == XML_PARSER_EOF))
	return;
    if (ctxt != NULL) {
        ctxt->errNo = XML_ERR_NO_MEMORY;
        ctxt->instate = XML_PARSER_EOF;
        ctxt->disableSAX = 1;
    }
    if (extra)
        hw___xmlRaiseError(NULL, NULL, NULL, ctxt, NULL, XML_FROM_PARSER,
                        XML_ERR_NO_MEMORY, XML_ERR_FATAL, NULL, 0, extra,
                        NULL, NULL, 0, 0,
                        "Memory allocation failed : %s\n", extra);
    else
        hw___xmlRaiseError(NULL, NULL, NULL, ctxt, NULL, XML_FROM_PARSER,
                        XML_ERR_NO_MEMORY, XML_ERR_FATAL, NULL, 0, NULL,
                        NULL, NULL, 0, 0, "Memory allocation failed\n");
}

/**
 * hw___xmlErrEncoding:
 * @ctxt:  an XML parser context
 * @xmlerr:  the error number
 * @msg:  the error message
 * @str1:  an string info
 * @str2:  an string info
 *
 * Handle an encoding error
 */
void
hw___xmlErrEncoding(hw_xmlParserCtxtPtr ctxt, hw_xmlParserErrors xmlerr,
                 const char *msg, const hw_xmlChar * str1, const hw_xmlChar * str2)
{
    if ((ctxt != NULL) && (ctxt->disableSAX != 0) &&
        (ctxt->instate == XML_PARSER_EOF))
	return;
    if (ctxt != NULL)
        ctxt->errNo = xmlerr;
    hw___xmlRaiseError(NULL, NULL, NULL,
                    ctxt, NULL, XML_FROM_PARSER, xmlerr, XML_ERR_FATAL,
                    NULL, 0, (const char *) str1, (const char *) str2,
                    NULL, 0, 0, msg, str1, str2);
    if (ctxt != NULL) {
        ctxt->wellFormed = 0;
        if (ctxt->recovery == 0)
            ctxt->disableSAX = 1;
    }
}

/**
 * xmlErrInternal:
 * @ctxt:  an XML parser context
 * @msg:  the error message
 * @str:  error informations
 *
 * Handle an internal error
 */
static void
xmlErrInternal(hw_xmlParserCtxtPtr ctxt, const char *msg, const hw_xmlChar * str)
{
    if ((ctxt != NULL) && (ctxt->disableSAX != 0) &&
        (ctxt->instate == XML_PARSER_EOF))
	return;
    if (ctxt != NULL)
        ctxt->errNo = XML_ERR_INTERNAL_ERROR;
    hw___xmlRaiseError(NULL, NULL, NULL,
                    ctxt, NULL, XML_FROM_PARSER, XML_ERR_INTERNAL_ERROR,
                    XML_ERR_FATAL, NULL, 0, (const char *) str, NULL, NULL,
                    0, 0, msg, str);
    if (ctxt != NULL) {
        ctxt->wellFormed = 0;
        if (ctxt->recovery == 0)
            ctxt->disableSAX = 1;
    }
}

/**
 * xmlErrEncodingInt:
 * @ctxt:  an XML parser context
 * @error:  the error number
 * @msg:  the error message
 * @val:  an integer value
 *
 * n encoding error
 */
static void
xmlErrEncodingInt(hw_xmlParserCtxtPtr ctxt, hw_xmlParserErrors error,
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

/************************************************************************
 *									*
 * 		Input handling functions for progressive parsing	*
 *									*
 ************************************************************************/

/* #define DEBUG_STACK */


/* we need to keep enough input to show errors in context */
#define LINE_LEN        80


#define CHECK_BUFFER(in) 



/**
 * hw_xmlParserInputGrow:
 * @in:  an XML parser input
 * @len:  an indicative size for the lookahead
 *
 * This function increase the input for the parser. It tries to
 * preserve pointers to the input buffer, and keep already read data
 *
 * Returns the number of xmlChars read, or -1 in case of error, 0 indicate the
 * end of this entity
 */
int
hw_xmlParserInputGrow(hw_xmlParserInputPtr in, int len) {
    int ret;
    int indx;

    if (in == NULL) return(-1);
    if (in->buf == NULL) return(-1);
    if (in->base == NULL) return(-1);
    if (in->cur == NULL) return(-1);
    if (in->buf->buffer == NULL) return(-1);

    CHECK_BUFFER(in);

    indx = in->cur - in->base;
    if (in->buf->buffer->use > (unsigned int) indx + hw_INPUT_CHUNK) {

	CHECK_BUFFER(in);

        return(0);
    }
    if (in->buf->readcallback != NULL)
	ret = hw_xmlParserInputBufferGrow(in->buf, len);
    else	
        return(0);

    /*
     * NOTE : in->base may be a "dangling" i.e. freed pointer in this
     *        block, but we use it really as an integer to do some
     *        pointer arithmetic. Insure will raise it as a bug but in
     *        that specific case, that's not !
     */
    if (in->base != in->buf->buffer->content) {
        /*
	 * the buffer has been reallocated
	 */
	indx = in->cur - in->base;
	in->base = in->buf->buffer->content;
	in->cur = &in->buf->buffer->content[indx];
    }
    in->end = &in->buf->buffer->content[in->buf->buffer->use];

    CHECK_BUFFER(in);

    return(ret);
}

/**
 * hw_xmlParserInputShrink:
 * @in:  an XML parser input
 *
 * This function removes used input for the parser.
 */
void
hw_xmlParserInputShrink(hw_xmlParserInputPtr in) {
    int used;
    int ret;
    int indx;

    if (in == NULL) return;
    if (in->buf == NULL) return;
    if (in->base == NULL) return;
    if (in->cur == NULL) return;
    if (in->buf->buffer == NULL) return;

    CHECK_BUFFER(in);

    used = in->cur - in->buf->buffer->content;
    /*
     * Do not shrink on large buffers whose only a tiny fraction
     * was consumed
     */
    if (used > hw_INPUT_CHUNK) {
	ret = hw_xmlBufferShrink(in->buf->buffer, used - LINE_LEN);
	if (ret > 0) {
	    in->cur -= ret;
	    in->consumed += ret;
	}
	in->end = &in->buf->buffer->content[in->buf->buffer->use];
    }

    CHECK_BUFFER(in);

    if (in->buf->buffer->use > hw_INPUT_CHUNK) {
        return;
    }
    hw_xmlParserInputBufferRead(in->buf, 2 * hw_INPUT_CHUNK);
    if (in->base != in->buf->buffer->content) {
        /*
	 * the buffer has been reallocated
	 */
	indx = in->cur - in->base;
	in->base = in->buf->buffer->content;
	in->cur = &in->buf->buffer->content[indx];
    }
    in->end = &in->buf->buffer->content[in->buf->buffer->use];

    CHECK_BUFFER(in);
}

/************************************************************************
 *									*
 * 		UTF8 character input and related functions		*
 *									*
 ************************************************************************/

/**
 * hw_xmlNextChar:
 * @ctxt:  the XML parser context
 *
 * Skip to the next char input char.
 */

void
hw_xmlNextChar(hw_xmlParserCtxtPtr ctxt)
{
    if ((ctxt == NULL) || (ctxt->instate == XML_PARSER_EOF) ||
        (ctxt->input == NULL))
        return;

    if (ctxt->charset == XML_CHAR_ENCODING_UTF8) {
        if ((*ctxt->input->cur == 0) &&
            (hw_xmlParserInputGrow(ctxt->input, hw_INPUT_CHUNK) <= 0) &&
            (ctxt->instate != XML_PARSER_COMMENT)) {
            /*
             * If we are at the end of the current entity and
             * the context allows it, we pop consumed entities
             * automatically.
             * the auto closing should be blocked in other cases
             */
            hw_xmlPopInput(ctxt);
        } else {
            const unsigned char *cur;
            unsigned char c;

            /*
             *   2.11 End-of-Line Handling
             *   the literal two-character sequence "#xD#xA" or a standalone
             *   literal #xD, an XML processor must pass to the application
             *   the single character #xA.
             */
            if (*(ctxt->input->cur) == '\n') {
                ctxt->input->line++; ctxt->input->col = 1;
            } else
                ctxt->input->col++;

            /*
             * We are supposed to handle UTF8, check it's valid
             * From rfc2044: encoding of the Unicode values on UTF-8:
             *
             * UCS-4 range (hex.)           UTF-8 octet sequence (binary)
             * 0000 0000-0000 007F   0xxxxxxx
             * 0000 0080-0000 07FF   110xxxxx 10xxxxxx
             * 0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx 
             *
             * Check for the 0x110000 limit too
             */
            cur = ctxt->input->cur;

            c = *cur;
            if (c & 0x80) {
	        if (c == 0xC0)
		    goto encoding_error;
                if (cur[1] == 0)
                    hw_xmlParserInputGrow(ctxt->input, hw_INPUT_CHUNK);
                if ((cur[1] & 0xc0) != 0x80)
                    goto encoding_error;
                if ((c & 0xe0) == 0xe0) {
                    unsigned int val;

                    if (cur[2] == 0)
                        hw_xmlParserInputGrow(ctxt->input, hw_INPUT_CHUNK);
                    if ((cur[2] & 0xc0) != 0x80)
                        goto encoding_error;
                    if ((c & 0xf0) == 0xf0) {
                        if (cur[3] == 0)
                            hw_xmlParserInputGrow(ctxt->input, hw_INPUT_CHUNK);
                        if (((c & 0xf8) != 0xf0) ||
                            ((cur[3] & 0xc0) != 0x80))
                            goto encoding_error;
                        /* 4-byte code */
                        ctxt->input->cur += 4;
                        val = (cur[0] & 0x7) << 18;
                        val |= (cur[1] & 0x3f) << 12;
                        val |= (cur[2] & 0x3f) << 6;
                        val |= cur[3] & 0x3f;
                    } else {
                        /* 3-byte code */
                        ctxt->input->cur += 3;
                        val = (cur[0] & 0xf) << 12;
                        val |= (cur[1] & 0x3f) << 6;
                        val |= cur[2] & 0x3f;
                    }
                    if (((val > 0xd7ff) && (val < 0xe000)) ||
                        ((val > 0xfffd) && (val < 0x10000)) ||
                        (val >= 0x110000)) {
			xmlErrEncodingInt(ctxt, XML_ERR_INVALID_CHAR,
					  "Char 0x%X out of allowed range\n",
					  val);
                    }
                } else
                    /* 2-byte code */
                    ctxt->input->cur += 2;
            } else
                /* 1-byte code */
                ctxt->input->cur++;

            ctxt->nbChars++;
            if (*ctxt->input->cur == 0)
                hw_xmlParserInputGrow(ctxt->input, hw_INPUT_CHUNK);
        }
    } else {
        /*
         * Assume it's a fixed length encoding (1) with
         * a compatible encoding for the ASCII set, since
         * XML constructs only use < 128 chars
         */

        if (*(ctxt->input->cur) == '\n') {
            ctxt->input->line++; ctxt->input->col = 1;
        } else
            ctxt->input->col++;
        ctxt->input->cur++;
        ctxt->nbChars++;
        if (*ctxt->input->cur == 0)
            hw_xmlParserInputGrow(ctxt->input, hw_INPUT_CHUNK);
    }
    if ((*ctxt->input->cur == '%') && (!ctxt->html))
        hw_xmlParserHandlePEReference(ctxt);
    if ((*ctxt->input->cur == 0) &&
        (hw_xmlParserInputGrow(ctxt->input, hw_INPUT_CHUNK) <= 0))
        hw_xmlPopInput(ctxt);
    return;
encoding_error:
    /*
     * If we detect an UTF8 error that probably mean that the
     * input encoding didn't get properly advertised in the
     * declaration header. Report the error and switch the encoding
     * to ISO-Latin-1 (if you don't like this policy, just declare the
     * encoding !)
     */
    if ((ctxt == NULL) || (ctxt->input == NULL) ||
        (ctxt->input->end - ctxt->input->cur < 4)) {
	hw___xmlErrEncoding(ctxt, XML_ERR_INVALID_CHAR,
		     "Input is not proper UTF-8, indicate encoding !\n",
		     NULL, NULL);
    } else {
        char buffer[150];

	snprintf(buffer, 149, "Bytes: 0x%02X 0x%02X 0x%02X 0x%02X\n",
			ctxt->input->cur[0], ctxt->input->cur[1],
			ctxt->input->cur[2], ctxt->input->cur[3]);
	hw___xmlErrEncoding(ctxt, XML_ERR_INVALID_CHAR,
		     "Input is not proper UTF-8, indicate encoding !\n%s",
		     hw_BAD_CAST buffer, NULL);
    }
    ctxt->charset = XML_CHAR_ENCODING_8859_1;
    ctxt->input->cur++;
    return;
}

/**
 * hw_xmlCurrentChar:
 * @ctxt:  the XML parser context
 * @len:  pointer to the length of the char read
 *
 * The current char value, if using UTF-8 this may actually span multiple
 * bytes in the input buffer. Implement the end of line normalization:
 * 2.11 End-of-Line Handling
 * Wherever an external parsed entity or the literal entity value
 * of an internal parsed entity contains either the literal two-character
 * sequence "#xD#xA" or a standalone literal #xD, an XML processor
 * must pass to the application the single character #xA.
 * This behavior can conveniently be produced by normalizing all
 * line breaks to #xA on input, before parsing.)
 *
 * Returns the current char value and its length
 */

int
hw_xmlCurrentChar(hw_xmlParserCtxtPtr ctxt, int *len) {
    if ((ctxt == NULL) || (len == NULL) || (ctxt->input == NULL)) return(0);
    if (ctxt->instate == XML_PARSER_EOF)
	return(0);

    if ((*ctxt->input->cur >= 0x20) && (*ctxt->input->cur <= 0x7F)) {
	    *len = 1;
	    return((int) *ctxt->input->cur);
    }
    if (ctxt->charset == XML_CHAR_ENCODING_UTF8) {
	/*
	 * We are supposed to handle UTF8, check it's valid
	 * From rfc2044: encoding of the Unicode values on UTF-8:
	 *
	 * UCS-4 range (hex.)           UTF-8 octet sequence (binary)
	 * 0000 0000-0000 007F   0xxxxxxx
	 * 0000 0080-0000 07FF   110xxxxx 10xxxxxx
	 * 0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx 
	 *
	 * Check for the 0x110000 limit too
	 */
	const unsigned char *cur = ctxt->input->cur;
	unsigned char c;
	unsigned int val;

	c = *cur;
	if (c & 0x80) {
	    if (c == 0xC0)
		goto encoding_error;
	    if (cur[1] == 0)
		hw_xmlParserInputGrow(ctxt->input, hw_INPUT_CHUNK);
	    if ((cur[1] & 0xc0) != 0x80)
		goto encoding_error;
	    if ((c & 0xe0) == 0xe0) {

		if (cur[2] == 0)
		    hw_xmlParserInputGrow(ctxt->input, hw_INPUT_CHUNK);
		if ((cur[2] & 0xc0) != 0x80)
		    goto encoding_error;
		if ((c & 0xf0) == 0xf0) {
		    if (cur[3] == 0)
			hw_xmlParserInputGrow(ctxt->input, hw_INPUT_CHUNK);
		    if (((c & 0xf8) != 0xf0) ||
			((cur[3] & 0xc0) != 0x80))
			goto encoding_error;
		    /* 4-byte code */
		    *len = 4;
		    val = (cur[0] & 0x7) << 18;
		    val |= (cur[1] & 0x3f) << 12;
		    val |= (cur[2] & 0x3f) << 6;
		    val |= cur[3] & 0x3f;
		} else {
		  /* 3-byte code */
		    *len = 3;
		    val = (cur[0] & 0xf) << 12;
		    val |= (cur[1] & 0x3f) << 6;
		    val |= cur[2] & 0x3f;
		}
	    } else {
	      /* 2-byte code */
		*len = 2;
		val = (cur[0] & 0x1f) << 6;
		val |= cur[1] & 0x3f;
	    }
	    if (!hw_IS_CHAR(val)) {
	        xmlErrEncodingInt(ctxt, XML_ERR_INVALID_CHAR,
				  "Char 0x%X out of allowed range\n", val);
	    }    
	    return(val);
	} else {
	    /* 1-byte code */
	    *len = 1;
	    if (*ctxt->input->cur == 0xD) {
		if (ctxt->input->cur[1] == 0xA) {
		    ctxt->nbChars++;
		    ctxt->input->cur++;
		}
		return(0xA);
	    }
	    return((int) *ctxt->input->cur);
	}
    }
    /*
     * Assume it's a fixed length encoding (1) with
     * a compatible encoding for the ASCII set, since
     * XML constructs only use < 128 chars
     */
    *len = 1;
    if (*ctxt->input->cur == 0xD) {
	if (ctxt->input->cur[1] == 0xA) {
	    ctxt->nbChars++;
	    ctxt->input->cur++;
	}
	return(0xA);
    }
    return((int) *ctxt->input->cur);
encoding_error:
    /*
     * An encoding problem may arise from a truncated input buffer
     * splitting a character in the middle. In that case do not raise
     * an error but return 0 to endicate an end of stream problem
     */
    if (ctxt->input->end - ctxt->input->cur < 4) {
	*len = 0;
	return(0);
    }

    /*
     * If we detect an UTF8 error that probably mean that the
     * input encoding didn't get properly advertised in the
     * declaration header. Report the error and switch the encoding
     * to ISO-Latin-1 (if you don't like this policy, just declare the
     * encoding !)
     */
    {
        char buffer[150];

	snprintf(&buffer[0], 149, "Bytes: 0x%02X 0x%02X 0x%02X 0x%02X\n",
			ctxt->input->cur[0], ctxt->input->cur[1],
			ctxt->input->cur[2], ctxt->input->cur[3]);
	hw___xmlErrEncoding(ctxt, XML_ERR_INVALID_CHAR,
		     "Input is not proper UTF-8, indicate encoding !\n%s",
		     hw_BAD_CAST buffer, NULL);
    }
    ctxt->charset = XML_CHAR_ENCODING_8859_1; 
    *len = 1;
    return((int) *ctxt->input->cur);
}

/**
 * hw_xmlStringCurrentChar:
 * @ctxt:  the XML parser context
 * @cur:  pointer to the beginning of the char
 * @len:  pointer to the length of the char read
 *
 * The current char value, if using UTF-8 this may actually span multiple
 * bytes in the input buffer.
 *
 * Returns the current char value and its length
 */

int
hw_xmlStringCurrentChar(hw_xmlParserCtxtPtr ctxt, const hw_xmlChar * cur, int *len)
{
    if ((len == NULL) || (cur == NULL)) return(0);
    if ((ctxt == NULL) || (ctxt->charset == XML_CHAR_ENCODING_UTF8)) {
        /*
         * We are supposed to handle UTF8, check it's valid
         * From rfc2044: encoding of the Unicode values on UTF-8:
         *
         * UCS-4 range (hex.)           UTF-8 octet sequence (binary)
         * 0000 0000-0000 007F   0xxxxxxx
         * 0000 0080-0000 07FF   110xxxxx 10xxxxxx
         * 0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx 
         *
         * Check for the 0x110000 limit too
         */
        unsigned char c;
        unsigned int val;

        c = *cur;
        if (c & 0x80) {
            if ((cur[1] & 0xc0) != 0x80)
                goto encoding_error;
            if ((c & 0xe0) == 0xe0) {

                if ((cur[2] & 0xc0) != 0x80)
                    goto encoding_error;
                if ((c & 0xf0) == 0xf0) {
                    if (((c & 0xf8) != 0xf0) || ((cur[3] & 0xc0) != 0x80))
                        goto encoding_error;
                    /* 4-byte code */
                    *len = 4;
                    val = (cur[0] & 0x7) << 18;
                    val |= (cur[1] & 0x3f) << 12;
                    val |= (cur[2] & 0x3f) << 6;
                    val |= cur[3] & 0x3f;
                } else {
                    /* 3-byte code */
                    *len = 3;
                    val = (cur[0] & 0xf) << 12;
                    val |= (cur[1] & 0x3f) << 6;
                    val |= cur[2] & 0x3f;
                }
            } else {
                /* 2-byte code */
                *len = 2;
                val = (cur[0] & 0x1f) << 6;
                val |= cur[1] & 0x3f;
            }
            if (!hw_IS_CHAR(val)) {
	        xmlErrEncodingInt(ctxt, XML_ERR_INVALID_CHAR,
				  "Char 0x%X out of allowed range\n", val);
            }
            return (val);
        } else {
            /* 1-byte code */
            *len = 1;
            return ((int) *cur);
        }
    }
    /*
     * Assume it's a fixed length encoding (1) with
     * a compatible encoding for the ASCII set, since
     * XML constructs only use < 128 chars
     */
    *len = 1;
    return ((int) *cur);
encoding_error:

    /*
     * An encoding problem may arise from a truncated input buffer
     * splitting a character in the middle. In that case do not raise
     * an error but return 0 to endicate an end of stream problem
     */
    if ((ctxt == NULL) || (ctxt->input == NULL) ||
        (ctxt->input->end - ctxt->input->cur < 4)) {
	*len = 0;
	return(0);
    }
    /*
     * If we detect an UTF8 error that probably mean that the
     * input encoding didn't get properly advertised in the
     * declaration header. Report the error and switch the encoding
     * to ISO-Latin-1 (if you don't like this policy, just declare the
     * encoding !)
     */
    {
        char buffer[150];

	snprintf(buffer, 149, "Bytes: 0x%02X 0x%02X 0x%02X 0x%02X\n",
			ctxt->input->cur[0], ctxt->input->cur[1],
			ctxt->input->cur[2], ctxt->input->cur[3]);
	hw___xmlErrEncoding(ctxt, XML_ERR_INVALID_CHAR,
		     "Input is not proper UTF-8, indicate encoding !\n%s",
		     hw_BAD_CAST buffer, NULL);
    }
    *len = 1;
    return ((int) *cur);
}

/**
 * hw_xmlCopyCharMultiByte:
 * @out:  pointer to an array of hw_xmlChar
 * @val:  the char value
 *
 * append the char value in the array 
 *
 * Returns the number of hw_xmlChar written
 */
int
hw_xmlCopyCharMultiByte(hw_xmlChar *out, int val) {
    if (out == NULL) return(0);
    /*
     * We are supposed to handle UTF8, check it's valid
     * From rfc2044: encoding of the Unicode values on UTF-8:
     *
     * UCS-4 range (hex.)           UTF-8 octet sequence (binary)
     * 0000 0000-0000 007F   0xxxxxxx
     * 0000 0080-0000 07FF   110xxxxx 10xxxxxx
     * 0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx 
     */
    if  (val >= 0x80) {
	hw_xmlChar *savedout = out;
	int bits;
	if (val <   0x800) { *out++= (val >>  6) | 0xC0;  bits=  0; }
	else if (val < 0x10000) { *out++= (val >> 12) | 0xE0;  bits=  6;}
	else if (val < 0x110000)  { *out++= (val >> 18) | 0xF0;  bits=  12; }
	else {
	    xmlErrEncodingInt(NULL, XML_ERR_INVALID_CHAR,
		    "Internal error, hw_xmlCopyCharMultiByte 0x%X out of bound\n",
			      val);
	    return(0);
	}
	for ( ; bits >= 0; bits-= 6)
	    *out++= ((val >> bits) & 0x3F) | 0x80 ;
	return (out - savedout);
    }
    *out = (hw_xmlChar) val;
    return 1;
}

/**
 * hw_xmlCopyChar:
 * @len:  Ignored, compatibility
 * @out:  pointer to an array of hw_xmlChar
 * @val:  the char value
 *
 * append the char value in the array 
 *
 * Returns the number of hw_xmlChar written
 */

int
hw_xmlCopyChar(int len ATTRIBUTE_UNUSED, hw_xmlChar *out, int val) {
    if (out == NULL) return(0);
    /* the len parameter is ignored */
    if  (val >= 0x80) {
	return(hw_xmlCopyCharMultiByte (out, val));
    }
    *out = (hw_xmlChar) val;
    return 1;
}

/************************************************************************
 *									*
 *		Commodity functions to switch encodings			*
 *									*
 ************************************************************************/

/**
 * hw_xmlSwitchEncoding:
 * @ctxt:  the parser context
 * @enc:  the encoding value (number)
 *
 * change the input functions when discovering the character encoding
 * of a given entity.
 *
 * Returns 0 in case of success, -1 otherwise
 */
int
hw_xmlSwitchEncoding(hw_xmlParserCtxtPtr ctxt, hw_xmlCharEncoding enc)
{
    hw_xmlCharEncodingHandlerPtr handler;

    if (ctxt == NULL) return(-1);
    switch (enc) {
	case XML_CHAR_ENCODING_ERROR:
	    hw___xmlErrEncoding(ctxt, XML_ERR_UNKNOWN_ENCODING,
	                   "encoding unknown\n", NULL, NULL);
	    return(-1);
	case XML_CHAR_ENCODING_NONE:
	    /* let's assume it's UTF-8 without the XML decl */
	    ctxt->charset = XML_CHAR_ENCODING_UTF8;
	    return(0);
	case XML_CHAR_ENCODING_UTF8:
	    /* default encoding, no conversion should be needed */
	    ctxt->charset = XML_CHAR_ENCODING_UTF8;

	    /*
	     * Errata on XML-1.0 June 20 2001
	     * Specific handling of the Byte Order Mark for
	     * UTF-8
	     */
	    if ((ctxt->input != NULL) &&
		(ctxt->input->cur[0] == 0xEF) &&
		(ctxt->input->cur[1] == 0xBB) &&
		(ctxt->input->cur[2] == 0xBF)) {
		ctxt->input->cur += 3;
	    }
	    return(0);
    case XML_CHAR_ENCODING_UTF16LE:
    case XML_CHAR_ENCODING_UTF16BE:
        /*The raw input characters are encoded
         *in UTF-16. As we expect this function
         *to be called after hw_xmlCharEncInFunc, we expect
         *ctxt->input->cur to contain UTF-8 encoded characters.
         *So the raw UTF16 Byte Order Mark
         *has also been converted into
         *an UTF-8 BOM. Let's skip that BOM.
         */
        if ((ctxt->input != NULL) && (ctxt->input->cur != NULL) &&
            (ctxt->input->cur[0] == 0xEF) &&
            (ctxt->input->cur[1] == 0xBB) &&
            (ctxt->input->cur[2] == 0xBF)) {
            ctxt->input->cur += 3;
        }
	break ;
	default:
	    break;
    }
    handler = hw_xmlGetCharEncodingHandler(enc);
    if (handler == NULL) {
	/*
	 * Default handlers.
	 */
	switch (enc) {
	    case XML_CHAR_ENCODING_ASCII:
		/* default encoding, no conversion should be needed */
		ctxt->charset = XML_CHAR_ENCODING_UTF8;
		return(0);
	    case XML_CHAR_ENCODING_UTF16LE:
		break;
	    case XML_CHAR_ENCODING_UTF16BE:
		break;
	    case XML_CHAR_ENCODING_UCS4LE:
		hw___xmlErrEncoding(ctxt, XML_ERR_UNSUPPORTED_ENCODING,
			       "encoding not supported %s\n",
			       hw_BAD_CAST "USC4 little endian", NULL);
		break;
	    case XML_CHAR_ENCODING_UCS4BE:
		hw___xmlErrEncoding(ctxt, XML_ERR_UNSUPPORTED_ENCODING,
			       "encoding not supported %s\n",
			       hw_BAD_CAST "USC4 big endian", NULL);
		break;
	    case XML_CHAR_ENCODING_EBCDIC:
		hw___xmlErrEncoding(ctxt, XML_ERR_UNSUPPORTED_ENCODING,
			       "encoding not supported %s\n",
			       hw_BAD_CAST "EBCDIC", NULL);
		break;
	    case XML_CHAR_ENCODING_UCS4_2143:
		hw___xmlErrEncoding(ctxt, XML_ERR_UNSUPPORTED_ENCODING,
			       "encoding not supported %s\n",
			       hw_BAD_CAST "UCS4 2143", NULL);
		break;
	    case XML_CHAR_ENCODING_UCS4_3412:
		hw___xmlErrEncoding(ctxt, XML_ERR_UNSUPPORTED_ENCODING,
			       "encoding not supported %s\n",
			       hw_BAD_CAST "UCS4 3412", NULL);
		break;
	    case XML_CHAR_ENCODING_UCS2:
		hw___xmlErrEncoding(ctxt, XML_ERR_UNSUPPORTED_ENCODING,
			       "encoding not supported %s\n",
			       hw_BAD_CAST "UCS2", NULL);
		break;
	    case XML_CHAR_ENCODING_8859_1:
	    case XML_CHAR_ENCODING_8859_2:
	    case XML_CHAR_ENCODING_8859_3:
	    case XML_CHAR_ENCODING_8859_4:
	    case XML_CHAR_ENCODING_8859_5:
	    case XML_CHAR_ENCODING_8859_6:
	    case XML_CHAR_ENCODING_8859_7:
	    case XML_CHAR_ENCODING_8859_8:
	    case XML_CHAR_ENCODING_8859_9:
		/*
		 * We used to keep the internal content in the
		 * document encoding however this turns being unmaintainable
		 * So hw_xmlGetCharEncodingHandler() will return non-null
		 * values for this now.
		 */
		if ((ctxt->inputNr == 1) &&
		    (ctxt->encoding == NULL) &&
		    (ctxt->input != NULL) &&
		    (ctxt->input->encoding != NULL)) {
		    ctxt->encoding = hw_xmlStrdup(ctxt->input->encoding);
		}
		ctxt->charset = enc;
		return(0);
	    case XML_CHAR_ENCODING_2022_JP:
		hw___xmlErrEncoding(ctxt, XML_ERR_UNSUPPORTED_ENCODING,
			       "encoding not supported %s\n",
			       hw_BAD_CAST "ISO-2022-JP", NULL);
		break;
	    case XML_CHAR_ENCODING_SHIFT_JIS:
		hw___xmlErrEncoding(ctxt, XML_ERR_UNSUPPORTED_ENCODING,
			       "encoding not supported %s\n",
			       hw_BAD_CAST "Shift_JIS", NULL);
		break;
	    case XML_CHAR_ENCODING_EUC_JP:
		hw___xmlErrEncoding(ctxt, XML_ERR_UNSUPPORTED_ENCODING,
			       "encoding not supported %s\n",
			       hw_BAD_CAST "EUC-JP", NULL);
		break;
	    default:
	        break;
	}
    }
    if (handler == NULL)
	return(-1);
    ctxt->charset = XML_CHAR_ENCODING_UTF8;
    return(hw_xmlSwitchToEncoding(ctxt, handler));
}

/**
 * hw_xmlSwitchInputEncoding:
 * @ctxt:  the parser context
 * @input:  the input stream
 * @handler:  the encoding handler
 *
 * change the input functions when discovering the character encoding
 * of a given entity.
 *
 * Returns 0 in case of success, -1 otherwise
 */
int
hw_xmlSwitchInputEncoding(hw_xmlParserCtxtPtr ctxt, hw_xmlParserInputPtr input,
                       hw_xmlCharEncodingHandlerPtr handler)
{
    int nbchars;

    if (handler == NULL)
        return (-1);
    if (input == NULL)
        return (-1);
    if (input->buf != NULL) {
        if (input->buf->encoder != NULL) {
            /*
             * Check in case the auto encoding detetection triggered
             * in already.
             */
            if (input->buf->encoder == handler)
                return (0);

            /*
             * "UTF-16" can be used for both LE and BE
             if ((!hw_xmlStrncmp(hw_BAD_CAST input->buf->encoder->name,
             hw_BAD_CAST "UTF-16", 6)) &&
             (!hw_xmlStrncmp(hw_BAD_CAST handler->name,
             hw_BAD_CAST "UTF-16", 6))) {
             return(0);
             }
             */

            /*
             * Note: this is a bit dangerous, but that's what it
             * takes to use nearly compatible signature for different
             * encodings.
             */
            hw_xmlCharEncCloseFunc(input->buf->encoder);
            input->buf->encoder = handler;
            return (0);
        }
        input->buf->encoder = handler;

        /*
         * Is there already some content down the pipe to convert ?
         */
        if ((input->buf->buffer != NULL) && (input->buf->buffer->use > 0)) {
            int processed;
	    unsigned int use;

            /*
             * Specific handling of the Byte Order Mark for 
             * UTF-16
             */
            if ((handler->name != NULL) &&
                (!strcmp(handler->name, "UTF-16LE") ||
                 !strcmp(handler->name, "UTF-16")) &&
                (input->cur[0] == 0xFF) && (input->cur[1] == 0xFE)) {
                input->cur += 2;
            }
            if ((handler->name != NULL) &&
                (!strcmp(handler->name, "UTF-16BE")) &&
                (input->cur[0] == 0xFE) && (input->cur[1] == 0xFF)) {
                input->cur += 2;
            }
            /*
             * Errata on XML-1.0 June 20 2001
             * Specific handling of the Byte Order Mark for
             * UTF-8
             */
            if ((handler->name != NULL) &&
                (!strcmp(handler->name, "UTF-8")) &&
                (input->cur[0] == 0xEF) &&
                (input->cur[1] == 0xBB) && (input->cur[2] == 0xBF)) {
                input->cur += 3;
            }

            /*
             * Shrink the current input buffer.
             * Move it as the raw buffer and create a new input buffer
             */
            processed = input->cur - input->base;
            hw_xmlBufferShrink(input->buf->buffer, processed);
            input->buf->raw = input->buf->buffer;
            input->buf->buffer = hw_xmlBufferCreate();
	    input->buf->rawconsumed = processed;
	    use = input->buf->raw->use;

            if (ctxt->html) {
                /*
                 * convert as much as possible of the buffer
                 */
                nbchars = hw_xmlCharEncInFunc(input->buf->encoder,
                                           input->buf->buffer,
                                           input->buf->raw);
            } else {
                /*
                 * convert just enough to get
                 * '<?xml version="1.0" encoding="xxx"?>'
                 * parsed with the autodetected encoding
                 * into the parser reading buffer.
                 */
                nbchars = hw_xmlCharEncFirstLine(input->buf->encoder,
                                              input->buf->buffer,
                                              input->buf->raw);
            }
            if (nbchars < 0) {
                xmlErrInternal(ctxt,
                               "switching encoding: encoder error\n",
                               NULL);
                return (-1);
            }
	    input->buf->rawconsumed += use - input->buf->raw->use;
            input->base = input->cur = input->buf->buffer->content;
            input->end = &input->base[input->buf->buffer->use];

        }
        return (0);
    } else if (input->length == 0) {
	/*
	 * When parsing a static memory array one must know the
	 * size to be able to convert the buffer.
	 */
	xmlErrInternal(ctxt, "switching encoding : no input\n", NULL);
	return (-1);
    }
    return (0);
}

/**
 * hw_xmlSwitchToEncoding:
 * @ctxt:  the parser context
 * @handler:  the encoding handler
 *
 * change the input functions when discovering the character encoding
 * of a given entity.
 *
 * Returns 0 in case of success, -1 otherwise
 */
int
hw_xmlSwitchToEncoding(hw_xmlParserCtxtPtr ctxt, hw_xmlCharEncodingHandlerPtr handler) 
{
    int ret = 0;

    if (handler != NULL) {
        if (ctxt->input != NULL) {
	    ret = hw_xmlSwitchInputEncoding(ctxt, ctxt->input, handler);
	} else {
	    xmlErrInternal(ctxt, "hw_xmlSwitchToEncoding : no input\n",
	                   NULL);
	    return(-1);
	}
	/*
	 * The parsing is now done in UTF8 natively
	 */
	ctxt->charset = XML_CHAR_ENCODING_UTF8;
    } else 
	return(-1);
    return(ret);
}

/************************************************************************
 *									*
 *	Commodity functions to handle entities processing		*
 *									*
 ************************************************************************/

/**
 * hw_xmlFreeInputStream:
 * @input:  an hw_xmlParserInputPtr
 *
 * Free up an input stream.
 */
void
hw_xmlFreeInputStream(hw_xmlParserInputPtr input) {
    if (input == NULL) return;

    if (input->filename != NULL) hw_xmlFree((char *) input->filename);
    if (input->directory != NULL) hw_xmlFree((char *) input->directory);
    if (input->encoding != NULL) hw_xmlFree((char *) input->encoding);
    if (input->version != NULL) hw_xmlFree((char *) input->version);
    if ((input->free != NULL) && (input->base != NULL))
        input->free((hw_xmlChar *) input->base);
    if (input->buf != NULL) 
        hw_xmlFreeParserInputBuffer(input->buf);
    hw_xmlFree(input);
}

/**
 * hw_xmlNewInputStream:
 * @ctxt:  an XML parser context
 *
 * Create a new input stream structure
 * Returns the new input stream or NULL
 */
hw_xmlParserInputPtr
hw_xmlNewInputStream(hw_xmlParserCtxtPtr ctxt) {
    hw_xmlParserInputPtr input;
    static int id = 0;

    input = (hw_xmlParserInputPtr) hw_xmlMalloc(sizeof(hw_xmlParserInput));
    if (input == NULL) {
        hw_xmlErrMemory(ctxt,  "couldn't allocate a new input stream\n");
	return(NULL);
    }
    memset(input, 0, sizeof(hw_xmlParserInput));
    input->line = 1;
    input->col = 1;
    /*
     * we don't care about thread reentrancy unicity for a single
     * parser context (and hence thread) is sufficient.
     */
    input->id = id++;
    return(input);
}


/**
 * hw_xmlNewEntityInputStream:
 * @ctxt:  an XML parser context
 * @entity:  an Entity pointer
 *
 * Create a new input stream based on an hw_xmlEntityPtr
 *
 * Returns the new input stream or NULL
 */
hw_xmlParserInputPtr
hw_xmlNewEntityInputStream(hw_xmlParserCtxtPtr ctxt, hw_xmlEntityPtr entity) {
    hw_xmlParserInputPtr input;

    if (entity == NULL) {
        xmlErrInternal(ctxt, "hw_xmlNewEntityInputStream entity = NULL\n",
	               NULL);
	return(NULL);
    }
    if (entity->content == NULL) {
	switch (entity->etype) {
            case XML_EXTERNAL_GENERAL_UNPARSED_ENTITY:
	        xmlErrInternal(ctxt, "Cannot parse entity %s\n",
		               entity->name);
                break;
            case XML_EXTERNAL_GENERAL_PARSED_ENTITY:
            case XML_EXTERNAL_PARAMETER_ENTITY:
		return(hw_xmlLoadExternalEntity((char *) entity->URI,
		       (char *) entity->ExternalID, ctxt));
            case XML_INTERNAL_GENERAL_ENTITY:
	        xmlErrInternal(ctxt,
		      "Internal entity %s without content !\n",
		               entity->name);
                break;
            case XML_INTERNAL_PARAMETER_ENTITY:
	        xmlErrInternal(ctxt,
		      "Internal parameter entity %s without content !\n",
		               entity->name);
                break;
            case XML_INTERNAL_PREDEFINED_ENTITY:
	        xmlErrInternal(ctxt,
		      "Predefined entity %s without content !\n",
		               entity->name);
                break;
	}
	return(NULL);
    }
    input = hw_xmlNewInputStream(ctxt);
    if (input == NULL) {
	return(NULL);
    }
    input->filename = (char *) entity->URI;
    input->base = entity->content;
    input->cur = entity->content;
    input->length = entity->length;
    input->end = &entity->content[input->length];
    return(input);
}

/**
 * hw_xmlNewInputFromFile:
 * @ctxt:  an XML parser context
 * @filename:  the filename to use as entity
 *
 * Create a new input stream based on a file or an URL.
 *
 * Returns the new input stream or NULL in case of error
 */
hw_xmlParserInputPtr
hw_xmlNewInputFromFile(hw_xmlParserCtxtPtr ctxt, const char *filename) {
    hw_xmlParserInputBufferPtr buf;
    hw_xmlParserInputPtr inputStream;
    char *directory = NULL;
    hw_xmlChar *URI = NULL;

    if (ctxt == NULL) return(NULL);
    buf = hw_xmlParserInputBufferCreateFilename(filename, XML_CHAR_ENCODING_NONE);
    if (buf == NULL) {
	if (filename == NULL)
	    hw___xmlLoaderErr(ctxt,
	                   "failed to load external entity: NULL filename \n",
			   NULL);
	else
	    hw___xmlLoaderErr(ctxt, "failed to load external entity \"%s\"\n",
			   (const char *) filename);
	return(NULL);
    }

    inputStream = hw_xmlNewInputStream(ctxt);
    if (inputStream == NULL)
	return(NULL);

    inputStream->buf = buf;
    
    if (inputStream->filename == NULL)
	URI = hw_xmlStrdup((hw_xmlChar *) filename);
    else
	URI = hw_xmlStrdup((hw_xmlChar *) inputStream->filename);
    directory = hw_xmlParserGetDirectory((const char *) URI);
    inputStream->filename = (char *) hw_xmlCanonicPath((const hw_xmlChar *) URI);
    if (URI != NULL) hw_xmlFree((char *) URI);
    inputStream->directory = directory;

    inputStream->base = inputStream->buf->buffer->content;
    inputStream->cur = inputStream->buf->buffer->content;
    inputStream->end = &inputStream->base[inputStream->buf->buffer->use];
    if ((ctxt->directory == NULL) && (directory != NULL))
        ctxt->directory = (char *) hw_xmlStrdup((const hw_xmlChar *) directory);
    return(inputStream);
}

/************************************************************************
 *									*
 *		Commodity functions to handle parser contexts		*
 *									*
 ************************************************************************/

/**
 * hw_xmlInitParserCtxt:
 * @ctxt:  an XML parser context
 *
 * Initialize a parser context
 *
 * Returns 0 in case of success and -1 in case of error
 */

int
hw_xmlInitParserCtxt(hw_xmlParserCtxtPtr ctxt)
{
    hw_xmlParserInputPtr input;

    if(ctxt==NULL) {
        xmlErrInternal(NULL, "Got NULL parser context\n", NULL);
        return(-1);
    }

    if (ctxt->dict == NULL)
	ctxt->dict = hw_xmlDictCreate();
    if (ctxt->dict == NULL) {
        hw_xmlErrMemory(NULL, "cannot initialize parser context\n");
	return(-1);
    }
    if (ctxt->sax == NULL)
	ctxt->sax = (hw_xmlSAXHandler *) hw_xmlMalloc(sizeof(hw_xmlSAXHandler));
    if (ctxt->sax == NULL) {
        hw_xmlErrMemory(NULL, "cannot initialize parser context\n");
	return(-1);
    }
    else
        hw_xmlSAXVersion(ctxt->sax, 2);

    ctxt->maxatts = 0;
    ctxt->atts = NULL;
    /* Allocate the Input stack */
    if (ctxt->inputTab == NULL) {
	ctxt->inputTab = (hw_xmlParserInputPtr *)
		    hw_xmlMalloc(5 * sizeof(hw_xmlParserInputPtr));
	ctxt->inputMax = 5;
    }
    if (ctxt->inputTab == NULL) {
        hw_xmlErrMemory(NULL, "cannot initialize parser context\n");
	ctxt->inputNr = 0;
	ctxt->inputMax = 0;
	ctxt->input = NULL;
	return(-1);
    }
    while ((input = hw_inputPop(ctxt)) != NULL) { /* Non consuming */
        hw_xmlFreeInputStream(input);
    }
    ctxt->inputNr = 0;
    ctxt->input = NULL;

    ctxt->version = NULL;
    ctxt->encoding = NULL;
    ctxt->standalone = -1;
    ctxt->hasExternalSubset = 0;
    ctxt->hasPErefs = 0;
    ctxt->html = 0;
    ctxt->external = 0;
    ctxt->instate = XML_PARSER_START;
    ctxt->token = 0;
    ctxt->directory = NULL;

    /* Allocate the Node stack */
    if (ctxt->nodeTab == NULL) {
	ctxt->nodeTab = (hw_xmlNodePtr *) hw_xmlMalloc(10 * sizeof(hw_xmlNodePtr));
	ctxt->nodeMax = 10;
    }
    if (ctxt->nodeTab == NULL) {
        hw_xmlErrMemory(NULL, "cannot initialize parser context\n");
	ctxt->nodeNr = 0;
	ctxt->nodeMax = 0;
	ctxt->node = NULL;
	ctxt->inputNr = 0;
	ctxt->inputMax = 0;
	ctxt->input = NULL;
	return(-1);
    }
    ctxt->nodeNr = 0;
    ctxt->node = NULL;

    /* Allocate the Name stack */
    if (ctxt->nameTab == NULL) {
	ctxt->nameTab = (const hw_xmlChar **) hw_xmlMalloc(10 * sizeof(hw_xmlChar *));
	ctxt->nameMax = 10;
    }
    if (ctxt->nameTab == NULL) {
        hw_xmlErrMemory(NULL, "cannot initialize parser context\n");
	ctxt->nodeNr = 0;
	ctxt->nodeMax = 0;
	ctxt->node = NULL;
	ctxt->inputNr = 0;
	ctxt->inputMax = 0;
	ctxt->input = NULL;
	ctxt->nameNr = 0;
	ctxt->nameMax = 0;
	ctxt->name = NULL;
	return(-1);
    }
    ctxt->nameNr = 0;
    ctxt->name = NULL;

    /* Allocate the space stack */
    if (ctxt->spaceTab == NULL) {
	ctxt->spaceTab = (int *) hw_xmlMalloc(10 * sizeof(int));
	ctxt->spaceMax = 10;
    }
    if (ctxt->spaceTab == NULL) {
        hw_xmlErrMemory(NULL, "cannot initialize parser context\n");
	ctxt->nodeNr = 0;
	ctxt->nodeMax = 0;
	ctxt->node = NULL;
	ctxt->inputNr = 0;
	ctxt->inputMax = 0;
	ctxt->input = NULL;
	ctxt->nameNr = 0;
	ctxt->nameMax = 0;
	ctxt->name = NULL;
	ctxt->spaceNr = 0;
	ctxt->spaceMax = 0;
	ctxt->space = NULL;
	return(-1);
    }
    ctxt->spaceNr = 1;
    ctxt->spaceMax = 10;
    ctxt->spaceTab[0] = -1;
    ctxt->space = &ctxt->spaceTab[0];
    ctxt->userData = ctxt;
    ctxt->myDoc = NULL;
    ctxt->wellFormed = 1;
    ctxt->nsWellFormed = 1;
    ctxt->valid = 1;
    ctxt->linenumbers = 0;
    ctxt->keepBlanks = 1;

    ctxt->vctxt.finishDtd = hw_XML_CTXT_FINISH_DTD_0;
    ctxt->vctxt.userData = ctxt;
    ctxt->vctxt.error = hw_xmlParserValidityError;
    ctxt->vctxt.warning = hw_xmlParserValidityWarning;
    ctxt->replaceEntities = 0;
    ctxt->record_info = 0;
    ctxt->nbChars = 0;
    ctxt->checkIndex = 0;
    ctxt->inSubset = 0;
    ctxt->errNo = XML_ERR_OK;
    ctxt->depth = 0;
    ctxt->charset = XML_CHAR_ENCODING_UTF8;
    ctxt->catalogs = NULL;
    hw_xmlInitNodeInfoSeq(&ctxt->node_seq);
    return(0);
}

/**
 * hw_xmlFreeParserCtxt:
 * @ctxt:  an XML parser context
 *
 * Free all the memory used by a parser context. However the parsed
 * document in ctxt->myDoc is not freed.
 */

void
hw_xmlFreeParserCtxt(hw_xmlParserCtxtPtr ctxt)
{
    hw_xmlParserInputPtr input;

    if (ctxt == NULL) return;

    while ((input = hw_inputPop(ctxt)) != NULL) { /* Non consuming */
        hw_xmlFreeInputStream(input);
    }
    if (ctxt->spaceTab != NULL) hw_xmlFree(ctxt->spaceTab);
    if (ctxt->nameTab != NULL) hw_xmlFree((hw_xmlChar * *)ctxt->nameTab);
    if (ctxt->nodeTab != NULL) hw_xmlFree(ctxt->nodeTab);
    if (ctxt->inputTab != NULL) hw_xmlFree(ctxt->inputTab);
    if (ctxt->version != NULL) hw_xmlFree((char *) ctxt->version);
    if (ctxt->encoding != NULL) hw_xmlFree((char *) ctxt->encoding);
    if (ctxt->sax != NULL)
        hw_xmlFree(ctxt->sax);
    if (ctxt->directory != NULL) hw_xmlFree((char *) ctxt->directory);
    if (ctxt->vctxt.nodeTab != NULL) hw_xmlFree(ctxt->vctxt.nodeTab);
    if (ctxt->atts != NULL) hw_xmlFree((hw_xmlChar * *)ctxt->atts);
    if (ctxt->dict != NULL) hw_xmlDictFree(ctxt->dict);
    if (ctxt->nsTab != NULL) hw_xmlFree((char *) ctxt->nsTab);
    if (ctxt->pushTab != NULL) hw_xmlFree(ctxt->pushTab);
    if (ctxt->attallocs != NULL) hw_xmlFree(ctxt->attallocs);
    if (ctxt->freeElems != NULL) {
        hw_xmlNodePtr cur, next;

	cur = ctxt->freeElems;
	while (cur != NULL) {
	    next = cur->next;
	    hw_xmlFree(cur);
	    cur = next;
	}
    }
    if (ctxt->freeAttrs != NULL) {
        hw_xmlAttrPtr cur, next;

	cur = ctxt->freeAttrs;
	while (cur != NULL) {
	    next = cur->next;
	    hw_xmlFree(cur);
	    cur = next;
	}
    }
    /*
     * cleanup the error strings
     */
    if (ctxt->lastError.message != NULL)
        hw_xmlFree(ctxt->lastError.message);
    if (ctxt->lastError.file != NULL)
        hw_xmlFree(ctxt->lastError.file);
    if (ctxt->lastError.str1 != NULL)
        hw_xmlFree(ctxt->lastError.str1);
    if (ctxt->lastError.str2 != NULL)
        hw_xmlFree(ctxt->lastError.str2);
    if (ctxt->lastError.str3 != NULL)
        hw_xmlFree(ctxt->lastError.str3);

    hw_xmlFree(ctxt);
}

/**
 * hw_xmlNewParserCtxt:
 *
 * Allocate and initialize a new parser context.
 *
 * Returns the hw_xmlParserCtxtPtr or NULL
 */

hw_xmlParserCtxtPtr
hw_xmlNewParserCtxt(void)
{
    hw_xmlParserCtxtPtr ctxt;
    ctxt = (hw_xmlParserCtxtPtr) hw_xmlMalloc(sizeof(hw_xmlParserCtxt));
    if (ctxt == NULL) {
	hw_xmlErrMemory(NULL, "cannot allocate parser context\n");
	return(NULL);
    }
    memset(ctxt, 0, sizeof(hw_xmlParserCtxt));
    if (hw_xmlInitParserCtxt(ctxt) < 0) {
        hw_xmlFreeParserCtxt(ctxt);
	return(NULL);
    }
    return(ctxt);
}

/************************************************************************
 *									*
 *		Handling of node informations				*
 *									*
 ************************************************************************/


/**
 * hw_xmlInitNodeInfoSeq:
 * @seq:  a node info sequence pointer
 *
 * -- Initialize (set to initial state) node info sequence
 */
void
hw_xmlInitNodeInfoSeq(hw_xmlParserNodeInfoSeqPtr seq)
{
    if (seq == NULL)
        return;
    seq->length = 0;
    seq->maximum = 0;
    seq->buffer = NULL;
}

/**
 * hw_xmlClearNodeInfoSeq:
 * @seq:  a node info sequence pointer
 *
 * -- Clear (release memory and reinitialize) node
 *   info sequence
 */
void
hw_xmlClearNodeInfoSeq(hw_xmlParserNodeInfoSeqPtr seq)
{
    if (seq == NULL)
        return;
    if (seq->buffer != NULL)
        hw_xmlFree(seq->buffer);
    hw_xmlInitNodeInfoSeq(seq);
}

/**
 * hw_xmlParserFindNodeInfoIndex:
 * @seq:  a node info sequence pointer
 * @node:  an XML node pointer
 *
 * 
 * hw_xmlParserFindNodeInfoIndex : Find the index that the info record for
 *   the given node is or should be at in a sorted sequence
 *
 * Returns a long indicating the position of the record
 */
unsigned long
hw_xmlParserFindNodeInfoIndex(const hw_xmlParserNodeInfoSeqPtr seq,
                           const hw_xmlNodePtr node)
{
    unsigned long upper, lower, middle;
    int found = 0;

    if ((seq == NULL) || (node == NULL))
        return ((unsigned long) -1);

    /* Do a binary search for the key */
    lower = 1;
    upper = seq->length;
    middle = 0;
    while (lower <= upper && !found) {
        middle = lower + (upper - lower) / 2;
        if (node == seq->buffer[middle - 1].node)
            found = 1;
        else if (node < seq->buffer[middle - 1].node)
            upper = middle - 1;
        else
            lower = middle + 1;
    }

    /* Return position */
    if (middle == 0 || seq->buffer[middle - 1].node < node)
        return middle;
    else
        return middle - 1;
}


/**
 * hw_xmlParserAddNodeInfo:
 * @ctxt:  an XML parser context
 * @info:  a node info sequence pointer
 *
 * Insert node info record into the sorted sequence
 */
void
hw_xmlParserAddNodeInfo(hw_xmlParserCtxtPtr ctxt,
                     const hw_xmlParserNodeInfoPtr info)
{
    unsigned long pos;

    if ((ctxt == NULL) || (info == NULL)) return;

    /* Find pos and check to see if node is already in the sequence */
    pos = hw_xmlParserFindNodeInfoIndex(&ctxt->node_seq, (hw_xmlNodePtr)
                                     info->node);

    if ((pos < ctxt->node_seq.length) && 
        (ctxt->node_seq.buffer != NULL) &&
        (ctxt->node_seq.buffer[pos].node == info->node)) {
        ctxt->node_seq.buffer[pos] = *info;
    }

    /* Otherwise, we need to add new node to buffer */
    else {
        if (ctxt->node_seq.length + 1 > ctxt->node_seq.maximum) {
            hw_xmlParserNodeInfo *tmp_buffer;
            unsigned int byte_size;

            if (ctxt->node_seq.maximum == 0)
                ctxt->node_seq.maximum = 2;
            byte_size = (sizeof(*ctxt->node_seq.buffer) *
			(2 * ctxt->node_seq.maximum));

            if (ctxt->node_seq.buffer == NULL)
                tmp_buffer = (hw_xmlParserNodeInfo *) hw_xmlMalloc(byte_size);
            else
                tmp_buffer =
                    (hw_xmlParserNodeInfo *) hw_xmlRealloc(ctxt->node_seq.buffer,
                                                     byte_size);

            if (tmp_buffer == NULL) {
		hw_xmlErrMemory(ctxt, "failed to allocate buffer\n");
                return;
            }
            ctxt->node_seq.buffer = tmp_buffer;
            ctxt->node_seq.maximum *= 2;
        }

        /* If position is not at end, move elements out of the way */
        if (pos != ctxt->node_seq.length) {
            unsigned long i;

            for (i = ctxt->node_seq.length; i > pos; i--)
                ctxt->node_seq.buffer[i] = ctxt->node_seq.buffer[i - 1];
        }

        /* Copy element and increase length */
        ctxt->node_seq.buffer[pos] = *info;
        ctxt->node_seq.length++;
    }
}

