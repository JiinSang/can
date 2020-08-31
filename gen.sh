sudo /sbin/ip link set can0 down

gcc joystick.c -o joystick-"`date +"%d-%m-%Y"`"

./joystick-"`date +"%d-%m-%Y"`"