#include "opencl.h"

/*
 *  pgopencl_init_devices
 *      Enumerate and initialize contexts for all OpenCL devices installed
 *      in the system. Use GUCs to determine the size/scope of these contexts.
 *
 *      Once the contexts have been established, these should be stored in
 *      shared memory such that all background workers have access to the devices
 *      and their contexts
 */
void pgopencl_init_devices( void )
{
    cl_platform_id * platforms            = NULL;
    cl_device_id *   devices              = NULL;
    cl_int           rc                   = 0;
    cl_uint          num_platforms        = 0;
    cl_uint          num_devices          = 0;
    void *           param_value          = NULL;
    size_t           param_value_size_ret = 0;
    unsigned int     i                    = 0;
    unsigned int     j                    = 0;
    unsigned int     k                    = 0;

    /*
     * Discover Platforms installed on this system.
     *    These platforms may have one or more devices present on them
     *    in which a context may be instantiated
     */
    rc = clGetPlatformIDs(
        0,
        NULL,
        &num_platforms
    );

    if( rc != CL_SUCCESS )
        _handle_cl_error( rc );

    if( num_platforms > 0 )
    {
        platforms = ( cl_platform_id * ) palloc0( sizeof( cl_platform_id ) * num_platforms );

        if( platforms == NULL )
        {
            ereport(
                ERROR,
                (
                    errcode( ),
                    errmsg( "Failed to allocate memory for OpenCL device enumeration" )
                )
            );
        }

        rc = clGetPlatformIDs(
            num_platforms,
            platforms,
            NULL
        );

        if( rc != CL_SUCCESS )
            _handle_cl_error( rc );

        for( i = 0; i < num_platforms; i++ )
        {
            elog(
                LOG,
                "Device: %d",
                i
            );

            // Get the value for all platform attributes in this platform
            for( j = 0; i < PLATFORM_ATTRIBUTE_COUNT; j++ )
            {
                rc = clGetPlatformInfo(
                    platforms[i],
                    platform_attribute_types[j],
                    0,
                    NULL,
                    &param_value_size_ret
                );

                if( rc != CL_SUCCESS )
                    _handle_cl_error( rc );

                param_value = ( char * ) palloc0( sizeof( char ) * param_value_size_ret );

                if( param_value == NULL )
                {
                    pfree( platforms );
                    ereport(
                        ERROR,
                        (
                            errcode( ),
                            errmsg( "Failed to allocate memory for OpenCL device property" )
                        )
                    );
                }

                rc = clGetPlatformInfo(
                    platforms[i],
                    platform_attribute_types[j],
                    param_value_size_ret,
                    param_value,
                    NULL
                );

                if( rc != CL_SUCCESS )
                    _handle_cl_error( rc );

                elog(
                    LOG,
                    "%s: %s",
                    platform_attribute_names[j],
                    param_value
                );

                pfree( param_value );
            }

            rc = clGetDeviceIDs(
                platforms[i],
                CL_DEVICE_TYPE_ALL,
                0,
                NULL,
                &num_devices
            );

            if( rc != CL_sUCCESS )
                _handle_cl_error( rc );

            if( num_devices > 0 )
            {
                rc = clGetDeviceIDs(
                    platforms[i],
                    CL_DEVICE_TYPE_ALL,
                    num_devices,
                    devices,
                    NULL
                );

                if( rc != CL_SUCCESS )
                    _handle_cl_error( rc );

                for( j = 0; j < num_devices; j++ )
                {
                    for( k = 0; k < DEVICE_ATTRIBUTE_COUNT; k++ )
                    {
                        rc = clGetDeviceInfo(
                            devices[j],
                            device_attribute_types[k],
                            0,
                            NULL,
                            &param_value_size_ret
                        );

                        if( rc != CL_SUCCESS )
                            _handle_cl_error( rc );

                        param_value = ( void * ) palloc0( sizeof( char ) * param_value_size_ret );

                        if( param_value == NULL )
                        {
                            pfree( platforms );
                            ereport(
                                ERROR,
                                (
                                    errcode( ),
                                    errmsg( "Failed to allocate memory for OpenCL device property" )
                                )
                            );
                        }

                        rc = clGetDeviceInfo(
                            devices[j],
                            device_attribute_types[k],
                            param_value_size_ret,
                            param_value,
                            NULL
                        );

                        if( rc != CL_SUCCESS )
                            _cl_handle_error( rc );

                        elog(
                            LOG,
                            "   %s: %s",
                            device_attribute_names[k],
                            param_value
                        );

                        pfree( param_value );
                    }
                }
            }
            else
            {
                // Nothing to do :(
            }
        }
    }
    else
    {
        // Nothing to do :(
    }

    pfree( platforms );
    return;
}

void _handle_cl_error( cl_int rc )
{
    char * message;

    switch ( rc )
    {
        case CL_INVALID_PLATFORM:
            message = "Invalid Platform struct provided";
            break;
        case CL_INVALID_VALUE:
            message = "Invalid call to OpenCL function";
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = "Out of memory.";
            break;
        case CL_INVALID_DEVICE_TYPE:
            message = "Invalid device type specified.";
            break;
        case CL_DEVICE_NOT_FOUND:
            message = "No devices of the specified type are installed in this system.";
            break;
        case CL_OUT_OF_RESOURCES:
            message = "Host is out of resources.";
            break;
        case default:
            message = "Unhandled error.";
            break;
    }

    ereport(
        ERROR,
        (
            errcode( ERRCODE_OBJECT_NOT_IN_PREREQUISITE_STATE ),
            errmsg( message )
        )
    );
}
