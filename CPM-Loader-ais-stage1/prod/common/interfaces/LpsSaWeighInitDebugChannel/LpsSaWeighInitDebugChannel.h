#include <ais/serialization/Datum.h> // For Boost serialization
#include <ais/interfaces/traits/csvable.h>

#ifndef __LPS_COMMON_TYPE_DEF_H__
#include <LpsCommonTypeDef.h>
#endif

#ifndef __CPM_FILTER_H__
#include <cpm_filter.h>
#endif

#ifndef __LPS_COMMON_STRUCTURES_H__
#include <LpsCommonStructures.h>
#endif

#ifndef __LPS_PUBLIC_H__
#include <LpsPublic.h>
#endif

#ifndef __LPS_SA_INCLUDES_H__
#include <LpsSaWeighApp/LpsSaIncludes.h>
#endif

#ifndef _LpsSaWeighInitDebugChannel_h_
#define _LpsSaWeighInitDebugChannel_h_
class LpsSaWeighInitDebugChannelStorage: public csvable
{  
public:  
     LpsSaWeighInitDebugChannelStorage()  /* Initialize Fields Here */
     {
     }

     template <class Archive>
     void serialize(Archive &ar, unsigned int version)
     {
       /* Archive Fields Here */
    	ar & m_WeighInitTbl.ExecRate;
    	ar & m_WeighInitTbl.MachSpecificCfg.CalibTbl.CalWeight;
    	ar & m_WeighInitTbl.MachSpecificCfg.CalibTbl.CalStatus;
    	ar & m_WeighInitTbl.MachSpecificCfg.CalibTbl.CalAdjust;
    	ar & m_WeighInitTbl.MachSpecificCfg.CalibTbl.ZeroWeight;
    	ar & m_WeighInitTbl.MachSpecificCfg.DigConfig.DigTargetWt;
    	ar & m_WeighInitTbl.MachSpecificCfg.StartOfWeigh;
    	ar & m_WeighInitTbl.MachSpecificCfg.TiltComp.TiltCompGainScalar;
     }

     void toCsv(CsvOutStream& out) const
     {
    	 //out("Minus One JobMgr Response Code", MinusOneResp);
     }

     LpsInitTbl_t m_WeighInitTbl;

 private:  
     /* Add Fields Here */

};  

typedef Datum<LpsSaWeighInitDebugChannelStorage> LpsSaWeighInitDebugChannel;

BOOST_CLASS_VERSION(LpsSaWeighInitDebugChannelStorage, 1);
#endif

