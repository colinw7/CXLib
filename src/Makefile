CC = g++
AR = ar
RM = rm

CDEBUG = -g
LDEBUG = -g

INC_DIR = ../include
OBJ_DIR = ../obj
LIB_DIR = ../lib

all: $(LIB_DIR)/libCXLib.a

SRC = \
CXAtom.cpp \
CXColor.cpp \
CXCursor.cpp \
CXDragWindow.cpp \
CXDrawable.cpp \
CXFont.cpp \
CXGraphics.cpp \
CXImage.cpp \
CXMachine.cpp \
CXNamedEvent.cpp \
CXPixmap.cpp \
CXScreen.cpp \
CXTimer.cpp \
CXtTimer.cpp \
CXUtil.cpp \
CXWindow.cpp \
CXrtFont.cpp \
CWindow.cpp \

OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))

CPPFLAGS = \
-I$(INC_DIR) \
-I../../CImageLib/include \
-I../../CFont/include \
-I../../CConfig/include \
-I../../COS/include \
-I../../CFile/include \
-I../../CStrUtil/include \
-I../../CMath/include \
-I../../CUtil/include \
-I.

clean:
	$(RM) -f $(OBJ_DIR)/*.o
	$(RM) -f $(LIB_DIR)/libCXLib.a

$(OBJS): $(OBJ_DIR)/%.o: %.cpp
	$(CC) $(CDEBUG) -c $< -o $(OBJ_DIR)/$*.o $(CPPFLAGS)

.SUFFIXES: .cpp

$(LIB_DIR)/libCXLib.a: $(OBJS)
	$(AR) crv $(LIB_DIR)/libCXLib.a $(OBJS)
