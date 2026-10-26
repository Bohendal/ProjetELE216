#include "vecteur_gen.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

// Définition de la structure dans le .c.
// La structure est encapsulée et il est impossible d'accéder aux champs de
// l'extérieur de ce module. Il est nécessaire de passer par les fonctions
// déclarés dans l'interface (vecteur.h).
struct vecteur {
    int capacite;           // L'espace de stockage actuel
    int taille;             // L'espace utilisé
    void **contenu;         // Pointeur vers les données (un tableau de pointeurs)
};

static bool agrandir(vecteur_t* vect);

vecteur_t *vecteur_creer(int taille_initiale) {
    if (taille_initiale <= 0) { return NULL; }
    
    vecteur_t *vect = malloc(sizeof(vecteur_t));
    if (vect == NULL) { return NULL; }

    void** contenu_initial = malloc(taille_initiale * sizeof(void*));
    if (contenu_initial == NULL) { 
        free(vect);
        return NULL; 
    }

    vect->capacite = taille_initiale;
    vect->taille = 0;
    vect->contenu = contenu_initial;
    return vect;
}

int vecteur_ajouter(vecteur_t* vect, void *valeur) {
    assert(vect   != NULL);
    //assert(valeur != NULL);
    if (vect->taille == vect->capacite) {
        if(agrandir(vect) == false) {return 0;}
    }
    vect->contenu[vect->taille] = valeur;
    vect->taille++;
    return vect->taille;
}

/**
 * Agrandis le tableau pour permettre l'insertion. Double la taille du
 * stockage alloué au tableau. Si l'agrandissement est impossible, cette
 * fonction n'a pas d'effet sur le contenu du tableau.
 *
 * @param vect pointeur non-nul vers le tableau dynamique.
 * @return true si l'agrandissement est effectué avec succès, false sinon.
 */
static bool agrandir(vecteur_t* vect) {
    assert(vect != NULL);
    void **nouveau = realloc(vect->contenu, vect->capacite * sizeof(void *) * 2);
    if (nouveau == NULL) { return false; }
    vect->contenu = nouveau;
    vect->capacite *= 2;
    return true;
}

void *vecteur_obtenir(vecteur_t *vect, int position) {
    assert(vect != NULL);
    assert(position >= 0 && position < vect->taille);
    return vect->contenu[position];
}

void *vecteur_retirer_dernier(vecteur_t *vect) {
    assert(vect != NULL);
    vect->taille--;
    return vect->contenu[vect->taille];
}

void *vecteur_retirer_premier(vecteur_t *vect) {
    assert(vect != NULL);
    void *premier = vect->contenu[0];
    vect->taille--;
    for (int i = 0; i < vect->taille; i++) {
        vect->contenu[i] = vect->contenu[i+1];
    }
    return premier;
}


void vecteur_liberer(vecteur_t* vect) {
    assert(vect != NULL);

    // La libération des éléments est la responsabilité de l'appelant.
    free(vect->contenu);
    free(vect);
}

void vecteur_afficher(vecteur_t* vect) {
    assert(vect != NULL);
    printf("taille : %d, nb_elements : %d\n", vect->capacite, vect->taille);
    printf("{ ");
    for (int i = 0; i < vect->taille; i++) {
        printf("%p, ", vect->contenu[i]);
    }
    printf(" }\n");
}

bool vecteur_modifier(vecteur_t* tableau, size_t indice, void *element) {
    assert(tableau != NULL);
    if (indice >= tableau->taille) {
        return false;
    }
    tableau->contenu[indice] = element;
    return true;
}

// Accesseurs
int vecteur_taille(vecteur_t* vect) { 
    assert(vect != NULL);
    return vect->taille;
}

bool vecteur_est_vide(vecteur_t *vect) {
    assert(vect != NULL);
    return vect->taille == 0;
}

int vecteur_capacite(vecteur_t* vect) {
    assert(vect != NULL);
    return vect->capacite;
}

