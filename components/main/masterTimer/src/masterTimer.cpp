#include "masterTimer.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::MasterTimer)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;

namespace OCL
{
 MasterTimer::MasterTimer(std::string name)
	 : TaskContext(name)
{
	// Add properties
	properties()->addProperty( "imu_target_hz",_imu_target_hz )
		.doc("The target measurement rate of the imu (Hz)");
	properties()->addProperty( "camera_target_hz",_camera_target_hz )
		.doc("The target framerate of the camera (Hz)");

	// Add ports
	ports()->addPort( "cameraClock",_cameraClock ).doc("The description of the port");
	ports()->addPort( "imuClock",_imuClock ).doc("The description of the port");
	addPort("imuCameraRatio" , _imuCameraRatio).doc("The ratio of imu freq. on camera freq.");

	addPort("deltaIn",_deltaIn);
	addPort("deltaOut",_deltaOut);

	base_clock_index = 0;
}

MasterTimer::~MasterTimer()
{
}

bool  MasterTimer::configureHook()
{
	assert(this->getActivity()->isPeriodic());
	double myperiod = this->getActivity()->getPeriod();

	portPointers[0] = &_imuClock;
	portPointers[1] = &_cameraClock;

	for(int i=0; i<CLOCK_COUNT; i++)
	{
		double target_period = 1.0 / target_hz[i];

		double temp_divider = target_period/myperiod;
		double thresh = fabs((temp_divider - round(temp_divider))/temp_divider);
		if( thresh < 1.0e-6)
		{
			// The divider was probably intended to be the integer.
			dividers[i] = round(temp_divider);
		} else {
			// The divider needs to be rounded up.
			dividers[i] = ceil(temp_divider);
		}
		cout << "dividers[" << i << "] = " << dividers[i] << endl;
	}
	// Write the ratio of IMU measurements on camera measurements on a port
	_imuCameraRatio.write(dividers[1]/dividers[0]);
	return true;
}

bool  MasterTimer::startHook()
{
	return true;
}

void  MasterTimer::updateHook()
{
	TIME_TYPE myticks = RTT::os::TimeService::Instance()->getTicks(); // Get current time

	for(int i=0; i<CLOCK_COUNT; i++)
	{
		if(base_clock_index % dividers[i]==0)
		{
			// Trigger event port i
			if(i==1){//camera clock
				_deltaIn.read(delta);
				_deltaOut.write(delta);
			}
			portPointers[i]->write(myticks);
		}
	}
	_imuCameraRatio.write(dividers[1]/dividers[0]);
	base_clock_index++;
}

void  MasterTimer::stopHook()
{
}

void  MasterTimer::cleanUpHook()
{
}

}//namespace
