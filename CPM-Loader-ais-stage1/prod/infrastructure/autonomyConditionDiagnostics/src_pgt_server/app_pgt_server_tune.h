/*******************************************************************************
Copyright 2014 Caterpillar Inc.  All rights reserved.                      
--------------------------------------------------------------------------------
Filename:  csf_pgt_server_tune.h 

Description: 
The application will copy this file and adjust the PMF #defines to meet
their needs
*******************************************************************************/
#ifndef CSF_PGT_SRV_TUNE_H_
#define CSF_PGT_SRV_TUNE_H_

#ifdef __cplusplus
extern "C" {
#endif
/* Maximum number of bytes for a UDP txt message. This is Data buffer size for PID data 
   UDP packet format as per PGT protocol
    _______________________________________________________________________________________________
   |Nm | Ns | session-id | Ng | group-id | client-fid | server-fid | sequence-number | data, where |
   |_______________________________________________________________________________________________|
   Nm (2 bytes) = byte count of whole PGT message excluding Nm  = 1 + Ns + 1 + Ng + 4 + 4 + 2 + Nd
   Ns (1 byte) = byte count of session-id 
   session-id (Ns bytes) = session identifier
   Ng (1 byte) = byte count of group-id
   group-id (Ng bytes) = group identifier
   client-fid (4 bytes) = client functional identifier
   server-fid (4 bytes) = server functional identifier
   sequence-number (2 bytes) = message sequence number to prevent reception of stale messages
   data  (Nd bytes) = parameter data
   Nd = byte count of parameter data -- not an explicit field in the message, used only to determine Nm 
   
   The value of CSF_PGT_SERVER_MAX_GRP_UDP_TX_BYTES macro is  calculated based on number of PIDs and PID legnth.
   
   CSF_PGT_SERVER_MAX_GRP_UDP_TX_BYTES = (16+ total PID's data length)
   14 is the Nm(2)+NS(1)+Session-id(1)+Ng(1)+group-id(1)+Client-fid(4)+Server-fid(4)+ Sequence number(2)
   
 */ 
#define CSF_PGT_SERVER_MAX_GRP_UDP_TX_BYTES               1024

/*Maximum number of PID supported. for all clients */
#define CSF_PGT_SERVER_MAX_GROUP_COUNT            50

/*Maximum number of session supported for all clients*/
#define CSF_PGT_SERVER_MAX_SESSION_COUNT                 50

/*Server Maximum Parameters count for all clients*/
#define  CSF_PGT_SERVER_MAX_PARAM_COUNT            250

#ifdef __cplusplus
}
#endif


#endif /* CSF_PMF_SRV_TUNE_H_ */