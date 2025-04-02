# Compiler and flags
CC = gcc
CFLAGS = -Wall -lpthread

# Source files and object files
SRC = web_server.c mongoose.c
OBJ = $(SRC:.c=.o)
EXEC = web_server

# Default target: Build the executable
all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Clean the build
clean:
	rm -f $(OBJ) $(EXEC)
	@echo "Build cleaned."

# Rebuild
rebuild: clean all
	@echo "Rebuild completed."

# Run unit tests or other actions (No Test cases for our software as it is bsed on the GUI implementation, it will be easier to test it that way!)
test:
	@echo "Running tests..." 
	#No test cases found to be tested...
