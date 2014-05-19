// Description: MHE based on dynamic (DAE) kite model
// Author:      Milan Vukov, milan.vukov@esat.kuleuven.be
// Date:        August 2013.

#ifndef DYNAMIC_MHE_HPP
#define DYNAMIC_MHE_HPP

#include <rtt/TaskContext.hpp>
#include <rtt/Port.hpp>

/// Common header for the autogenerated code
#include "autogenerated/acado_common.h"
/// Include file with MHE configuration
#include "autogenerated/mhe_configuration.h"

#if ACADO_QP_SOLVER != ACADO_QPOASES
#error "The current impementation supports qpOASES based OCP solver only!"
#endif

#if mhe_num_markers != 12
#error "Number of vision system markers must be 12"
#endif

//
// Define some common stuff
//
#define NX		ACADO_NX	// # differential variables
#define NXA		ACADO_NXA	// # algebraic variables
#define NU		ACADO_NU	// # control variables
#define NP		ACADO_NP	// # user parameters
#define NY		ACADO_NY	// # measurements, nodes 0.. N - 1
#define NYN		ACADO_NYN	// # measurements, last node
#define N 		ACADO_N		// # estimation intervals

typedef uint64_t TIME_TYPE;

// Here we import custom data type definitions
#include "mcuHandler/types/McuHandlerDataType.hpp"
#include "encoder/types/EncoderDataType.hpp"
#include "LEDTracker/types/LEDTrackerDataType.hpp"
#include "lineAngleSensor/types/LineAngleSensorDataType.hpp"
#include "winchControl/types/WinchControlDataType.hpp"

#include "types/DynamicMheDataTypes.hpp"

/// Dynamic MHE class
class DynamicMhe
	: public RTT::TaskContext
{
public:
	/// Ctor
	DynamicMhe(std::string name);
	/// Dtor
	virtual ~DynamicMhe()
	{}
	
	/// Configuration hook.
	virtual bool configureHook( );
	/// Start hook.
	virtual bool startHook( );
	/// Update hook.
	virtual void updateHook( );
	/// Stop hook.
	virtual void stopHook( );
	/// Cleanup hook.
	virtual void cleanupHook( );
	/// Error hook.
	virtual void errorHook( );
	/// Exception hook.
	virtual void exceptionHook();

protected:
	//
	// Input ports
	//

	/// Input trigger port
	RTT::InputPort< TIME_TYPE > portTrigger;
	/// MCU handler data inputs; buffered port
	RTT::InputPort< McuHandlerDataType > portMcuHandlerData;
	/// IMU data holder
	std::vector< McuHandlerDataType > imuData;
	/// Encoder data input
	RTT::InputPort< EncoderDataType > portEncoderData;
	/// Encoder data holder
	EncoderDataType encData;
	/// Camera data input
	RTT::InputPort< LEDTrackerDataType > portLEDTrackerData;
	/// Camera data holder
	LEDTrackerDataType camData;
	/// LAS data input
	RTT::InputPort< LineAngleSensorDataType > portLASData;
	/// LAS data holder
	std::vector< LineAngleSensorDataType > lasData;
	/// Winch data input
	RTT::InputPort< WinchControlDataType > portWinchData;
	/// Winch data holder
	WinchControlDataType winchData;

	//
	// Output ports
	//

	/// Port for the current state estimate
	RTT::OutputPort< DynamicMheStateEstimate > portStateEstimate;
	/// Current state estimate data holder
	DynamicMheStateEstimate stateEstimate;

	/// Output data for debug
	RTT::OutputPort< DynamicMheHorizon > portDebugData;
	/// Debug data port holder
	DynamicMheHorizon debugData;

	//
	// Properties
	//
	
private:
	bool prepareInitialGuess( void );
	bool prepareWeights( void );
	bool prepareMeasurements( void );
	bool prepareDebugData( void );

	double mheWeights[ NY ];
	double ledData[ mhe_num_markers ];
	double ledWeights[ mhe_num_markers ];

	double cableLength, cableLengthWeight;

	unsigned runCnt;
	double execY[ NY ], execYN[ NYN ];

	int mheStatus, errorCode;
	unsigned numOfFailures;
};

#endif // DYNAMIC_MHE_HPP
