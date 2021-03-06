#include "carouselSimulator.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include "steady_state_lookup_tables.h"

using namespace std;
using namespace RTT;
using namespace RTT::os;

typedef uint64_t TIME_TYPE;

CarouselSimulator::CarouselSimulator(std::string name):TaskContext(name,PreOperational) 
{
	addOperation("lookup_steady_state_speed", lookup_steady_state_speed, ClientThread)
		.doc("Given a line angle sensor elevation, look up the corresponding steady-state carousel speed.")
		.arg("elevation", "Line angle sensor elevation [Rad], usually a negative number.");

	addOperation("lookup_steady_state_elevation", lookup_steady_state_elevation, ClientThread)
		.doc("Given a carousel speed, look up the corresponding steady-state line angle sensor elevation.")
		.arg("speed", "carousel speed [Rad/s]");

//	addPort("driveState",portDriveState).doc("Siemens Drives Measurements");
//	addPort("lineAngles",portLineAngles).doc("Line Angle Sensor Measurements");
//	addPort("data",portData).doc("Resampled measurements from all sensors");

//	memset(&driveState, 0, sizeof( driveState ));
//	memset(&lineAngles, 0, sizeof( driveState ));
//	memset(&resampledMeasurements, 0, sizeof( resampledMeasurements ));
}

bool CarouselSimulator::configureHook()
{
	// The static arrays in these functions might not get initialized
	// until they are called the first time, so to be safe,
	// go ahead and call them...
	double speed = lookup_steady_state_speed(-0.964);
	if(isnan(speed))
	{
		log(Error) << "Runtime sanity check of lookup functions failed; speed is nan!" << endlog();
		return false;
	}
	if(abs(speed)>10)
	{
		log(Error) << "Runtime sanity check of lookup functions failed; speed is huge!" << endlog();
		cout << "Returned speed value was " << speed << endl;
		return false;
	}
	double elevation = lookup_steady_state_elevation(speed);
	if(isnan(elevation))
	{
		log(Error) << "Runtime sanity check of lookup functions failed; elevation is nan!" << endlog();
		cout << "Passed speed was " << speed << endl;
		return false;
	}
	if ( abs(elevation-(-0.964)) > .1 )
	{
		log(Error) << "Runtime sanity check of lookup functions failed; bad round-trip lookup accuracy!" << endlog();
		return false;
	}

	return true;
}

bool  CarouselSimulator::startHook()
{
	return true;
}

void  CarouselSimulator::updateHook()
{
	//TIME_TYPE trigger = TimeService::Instance()->getTicks();
	//portDriveState.read(driveState);

	//resampledMeasurements.ts_trigger = trigger;
	//resampledMeasurements.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	//portData.write(resampledMeasurements);

}

void  CarouselSimulator::stopHook()
{}

void  CarouselSimulator::cleanupHook()
{}

void  CarouselSimulator::errorHook()
{}

ORO_CREATE_COMPONENT( CarouselSimulator )
