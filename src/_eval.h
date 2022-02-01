/** Number of AVStructures each Patternlist can store. */
#include "corpmanag.h"
#include "symtab.h"

typedef struct dfa {
  int Max_States;         /**< max number of states of the current dfa;
  state no. 0 is the initial state.             */
    int Max_Input;          /**< max number of input chars of the current dfa. */
    int **TransTable;       /**< state transition table of the current dfa.    */
    Boolean *Final;         /**< set of final states.                          */
    int E_State;            /**< Error State -- it is introduced in order to
  *   make the dfa complete, so the state transition
  *   is a total mapping. The value of this variable
  *   is Max_States.
  */
} DFA;
/** Number of AVStructures each Patternlist can store. */
#define MAXPATTERNS 5000

/** maximum number of EvalEnvironments in the global array */
#define MAXENVIRONMENT 10


/*
 * definition of the evaluation tree of boolean expressions
 */


/**
 * Labels a boolean operation.
 */
enum b_ops {
             b_and,      /**< boolean and operator           */
             b_or,       /**< boolean or operator            */
             b_implies,  /**< boolean implication (->) operator */
             b_not,      /**< boolean negation               */

             cmp_gt,     /**< compare: greater than          */
             cmp_lt,     /**< compare: less than             */
             cmp_get,    /**< compare: greater or equal than */
             cmp_let,    /**< compare: less or equal than    */
             cmp_eq,     /**< compare: equal                 */
             cmp_neq,    /**< compare: not equal             */

             cmp_ex      /**< is value present? bool exprs   */
};


/**
 * Labels the type of a "wordform" comparison: normal, regular expression, or cpos id ref.
 */
enum wf_type {
    NORMAL,  /**< type of comparison = normal. */
    REGEXP,  /**< type of comparison = regex. */
    CID      /**< type of comparison = compare as integer corpus ID refs. */
};
/*
 * NB. An ancient comment in the eval.c code says, regarding CID:
 *        "that's quick & dirty. We should have a cidref node"
 * IE, the CID type of "wordform" comparison was a stopgap originally -
 * patching integer comparison into the strign comparison.
 */

/**
 * Labels the type of a boolean node.
 */
enum bnodetype {
                 bnode,                 /**< boolean evaluation node            */
                 cnode,                 /**< constant node                      */
                 func,                  /**< function call                      */
                 sbound,                /**< structure boundary (open or close) */
                 pa_ref,                /**< reference to positional attribute  */
                 sa_ref,                /**< reference to structural attribute  */

                 string_leaf,           /**< string constant */
                 int_leaf,              /**< integer constant */
                 float_leaf,            /**< float constant */

                 id_list,               /**< list of IDs */
                 var_ref                /**< variable reference */
               };

/**
 * Union of structures underlying the Constraint / Constrainttree objects.
 *
 * Each Constraint is a boolean node in the Constrainttree, i.e. a single element of a compiled CQP query.
 */
typedef union c_tree {

  /** The type of this particular node.
   * Allows the type member of the other structures within the union to be accessed. */
  enum bnodetype type;

  /** "standard" operand node in the evaluation tree; type is "bnode" */
  struct {
    enum bnodetype type;                  /**< must be bnode                     */
    enum b_ops     op_id;                 /**< identifier of the bool operator   */
    union c_tree  *left,                  /**< points to the first operand       */
                  *right;                 /**< points to the second operand,
                                               if if there is one                */
  }                node;

  /** "constant" node in the evaluation tree */
  struct {
    enum bnodetype type;                  /**< must be cnode                     */
    int            val;                   /**< Value of the constant: 1 or 0 for
                                               true or false                     */
  }                constnode;

  /** function call (builtin, or dynamic attribute), type is "func" */
  struct {
    enum bnodetype type;                  /**< must be func                      */
    int            predef;                /**< identifier of a builtin function
                                               (or -1 if this is a d-attribute)  */
    Attribute     *dynattr;               /**< a dynamic attribute (or NULL if
                                               this is a builtin function)       */
    struct _ActualParamList *args;        /**< parameter list for the function
                                               (head of linked list)             */
    int            nr_args;               /**< n of parameters on the args list  */
  }                func;

  /** structure boundary */
  struct {
    enum bnodetype type;                  /**< must be sbound                    */
    Attribute     *strucattr;             /**< the attribute which corresponds
                                               to the structure                  */
    Boolean        is_closing;            /**< True if closing boundary, False
                                               if opening boundary               */
  }                sbound;

  /** reference to positional attribute */
  struct {
    enum bnodetype type;                  /**< must be pa_ref                    */
    LabelEntry     label;                 /**< may be empty (NULL)               */
    Attribute     *attr;                  /**< p-attribute the node refers to    */
    int            del;                /**< delete label after using it?      */
  }                pa_ref;

  /**
   * reference to structural attribute.
   *
   * If label is empty, this checks if the current position is at start
   * or end of structural_attribute and returns INT value (this is kept for
   * backward compatibility regarding lbound() and rbound() builtins; the new
   * syntax is to use {s} and {/s}, which are represented as 'Tag' nodes.
   *
   * If label is non-empty, the referenced S-attribute must have values, and
   * the value of the enclosing region is returned as a string; in short,
   * values of attributes can be accessed through label references .
   */
  struct {
    enum bnodetype type;                  /**< must be sa_ref                    */
    LabelEntry     label;                 /**< may be empty (NULL)               */
    Attribute     *attr;                  /**< s-attribute the node refers to    */
    int            del;                /**< delete label after using it?      */
  }                sa_ref;

  /** reference to a variable */
  struct {
    enum bnodetype type;                  /**< must be var_ref                   */
    char          *varName;
  }                varref;

  /** list of item IDs */
  struct {
    enum bnodetype type;                  /**< must be id_list                   */
    Attribute     *attr;
    LabelEntry     label;                 /**< may be empty (NULL)               */
    int            negated;
    int            nr_items;              /**< size of the "items" array         */
    int           *items;                 /**< array of item IDs                 */
    int            del;                /**< delete label after using it?      */
  }                idlist;

  /** leaf: a constant (string, int, float, ...) */
  struct {
    enum bnodetype type;                  /**< string_leaf, int_leaf, float_leaf */
    int            canon;                 /**< canonicalization mode (i.e. flags)*/
    enum wf_type   pat_type;              /**< pattern type: normal wordform or
                                               regex (applies to string_leaf)    */
    CL_Regex       rx;                    /**< compiled regular expression
                                               (applies to string_leaf)          */

    /** Union containing the constant. */
    union {
      char        *sconst;               /**< operand is a string constant.           */
      int          iconst;               /**< operand is a integer constant.          */
      int          cidconst;             /**< operand is {?? corpus position?? corpus lexicon id??} constant */
      double       fconst;               /**< operand is a float (well, double) constant */
    }              ctype;
  }                leaf;
} Constraint;

/**
 * The Constrainttree object.
 */
typedef Constraint *Constrainttree;


/**
 * The ActualParamList object: used to build a linked list of parameters,
 * each one of which is a Constrainttree.
 */
typedef struct _ActualParamList {
  Constrainttree param;
  struct _ActualParamList *next;
} ActualParamList;


/** Enumeration specifying different types of eval-tree node. */
enum tnodetype {
    node,           /**< This is a branching node in a "normal" (regex) query tree. */
    leaf,           /**< This is a terminal node in a "normal" (regex) query tree. */
    meet_union,     /**< This is a tree for a MU (meet-union) query. */
    tabular         /**< This is a tree for a TAB (tabular) query. */
};

/** Enumeration of regular expression operations (for token-level regex) */
enum re_ops  {
    re_od_concat,   /**< regex operation: order dependent concatenation   */
    re_oi_concat,   /**< regex operation: order independent concatenation (unused) */
    re_disj,        /**< regex operation: disjunction               */
    re_repeat       /**< regex operation: repetition, i.e. ({n} and {n,k})  */
};

/** Symbols for the two operations of meet-union queries, that is meet and union! */
enum cooc_op {
    cooc_meet,      /** MU operation: meet */
    cooc_union      /** MU operation: union */
};


/**
 * Evaltree object: structure for a compiled CQP query.
 */
typedef union e_tree *Evaltree;


/* cross-check tree.h after changes of this data structure!!!
 * also check the print commands in tree.c */

/**
 * Underlying union for the Evaltree object.
 *
 * Consists of a number of anonymous-type structures
 * (node, leaf, cooc, tab_el) that can be found in a tree.
 *
 * The type member is always accessible.
 *
 * @see tnodetype
 */
union e_tree {

  /** What type of node does this union represent? */
  enum tnodetype type;

  /** node type: node */
  struct {
    enum tnodetype type;
    enum re_ops    op_id;      /**< id_number of the RE operator */
    Evaltree       left,       /**< points to the first argument */
                   right;      /**< points to the second argument -- if it exists. */
    int            min,        /**< minimum number of repetitions.  */
                   max;        /**< maximum number of repetitions.  */
  }                node;

  /** node type: leaf */
  struct {
    enum tnodetype type;
    int            patindex;   /**< index into the patternlist */
  }                leaf;

  /** node type: meet_union (MU query) */
  struct {
    enum tnodetype type;
    enum cooc_op   op_id;
    int            lw, rw;
    Attribute     *struc;
    Evaltree       left,
                   right;
    int            negated;
  }                cooc;

  /** node type: tabular (TAB query) */
  struct {
    enum tnodetype type;
    int patindex;              /**< index into the pattern list */
    int min_dist;              /**< minimal distance to next pattern */
    int max_dist;              /**< maximal distance to next pattern */
    Evaltree       next;       /**< next pattern */
  }                tab_el;

};

/*
 * Definition of a Patternlist, which holds the individual elements (token expressions and XML tags)
 * over which the regular expression of a finite-state query is build (formally speaking, the Patternlist
 * forms the symbol vocabulary for the regular expression). The items of a Patternlist are of type AVS,
 * which presumably stands for attribute-value structure due to historical roots that no longer apply.
 */

/** The AVS type: tells us what kind of query element an AVS holds. */
typedef enum _avstype {
  /** a token expression, `[...conditions...]` in CQP syntax, represented as a constraint tree (tree of Boolean nodes) */
  Pattern,
  /** an XML tag matching the start/end of an s-attribute region, possibly with regexp filter on the annotated value  */
  Tag,
  /** a matchall item, i.e. a token expression without any constraints, matching an arbitary token; `[]` or `[::]` in CQP syntax. */
  MatchAll,
  /** an XML tag matching an anchor point in a subquery, e.g. `<match>` or `</target>` in CQP syntax */
  Anchor,
  /** a region element, `<<name>>` in CQP syntax, matching an entire region of an s-attribute or named query result */
  Region
} AVSType;

typedef enum target_nature {
  IsNotTarget = 0, IsTarget = 1, IsKeyword = 2
} target_nature;

typedef enum avs_region_op {
  AVSRegionEnter = 1, AVSRegionWait = 2, AVSRegionEmit = 3
} avs_region_op;

/**
 * AVStructure: underlying data union for an AVS object representing an element of a finite-state query
 *
 * Internal format: a union of structures (with the type member always accessible).
 *
 * The Patternlist (i.e. the "symbol" vocabulary over which the regular expression of a finite-state query is formulated)
 * is an array of AVS objects.
 *
 */
typedef union _avs {

  /** What type of AV structure is this? */
  AVSType type;

  /** a matchall AVS */
  struct {
    AVSType type;                /* set to MatchAll */
    LabelEntry label;
    target_nature is_target;     /**< whether pattern is marked as target (= 1) or keyword (= 2) */
    Boolean lookahead;           /**< whether pattern is just a lookahead constraint */
  } matchall;

  /** a token expression AVS (i.e. a Boolean constraint tree) */
  struct {
    AVSType type;                /* set to Pattern */
    LabelEntry label;
    Constrainttree constraint;
    target_nature is_target;     /**< whether pattern is marked as target (= 1) or keyword (= 2) */
    Boolean lookahead;           /**< whether pattern is just a lookahead constraint */
  } con;

  /** an XML tag AVS */
  struct {
    AVSType type;                /* set to Tag */
    int is_closing;
    Attribute *attr;
    char *constraint;            /**< constraint for annotated value of region (string or regexp); NULL = no constraint */
    int flags;                   /**< flags passed to regexp or string constraint (information purposes only) */
    CL_Regex rx;                 /**< if constraint is a regexp, this holds the compiled regexp; otherwise NULL */
    int negated;                 /**< whether constraint is negated (!=, not matches, not contains) */
    LabelEntry right_boundary;   /**< label in RDAT namespace: contains right boundary of constraining region (in StrictRegions mode) */
  } tag;

  /** an anchor point tag AVS (used in subqueries) */
  struct {
    AVSType type;                /* set to Anchor */
    int is_closing;
    FieldType field;
  } anchor;

  /** a region AVS (a CQP query element <<name>> is implemented as a loop consisting of three linked Region AVS elements) */
  struct {
    AVSType type;                /* set to Region */
    avs_region_op opcode;        /**< which part of the loop: 1 = ENTER, 2 = WAIT, 3 = EMIT; note that all other items except for `queue` are only defined for the ENTER operation */
    char *name;                  /**< name of the region element (s-attribute or NQR), for debugging purposes */
    StateQueue queue;            /**< queue of simulation states waiting to be emitted; this is a weak reference for WAIT and EMIT operations */
    LabelEntry start_label;      /**< optional label to set on first cpos of the region */
    target_nature start_target;  /**< whether first cpos of the region should be marked as target (= 1) or keyword (= 2) */
    LabelEntry end_label;        /**< optional label to set on last cpos of the region */
    target_nature end_target;    /**< whether last cpos of the region should be marked as target (= 1) or keyword (= 2) */
    Attribute *attr;             /**< query element matches region of this s-attribute (NULL if it matches a NQR) */
    CorpusList *nqr;             /**< query element matches range of this named query result (NULL if it matches an s-attribute) */
  } region;

} AVStructure;

/** AVS object: defined as a pointer type for AVStructure (it's not clear what "AV" stands for) @see AVStructure*/
typedef AVStructure *AVS;

/** A Patternlist is an array of AVS objects. */
typedef AVStructure Patternlist[MAXPATTERNS];

/* ====================================================================== */

/** the direction of a Context (for expand ...) */
typedef enum ctxtdir { ctxtdir_leftright, ctxtdir_left, ctxtdir_right } ctxtdir;
/** used to record whether the "space type" of a Context is "word or structure" */
typedef enum spacet { word, structure } spacet;

/**
 * The (ambiguously named) Context object.
 *
 * This stores information about context space.
 *
 * "Context" here means the context for evaluation of a query result within
 * a corpus. (???)
 *
 * CUT TO THE CHASE: it's used for the "within..." clause, "expand", and for MUs.
 */
typedef struct ctxtsp {
  ctxtdir     direction;     /**< direction of context expansion (if valid).
                                  Might be left, right, or leftright (all ctxtdir_). */
  spacet      space_type;    /**< kind of space (word or structure)         */
  Attribute  *attrib;        /**< attribute representing the structure.     */
  int         size;          /**< size of space in number of structures.    */
  int         size2;         /**< only for meet-context                     */
} Context;


/* ====================================================================== */

/**
 * The MatchSelector structure defines which part of a (standard) query match is returned.
 *
 * The structure consists of labels defining the start and end of the part to be extracted
 * (or NULL for match/matchend) and optional offsets to be applied.
 *
 * It is used by the new "show a .. b" syntax introduced into standard queries by CQP v3.4.32,
 * and does not apply to MU or TAB queries.
 */
typedef struct MatchSelector {
  LabelEntry begin;          /**< label that determines the beginning of the extracted span (if NULL, the `match` anchor is implied) */
  int        begin_offset;   /**< offset to be added to the beginning cpos */
  LabelEntry end;            /**< label that determines the end of the extracted span (if NULL, the `matchend` anchor is implied) */
  int        end_offset;     /**< offset to be added to the end cpos */
} MatchSelector;


/* ====================================================================== */

/**
 * The EvalEnvironment object: variables for the evaluation of a standard query.
 *
 * This includes the query itself, as an evaltree and compiled into a DFA; the corpus being queried;
 * the match strategy; and other information.
 */
typedef struct evalenv {
  CorpusList *query_corpus;         /**< the search corpus for this query part (IE the subcorpus whose ranges = matches) */

  int rp;                           /**< index of current range (in subqueries) */

  SymbolTable labels;               /**< symbol table for labels */

  int MaxPatIndex;                  /**< the current number of patterns */
  Patternlist patternlist;          /**< The pattern list itself (Array of AVS objects) */

  Constrainttree gconstraint;       /**< the "global constraint" (boolean tree that applies to whole thing, not just one cpos) */

  Evaltree evaltree;                /**< the evaluation tree (with regular exprs) produced by the YACC parser */

  DFA dfa;                          /**< the compiled automaton for the token-level regex of the query */

  int has_target_indicator;         /**< is there a target mark ('@') in the query? */
  LabelEntry target_label;          /**< targets are implemented as a special label "target" now */

  int has_keyword_indicator;        /**< is there a keyword mark (default '@9') in the query? */
  LabelEntry keyword_label;         /**< keywords are implemented as a special label "keyword" */

  LabelEntry match_label;           /**< special "match" label for access to start of match within query */
  LabelEntry matchend_label;        /**< special "matchend" label for access to end of match within query */

  Context search_context;           /**< the search context (within...) */
  MatchSelector match_selector;     /**< selects a subspan of a standard query to be returned as the matching range */

  Attribute *aligned;               /**< the attribute holding the alignment info */

  int negated;                      /**< Boolean: 1 iff we should negate the alignment constraint */

  MatchingStrategy matching_strategy; /**< copied from global option unless overwritten by (?...) directive */

} EvalEnvironment;

/**
 * EEPs are EvalEnvironment pointers.
 */
typedef EvalEnvironment *EEP;



/* Global array of EvalEnvironment structures */
extern EvalEnvironment Environment[MAXENVIRONMENT];

/* Global eval environment array index */
extern int ee_ix;


extern EEP CurEnv;
extern EEP evalenv;

/* ---------------------------------------------------------------------- */



/* ==================== the three query types */

void cqp_run_query(int cut, int keep_old_ranges);

void cqp_run_mu_query(int keep_old_ranges, int cut_value);

void cqp_run_tab_query();


/* ==================== Functions for working with the global array of EvalEnvironments.  */

int next_environment(void);

int free_environment(int thisenv);

void show_environment(int thisenv);

void free_all_environments(void);


