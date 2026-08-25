///////////////////////////////////////////////////////////////////////////////
/// @file      StartupInhibit.h
/// @author    J Struble
/// @date      10/13/2011
/// @brief     InhibitCondition to implement a startup inhibit
///
/// @attention COPYRIGHT (C) 2011 CATERPILLAR INC. ALL RIGHTS RESERVED.
///////////////////////////////////////////////////////////////////////////////
#ifndef STARTUPINHIBIT_H_
#define STARTUPINHIBIT_H_

#include "ais/time/GetTime.h"
#include "ais/log/Logger.h"
#include "InhibitCondition.h"

class StartupInhibit: public InhibitCondition
{
  public:
    StartupInhibit( const std::string& name ): InhibitCondition( name ), m_startupInhibitTime_sec( 0.0 ) 
    {
      m_startupTime = localhostNow();
    }
    virtual ~StartupInhibit() {}
  
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Configure the Inhibit
    ///
    /// @param cs (in) - The configSection containing the config values
    /// @return true if the configuration was successful
    ///////////////////////////////////////////////////////////////////////////////  
    virtual bool configure( ConfigSection& cs )
    {
      if ( !cs.get( "StartupInhibitTime_sec", m_startupInhibitTime_sec ) )
      {
        AIS_LOG_FATAL( "StartupInhibitTime_sec configuration required for %s inhibit", m_name.c_str() );
        return false;                 
      }
      return true;            
    }
  
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Return whether the inhibit condition is active
    ///
    /// @return true if the inhibit is active
    ///////////////////////////////////////////////////////////////////////////////  
    virtual bool inhibitActive()
    {
      TimeStamp timeNow;
      timeNow = localhostNow();
      if ( ( timeNow - m_startupTime ).toDouble() > m_startupInhibitTime_sec )
        return false;
      
      return true;            
    }
  
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Return whether the inhibit condition is active
    ///
    /// @return true if the inhibit is active
    ///////////////////////////////////////////////////////////////////////////////
    double getTimeRemaining_sec() const
    {
      TimeStamp timeNow = localhostNow();
      return ( m_startupInhibitTime_sec - ( timeNow - m_startupTime ).toDouble() );
    }

  private:
    double              m_startupInhibitTime_sec;
    TimeStamp           m_startupTime;      
};


#endif /*STARTUPINHIBIT_H_*/
