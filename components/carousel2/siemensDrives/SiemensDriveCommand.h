#ifndef SIEMENS_DRIVE_COMMAND
#define SIEMENS_DRIVE_COMMAND

// This is a unit converted version of UDPSendPacket,
// plus a couple fields for timestamps
struct SiemensDriveCommand{
	double winchSpeedSetpoint;
	double carouselSpeedSetpoint;

	double ts_trigger;
	double ts_elapsed;
};

#endif
