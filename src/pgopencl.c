#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <CL/cl.h>
#include "lib/util.h"

static const char * ksrc = "\
    kernel void add(\
        unsigned long n,\
        global const double * a,\
        global const double * b,\
        global double * c \
    )\
    {\
        size_t i = get_global_id(0);\
\
        if( i < n )\
        {\
            c[i] = a[i] + b[i];\
        }\
\
        return;\
    }";

// Parse CL_PLATFORM_VERSION string
// It is in the form of:
// OpenCL <Maj>.<Min> <Platform specific vers>
int gcd_cl_major = 9;
int gcd_cl_minor = 9;

void initialize_opencl_devices( void );

void initialize_opencl_devices( void )
{
    cl_int retval;
    cl_platform_id * platforms = NULL;
    cl_uint num_platforms;
    int i = 0;
    int j = 0;
    void * param_value;
    size_t param_value_size_ret;

    const char * attribute_names[5] = {
        "Name",
        "Vendor",
        "Version",
        "Profile",
        "Extensions"
    };

    const cl_platform_info attribute_types[5] = {
        CL_PLATFORM_NAME,
        CL_PLATFORM_VENDOR,
        CL_PLATFORM_VERSION,
        CL_PLATFORM_PROFILE,
        CL_PLATFORM_EXTENSIONS
    };

    retval = clGetPlatformIDs(
        0,
        NULL,
        &num_platforms
    );

    if( retval == CL_INVALID_VALUE )
    {
        _log(
            LOG_LEVEL_FATAL,
            "Invalid call to clGetPlatformIDs"
        );
    }
    else if( retval == CL_OUT_OF_HOST_MEMORY )
    {
        _log(
            LOG_LEVEL_FATAL,
            "Failed to enumerate OpenCL Platforms"
        );
    }

    if( num_platforms > 0 )
    {
        platforms = ( cl_platform_id * ) malloc( sizeof( cl_platform_id ) * num_platforms );

        if( platforms == NULL )
        {
            _log(
                LOG_LEVEL_FATAL,
                "Failed to allocate memory for platform ids."
            );
        }

        retval = clGetPlatformIDs(
            num_platforms,
            platforms,
            NULL
        );

        if( retval == CL_INVALID_VALUE )
        {
            _log(
                LOG_LEVEL_FATAL,
                "Second call to clGetPlaformIDs is invalid"
            );
        }
        else if( retval == CL_OUT_OF_HOST_MEMORY )
        {
            _log(
                LOG_LEVEL_FATAL,
                "Host is out of memory"
            );
        }
    }
    else
    {
        _log(
            LOG_LEVEL_WARNING,
            "There are no OpenCL Platforms present on this system."
        );
        return;
    }

    for( i = 0; i < num_platforms; i++ )
    {
        for( j = 0; j < 5; j++ )
        {
            retval = clGetPlatformInfo(
                platforms[i],
                attribute_types[j],
                0,
                NULL,
                &param_value_size_ret
            );

            if( retval == CL_INVALID_PLATFORM )
            {
                _log(
                    LOG_LEVEL_FATAL,
                    "Platform object is not valid"
                );
            }
            else if( retval == CL_INVALID_VALUE )
            {
                _log(
                    LOG_LEVEL_FATAL,
                    "Param name is not supported"
                );
            }
            else if( retval == CL_OUT_OF_HOST_MEMORY )
            {
                _log(
                    LOG_LEVEL_FATAL,
                    "Out of memory"
                );
            }

            param_value = ( char * ) malloc( sizeof( char ) * param_value_size_ret );
            if( param_value == NULL )
            {
                _log(
                    LOG_LEVEL_FATAL,
                    "Failed to allocate memory for parameter"
                );
            }

            retval = clGetPlatformInfo(
                platforms[i],
                attribute_types[j],
                param_value_size_ret,
                param_value,
                NULL
            );

            if( retval == CL_INVALID_PLATFORM )
            {
                _log(
                    LOG_LEVEL_FATAL,
                    "Platform object is not valid"
                );
            }
            else if( retval == CL_INVALID_VALUE )
            {
                _log(
                    LOG_LEVEL_FATAL,
                    "Parameter name is not supported"
                );
            }
            else if( retval == CL_OUT_OF_HOST_MEMORY )
            {
                _log(
                    LOG_LEVEL_FATAL,
                    "Out of memory"
                );
            }

            printf(
                "Device %d: %s: %s",
                i,
                attribute_names[j],
                (char * ) param_value
            );

            free( param_value );
        }
    }

    free( platforms );
    return;
}

int main( void )
{
    return 0;
}
