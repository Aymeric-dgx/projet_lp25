# Compilateur
CC = gcc

# Options de compilation
CFLAGS = -Wall -g

# Bibliothèques
LIBS = -lncurses -pthread -lssh

# Exécutable
EXEC = main

# Fichiers
SRC = manage.c local.c network.c ui.c
OBJ = manage.o local.o network.o ui.o
HEADER = header.h


# Règle par défaut
all: $(EXEC)

# Création de l'exécutable
$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LIBS)


# Compilation des fichiers sources (dépendent du header)
manage.o: manage.c $(HEADER)
	$(CC) $(CFLAGS) -c manage.c


local.o: local.c $(HEADER)
	$(CC) $(CFLAGS) -c local.c


network.o: network.c $(HEADER)
	$(CC) $(CFLAGS) -c network.c


ui.o: ui.c $(HEADER)
	$(CC) $(CFLAGS) -c ui.c



# Nettoyage
clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(EXEC)

re: fclean all
