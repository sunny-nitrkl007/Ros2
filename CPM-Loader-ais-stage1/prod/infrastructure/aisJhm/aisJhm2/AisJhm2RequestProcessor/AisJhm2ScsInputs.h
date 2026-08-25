///////////////////////////////////////////////////////////////////////////////
/// @file      AisJhm2ScsInputs.h
/// @author    J Struble
/// @date      3/22/2013
/// @brief     Structure containing all data coming in from SCS that needs
///            to displayed in jhm2
///
/// @attention COPYRIGHT (C) 2012 CATERPILLAR INC. ALL RIGHTS RESERVED.
///////////////////////////////////////////////////////////////////////////////
#ifndef AISJHM2SCSINPUTS_H_
#define AISJHM2SCSINPUTS_H_

#include <scsIOContainer/SCSInputs.h>
#include <scsIOContainer/SCSInData.h>
#include <ais/config/ConfigSection.h>

//Interfaces we want to receive and store
#include <interfaces/AisJhm2TxChannel/InterfaceTypes.h>
#include <interfaces/DemoAppTxChannel/InterfaceTypes.h>
#include <interfaces/EventDiagnosticData/InterfaceTypes.h>
#include <interfaces/WorkOrderAssist/TxInterface.hpp>


class AisJhm2ScsInputs : public SCSInputs
{
public:
    SCSInData<DemoAppTxChannel> m_jhm_demo;
    SCSInData<AisJhm2TxChannel> m_jhm_payLoad;
    SCSInData<EventDiagnosticData> m_jhm_eddt;
    SCSInData<WorkOrderAssistTxInterface> m_jhm_dispatch_tx;

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Run "update" on all of the scs channels in the structure
    ///
    /// @return true if any channel has new data, false otherwise
    ///////////////////////////////////////////////////////////////////////////////
    bool update()
    {
        bool retVal = false;

        if (m_jhm_demo.update()) { retVal = true; }
        if (m_jhm_payLoad.update()) { retVal = true; }
        if (m_jhm_eddt.update()) { retVal = true; }
        if (m_jhm_dispatch_tx.update()) { retVal = true; }

        return retVal;
    }

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Run "initReadInterface" on all of the scs channels in the structure
    ///
    /// @return true if all interfaces were successfully configured, false if any failed
    ///////////////////////////////////////////////////////////////////////////////
    bool initReadInterface(ConfigSection& cs)
    {
        bool retVal = true;

        if (!m_jhm_demo.initReadInterface("DemoAppTxChannelInput", cs)) { retVal = false; }
        if (!m_jhm_payLoad.initReadInterface("AisJhm2TxChannelInput", cs)) { retVal = false; }
        if (!m_jhm_eddt.initReadInterface("EventDiagnosticDataInput", cs)) { retVal = false; }
        if (!m_jhm_dispatch_tx.initReadInterface("DispatchTxInput", cs)) { retVal = false; }

        return retVal;
    }

};

#endif /*AISJHM2SCSINPUTS_H_*/
