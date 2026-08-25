///////////////////////////////////////////////////////////////////////////////
/// @file      SatsJhm2ScsOutputs.h
/// @author    Safeer
/// @date      3/28/2017
/// @brief     Structure containing all going out to SCS
///
/// @attention COPYRIGHT (C) 2012 CATERPILLAR INC. ALL RIGHTS RESERVED.
///////////////////////////////////////////////////////////////////////////////
#ifndef SATSJHM2SCSOUTPUTS_H_
#define SATSJHM2SCSOUTPUTS_H_

#include <ais/config/ConfigSection.h>
#include <scsIOContainer/SCSOutputs.h>
#include <scsIOContainer/SCSOutData.h>

//Interfaces we want to receive and store

#include <interfaces/LpsSaWeighReqstChannel/InterfaceTypes.h>
#include <interfaces/MachineSN/InterfaceTypes.h>

class AisJhm2ScsOutputs : public SCSOutputs
{

 public:
  
  SCSOutData<MachineSN>                m_jhm_MachineSN;
  ///////////////////////////////////////////////////////////////////////////////
  /// @brief Run "send" on all of the scs channels in the structure
  ///
  /// @return true if any channel has new data, false otherwise
  ///////////////////////////////////////////////////////////////////////////////

  bool send()
  {
    bool retVal = false;
    if ( m_jhm_MachineSN.send() ) retVal = true;
    return retVal;
}

  ///////////////////////////////////////////////////////////////////////////////
  /// @brief Run "initPublishInterface" on all of the scs channels in the structure
  ///
  /// @return true if all interfaces were successfully configured, false if any failed
  ///////////////////////////////////////////////////////////////////////////////
  bool initPublishInterface( ConfigSection& cs )
  {
    bool retVal = true;
    
    if ( !m_jhm_MachineSN.initPublishInterface("MachineSNOutput", cs) ) retVal = false;
    printf("MachineSNOutput publish  retVal:%d \n",retVal);

    return retVal;
  }

};

#endif /*SATSJHM2SCSOUTPUTS_H_*/
