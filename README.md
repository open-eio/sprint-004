# sprint-004
Trying to get a BBB working to measure depth, conductivity, temp

E.g.: 

load opk_serial_depth.ino onto an arduino attached to /dev/ttyUSB0

then:

sudo python pull-serial --path /dev/ttyUSB0 | ./push_phant_json --publickey AJjpo34AYQc1d401qA41 --privatekey rzqrD4bYKMiznp0zqBpz
