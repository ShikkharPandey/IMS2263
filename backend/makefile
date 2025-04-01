# Compiler and flags
CC = gcc
CFLAGS = -Wall -lpthread

# Source files and object files
SRC = web_server.c mongoose.c
OBJ = $(SRC:.c=.o)
EXEC = web_server

# Include directories (if mongoose.h or other headers are in a specific folder)
INCLUDE_DIRS = -I./

# Default target: Build the executable
all: $(EXEC)

# Link the object files to create the executable
$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

# Compile .c files into .o object files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Clean the build
clean:
	rm -f $(OBJ) $(EXEC)
	@echo "Build cleaned."

# Rebuild everything from scratch
rebuild: clean all
	@echo "Rebuild completed."

# Optional: Run unit tests or other actions (adjust based on your testing setup)
test:
	@echo "Running tests..." 
	# Example test command: ./run_tests.sh or your test script
	# If no tests exist, leave this section as a placeholder

