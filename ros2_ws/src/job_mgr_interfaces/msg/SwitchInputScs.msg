# PARTIAL RECONSTRUCTION -- the real struct (folder interfaces/SwitchInputScs/,
# confirmed via LpsSaJobMgrApp.h:36) does not exist anywhere in this checkout.
# No struct declaration, no enum values, no field list available. This is
# reconstructed PURELY from usage evidence -- every real call site that
# touches this type, anywhere in the tree:
#   LpsSaJobMgrScs.cpp:60      STG_Input.get_STG_value(STG4) == STG::CLOSED
#   ReadStoreSwitchTx.cpp:10   STG_Input.get_STG_value(3) == STG::CLOSED
#   hornOnStoreTest.cpp:162    switchInput.set_STG_value(3, pressed)
#
# What this tells us, and no more: the real class exposes
# get_STG_value(int)/set_STG_value(int, int) accessors over what is presumably
# an internal array of switch-to-ground inputs, and the STG enum has at least
# a CLOSED value (compared, never assigned a number anywhere in this tree).
# JobMgr and every other consumer found ONLY ever touches index 3/STG4 (the
# "Store" switch) -- never any other index. SCOPED to exactly that: this
# message does not attempt to model however many total switches the real
# struct actually holds, since there is zero usage evidence for any of them.
#
# `closed` collapses whatever the real STG enum's full value set is down to
# the one binary distinction every consumer actually tests
# (`== STG::CLOSED` vs not) -- a deliberate scoping simplification, not a
# claim that STG only has two states.

bool closed
