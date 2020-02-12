target extended-remote /dev/cu.usbmodem7AB685971
monitor swdp_scan
attach 1
set confirm off
define lc
  load
  continue
end
define lr
  load
  run
end
define lq
  load
  quit
end
