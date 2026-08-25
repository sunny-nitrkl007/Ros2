/******************************Standalone********************************/
#include "../AutonomyConditionDiagnostics.h"
#include "../src_j1939/app_et_j1939_support.h"

void AutonomyConditionDiagnostics::ReadStoreSwitchTx(void)
{
	SwitchInputScs STG_Input;

	 while(LpsSaSwitchInput->get( STG_Input ))
		 {
			 if(STG_Input.get_STG_value(3) == STG::CLOSED)
			 {
				 PidF1AA = 0x01; /* Store Switch is depressed */
			 }
			 else
			 {
				 PidF1AA = 0x00; /* Store Switch is released  */
			 }
		 }
}
