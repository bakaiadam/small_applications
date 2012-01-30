QT += network

SOURCES += \
    main.cc \
    chipmunk/cpVect.c \
    chipmunk/cpSweep1D.c \
    chipmunk/cpSpatialIndex.c \
    chipmunk/cpSpaceStep.c \
    chipmunk/cpSpaceQuery.c \
    chipmunk/cpSpaceHash.c \
    chipmunk/cpSpaceComponent.c \
    chipmunk/cpSpace.c \
    chipmunk/cpShape.c \
    chipmunk/cpPolyShape.c \
    chipmunk/cpHashSet.c \
    chipmunk/cpCollision.c \
    chipmunk/cpBody.c \
    chipmunk/cpBBTree.c \
    chipmunk/cpBB.c \
    chipmunk/cpArray.c \
    chipmunk/cpArbiter.c \
    chipmunk/chipmunk.c \
    chipmunk/constraints/cpSlideJoint.c \
    chipmunk/constraints/cpSimpleMotor.c \
    chipmunk/constraints/cpRotaryLimitJoint.c \
    chipmunk/constraints/cpRatchetJoint.c \
    chipmunk/constraints/cpPivotJoint.c \
    chipmunk/constraints/cpPinJoint.c \
    chipmunk/constraints/cpGrooveJoint.c \
    chipmunk/constraints/cpGearJoint.c \
    chipmunk/constraints/cpDampedSpring.c \
    chipmunk/constraints/cpDampedRotarySpring.c \
    chipmunk/constraints/cpConstraint.c

HEADERS += \
    ../drawer.hh \
    chipmunk/prime.h \
    chipmunk/cpVect.h \
    chipmunk/cpSpatialIndex.h \
    chipmunk/cpSpace.h \
    chipmunk/cpShape.h \
    chipmunk/cpPolyShape.h \
    chipmunk/cpBody.h \
    chipmunk/cpBB.h \
    chipmunk/cpArbiter.h \
    chipmunk/chipmunk.h \
    chipmunk/chipmunk_unsafe.h \
    chipmunk/chipmunk_types.h \
    chipmunk/chipmunk_private.h \
    chipmunk/chipmunk_ffi.h \
    chipmunk/constraints/util.h \
    chipmunk/constraints/cpSlideJoint.h \
    chipmunk/constraints/cpSimpleMotor.h \
    chipmunk/constraints/cpRotaryLimitJoint.h \
    chipmunk/constraints/cpRatchetJoint.h \
    chipmunk/constraints/cpPivotJoint.h \
    chipmunk/constraints/cpPinJoint.h \
    chipmunk/constraints/cpGrooveJoint.h \
    chipmunk/constraints/cpGearJoint.h \
    chipmunk/constraints/cpDampedSpring.h \
    chipmunk/constraints/cpDampedRotarySpring.h \
    chipmunk/constraints/cpConstraint.h

INCLUDEPATH +=chipmunk/

QMAKE_CFLAGS += -std=gnu99

