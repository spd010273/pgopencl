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
void pgopencl_init_devices( pgopencl_devices ** cldevices )
{
    cl_int           rc                   = 0;
    cl_uint          num_platforms        = 0;
    cl_uint          num_devices          = 0;
    void *           param_value          = NULL;
    size_t           param_value_size_ret = 0;
    unsigned int     i                    = 0;
    unsigned int     j                    = 0;
    unsigned int     k                    = 0;
    unsigned int     device_index         = 0;
    unsigned int     device_arr_count     = 0;

    if( (*devices) != NULL )
    {
        // Seems they are already initialized ¯\_(ツ)_/¯
        return;
    }

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
        (*cldevices) = ( pgopencl_devices * ) palloc0( sizeof( pgopencl_devices ) );

        if( (*cldevices) == NULL )
        {
            ereport(
                ERROR,
                (
                    errcode( ERRCODE_OUT_OF_MEMORY ),
                    errmsg( "Failed to allocate memory for OpenCL device handles" )
                )
            );
        }

        (*cldevices)->device_count = 0;
        (*cldevices)->devices      = NULL;

        (*cldevices)->_platforms = ( cl_platform_id * ) palloc0(
            sizeof( cl_platform_id ) * ( unsigned int ) num_platforms
        );

        if( (*cldevices)->_platforms == NULL )
        {
            ereport(
                ERROR,
                (
                    errcode( ERRCODE_OUT_OF_MEMORY ),
                    errmsg( "Failed to allocate memory for OpenCL device enumeration" )
                )
            );
        }

        (*cldevices)->_platform_devices = ( cl_device_id ** ) palloc0(
            sizeof( cl_device_id * ) * ( unsigned int ) num_platforms
        );

        if( (*cldevices)->_platform_devices == NULL )
        {
            ereport(
                ERROR,
                (
                    errcode( ERRCODE_OUT_OF_MEMORY ),
                    errmsg( "Failed to allocate memory for OpenCL platform device enumeration" )
                )
            );
        }

        (*cldevices)->_num_platform_devices = ( unsigned int ) num_platforms;

        (*cldevices)->_device_counts = ( unsigned int * ) palloc0(
            sizeof( unsigned int ) * ( unsigned int ) num_platforms
        );

        if( (*cldevices)->_device_counts == NULL )
        {
            ereport(
                ERROR,
                (
                    errcode( ERRCODE_OUT_OF_MEMORY ),
                    errmsg( "Could not create OpenCL device listings" );
                )
            );
        }

        rc = clGetPlatformIDs(
            num_platforms,
            (*cldevices)->_platforms,
            NULL
        );

        if( rc != CL_SUCCESS )
            _handle_cl_error( rc );

        if( num_platforms == 0 )
        {
            // shouldnt happen
            return;
        }

        (*cldevices)->_platform_count = (unsigned int) num_platforms;

        for( i = 0; i < num_platforms; i++ )
        {
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
                (*cldevices)->_platform_devices[i] = ( cl_device_id * ) palloc0(
                    sizeof( cl_device_id ) * (unsigned int) num_devices
                );

                if( (*cldevices)->_platform_devices[i] == NULL )
                {
                    ereport(
                        ERROR,
                        (
                            errcode( ERRCODE_OUT_OF_MEMORY ),
                            errmsg( "Failed to create OpenCL device array" )
                        )
                    );
                }

                rc = clGetDeviceIDs(
                    platforms[i],
                    CL_DEVICE_TYPE_ALL,
                    num_devices,
                    (*cldevices)->_platform_devices[i],
                    NULL
                );

                if( rc != CL_SUCCESS )
                    _handle_cl_error( rc );

                (*cldevices)->_device_count[i] = (unsigned int) num_devices;

                for( j = 0; j < num_devices; j ++ )
                {
                    device_index = pgopencl_add_device(
                        ( pgopencl_devices * ) (*cldevices),
                        (*cldevices)->_platforms[i],
                        (*cldevices)->_platform_devices[i][j]
                    );

                    if(
                           (*cldevices) == NULL
                        || (*cldevices)->device_count == 0
                        || (*cldevices)->devices[device_index] == NULL
                      )
                    {
                        elog( ERROR, "Failed to initialize new opencl device" );
                        continue;
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

unsigned int pgopencl_add_device( pgopencl_devices * cldevices, cl_platform_id * platform, cl_device_id * device )
{
    pgopencl_device * new_device = NULL;

    if( cldevices == NULL || platform == NULL || device == NULL )
    {
        return 0;
    }

    // Extend device array by 1 or initialize
    if( cldevices->devices == NULL )
    {
        cldevices->devices = ( pgopencl_device ** ) palloc0( sizeof( pgopencl_device ) );
    }
    else
    {
        cldevices->devices = ( pgopencl device ** ) repalloc(
            ( pgopencl_device ** ) cldevices->devices,
            sizeof( pgopencl_device * ) * ( cldevices->device_count + 1 )
        );
    }

    if( cldevices->devices == NULL )
    {
        cldevices->device_count = 0;

        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Could not extend OpenCL device handle array" )
            )
        );
    }

    new_device = ( pgopencl_device * ) palloc0( sizeof( pgopencl_device ) );

    if( new_device == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Could not create new device handle" )
            )
        );
    }

    new_device->platform      = platform;
    new_device->device        = device;
    new_device->kernels       = NULL;
    new_device->device_state  = NULL;
    new_device->compute_units = 0;
    new_device->mem_size      = 0;
    new_device->bogo_mips     = 0;

    cldevices->device[cldevices->device_count] = new_device;
    cldevices->device_count++;

    return cl_devices->device_count - 1; // Return index of the device we just added
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
