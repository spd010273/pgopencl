#include "opencl.h"

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
