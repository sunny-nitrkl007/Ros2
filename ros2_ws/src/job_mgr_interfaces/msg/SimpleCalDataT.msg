# Wire equivalent of AisJhm2TxChannelStorage::simplecal_data_t.
# Source: AisJhm2TxChannel.h:14-19
# Named SimpleCalDataT (not SimpleCalData) to avoid confusion with the
# unrelated SimpleCalData.msg used by LpsSaJobMgrTxChannel.msg -- different
# real C++ struct, coincidentally similar name.

string time_stamp
float32 adjtruckweight
float32 zeroed_truck_wt
bool new_data_flag
