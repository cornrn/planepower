#!/usr/bin/env rttlua-i
require "deployment_helpers"
for i,symbol in ipairs({"load_component",
						"load_reporter",
						"load_properties",
						"get_property",
						"set_property",
						"sleep"}) do
	_G[symbol] = deployment_helpers[symbol]
end

PLANEPOWER="../"
PROPERTIES=PLANEPOWER.."properties/"

soemPrio=99
masterTimerPrio = 98
sensorPrio = 97
LEDTrackerPrio = 70
reporterPrio = 50

load_component("masterTimer","MasterTimer","masterTimer")
load_properties("masterTimer",PROPERTIES.."masterTimer.cpf")
base_hz = get_property("masterTimer","imu_target_hz")

-- Fully start up soem hardware before anything else
deployer:import("soem_master")
deployer:import("soem_ebox")
deployer:loadComponent("soemMaster","soem_master::SoemMasterComponent")
soemMaster=deployer:getPeer("soemMaster")
set_property("soemMaster","ifname","eth2")
deployer:setActivityOnCPU("soemMaster", 0.002, soemPrio, ORO_SCHED_RT,2)
soemMaster:configure()
soemMaster:start()

-- Load up the rest of the Carousel 1 hardware that interfaces with sensors and hardware
libraryNames={"mcuHandler","voltageController","encoder","cameraTrigger","lineAngleSensor","LEDTracker"}
classNames={  "McuHandler","VoltageController","Encoder","CameraTrigger","LineAngleSensor","LEDTracker"}
instanceNames=libraryNames
print "HEEEEEEEEEEEEEEERE 0"

for i=1,#libraryNames do
	load_component(libraryNames[i],classNames[i],instanceNames[i])
end
print "HEEEEEEEEEEEEEEERE 1"

-- Several components need to be peered with soemMaster component
for i,name in ipairs({"encoder","cameraTrigger","lineAngleSensor"}) do
	deployer:connectPeers("soemMaster", name)
end


-- Configure the hardware components
load_properties("mcuHandler",PROPERTIES.."tcp.cpf") -- Throwing "TinyDemarshaller" type warnings

set_property("mcuHandler","Ts",1.0/base_hz)
set_property("mcuHandler","rtMode",true)

set_property("encoder","encoderPort",0)
set_property("encoder","Ts",0.002)

-- LED tracker - hangs until frame arrival, does processing, and re-triggers itself.
set_property("LEDTracker","useExternalTrigger",true)
set_property("LEDTracker","sigma_marker",20)


----------------- Set Priorities and activities

deployer:setActivityOnCPU("masterTimer", 1.0 / base_hz, masterTimerPrio, ORO_SCHED_RT,2)

deployer:setActivityOnCPU("mcuHandler", 0.0, sensorPrio, ORO_SCHED_RT,6)
deployer:setActivityOnCPU("voltageController", 0.01, sensorPrio, ORO_SCHED_RT,6)
deployer:setActivityOnCPU("encoder", 0.0, sensorPrio, ORO_SCHED_RT,6)
deployer:setActivityOnCPU("cameraTrigger", 0.0, sensorPrio, ORO_SCHED_RT,6)
deployer:setActivityOnCPU("lineAngleSensor", 0.0, sensorPrio, ORO_SCHED_RT,6)
deployer:setActivityOnCPU("LEDTracker", 0.0, LEDTrackerPrio, ORO_SCHED_RT,6)
--deployer:setActivityOnCPU("poseFromMarkers", 0.0, LEDTrackerPrio, ORO_SCHED_RT,6)

---------------- Connect Components

cp = rtt.Variable("ConnPolicy")
cpLT = rtt.Variable("ConnPolicy")
cpLT.type = 1
cpLT.size = 5

deployer:connect("masterTimer.imuClock","mcuHandler.trigger", cp)
deployer:connect("masterTimer.cameraClock", "LEDTracker.triggerTimeStampIn", cpLT)
deployer:connect("masterTimer.cameraClock", "cameraTrigger.Trigger", cp)
--deployer:connect("LEDTracker.markerPositions", "poseFromMarkers.markerPositions", cp)

deployer:connect("voltageController.eboxAnalog", "soemMaster.Slave_1001.AnalogIn", cp)
deployer:connect("soemMaster.Slave_1001.Measurements", "encoder.eboxOut", cp)
deployer:connect("soemMaster.Slave_1001.Measurements", "lineAngleSensor.eboxOut", cp)

--------------- Configure and start the components

masterTimer:configure()

for i=1,#instanceNames do
	_G[instanceNames[i]]:configure()
	_G[instanceNames[i]]:start()
	if instanceNames[i]=="encoder" then
		print "Sleeping for 5 seconds after starting of encoder component, because its velocity estimate starts way off sometimes."
		sleep(5)
		print "Done sleeping, continuing loading the other components."
	end
end

