CXX = g++
CXXFLAGS = -I./include -I../include -std=c++11 -fPIC
LDFLAGS = -shared
SRCS = core.cpp decrypt.cpp encrypt.cpp hash.cpp permutations.cpp backup.cpp
OBJS = $(SRCS:.cpp=.o)

TARGET_LIB = libcryptography.so

# Default target
all: $(TARGET_LIB)

# Build the shared library
$(TARGET_LIB): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

# Compile each .cpp file to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up object files and the shared library
clean:
	rm -f $(OBJS) $(TARGET_LIB)

# Build the decrypt_binary executable
decrypt_binary: decrypt.cpp
	$(CXX) $(CXXFLAGS) decrypt.cpp -o decrypt_binary -L. -lcryptography

# decrypt_p_binary: decrypt_process.cpp
# 	$(CXX) $(CXXFLAGS) decrypt_process.cpp -o decrypt_p_binary -L. -lcryptography
