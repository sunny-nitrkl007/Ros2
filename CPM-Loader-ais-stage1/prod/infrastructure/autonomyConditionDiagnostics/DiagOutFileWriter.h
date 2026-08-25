///////////////////////////////////////////////////////////////////////////////
/// @file      AutoDiagEvent.h
/// @author    J Struble
/// @date      9/27/2011
/// @brief     Utility object to write to the Autonomy Events/Diagnostics Logfile
///
/// @attention COPYRIGHT (C) 2011 CATERPILLAR INC. ALL RIGHTS RESERVED.
///////////////////////////////////////////////////////////////////////////////
#ifndef DIAGOUTFILEWRITER_H_
#define DIAGOUTFILEWRITER_H_

#include <ais/config/ConfigSection.h>
#include <geometry/poses/ISOPose3D.h>
#include <ais/log/Logger.h>
#include <iostream>
#include <fstream>
#include "AutoDiagEvent.h"

class DiagOutFileWriter
{
  public:
    DiagOutFileWriter();
    ~DiagOutFileWriter(); 

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Configure the writer
    ///
    /// @param cs (in) - The configSection containing the config values
    /// @return true if the configuration was successful
    ///////////////////////////////////////////////////////////////////////////////    
    bool configure( ConfigSection& cs );
    
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Log an event/diagnostic to the logfile
    ///
    /// @param event (in) - The diag/event to log
    /// @param pose (in) - Current vehicle pose
    /// @param engHours (in) - Current engine hours    
    /// @return true if write was successful
    ///////////////////////////////////////////////////////////////////////////////       
    bool log( const AutoDiagEvent& event, const nrec::geometry::ISOPose3D<double>& pose, const double& engHours );
    
  private:
    std::ofstream     m_outfile;
    logger::Logger    m_logger;
  
};

#endif /*DIAGOUTFILEWRITER_H_*/
