#ifndef PFD_H
#define PFD_H


// ==================================
		String version_PFD_h="36.4";
// ==================================

#include <stdint.h>


#include <TFT_eSPI.h> // Hardware-specific library
#include "SPI.h"


TFT_eSPI TFT480 = TFT_eSPI(); // Configurer le fichier User_Setup.h de la bibliothèque TFT480_eSPI au préalable


/** ***********************************************************************************
	CLASS VOYANT  // sorte de LED qui peut s'allumer de différentes couleurs et afficher un mot				
***************************************************************************************/


class VOYANT
{
public:

	uint16_t x0;
	uint16_t y0;
	uint16_t dx;
	uint16_t dy;

	uint16_t couleur_texte;
	uint16_t couleur_fond;
	char caract1;
	char caract2;

	VOYANT();
	
	void init(uint16_t x, uint16_t y, uint16_t dx, uint16_t dy);
	void affiche(uint16_t couleur_texte_i, uint16_t couleur_fond_i);
	
private:


};


/** ***********************************************************************************
		CLASS Led  // affiche un petit rectangle rectangle coloré
***************************************************************************************/

class Led
{
public:

	uint16_t x0; //position
	uint16_t y0;
	
	uint16_t dx; //dimension
	uint16_t dy;


//couleurs
	uint16_t couleur;
	
	Led();
	
	void init(uint16_t x, uint16_t y, uint16_t dx, uint16_t dy);
	void set_couleur(uint16_t coul_i);
	void allume();
	void eteint();

	
private:


};


#endif
