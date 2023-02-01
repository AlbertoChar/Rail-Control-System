# Compiler to use
CC = gcc

# Directories
BIN_DIR = bin
INCL_DIR = include
SRC_DIR = src
OBJ_DIR = obj

# Compiler flags
INCL_FLAG = $(addprefix -I,$(INCL_DIR)) # Include directories
CFLAGS = $(INCL_FLAG) -MMD -MP -g # Compiler flags

# Paths to source files and object files
SRCS := $(shell find $(SRC_DIR) -name '*.c') # Find all source files in the src directory
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o) # Create a list of object files from the list of source files
DEPS := $(OBJS:.o=.d) # Create a list of dependency files from the list of object files

# Files
MAIN_BIN = SOProj # Main executable
PTRENI_BIN = padre_treni # padre_treni executable
REG_BIN = registro # registro executable
TRENO_BIN = treno # treno executable
RBC_BIN = rbc # rbc executable

# Object files
_MAIN_OBJS = main includeFunctions log map signal  # Object files for the main executable
MAIN_OBJS := $(_MAIN_OBJS:%=$(OBJ_DIR)/%.o) # Convert object file names to paths
_PTRENI_OBJS = padre_treni includeFunctions log map signal # Object files for the padre_treni executable
PTRENI_OBJS := $(_PTRENI_OBJS:%=$(OBJ_DIR)/%.o)   # Convert object file names to paths
_RBC_OBJS = rbc includeFunctions log map signal # Object files for the rbc executable
RBC_OBJS := $(_RBC_OBJS:%=$(OBJ_DIR)/%.o)           # Convert object file names to paths
_REG_OBJS = registro includeFunctions log map signal  # Object files for the registro executable
REG_OBJS := $(_REG_OBJS:%=$(OBJ_DIR)/%.o)           # Convert object file names to paths
_TRENO_OBJS = treno includeFunctions log map signal # Object files for the treno executable
TRENO_OBJS := $(_TRENO_OBJS:%=$(OBJ_DIR)/%.o)       # Convert object file names to paths

# Phony targets
.PHONY: clean

# Make
# Exec files
all: $(BIN_DIR)/$(MAIN_BIN) \
$(BIN_DIR)/$(PTRENI_BIN) \
$(BIN_DIR)/$(REG_BIN) \
$(BIN_DIR)/$(TRENO_BIN) \
$(BIN_DIR)/$(RBC_BIN)

# Exec proj
$(BIN_DIR)/$(MAIN_BIN): $(MAIN_OBJS)
	mkdir -p $(dir $@) # Create directories if they do not exist
	$(CC) $(MAIN_OBJS) -o $@ $(LINK_FLAG) # Link object files and generate the main executable
# Exec treno
$(BIN_DIR)/$(TRENO_BIN): $(TRENO_OBJS)
	mkdir -p $(dir $@) # Create directories if they do not exist
	$(CC) $(TRENO_OBJS) -o $@ $(LINK_FLAG) # Link object files and generate the treno executable
# Exec rbc
$(BIN_DIR)/$(RBC_BIN): $(RBC_OBJS)
	mkdir -p $(dir $@) # Create directories if they do not exist
	$(CC) $(RBC_OBJS) -o $@ $(LINK_FLAG) # Link object files and generate the rbc executable
# Exec registro
$(BIN_DIR)/$(REG_BIN): $(REG_OBJS)
	mkdir -p $(dir $@) # Create directories if they do not exist
	$(CC) $(REG_OBJS) -o $@ $(LINK_FLAG) # Link object files and generate the registro executable
# Exec padre treni
$(BIN_DIR)/$(PTRENI_BIN): $(PTRENI_OBJS)
	mkdir -p $(dir $@) # Create directories if they do not exist
	$(CC) $(PTRENI_OBJS) -o $@ $(LINK_FLAG) # Link object files and generate the padre_treni executable

# Compilation
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@) # Create directories if they do not exist
	$(CC) $(CFLAGS) -c $< -o $@ # Compile the source file and generate an object file

# Make clean
clean:
	rm -rf bin obj log /tmp/MA*.txt /tmp/rbc_server /tmp/registroPipe* # Remove directories and files

-include $(DEPS) # Include dependency files


