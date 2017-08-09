# EMG-processing
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 2.8)

file(GLOB scripts  ${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.template)
file(GLOB scriptsXml  ${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.xml)
file(GLOB conf_ini ${CMAKE_CURRENT_SOURCE_DIR}/conf/*.ini)

yarp_install(FILES ${conf_ini} DESTINATION ${ICUBCONTRIB_CONTEXTS_INSTALL_DIR}/${PROJECTNAME})
yarp_install(FILES ${scripts}  DESTINATION ${ICUBCONTRIB_APPLICATIONS_TEMPLATES_INSTALL_DIR})
yarp_install(FILES ${scriptsXml}  DESTINATION ${ICUBCONTRIB_APPLICATIONS_INSTALL_DIR})

