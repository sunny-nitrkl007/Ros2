# Wire equivalent of SimpleCalData_t.
# Source: CPM-Loader-ais/prod/common/interfaces/LpsSaJobMgrTxChannel/LpsSaJobMgrTxChannel.h:67-80
#   typedef struct {
#       std::string  timeStamp; // contains sec,min,hrs,mm,dd,yy
#       float truckWt;
#       float zeroedTruckWt;
#   } SimpleCalData_t;
# Carried as LpsSaJobMgrTxChannel.simple_cal_data[] -- was std::deque<SimpleCalData_t>.

string time_stamp
float32 truck_wt
float32 zeroed_truck_wt
