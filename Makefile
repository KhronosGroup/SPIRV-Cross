TARGET := spirv-cross
SOURCES := $(wildcard *.cpp)
OBJECTS := $(SOURCES:.cpp=.o)
DEPS := $(OBJECTS:.o=.d)

CXXFLAGS += -std=c++11 -Wall -Wextra

ifeq ($(DEBUG), 1)
	CXXFLAGS += -O0 -gdwarf-2
else
	CXXFLAGS += -O2 -gdwarf-2
endif

all: $(TARGET)

-include $(DEPS)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS) -MMD

clean:
	rm -f $(TARGET) $(OBJECTS) $(DEPS)

.PHONY: clean
