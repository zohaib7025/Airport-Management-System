CC = gcc
CFLAGS = -O2 -Wall -I./SQLite

# Windows does NOT use pthread or dl
LIBS = -lm

# SQLite paths
SQLITE_SRC = SQLite/sqlite3.c
SQLITE_OBJ = SQLite/sqlite3.o

# Compile SQLite once
$(SQLITE_OBJ): $(SQLITE_SRC)
	$(CC) -c $< -o $@ $(CFLAGS)

# Build executable from any .c file
%: %.c $(SQLITE_OBJ)
	$(CC) $< $(SQLITE_OBJ) -o $@ $(CFLAGS) $(LIBS)

# Clean build files
clean:
	del /Q *.exe 2>nul
	del /Q SQLite\sqlite3.o 2>nul
