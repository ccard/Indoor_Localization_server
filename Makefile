#Variables
GCC = g++
HEAD = ./Localization_CodeRedizine
LIB = `pkg-config --libs opencv`
CXXFLAGS = -I$(HEAD) `pkg-config --cflags opencv` $(LIB)
DEBUGFLAGS = -Wall -Wextra -ggdb3
DEBUG = 
EXEC = Localize
EXDEBUG = $(EXEC).d
BIN = ./bin
SRC = ./Localization_CodeRedizine
IMGCONTAINER = $(BIN)/ImageContainer.o $(BIN)/DBImage.o $(BIN)/MyMat.o
PROVIDER = $(BIN)/DBProvider.o $(BIN)/MatProvider.o
MATCHER = $(BIN)/Matcher.o $(BIN)/LSHMatching.o $(BIN)/MapMatching.o
UTIL = $(BIN)/MyDMatch.o $(BIN)/MyLine.o
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

$(BIN)/MapBuilder.o: $(SRC)/MapBuilder.cpp $(IMGCONTAINER) $(PROVIDER) $(UTIL) $(MATCHER)
	$(GCC) $(DEBUG) -c $(CXXFLAGS) $^ -o $@

$(BIN)/LocalizationManager.o: $(SRC)/LocalizationManager.cpp $(IMGCONTAINER) $(PROVIDER) $(UTIL) $(MATCHER)
	$(GCC) $(DEBUG) -c $(CXXFLAGS) $^ -o $@

$(BIN)/MapMatching.o: $(SRC)/MapMatching.cpp $(IMGCONTAINER) $(PROVIDER) $(UTIL) $(BIN)/Matcher.o
	$(GCC) $(DEBUG) -c $(CXXFLAGS) $^ -o $@

$(BIN)/LSHMatching.o: $(SRC)/LSHMatching.cpp $(IMGCONTAINER) $(PROVIDER) $(UTIL) $(BIN)/Matcher.o
	$(GCC) $(DEBUG) -c $(CXXFLAGS) $^ -o $@

$(BIN)/Matcher.o: $(SRC)/Matcher.cpp $(IMGCONTAINER) $(PROVIDER) $(UTIL)
	$(GCC) $(DEBUG) -c $(CXXFLAGS) $^ -o $@

$(BIN)/MatProvider.o: $(SRC)/MatProvider.cpp $(IMGCONTAINER)
	$(GCC) $(DEBUG) -c $(CXXFLAGS) $^ -o $@

$(BIN)/DBProvider.o: $(SRC)/DBProvider.cpp $(IMGCONTAINER)
	$(GCC) $(DEBUG) -c $(CXXFLAGS) $^ -o $@

$(BIN)/MyDMatch.o: $(SRC)/MyDMatch.cpp
	$(GCC) $(DEBUG) -c $(CXXFLAGS) $^ -o $@

$(BIN)/MyMat.o: $(SRC)/MyMat.cpp $(BIN)/ImageContainer.o $(BIN)/MyLine.o
	$(GCC) $(DEBUG) -c $(CXXFLAGS) $^ -o $@

$(BIN)/DBImage.o: $(SRC)/DBImage.cpp $(BIN)/ImageContainer.o
	$(GCC) $(DEBUG) -c $(CXXFLAGS) $^ -o $@

$(BIN)/ImageContainer.o: $(SRC)/ImageContainer.cpp
	$(GCC) $(DEBUG) -c $(CXXFLAGS) $^ -o $@

$(BIN)/MyLine.o: $(SRC)/MyLine.cpp
	$(GCC) $(DEBUG) -c $(CXXFLAGS) $^ -o $@