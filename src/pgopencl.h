#ifndef PGOPENCL_H
#define PGOPENCL_H
#include "postgres.h"
#include "storage/ipc.h"
#include "storage/pg_shmem.h"
#include "utils/builtins.h"
#include "utils/guc.h"
#include "utils/lsyscache.h"
#include "utils/ruleutils.h"
#include "fmgr.h"
#include "access/hash.h"
#include "optimizer/clauses.h"
#include "optimizer/cost.h"
#include "optimizer/pathnode.h"
#include "optimizer/planner.h"
#include "parser/parsetree.h"

#define PGOPENCL_VERSION 0.1

typedef struct {
    CustomScanState css;
    List * pgopencl_quals;
} pgopencl_scan_state;

// Static Variables
static bool enable_pgopencl;
static set_rel_pathlist_hook_type set_rel_pathlist_next;
static CustomExecMethods pgopencl_scan_exec_methods;

// Callback Setup
static CustomPathMethods pgopencl_path_methods;

// Function Declarations
static void set_pgopencl_scan_path( PlannerInfo *, RelOptInfo *, Index, RangeTblEntry * );
static void pgopencl_estimate_costs( PlannerInfo *, RelOptInfo *, CustomPath * );
static Plan * plan_opencl_scan_path( PlannerInfo *, RelOptInfo *, CustomPath *, List *, List *, List * );
void _PG_init( void );
inline bool is_pgopencl_var( Node *, int );

// CustomScan methods
static void pgopencl_scan_explain( CustomScanState *, List *, ExplainState * );
#endif
