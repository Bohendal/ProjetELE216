NAME    = test_image_bmp
CC      = gcc
CFLAGS  = -Wall -Wextra -g -std=gnu11
INCLUDES = -I ./ppm2bmp -I ../unity
LDLIBS  = -lm
LDFLAGS =

# Définition des liste de fichiers 
# (SRC = liste des sources, OBJ = liste des objets, DEP = liste des fichiers de dépendance)
SRC = $(NAME).c image_bmp.c \
			./ppm2bmp/imagebmp.c \
			./ppm2bmp/imageppm.c \
			./ppm2bmp/vecteur_gen.c \
			../unity/unity.c
OBJ = $(SRC:.c=.o)
DEP = $(OBJ:.o=.d)

# Le target .PHONY est utilisé pour indiquer à make que ces termes ne sont pas des
# fichiers sur le disque. Empeche la collision avec un fichier appellé 'run' par exemple
.PHONY: all clean run distclean

# Le target all est le 'top-level' qui lance tout le processus
# C'est la cible par défaut, entrer seulement 'make' est équivalent de 'make all'
all: $(NAME)

# Étape d'édition de lien. Assemble tous les .o en un seul exécutable à l'aide du linker
# ($^ :  liste des prérequis, par exemple (main.o engine.o io.o))
# ($@ : nom du fichier executable cible, par exemple "mon_projet")
$(NAME): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# Étape de compilation. Génère un .o pour chaque .c
# -MMD -MP génère un fichier de dépendance .d pour chaque .o,
# pour que l'édition d'un .h entraîne une recompilation de tous les fichiers qui l'utilisent.
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

# Lance le programme compilé
run: all
	./$(NAME)

# Élimine les produits de compilation intermédiaire (.o, .d)
# conserve l'exécutable produit.
clean:
	rm -f $(OBJ) $(DEP)

# Élimine tous les produits de compilation et l'exécutable, laissant le source intact
distclean: clean
	rm -f $(NAME)

# Inclusion des fichiers de dépendance
-include $(DEP)
