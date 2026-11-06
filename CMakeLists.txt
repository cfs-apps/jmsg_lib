project(CFS_JMSG_LIB C)

include_directories(fsw/mission_inc)
include_directories(fsw/platform_inc)
include_directories(fsw/topic_plugins)
include_directories(fsw/usr_tplugs)
include_directories(${app_c_fw_MISSION_DIR}/fsw/app_inc)
include_directories(${app_c_fw_MISSION_DIR}/fsw/platform_inc)
include_directories(${app_c_fw_MISSION_DIR}/fsw/mission_inc)
#Example app with TPLUGs: include_directories(${sc_sim_MISSION_DIR}/fsw/usr_tplugs)

aux_source_directory(fsw/src LIB_SRC_FILES)
aux_source_directory(fsw/topic_plugins LIB_SRC_FILES)
aux_source_directory(fsw/usr_tplugs LIB_SRC_FILES)
#Example app with TPLUGs: aux_source_directory(${sc_sim_MISSION_DIR}/fsw/usr_tplugs LIB_SRC_FILES)

# Create the app module
add_cfe_app(jmsg_lib ${LIB_SRC_FILES})

