///////////////////////////////////////////////////////////////////////////////
// @file    DiagOutFileWriter.cpp
// @author  J Struble
// @date    9/27/2011
//
// @attention COPYRIGHT (C) 2011 CATERPILLAR INC. ALL RIGHTS RESERVED.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// -- #Include's --
////////////////////////////////////////////////////////////////////////////////
#include "ais/time/GetTime.h"
#include "ais/time/ConvertTime.h"
#include "DiagOutFileWriter.h"
#include <iomanip>

////////////////////////////////////////////////////////////////////////////////
// -- #Define, Struct's, Typedef's, Enum's --
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// -- Function Prototypes --
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// -- Symbols --
////////////////////////////////////////////////////////////////////////////////
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// -- Start of code for this file --
////////////////////////////////////////////////////////////////////////////////
DiagOutFileWriter::DiagOutFileWriter() 
{

}

DiagOutFileWriter::~DiagOutFileWriter() 
{
  m_outfile.close();
}

bool DiagOutFileWriter::configure( ConfigSection& cs )
{
  string outFileLoc;
  if ( !cs.get("DiagnosticOutputFile", outFileLoc) )
  {
    AIS_LOG_FATAL("DiagnosticOutputFile not found in config file");
    return false;
  }

  string path(getenv("HOME"));
  outFileLoc.insert (0, 1, '/');
  outFileLoc.insert(0, path);

  m_outfile.open( outFileLoc.c_str(), ios_base::out );
  
  if ( !m_outfile.good() )
  {
    AIS_LOG_FATAL("Error opening %s for writing.", outFileLoc.c_str());
    return false;
  }
  
  return true;      
}
  

bool DiagOutFileWriter::log( const AutoDiagEvent& event, const nrec::geometry::ISOPose3D<double>& pose, const double& engHours )
{
  if ( !m_outfile.good() )
  {
    m_logger.log_error("Outfile write error");
    return false;
  }

  m_outfile << endl;
  for (unsigned int i = 0; i < 120; ++i)
    m_outfile << "=";
    
  m_outfile << endl;      
  m_outfile.setf(ios::fixed,ios::floatfield);   // floatfield set to fixed
  m_outfile << event.getId() << " \t";
  m_outfile << event.getDesc() << "  ";
  if ( event.m_instanceNameIncl )
    m_outfile << event.m_instanceName << "  ";
  std::string gpsTimeString = commonNow().getTimevalString();
  gpsTimeString = gpsTimeString.substr(0,gpsTimeString.size()-3);
  m_outfile << "GPS: " << gpsTimeString << "  ";
  std::string utcTimeString = convertGPSToUTC(commonNow()).getTimevalString();
  utcTimeString = utcTimeString.substr(0,utcTimeString.size()-3);
  m_outfile << "UTC: " << utcTimeString << "  ";
  m_outfile << "GMT: "<< convertGPSToGMTString(commonNow()) << "   ";
  m_outfile << setprecision (2) << pose.x() << "  " << pose.y() << "  ";
  m_outfile << setprecision (2) << engHours << endl;
  m_outfile << endl;
  m_outfile << event.getConditionString();
  m_outfile << endl;
  
  for (unsigned int i = 0; i < 120; ++i)
    m_outfile << "=";

  m_outfile << endl;
  m_outfile.flush();
  return m_outfile.good();
}

