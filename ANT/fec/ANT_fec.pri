INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD



SOURCES     +=\
    $$PWD/fec_controller.cpp
   

HEADERS     += \
    $$PWD/fec_controller.h


#////////////////////////////////////////////////////////////////////////////////////////////////////////
win32 {

    INCLUDEPATH += D:\Workspaces\Qt\ANT_LIB\inc
    INCLUDEPATH += D:\Workspaces\Qt\ANT_LIB\common
    INCLUDEPATH += D:\Workspaces\Qt\ANT_LIB\libraries
    INCLUDEPATH += D:\Workspaces\Qt\ANT_LIB\software\ANTFS
    INCLUDEPATH += D:\Workspaces\Qt\ANT_LIB\software\serial
    INCLUDEPATH += D:\Workspaces\Qt\ANT_LIB\software\serial\device_management
    INCLUDEPATH += D:\Workspaces\Qt\ANT_LIB\software\system
    INCLUDEPATH += D:\Workspaces\Qt\ANT_LIB\software\USB
    INCLUDEPATH += D:\Workspaces\Qt\ANT_LIB\software\USB\device_handles
    INCLUDEPATH += D:\Workspaces\Qt\ANT_LIB\software\USB\devices
}
