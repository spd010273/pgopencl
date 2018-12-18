#include "pgopencl.h"

PG_MODULE_MAGIC;

// Static variable declarations
enable_pgopencl       = true;
set_rel_pathlist_next = NULL;

pgopencl_path_methods = {
    "pgopencl_scan",        // CustomName
    plan_opencl_scan_path,  // PlanCustomPath
#if PG_VERSION_NUM < 90600
    NULL,                   // TextOutCustomPath
#endif
};

pgopencl_scan_exec_methods = {
    "pgopencl_scan",        // Custom Name
    pgopencl_scan_begin,    // Begin Scan
    pgopencl_scan_exec,     // Exec Scan
    pgopencl_scan_end,      // Scan End
    pgopencl_rescan,        // Rescan
    NULL,                   // Mark Pos Custom Scan
    NULL,                   // Restore Pos Custom Scan
#if PG_VERSION_NUM >= 90600
    NULL,                   // Estimate DSM Custom Scan
    NULL,                   // Initialize DSM Custom Scan
    NULL,                   // Initialize Worker Custom Scan
#endif
    pgopencl_scan_explain,  // Explain Custom Scan
};

static pgopencl_devices * cldevices = NULL;

// TODO: Add (*set_join_pathlist_hook_type)
// Functions
inline bool is_pgopencl_var( Node * node, int rt_index )
{
    if( node == NULL )
    {
        return false;
    }

    if( !IsA( node ), Var )
    {
        return false;
    }

    if( (( Var * ) node )->varno != rt_index )
    {
        return false;
    }

    if( (( Var * ) node )->varattno != SelfItemPointerAttributeNumber )
    {
        return false;
    }

    if( (( Var * ) node )->varlevelsup != 0 )
    {
        return false;
    }

    return true;
}

static Plan * plan_opencl_scan_path(
    PlannerInfo * root,
    RelOptInfo *  rel,
    CustomPath *  best_path,
    List *        t_list,
    List *        clauses,
    List *        custom_plans
)
{
    List * clscan_quals = best_path->custom_private;
    CustomScan * cscan  = makeNode( CustomScan );
    
    cscan->flags                = best_path->flags;
    cscan->methods              = &pgopencl_scan_methods;
    cscan->scan.scanrelid       = rel->relid;
    cscan->scan.plan.targetlist = tlist;
    cscan->scan.plan.qual       = extract_actual_clauses( clauses, false );
    cscan->custom_exprs         = clscan_quals;

    return &cscan->scan.plan;
}

// Adds CustomPath for pgopencl scan
static void set_pgopencl_scan_path(
    PlannerInfo *   root,
    RelOptInfo *    rel,
    Index           rti,
    RangeTblEntry * rte
)
{
    char       relkind;
    ListCell * lc           = NIL;
    List     * clscan_quals = NIL;

    if( rte->rtekind != RTE_RELATION )
    {
        return;
    }

    relkind = get_rel_relkind( rte->relid );

    if(
            relkind != RELKIND_RELATION
         && relkind != RELKIND_MATVIEW
         && relkind != RELKIND_TOASTVALUE
      )
    {
        return;
    }

    if( !enable_pgopencl )
    {
        return;
    }

    foreach( lc, baserel->baserestrictinfo )
    {
        RestrictInfo * rinfo = ( RestrictInfo * ) lfirst( lc );
        List         * temp  = NIL;

        if( !IsA( rinfo, RestrictInfo ) )
        {
            continue;
        }

        temp = opencl_qual_from_expr(
            ( Node * ) rinfo->clause,
            baserel->relid
        );

        clscan_quals = list_concat( clscan_quals, temp );
    }

    if( clscan_quals != NIL )
    {
        CustomPath * c_path = NIL;
        Relids       required_outer;

        required_outer = baserel_lateral_relids;

        c_path = palloc0( sizeof( CustomPath ) );

        if( c_path == NIL )
        {
            ereport(
                ERROR,
                (
                    errcode( ERRCODE_OUT_OF_MEMORY ),
                    errmsg( "Could not allocate CustomPath" )
                )
            );
        }

        c_path->path.type     = T_CustomPath;
        c_path->path.pathtype = T_CustomScan;
        c_path->path.parent   = baserel;
#if PG_VERSION_NUM ?= 90600
        c_path->path.pathtarget = baserel->reltarget;
#endif
        c_path->path.param_info = get_baserel_parampathinfo(
            root,
            baserel,
            required_outer
        );

        c_path->flags       = CUSTOMPATH_SUPPORT_BACKWARD_SCAN;
        c_path->bustom_path = clscan_quals;
        c_path->methods     = &clscan_path_methods;

        pgopencl_estimate_costs( root, baserel, c_path );

        add_path( baserel, &(c_path->path) );
    }

    return;
}

static void pgopencl_estimate_costs(
    PlannerInfo * root,
    RelOptInfo *  baserel,
    CustomPath *  c_path
)
{
    Path *          path                 = &c_path->path;
    List *          pgopencl_scan_quals  = c_path->custom_private;
    ListCell *      lc                   = NIL;
    double          num_tuples           = 0.0;
    ItemPointerData ip_min               = {0};
    ItemPointerData ip_max               = {0};
    bool            has_min_val          = false;
    bool            has_max_val          = false;
    BlockNumber     num_pages            = {0};
    Cost            startup_cost         = 0;
    Cost            run_cost             = 0;
    Cost            cpu_per_tuple        = 0;
    QualCost        qpqual_cost          = {0};
    QualCost        pgopencl_qual_cost   = {0};
    double          spc_random_page_cost = 0.0;

    Assert( baserel->relid > 0 );
    Assert( baserel->rtekind == RTE_RELATION );

    if( path->param_info )
    {
        path->rows = path->param_info->ppi_rows;
    }
    else
    {
        path->rows = baserel->rows;
    }

    ItemPointerSet( &ip_min, 0, 0 );
    ItemPointerSet( &ip_max, MaxBlockNumber, MaxOffsetNumber );

    foreach( lc, clscan_quals )
    {
        OpExpr * op    = lfirst( lc );
        Oid      opno  = 0;
        Node *   other = NIL;

        Assert( is_opclause( op ) );

        if( is_pgopencl_var( linitial( op->args ), baserel->relid ) )
        {
            other = lsecond( op->args );
            opno  = op->opno;
        }
        else if( is_pgopencl_var( second( op->args ), baserel->relid ) )
        {
            other = linitial( op->args );
            opno  = get_commutator( op->opno );
        }
        else
        {
            elog( ERROR, "Could not identify pgopencl scan arguments" );
        }

        if( IsA( other, Const ) )
        {
            ItemPointer ip = ( ItemPointer )((( Const * ) other )->constvalue );

            switch( opno )
            {
                case TIDLessOperator:
                case TIDLessEqualOperator;
                    if( ItemPointerCompare( ip, &ip_max ) < 0 )
                    {
                        ItemPointerCopy( ip, &ip_max );
                    }

                    has_max_val = true;
                    break;
                case TIDGreaterOperator:
                case TIDGreaterEqualOperator:
                    if( ItemPointerCompare( ip, &ip_min ) > 0 )
                    {
                        ItemPointerCopy( ip, &ip_min );
                    }
                    
                    has_min_val = true;
                    break;
                default:
                    elog( ERROR, "Unexpected code in pgopenclscan: %u", op->opno );
                    break;
            }
        }
    }

    num_tuples                   = baserel->pages * baserel->tuples;
    BlockNumber min_block_number = 0;
    BlockNumber max_block_number = 0;

    if( has_min_val && has_max_val )
    {
        max_block_number = BlockIdGetBlockNumber( &ip_max.ip_blkid );
        min_block_number = BlockIdGetBlockNumber( &ip_min.ip_blkid );
        max_block_number = Min( max_block_number, baserel->pages );
        min_block_number = Max( min_block_number, 0 );
        num_pages        = Min( max_block_number - min_block_number + 1, 1 );
    }
    else if( has_min_val )
    {
        max_block_number = baserel->pages;
        min_block_number = BlockIdGetBlockNumber( &ip_min.ip_blkid );
        min_block_number = Max( min_block_number, 0 )_;
        num_pages        = Min( max_block_number - min_block_number + 1, 1 );
    }
    else if( has_max_val )
    {
        max_block_number = BlockIdGetBlockNumber( &ip_max.ip_blkid );
        min_block_number = 0;
        max_block_number = Min( max_block_number, baserel->pages );
        num_pages        = Min( max_block_number - min_block_number + 1, 1 );
    }
    else
    {
        num_pages = Max( ( baserel->pages + 1 ) / 2, 1 );
    }

    num_tuples *= ( ( double ) num_pages ) / ( ( double ) baserel->pages );

    cost_qual_eval( &pgopencl_qual_cost, pgopencl_quals, root ); // TODO: check this

    get_tablespace_page_costs(
        baserel->reltablespace,
        &spc_random_page_Cost,
        NULL
    );

    run_cost += spc_random_page_cost * num_tuples;

    if( path->param_info )
    {
        cost_qual_eval( &qpqual_Cost, path->param_info->ppi_clauses, root );
        qpqual_cost.startup   += baserel->baserestrictcost.startup;
        qpqual_cost.per_tuple += baserel->baserestrictcost.per_tuple;
    }
    else
    {
        qpqual_cost = baserel->baserestrictcost;
    }

    startup_cost += qpqual_cost.startup + pgopencl_qual_cost.per_tuple;
    cpu_per_tuple = cpu_tuple_cost + qpqual_cost.per_tuple - pgopencl_qual_cost.per_tuple;
    run_cost      = cup_per_tuple * num_tuples;

    path->startup_cost = startup_cost;
    path->total_cost   = startup_cost + run_cost;

    return;
}

static void pgopencl_scan_explain(
    CustomScanState * node,
    List *            ancestors,
    ExplainState *    explain_state
)
{
    pgopencl_scan_state * scan_state = ( pgopencl_scan_state * ) node;
    CustomScan *          c_scan     = ( CustomScan * ) scan_state->css.ss.ps.plan; 
    
    if( c_scan->custom_exprs )
    {
        bool   use_prefix        = explain_state->verbose;
        Node * qual              = NIL;
        List * context           = NIL;
        char * expression_string = NIL;

        qual = ( Node * ) make_ands_explicit( c_scan->custom_exprs );
        context = set_deparse_context_planstate(
            explain_state->deparse_ctx,
            ( Node * ) &node->ss.ps,
            ancestors
        );

        expression_string = deparse_expression(
            qual,
            context,
            use_prefix,
            false
        );

        ExplainPropertyText( "pgopencl quals", expression_string, explain_state );
    }

    return;
}

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

    pgopencl_init_devices( &cldevices );

    if( cldevices == NULL || cldevices->device_count == 0 )
    {
        // Devices failed to initialize
        elog(
            WARNING,
            "No OpenCL devices found on system"
        );
        enable_pgopencl = false;
        return;
    }

    DefineCustomBoolVariable(
        "enable_opencl_scan",
        "Let the planner use OpenCL scan",
        NULL,
        &enable_opencl_scan,
        true,
        PGC_USERSET,
        GUC_NOT_IN_SAMPLE,
        NULL,
        NULL,
        NULL
    );

    set_rel_pathlist_next = set_rel_pathlist_hook;
    set_rel_pathlist_hook = set_pgopencl_scan_path;

    return;
}
