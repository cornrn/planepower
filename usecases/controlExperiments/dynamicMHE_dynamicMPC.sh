#!/bin/sh
$MYOROCOSCOMPONENTS/usecases/common.sh

rosrun ocl deployer-gnulinux -lerror -s dynamicMHE_dynamicMPC.ops
