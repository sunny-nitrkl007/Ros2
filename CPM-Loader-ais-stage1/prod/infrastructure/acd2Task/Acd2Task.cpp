///////////////////////////////////////////////////////////////////////////////
// @attention COPYRIGHT (C) 2011-2015 CATERPILLAR INC. ALL RIGHTS RESERVED.
//
// @file    Acd2Task.cpp
//
// @brief   This is the autonomyCondition diagnostics and events application.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// -- #Include's --
///////////////////////////////////////////////////////////////////////////////

#include <ais/time/PassiveTimer.h>
#ifdef CAL_MANAGER_SUPPORTED
#include <taskPlugins/calManager/CalManager.h>
#endif
#include <taskPlugins/machineSNPublisher/MachineSNPublisher.h>
#include <taskPlugins/interfaces/PlatformDefines.h>
#if SCL_CRYPTO_SUPPORTED
#include <taskPlugins/seaManager/SEAManager.h>
#endif
#include <taskPlugins/tripDataManager/TripDataManager.h>
#include <taskPlugins/datalinkManagerTest/TestDatalinkManagerDataConsumer.hpp>
#include <taskPlugins/datalinkManagerTest/TestDatalinkManagerDataProducer.hpp>

#include "Acd2Task.h"

using namespace task;

///////////////////////////////////////////////////////////////////////////////
// -- #Define, Struct's, Typedef's, Enum's --

///////////////////////////////////////////////////////////////////////////////
// -- Symbols --
///////////////////////////////////////////////////////////////////////////////

TripDataManagerInterface* pTripDataManager{nullptr};

///////////////////////////////////////////////////////////////////////////////
// -- Start of code for this file --
///////////////////////////////////////////////////////////////////////////////

AbstractTaskCore *task::getTaskImplementation()
{
   static Acd2Task thisTask("Acd2Task");
   return dynamic_cast<Task *>(&thisTask);
}


Acd2Task::Acd2Task(const std::string &taskName) :
    Task(taskName),
    Acd2Lib(static_cast<task::Task &>(*this))
{
}


Acd2Task::~Acd2Task()
{
}


bool Acd2Task::initialize(void)
{
   // Initialize Acd2Lib
   if (!Acd2Lib::initialize()) return false;

   // *** Add Application specific code here ***

   return true;
}


bool Acd2Task::executive(void)
{
   // Update Acd2Lib
   if (!Acd2Lib::executive()) return false;

   // *** Add Application specific code here ***

   return true;
}

void Acd2Task::cleanup(void)
{
   // Shutdown TripDataManager gracefully before stopping the OEL tasks
   if (pTripDataManager != nullptr)
   {
      pTripDataManager->shutdown();
   }

   // Cleanup Acd2Lib
   Acd2Lib::cleanup();

   // *** Add Application specific code here ***

}

bool Acd2Task::addPlugins()
{
   AIS_LOG_DEBUG("addPlugins");

   // Add Acd2Lib plugins
   addAcdPlugins();

   // *** Add Application specific plugins here, must be after addAcdPlugins() ***
#if SCL_CRYPTO_SUPPORTED
   addOelPlugin<SEAManager>(*m_pDatalinkManager, *m_pDatalinkManager, *this, *this);
#endif
#ifdef CAL_MANAGER_SUPPORTED
   addOelPlugin<CalManager>(*m_pDatalinkManager, *this, *m_pDatalinkManager);
#endif
   pTripDataManager = &addOelPlugin<TripDataManager>(*m_pDatalinkManager, *m_pDatalinkManager, *this, *this);
   addPlugin<MachineSNPublisher>(*m_pDatalinkManager);

   // Test Data Producer, used for setting Tx values from Ruby or csv
   addPlugin<TestDatalinkManagerDataProducer>(*m_pDatalinkManager, *m_pDatalinkManager);

   // Test Data Consumer, useful for logging Rx values
   addPlugin<TestDatalinkManagerDataConsumer>(*m_pDatalinkManager);

   // Return false if any plugins have faulted
   return addPluginsSuccessful();
}
