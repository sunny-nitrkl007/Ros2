///////////////////////////////////////////////////////////////////////////////
// @file    InhibitConditionList.cpp
// @author  J Struble
// @date    10/13/2011
//
// @attention COPYRIGHT (C) 2011 CATERPILLAR INC. ALL RIGHTS RESERVED.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// -- #Include's --
////////////////////////////////////////////////////////////////////////////////
#include "InhibitConditionList.h"
#include "InhibitConditionBuilder.h"
#include <ais/config/ConfigSection.h>
#include <ais/log/Logger.h>

////////////////////////////////////////////////////////////////////////////////
// -- #Define, Struct's, Typedef's, Enum's --
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// -- Function Prototypes --
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// -- Symbols --
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// -- Start of code for this file --
////////////////////////////////////////////////////////////////////////////////
InhibitConditionList::InhibitConditionList()
{

}

InhibitConditionList::~InhibitConditionList() 
{

}

bool InhibitConditionList::configure( TaskParser& tp, AutonomyConditionsList& condList )
{
  ConfigSection cs;
  if ( !tp.getSection( "InhibitConditionConfig", cs ) )
  {
    AIS_LOG_FATAL( "InhibitConditionConfig configSection not found in .rb file" );
    return false;
  }
  
  std::vector<std::string> configuredInhibits;
  if ( !cs.getArray( "ConfiguredInhibits", configuredInhibits ) )
  {
    AIS_LOG_FATAL( "ConfiguredInhibits not found" );
    return false;
  }
  
  for ( unsigned int i = 0; i < configuredInhibits.size(); ++i )
  {
    ConfigSection inhibSection;
    if ( !tp.getSection( configuredInhibits.at(i), inhibSection ) )
    {
      AIS_LOG_FATAL( "%s configSection not found", configuredInhibits.at(i).c_str() );
      return false;
    }
    
    unsigned int inhibType;
    if (!inhibSection.get( "InhibitType", inhibType ))
    {
      AIS_LOG_FATAL( "InhibitType not found in %s configSection", configuredInhibits.at(i).c_str() );
      return false;          
    }
    
    InhibitCondition* p_newCondition = InhibitConditionBuilder::get( inhibType, configuredInhibits.at(i), condList );
    if ( p_newCondition == NULL )
    {
      AIS_LOG_FATAL( "InhibitType %d in %s configSection is invalid", configuredInhibits.at(i).c_str() );
      return false;                    
    }
    
    if ( !p_newCondition->configure( inhibSection ) )
      return false;

    if ( !addCondition( p_newCondition ) )
    {
      AIS_LOG_FATAL( "Error adding InhibitCondition, possible duplicate." );
      return false;
    }          
  }
  
  return true;      
}

InhibitCondition* InhibitConditionList::get( const std::string& name ) const
{
  for( unsigned int i = 0; i < size(); ++i )
  {
    if ( at(i)->name() == name )
      return at(i);
  }
  return NULL;      
}

bool InhibitConditionList::addCondition( InhibitCondition* newCondition )
{
  if ( newCondition == NULL )
  {
    AIS_LOG_ERROR( "Condition is NULL, can't add." );
    return false;
  }
    
  //First check for duplictes
  for ( unsigned int i = 0; i < size(); ++i )
  {
    if ( at(i)->name() == newCondition->name() )
    {
      AIS_LOG_FATAL( "Duplicate found, can't add." );
      return false;
    }
  }
  
  push_back( newCondition );
  return true;
}

bool InhibitConditionList::startupInhibitActive( double& timeRemaining_sec )
{
  bool startupInhActive = false;
  timeRemaining_sec = 0;
  for ( unsigned int i = 0; i < size(); ++i )
  {
    StartupInhibit* startupInh = dynamic_cast<StartupInhibit*>( at(i) );
    //if this condition is a startup inhibit condition
    if ( startupInh )
    {
      //And it's active
      if ( startupInh->inhibitActive() )
      {
        startupInhActive = true;
        double timeLeft = startupInh->getTimeRemaining_sec();
        if ( timeLeft > timeRemaining_sec )
          timeRemaining_sec = timeLeft;
      }
    }          
  }
  
  return startupInhActive;
}
