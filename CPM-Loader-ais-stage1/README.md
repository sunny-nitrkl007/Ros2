# Loader Payload System: Moving Inter-App Communication to ROS2/DDS

## What this is

Two applications on the machine's control system — the Job Manager and the
Weighing application — talk to each other constantly while the machine is
working: the Job Manager asks the Weighing app to zero the scale, capture a
calibration reference, clear a warning, and so on, and the Weighing app
answers back and also continuously broadcasts its live payload and status
data so the rest of the system can use it.

That conversation currently happens over the platform's internal
communication service (SCS), a mechanism that only works between processes
running together in this system. This work replaces that transport with
ROS2 and DDS, while
leaving everything else about how the two applications behave completely
untouched.


## What did NOT change

Current scope covers communication-only migration. The weighing calculations, the
calibration logic, the diagnostic conditions, the state machines, the
timing — none of that changed. If a request came in and triggered a
recalibration before, it still does exactly that, in exactly the same way.
The only thing that's different is how the request physically got from one
application to the other.

## The approach: a drop-in wrapper

Rather than rewriting how each application's business logic talks to the
outside world, we built a small wrapper layer that looks, from the
business logic's point of view, just like the old communication interface
it already knew how to use: "check if a request came in," "send a
response," "publish the latest status." Underneath, that wrapper is now
backed by a real ROS2 publisher or subscriber instead of the old
communication channel.

Practically, this meant two things changed in each application:
- The data structures being sent and received moved from hand-written
  structs to ROS2 message definitions (the same information, just
  described in a way ROS2's tooling understands).
- The few lines of code that construct these interfaces at startup now
  build the new ROS2-backed version instead of the old one.

Everywhere else, the code that actually uses these interfaces reads almost
identically to before, because the wrapper deliberately kept the same
shape.


## Current scope

This covers one specific slice of the communication
between the two applications — just to confirm the approach works
end-to-end. The remaining
communication paths still run on the original transport for now and are
expected to move over later, following the same pattern established here.

That slice is three data paths between the Job Manager and the Weighing app:
- **`LpsSaWeighReqstChannel`** — Job Manager asks the Weighing app to do
  something (zero the scale, capture a calibration reference, clear a warning)
- **`LpsSaWeighRespChannel`** — the Weighing app's reply to that request
- **`LpsSaWeighTxChannel`** — the Weighing app's continuous stream of live
  payload and status data

A few other applications on the machine read some of this same data
independently, the old way, over the original transport — a diagnostics
data server, a condition-monitoring service, a test-tooling app. So those
three data paths currently go out twice for now: once over ROS2 like everything
else here, and once over the original transport, unchanged, so those
other applications don't notice anything changed. Later this will be replaced by the bridge between the jobmanager/weighapp and other applications.


## Files touched

**Changed — communication wiring only, logic untouched:**
- `apps/LpsSaJobMgrApp/LpsSaJobMgrApp.h` / `.cpp` — the interface member
  types and the few lines at startup that build them
- `apps/LpsSaJobMgrApp/LpsSaJobMgrScs.cpp` — the request-sending call site
  (plus, separately, the dual-send addition described above)
- `apps/LpsSaWeighApp/LpsSaWeighApp.h` / `.cpp` — same, for the Weighing app
- `apps/LpsSaWeighApp/LpsSaScs.cpp` — the response and status-broadcast
  call sites (plus the same dual-send addition)

**New — the wrapper layer itself:**
- `prod/common/ros2wrapper/RosInputInterface.h`
- `prod/common/ros2wrapper/RosOutputInterface.h`
- `prod/common/interfaces/LpsSaWeighReqstChannel/DDSWeighAppInf.hpp` — the
  ROS2-backed version of the small helper class that manages this
  particular request/response/status conversation; kept as its own file
  rather than editing the original in place, since other, unrelated parts
  of the system still use the original as-is

**Everything else** in this tree — the weighing/calibration math, the
job-tracking logic, NVM storage, diagnostics, all of it — is present
exactly as it always was, unedited, so the application builds as a
complete whole rather than needing to be assembled against a separate
checkout.

