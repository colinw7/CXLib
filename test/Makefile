CC = g++
RM = rm

CDEBUG = -g
LDEBUG = -g

INC_DIR = ../include
OBJ_DIR = .
LIB_DIR = ../lib
BIN_DIR = ../bin

all: $(BIN_DIR)/CXRootImage

SRC = \
CXRootImage.cpp \

OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))

LIBS = \
-lCXLib -lCConfig -lCImageLib -lCFont -lCTimer -lCArgs \
-lCFile -lCUtil -lCOS -lCStrUtil -lXt -lX11 -lpng -ljpeg

CPPFLAGS = \
-I$(INC_DIR) \
-I../../CXLib/include \
-I../../CImageLib/include \
-I../../CFont/include \
-I../../CArgs/include \
-I../../CFile/include \
-I../../COS/include \
-I../../CStrUtil/include \
-I../../CMath/include \
-I../../CUtil/include \
-I../../CTimer/include \
-I.

LFLAGS = \
-L$(LIB_DIR) \
-L../../CXLib/lib \
-L../../CImageLib/lib \
-L../../CFont/lib \
-L../../CArgs/lib \
-L../../CFile/lib \
-L../../COS/lib \
-L../../CStrUtil/lib \
-L../../CConfig/lib \
-L../../CUtil/lib \
-L../../CTimer/lib \

clean:
	$(RM) -f $(OBJ_DIR)/*.o
	$(RM) -f $(BIN_DIR)/CXRootImage

.SUFFIXES: .cpp

.cpp.o:
	$(CC) -c $< -o $(OBJ_DIR)/$*.o $(CPPFLAGS)

$(BIN_DIR)/CXRootImage: $(OBJS) $(LIB_DIR)/libCXLib.a
	$(CC) $(LDEBUG) -o $(BIN_DIR)/CXRootImage $(OBJS) $(LFLAGS) $(LIBS)
