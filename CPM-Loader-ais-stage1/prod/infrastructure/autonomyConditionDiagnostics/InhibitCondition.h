///////////////////////////////////////////////////////////////////////////////
/// @file      InhibitCondition.h
/// @author    J Struble
/// @date      10/13/2011
/// @brief     Base type for an Inhibit Condition
///
/// @attention COPYRIGHT (C) 2011 CATERPILLAR INC. ALL RIGHTS RESERVED.
///////////////////////////////////////////////////////////////////////////////
#ifndef INHIBITCONDITION_H_
#define INHIBITCONDITION_H_

#include <string>
#include <ais/config/ConfigSection.h>

class InhibitCondition
{
  public:
    InhibitCondition( const std::string& name ): m_name( name ) {}
    virtual ~InhibitCondition() {}
    
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Configure the Inhibit
    ///
    /// @param cs (in) - The configSection containing the config values
    /// @return true if the configuration was successful
    ///////////////////////////////////////////////////////////////////////////////    
    virtual bool configure( ConfigSection& cs ) { return true; }
    
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Return whether the inhibit condition is active
    ///
    /// @return true if the inhibit is active
    ///////////////////////////////////////////////////////////////////////////////     
    virtual bool inhibitActive()=0;
    
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Get the unique name of this inhibit condition
    ///
    /// @return true if the inhibit is active
    ///////////////////////////////////////////////////////////////////////////////         
    virtual std::string name() { return m_name; }
    
  protected:
    std::string         m_name;
  
};

#endif /*INHIBITCONDITION_H_*/
