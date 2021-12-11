/* ****************************************************************
 * Copyright 2013 VMware, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * ****************************************************************/

#include "vmkapi.h"
#include "vmklinux_version.h"
#include <linux/module.h>
/* NOTE: __namespace.h is generated by the build from the driver's .sc file. */
#include "__namespace.h"

#ifndef MODULE
#error "You can only compile and link vmklinux_module with modules, which" \
       "means that MODULE has to be defined when compiling it..."
#endif

VMK_LICENSE_INFO(VMK_MODULE_LICENSE_GPLV2);
MODULE_VERSION2(VMKLNX_STRINGIFY(PRODUCT_VERSION), VMKLNX_MY_NAMESPACE_VERSION);

/*
 * All vmkdriver modules are built using the latest vmakpi interface.
 */
VMK_NAMESPACE_REQUIRED(VMK_NAMESPACE_VMKAPI, VMK_NAMESPACE_CURRENT_VERSION);
#if VMKAPI_REVISION >= VMK_REVISION_FROM_NUMBERS(2, 2, 0, 0) && \
    defined(VMK_DEVKIT_USES_BINARY_INCOMPATIBLE_APIS)
VMK_NAMESPACE_REQUIRED(VMK_NAMESPACE_VMKAPI_INCOMPAT,
                       VMK_NAMESPACE_INCOMPAT_CURRENT_VERSION);
#endif

vmk_ModuleID vmkshim_module_id;

int
vmk_early_init_module(void)
{
   VMK_ReturnStatus vmk_status;

   vmk_status = vmk_ModuleRegister(&vmkshim_module_id, VMKAPI_REVISION);
   if (vmk_status != VMK_OK) {
      vmk_WarningMessage("Registration failed (%#x): %s",
                          vmk_status, vmk_StatusToString(vmk_status));
      return vmk_status;
   }

   return 0;
}

int
vmk_late_cleanup_module(void)
{
   return 0;
}

/*
 * Symbols passed in directly from libfc without the need for a shim function.
 */
VMK_MODULE_EXPORT_ALIAS(fc_change_queue_depth);
VMK_MODULE_EXPORT_ALIAS(fc_change_queue_type);
VMK_MODULE_EXPORT_ALIAS(fc_cpu_mask);
VMK_MODULE_EXPORT_ALIAS(fc_disc_init);
VMK_MODULE_EXPORT_ALIAS(fc_eh_abort);
VMK_MODULE_EXPORT_ALIAS(fc_eh_device_reset);
VMK_MODULE_EXPORT_ALIAS(fc_eh_host_reset);
VMK_MODULE_EXPORT_ALIAS(fc_elsct_init);
VMK_MODULE_EXPORT_ALIAS(fc_elsct_send);
VMK_MODULE_EXPORT_ALIAS(fc_exch_init);
VMK_MODULE_EXPORT_ALIAS(fc_exch_mgr_add);
VMK_MODULE_EXPORT_ALIAS(fc_exch_mgr_alloc);
VMK_MODULE_EXPORT_ALIAS(fc_exch_mgr_del);
VMK_MODULE_EXPORT_ALIAS(fc_exch_mgr_free);
VMK_MODULE_EXPORT_ALIAS(fc_exch_mgr_reset);
VMK_MODULE_EXPORT_ALIAS(fc_exch_recv);
VMK_MODULE_EXPORT_ALIAS(fc_fabric_login);
VMK_MODULE_EXPORT_ALIAS(fc_fabric_logoff);
VMK_MODULE_EXPORT_ALIAS(fc_fcp_destroy);
VMK_MODULE_EXPORT_ALIAS(fc_fcp_init);
VMK_MODULE_EXPORT_ALIAS(_fc_frame_alloc);
VMK_MODULE_EXPORT_ALIAS(fc_frame_alloc_fill);
VMK_MODULE_EXPORT_ALIAS(fc_frame_crc_check);
VMK_MODULE_EXPORT_ALIAS(fc_get_host_port_state);
VMK_MODULE_EXPORT_ALIAS(fc_get_host_port_type);
VMK_MODULE_EXPORT_ALIAS(fc_get_host_speed);
VMK_MODULE_EXPORT_ALIAS(fc_get_host_stats);
VMK_MODULE_EXPORT_ALIAS(fc_linkdown);
VMK_MODULE_EXPORT_ALIAS(fc_linkup);
VMK_MODULE_EXPORT_ALIAS(fc_lport_config);
VMK_MODULE_EXPORT_ALIAS(fc_lport_destroy);
VMK_MODULE_EXPORT_ALIAS(fc_lport_flogi_resp);
VMK_MODULE_EXPORT_ALIAS(fc_lport_init);
VMK_MODULE_EXPORT_ALIAS(fc_lport_logo_resp);
VMK_MODULE_EXPORT_ALIAS(fc_lport_reset);
VMK_MODULE_EXPORT_ALIAS(fc_queuecommand);
VMK_MODULE_EXPORT_ALIAS(fc_rport_init);
VMK_MODULE_EXPORT_ALIAS(fc_rport_terminate_io);
VMK_MODULE_EXPORT_ALIAS(fc_set_mfs);
VMK_MODULE_EXPORT_ALIAS(fc_set_rport_loss_tmo);
VMK_MODULE_EXPORT_ALIAS(fc_slave_alloc);
VMK_MODULE_EXPORT_ALIAS(fc_slave_configure);
VMK_MODULE_EXPORT_ALIAS(fc_vport_setlink);
VMK_MODULE_EXPORT_ALIAS(libfc_lun_qdepth);
VMK_MODULE_EXPORT_ALIAS(libfc_vport_create);
VMK_MODULE_EXPORT_ALIAS(vmklnx_fc_eh_bus_reset);
