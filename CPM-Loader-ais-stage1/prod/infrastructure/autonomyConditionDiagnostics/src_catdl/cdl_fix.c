/*******************************************************************************
 * cdl_fix.c
 *
 * CDL stub for platforms without physical CDL hardware (A7H4).
 * Based on the upstream datalinkManager/src_catdl/cdl_fix.c pattern.
 *
 * Why cdl2_ is linked but cdl_ is not:
 *   libcal.a(cal_interface_init.o) needs cdl2_transmit → libcdl2_.a must be linked
 *   libcdl2_.a(cdl2_utils.o) in turn references cdl_enq_tx_msg, cdl_tx_msg_stat,
 *   cdl_tx_msg_stat_reset from libcdl_.a.  But libcdl_.a needs hal_cdl_* hardware
 *   symbols unavailable on A7H4.  These 3 stubs satisfy cdl2_utils.o without
 *   pulling in libcdl_.a.
 *******************************************************************************/
#ifndef STD_TYPES_H_
#include <std_types.h>
#endif

#include <cdl2_proto.h>
#include <cdl_proto.h>
#include <oel_rtos_posix.h>
#include <string.h>

/* ---------------------------------------------------------------------------
 * cdl2_tx_f_hdlr - standard CDL2 TX frame handler (upstream pattern)
 * --------------------------------------------------------------------------- */
int_16 cdl2_tx_f_hdlr(void *pie, unsigned_8 *dest_data)
{
   Cdl_gen_tx_pie_t *source;
   int_16 len;
   unsigned_8 data8;
   unsigned_16 data16;
   unsigned_32 data32;

   source = pie;
   len = source->len;

   switch (len)
   {
      case 1:
         data8 = *((unsigned_8 *)source->pdata);
         OEL_PACK_BE_8_NO_INCR(dest_data, data8);
         break;
      case 2:
         data16 = *((unsigned_16 *)source->pdata);
         OEL_PACK_BE_16_NO_INCR(dest_data, data16);
         break;
      case 4:
         data32 = *((unsigned_32 *)source->pdata);
         OEL_PACK_BE_32_NO_INCR(dest_data, data32);
         break;
      default:
         memcpy(dest_data, source->pdata, len);
         break;
   }
   return len;
}

/* ---------------------------------------------------------------------------
 * Stubs for cdl_ symbols referenced by libcdl2_.a(cdl2_utils.o).
 * cdl2_transmit (called by libcal.a) pulls in cdl2_utils.o which needs these.
 * Not exercised at runtime on A7H4 (no CDL hardware).
 * --------------------------------------------------------------------------- */

int_16 cdl_enq_tx_msg(Cdl_buf_t *buf, int_16 msg_len)
{
   (void)buf;
   (void)msg_len;
   return -1;
}

int_16 cdl_tx_msg_stat(Cdl_buf_t *buf)
{
   (void)buf;
   return 0;
}

void cdl_tx_msg_stat_reset(Cdl_buf_t *buf)
{
   (void)buf;
}
