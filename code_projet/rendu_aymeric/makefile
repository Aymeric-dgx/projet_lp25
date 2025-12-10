# Compilateur et flags
CC = gcc
CFLAGS = -Wall -Wextra -pthread -lncurses -g

# Nom de l'exécutable final
TARGET = monitor

# Règle par défaut : construit l'exécutable
all: $(TARGET)




# Link final : crée l'exécutable à partir des objets
$(TARGET): manage.o local.o ui.o
	$(CC) -pthread -g -o monitor manage.o local.o ui.o -lncurses
	

# Compilation individuelle de chaque fichier
manage.o: manage.c header.h
	$(CC) $(CFLAGS) -c manage.c -o manage.o

local.o: local.c header.h
	$(CC) $(CFLAGS) -c local.c -o local.o

ui.o: ui.c header.h
	$(CC) $(CFLAGS) -c ui.c -o ui.o

# Nettoyage
clean:
	rm -f *.o $(TARGET)
