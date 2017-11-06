# Copyright: (C) 2017 Inria
# Authors: Serena Ivaldi, Waldez Gomes
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

set(BUILD_EMG_GUI ON CACHE BOOL "Build EMGui module (interacts with other EMG modules)")

add_subdirectory(EMGserver)
add_subdirectory(EMGhuman)
add_subdirectory(EMGhuman2robot)

if(BUILD_EMG_GUI)

add_subdirectory(EMGui)

endif(BUILD_EMG_GUI)



