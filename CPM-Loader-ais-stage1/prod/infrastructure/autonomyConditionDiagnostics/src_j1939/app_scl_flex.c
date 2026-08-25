/*******************************************************************************
 Copyright 2019 Caterpillar Inc. All rights reserved.
--------------------------------------------------------------------------------
Filename:   app_scl_flex.c

Description: This file contains the data to be used by scl_flex to flex across
to ET and initialization function to initialize this data.
*******************************************************************************/

#ifndef STD_T_H_
#include <std_t.h>
#endif

#ifndef SCL_FLEX_DEF_STRUCTS_H_
#include <scl_flex_def_structs.h>
#endif

#ifndef SCL_FLEX_H_
#include <scl_flex.h>
#endif

#ifndef SCL_FLEX_ENUM_INDICES_H_
#include <scl_flex_enum_indices.h>
#endif

/* For printf */
#include <stdio.h>


/* PIDFC06 Data Characteristics: DSI not supported, LSB first and Fixed data length */ 
#define LOADCOUNTCHARACTERISTICS SCL_FLEX_DSI_NA | SCL_FLEX_DATA_ORDER_LSB | SCL_FLEX_DATA_LNG_FIXED

/* PIDFC06 Encoding type: */ 
#define LOADCOUNTDATATYPE SCL_FLEX_ENCODING_USCALED_ZERO

/* PIDFC06 Misc.:  */
#define LOADCOUNTPIDMISC SCL_FLEX_DECIMAL_PRECISION_0


/*NOTE: PID 0xF9B4 was flexed for testing purposes. 
        It is now configured as NS for both ET Config and ET ECM Summary screen 
*/
/* PIDF9B4 Data Characteristics: DSI not supported, No data order and Variable data length */ 
#define MODULEDEFNCHARACTERISTICS SCL_FLEX_DSI_NA | SCL_FLEX_DATA_ORDER_NONE | SCL_FLEX_DATA_LNG_VAR_BYTE_CNT /* Char */

/* PIDF9B4 Encoding type: */ 
#define MODULEDEFNDATATYPE SCL_FLEX_ENCODING_ASCII

/* PIDF9B4 Misc.:  */
#define MODULEDEFNPIDMISC SCL_FLEX_CONFIG_NS & SCL_FLEX_STATUS_NS & SCL_FLEX_TOTALS_NS


/*NOTE: PID 0xD10049 was flexed for testing purposes. 
        It is now configured as NS for both ET Config and ET ECM Summary screen 
*/
/* PIDD10049 Data Characteristics: DSI supported, MSB first and Fixed data length */ 
#define LINKAGECHARACTERISTICS SCL_FLEX_DSI_32_LRGST_VALUE | SCL_FLEX_DATA_ORDER_MSB | SCL_FLEX_DATA_LNG_FIXED

/* PIDD10049 Encoding type: */ 
#define LINKAGEDATATYPE SCL_FLEX_ENCODING_SLISTED

/* PIDD10049 Misc.:  */
#define LINKAGEPIDMISC SCL_FLEX_CONFIG_NS & SCL_FLEX_STATUS_NS & SCL_FLEX_TOTALS_NS & SCL_FLEX_DECIMAL_PRECISION_0


/*NOTE: PID 0xD02BB6 was flexed for testing purposes. 
        It is now configured as NS for both ET Config and ET ECM Summary screen 
*/
/* PIDD02BB6 Data Characteristics: DSI supported, LSB first and Fixed data length */ 
#define PITCHCALOFFSETCHARACTERISTICS SCL_FLEX_DSI_32_SMLST_VALUE | SCL_FLEX_DATA_ORDER_LSB | SCL_FLEX_DATA_LNG_FIXED

/* PIDD02BB6 Encoding type: */ 
#define PITCHCALOFFSETDATATYPE SCL_FLEX_ENCODING_SCALED_ZERO

/* PIDD02BB6 Misc.:  */
#define PITCHCALOFFSETPIDMISC SCL_FLEX_CONFIG_NS & SCL_FLEX_STATUS_NS & SCL_FLEX_TOTALS_NS & SCL_FLEX_DECIMAL_PRECISION_3


/*NOTE: PID 0xD02BB7 was flexed for testing purposes. 
        It is now configured as NS for both ET Config and ET ECM Summary screen 
*/
/* PIDD02BB7 Data Characteristics: DSI supported, LSB first and Fixed data length */ 
#define TIPOFFCALOFFSETCHARACTERISTICS SCL_FLEX_DSI_32_SMLST_VALUE | SCL_FLEX_DATA_ORDER_LSB | SCL_FLEX_DATA_LNG_FIXED

/* PIDD02BB7 Encoding type: */ 
#define TIPOFFCALOFFSETDATATYPE SCL_FLEX_ENCODING_SCALED_ZERO

/* PIDD02BB7 Misc.:  */
#define TIPOFFCALOFFSETPIDMISC SCL_FLEX_CONFIG_NS & SCL_FLEX_STATUS_NS & SCL_FLEX_TOTALS_NS & SCL_FLEX_DECIMAL_PRECISION_3


/*NOTE: PID 0xD02BB8 was flexed for testing purposes. 
        It is now configured as NS for both ET Config and ET ECM Summary screen 
*/
/* PIDD02BB8 Data Characteristics: DSI supported, LSB first and Fixed data length */ 
#define TIPOFFCALSLOPECHARACTERISTICS SCL_FLEX_DSI_32_LRGST_VALUE | SCL_FLEX_DATA_ORDER_LSB | SCL_FLEX_DATA_LNG_FIXED

/* PIDD02BB8 Encoding type: */ 
#define TIPOFFCALSLOPESETDATATYPE SCL_FLEX_ENCODING_USCALED_ZERO

/* PIDD02BB8 Misc.:  */
#define TIPOFFCALSLOPESETPIDMISC SCL_FLEX_CONFIG_NS & SCL_FLEX_STATUS_NS & SCL_FLEX_TOTALS_NS & SCL_FLEX_DECIMAL_PRECISION_4


/* State listed data values that are allowed */
const uint_least32_t app_slisted_dvalues[] = {
      0x0034, /* Standard */
      0x0089, /* High Lift */
      0x008A, /* Short Lift */
      0x0108, /* Special Application */
      0x016F, /* Logger */
      0x0170, /* Highlift Grapple */
      0x0171, /* Block Handler */
      0x0172, /* Integrated Tool Carrier */
      0x01C0, /* Extende Capacity */
      0x002C, /* Not Configured (Read Only.  Use of PID $F85B is preferred) */
      0x01F0, /* Mid Lift */
      0x04E7, /* Super High Lift */
      0x04E8, /* Extended High Lift */
      0x0088, /* Heavy Lift */
      0x063B, /* Steel Mill */
      0x0821 /* High Lift Plus */
};


const scl_flex_state_listed_t app_state_listed =
{
   SCL_FLEX_STATE_IDX_CDL_ID,      /* state_id_list_type */
   16,                             /* number of accepted data values */
   app_slisted_dvalues             /* data values */
};

/* PIDD10049 displays $0034 as "Available" instead of "Standard" on ET. 
   Hence, I am trying to flex $0034 text description as "Standard". 
   However, ET team said it will not work because $0034 is already 
   defined in ET database. Howevever, I am keeping the code
   for future reference for other flexing development.
*/
// const uint_least8_t app_pid_state_label_0[30] = "Standard";

//const scl_flex_pid_state_dvalue_lbl_t app_pid_dvalue_and_labels[1]=
//{
//   {
//      0x0034,                          /* State Data Value */
//      sizeof(app_pid_state_label_0),   /* State Label Byte Count */
//      app_pid_state_label_0            /* State Label */
//   }, 
//};


const scl_flex_ascii_encoded_text_t module_defn_config_listed =
{
   
   SCL_FLEX_ENCODED_TEXT_DLIM_PAD_NA,  /* flags */
   SCL_FLEX_ENCODED_TEXT_ASCII_VAL_NA  /* padding value */
   
};

const scl_flex_scaled_zoffset_t app_pid_load_count =
{
   SCL_FLEX_SIDX_1,                 /* Scaling Index - None (1 count/bit)  */
   SCL_FLEX_UIDX_NONE,              /* Unit Index - None      */
};

const scl_flex_scaled_zoffset_t app_pid_pitch_cal_offset =
{
   SCL_FLEX_SIDX_1_DIV_1000,         /* Scaling Index - None (1 count/bit)  */
   SCL_FLEX_UIDX_DEGREE,             /* Unit Index - None      */
};

const scl_flex_scaled_zoffset_t app_pid_tipoff_cal_offset =
{
   SCL_FLEX_SIDX_1_DIV_1000,         /* Scaling Index - None (1 count/bit)  */
   SCL_FLEX_UIDX_TONNE,             /* Unit Index - None      */
};

const scl_flex_scaled_zoffset_t app_pid_tipoff_cal_slope =
{
   SCL_FLEX_SIDX_1_DIV_10000,         /* Scaling Index - None (1 count/bit)  */
   SCL_FLEX_UIDX_NONE,             /* Unit Index - None      */
};

const scl_flex_pid_t app_pid_definitions[] =
{
   /* Load Count Totals */
   {
      {
         SCL_FLEX_ID_CDLPID,         /* Identifier Type       */
         0xFC06                      /* PID Identifier        */
      },
      LOADCOUNTCHARACTERISTICS,      /* Characteristics       */
      32,                            /* Data Size - 4bytes    */
      LOADCOUNTDATATYPE,             /* Data Type             */
      LOADCOUNTPIDMISC,              /* Pid Misc              */
      &app_pid_load_count            /* Scaling               */
   },
 
   /* Machine Model */
   {
      {
         SCL_FLEX_ID_CDLPID,         /* Identifier Type       */
         0xF9B4                      /* PID Identifier        */
      },
      MODULEDEFNCHARACTERISTICS,     /* Characteristics       */
      0xFFFF,                           /* Data Size - Upto 27bytes    */
      MODULEDEFNDATATYPE,            /* Data Type             */
      MODULEDEFNPIDMISC,             /* Pid Misc              */
      &module_defn_config_listed     /* Scaling               */
   },

   /* Linkage Configuration */
   {
      {
         SCL_FLEX_ID_CDLPID,         /* Identifier Type       */
         0xD10049                    /* PID Identifier        */
      },
      LINKAGECHARACTERISTICS,        /* Characteristics       */
      16,                            /* Data Size - 2bytes    */
      LINKAGEDATATYPE,               /* Data Type             */
      LINKAGEPIDMISC,                /* Pid Misc              */
      &app_state_listed              /* Scaling  */            
   },

   /* Pitch Cal Offset Configuration */
   {
      {
         SCL_FLEX_ID_CDLPID,         /* Identifier Type       */
         0xD02BB6                    /* PID Identifier        */
      },
      PITCHCALOFFSETCHARACTERISTICS, /* Characteristics       */
      16,                            /* Data Size - 2bytes    */
      PITCHCALOFFSETDATATYPE,        /* Data Type             */
      PITCHCALOFFSETPIDMISC,         /* Pid Misc              */
      &app_pid_pitch_cal_offset       /* Scaling  */            
   },

   /* TipOff Cal Offset Configuration */
   {
      {
         SCL_FLEX_ID_CDLPID,         /* Identifier Type       */
         0xD02BB7                    /* PID Identifier        */
      },
      TIPOFFCALOFFSETCHARACTERISTICS, /* Characteristics       */
      16,                            /* Data Size - 2bytes    */
      TIPOFFCALOFFSETDATATYPE,        /* Data Type             */
      TIPOFFCALOFFSETPIDMISC,         /* Pid Misc              */
      &app_pid_tipoff_cal_offset       /* Scaling  */            
   },

   /* TipOff Cal Slope Configuration */
   {
      {
         SCL_FLEX_ID_CDLPID,         /* Identifier Type       */
         0xD02BB8                    /* PID Identifier        */
      },
      TIPOFFCALSLOPECHARACTERISTICS, /* Characteristics       */
      16,                            /* Data Size - 2bytes    */
      TIPOFFCALSLOPESETDATATYPE,        /* Data Type             */
      TIPOFFCALSLOPESETPIDMISC,         /* Pid Misc              */
      &app_pid_tipoff_cal_slope       /* Scaling  */            
   },
};


/* Parameter Label structure. */
const uint_least8_t app_labels_pidfc06[]   = "Total Load Count";
const uint_least8_t app_labels_pidf9b4[]   = "Machine Model";
const uint_least8_t app_labels_pidd10049[] = "Implement Linkage Configuration";
const uint_least8_t app_labels_pidd02bb6[] = "Implement Linkage Pitch Calibration Offset";
const uint_least8_t app_labels_pidd02bb7[] = "Payload Tip-Off Weight Calibration Offset";
const uint_least8_t app_labels_pidd02bb8[] = "Payload Tip-Off Weight Calibration Slope";


const scl_flex_id_label_t app_identifier_labels[] =
{
  { 
    {
      SCL_FLEX_ID_CDLPID,            /* ID Type          */
       0xFC06                         /* ID               */
    },
    SCL_FLEX_LABEL_ENCODING_ASCII,    /* Label Encoding   */
    sizeof(app_labels_pidfc06) - 1,        /* Label Size       */
    app_labels_pidfc06                     /* Label            */
 },

 {
   {
      SCL_FLEX_ID_CDLPID,            /* ID Type          */
      0xF9B4                         /* ID               */
   },
   SCL_FLEX_LABEL_ENCODING_ASCII,    /* Label Encoding   */
   sizeof(app_labels_pidf9b4) - 1,        /* Label Size       */
   app_labels_pidf9b4                     /* Label            */
 },

 {
    {
      SCL_FLEX_ID_CDLPID,            /* ID Type          */
       0xD10049                      /* ID               */
    },
    SCL_FLEX_LABEL_ENCODING_ASCII,   /* Label Encoding   */
    sizeof(app_labels_pidd10049) - 1,       /* Label Size       */
    app_labels_pidd10049                    /* Label            */
 },

 {
    {
      SCL_FLEX_ID_CDLPID,            /* ID Type          */
       0xD02BB6                      /* ID               */
    },
    SCL_FLEX_LABEL_ENCODING_ASCII,   /* Label Encoding   */
    sizeof(app_labels_pidd02bb6) - 1,       /* Label Size       */
    app_labels_pidd02bb6                    /* Label            */
 },

 {
    {
      SCL_FLEX_ID_CDLPID,            /* ID Type          */
       0xD02BB7                      /* ID               */
    },
    SCL_FLEX_LABEL_ENCODING_ASCII,   /* Label Encoding   */
    sizeof(app_labels_pidd02bb7) - 1,       /* Label Size       */
    app_labels_pidd02bb7                    /* Label            */
 },

 {
    {
      SCL_FLEX_ID_CDLPID,            /* ID Type          */
       0xD02BB8                      /* ID               */
    },
    SCL_FLEX_LABEL_ENCODING_ASCII,   /* Label Encoding   */
    sizeof(app_labels_pidd02bb8) - 1,       /* Label Size       */
    app_labels_pidd02bb8                    /* Label            */
 }

};

const scl_flex_pid_comm_t app_parameter_comm[] =
{
   {
      {
         SCL_FLEX_ID_CDLPID,                          /* PID Type */
         0xFC06                                       /* PID */
      },
      SCL_FLEX_J1939_MA_RD_S & SCL_FLEX_J1939_MA_WR_S /* Misc Mechanisms */
   },

   {
      {
         SCL_FLEX_ID_CDLPID,                          /* PID Type */
         0xF9B4                                       /* PID */
      },
      SCL_FLEX_J1939_MA_RD_S & SCL_FLEX_J1939_MA_WR_S /* Misc Mechanisms */
   },

   {
      {
         SCL_FLEX_ID_CDLPID,                          /* PID Type */
         0xD10049                                       /* PID */
      },
      SCL_FLEX_J1939_MA_RD_S & SCL_FLEX_J1939_MA_WR_S /* Misc Mechanisms */
   },

   {
      {
         SCL_FLEX_ID_CDLPID,                          /* PID Type */
         0xD02BB6                                       /* PID */
      },
      SCL_FLEX_J1939_MA_RD_S & SCL_FLEX_J1939_MA_WR_S /* Misc Mechanisms */
   },

   {
      {
         SCL_FLEX_ID_CDLPID,                          /* PID Type */
         0xD02BB7                                       /* PID */
      },
      SCL_FLEX_J1939_MA_RD_S & SCL_FLEX_J1939_MA_WR_S /* Misc Mechanisms */
   },

   {
      {
         SCL_FLEX_ID_CDLPID,                          /* PID Type */
         0xD02BB8                                       /* PID */
      },
      SCL_FLEX_J1939_MA_RD_S & SCL_FLEX_J1939_MA_WR_S /* Misc Mechanisms */
   }
};

/* Parameter Security Structure */
const scl_flex_pid_security_t app_pid_security[] =
{
   {
      {
         SCL_FLEX_ID_CDLPID, /* PID Type */
         0xFC06              /* PID */
      },
      SCL_FLEX_NO_SECURITY,  /* Read Security */
      SCL_FLEX_NO_SECURITY,  /* Write Security */
      0xFFFF                 /* Reason Code */
   }
};


/*******************************************************************************
Function name: app_scl_flex_init

Description: This operation will initialize scl_flex and the application's flex
   data.

Parameter Description: None

Return Description: None
*******************************************************************************/
void app_scl_flex_init(void)
{

   uint_least8_t i;
   uint_least8_t num_pids;

   //uint_least8_t num_pidd10049_states;
   
   scl_flex_t *app_scl_flex = scl_flex_init();
 
   /* Required in order to flex PIDS */
   num_pids = (uint_least8_t)(sizeof(app_pid_definitions)/ sizeof(app_pid_definitions[0]));
   for (i=0; i<num_pids; i++)
   {
      scl_flex_add_pid(app_scl_flex, &app_pid_definitions[i] );
   }
   
   /* Required in order to flex PIDs */
   num_pids = (uint_least8_t)(sizeof(app_parameter_comm)/sizeof(app_parameter_comm[0]));
   for (i = 0; i < num_pids; i++) {
         scl_flex_add_pid_comm( app_scl_flex, &app_parameter_comm[i]);
   }
   
   /* Needed if only the security level needs to be flexed. 
      For this application, we need to flex security level for 
      PID FC06
   */
   num_pids = (uint_least8_t)(sizeof(app_pid_security)/sizeof(app_pid_security[0]));
   for (i = 0; i < num_pids; ++i) {
       scl_flex_add_pid_security(app_scl_flex, &app_pid_security[i]);
   }
    
   /* Needed for displaying the PID labels */ 
   num_pids = (uint_least8_t)(sizeof(app_identifier_labels)/sizeof(app_identifier_labels[0]));
   for (i = 0; i < num_pids; ++i) {
       scl_flex_add_id_label(app_scl_flex, &app_identifier_labels[i]);
   }

   /* PIDD10049 displays $0034 as "Available" instead of "Standard" on ET. 
      Hence, I am trying to flex $0034 text description as "Standard".
      However, ET team said it will not work but I am keeping the code
      for future reference for other flexing development.
   */ 
   //num_pidd10049_states = (uint_least8_t)(sizeof(app_pid_dvalue_and_labels) /sizeof(app_pid_dvalue_and_labels[0]));
   //for (i=0; i<num_pidd10049_states; i++)
   //{
   //   scl_flex_add_pid_s_dvalue_lbl( app_scl_flex, 0xD10049, SCL_FLEX_ID_CDLPID, &app_pid_dvalue_and_labels[i] );
   //}
}
