///////////////////////////////////////////////////////////////////////////////
/// @file      aisJhm2DataServer.h
/// @author    Josh Struble
/// @date      Aug 15, 2014
/// @brief     App to serve data to CGI web script
///
/// @attention Copyright (C) 2013
/// @attention Caterpillar, Inc.
///////////////////////////////////////////////////////////////////////////////
#ifndef aisJhm2DataServer_h
#define aisJhm2DataServer_h

#include "ais/task/Task.h"

#include <jhm2/Jhm2Ipc/Jhm2IpcServer.h>
#include "../AisJhm2RequestProcessor/AisJhm2RequestProcessor.h"
#include <autonomyConditions/conditions/BEMSimDerate.h>
#include <autonomyConditions/conditions/BEMSimLevelWarning.h>



///////////////////////////////////////////////////////////////////////////////
/// @brief This is the main task class for PcmRemoteTask
///
/// PcmRemoteTask will launch a curses interface once task initialization
/// is successful.  Through this interface, the user can configure a scenario,
/// start and stop the system, and monitor each PCM process.
///////////////////////////////////////////////////////////////////////////////
class aisJhm2DataServer: public task::Task
{
  public:
    aisJhm2DataServer( const std::string& taskName );
    virtual ~aisJhm2DataServer( );

    virtual bool initialize( );
    virtual bool executive( );
    virtual void cleanup( );

  protected:
  private:
    aisJhm2DataServer(const aisJhm2DataServer&);  ///< Protect from accidental use
    void operator=(const aisJhm2DataServer&); ///< Protect from accidental use

    Jhm2IpcServer                     m_requestServer;
    AisJhm2RequestProcessor          m_requestProc;

};

#endif
