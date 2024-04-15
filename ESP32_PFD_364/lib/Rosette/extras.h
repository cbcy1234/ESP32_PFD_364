#include <stdio.h>
#include <stdlib.h>
#include  <math.h>
#include"design.h"
#define r 30


///definition d'un type de variables : info_pion
typedef struct info_pion info_pion;
struct info_pion
   {
    int i;
    int j;
    int player;
   };

static info_pion tab_pions[300];

int isEmpty()
  {
    ///cette fonction retourne 1 si le tableau est vide 0 sinon
    if( tab_pions[0].i == NULL)
        return 1;
    return 0;
  }

void inserer(info_pion ip)
   {
    ///cette fonction insere un pion dans le tableau soit a la fin du tableau
    int k=0;
    while(tab_pions[k].i != NULL)
        { ///test sur le champ i sachant qu'il ne peut
                                ///etre affecté par cett valeur ;-)
        k++;
        }
    tab_pions[k]=ip;
   }
int nombre_pions1()
  {
    int k=0 , reto=0;
    while(tab_pions[k].i != NULL)
    {  if(tab_pions[k].player==1)
       {
        reto++;
       }
        k++;
    }
    return reto;
  }

  int nombre_pions2()
  {
    int k=0 , reto=0;
    while(tab_pions[k].i != NULL)
    {  if(tab_pions[k].player==2)
       {
        reto++;
       }
        k++;
    }
    return reto;
  }


  int nombre_pions()
  {
    int k=0;
    while(tab_pions[k].i != NULL)
    {
         k++;
    }
    return k;
  }
void afficher_pions()
{
    ///affiche l'info complet du pion a savoir ses coord & son player
    int k=0;
    double c = sqrt(3) / 2;
    int yo=getmaxheight()/2-13*c*r,
        xo=getmaxwidth()/2+r/2;
    while(tab_pions[k].i != NULL)
        { if(tab_pions[k].player == 1)
          { pion1(xo+tab_pions[k].i * (r/2) , yo+tab_pions[k].j * c *r);
          }
           else if(tab_pions[k].player == 2)
            {pion2(xo+tab_pions[k].i * (r/2) , yo+tab_pions[k].j * c *r);
            }
        k++;
        }
}
int isExist(info_pion ip)
{
    ///cette fonction verifie l'existence d'un pion (coordonnee ) dans le tableau
    ///elle retourne le player s'il existe 0 sinon
    int k = 0;
    while(tab_pions[k].i != NULL)
        {
        if(tab_pions[k].i == ip.i &&
            tab_pions[k].j == ip.j)
                return tab_pions[k].player;
        k++;
        }
    return 0;
}
int retirer (info_pion ip)
{
    ///enleve un pion du tableau, en le remplacant par le dernier pion enregistré dans
    ///le tableau
    ///elle retourne 1 si l'operation reussie 0 sinon
    int k = 0;
    while(tab_pions[k].i != NULL)
        {
        if(tab_pions[k].i == ip.i &&
            tab_pions[k].j == ip.j &&
                tab_pions[k].player == ip.player)
                {
                    int dernier = nombre_pions();
                    tab_pions[k] = tab_pions[dernier-1];
                    tab_pions [dernier - 1].i =0;
                    tab_pions [dernier - 1].j =0;
                    tab_pions [dernier - 1].player =0;
                    return 1;
                }
        k++;
       }
    return 0;
}
void vider ()
{
    int k = 0;
    while(tab_pions[k].i != NULL)
        {
            tab_pions [k].i = 0;
            tab_pions [k].j = 0;
            tab_pions [k].player = 0;
        k++;
        }
}
