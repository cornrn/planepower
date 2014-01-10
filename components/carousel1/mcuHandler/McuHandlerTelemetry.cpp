#include "McuHandlerTelemetry.hpp"

#include <rtt/Component.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;
using namespace McuHandlerProto;

McuHandlerTelemetry::McuHandlerTelemetry(std::string name)
	: RTT::TaskContext(name, PreOperational),
	  zContext( NULL ), zSocket( NULL )
{
	//
	// Add ports
	//
	addEventPort("msgData", portMsgData)
		.doc("Message data");

	port = "tcp://*:5563";
}

bool McuHandlerTelemetry::configureHook()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	zContext = new zmq::context_t( 1 );

	return true;
}

bool McuHandlerTelemetry::startHook()
{
	zSocket = new zmq::socket_t(*zContext, ZMQ_PUB);
	zSocket->bind( port.c_str() );

	return true;
}

void McuHandlerTelemetry::updateHook()
{
	if (portMsgData.read( msgData ) == NewData)
	{
		msg.set_gyro_x( msgData.gyro_x );
		msg.set_gyro_y( msgData.gyro_y );
		msg.set_gyro_z( msgData.gyro_z );

		msg.set_accl_x( msgData.accl_x );
		msg.set_accl_y( msgData.accl_y );
		msg.set_accl_z( msgData.accl_z );

		msg.set_ua1( msgData.ua1 );
		msg.set_ua2( msgData.ua2 );
		msg.set_ue( msgData.ue );

		msg.set_ts_trigger( msgData.ts_trigger );
		msg.set_ts_elapsed( msgData.ts_elapsed );

		msg.SerializeToString( &raw );

		if (s_send(*zSocket, raw) == false)
			exception();

		// NOTE At test run we need to determine byte size of the message
		// NOTE Type of the zmqMsg will be zmq::message_t, ctor argument is the size
		// NOTE SerializeToArray requires that all protobuf msg fields are set!
		// msg.SerializeToArray(static_cast<void*>zmqMsg.data(), msg.ByteSize());
	}
}

void McuHandlerTelemetry::stopHook()
{
	if (zSocket != NULL)
		delete zSocket;
}

void McuHandlerTelemetry::cleanupHook()
{
	// Optional, to be tested
	google::protobuf::ShutdownProtobufLibrary();

	if (zContext != NULL)
		delete zContext;
}

void McuHandlerTelemetry::errorHook()
{}

ORO_CREATE_COMPONENT_LIBRARY()
ORO_LIST_COMPONENT_TYPE( McuHandlerTelemetry )
//ORO_CREATE_COMPONENT( McuHandlerTelemetry )