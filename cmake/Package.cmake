#
# CPack Configuration
#

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Open Source EchoLink Proxy")

set(CPACK_PACKAGE_EXECUTABLES "openelpd;openelpd")

set(CPACK_PACKAGE_INSTALL_DIRECTORY "OpenELP")
set(CPACK_PACKAGE_NAME "OpenELP")
set(CPACK_PACKAGE_VENDOR "Scott K Logan (KM0H)")
set(CPACK_PACKAGE_CONTACT "logans@cottsay.net")

set(CPACK_PACKAGE_VERSION_MAJOR "${OPENELP_MAJOR_VERSION}")
set(CPACK_PACKAGE_VERSION_MINOR "${OPENELP_MINOR_VERSION}")
set(CPACK_PACKAGE_VERSION_PATCH "${OPENELP_PATCH_VERSION}")

set(CPACK_RESOURCE_FILE_LICENSE "${OPENELP_DIR}/LICENSE")

# There is an NSIS bug here - we have to use a backslash
set(CPACK_PACKAGE_ICON "${OPENELP_DOC_DIR}/icons\\\\installer.bmp")

set(CPACK_NSIS_INSTALLED_ICON_NAME "bin/openelpd.exe")

if(WIN32)
  set(CPACK_COMPONENTS_SERVICE "service")
else()
  set(CPACK_COMPONENTS_SERVICE "")
endif()

if(OPENELP_DOC_HTMLHELP)
  set(CPACK_COMPONENTS_HELP "help")
  set(CPACK_NSIS_CREATE_ICONS_EXTRA "${CPACK_NSIS_CREATE_ICONS_EXTRA}
    CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\OpenELP Help.lnk' '$INSTDIR\\\\openelp.chm' ''"
    )
  set(CPACK_NSIS_DELETE_ICONS_EXTRA "${CPACK_NSIS_DELETE_ICONS_EXTRA}
    Delete '$SMPROGRAMS\\\\$START_MENU\\\\OpenELP Help.lnk'"
    )
else()
  set(CPACK_COMPONENTS_HELP "")
endif()

set(CPACK_COMPONENTS_ALL
  app
  config
  devel
  libs
  license
  ${CPACK_COMPONENTS_SERVICE}
  ${CPACK_COMPONENTS_HELP}
  )
set(CPACK_COMPONENT_LIBS_REQUIRED TRUE)
set(CPACK_COMPONENT_LICENSE_HIDDEN TRUE)
set(CPACK_COMPONENT_HELP_HIDDEN TRUE)
set(CPACK_COMPONENT_APP_DISPLAY_NAME "Executable Application")
set(CPACK_COMPONENT_CONFIG_DISPLAY_NAME "Sample Configuration")
set(CPACK_COMPONENT_LIBS_DISPLAY_NAME "Proxy Library")
set(CPACK_COMPONENT_DEVEL_DISPLAY_NAME "Development Files")
set(CPACK_COMPONENT_SERVICE_DISPLAY_NAME "Windows Service")
set(CPACK_COMPONENT_APP_DESCRIPTION
  "Simple executable application which opens an EchoLink proxy"
  )
set(CPACK_COMPONENT_CONFIG_DESCRIPTION
  "Sample configuration file for customizing an EchoLink proxy"
  )
set(CPACK_COMPONENT_LIBS_DESCRIPTION
  "Library used by applications to set up and control an EchoLink proxy"
  )
set(CPACK_COMPONENT_DEVEL_DESCRIPTION
  "C headers and library files used by other projects to build using OpenELP"
  )
set(CPACK_COMPONENT_SERVICE_DESCRIPTION
  "Windows background service"
  )
set(CPACK_COMPONENT_APP_DEPENDS libs)
set(CPACK_COMPONENT_DEVEL_DEPENDS libs)
set(CPACK_COMPONENT_SERVICE_DEPENDS libs)
set(CPACK_COMPONENT_DEVEL_DISABLED True)

set(CPACK_NSIS_URL_INFO_ABOUT "http://github.com/cottsay/openelp")
set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "${CPACK_NSIS_EXTRA_INSTALL_COMMANDS}
  nsExec::Exec '$INSTDIR\\\\bin\\\\openelp_service.exe install'
  !include 'FileFunc.nsh'
  \\\${GetSize} '$INSTDIR' '/S=0K' $0 $1 $2
  IntFmt $0 '0x%08X' $0
  WriteRegDWORD HKLM 'SOFTWARE\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\Uninstall\\\\OpenELP' 'EstimatedSize' '$0'
  WriteRegStr HKLM 'SYSTEM\\\\CurrentControlSet\\\\Services\\\\EventLog\\\\Application\\\\OpenELP' 'EventMessageFile' '$INSTDIR\\\\bin\\\\openelp.dll'
  WriteRegDWORD HKLM 'SYSTEM\\\\CurrentControlSet\\\\Services\\\\EventLog\\\\Application\\\\OpenELP' 'TypesSupported' '0x07'"
  )
set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "${CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS}
  nsExec::Exec '$INSTDIR\\\\bin\\\\openelp_service.exe uninstall'
  DeleteRegKey HKLM 'SYSTEM\\\\CurrentControlSet\\\\Services\\\\EventLog\\\\Application\\\\OpenELP'"
  )

include(CPack)

#
# Licenses
#

if(WIN32)
  install(FILES "${OPENELP_DIR}/LICENSE"
    DESTINATION "./"
    COMPONENT license
    RENAME "LICENSE.txt"
    )

  if(OPENELP_BUNDLE_PCRE)
    add_custom_command(TARGET pcre POST_BUILD
      COMMAND ${CMAKE_COMMAND} -P ${OPENELP_DIR}/cmake/unix2dos.cmake
        "${OPENELP_PCRE_LICENSE_PATH}"
        "${CMAKE_CURRENT_BINARY_DIR}/LICENSE.pcre"
      )

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/LICENSE.pcre"
      DESTINATION "./"
      COMPONENT license
      RENAME "LICENSE.pcre.txt"
      )
  endif()
endif()
