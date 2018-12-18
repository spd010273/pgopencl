#ifndef OPENCL_SCAN_H
#define OPENCL_SCAN_H

#include "postgres.h"
#include "access/relscan.h"
#include "access/sysattr.h"
#include "access/xact.h"
#include "catalog/heap.h"
#include "catalog/pg_namespace.h"
#include "catalog/pg_type.h"
#include "executor/nodeCustom.h"
#include "miscadmin.h"
#include "nodes/makefuncs.h"
#include "nodes/nodeFuncs.h"
#include "optimizer/clauses.h"
#include "optimizer/paths.h"
#include "optimizer/plancat.h"
#include "optimizer/restrictinfo.h"
#include "optimizer/var.h"
#include "parser/parsetree.h"
#include "storage/bufmgr.h"
#include "utils/builtins.h"
#include "utils/guc.h"
#include "utils/lsyscache.h"
#include "utils/memutils.h"
#include "utils/rel.h"
#include "utils/ruleutils.h"
#include "utils/spccache.h"

static bool enable_opencl_scan;
#endif // OPENCL_SCAN_H
