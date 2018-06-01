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

#include "init.h"

#define PGOPENCL_VERSION 0.1

PG_MODULE_MAGIC;

bool pgopencl_enabled;

void _PG_init( void )
{
    if( !process_shared_preload_libraries_in_progress )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OBJECT_NOT_IN_PREREQUISITE_STATE ),
                errmsg( "pgopencl must be loaded via shared_preload_libraries in postgresql.conf" )
            )
        );
    }
    
    elog(
        LOG,
        "pgopencl version %s loaded",
        PGOPENCL_VERSION
    );
    
    pgopencl_init_devices();
}
