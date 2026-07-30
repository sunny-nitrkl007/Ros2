# Wire equivalent of SystemHardwareHealthRequestStorage
# (SystemHardwareHealthRequestOutput channel, WeighApp's
# SystemHardwareHealthRequestOutput_).
# Source: eta-ais/prod/coretech/ais_pcm/ais/interfaces/SystemHardwareHealthRequest/SystemHardwareHealthRequest.h:9-30
# Genuinely empty -- the real struct's serialize() body is a template
# placeholder comment ("/* Archive Fields Here */"), never filled in. A
# pure trigger/ping message: SCS's send() call (LpsSaScs.cpp:581) is what
# carries meaning, not any payload.
#
# NOTE (binding pattern, relevant for Step 5, not this step): unlike every
# other WeighApp channel, this one is NOT bound via task::InterfaceDb::bind()
# to an OutputInterface<T>* -- it's a direct SCSOutData<SystemHardwareHealth
# Request> member (LpsSaWeighApp.h:394), publicly inheriting from the Datum
# type itself and wrapping send-on-change/rate-limited send() logic around
# an internally-held OutputInterface<T>*. RosOutputInterface<T>'s plain
# publish(x) shim (matching SCS's plain OutputInterface<T>::publish())
# does not match this shape 1:1 -- confirm during Step 5 whether a plain
# publish() call is an acceptable simplification (payload is empty, so the
# send-on-change semantics carry no real data-change meaning anyway) or
# whether a small adapter is needed.
