# About
This creates a joystick device from a Space Navigator using spacenavd. Useful for things
such as support in Kerbal Space Program. This is feature complete as is, so unless
there is a feature requst, I'm probably not going to update the repo.

# Building
Standard cmake build, requires spacenavd and spacenavd header files. Install as a package
from your distribution.

# Install
Make sure you can create uinput devices as a user, use this as a udev rule:

KERNEL=="uinput", MODE="0660", GROUP="YOUR_USERNAME_HERE", OPTIONS+="static_node=uinput"

If you have a Steam Controller, you maybe already have this rule.

# Running
-l <n> sets a logging level. 0(default) for fatal, 1 for warnings, 2 for informational, 3 for debug

-d daemonize(run in the background)

-u specify the device file for uinput, default is /dev/uinput

# Configuration
Use the default spacenavd configuration to set things such as deadzones, sensitivity, etc.

# Issues
None that I know of. Also, I do not know it works with other 3dconnexion devices. This only uses the puck and the first two buttons for input.
