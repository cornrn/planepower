#ifndef __CONTROLLERTEMPLATE__
#define __CONTROLLERTEMPLATE__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

#include "ResampledMeasurements.h"
#include "SiemensDriveCommand.h"

#include "Reference.h"
#include "LQRControllerDebug.h"

typedef uint64_t TIME_TYPE;

class LqrController : public RTT::TaskContext
{
public:
	LqrController(std::string name);
	virtual ~LqrController(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

protected:
	RTT::InputPort< ResampledMeasurements > portResampledMeasurements;
	RTT::InputPort< LQRGains > portLQRControllerGains;
	RTT::InputPort< Reference > portReference;
	RTT::OutputPort< SiemensDriveCommand > portDriveCommand;
	RTT::OutputPort< LQRGains > portGainsOut;
	RTT::OutputPort< LQRControllerDebug > portDebug;
	bool freezeFeedForwardTerm;

private:
	ResampledMeasurements resampledMeasurements;
	SiemensDriveCommand driveCommand;
	LQRGains gains;
	Reference reference;
	double error;
	double ierror;
	double derror;
	double lastElevation;

	double referenceElevation;
	double feedForwardTermAsAngle;
	double feedForwardTermAsSpeed;
	bool feedForwardTermHasBeenSet;

	TIME_TYPE trigger_last;	
	TIME_TYPE trigger;
	double derivativeLowpassFilterState;
	bool trigger_last_is_valid;
	bool derivativeFilterReady;

	RTT::OperationCaller< double(double) > lookup_steady_state_speed;
	RTT::OperationCaller< double(double) > lookup_steady_state_elevation;

	LQRControllerDebug debug;

};

#endif
