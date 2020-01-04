INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD



SOURCES     +=\
    $$PWD/hub.cpp \
    $$PWD/antmsg.cpp \
    $$PWD/ant_controller.cpp \
    $$PWD/kickrant.cpp

HEADERS     += \
    $$PWD/hub.h \
    $$PWD/common_pages.h \
    $$PWD/antplus.h \
    $$PWD/antmsg.h \
    $$PWD/antdefine.h \
    $$PWD/ant_controller.h \
    $$PWD/kickrant.h \



#////////////////////////////////////////////////////////////////////////////////////////////////////////
win32 {

    #INCLUDEPATH += F:\Workspaces\Qt\ANT_LIB\inc
#    INCLUDEPATH += F:\Workspaces\Qt\ANT_LIB\common
#    INCLUDEPATH += F:\Workspaces\Qt\ANT_LIB\libraries
#    INCLUDEPATH += F:\Workspaces\Qt\ANT_LIB\software\ANTFS
#    INCLUDEPATH += F:\Workspaces\Qt\ANT_LIB\software\serial
#    INCLUDEPATH += F:\Workspaces\Qt\ANT_LIB\software\serial\device_management
#    INCLUDEPATH += F:\Workspaces\Qt\ANT_LIB\software\system
#    INCLUDEPATH += F:\Workspaces\Qt\ANT_LIB\software\USB
#    INCLUDEPATH += F:\Workspaces\Qt\ANT_LIB\software\USB\device_handles
#    INCLUDEPATH += F:\Workspaces\Qt\ANT_LIB\software\USB\devices


#    LIBS +=  "C:\Program Files (x86)\Windows Kits\8.0\Lib\win8\um\x86\User32.lib"
#    LIBS +=  "C:\Program Files (x86)\Windows Kits\8.0\Lib\win8\um\x86\AdvAPI32.lib"

   LIBS +=  "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.17763.0\um\x64\User32.lib"
   LIBS +=  "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.17763.0\um\x64\AdvAPI32.lib"


    #static ANT_LIB builded with ANT_LIB project (Download source from thisisant.com, build with VisualStudio)
    #Also include the DLL inside /Release
    #LIBS +=  "C:\Dropbox\MSVC2012\ANT_Lib.lib"
    #LIBS +=  "C:\Dropbox\MSVC2013\ANT_Lib.lib"
    #LIBS +=  "F:\Workspaces\Qt\ANT-SDK_PC.3.5\Debug\*.dll"
#TODO: rebuild with MSVC2017 HERE




}

#////////////////////////////////////////////////////////////////////////////////////////////////////////



macx {

    INCLUDEPATH += /Users/$${MAC_USERNAME}/Dropbox/ANT-SDK_Mac.3.5/ANT_LIB/common
    INCLUDEPATH += /Users/$${MAC_USERNAME}/Dropbox/ANT-SDK_Mac.3.5/ANT_LIB/inc
    INCLUDEPATH += /Users/$${MAC_USERNAME}/Dropbox/ANT-SDK_Mac.3.5/ANT_LIB/software/ANTFS
    INCLUDEPATH += /Users/$${MAC_USERNAME}/Dropbox/ANT-SDK_Mac.3.5/ANT_LIB/software/serial
    INCLUDEPATH += /Users/$${MAC_USERNAME}/Dropbox/ANT-SDK_Mac.3.5/ANT_LIB/software/serial/device_management
    INCLUDEPATH += /Users/$${MAC_USERNAME}/Dropbox/ANT-SDK_Mac.3.5/ANT_LIB/software/system
    INCLUDEPATH += /Users/$${MAC_USERNAME}/Dropbox/ANT-SDK_Mac.3.5/ANT_LIB/software/USB
    INCLUDEPATH += /Users/$${MAC_USERNAME}/Dropbox/ANT-SDK_Mac.3.5/ANT_LIB/software/USB/device_handles
    INCLUDEPATH += /Users/$${MAC_USERNAME}/Dropbox/ANT-SDK_Mac.3.5/ANT_LIB/software/USB/devices
    INCLUDEPATH += /Users/$${MAC_USERNAME}/Dropbox/ANT-SDK_Mac.3.5/ANT_LIB/software/USB/iokit_driver


    #static ANT_LIB builded with ANT_LIB project (Download source from thisisant.com, build with XCode)
    LIBS += /Users/$${MAC_USERNAME}/Dropbox/ANT-SDK_Mac.3.5/bin/libantbase.a



}






