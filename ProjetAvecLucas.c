#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>
//#include "lib/lib.h"

#define L 150

//structures nécessaires à la création de l'arbre
struct noeud
{
	 char fichier[FILENAME_MAX];
	 struct noeud * gauche;
	 struct noeud * droit;
};
typedef struct noeud * arbre;

//sépare une chaîne de caractères en deux avec pour séparateur "="
int parse(char *s, char *root, char *chemin)
{
	return sscanf(s,"%[^=]=%s",root,chemin)==2;
}

//parcours d'affichage préfixe d'un arbre
void prefixe(arbre A)
{
	if(A!=NULL)
	{
		printf("%s\n",A->fichier);
		prefixe(A->gauche);
		prefixe(A->droit);
	}
}

//fonction créant automatiquement un arbre avec pour racine le chemin donné
arbre creationArbre (char * chemin)
{
	arbre A = NULL;
	A = malloc(sizeof(struct noeud));
	strcpy(A->fichier,chemin);
	A->gauche = NULL;
	A->droit = NULL;
	return A;
}


//on définit les fils (=contenu) d'un répertoire à sa droite et les frères d'un fichier ou d'un répertoire à sa gauche dans la fonction load
void load(char * chemin, arbre * A)
{
	DIR * D;
	//nouveau chemin de taille FILENAME_MAX, défini par le système
	char newpath[FILENAME_MAX];

	//ouverture du répertoire
	D = opendir(chemin);
	assert(D!=NULL);

	struct dirent *dir;
	arbre tmpfrere;
	int fils;
	fils=1;

	//lecture du répertoire racine
	while( (dir=readdir(D)) != NULL)
	{
		//si c'est un fils on ajoute à droite
		if (fils==1)
		{
			//si le premier fichier est dossier (et pas dossier parent ou courant)
			if ( (dir->d_type == DT_DIR) && (strcmp(dir->d_name,"..")) && (strcmp(dir->d_name,".")) )
			{
				//on rajoute la racine au dossier trouvé pour la récursivité
				sprintf(newpath,"%s/%s",chemin,dir->d_name);
				//nouveau dossier donc création d'un nouvel arbre
				arbre arbreFils = creationArbre(dir->d_name);
				//on l'ajoute à l'arbre actuel
				(*A)->droit = arbreFils;
				//on le marque pour ajouter ses frères à gauche par la suite
				tmpfrere = arbreFils;


				/*idée de réc pour plus tard
				tmpfils = arbreFils->droit;*/
				load(newpath,&arbreFils);
			}
			//si le premier fichier est un fichier régulier
			else if (dir->d_type == DT_REG )
			{
				//nouveau fichier donc création d'un nouvel arbre
				arbre arbreFils = creationArbre(dir->d_name);
				//on l'ajoute à l'arbre actuel
				(*A)->droit = arbreFils;
				//on le marque pour ajouter ses frères à gauche par la suite
				tmpfrere = arbreFils;
			}
			++fils;
		}
		//sinon c'est un frère on ajoute à gauche
		else
		{
			if ( (dir->d_type == DT_DIR) && (strcmp(dir->d_name,"..")) && (strcmp(dir->d_name,".")) )
			{
				//on rajoute la racine au dossier trouvé pour la récursivité
				sprintf(newpath,"%s/%s",chemin,dir->d_name);
				//nouveau dossier donc création d'un nouvel arbre
				arbre arbreFrere = creationArbre(dir->d_name);
				//on l'ajoute à l'arbre actuel
				tmpfrere->gauche = arbreFrere;
				//on le marque pour ajouter ses éventuels frères à gauche par la suite
				tmpfrere = arbreFrere;

				/*idée de réc pour plus tard
				tmpfils = arbreFrere->droit;*/
				load(newpath,&arbreFrere);
			}
			else if (dir->d_type == DT_REG )
			{
				//nouveau fichier donc création d'un nouvel arbre
				arbre arbreFrere = creationArbre(dir->d_name);
				//on l'ajoute à l'arbre actuel
				tmpfrere->gauche = arbreFrere;
				//on le marque pour ajouter ses frères éventuels à gauche par la suite
				tmpfrere = arbreFrere;
			}

		}
	}
	//fermeture du répertoire
	closedir(D);
}

//permet de récupérer l'arbre chargé en mémoire par load
arbre lancementLoad(char * chemin)
{
	arbre retour;
	retour = malloc(sizeof(struct noeud));
	strcpy(retour->fichier,chemin);
	retour->gauche = NULL;
	retour->droit = NULL;
	load(chemin,&retour);
	return retour;
}

//recherche d'un fichier dans un arbre
void search (char * fichier, arbre A, char * path)
{

	char newpath[FILENAME_MAX];
	//si l'arbre est nul on ne fait rien et on affiche erreur
	if (A==NULL)
	{
		printf("Erreur, arbre nul");
	}
	//si le noeud contient le fichier qu'on cherche on l'affichage précédé de son chemin
	else if (!strcmp(A->fichier,fichier))
	{
		sprintf(newpath,"%s/%s",path,A->fichier);
		printf("Fichier trouvé : %s \n",newpath);
	}
	//récursivité pour la recherche
	else
	{
		//seuls les dossiers ont un fils à droite, on les repère ainsi donc on peut concaténer au chemin path
		if (A->droit != NULL)
		{
			sprintf(newpath,"%s/%s",path,A->fichier);
			search(fichier,A->droit,newpath);
		}
		if (A->gauche != NULL)
		{
			search(fichier,A->gauche,path);
		}
	}
}


/*--------------------------------------------------------*/

int main(int nbarg, char *arg[])
{
	//vérification des paramètres
	if (nbarg != 2)
	{
		printf("un seul argument obligatoire est accepté (le nom du fichier recherché)\n");
		return 1;
	}

	//ouverture du fichier (+ déclaration)

	char s[L];
	FILE * source;
	source = fopen("tree.conf","r");

	//erreur s'il y a un problème à l'ouverture

	if (source==NULL)
	{
		perror("tree.conf");
		return 2;
	}

	//déclaration des variables qui vont récupérer l'arborescence

	char root[10], chemin[100];
	int result;

	//lecture du fichier

	while(fgets(s,L-1,source)!=NULL)
	{
		printf("%s\n",s);
		if ( (s[0]!='#') || (strlen(s)!=1) )
		{
			result=parse(s,root,chemin);
			if(result==1){printf("root='%s', chemin='%s' result=%d\n",root,chemin,result) ;}
		}
	}

	//fermeture du fichier
	fclose(source);

	//déclaration arbre
	arbre ret;
	//mise en mémoire de l'arbre à partir du chemin donné
	ret = lancementLoad(chemin);
	//un affichage pour pouvoir regarder le contenu de l'arbre
	prefixe(ret);

	//on passe un chemin vide en paramètre qui se complètera au fil du parcours pour pouvoir afficher le chemin du fichier trouvé
	char path[FILENAME_MAX];
	//recherche du fichier passé en argument au main dans l'arbre ret
	search(arg[1],ret,path);

	return 0;
}
