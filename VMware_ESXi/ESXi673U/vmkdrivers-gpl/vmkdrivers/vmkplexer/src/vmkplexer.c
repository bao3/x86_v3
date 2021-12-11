/* **********************************************************
 * Copyright 2010 VMware, Inc.  All rights reserved.
 * **********************************************************/

/*
 * vmkplexer.c
 */

#include "vmkapi.h"
#include <vmkplexer_module.h>
#include <vmkplexer_log.h>
#include <vmkplexer_init.h>
#include "vmkplexer_version.h"

#ifndef MODULE
#error "You can only compile and link vmkplexer with modules, which" \
       "means that MODULE has to be defined when compiling it..."
#endif

VMK_VERSION_INFO(                                                       \
   "Version " VMKPLX_BUILD_VERSION ", "                                 \
   "Build: " VMK_REVISION_EXPANDSTR(BUILD_NUMBER_NUMERIC) ", "          \
   " Built on: " __DATE__);

/* Modules are built using the latest vmkapi */
VMK_NAMESPACE_REQUIRED(VMK_NAMESPACE_VMKAPI,
                       VMK_NAMESPACE_CURRENT_VERSION);
#if VMKAPI_REVISION >= VMK_REVISION_FROM_NUMBERS(2, 2, 0, 0) && \
    defined(VMK_DEVKIT_USES_BINARY_INCOMPATIBLE_APIS)
VMK_NAMESPACE_REQUIRED(VMK_NAMESPACE_VMKAPI_INCOMPAT,
                       VMK_NAMESPACE_INCOMPAT_CURRENT_VERSION);
#endif

/*
 * vmkplexer defines its namespace in the vmkplexer.sc file.
 * NOTE: __namespace.h is generated by the build from the driver's .sc file.
 */
#include "__namespace.h"
VMK_LICENSE_INFO(VMK_MODULE_LICENSE_GPLV2);

static int VmkplxrHeapInit(void);
static int VmkplxrHeapDestroy(void);
static VMK_ReturnStatus VmkplxrMemPoolInit(void);
static void VmkplxrMemPoolDestroy(void);

vmk_ModuleID vmkplxr_module_id;
vmk_HeapID vmkplxr_heap_id = VMK_INVALID_HEAP_ID;
vmk_LogComponent vmkplexerLog;
vmk_MemPool vmkplexer_mem_pool = VMK_MEMPOOL_INVALID;

#define VMKPLXR_HEAP_NAME "vmkplexer-heap"
#define VMKPLXR_MEM_POOL_NAME "vmkplexer"

int
vmk_early_init_module(void)
{
   int status;
   VMK_ReturnStatus vmk_status;

   vmk_status = vmk_ModuleRegister(&vmkplxr_module_id, VMKAPI_REVISION);
   if (vmk_status != VMK_OK) {
      vmk_WarningMessage("Registration failed (%#x): %s",
                          vmk_status, vmk_StatusToString(vmk_status));
      return vmk_status;
   }

   vmk_status = VmkplxrMemPoolInit();
   VMK_ASSERT(vmk_status == VMK_OK);

   status = VmkplxrHeapInit();
   if (status != 0) {
      return status;
   }

   vmk_LogMessage("vmkplexer registration succeeded!");
   return status; 
}

int init_module(void)
{
   VMK_ReturnStatus vmk_status;
   vmk_LogProperties logProps;

   vmk_status = vmk_NameInitialize(&logProps.name, "vmkplexer");
   VMK_ASSERT(vmk_status == VMK_OK);
   logProps.module = vmkplxr_module_id;
   logProps.heap = vmkplxr_heap_id;
   logProps.defaultLevel = 0;
   logProps.throttle = NULL;
   vmk_status = vmk_LogRegister(&logProps, &vmkplexerLog);
   if (vmk_status != VMK_OK) {
      vmk_WarningMessage("Failed to create vmkplexer's log (%#x): %s",
                         vmk_status, vmk_StatusToString(vmk_status));
      goto error_log;
   } 

   vmk_status = vmkplxr_ChardevsInit();
   if (vmk_status != VMK_OK) {
      VMKPLXR_WARN("Initialization of chardev subsystem failed (%#x): %s",
                     vmk_status, vmk_StatusToString(vmk_status));
      goto error_chardevs;
   } 
   vmk_status = vmkplxr_EntropyInit();
   if (vmk_status != VMK_OK) {
      VMKPLXR_WARN("Initialization of entropy subsystem failed (%#x): %s",
                    vmk_status, vmk_StatusToString(vmk_status));
      goto error_entropy;
   }

   vmk_status = vmkplxr_ProcfsInit();
   if (vmk_status != VMK_OK) {
      VMKPLXR_WARN("Initialization of procfs subsystem failed (%#x): %s",
                     vmk_status, vmk_StatusToString(vmk_status));
      goto error_procfs;
   } 

   vmk_status = vmkplxr_ScsiInit();

   if (vmk_status != VMK_OK) {
      VMKPLXR_WARN("Initialization of scsi subsystem failed (%#x): %s",
                     vmk_status, vmk_StatusToString(vmk_status));
      goto error_scsi;
   }

   return 0;

error_scsi:
error_procfs:
error_entropy:
error_chardevs:
error_log:
   return -1;   
}

int
vmk_late_cleanup_module(void)
{
   int status = 0;

   vmkplxr_ScsiCleanup();
   vmkplxr_ProcfsCleanup();

   status = VmkplxrHeapDestroy();

   VmkplxrMemPoolDestroy();

   return status;
}

static int 
VmkplxrHeapInit(void)
{
   VMK_ReturnStatus vmk_status;
   int status = 0;
   int heap_max = VMKPLXR_HEAP_MAX;
   int heap_initial = VMKPLXR_HEAP_INITIAL;
   vmk_HeapCreateProps heapProps;

   heapProps.type = VMK_HEAP_TYPE_SIMPLE;
   status = vmk_NameInitialize(&heapProps.name, VMKPLXR_HEAP_NAME);
   VMK_ASSERT(status == VMK_OK);
   heapProps.module = vmk_ModuleStackTop();
   heapProps.initial = heap_initial;
   heapProps.max = heap_max;
   heapProps.creationTimeoutMS = VMK_TIMEOUT_NONBLOCKING;

   vmk_LogMessage("%s:  initial size : %d, max size: %d",
                  VMKPLXR_HEAP_NAME, heap_initial, heap_max);

   vmk_status = vmk_HeapCreate(&heapProps, &vmkplxr_heap_id);

   if (vmk_status != VMK_OK) {
      vmk_LogMessage("%s: heap creation failed - %s",
                     VMKPLXR_HEAP_NAME, vmk_StatusToString(status));
      status = -1;
   } else {
      vmk_LogMessage("%s: heap creation succeeded. id = %p",
                     VMKPLXR_HEAP_NAME, vmkplxr_heap_id);
   }

   vmk_ModuleSetHeapID(vmkplxr_module_id, vmkplxr_heap_id);

   return status;
}

static int 
VmkplxrHeapDestroy(void)
{
   vmk_HeapDestroy(vmkplxr_heap_id);
   vmkplxr_heap_id = VMK_INVALID_HEAP_ID;
   return 0;
}

static VMK_ReturnStatus
VmkplxrMemPoolInit(void)
{
   VMK_ReturnStatus status;
   vmk_MemPoolProps props;

   props.module = vmk_ModuleStackTop();
   props.parentMemPool = VMK_MEMPOOL_INVALID;
   props.memPoolType = VMK_MEM_POOL_PARENT;
   props.resourceProps.reservation = 0;
   props.resourceProps.limit = VMK_MEMPOOL_NO_LIMIT;
   status = vmk_NameInitialize(&props.name, VMKPLXR_MEM_POOL_NAME);
   VMK_ASSERT(status == VMK_OK);

   return vmk_MemPoolCreate(&props, &vmkplexer_mem_pool);
}


VMK_ReturnStatus
vmkplxr_GetMemPool(vmk_MemPool *mem_pool)
{
   VMK_ASSERT(vmk_PreemptionIsEnabled() == VMK_FALSE);
   VMK_ASSERT(vmkplexer_mem_pool != VMK_MEMPOOL_INVALID);

   if (vmkplexer_mem_pool != VMK_MEMPOOL_INVALID) {
      *mem_pool = vmkplexer_mem_pool;
      return VMK_OK;
   }
   *mem_pool = VMK_MEMPOOL_INVALID;
   return VMK_NOT_FOUND;
}
VMK_MODULE_EXPORT_SYMBOL(vmkplxr_GetMemPool);


static void
VmkplxrMemPoolDestroy(void)
{
   if (vmkplexer_mem_pool != VMK_MEMPOOL_INVALID) {
      vmk_MemPoolDestroy(vmkplexer_mem_pool);
   }
   vmkplexer_mem_pool = VMK_MEMPOOL_INVALID;
}
