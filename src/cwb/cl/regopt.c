/*
 *  IMS Open Corpus Workbench (CWB)
 *  Copyright (C) 1993-2006 by IMS, University of Stuttgart
 *  Copyright (C) 2007-     by the respective contributers (see file AUTHORS)
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2, or (at your option) any later
 *  version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 *  Public License for more details (in the file "COPYING", or available via
 *  WWW at http://www.gnu.org/copyleft/gpl.html).
 */
/*
 *  Windows/Unicode-compatibility extensions to CWB in this file
 *  Copyright (C) 2010      by ANR Textométrie, ENS de Lyon
 */

/**
 * @file
 *
 * This file contains the Optimised Regular Expression object, which wraps around  PCRE.
 */


#include <glib.h>

/* include external regular expression library */

#ifndef PCRE2_CODE_UNIT_WIDTH
#define PCRE2_CODE_UNIT_WIDTH 8
#endif
#include <pcre2.h>

#include "globals.h"
void Rprintf(const char *, ...);


/**
 * Maximum number of grains of optimisation.
 *
 * There's no point in scanning for too many grains, but some regexps used to be bloody inefficient.
 */
#define MAX_GRAINS 12

/**
 * Underlying structure for CL_Regex object.
 *
 * @see regopt.c
 */
struct _cl_regex {
  pcre2_code *needle;                      /**< buffer for the actual regex object (PCRE) */
  /* pcre_extra *extra; */                /**< buffer for PCRE's internal optimisation data */
  pcre2_match_data *mdata;           /* patched */
  uint32_t options;
  CorpusCharset charset;             /**< the character set in use for this regex */
  int icase;                         /**< whether IGNORE_CASE flag was set for this regex (needs special processing) */
  int idiac;                         /**< whether IGNORE_DIAC flag was set for this regex */
  char *haystack_buf;                /**< a buffer of size CL_MAX_LINE_LENGTH used for accent folding by cl_regex_match(),
                                          allocated only if IGNORE_DIAC was specified */
  char *haystack_casefold;           /**< additional, larger (2 * CL_MAX_LINE_LENGTH) buffer for a case-folded version,
                                          allocated only if optimizer is active and IGNORE_CASE was specified */
  /* Note: these buffers are for the string being tested, NOT for the regular expression.
   * They are allocated once here to avoid frequent small allocation and deallocations in cl_regex_match(). */

  /* data from optimiser (see global variables in regopt.c for comments) */
  int grains;                        /**< number of grains (0 = not optimised). @see cl_regopt_grains */
  int grain_len;                     /**< @see cl_regopt_grain_len */
  char *grain[MAX_GRAINS];           /**< @see cl_regopt_grain */
  int anchor_start;                  /**< @see cl_regopt_anchor_start */
  int anchor_end;                    /**< @see cl_regopt_anchor_end */
  int jumptable[256];                /**< @see cl_regopt_jumptable @see make_jump_table */
};


/**
 * @file
 *
 * The CL_Regex object, and the CL Regular Expression Optimiser.
 *
 * This is the CL front-end to POSIX regular expressions with CL semantics
 * (most notably: CL regexes always match the entire string and NOT
 * substrings.)
 *
 * Note that the optimiser is handled automatically by the CL_Regex object.
 *
 * All variables / functions containing "regopt" are internal to this
 * module and are not exported in the CL API.
 *
 * Optimisation is done by means of "grains". The grain array in a CL_Regex
 * object is a list of short strings. Any string which will match the
 * regex must contain at least one of these. Thus, the grains
 * provide a quick way of filtering out strings that definitely WON'T
 * match, and avoiding a time-wasting call to the POSIX regex
 * matching function.
 *
 * While a regex is being optimised, the grains are stored in non-exported
 * global variables in this module. Subsequently they are transferred to
 * members of the CL_regex object with which they are associated.
 * The use of global variables and a fixed-size buffer for
 * grains is partly due to historical reasons,
 * but it does also serve to reduce memory allocation overhead.
 */

/* optimiser variables */

char *cl_regopt_grain[MAX_GRAINS]; /**< list of 'grains' (any matching string must contain one of these) */
int cl_regopt_grain_len;           /**< length of shortest grain (in characters) */
int cl_regopt_grains;              /**< number of grains */
int cl_regopt_anchor_start;        /**< Boolean: whether grains are anchored at beginning of string */
int cl_regopt_anchor_end;          /**< Boolean: whether grains are anchored at end of string */

/** whether the regular expression is in UTF-8 encoding (set before calling cl_regopt_analyse()) */
int cl_regopt_utf8;


/**
 * Intermediate buffer for grains.
 *
 * When a regex is parsed, grains for each segment are written to this intermediate buffer;
 * if the new set of grains is better than the current one, it is copied to the cl_regopt_ variables.
 */
char *grain_buffer[MAX_GRAINS];   /**< grains in the local buffer may have different lengths */
int grain_buffer_len[MAX_GRAINS]; /**< the length of each grain (in characters) */
/** The number of grains currently in the intermediate buffer. @see grain_buffer */
int grain_buffer_grains = 0;

/** A buffer for grain strings. @see local_grain_data */
char public_grain_data[CL_MAX_LINE_LENGTH]; /* input regexp shouldn't be longer than CL_MAX_LINE_LENGTH, so all grains must fit;
                                             * RE() operator can create GIANT regexen, so cl_new_regex() only runs the optimiser
                                             * iff the length of the regex is less than CL_MAX_LINE_LENGTH. */

/** A buffer for grain strings. @see public_grain_data */
char local_grain_data[CL_MAX_LINE_LENGTH];

/**
 * A counter of how many times the "grain" system has allwoed us to avoid
 * calling the regex engine.
 *
 * @see cl_regopt_count_get
 */
int cl_regopt_successes = 0;


/*
 * interface functions (ie "public methods" of CL_Regex)
 */

/**
 * The error message from (PCRE) regex compilation are placed in this buffer
 * if cl_new_regex() fails.
 *
 * This global variable is part of the CL_Regex object's API.
 */
char cl_regex_error[CL_MAX_LINE_LENGTH];


/*
 * static prototypes
 */
/* static void regopt_data_copy_to_regex_object(CL_Regex rx); */
/* static int cl_regopt_analyse(char *regex); */

/**
 * Create a new CL_regex object (ie a regular expression buffer).
 *
 * This function compiles the regular expression according to the specified flags
 * (IGNORE_CASE and/or IGNORE_DIAC and/or REQUIRE_NFC) and for the specified
 * character encoding. The regex is automatically anchored to the start and end of
 * the string (i.e. wrapped in ^(?:...)$).
 *
 * The regular expression engine used is PCRE. However, the regex is optimized by
 * scanning it for literal strings ("grains") that must be contained in any
 * match; the grains can be used as a fast pre-filter (using Boyer-Moore search
 * for the grains).
 *
 * The optimizer only understands a subset of PCRE syntax:
 *  - literal characters (alphanumeric, safe punctuation, escaped punctuation)
 *  - numeric character codes (\\x and \\o)
 *  - escape sequences for character classes and Unicode properties
 *  - all repetition operators
 *  - simple alternatives (...|...|...)
 *  - nested capturing (...) and non-capturing (?:...) groups
 * Any regexp that contains other syntactic elements such as
 *  - character sets [...]
 *  - named groups, look-ahead and look-behind patterns, etc.
 *  - backreferences
 *  - modifiers such as (?i)
 * cannot be parsed and optimized. Note that even if a regexp is parsed by
 * the optimizer, it might not be able to extract all grains (because grain
 * recognition uses an even more restrictive syntax).
 *
 * The optimizer is always disabled with IGNORE_DIAC if either PCRE JIT is
 * available or the charset is UTF-8. Testing has showed that in these cases
 * the overhead from case-folding each input string outweighs the benefits
 * of the optimizer.
 *
 * @param regex    String containing the regular expression
 * @param flags    IGNORE_CASE, or IGNORE_DIAC, or both, or 0.
 * @param charset  The character set of the regex.
 * @return         The new CL_Regex object, or NULL in case of error.
 */
CL_Regex
cl_new_regex(char *regex, int flags, CorpusCharset charset)
{
  /* allocate buffers dynamically to support very long regexps (from RE() operator) */
  char *delatexed_regex;
  char *preprocessed_regex;
  char *anchored_regex;
  PCRE2_SIZE length_regex;

  CL_Regex rx;
  int /* optimised, */ l;

  uint32_t  options_for_pcre2 = 0; /* pcre2 patch (previously: int)*/
  /* const char *errstring_for_pcre2 = NULL; */
  int errorcode_for_pcre2; /* patched */
  PCRE2_SIZE erroffset_for_pcre2 = (PCRE2_SIZE)0; /* pcre2 patch */
  int is_pcre2_jit_available; /* in some cases, PCRE + JIT may be faster without the optimizer */


  /* allocate temporary strings */
  l = strlen(regex);
  delatexed_regex = (char *) cl_malloc(l + 1);

  /* allocate and initialise CL_Regex object */
  rx = (CL_Regex) cl_malloc(sizeof(struct _cl_regex));
  rx->haystack_buf = NULL;
  rx->haystack_casefold = NULL;
  rx->charset = charset;
  rx->icase = (flags & IGNORE_CASE); /* handled separately in CWB 3.4.10+ */
  rx->idiac = (flags & IGNORE_DIAC);
  rx->grains = 0; /* indicates no optimisation -> other optimizer-related fields are invalid */

  /* pre-process regular expression (translate latex escapes, normalize, fold accents if required) */
  cl_string_latex2iso(regex, delatexed_regex, l);
  /* only fold accents at this stage */
  preprocessed_regex = cl_string_canonical(delatexed_regex, charset, rx->idiac | REQUIRE_NFC, CL_STRING_CANONICAL_STRDUP);
  cl_free(delatexed_regex);

  /* add start and end anchors to improve performance of regex matcher for expressions such as ".*ung" */
  int regex_len = strlen(preprocessed_regex) + 7;
  anchored_regex = (char *) cl_malloc(regex_len);
  snprintf(anchored_regex, regex_len, "^(?:%s)$", preprocessed_regex);
  length_regex = (PCRE2_SIZE)strlen(anchored_regex);

  /* compile regular expression with PCRE library function */
  options_for_pcre2 = PCRE2_UCP; /* use Unicode properties for \w, \d, etc. */
  if (rx->icase)
    options_for_pcre2 |= PCRE2_CASELESS; /* case folding is left to the PCRE matcher */
  if (charset == utf8) {
    if (cl_debug)
      Rprintf("CL: enabling PCRE2's UTF8 mode for regex %s\n", anchored_regex);
    /* note we assume all strings have been checked upon input (i.e. indexing or by the parser) */
    /* options_for_pcre2 |= PCRE_UTF8|PCRE_NO_UTF8_CHECK; */
  }
  rx->needle = pcre2_compile((PCRE2_SPTR)anchored_regex, length_regex, options_for_pcre2, &errorcode_for_pcre2, &erroffset_for_pcre2, NULL);
  rx->options = options_for_pcre2; /* patched */
  if (rx->needle == NULL) {
    /* strcpy(cl_regex_error, errstring_for_pcre2); */
    Rprintf("CL: Regex Compile Error: %d\n", errorcode_for_pcre2);
    cl_free(rx);
    cl_free(preprocessed_regex);
    cl_free(anchored_regex);
    cl_errno = CDA_EBADREGEX;
    return NULL;
  } else if (cl_debug) {
    Rprintf("CL: Regex compiled successfully using PCRE2 library\n");
  }
    


  /* a spot of code to handle use with a pre-JIT version of PCRE.
   * Note that JIT compilation was added to PCRE v8.20. */
  #ifdef PCRE2_CONFIG_JIT
    pcre2_config(PCRE2_CONFIG_JIT, &is_pcre2_jit_available);
  #else
    is_pcre2_jit_available = 0;
  /* #define PCRE_STUDY_JIT_COMPILE 0 */
  #endif
  if (cl_debug)
    Rprintf("CL: PCRE's JIT compiler is %s.\n", (is_pcre2_jit_available ? "available" : "unavailable"));

  /* always use pcre_study because nearly all our regexes are going to be used lots of times;
   * with recent version of PCRE, this will also JIT-compile the expression for much faster matching */
  /* rx->extra = pcre_study(rx->needle, PCRE_STUDY_JIT_COMPILE, &errstring_for_pcre2); */
  /* if (errstring_for_pcre2 != NULL) { */
    /* rx->extra = NULL; */
    /* if (cl_debug) */
      /* Rprintf("CL: calling pcre_study failed with message...\n   %s\n", errstring_for_pcre2); */
    /* note that failure of pcre_study is not a critical error, we can just continue without the extra info */
  /* } */
  /* if (cl_debug && rx->extra) */
    /* Rprintf("CL: calling pcre_study produced useful information...\n"); */

  /* attempt to optimise regular expression */
  /* cl_regopt_utf8 = (charset == utf8); */
  /* if (CL_MAX_LINE_LENGTH <= (l+1)) */
    /* optimised = cl_regopt_analyse(preprocessed_regex); */
  /* else */
    /* optimised = 0; */   /* IE: avoid optimisation if the regex is longer than the grain buffer. */

  /* decide whether it makes sense to use the optimizer:
   *  - testing showed that it is usually faster to rely directly on PCRE's caseless matching than
   *    casefold each input string for a Boyer-Moore search
   *  - this is always the case if PCRE has JIT capability
   *  - without JIT, it is still usually better to avoid expensive UTF-8 case-folding
   * NB: accent-folding for %d cannot be avoided, so it makes no sense to disable the optimizer there
   * NB: cannot debug the optimizer with %c any more (because regopt_data_copy_to_regex_object will not be called)
   */
  /* if (rx->icase && (charset == utf8 || is_pcre2_jit_available)) { */
    /* if (optimised && cl_debug) { */
    /*  int i; */
    /*  Rprintf("CL: Found grain set with %d items(s)", cl_regopt_grains); */
    /*  for (i = 0; i < cl_regopt_grains; i++) { */
    /*    Rprintf(" [%s]", cl_regopt_grain[i]); */
    /*  } */
    /*  Rprintf("\nCL: but optimization disabled for case-insensitive search\n"); */
    /* } */
    /* optimised = 0; */
  /* } */

  /* if (optimised) { */
    /* copy optimiser data to CL_Regex object and construct jump table for Boyer-Moore search */
    /* regopt_data_copy_to_regex_object(rx);  */ /* will also casefold grains if rx->icase is set */
  /* } */

  if (rx->idiac)
    /* allocate string buffer for accent folding in cl_regex_match() */
    rx->haystack_buf = (char *) cl_malloc(CL_MAX_LINE_LENGTH); /* this is for the string being matched, not the regex! */
  
  if (rx->icase /* && optimised */)
    /* allocate second buffer for case-folded version (only needed for optimizer) */
    rx->haystack_casefold = (char *) cl_malloc(2 * CL_MAX_LINE_LENGTH);
  
  rx->mdata = pcre2_match_data_create_from_pattern(rx->needle, NULL); /* patched */

  cl_free(preprocessed_regex);
  cl_free(anchored_regex);
  cl_errno = CDA_OK;
  return rx;
}

/**
 * Finds the level of optimisation of a CL_Regex.
 *
 * This function returns the approximate level of optimisation,
 * computed from the ratio of grain length to number of grains
 * (0 = no grains, ergo not optimised at all).
 *
 * @param rx  The CL_Regex to check.
 * @return    0 if rx is not optimised; otherwise an integer
 *            indicating optimisation level.
 */
int
cl_regex_optimised(CL_Regex rx)
{
  if (rx->grains == 0)
    return 0; /* not optimised */
  else {
    int level = (3 * rx->grain_len) / rx->grains;
    return ((level >= 1) ? level + 1 : 1);
  }
}

/**
 * Matches a regular expression against a string.
 *
 * The pre-compiled regular expression contained in the CL_Regex is compared to the string.
 * This regex automatically uses the case/accent folding flags and character encoding
 * that were specified when the CL_Regex constructor was called.
 *
 * If the subject string is a UTF-8 string from an external sources, the caller can request
 * enforcement of the subject to canonical NFC form by setting the third argument to true.
 *
 * @see   cl_new_regex
 * @param rx              The regular expression to match.
 * @param str             The subject (the string to compare the regex to).
 * @param normalize_utf8  Boolean: if a UTF-8 string from an external source is passed as subject,
 *                        set to this parameter to true, and the function will make sure that
 *                        the comparison is based on the canonical NFC form. For known-NFC
 *                        strings, this parameter should be false. If the regex is not UTF-8,
 *                        this parameter is ignored.
 * @return                Boolean: true if the regex matched, otherwise false.
 */
int
cl_regex_match(CL_Regex rx, char *str, int normalize_utf8)
{
  char *haystack_pcre2, *haystack; /* possibly case/accent folded versions of str for PCRE regexp and optimizer, respectively */
  int optimised = (rx->grains > 0);
  int i, di, k, max_i, /* len, */ jump; /* pcre2 */
  PCRE2_SIZE len;
  PCRE2_SIZE startoffset = (PCRE2_SIZE)0;
  int grain_match, result;
  /* int ovector[30]; */ /* memory for pcre to use for back-references in pattern matches */
  int do_nfc = (normalize_utf8 && (rx->charset == utf8)) ? REQUIRE_NFC : 0; /* whether we need to normalize the input to NFC */

  if (rx->idiac || do_nfc) { /* perform accent folding on input string if necessary */
    haystack_pcre2 = rx->haystack_buf;
    cl_strcpy(haystack_pcre2, str);
    cl_string_canonical(haystack_pcre2, rx->charset, rx->idiac | do_nfc, CL_MAX_LINE_LENGTH);
  }
  else
    haystack_pcre2 = str;
  
  len = (PCRE2_SIZE)strlen(haystack_pcre2); /* patched */

  /* Beta versions 3.4.10+ leading up to 3.5:
   *  - use regexp optimizer only if cl_optimize is set
   *  - allows comparative testing & benchmarking
   *  - question: is the optimizer still worth the effort for PCRE with JIT?
   *  - TODO switch optimizer back to default before release
   */
  if (optimised && cl_optimize) {
    if (rx->icase) {
      haystack = rx->haystack_casefold;
      cl_strcpy(haystack, haystack_pcre2);
      cl_string_canonical(haystack, rx->charset, rx->icase, 2 * CL_MAX_LINE_LENGTH);
    }
    else
      haystack = haystack_pcre2;
    /* this 'optimised' matcher may look fairly complicated, but it's still way ahead of POSIX regexen */
    /* string offset where first character of each grain would be */
    grain_match = 0;
    max_i = len - rx->grain_len; /* stop trying to match when i > max_i */
    if (rx->anchor_end)
      i = (max_i >= 0) ? max_i : 0; /* if anchored at end, align grains with end of string */
    else
      i = 0;

    while (i <= max_i) {
      jump = rx->jumptable[(unsigned char) haystack[i + rx->grain_len - 1]];
      if (jump > 0)
        i += jump; /* Boyer-Moore search */
      else {
        /* for each grain */
        for (k = 0; k < rx->grains; k++) {
          di = 0;
          while ((di < rx->grain_len) && (rx->grain[k][di] == haystack[i + di]))
            di++;
          if (di >= rx->grain_len) {
            grain_match = 1;
            break; /* we have found a grain match and can quit the loop */
          }
        }
        i++;
      }
      if (rx->anchor_start)
        break; /* if anchored at start, only the first iteration can match */
    }
  } /* endif optimised */
  else
    /* if the regex is not optimised, always behave as if a grain was matched */
    grain_match = 1;

  /* if there was a grain-match, we call pcre_exec, which might match or might not find a match in the end;
   * but if there wasn't a grain-match, we know that PCRE won't match; so we don't bother calling it. */

  if (!grain_match) { /* enabled since version 2.2.b94 (14 Feb 2006) -- before: && cl_optimize */
    cl_regopt_successes++;
    result = PCRE2_ERROR_NOMATCH;  /* the return code from PCRE when there is, um, no match */
  }
#if 1
  /* set to 0 for debug purposes: always calls PCRE regardless of whether grains matched. */
  /* this allows the code in the #if 1 below to check whether or not grains are behaving as they should. */
  else {
#else
  if (1) {
#endif
    result = pcre2_match(rx->needle, /* rx->extra, */ (PCRE2_SPTR)haystack_pcre2,
                       len, startoffset, (uint32_t)0,
                       rx->mdata, NULL);
    if (result < PCRE2_ERROR_NOMATCH && cl_debug)
      /* note, "no match" is a PCRE2 "error", but all actual errors are lower numbers */
      Rprintf("CL: Regex Execute Error no. %d (see `man pcreapi` for error codes)\n", result);
  }


#if 1
  /* debugging code used before version 2.2.b94, modified to pcre return values & re-enabled in 3.2.b3 */
  /* check for critical error: optimiser didn't accept candidate, but regex matched */
  if ((result > 0) && !grain_match)
    Rprintf("CL ERROR: regex optimiser did not accept '%s' although it should have!\n", str);
#endif

  return (result > 0); /* return true if regular expression matched */
}

/**
 * Deletes a CL_Regex object, and frees all resources associated with
 * the pre-compiled regex.
 *
 * @param rx  The CL_Regex to delete. Null-safe.
 */
void
cl_delete_regex(CL_Regex rx)
{
  /* DON'T use cl_free() for PCRE opaque objects, just in case; use PCRE built-in
   * pcre_free(). Note this will probably just be set to = free(). But it might not.
   * We can let PCRE worry about that. That does mean, however, we should test the
   * pointers for non-nullity before calling pcre_free. Normally we would also set the
   * pointers to NULL after freeing the target. However, in this case, we know the
   * structure they belong to will be freed by the end of the function, so no worries.
   */
  int i;

  /* sanity check for NULL pointer */
  if (!rx)
    return;

  if (rx->needle)
    pcre2_code_free(rx->needle);         /* free PCRE regex buffer */
  /* if (rx->extra) */
/* #ifdef PCRE_CONFIG_JIT */
    /* pcre_free_study(rx->extra); */    /* and "extra" buffer (iff JIT was a possibility)*/
/* #else */
    /* pcre_free(rx->extra); */          /* and "extra" buffer (iff we know for certain there was no JIT) */
/* #endif */
  
  if (rx->mdata)
    pcre2_match_data_free(rx->mdata);

  cl_free(rx->haystack_buf);       /* free string buffers if they were allocated */
  cl_free(rx->haystack_casefold);
  for (i = 0; i < rx->grains; i++)
    cl_free(rx->grain[i]);         /* free grain strings if regex was optimised */

  cl_free(rx);
}

/*
 * ================================
 * helper functions (for optimiser)
 * (non-exported in the public API)
 * ================================
 */

/**
 * Is the given character a 'safe' character which will only match itself in a regex?
 *
 * What counts as safe: A to Z, a to z, 0 to 9, minus, quote marks, percent,
 * ampersand, slash, excl mark, colon, semi colon, character, underscore, tilde,
 * any values above 0x7f (ISO 8859 extension or UTF-8 non-ASCII character).
 *
 * What counts as not safe therefore includes: brackets, braces, square brackets;
 * questionmark, plus, and star; circumflex and dollar sign; dot; hash; backslash, etc.
 * (But, in UTF8, Unicode PUNC area equivalents of these characters will be safe.)
 *
 * A safe character can never be the start of a meta element (even if it might appear
 * as part of one), so it's safe to include in a literal grain.
 *
 * @param c  The character (cast to unsigned for the comparison).
 * @return   True for non-special characters; false for special characters.
 */
int
is_safe_char(unsigned char c)
{
  /* note: this function is UTF8-safe because byte values above 0x7f
   * (forming UTF-8 multi-byte sequences) are always allowed */
  if (
      (c >= 'A' && c <= 'Z') ||
      (c >= 'a' && c <= 'z') ||
      (c >= '0' && c <= '9') ||
      (c >= 128) /* & (c <= 255); omitted to avoid compiler warnings */
  ) {
    return 1;
  }
  else {
    switch (c) {
    case '!':
    case '"':
    case '#':
    case '%':
    case '&':
    case '\'':
    case ',':
    case '-':
    case '/':
    case ':':
    case ';':
    case '<':
    case '=':
    case '>':
    case '@':
    case '_':
    case '`':
    case '~':
      return 1;
    default:
      return 0;
    }
  }
}

/**
 * Is the given character an ASCII alphanumeric?
 *
 * ASCII alphanumeric characters comprise A-Z, a-z and 0-9; they are the only
 * characters that form special escape sequences in PCRE regular expressions.
 *
 * @param c  The character (cast to unsigned for the comparison).
 * @return   True if ASCII alphanumeric; false otherwise.
 */
int
is_ascii_alnum(unsigned char c) {
  if ((c >= 'A' && c <= 'Z') ||
      (c >= 'a' && c <= 'z') ||
      (c >= '0' && c <= '9')) {
    return 1;
  }
  else {
    return 0;
  }
}

/**
 * Is the given character ASCII punctuation?
 *
 * ASCII punctuation symbols are the only characters that may need to be protected
 * by a \ in regular expressions. They cannot form special escape sequences.
 *
 * @param c  The character (cast to unsigned for the comparison).
 * @return   True if ASCII alphanumeric; false otherwise.
 */
int
is_ascii_punct(unsigned char c) {
  switch (c) {
  case '!':
  case '"':
  case '#':
  case '$':
  case '%':
  case '&':
  case '(':
  case '\'':
  case ')':
  case '*':
  case '+':
  case ',':
  case '-':
  case '.':
  case '/':
  case ':':
  case ';':
  case '<':
  case '=':
  case '>':
  case '?':
  case '@':
  case '[':
  case '\\':
  case ']':
  case '^':
  case '_':
  case '`':
  case '{':
  case '|':
  case '}':
  case '~':
    return 1;
  default:
    return 0;
  }
}

/**
 * Is the given character a valid hexadecimal digit?
 *
 * @param c  The character (cast to unsigned for the comparison).
 * @return   True if valid hexidecimal digit; false otherwise.
 */
int
is_hexadecimal(unsigned char c) {
  if ((c >= 'A' && c <= 'F') ||
      (c >= 'a' && c <= 'f') ||
      (c >= '0' && c <= '9')) {
    return 1;
  }
  else {
    return 0;
  }
}


/**
 * Read in an escape sequence for a character or class - part of the CL Regex Optimiser.
 *
 * This function reads one of the following escape sequences:
 *
 *     \x##, \x{###} ... hexadecimal character code
 *     \o{###}       ... octal character code
 *     \w, \W, \d, \D, \s, \S  ...  generic character types
 *     \p#, \p{###}  ... Unicode properties
 *     \P#, \P{###}  ... negated Unicode properties
 *     \X            ... Unicode extended grapheme cluster
 *
 */
char *
read_escape_seq(char *mark)
{
  char *point = mark;
  if (*point != '\\')
    return mark;
  else
    point++;
  switch (*point) {
  case 'x':
    point++;
    if (*point == '{') {
      point++;
      while (is_hexadecimal(*point))
        point++;
      if (*point == '}')
        return point + 1;
    }
    else {
      if (is_hexadecimal(point[0]) && is_hexadecimal(point[1]))
        return point + 2;
    }
    return mark; /* not recognised */
  case 'o':
    point++;
    if (*point == 'o') {
      point++;
      while (*point >= '0' && *point <= '7')
        point++;
      if (*point == '}')
        return point + 1;
    }
    return mark; /* not recognised */
  case 'w':
  case 'W':
  case 'd':
  case 'D':
  case 's':
  case 'S':
  case 'X':
    return point + 1;
  case 'p':
  case 'P':
    point++;
    if (*point >= 'A' && *point <= 'Z')
      return point + 1;
    else if (*point == '{') {
      point++;
      while (is_ascii_alnum(*point) || *point == '_' || *point == '&')
        point++;
      if (*point == '}')
        return point + 1;
    }
    return mark; /* not recognised */
  default:
    return mark; /* not recognised */
  }
}

/**
 * Reads in an element matching some character - part of the CL Regex Optimiser.
 *
 * This function reads in an element known to match some character.
 * The following elements are currently recognized:
 *  - a matchall (.)
 *  - a safe literal character or escaped ASCII punctuation
 *  - an escape sequence for a simple character class (\\w, \\d, etc.)
 *  - an escape sequence for a Unicode property
 *  - a hexadecimal (\\x) or octal (\\o) character code
 *  - a simple character set such as [a-z], [A-Z] and [0-9]
 * The precise syntax of character sets is rather messy, so we will not
 * make an attempt to recognize more complex sets.
 *
 * @param mark  Pointer to location in the regex string from
 *              which to read.
 * @return      Pointer to the first character after the character
 *              (class) it has read in (or the original "mark"
 *              pointer if nothing suitable was found).
 */
char *
read_matchall(char *mark)
{
  char *point = mark;
  char *p2;

  if (*mark == '.') {
    /* read the matchall dot */
    return mark + 1;
  }
  else if (is_safe_char(*mark)) {
    /* a literal character */
    if (cl_regopt_utf8)
      return g_utf8_next_char(mark);
    else
      return mark + 1;
  }
  else if (*mark == '\\') {
    /* check for a supported escape sequence */
    return read_escape_seq(mark);
  }
  else if (*mark == '[') {
    /* a character set */
    point = mark + 1;
    if (*point == '^')
      point++; /* negated character set */
    while (1) {
      if (is_safe_char(*point) || (*point == '-'))
        point++;
      else {
        p2 = read_escape_seq(point);
        if (p2 > point)
          point = p2;
        else
          break;
      }
    }
    if (*point == ']' && point > mark + 1)
      return point + 1; /* note that we exclude [] because of ambiguity with []..] */
  }
  return mark; /* no element has been recognized */
}

/**
 * Reads in a repetition operator - part of the CL Regex Optimiser.
 *
 * This function reads in any repetition operator allowed in PCRE syntax:
 *  - * (Kleene star), ?, +
 *  - {n}, {n,m}, {,m}, {n,}
 *  - optionally followed by a non-greedy (?) or possessive (+) modifier
 * it returns a pointer to the first character after the repetition modifier
 * it has found. If *one_or_more is not null, it is set to True if the
 * quantifier operator mandates at least one repetition (i.e. it does not
 * make the preceding element optional).
 *
 * @param mark  Pointer to location in the regex string from
 *              which to read.
 * @param one_or_more  Optional pointer to integer, which will be set
 *                     to True if repetition is not optional.
 * @return      Pointer to the first character after the star
 *              or other modifier it has read in (or the original
 *              "mark" pointer if a repetition modifier was not
 *              read).
 */
char *
read_kleene(char *mark, int *one_or_more)
{
  int plus = 0; /* repetition is non-optional, i.e. 1 or more */
  int ok = 1;   /* whether a valid quantifier has been recognized */
  char *point = mark;
  if (*point == '?' || *point == '*') {
    plus = 0;
    point++;
  }
  else if (*point == '+') {
    plus = 1;
    point++;
  }
  else if (*point == '{') {
    point++;
    plus = (*point >= '1' && *point <= '9');
    while ((*point >= '0' && *point <= '9') || (*point == ',')) {
      point++;
    }
    if (*point != '}')
      ok = 0;
    else
      point++;
  }
  else
    ok = 0;

  if (ok) {
    if (*point == '?' || *point == '+')
      point++; /* lazy or possessive quantifier */
    if (one_or_more)
      *one_or_more = plus;
    return point;
  }
  else
    return mark;
}

/**
 * Reads in a wildcard - part of the CL Regex Optimiser.
 *
 * This function reads in a wildcard segment consisting of an
 * element matching some character (read_matchall) or a capturing
 * or non-capturing group, followed by an optional quantifier (read_kleene).
 * It returns a pointer to the first character after the wildcard segment.
 *
 * Groups are parsed recursively and must consist of one or more
 * alternatives containing only valid wildcard segments.
 *
 * @param mark  Pointer to location in the regex string from which to read.
 * @return      Pointer to the first character after the
 *              wildcard segment (or the original "mark" pointer
 *              if a wildcard segment was not found).
 */
char *
read_wildcard(char *mark)
{
  char *point, *p2;
  point = read_matchall(mark);
  if (point > mark) {
    return read_kleene(point, NULL);
  }
  else if (*point == '(') {
    point++;
    if (*point == '?') {
      if (point[1] == ':')
        point += 2;  /* non-capturing group */
      else
        return mark; /* other special elements are not supported */
    }
    while (1) {
      /* body of group must consist only of wildcard segments and disjunction operators */
      p2 = read_wildcard(point);
      if (p2 > point)
        point = p2;
      else if (*point == '|')
        point++;
      else
        break;
    }
    if (*point == ')')
      return read_kleene(point + 1, NULL);
  }
  return mark; /* no wildcard segment found */
}

/**
 * Reads in a literal grain from a regex - part of the CL Regex Optimiser.
 *
 * A grain is a string of safe symbols: alphanumeric, safe punctuation and
 * escaped punctuation; numeric character codes (\\x, \\u) are not supported.
 * The last symbol might be followed by a repetition operator. It is only
 * included in the grain if the repetition count is at least one.
 *
 * This function finds the longest grain it can starting at the point
 * in the regex indicated by mark.  If *grain is not NULL, the grain data
 * are unescaped and copied to the specified buffer.  Note that the buffer
 * will be mangled even if no grain is matched; it is guaranteed to contain
 * a NUL-terminated string, though.  If *len is not null, the length of the
 * grain (in characters) is stored there.
 *
 * @param mark  Pointer to location in the regex string from
 *              which to read.
 * @param grain Optional pointer to a buffer into which the grain data
 *              will be copied. Guaranteed to contain a NUL-terminated
 *              string even if no grain is found.
 * @param len   Pointer to integer in which length of grain in characters
 *              will be stored.
 * @return      Pointer to the first character after the grain
 *              it has read in (or the original "mark" pointer
 *              if no grain is found).
 */
char *
read_grain(char *mark, char *grain, int *len)
{
  char *point = mark;
  char *grain_point = grain;
  char *grain_last_char = grain; /* pointer to start of last symbol in grain buffer */
  char *end, *q;
  int glen = 0; /* length of grain in characters */
  int one_or_more;

  /* read sequence of safe literal characters */
  while (
      is_safe_char(*point) ||
      (*point == '\\' && is_ascii_punct(point[1]))
  ) {
    if (*point == '\\') {
      /* copy escaped character to grain */
      if (grain) {
        grain_last_char = grain_point;
        *grain_point++ = point[1];
      }
      glen++;
      point += 2;
    }
    else {
      /* copy complete character to grain */
      end = (cl_regopt_utf8) ? g_utf8_next_char(point) : point + 1;
      if (grain) {
        grain_last_char = grain_point;
        for (q = point; q < end; q++) {
          *grain_point++ = *q;
        }
      }
      glen++;
      point = end;
    }
  }

  if (glen > 0) {
    /* check for quantifier on last symbol of grain */
    end = read_kleene(point, &one_or_more);
    if (end > point) {
      /* remove last symbol from grain if made optional by quantifier */
      if (!one_or_more) {
        glen--;
        if (grain)
          grain_point = grain_last_char;
      }
    }
    point = end;
  }

  if (grain)
    *grain_point = '\0'; /* NUL-terminated extracted grain */
  if (len)
    *len = glen;

  if (glen > 0)
    return point;
  else
    return mark;
}

/**
 * Finds grains in a simple disjunction group - part of the CL Regex Optimiser.
 *
 * This function parses a simple parenthesized disjunction within a regular expression
 * and attempts to extract one grain from each alternative. Grains are written to the
 * local grain buffer. If a complete grain set has been found, the functions returns
 * a pointer to the first character after the disjunction. Otherwise it returns mark
 * and the caller can try to accept the group with read_matchall.
 *
 * For simplicity, only the first grain in each alternative is considered. This makes
 * it easier to check start/end alignment of the grains. Note that the local grain
 * buffer is always mangled, even if the function is unsuccessful.
 *
 * The first argument, mark, must point to the '(' at the beginning of the
 * disjunction group (unless no_paren is set).
 *
 * The booleans align_start and align_end are set to true if the grains from
 * *all* alternatives are anchored at the start or end of the disjunction
 * group, respectively.
 *
 * This is a non-exported function.
 *
 * @param mark         Pointer to the disjunction group (see also function
 *                     description).
 * @param align_start  See function description.
 * @param align_end    See function description.
 * @param no_paren     Attempt to read a top-level disjunction without parentheses,
 *                     which must extend to the end of the string.
 * @return             A pointer to first character after the disjunction group
 *                     iff the parse succeeded, the original pointer in
 *                     the mark argument otherwise.
 *
 */
char *
read_disjunction(char *mark, int *align_start, int *align_end, int no_paren)
{
  char *point, *p2, *buf;
  int grain;

  point = mark;
  if (no_paren) {
    if (*point == '(')
      return mark; /* bare disjunction must not be parenthesized */
  }
  else {
    if (*point == '(') {
      point++;
      if (*point == '?') {
        point++;
        /* don't accept special (?...) elements, except for simple non-capturing (?:...) */
        if (*point == ':')
          point++;
        else
          return mark; /* failed to parse disjunction */
      }
    }
    else
      return mark; /* disjunction group must be parenthesized */
  }

  buf = local_grain_data;
  grain_buffer_grains = 0;
  grain = 0;

  /* Note: if we can extend the disjunction parser further to allow parentheses around the
   * initial segment of an alternative, then regexen created by the matches operator will
   * also be optimised. */
  *align_start = *align_end = 1;
  while (1) { /* loop over alternatives in disjunction, using first grain found in each */
    p2 = read_grain(point, buf, &(grain_buffer_len[grain])); /* attempt to read a grain */
    while (p2 == point) {
      p2 = read_wildcard(point); /* try skipping a wildcard element */
      if (p2 > point) {
        point = p2;       /* advance point and look for grain again */
        *align_start = 0; /* grain in this alternative can't be aligned at start */
      }
      else
        return mark;      /* no grain found in this alternative -> unsuccessful */
      p2 = read_grain(point, buf, &(grain_buffer_len[grain]));
    }
    grain_buffer[grain] = buf; /* store grain in local grain buffer */
    buf += strlen(buf) + 1;
    grain++;
    if (grain >= MAX_GRAINS)
      return mark; /* too many alternatives, can't extract full grain set */
    point = p2;
    while (*point != '|' && *point != ')') {
      p2 = read_wildcard(point); /* try skipping data up to next | or ) */
      if (p2 > point) {
        point = p2;
        *align_end = 0;          /* grain in this alternative can't be aligned at end */
      }
      else
        break;
    }
    if (*point == '|')
      point++;                   /* continue with next alternative */
    else
      break;                     /* abort scanning */
  } /* end of while (1) loop over alternatives in disjunction */

  if ((!no_paren && *point == ')') ||
      (no_paren && *point == '\0')) {
    /* we've successfully read the entire disjunction */
    grain_buffer_grains = grain;
    if (grain >= 1)
      return point + 1;          /* success */
  }

  return mark;  /* unsuccessful */
}


/**
 * Computes a jump table for Boyer-Moore searches -- part of the CL Regex Optimiser.
 *
 * Unlike the textbook version, this jumptable includes the last character of each grain
 * (so we don't have to start the string comparison loops every time).
 *
 * A non-exported function.
 */
void
make_jump_table(CL_Regex rx)
{
  int j, k, jump;
  unsigned int ch;
  unsigned char *grain; /* want unsigned char to compare with unsigned int ch */

  /* clear the jump table */
  for (ch = 0; ch < 256; ch++)
    rx->jumptable[ch] = 0;

  if (rx->grains > 0) {
    /* compute smallest jump distance for each byte (0 -> matches last byte of one or more grains) */
    for (ch = 0; ch < 256; ch++) {
      jump = rx->grain_len; /* if character isn't contained in any of the grains, jump by grain length */

      for (k = 0; k < rx->grains; k++) { /* for each grain... */
        grain = (unsigned char *) rx->grain[k] + rx->grain_len - 1; /* pointer to last byte in grain */
        for (j = 0; j < rx->grain_len; j++, grain--) {
          if (*grain == ch) {
            if (j < jump)
              jump = j;
            break; /* can't find shorter jump distance for this grain */
          }
        }
      }

      rx->jumptable[ch] = jump;
    }
    if (cl_debug) {
      /* in debug mode, print out the entire jumptable */
      Rprintf("CL: cl_regopt_jumptable for Boyer-Moore search is\n");
      for (k = 0; k < 256; k += 16) {
        Rprintf("CL: ");
        for (j = 0, ch = k ; j < 15 ; j++, ch++)
          Rprintf((ch >= 32 && ch < 127) ? "|%2d %c  " : "|%2d %02X ", rx->jumptable[ch], ch);
        Rprintf("\n");
      }
    }
  }
  /* if no grains have been found, don't do anything (just clear the jump table) */
}




/* two monitoring functions */

/**
 * Reset the "success counter" for optimised regexes.
 */
void
cl_regopt_count_reset(void)
{
  cl_regopt_successes = 0;
}

/**
 * Get a reading from the "success counter" for optimised regexes.
 *
 * The counter is incremented by 1 every time the "grain" system
 * is used successfully to avoid calling PCRE. That is, it is
 * incremented every time a string is scrutinised and found to
 * contain none of the grains.
 *
 * Usage:
 *
 * cl_regopt_count_reset();
 *
 * for (i = 0, hits = 0; i < n; i++)
 *   if (cl_regex_match(rx, haystacks[i]))
 *     hits++;
 *
 * Rprintf(
 *         "Found %d matches; avoided regex matching %d times out of %d trials",
 *         hits, cl_regopt_count_get(), n );
 *
 * @see cl_regopt_count_reset
 * @return an integer indicating the number of times a regular expression
 *         has been matched using the regopt system of "grains", rather
 *         than by calling an external regex library.
 */
int
cl_regopt_count_get(void)
{
  return cl_regopt_successes;
}
