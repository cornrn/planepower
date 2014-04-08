// Description: NMPC based on dynamic (DAE) kite model
// Author:      Milan Vukov, milan.vukov@esat.kuleuven.be
// Date:        August 2013.

#ifndef DYNAMIC_MPC_HPP
#define DYNAMIC_MPC_HPP

#include <rtt/TaskContext.hpp>
#include <rtt/Port.hpp>

/// Common header for the autogenerated code
#include "autogenerated/acado_common.h"
/// Include auto-generated configuration file
#include "autogenerated/mpc_configuration.h"

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

// Include MHE data types
#include "DynamicMhe/types/DynamicMheDataTypes.hpp"
// Include MCU handler data type for ctrl surfaces' typekit
#include "mcuHandler/types/McuHandlerDataType.hpp"
// Include NMPC data types
#include "types/DynamicMpcDataTypes.hpp"

class DynamicMpc
	: public RTT::TaskContext
{
public:
	/// Ctor
	DynamicMpc(std::string name);
	/// Dtor
	virtual ~DynamicMpc()
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

protected:

	//
	// Input ports
	//

	/// Current state estimate
	RTT::InputPort< DynamicMheStateEstimate > portFeedback;
	/// State estimate data holder
	DynamicMheStateEstimate feedback;

	// Current controls
	RTT::InputPort< McuHandlerDataType > portCurrentControls;
	/// control data holder
	McuHandlerDataType currentControls;

	//
	// Output ports
	//
	
	/// Output port with control signals
	RTT::OutputPort< ControlSurfacesValues > portControls;
	/// control data holder
	ControlSurfacesValues controls;

	/// Debug port
	RTT::OutputPort< DynamicMpcHorizon > portDebugData;
	/// Debug data holder
	DynamicMpcHorizon debugData;
	
	//
	// Properties
	//

	/// # SQP iterations
	unsigned numSqpIterations;

private:

	bool prepareInitialGuess( void );
	bool prepareWeights( void );
	bool prepareReference( void );
	bool prepareDebugData( void );

	unsigned runCnt;
	int mpcStatus, errorCode;
//	double execY[ NY ], execYN[ NYN ];
};

#endif // DYNAMIC_MPC_HPP
