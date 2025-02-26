cmake_minimum_required(VERSION 3.18)
project(FlasherApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#include_directories(/usr/include/mbedtls)
## Lier les bibliothèques de mbedtls
#set(MBEDTLS_LIBRARIES /usr/lib/x86_64-linux-gnu/libmbedtls.a /usr/lib/x86_64-linux-gnu/libmbedx509.a /usr/lib/x86_64-linux-gnu/libmbedcrypto.a)

# inclure les fichiers d'en-tête
include_directories(${CMAKE_SOURCE_DIR}/mbedtls-2.16.9/include)
# inclure les billiothèque compilées
link_directories(${CMAKE_SOURCE_DIR}/mbedtls-2.16.9/build/library)
set(MBEDTLS_LIBRARIES ${CMAKE_SOURCE_DIR}/mbedtls-2.16.9/build/library/libmbedcrypto.a
                      ${CMAKE_SOURCE_DIR}/mbedtls-2.16.9/build/library/libmbedtls.a
                      ${CMAKE_SOURCE_DIR}/mbedtls-2.16.9/build/library/libmbedx509.a
)
find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        Test
        Concurrent
        REQUIRED)
enable_testing(true)
add_executable(${PROJECT_NAME} App/main.cpp
        sources/mainWindow/mainwindow.cpp
        sources/mainWindow/mainwindow.h
        sources/mainWindow/MainWindow.ui
        sources/signature/CheckSignature.cpp
        sources/signature/CheckSignature.h
        sources/serial_number/SerialNumber.cpp
        sources/serial_number/SerialNumber.h
        sources/jlink_command/JlinkCommand.cpp
        sources/jlink_command/JlinkCommand.h
        sources/mainWindow/ThreadBouton.cpp
        sources/mainWindow/ThreadBouton.h
)
target_sources(${PROJECT_NAME}
                PUBLIC
                "${CMAKE_CURRENT_LIST_DIR}/sources/mainWindow/mainwindow.h"
                "${CMAKE_CURRENT_LIST_DIR}/sources/signature/CheckSignature.h"
                "${CMAKE_CURRENT_LIST_DIR}/sources/serial_number/SerialNumber.h"
                "${CMAKE_CURRENT_LIST_DIR}/sources/jlink_command/JlinkCommand.h"
                "${CMAKE_CURRENT_LIST_DIR}/sources/mainWindow/ThreadBouton.h"

)
target_include_directories(${PROJECT_NAME}
                PUBLIC
                "${CMAKE_CURRENT_LIST_DIR}/sources/mainWindow/"
                "${CMAKE_CURRENT_LIST_DIR}/sources/signature/"
                "${CMAKE_CURRENT_LIST_DIR}/sources/serial_number/"
                "${CMAKE_CURRENT_LIST_DIR}/sources/jlink_command/"
)
# Lier les bibliothèques Qt6 et mbedtls à l'exécutable
target_link_libraries(FlasherApp
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Test
        Qt::Concurrent
        ${MBEDTLS_LIBRARIES}
)

# pour les test unitaires
add_executable(TU_SIGNATURE_SHOULD sources/signature/CheckSignature.cpp
                                    unit_test/signature_test/SignatureTest.cpp)
target_include_directories(TU_SIGNATURE_SHOULD PRIVATE
                                                        sources
                                                        unit_test)

target_link_libraries(TU_SIGNATURE_SHOULD
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Test
        ${MBEDTLS_LIBRARIES}
)
add_executable(TU_SERIAL_NUMBER_SHOULD sources/serial_number/SerialNumber.cpp
                                        unit_test/serial_number_test/SerialNumberTest.cpp)
target_include_directories(TU_SERIAL_NUMBER_SHOULD PRIVATE
                                                            sources
                                                            unit_test)
target_link_libraries(TU_SERIAL_NUMBER_SHOULD
        Qt::Core
        Qt::Gui
        Qt::Test
        Qt::Widgets
        ${MBEDTLS_LIBRARIES}
)
add_executable(TU_MAINWINDOW_SHOULD sources/mainWindow/mainwindow.cpp
                                    unit_test/mainWindow_test/MainWindowTest.cpp
        sources/mainWindow/ThreadBouton.cpp
        sources/mainWindow/ThreadBouton.h)
target_include_directories(TU_MAINWINDOW_SHOULD PRIVATE
                                                sources
                                                unit_test)
target_link_libraries(TU_MAINWINDOW_SHOULD
        Qt::Core
        Qt::Gui
        Qt::Test
        Qt::Widgets
        ${MBEDTLS_LIBRARIES}
)
