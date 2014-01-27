/// Base class
#include "TelemetryBase.hpp"
/// Custom data type used by OROCOS
#include "types/LineAngleSensorDataType.hpp"
/// Protobuf header, autogenerated
#include "LineAngleSensorTelemetry.pb.h"

class LineAngleSensorTelemetry
  : public TelemetryBase<LineAngleSensorDataType, LineAngleSensorProto::LineAngleSensorMsg>
{
  typedef TelemetryBase<LineAngleSensorDataType, LineAngleSensorProto::LineAngleSensorMsg> Base;

public:
  LineAngleSensorTelemetry(std::string name)
    : Base( name )
  {}

  ~LineAngleSensorTelemetry()
  {}

protected:

  virtual void fill();

};

void LineAngleSensorTelemetry::fill()
{
  msg.set_angle_ver( data.angle_ver );
  msg.set_angle_hor( data.angle_hor );

  msg.set_ts_trigger(data.ts_trigger * 1e-9);
  msg.set_ts_elapsed( data.ts_elapsed );
}

ORO_CREATE_COMPONENT_LIBRARY()
ORO_LIST_COMPONENT_TYPE( LineAngleSensorTelemetry )
