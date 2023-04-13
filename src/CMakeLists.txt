function(prepend var prefix)
    set(listVar "")
    foreach(f ${ARGN})
        list(APPEND listVar "${prefix}/${f}")
    endforeach(f)
    set(${var} "${listVar}" PARENT_SCOPE)
endfunction(prepend)

prepend(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}
        badooapi.h badooapi.cpp
        badooenums.h
        badoologindialog.h badoologindialog.cpp
        badoowrapper.h badoowrapper.cpp
        badoosearchsettingsdialog.h badoosearchsettingsdialog.cpp
        browsefolderdialog.h browsefolderdialog.cpp
        folderviewer.h folderviewer.cpp
        httprequest.h httprequest.cpp
        main.cpp main.qrc
        mainwindow.cpp mainwindow.h mainwindow.ui
        mediacontrols.h mediacontrols.cpp
        mediaviewer.h mediaviewer.cpp
        playencountersdialog.h playencountersdialog.cpp
        profileviewer.h profileviewer.cpp profileviewer.ui
        badwrapi.rc
)

set(SRC_FILES ${SRC_FILES} PARENT_SCOPE)
