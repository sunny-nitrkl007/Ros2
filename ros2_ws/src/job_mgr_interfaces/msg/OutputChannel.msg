# PARTIAL RECONSTRUCTION -- OutputChannelOutput (live call site
# LpsSaJobMgrScs.cpp:930 ->publish()). Real struct not in this checkout, see
# OutputCmd.msg for the full evidence trail.
#
# `outputChannel.AddOutputAppCmd(command)` (LpsSaJobMgrScs.cpp:928) is an
# "Add" method, not a setter -- strong evidence this wraps a collection
# (vector-like) of OutputCmd, not a single one. Modeled as an array on that
# basis. Only ever seen with exactly one command added per publish in this
# tree; whether multiple commands per message is actually used elsewhere is
# unknown -- the array form is still correct either way.

OutputCmd[] commands
