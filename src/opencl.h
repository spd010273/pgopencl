#ifndef OPENCL_H
#define OPENCL_H
#include "postgres.h"
#include "CL/cl.h"

#define PLATFORM_ATTRIBUTE_COUNT 5
#define DEVICE_ATTRIBUTE_COUNT 53

void pgopencl_init_devices( void );
void _handle_cl_error( cl_int );

const char * platform_attribute_names[PLATFORM_ATTRIBUTE_COUNT] = {
    "Name",
    "Vendor",
    "Version",
    "Profile",
    "Extensions"
};

const cl_platform_info platform_attribute_types[PLATFORM_ATTRIBUTE_COUNT] = {
    CL_PLATFORM_NAME,
    CL_PLATFORM_VENDOR,
    CL_PLATFORM_VERSION,
    CL_PLATFORM_PROFILE,
    CL_PLATFORM_EXTENSIONS
};

const char * device_attribute_names[DEVICE_ATTRIBUTE_COUNT] = {
    "Device Type",
    "Vendor ID",
    "Compute Units",
    "Work Item Dimension",
    "Work Item Size",
    "Work Group Size",
    "Vector Width (char)",
    "Vector Width (short)",
    "Vector Width (int)",
    "Vector Width (long)",
    "Vector Width (float)",
    "Vector Width (double)",
    "Vector Width (half)",
    "Clock Frequency",
    "Address Bits",
    "Max Memory Allocation",
    "Image Support",
    "Max Simultaneous Images (read)",
    "Max Simultaneous Images (write)",
    "2D Image Width",
    "2D Image Height",
    "3D Image Width",
    "3D Image Height",
    "3D Image Depth",
    "Max Samplers",
    "Max Argument Size",
    "Min. Var Bit Size",
    "Min. Var Byte Size",
    "SP Floating Point Capability",
    "Cache Type",
    "Cache Line Size (bytes)",
    "Cache Size (bytes",
    "Device Memory (bytes)",
    "Constants Buffer Size",
    "Constant Arguments",
    "Local Memory Support",
    "Local Memory Size (bytes)",
    "ECC Enabled",
    "Memory Unified with Host",
    "Profiling Timer Resolution",
    "Device is Little Endian",
    "Device Available",
    "Device Compiler Available",
    "Device Execution Capabilities",
    "Command Queue Properties",
    "Device Platform",
    "Device Name",
    "Device Vendor",
    "Driver Version",
    "Device Profile",
    "Device Version",
    "OpenCL C Version",
    "Device Extensions"
};

const cl_device_info device_attribute_types[DEVICE_ATTRIBUTE_COUNT] = {
    CL_DEVICE_TYPE, // cl_device_type
    CL_DEVICE_VENDOR_ID, // cl_uint
    CL_DEVICE_MAX_COMPUTE_UNITS, // cl_uint
    CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, // cl_uint
    CL_DEVICE_MAX_WORK_ITEM_SIZES, // size_t[]
    CL_DEVICE_MAX_WORK_GROUP_SIZE, // size_t
    CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, // cl_uint
    CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, // cl_uint
    CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, // cl_uint
    CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, // cl_uint
    CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, // cl_uint
    CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, // cl_uint
    CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, // cl_uint
    CL_DEVICE_MAX_CLOCK_FREQUENCY, // cl_uint
    CL_DEVICE_ADDRESS_BITS, // cl_uint
    CL_DEVICE_MAX_MEM_ALLOC_SIZE, // cl_ulong
    CL_DEVICE_IMAGE_SUPPORT, // cl_bool
    CL_DEVICE_MAX_READ_IMAGE_ARGS, // cl_uint
    CL_DEVICE_MAX_WRITE_IMAGE_ARGS, // cl_uint
    CL_DEVICE_IMAGE2D_MAX_WIDTH, // size_t
    CL_DEVICE_IMAGE2D_MAX_HEIGHT, // size_t
    CL_DEVICE_IMAGE3D_MAX_WIDTH, // size_t
    CL_DEVICE_IMAGE3D_MAX_HEIGHT, // size_t
    CL_DEVICE_IMAGE3D_MAX_DEPTH, // size_t
    CL_DEVICE_MAX_SAMPLERS, // cl_uint
    CL_DEVICE_MAX_PARAMETER_SIZE, // size_t
    CL_DEVICE_MEM_BASE_ADDR_ALIGN, // cl_uint
    CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, // cl_uint
    CL_DEVICE_SINGLE_FP_CONFIG, // cl_device_fp_config
    CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, // cl_device_mem_cache_type
    CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, // cl_uint
    CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, // cl_ulong
    CL_DEVICE_GLOBAL_MEM_SIZE, // cl_ulong
    CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, // cl_ulong
    CL_DEVICE_MAX_CONSTANT_ARGS, // cl_uint
    CL_DEVICE_LOCAL_MEM_TYPE, // cl_device_local_mem_type
    CL_DEVICE_LOCAL_MEM_SIZE, // cl_ulong
    CL_DEVICE_ERROR_CORRECTION_SUPPORT, // cl_bool
    CL_DEVICE_HOST_UNIFIED_MEMORY, // cl_bool
    CL_DEVICE_PROFILING_TIMER_RESOLUTION, // size_t
    CL_DEVICE_ENDIAN_LITTLE, // cl_bool
    CL_DEVICE_AVAILABLE, // cl_bool
    CL_DEVICE_COMPILER_AVAILABLE, // cl_bool
    CL_DEVICE_EXECUTION_CAPABILITIES, // cl_device_exec_capabilities
    CL_DEVICE_QUEUE_PROPERTIES, // cl_command_queue_properties
    CL_DEVICE_PLATFORM, // cl_platform_id
    CL_DEVICE_NAME, // char []
    CL_DEVICE_VENDOR, // char[]
    CL_DRIVER_VERSION, // char[]
    CL_DEVICE_PROFILE, // char[]
    CL_DEVICE_VERSION, // char[]
    CL_DEVICE_OPENCL_C_VERSION, // char[]
    CL_DEVICE_EXTENSIONS // char[]
};

#endif // OPENCL_H
