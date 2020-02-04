#include "LidarAngleChanger.hpp"

namespace isaac {
namespace ev3 {

void LidarAngleChanger::start() {
  tickOnMessage(rx_scan());
}

void LidarAngleChanger::stop() {
}

void LidarAngleChanger::tick() {

    auto lidar_proto = rx_scan().getProto();    
    auto lidar_return_proto = tx_flatscan().initProto();
    
    lidar_return_proto.setRanges(lidar_proto.getRanges());
    lidar_return_proto.setInvalidRangeThreshold(lidar_proto.getInvalidRangeThreshold());
    lidar_return_proto.setOutOfRangeThreshold(lidar_proto.getOutOfRangeThreshold());
    if(lidar_proto.getVisibilities().size()>0){
      lidar_return_proto.setVisibilities(lidar_proto.getVisibilities());
    }
    
    auto angles = lidar_proto.getAngles();
    auto changed_angles = lidar_return_proto.initAngles(angles.size());
    
    for(uint i = 0; i<angles.size();i++) {
        changed_angles.set(i,-angles[i]);
    }    


    tx_flatscan().publish();
}

}  // namespace ev3
}  // namespace isaac