/*
 * Auteurs :
 * BONNIER Antoine
 * MELLOUK Imân
 */

#define _DEFAULT_SOURCE
#define NDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#define TAILLE_MAX 256
#define CC "gcc"
#define CFLAGS "-Wall -ansi -Werror -Wfatal-errors -O2"

/* Affecte le caractère "extension" à la place du dernier caractère de la chaine pointée par "nom_fichier". */
void changer_extension(char *nom_fichier, char extension){
    assert(NULL != nom_fichier);
    assert('\0' != *nom_fichier);

    nom_fichier[strlen(nom_fichier) - 1] = extension;
}

/* Déclare une variable $OBJS contenant la liste des objets. */
int ecrire_objs(FILE *out){
    FILE *fp;
    char nom_fichier[TAILLE_MAX];

    assert(NULL != out);

    fprintf(out, "OBJS =");
    fp = popen("ls *.c", "r");
    if(NULL == fp){
        fprintf(stderr, "Erreur lors de l'ouverture du pipe !\n");
        return 0;
    }
    while(fscanf(fp, "%s", nom_fichier) == 1){
        changer_extension(nom_fichier, 'o');
        fprintf(out, " %s", nom_fichier);
    }
    fprintf(out, "\n");
    pclose(fp);
    return 1;
}

/* Déclare une variable $OUT qui représente le nom de l'exécutable cible. */
void ecrire_out(FILE *out, const char *nom_out){
    assert(NULL != out);
    assert(NULL != nom_out);

    fprintf(out, "OUT = %s\n", nom_out);
}

/* Déclare une variable $CC qui représente le nom du compilateur. Changez la constante "CC" de ce fichier pour changer le compilateur. */
void ecrire_cc(FILE *out){
    assert(NULL != out);

    fprintf(out, "CC = %s\n", CC);
}

/* Déclare une variable $CFLAGS qui représente les drapeaux de compilation choisis. Changez la constante "CFLAGS" de ce fichier pour changer ces drapeaux. */
void ecrire_cflags(FILE *out){
    assert(NULL != out);

    fprintf(out, "CFLAGS = %s\n", CFLAGS);
}

/* Cherche le fichier .h dont le nom est pointé par la chaine "nom_fichier_h" et écrit les éventuels drapeaux nécessaires pour ce fichier. */
int ecrire_ldflags_h(FILE *out, const char *nom_fichier_h){
    FILE *fichier_h;
    char mot_lu[TAILLE_MAX];

    assert(NULL != nom_fichier_h);
    assert(NULL != out);

    fichier_h = fopen(nom_fichier_h, "r");
    if(NULL == fichier_h){
        fprintf(stderr, "Erreur lors de l'ouverture du header \"%s\" !\n", nom_fichier_h);
        return 0;
    }
    while(fscanf(fichier_h, "%s", mot_lu) == 1){
        if(strcmp(mot_lu, "<math.h>") == 0)
            fprintf(out, "-lm ");
        else if(strcmp(mot_lu, "<ncurses.h>") == 0)
            fprintf(out, "-lncurses ");
        else if(strcmp(mot_lu, "<MLV/all.h>") == 0)
            fprintf(out, "-lMLV ");
        else if(strcmp(mot_lu, "<readline\\readline.h>") == 0)
            fprintf(out, "-lreadline ");
    }
    fclose(fichier_h);
    return 1;
}

/* Appelle la fonction ecrire_ldflags_h pour tous les fichiers .h présents dans le répertoire courant. */
int ecrire_ldflags_projet(FILE *out){
    FILE *fp;
    char nom_fichier_h[TAILLE_MAX];

    assert(NULL != out);

    fprintf(out, "LDFLAGS = ");
    fp = popen("ls *.h", "r");
    if(NULL == fp){
        fprintf(stderr, "Erreur lors de l'ouverture du pipe !\n");
        return 0;
    }
    while(fscanf(fp, "%s", nom_fichier_h) == 1){
        if(ecrire_ldflags_h(out, nom_fichier_h) == 0){
            fprintf(stderr, "Erreur lors de l'écriture des ldflags !\n");
            pclose(fp);
            return 0;
        }
    }
    pclose(fp);
    fprintf(out, "\n");
    return 1;
}

/* Ecrit les déclarations de variables du makefile. */
int ecrire_debut(FILE *out, const char *nom_out){
    int ret;

    assert(NULL != out);
    assert(NULL != nom_out);

    ret = ecrire_objs(out);
    ecrire_out(out, nom_out);
    ecrire_cc(out);
    ecrire_cflags(out);
    ret = ret && ecrire_ldflags_projet(out);
    fprintf(out, "\n");
    return ret;
}

/* Ecrit la ligne de compilation finale. */
int ecrire_all(FILE *out){
    assert(NULL != out);

    fprintf(out,"all : $(OBJS)\n");
    fprintf(out,"\t$(CC) $(CFLAGS) $(OBJS) -o $(OUT) $(LDFLAGS)\n");
    fprintf(out, "\n");
  return 1;
}

/* Ecrit la commande de compilation pour le fichier dont le nom est pointé par "nom_fichier" */
void ecrire_ligne_commande(FILE *out, char *nom_fichier){
    assert(NULL != out);
    assert(NULL != nom_fichier);

    changer_extension(nom_fichier, 'o');
    fprintf(out, "%s: ", nom_fichier);
    changer_extension(nom_fichier, 'c');
    fprintf(out, "%s ", nom_fichier);
    changer_extension(nom_fichier, 'h');
    if(access(nom_fichier, 0) == 0)
        fprintf(out, "%s", nom_fichier);
    fprintf(out, "\n");
    changer_extension(nom_fichier, 'c');
    fprintf(out, "\t$(CC) -c $(CFLAGS) %s\n", nom_fichier);
    fprintf(out, "\n");
}

/* Appelle la fonction ecrire_ligne_commande pour chaque fichier présent dans le répertoire courant. */
int ecrire_commandes(FILE *out){
    FILE *fp;
    char nom_fichier_c[TAILLE_MAX];

    assert(NULL != out);

    fp = popen("ls *.c", "r");
    if(NULL == fp){
        fprintf(stderr, "Erreur lors de l'ouverture du pipe !\n");
        return 0;
    }
    while(fscanf(fp, "%s", nom_fichier_c) == 1)
        ecrire_ligne_commande(out, nom_fichier_c);
    pclose(fp);
    return 1;
}

int ecrire_clean(FILE *out){
    assert(NULL != out);

    fprintf(out, "\nclean:\n");
    fprintf(out, "\trm -f $(OBJS) $(OUT)\n");
    return 1;
}

int main(int argc, const char **argv){
    FILE *out;
    int ret;

    if(2 != argc){
        fprintf(stderr, "Syntaxe incorrecte ! RTFM !!\n");
        return 1;
    }
    out = fopen("makefile", "w");
    if(NULL == out){
        fprintf(stderr, "Erreur lors de la création du fichier de sortie !\n");
        return 1;
    }
    system("rm genere_makefile.c");
    ret = ecrire_debut(out, argv[1]);
    ecrire_all(out);
    ret = ret && ecrire_commandes(out);
    ecrire_clean(out);
    fclose(out);
    return ret;
}
