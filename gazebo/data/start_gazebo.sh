#!/bin/bash

export LEGACY_GRAPHICS=0

# Start Gazebo with the iris model
if [ $LEGACY_GRAPHICS -eq 1 ]; then
  echo "Starting Gazebo with the iris model and legacy graphics (Ogre)"
  gz sim -v4 -r iris_runway.sdf --render-engine ogre &
else
  echo "Starting Gazebo with the iris model and normal graphics (Ogre2)"
  gz sim -v4 -r iris_runway.sdf &
fi

# Start the SITLs for Ardupilot
/ardupilot/build/sitl/bin/arducopter -S --model JSON --home="(0.00003, 0.00004, 0.0, 0.0)" --speedup 1 --slave 0 --defaults /ardupilot/Tools/autotest/default_params/copter.parm,/ardupilot/Tools/autotest/default_params/gazebo-iris.parm,/data/parm1.parm --sim-address=127.0.0.1 --sim-port-out=9801 --base-port 5760 -I0 > /dev/null &2>1 &

/ardupilot/build/sitl/bin/arducopter -S --model JSON --home="(0.00003, 0.00004, 0.0, 0.0)" --speedup 1 --slave 0 --defaults /ardupilot/Tools/autotest/default_params/copter.parm,/ardupilot/Tools/autotest/default_params/gazebo-iris.parm,/data/parm2.parm --sim-address=127.0.0.1 --sim-port-out=9802 --base-port 5770 -I0 > /dev/null &2>1 &

/ardupilot/build/sitl/bin/arducopter -S --model JSON --home="(0.00003, 0.00004, 0.0, 0.0)" --speedup 1 --slave 0 --defaults /ardupilot/Tools/autotest/default_params/copter.parm,/ardupilot/Tools/autotest/default_params/gazebo-iris.parm,/data/parm3.parm --sim-address=127.0.0.1 --sim-port-out=9803 --base-port 5780 -I0 > /dev/null &2>1 &

/ardupilot/build/sitl/bin/arducopter -S --model JSON --home="(0.00003, 0.00004, 0.0, 0.0)" --speedup 1 --slave 0 --defaults /ardupilot/Tools/autotest/default_params/copter.parm,/ardupilot/Tools/autotest/default_params/gazebo-iris.parm,/data/parm4.parm --sim-address=127.0.0.1 --sim-port-out=9804 --base-port 5790 -I0 > /dev/null &2>1 &

# mavproxy.py --out 127.0.0.1:14550 --master tcp:127.0.0.1:5760 --master tcp:127.0.0.1:5770 --sitl 127.0.0.1:5501

python3 /data/expose.py > out_expose.txt &2>1 &

bash 
