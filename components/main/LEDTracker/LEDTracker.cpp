#include "LEDTracker.hpp"
#include <ocl/Component.hpp>
#include <rtt/marsh/Marshalling.hpp>

#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include "types.hpp"
#include "cout.hpp"

#define VERBOSE 0

ORO_CREATE_COMPONENT( LEDTracker)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;

LEDTracker::LEDTracker(std::string name) : TaskContext(name, PreOperational)
{
	attributes()->addAttribute( "useExternalTrigger", _useExternalTrigger);
	_useExternalTrigger.set(false); // Default Value

	addPort("timeStamps", portTimeStamps)
		.doc("Time stamps: [trigger frameArrival compDone exit]");

	addPort( "markerPositions",_markerPositions ).doc("Pixel Locations of the markers");
	addPort( "markerPositionsAndCovariance",_markerPositionsAndCovariance ).doc("Pixel locatoins and weights");
	addEventPort("triggerTimeStampIn",_triggerTimeStampIn);
	addPort("triggerTimeStampOut",_triggerTimeStampOut);

	addPort("compTime",_compTime);

	addPort("frameArrivalTimeStamp",_frameArrivalTimeStamp);
	addPort("computationCompleteTimeStamp",_computationCompleteTimeStamp);

	addPort("deltaIn",_deltaIn);
	addPort("deltaOut",_deltaOut);

	markerPositions.resize(CAMERA_COUNT*LED_COUNT*2,0.0);
	markerPositionsAndCovariance.resize(CAMERA_COUNT*LED_COUNT*2*2,0.0);

	addProperty( "sigma_marker",sigma_marker).doc("The standard deviation of the camera measurements. Default = 1e3");
	sigma_marker = 1e3;

	tempTime = RTT::os::TimeService::Instance()->getTicks(); // Get current time

	timeStamps.resize(4, 0.0);
	portTimeStamps.setDataSample( timeStamps );

	_markerPositions.setDataSample( markerPositions );
	_markerPositions.write( markerPositions );
	_markerPositionsAndCovariance.setDataSample( markerPositionsAndCovariance );
	_markerPositionsAndCovariance.write( markerPositionsAndCovariance );
	_triggerTimeStampOut.setDataSample( tempTime );
	_triggerTimeStampOut.write( tempTime );
	_compTime.setDataSample( tempTime );
	_compTime.write( tempTime );
	_frameArrivalTimeStamp.setDataSample( tempTime );
	_frameArrivalTimeStamp.write( tempTime );
	_computationCompleteTimeStamp.setDataSample( tempTime );
	_computationCompleteTimeStamp.write( tempTime );
	_deltaOut.setDataSample( 0.0 );
	_deltaOut.write( 0.0 );
}

LEDTracker::~LEDTracker()
{
}

bool  LEDTracker::configureHook()
{
	cameraArray = new CameraArray(_useExternalTrigger.get());
	frame_w = cameraArray->frame_w;
	frame_h = cameraArray->frame_h;
	for(int i=0; i<CAMERA_COUNT; i++)
	{
		blobExtractors[i] = new BlobExtractor(frame_w, frame_h, NEED_TO_DEBAYER);
	}
	return true;
}

bool  LEDTracker::startHook()
{
	cameraArray->startHook();
	return true;
}

void  LEDTracker::updateHook()
{
	// At entry we should have just been woken up by the same event port
	// that woke up the cameraTrigger.

	// Read the timestamp from the eventPort
	if(_triggerTimeStampIn.read(triggerTimeStamp) != NewData) return 0; 

	// This blocks until a frame arrives from all cameras
	cameraArray->updateHook();

	// The timestamp the camera was triggered for the current frame.
	//while(_triggerTimeStampIn.read(triggerTimeStamp) == NewData); 
//	_triggerTimeStampIn.read(triggerTimeStamp);

	_deltaIn.read(delta);

	frameArrivalTimeStamp = RTT::os::TimeService::Instance()->getTicks(); // Get current time 
	_frameArrivalTimeStamp.write(frameArrivalTimeStamp);
#if VERBOSE
	double transferTime = (frameArrivalTimeStamp-triggerTimeStamp)*1e-9; // sec
	COUT << "Transfer time was: " << transferTime*1e3 << "ms" << ENDL;
#endif

	tempTime = RTT::os::TimeService::Instance()->getTicks(); // Refresh timestamp, in case PRINTF took time.
	// Note, this ~could trivially be done in parallel!
	for(int i=0; i<CAMERA_COUNT; i++)
	{
		blobExtractors[i] -> find_leds(cameraArray->current_frame_data[i]);
	}
	computationCompleteTimeStamp = RTT::os::TimeService::Instance()->getTicks();
	//double computationTime = (computationCompleteTimeStamp - triggerTimeStamp)*1e-9; // sec
	double computationTime = (computationCompleteTimeStamp - frameArrivalTimeStamp)*1e-9; // sec
#if VERBOSE
	COUT << "Total computation time was: " << computationTime*1.0e3 << "ms" << ENDL;
#endif
	_compTime.write(computationTime*1.0e3);

	// Copy marker location data from the extractors into our staging area.
	for(int i=0; i<CAMERA_COUNT; i++)
	{
		MarkerLocations * src = &(blobExtractors[i]->markerLocations);	
		MarkerLocations * dst = ((MarkerLocations*) &(markerPositions[0])) + i;
		*dst = *src;
	}

	// Rescale the pixel data.
	// Note!!!! whatever you do, make sure this is appropriate for the
	// camera calibration files you're using, or your camera measurement
	// function will be broken!!!
	if(cameraArray->frame_w == 800)
	{
		for(unsigned int i=0; i<CAMERA_COUNT*LED_COUNT*2; i++)
		{
			if( ! isnan(markerPositions[i])){ 
				markerPositions[i] *= 2; // Match old camera resolution until we redo geometric calibration
			} 
		}
	}
	
	// If a Marker was not detected properly,
	// put an arbitrary value and set the weight to 0
	// otherwise, set the weight properly
	for(unsigned int i=0; i<CAMERA_COUNT*LED_COUNT*2; i++)
	{
		if(isnan(markerPositions[i])){ 
			markerPositionsAndCovariance[i] = MARKER_NOT_DETECTED_VALUE;
			markerPositionsAndCovariance[i+CAMERA_COUNT*LED_COUNT*2] = 0.0;
		}
		else{
			markerPositionsAndCovariance[i] = markerPositions[i];
			markerPositionsAndCovariance[i+CAMERA_COUNT*LED_COUNT*2] = 1.0/(sigma_marker*sigma_marker);
		}
	}

	_triggerTimeStampOut.write(triggerTimeStamp);
	_deltaOut.write(delta);

	_markerPositions.write(markerPositions);
	_markerPositionsAndCovariance.write(markerPositionsAndCovariance);

	timeStamps[ 0 ] = (double) triggerTimeStamp;
	timeStamps[ 1 ] = (double) frameArrivalTimeStamp;
	timeStamps[ 2 ] = (double) computationCompleteTimeStamp;
	timeStamps[ 3 ] = (double) RTT::os::TimeService::Instance()->getTicks();

	portTimeStamps.write( timeStamps );

	// Tell orocos to re-trigger this component immediately after
	// it is done updating output ports, so that it can start waiting
	// for the next frame arrival
	//this->getActivity()->trigger();
}

void  LEDTracker::stopHook()
{
	for(unsigned int i=0; i<CAMERA_COUNT*LED_COUNT*2; i++)
	{
		markerPositionsAndCovariance[i] = MARKER_NOT_DETECTED_VALUE;
		markerPositionsAndCovariance[i+CAMERA_COUNT*LED_COUNT*2] = 0.0; 
	}
	_markerPositionsAndCovariance.write(markerPositionsAndCovariance);

	cameraArray->stopHook();
}

void  LEDTracker::cleanUpHook()
{
	cameraArray->cleanUpHook();
	delete cameraArray;
	for(int i=0; i<CAMERA_COUNT; i++)
	{
		delete blobExtractors[i];
	}
}


