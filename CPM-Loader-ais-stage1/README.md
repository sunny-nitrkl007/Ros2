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
ROS2 and DDS, an industry-standard publish/subscribe messaging layer, while
leaving everything else about how the two applications behave completely
untouched.

## Why

DDS is a widely used, well-supported standard for exactly this kind of
inter-process messaging, and moving onto it opens the door to interacting
with these applications from outside the original closed system —
diagnostics tools, simulators, other services — without needing to speak
the platform's proprietary protocol. It's also a step toward a more modern,
maintainable communication layer overall.

## What did NOT change

This is a communication-only migration. The weighing calculations, the
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

## Design choice: polling, not callbacks

Both applications already worked on a simple polling model — once per
work cycle, check if anything new has arrived, react if so, then move on.
Rather than introducing a callback-driven, multi-threaded model (which is
what a more "native" ROS2 integration might look like by default), we kept
that same polling rhythm: once per cycle, the application asks ROS2 to
deliver anything that's arrived, and the wrapper hands it over exactly the
way the old interface did.

This was a deliberate choice. It avoids introducing multi-threading and
the synchronization concerns that come with it, into code that was never
designed with that in mind, and it keeps the applications' overall
behavior as predictable and easy to reason about as it was before.

## Current scope

This first stage covers one specific, well-understood slice of the
communication between the two applications — enough to prove the approach
works end-to-end, without touching every message path at once. The
remaining communication paths still run on the original transport for now
and are expected to move over in later stages, following the same pattern
established here.

## Status

The converted communication path builds and runs, and has been validated
end-to-end on the intended message types. Wiring this into the platform's
real build system, and running it on real hardware, is the next step.
