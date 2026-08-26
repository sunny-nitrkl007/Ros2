# Keeping Legacy SCS Consumers Fed During the ROS2 Switch

## What this is

When the Job Manager and Weighing app's shared communication moved onto
ROS2/DDS, a few other applications on the machine got left behind. They
still read some of that same data the old way (over SCS), and nobody had
told them anything changed — so once the switch happened, they stopped
receiving updates entirely. This work sends the data back out over SCS
too, alongside ROS2, so nothing downstream notices a difference.

## The problem, in plain terms

Three pieces of data — a request the Job Manager sends to the Weighing
app, the Weighing app's response, and its continuous status broadcast —
used to go out over SCS. A few other applications (a diagnostics data
server, a condition-monitoring service, a test-tooling app) read that
same SCS data directly, independently of the Job Manager and Weighing
app. When the communication between the Job Manager and Weighing app
moved to ROS2, the SCS side of these three data paths went quiet. The
Job Manager and Weighing app themselves work fine over ROS2 — but
anything else still listening on the old SCS side gets nothing.

## The approach: send it out both ways

Rather than rebuilding those other applications to understand ROS2 (out
of scope here) or routing everything through a translator process (a
bigger, separate piece of work), the simplest fix is: whenever the Job
Manager or Weighing app would have sent this data, send it twice — once
over ROS2 like it already does, and once over the old SCS channel like
it always used to. Both sends use the same underlying data, just written
into two different formats side by side. If the ROS2 side ever has a
hiccup, the SCS side keeps going independently, and vice versa.

## What was actually wrong before this fix

For the response and status-broadcast paths, an SCS send had already
been attempted, but it was passing the ROS2 message straight into the
SCS send call — a mismatch that wouldn't build at all. For the request
path, there was no SCS send attempt in place yet. Both are addressed
here.

## Debug logging

Every send — SCS and ROS2, success and failure — now logs at error
level, tagged `[SCS]` or `[ROS2]`. The normal logging level in use only
shows error-level messages, so without this, there'd be no way to
confirm from the logs which path actually fired. This is not meant to
represent an actual error condition on the success lines — it's there
purely so the send can be confirmed from the logs.

## Files touched

**Weighing app**
- `apps/LpsSaWeighApp/LpsSaScs.cpp`
  - `LpsSaScsSendReqstResponse()` — fixed the response send
  - `LpsSaWeighingScsTx()` — fixed the status-broadcast send

**Job Manager**
- `apps/LpsSaJobMgrApp/LpsSaJobMgrApp.h` — added a handle for the SCS side of the request path
- `apps/LpsSaJobMgrApp/LpsSaJobMgrApp.cpp` — set up that handle at startup
- `apps/LpsSaJobMgrApp/LpsSaJobMgrScs.cpp` — `LpsSaJobMgrScsSendCmd()` now sends the request both ways

## What this doesn't cover

This is a stopgap so the other applications keep working right now. The
longer-term fix is a dedicated relay process that reads the ROS2 side and
republishes onto SCS for these legacy consumers, so the Job Manager and
Weighing app themselves don't need to know about SCS at all. That relay
doesn't exist yet for these three data paths.
