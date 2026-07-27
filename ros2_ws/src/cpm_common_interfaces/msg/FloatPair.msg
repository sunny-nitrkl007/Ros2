# Wire equivalent of std::pair<float, float>, used wherever the real SCS
# structs carry a std::vector<std::pair<float,float>> (ROS2 has no native
# pair type). First seen in LpsSaWeighReqstChannelStorage::arg.map
# (LpsSaWeighReqstChannel.h:73).

float32 first
float32 second
