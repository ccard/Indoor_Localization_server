#Variables
GCC = g++
HEAD = ./Localization_CodeRedizine
CXXFLAGS += -I$(HEAD)
DEBUGFLAGS = -Wall -Wextra -ggdb3
DEBUG = 
EXEC = Localize
EXDEBUG = $(EXEC).d
BIN = ./bin
SRC = ./Localization_CodeRedizine
IMGCONTAINER = $(BIN)/ImageContainer.o $(BIN)/DBImage.o $(BIN)/MyMat.o
PROVIDER = $(BIN)/ImageProvider.o $(BIN)/DBProvider.o $(BIN)/MatProvider.o
MATCHER = $(BIN)/Matcher.o $(BIN)/LSHMatching.o $(BIN)/MapMatching.o
UTIL = $(BIN)/MyDMatch.o
MANAGER = $(BIN)/LocalizationManager.o $(BIN)/MapBuilder.o
OBJECTS = $(BIN)/Localization_CodeRedizine.o $(IMGCONTAINER) $(PROVIDER) $(UTIL) $(MATCHER) $(MANAGER)

#Compile all target
all: $(EXEC)

#Compile debug target
debug: DEBUG += $(DEBUGFLAGS)
debug: $(EXDEBUG)

$(EXDEBUG): $(OBJECTS)
	$(GCC) $(DEBUG) $(CXXFLAGS) $^ -o $@

$(EXEC): $(OBJECTS)
	$(GCC) $(DEBUG) $(CXXFLAGS) $^ -o $@

$(BIN)/Localization_CodeRedizine.o: $(SRC)/Localization_CodeRedizine.cpp $(IMGCONTAINER) $(PROVIDER) $(UTIL) $(MATCHER) $(MANAGER)
	$(GCC) $(DEBUG) -c $(CXXFLAGS) $^ -o $@





$(BIN)/ImageContainer.o: $(BIN)/ImageContainer.cpp
	$(GCC) $(DEBUG) -c $(CXXFLAGS) $^ -o $@