if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "CPack packaging is configured only for Linux in this project.")
    return()
endif()

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "Joraslav")
set(CPACK_PACKAGE_CONTACT "maintainers@localhost")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Console utility for text log analysis")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")

set(CPACK_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}"
)

if(EXISTS "/etc/debian_version")
    set(CPACK_GENERATOR "DEB")
elseif(EXISTS "/etc/fedora-release" OR EXISTS "/etc/redhat-release")
    set(CPACK_GENERATOR "RPM")
else()
    set(CPACK_GENERATOR "DEB;RPM")
endif()

# Debian package settings.
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
endif()

# RPM package settings.
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
set(CPACK_RPM_PACKAGE_GROUP "Applications/System")
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_AUTOREQPROV ON)

include(CPack)
