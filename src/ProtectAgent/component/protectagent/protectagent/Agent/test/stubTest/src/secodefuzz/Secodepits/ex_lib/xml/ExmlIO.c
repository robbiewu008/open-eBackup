/*
 * xmlIO.c : implementation of the I/O interfaces used by the parser
 *
 * See Copyright for the status of this software.
 *
 * daniel@veillard.com
 *
 * 14 Nov 2000 ht - for VMS, truncated name of long functions to under 32 char
 */

#define IN_LIBXML


#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

/* Figure a portable way to know if a file is a directory. */
#ifndef HAVE_STAT
#  ifdef HAVE__STAT
     /* MS C library seems to define stat and _stat. The definition
        is identical. Still, mapping them to each other causes a warning. */
#    ifndef _MSC_VER
#      define stat(x,y) _stat(x,y)
#    endif
#    define HAVE_STAT
#  endif
#endif
#ifdef HAVE_STAT
#  ifndef S_ISDIR
#    ifdef _S_ISDIR
#      define S_ISDIR(x) _S_ISDIR(x)
#    else
#      ifdef S_IFDIR
#        ifndef S_IFMT
#          ifdef _S_IFMT
#            define S_IFMT _S_IFMT
#          endif
#        endif
#        ifdef S_IFMT
#          define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#        endif
#      endif
#    endif
#  endif
#endif

#include "Eparser.h"
#include "Etree.h"

/* #define VERBOSE_FAILURE */
/* #define DEBUG_EXTERNAL_ENTITIES */


#define MINLEN 4000


/*
 * Input I/O callback sets
 */
typedef struct _xmlInputCallback {
    hw_xmlInputMatchCallback matchcallback;
    hw_xmlInputOpenCallback opencallback;
    hw_xmlInputReadCallback readcallback;
    hw_xmlInputCloseCallback closecallback;
} hw_xmlInputCallback;

#define hw_MAX_INPUT_CALLBACK 15

static hw_xmlInputCallback hw_xmlInputCallbackTable[hw_MAX_INPUT_CALLBACK];
static int hw_xmlInputCallbackNr = 0;
static int hw_xmlInputCallbackInitialized = 0;


/************************************************************************
 *									*
 * 		Tree memory error handler				*
 *									*
 ************************************************************************/

static const char *hw_IOerr[] = {
    "Unknown IO error",         /* UNKNOWN */
    "Permission denied",	/* EACCES */
    "Resource temporarily unavailable",/* EAGAIN */
    "Bad file descriptor",	/* EBADF */
    "Bad message",		/* EBADMSG */
    "Resource busy",		/* EBUSY */
    "Operation canceled",	/* ECANCELED */
    "No child processes",	/* ECHILD */
    "Resource deadlock avoided",/* EDEADLK */
    "Domain error",		/* EDOM */
    "File exists",		/* EEXIST */
    "Bad address",		/* EFAULT */
    "File too large",		/* EFBIG */
    "Operation in progress",	/* EINPROGRESS */
    "Interrupted function call",/* EINTR */
    "Invalid argument",		/* EINVAL */
    "Input/output error",	/* EIO */
    "Is a directory",		/* EISDIR */
    "Too many open files",	/* EMFILE */
    "Too many links",		/* EMLINK */
    "Inappropriate message buffer length",/* EMSGSIZE */
    "Filename too long",	/* ENAMETOOLONG */
    "Too many open files in system",/* ENFILE */
    "No such device",		/* ENODEV */
    "No such file or directory",/* ENOENT */
    "Exec format error",	/* ENOEXEC */
    "No locks available",	/* ENOLCK */
    "Not enough space",		/* ENOMEM */
    "No space left on device",	/* ENOSPC */
    "Function not implemented",	/* ENOSYS */
    "Not a directory",		/* ENOTDIR */
    "Directory not empty",	/* ENOTEMPTY */
    "Not supported",		/* ENOTSUP */
    "Inappropriate I/O control operation",/* ENOTTY */
    "No such device or address",/* ENXIO */
    "Operation not permitted",	/* EPERM */
    "Broken pipe",		/* EPIPE */
    "Result too large",		/* ERANGE */
    "Read-only file system",	/* EROFS */
    "Invalid seek",		/* ESPIPE */
    "No such process",		/* ESRCH */
    "Operation timed out",	/* ETIMEDOUT */
    "Improper link",		/* EXDEV */
    "Attempt to load network entity %s", /* XML_IO_NETWORK_ATTEMPT */
    "encoder error",		/* XML_IO_ENCODER */
    "flush error",
    "write error",
    "no input",
    "buffer full",
    "loading error",
    "not a socket",		/* ENOTSOCK */
    "already connected",	/* EISCONN */
    "connection refused",	/* ECONNREFUSED */
    "unreachable network",	/* ENETUNREACH */
    "adddress in use",		/* EADDRINUSE */
    "already in use",		/* EALREADY */
    "unknown address familly",	/* EAFNOSUPPORT */
};

#if defined(WIN32) || defined (__DJGPP__) && !defined (__CYGWIN__)
/**
 * hw___xmlIOWin32UTF8ToWChar:
 * @u8String:  uft-8 string
 *
 * Convert a string from utf-8 to wchar (WINDOWS ONLY!)
 */
static wchar_t *
hw___xmlIOWin32UTF8ToWChar(const char *u8String)
{
	wchar_t *wString = NULL;

	if (u8String)
	{
		int wLen = MultiByteToWideChar(CP_UTF8,0,u8String,-1,NULL,0);
		if (wLen)
		{
			wString = malloc((wLen+1) * sizeof(wchar_t));
			if (wString)
			{
				if (MultiByteToWideChar(CP_UTF8,0,u8String,-1,wString,wLen+1) == 0)
				{
					free(wString);
					wString = NULL;
				}
			}
		}
	}
	
	return wString;
}
#endif

/**
 * hw_xmlIOErrMemory:
 * @extra:  extra informations
 *
 * Handle an out of memory condition
 */
static void
hw_xmlIOErrMemory(const char *extra)
{
    hw___xmlSimpleError(XML_FROM_IO, XML_ERR_NO_MEMORY, NULL, NULL, extra);
}

/**
 * hw___xmlIOErr:
 * @code:  the error number
 * @
 * @extra:  extra informations
 *
 * Handle an I/O error
 */
void
hw___xmlIOErr(int domain, int code, const char *extra)
{
    unsigned int idx;

    if (code == 0) {
    }
    idx = 0;
    if (code >= XML_IO_UNKNOWN) idx = code - XML_IO_UNKNOWN;
    if (idx >= (sizeof(hw_IOerr) / sizeof(hw_IOerr[0]))) idx = 0;
    
    hw___xmlSimpleError(domain, code, NULL, hw_IOerr[idx], extra);
}

/**
 * hw_xmlIOErr:
 * @code:  the error number
 * @extra:  extra informations
 *
 * Handle an I/O error
 */
static void
hw_xmlIOErr(int code, const char *extra)
{
    hw___xmlIOErr(XML_FROM_IO, code, extra);
}

/**
 * hw___xmlLoaderErr:
 * @ctx: the parser context
 * @extra:  extra informations
 *
 * Handle a resource access error
 */
void
hw___xmlLoaderErr(void *ctx, const char *msg, const char *filename)
{
    hw_xmlParserCtxtPtr ctxt = (hw_xmlParserCtxtPtr) ctx;
    hw_xmlStructuredErrorFunc schannel = NULL;
    hw_xmlGenericErrorFunc channel = NULL;
    void *data = NULL;
    hw_xmlErrorLevel level = XML_ERR_ERROR;

    if ((ctxt != NULL) && (ctxt->disableSAX != 0) &&
        (ctxt->instate == XML_PARSER_EOF))
	return;
    if ((ctxt != NULL) && (ctxt->sax != NULL)) {
         {
	    channel = ctxt->sax->warning;
	    level = XML_ERR_WARNING;
	}
	if (ctxt->sax->initialized == hw_XML_SAX2_MAGIC)
	    schannel = ctxt->sax->serror;
	data = ctxt->userData;
    }
    hw___xmlRaiseError(schannel, channel, data, ctxt, NULL, XML_FROM_IO,
                    XML_IO_LOAD_ERROR, level, NULL, 0,
		    filename, NULL, NULL, 0, 0,
		    msg, filename);
                    
}

/************************************************************************
 *									*
 * 		Tree memory error handler				*
 *									*
 ************************************************************************/

/**
 * hw_xmlCleanupInputCallbacks:
 *
 * clears the entire input callback table. this includes the
 * compiled-in I/O. 
 */
void
hw_xmlCleanupInputCallbacks(void)
{
    int i;

    if (!hw_xmlInputCallbackInitialized)
        return;

    for (i = hw_xmlInputCallbackNr - 1; i >= 0; i--) {
        hw_xmlInputCallbackTable[i].matchcallback = NULL;
        hw_xmlInputCallbackTable[i].opencallback = NULL;
        hw_xmlInputCallbackTable[i].readcallback = NULL;
        hw_xmlInputCallbackTable[i].closecallback = NULL;
    }

    hw_xmlInputCallbackNr = 0;
    hw_xmlInputCallbackInitialized = 0;
}



/************************************************************************
 *									*
 *		Standard I/O for file accesses				*
 *									*
 ************************************************************************/

/**
 * hw_xmlCheckFilename:
 * @path:  the path to check
 *
 * function checks to see if @path is a valid source
 * (file, socket...) for XML.
 *
 * if stat is not available on the target machine,
 * returns 1.  if stat fails, returns 0 (if calling
 * stat on the filename fails, it can't be right).
 * if stat succeeds and the file is a directory,
 * returns 2.  otherwise returns 1.
 */

int
hw_xmlCheckFilename (const char *path)
{
#ifdef HAVE_STAT
	struct stat stat_buffer;
#endif
	if (path == NULL)
		return(0);
  
#if defined(WIN32) || defined (__DJGPP__) && !defined (__CYGWIN__)
	{
		int retval = 0;
	
		wchar_t *wPath = hw___xmlIOWin32UTF8ToWChar(path);
		if (wPath)
		{
			struct _stat stat_buffer;
			
			if (_wstat(wPath,&stat_buffer) == 0)
			{
				retval = 1;
				
				if (((stat_buffer.st_mode & S_IFDIR) == S_IFDIR))
					retval = 2;
			}
	
			free(wPath);
		}

		return retval;
	}
#else
#ifdef HAVE_STAT
    if (stat(path, &stat_buffer) == -1)
        return 0;

#ifdef S_ISDIR
    if (S_ISDIR(stat_buffer.st_mode))
        return 2;
#endif /* S_ISDIR */
#endif /* HAVE_STAT */
#endif /* WIN32 */

    return 1;
}

static int
hw_xmlNop(void) {
    return(0);
}


/**
 * hw_xmlFileMatch:
 * @filename:  the URI for matching
 *
 * input from FILE *
 *
 * Returns 1 if matches, 0 otherwise
 */
int
hw_xmlFileMatch (const char *filename ATTRIBUTE_UNUSED) {
    return(1);
}

/**
 * hw_xmlFileOpen_real:
 * @filename:  the URI for matching
 *
 * input from FILE *, supports compressed input
 * if @filename is " " then the standard input is used
 *
 * Returns an I/O context or NULL in case of error
 */
static void *
hw_xmlFileOpen_real (const char *filename) {
    const char *path = NULL;
    FILE *fd;

    if (filename == NULL)
        return(NULL);

    if (!strcmp(filename, "-")) {
	fd = stdin;
	return((void *) fd);
    }

    if (!hw_xmlStrncasecmp(hw_BAD_CAST filename, hw_BAD_CAST "file://localhost/", 17))
#if defined (_WIN32) || defined (__DJGPP__) && !defined(__CYGWIN__)
	path = &filename[17];
#else
	path = &filename[16];
#endif
    else if (!hw_xmlStrncasecmp(hw_BAD_CAST filename, hw_BAD_CAST "file:///", 8)) {
#if defined (_WIN32) || defined (__DJGPP__) && !defined(__CYGWIN__)
	path = &filename[8];
#else
	path = &filename[7];
#endif
    } else 
	path = filename;

    if (path == NULL)
	return(NULL);
    if (!hw_xmlCheckFilename(path))
        return(NULL);

#if defined(WIN32) || defined (__DJGPP__) && !defined (__CYGWIN__)
	{
		wchar_t *wPath = hw___xmlIOWin32UTF8ToWChar(path);
		if (wPath)
		{
			fd = _wfopen(wPath, L"rb");
			free(wPath);
   	}
   	else
   	{
   	   fd = fopen(path, "rb");
	   }
	}	
#else
    fd = fopen(path, "r");
#endif /* WIN32 */
    if (fd == NULL) hw_xmlIOErr(0, path);
    return((void *) fd);
}

/**
 * hw_xmlFileOpen:
 * @filename:  the URI for matching
 *
 * Wrapper around hw_xmlFileOpen_real that try it with an unescaped
 * version of @filename, if this fails fallback to @filename
 *
 * Returns a handler or NULL in case or failure
 */
void *
hw_xmlFileOpen (const char *filename) {
    char *unescaped;
    void *retval;

    unescaped = hw_xmlURIUnescapeString(filename, 0, NULL);
    if (unescaped != NULL) {
	retval = hw_xmlFileOpen_real(unescaped);
	hw_xmlFree(unescaped);
    } else {
	retval = hw_xmlFileOpen_real(filename);
    }
    return retval;
}


/**
 * hw_xmlFileRead:
 * @context:  the I/O context
 * @buffer:  where to drop data
 * @len:  number of bytes to write
 *
 * Read @len bytes to @buffer from the I/O channel.
 *
 * Returns the number of bytes written or < 0 in case of failure
 */
int
hw_xmlFileRead (void * context, char * buffer, int len) {
    int ret;
    if ((context == NULL) || (buffer == NULL)) 
        return(-1);
    ret = fread(&buffer[0], 1,  len, (FILE *) context);
    if (ret < 0) hw_xmlIOErr(0, "fread()");
    return(ret);
}


/**
 * hw_xmlFileClose:
 * @context:  the I/O context
 *
 * Close an I/O channel
 *
 * Returns 0 or -1 in case of error
 */
int
hw_xmlFileClose (void * context) {
    FILE *fil;
    int ret;

    if (context == NULL)
        return(-1);
    fil = (FILE *) context;
    if ((fil == stdout) || (fil == stderr)) {
        ret = fflush(fil);
	if (ret < 0)
	    hw_xmlIOErr(0, "fflush()");
	return(0);
    }
    if (fil == stdin)
	return(0);
    ret = ( fclose((FILE *) context) == EOF ) ? -1 : 0;
    if (ret < 0)
        hw_xmlIOErr(0, "fclose()");
    return(ret);
}


/**
 * hw_xmlRegisterInputCallbacks:
 * @matchFunc:  the hw_xmlInputMatchCallback
 * @openFunc:  the hw_xmlInputOpenCallback
 * @readFunc:  the hw_xmlInputReadCallback
 * @closeFunc:  the hw_xmlInputCloseCallback
 *
 * Register a new set of I/O callback for handling parser input.
 *
 * Returns the registered handler number or -1 in case of error
 */
int
hw_xmlRegisterInputCallbacks(hw_xmlInputMatchCallback matchFunc,
	hw_xmlInputOpenCallback openFunc, hw_xmlInputReadCallback readFunc,
	hw_xmlInputCloseCallback closeFunc) {
    if (hw_xmlInputCallbackNr >= hw_MAX_INPUT_CALLBACK) {
	return(-1);
    }
    hw_xmlInputCallbackTable[hw_xmlInputCallbackNr].matchcallback = matchFunc;
    hw_xmlInputCallbackTable[hw_xmlInputCallbackNr].opencallback = openFunc;
    hw_xmlInputCallbackTable[hw_xmlInputCallbackNr].readcallback = readFunc;
    hw_xmlInputCallbackTable[hw_xmlInputCallbackNr].closecallback = closeFunc;
    hw_xmlInputCallbackInitialized = 1;
    return(hw_xmlInputCallbackNr++);
}


/**
 * hw_xmlRegisterDefaultInputCallbacks:
 *
 * Registers the default compiled-in I/O handlers.
 */
void
hw_xmlRegisterDefaultInputCallbacks
(void) {
    if (hw_xmlInputCallbackInitialized)
	return;

    hw_xmlRegisterInputCallbacks(hw_xmlFileMatch, hw_xmlFileOpen,
	                      hw_xmlFileRead, hw_xmlFileClose);

    hw_xmlInputCallbackInitialized = 1;
}


/**
 * hw_xmlAllocParserInputBuffer:
 * @enc:  the charset encoding if known
 *
 * Create a buffered parser input for progressive parsing
 *
 * Returns the new parser input or NULL
 */
hw_xmlParserInputBufferPtr
hw_xmlAllocParserInputBuffer(hw_xmlCharEncoding enc) {
    hw_xmlParserInputBufferPtr ret;

    ret = (hw_xmlParserInputBufferPtr) hw_xmlMalloc(sizeof(hw_xmlParserInputBuffer));
    if (ret == NULL) {
	hw_xmlIOErrMemory("creating input buffer");
	return(NULL);
    }
    memset(ret, 0, (size_t) sizeof(hw_xmlParserInputBuffer));
    ret->buffer = hw_xmlBufferCreateSize(2 * hw_xmlDefaultBufferSize);
    if (ret->buffer == NULL) {
        hw_xmlFree(ret);
	return(NULL);
    }
    ret->buffer->alloc = XML_BUFFER_ALLOC_DOUBLEIT;
    ret->encoder = hw_xmlGetCharEncodingHandler(enc);
    if (ret->encoder != NULL)
        ret->raw = hw_xmlBufferCreateSize(2 * hw_xmlDefaultBufferSize);
    else
        ret->raw = NULL;
    ret->readcallback = NULL;
    ret->closecallback = NULL;
    ret->context = NULL;
    ret->compressed = -1;
    ret->rawconsumed = 0;

    return(ret);
}


/**
 * hw_xmlFreeParserInputBuffer:
 * @in:  a buffered parser input
 *
 * Free up the memory used by a buffered parser input
 */
void
hw_xmlFreeParserInputBuffer(hw_xmlParserInputBufferPtr in) {
    if (in == NULL) return;

    if (in->raw) {
        hw_xmlBufferFree(in->raw);
	in->raw = NULL;
    }
    if (in->encoder != NULL) {
        hw_xmlCharEncCloseFunc(in->encoder);
    }
    if (in->closecallback != NULL) {
	in->closecallback(in->context);
    }
    if (in->buffer != NULL) {
        hw_xmlBufferFree(in->buffer);
	in->buffer = NULL;
    }

    hw_xmlFree(in);
}


hw_xmlParserInputBufferPtr
hw___xmlParserInputBufferCreateFilename(const char *URI, hw_xmlCharEncoding enc) {
    hw_xmlParserInputBufferPtr ret;
    int i = 0;
    void *context = NULL;

    if (hw_xmlInputCallbackInitialized == 0)
	hw_xmlRegisterDefaultInputCallbacks();

    if (URI == NULL) return(NULL);

    /*
     * Try to find one of the input accept method accepting that scheme
     * Go in reverse to give precedence to user defined handlers.
     */
    if (context == NULL) {
	for (i = hw_xmlInputCallbackNr - 1;i >= 0;i--) {
	    if ((hw_xmlInputCallbackTable[i].matchcallback != NULL) &&
		(hw_xmlInputCallbackTable[i].matchcallback(URI) != 0)) {
		context = hw_xmlInputCallbackTable[i].opencallback(URI);
		if (context != NULL) {
		    break;
		}
	    }
	}
    }
    if (context == NULL) {
	return(NULL);
    }

    /*
     * Allocate the Input buffer front-end.
     */
    ret = hw_xmlAllocParserInputBuffer(enc);
    if (ret != NULL) {
	ret->context = context;
	ret->readcallback = hw_xmlInputCallbackTable[i].readcallback;
	ret->closecallback = hw_xmlInputCallbackTable[i].closecallback;
    }
    else
      hw_xmlInputCallbackTable[i].closecallback (context);

    return(ret);
}

/**
 * hw_xmlParserInputBufferCreateFilename:
 * @URI:  a C string containing the URI or filename
 * @enc:  the charset encoding if known
 *
 * Create a buffered parser input for the progressive parsing of a file
 * If filename is "-' then we use stdin as the input.
 * Automatic support for ZLIB/Compress compressed document is provided
 * by default if found at compile-time.
 * Do an encoding check if enc == XML_CHAR_ENCODING_NONE
 *
 * Returns the new parser input or NULL
 */
hw_xmlParserInputBufferPtr
hw_xmlParserInputBufferCreateFilename(const char *URI, hw_xmlCharEncoding enc) {
	return hw___xmlParserInputBufferCreateFilename(URI, enc);
}



/**
 * hw_xmlParserInputBufferCreateMem:
 * @mem:  the memory input
 * @size:  the length of the memory block
 * @enc:  the charset encoding if known
 *
 * Create a buffered parser input for the progressive parsing for the input
 * from a memory area.
 *
 * Returns the new parser input or NULL
 */
hw_xmlParserInputBufferPtr
hw_xmlParserInputBufferCreateMem(const char *mem, int size, hw_xmlCharEncoding enc) {
    hw_xmlParserInputBufferPtr ret;
    int errcode;

    if (size <= 0) return(NULL);
    if (mem == NULL) return(NULL);

    ret = hw_xmlAllocParserInputBuffer(enc);
    if (ret != NULL) {
        ret->context = (void *) mem;
	ret->readcallback = (hw_xmlInputReadCallback) hw_xmlNop;
	ret->closecallback = NULL;
	errcode = hw_xmlBufferAdd(ret->buffer, (const hw_xmlChar *) mem, size);
	if (errcode != 0) {
	    hw_xmlFree(ret);
	    return(NULL);
	}
    }

    return(ret);
}

/**
 * hw_xmlParserInputBufferCreateStatic:
 * @mem:  the memory input
 * @size:  the length of the memory block
 * @enc:  the charset encoding if known
 *
 * Create a buffered parser input for the progressive parsing for the input
 * from an immutable memory area. This will not copy the memory area to
 * the buffer, but the memory is expected to be available until the end of
 * the parsing, this is useful for example when using mmap'ed file.
 *
 * Returns the new parser input or NULL
 */
hw_xmlParserInputBufferPtr
hw_xmlParserInputBufferCreateStatic(const char *mem, int size,
                                 hw_xmlCharEncoding enc) {
    hw_xmlParserInputBufferPtr ret;

    if (size <= 0) return(NULL);
    if (mem == NULL) return(NULL);

    ret = (hw_xmlParserInputBufferPtr) hw_xmlMalloc(sizeof(hw_xmlParserInputBuffer));
    if (ret == NULL) {
	hw_xmlIOErrMemory("creating input buffer");
	return(NULL);
    }
    memset(ret, 0, (size_t) sizeof(hw_xmlParserInputBuffer));
    ret->buffer = hw_xmlBufferCreateStatic((void *)mem, (size_t) size);
    if (ret->buffer == NULL) {
        hw_xmlFree(ret);
	return(NULL);
    }
    ret->encoder = hw_xmlGetCharEncodingHandler(enc);
    if (ret->encoder != NULL)
        ret->raw = hw_xmlBufferCreateSize(2 * hw_xmlDefaultBufferSize);
    else
        ret->raw = NULL;
    ret->compressed = -1;
    ret->context = (void *) mem;
    ret->readcallback = NULL;
    ret->closecallback = NULL;

    return(ret);
}



/**
 * hw_xmlParserInputBufferPush:
 * @in:  a buffered parser input
 * @len:  the size in bytes of the array.
 * @buf:  an char array
 *
 * Push the content of the arry in the input buffer
 * This routine handle the I18N transcoding to internal UTF-8
 * This is used when operating the parser in progressive (push) mode.
 *
 * Returns the number of chars read and stored in the buffer, or -1
 *         in case of error.
 */
int
hw_xmlParserInputBufferPush(hw_xmlParserInputBufferPtr in,
	                 int len, const char *buf) {
    int nbchars = 0;
    int ret;

    if (len < 0) return(0);
    if ((in == NULL) || (in->error)) return(-1);
    if (in->encoder != NULL) {
        unsigned int use;

        /*
	 * Store the data in the incoming raw buffer
	 */
        if (in->raw == NULL) {
	    in->raw = hw_xmlBufferCreate();
	}
	ret = hw_xmlBufferAdd(in->raw, (const hw_xmlChar *) buf, len);
	if (ret != 0)
	    return(-1);

	/*
	 * convert as much as possible to the parser reading buffer.
	 */
	use = in->raw->use;
	nbchars = hw_xmlCharEncInFunc(in->encoder, in->buffer, in->raw);
	if (nbchars < 0) {
	    hw_xmlIOErr(XML_IO_ENCODER, NULL);
	    in->error = XML_IO_ENCODER;
	    return(-1);
	}
	in->rawconsumed += (use - in->raw->use);
    } else {
	nbchars = len;
        ret = hw_xmlBufferAdd(in->buffer, (hw_xmlChar *) buf, nbchars);
	if (ret != 0)
	    return(-1);
    }
    return(nbchars);
}

/**
 * hw_endOfInput:
 *
 * When reading from an Input channel indicated end of file or error
 * don't reread from it again.
 */
static int
hw_endOfInput (void * context ATTRIBUTE_UNUSED,
	    char * buffer ATTRIBUTE_UNUSED,
	    int len ATTRIBUTE_UNUSED) {
    return(0);
}

/**
 * hw_xmlParserInputBufferGrow:
 * @in:  a buffered parser input
 * @len:  indicative value of the amount of chars to read
 *
 * Grow up the content of the input buffer, the old data are preserved
 * This routine handle the I18N transcoding to internal UTF-8
 * This routine is used when operating the parser in normal (pull) mode
 *
 * TODO: one should be able to remove one extra copy by copying directly
 *       onto in->buffer or in->raw
 *
 * Returns the number of chars read and stored in the buffer, or -1
 *         in case of error.
 */
int
hw_xmlParserInputBufferGrow(hw_xmlParserInputBufferPtr in, int len) {
    char *buffer = NULL;
    int res = 0;
    int nbchars = 0;
    int buffree;
    unsigned int needSize;

    if ((in == NULL) || (in->error)) return(-1);
    if ((len <= MINLEN) && (len != 4))
        len = MINLEN;

    buffree = in->buffer->size - in->buffer->use;
    if (buffree <= 0) {
	hw_xmlIOErr(XML_IO_BUFFER_FULL, NULL);
	in->error = XML_IO_BUFFER_FULL;
	return(-1);
    }

    needSize = in->buffer->use + len + 1;
    if (needSize > in->buffer->size){
        if (!hw_xmlBufferResize(in->buffer, needSize)){
	    hw_xmlIOErrMemory("growing input buffer");
	    in->error = XML_ERR_NO_MEMORY;
            return(-1);
        }
    }
    buffer = (char *)&in->buffer->content[in->buffer->use];

    /*
     * Call the read method for this I/O type.
     */
    if (in->readcallback != NULL) {
	res = in->readcallback(in->context, &buffer[0], len);
	if (res <= 0)
	    in->readcallback = hw_endOfInput;
    } else {
	hw_xmlIOErr(XML_IO_NO_INPUT, NULL);
	in->error = XML_IO_NO_INPUT;
	return(-1);
    }
    if (res < 0) {
	return(-1);
    }
    len = res;
    if (in->encoder != NULL) {
        unsigned int use;

        /*
	 * Store the data in the incoming raw buffer
	 */
        if (in->raw == NULL) {
	    in->raw = hw_xmlBufferCreate();
	}
	res = hw_xmlBufferAdd(in->raw, (const hw_xmlChar *) buffer, len);
	if (res != 0)
	    return(-1);

	/*
	 * convert as much as possible to the parser reading buffer.
	 */
	use = in->raw->use;
	nbchars = hw_xmlCharEncInFunc(in->encoder, in->buffer, in->raw);
	if (nbchars < 0) {
	    hw_xmlIOErr(XML_IO_ENCODER, NULL);
	    in->error = XML_IO_ENCODER;
	    return(-1);
	}
	in->rawconsumed += (use - in->raw->use);
    } else {
	nbchars = len;
    	in->buffer->use += nbchars;
	buffer[nbchars] = 0;
    }
    return(nbchars);
}

/**
 * hw_xmlParserInputBufferRead:
 * @in:  a buffered parser input
 * @len:  indicative value of the amount of chars to read
 *
 * Refresh the content of the input buffer, the old data are considered
 * consumed
 * This routine handle the I18N transcoding to internal UTF-8
 *
 * Returns the number of chars read and stored in the buffer, or -1
 *         in case of error.
 */
int
hw_xmlParserInputBufferRead(hw_xmlParserInputBufferPtr in, int len) {
    if ((in == NULL) || (in->error)) return(-1);
    if (in->readcallback != NULL)
	return(hw_xmlParserInputBufferGrow(in, len));
    else if ((in->buffer != NULL) &&
             (in->buffer->alloc == XML_BUFFER_ALLOC_IMMUTABLE))
	return(0);
    else
        return(-1);
}


/**
 * hw_xmlParserGetDirectory:
 * @filename:  the path to a file
 *
 * lookup the directory for that file
 *
 * Returns a new allocated string containing the directory, or NULL.
 */
char *
hw_xmlParserGetDirectory(const char *filename) {
    char *ret = NULL;
    char dir[1024];
    char *cur;
    char sep = '/';

#ifdef _WIN32_WCE  /* easy way by now ... wince does not have dirs! */
    return NULL;
#endif

    if (hw_xmlInputCallbackInitialized == 0)
	hw_xmlRegisterDefaultInputCallbacks();

    if (filename == NULL) return(NULL);
#if defined(WIN32) && !defined(__CYGWIN__)
    sep = '\\';
#endif

    strncpy(dir, filename, 1023);
    dir[1023] = 0;
    cur = &dir[strlen(dir)];
    while (cur > dir) {
         if (*cur == sep) break;
	 cur --;
    }
    if (*cur == sep) {
        if (cur == dir) dir[1] = 0;
	else *cur = 0;
	ret = (char*)hw_xmlMemStrdup((hw_xmlChar *)dir);
    } else {
       // if (getcwd(dir, 1024) != NULL) {
	//    dir[1023] = 0;
	//    ret = hw_xmlMemStrdup(dir);
	//}
    }
    return(ret);
}

/****************************************************************
 *								*
 *		External entities loading			*
 *								*
 ****************************************************************/


static int hw_xmlSysIDExists(const char *URL) {
#ifdef HAVE_STAT
    int ret;
    struct stat info;
    const char *path;

    if (URL == NULL)
	return(0);

    if (!hw_xmlStrncasecmp(hw_BAD_CAST URL, hw_BAD_CAST "file://localhost/", 17))
#if defined (_WIN32) || defined (__DJGPP__) && !defined(__CYGWIN__)
	path = &URL[17];
#else
	path = &URL[16];
#endif
    else if (!hw_xmlStrncasecmp(hw_BAD_CAST URL, hw_BAD_CAST "file:///", 8)) {
#if defined (_WIN32) || defined (__DJGPP__) && !defined(__CYGWIN__)
	path = &URL[8];
#else
	path = &URL[7];
#endif
    } else 
	path = URL;
    ret = stat(path, &info);
    if (ret == 0)
	return(1);
#endif
    return(0);
}

/**
 * hw_xmlDefaultExternalEntityLoader:
 * @URL:  the URL for the entity to load
 * @ID:  the System ID for the entity to load
 * @ctxt:  the context in which the entity is called or NULL
 *
 * By default we don't load external entitites, yet.
 *
 * Returns a new allocated hw_xmlParserInputPtr, or NULL.
 */
static hw_xmlParserInputPtr
hw_xmlDefaultExternalEntityLoader(const char *URL, const char *ID,
                               hw_xmlParserCtxtPtr ctxt)
{
    hw_xmlParserInputPtr ret = NULL;
    hw_xmlChar *resource = NULL;



    if (resource == NULL)
        resource = (hw_xmlChar *) URL;

    if (resource == NULL) {
        if (ID == NULL)
            ID = "NULL";
        hw___xmlLoaderErr(ctxt, "failed to load external entity \"%s\"\n", ID);
        return (NULL);
    }
    ret = hw_xmlNewInputFromFile(ctxt, (const char *) resource);
    if ((resource != NULL) && (resource != (hw_xmlChar *) URL))
        hw_xmlFree(resource);
    return (ret);
}

static hw_xmlExternalEntityLoader hw_xmlCurrentExternalEntityLoader =
       hw_xmlDefaultExternalEntityLoader;



/**
 * hw_xmlLoadExternalEntity:
 * @URL:  the URL for the entity to load
 * @ID:  the Public ID for the entity to load
 * @ctxt:  the context in which the entity is called or NULL
 *
 * Load an external entity, note that the use of this function for
 * unparsed entities may generate problems
 *
 * Returns the hw_xmlParserInputPtr or NULL
 */
hw_xmlParserInputPtr
hw_xmlLoadExternalEntity(const char *URL, const char *ID,
                      hw_xmlParserCtxtPtr ctxt) {
    if ((URL != NULL) && (hw_xmlSysIDExists(URL) == 0)) {
	char *canonicFilename;
	hw_xmlParserInputPtr ret;

	canonicFilename = (char *) hw_xmlCanonicPath((const hw_xmlChar *) URL);
	if (canonicFilename == NULL) {
            hw_xmlIOErrMemory("building canonical path\n");
	    return(NULL);
	}

	ret = hw_xmlCurrentExternalEntityLoader(canonicFilename, ID, ctxt);
	hw_xmlFree(canonicFilename);
	return(ret);
    }
    return(hw_xmlCurrentExternalEntityLoader(URL, ID, ctxt));
}

