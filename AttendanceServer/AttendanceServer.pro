QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    attendancewin.cpp \
    qfaceobject.cpp \
    registerwin.cpp \
    selectwin.cpp

HEADERS += \
    attendancewin.h \
    qfaceobject.h \
    registerwin.h \
    selectwin.h

FORMS += \
    attendancewin.ui \
    registerwin.ui \
    selectwin.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


#win32:CONFIG(release, debug|release): LIBS += -LD:/Opencv454/opencv/build/x64/vc15/lib/ -lopencv_world454
#else:win32:CONFIG(debug, debug|release): LIBS += -LD:/Opencv454/opencv/build/x64/vc15/lib/ -lopencv_world454d

#INCLUDEPATH += D:/Opencv454/opencv/build/include
#DEPENDPATH += D:/Opencv454/opencv/build/include


#添加opencv、seetaface的头文件
INCLUDEPATH += C:\opencv452\include
INCLUDEPATH += C:\opencv452\include\opencv2
INCLUDEPATH += D:\SeetaFace\include
INCLUDEPATH += D:\SeetaFace\include\seeta
#添加opencv\seetaface的库
LIBS += C:\opencv452\x64\mingw\lib\libopencv*
LIBS += D:\SeetaFace\lib\libSeeta*





#LIBS +=-L$$PWD/SeetaFace/lib \
#    -lSeetaFaceDetector \
#    -lSeetaFaceLandmarker \
#    -lSeetaFaceRecognizer \
#    -lSeetaFaceTracker \
#    -lSeetaNet \
#    -lSeetaQualityAssessor \


#LIBS += -L$$PWD/SeetaFace/lib -lseetafaceengine
#INCLUDEPATH += $$PWD/SeetaFace/include

#添加opencv、seetaface的头文件
#INCLUDEPATH += C:\opencv452\include
#INCLUDEPATH += C:\opencv452\include\opencv2
#INCLUDEPATH += C:\SeetaFace\include
#INCLUDEPATH += C:\SeetaFace\include\seeta
#添加opencv\seetaface的库
#LIBS += -LC:\SeetaFace\lib -lSeetaFaceDetection -lSeetaFaceLandmark
#LIBS += C:\opencv452\x64\mingw\lib\libopencv*
#LIBS += C:\SeetaFace\lib\libSeeta*


#LIBS += C:\SeetaFace\lib\libSeeta*
#INCLUDEPATH +=C:\SeetaFace\include
#INCLUDEPATH +=C:\SeetaFace\include\seeta

RESOURCES += \
    image.qrc

DISTFILES +=
