#include <Arduino.h>
//
// ==================================
	String version="36.4";
// ==================================

/*
	PFD.ino  - Primary Flight Display pour Flightgear et ESP32 - version Afficheur TFT 480x320
	Ne concerne pas un avion réel ! (ni ULM...)
	N'est pas destiné à un quelconque apprentissage du pilotage d'un avion réel (les instruments sont une vue d'artiste)
	l'artiste c'est moi !
	Les appellations des modules (SD, MCDU) ne correspondent pas non plus exactement à la réalité.
	Mais rien ne vous interdit de modifier tout ça le cas échéant, vous avez le code source.
	Fonctionne avec le simulateur FlightGear sous Linux et avec l'avion Citation X (mon préféré!)
	Les autres avions ont un autopilot différent et donc une "Property tree" différente, il faudrait adapter le programme
	- en particulier ne fonctionne pas tel-quel avec les B7xx ni les A3xx, à vous de jouer !

	par Silicium628
	
*/
/**---------------------------------------------------------------------------------------	
	Logiciel libre et gratuit : Pour les #includes issus de l'univers Arduino (que je ne fournis pas), il faut voir au cas par cas.
	(drivers d'affichage en particulier)

---------------------------------------------------------------------------------------
	De petites images à placer sur la SDcard centrées sur les aérodromes proviennent de OpenStreetMap

	OpenStreetMap® est un ensemble de données ouvertes,
	disponibles sous la licence libre Open Data Commons Open Database License (ODbL)
	accordée par la Fondation OpenStreetMap (OSMF).

	Voir ce lien pour plus de détails :
	https://www.openstreetmap.org/copyright
--------------------------------------------------------------------------------------**/

/*=====================================================================================================
CONCERNANT L'AFFICHAGE TFT : connexion :

( Pensez à configurer le fichier User_Setup.h de la bibliothèque ~/Arduino/libraries/TFT_eSPI/ )

les lignes qui suivent ne sont q'un commentaire pour vous indiquer la config à utiliser
placée ici, elle ne sont pas fonctionnelles
Il FAUT modifier le fichier User_Setup.h installé par le système Arduino dans ~/Arduino/libraries/TFT_eSPI/

// ESP32 pins used for the parallel interface TFT
#define TFT_CS   27  // Chip select control pin
#define TFT_DC   14  // Data Command control pin - must use a pin in the range 0-31
#define TFT_RST  26  // Reset pin

#define TFT_WR   12  // Write strobe control pin - must use a pin in the range 0-31
#define TFT_RD   13

#define TFT_D0   16  // Must use pins in the range 0-31 for the data bus
#define TFT_D1   4  // so a single register write sets/clears all bits
#define TFT_D2   2  // 23
#define TFT_D3   22
#define TFT_D4   21
#define TFT_D5   15 // 19
#define TFT_D6   25 // 18
#define TFT_D7   17
=====================================================================================================

Notes :
- Si les data de FG ne sont pas reçues, il faut vérifier que le PFD est bien connecté sur le port USB0 (et pas USB1 ou autre...)
- Le module PFD doit impérativement être lancé et connecté à l'USB_0 avant de lancer Flightgear et non l'inverse
sinon le dialogue USB avec l'ordinateur ne se fera pas. 
*/
	
// v14 la fonction Autoland() utilise le glide pour la descente et non plus une altitude calculée théorique
// v15.2 modif du fichier hardware4.xml (nav-distance)
// v15.3 revu autoland - possibilité de poser auto si localizer seul (aérodrome non équipé de glide de glide).
// v15.6 nombreuses modifs dans tous les fichiers, dont fichier FG_data.h
// v16.0 prise en charge du module "SW" (boutons poussoirs) par WiFi
// v16.2 prise en charge des 2 nouveaux boutons (target_speed) + le fichier "hardware4.xml" a été modifié en conséquence
// v20.0 autoland possible (par GPS au lieu de l'ILS) même pour aérodromes non équipé ILS
// v21.0 autoland toujours calculé par GPS, avec prise en compte de l'altitude de l'aérodrome
// v22.0 modifs fonction autolang & fichier 'hardware4.xml' (prise en charge du trim de profondeur, voir 'desengage_autoland()'
// v24.2 tous les calculs de position et de direction se font directement avec les coordonnées GPS,
// sans passer par système LAMBERT
// v26.0 prise en compte de la longueur de la piste pour l'autolanding
// v26.2 asservissement (facultatif) de la gouverne de direction et de la roue avant au décollage et à l'atterrissage
// v30.3 le module MCDU associé à ce PFD et au module ND permettent :
//  -un décollage entièrement automatique, avec engagement automatique de l'autopilot et la mise en palier
//  -une route automatique vers un point d'entrée en finale situé à 10NM d'une autre piste choisie pendant le vol
// 	-la gestion de la finale avec pente à 5%, l'arrondi, le touché de roues sur train principal et le guidage en lacet sur la piste
// Un aérodrome m'a posé quelques soucis : LFLB Chambéry. La finale se fait avec un pente nettement plus accentuée,
// au dessus du lac, en évitant d'abimer la montagne ! Le point d'entrée en finale se situe alors
// à 8 NM de la piste depuis une hauteur + importante (voir le fichier FG_data.h)
// v30.9 entré-sortie auto du train d'atterrissage (et modif dans le fichie "hardware4.xml")
// v31.0 gestion des freins (freine automatiquement à l'atterrissage)
// v35.0 Asservissement du roulis (maintient les ailes à plat) lors de phase de décollage et d'atterrissage
// deux modes d'atterrissage : long (=normal) et court (freine à fond + reverses).
// tous les fichiers sont donc affectés (même le fichier hardware4.xml qui comporte de nouvelles entrées)

	
/*  les numéros de version de tous les modules doivent être identiques (le contenu du .zip est cohérent) */


/** un peu de théorie : ****************************

ALTITUDE vs HAUTEUR

Une hauteur est la distance verticale entre un aéronef et la surface qu'il survole (terre ou eau).

Pour exprimer une hauteur, il est défini les hauteurs AGL (Above Ground Level) ou ASFC (Above Surface).
Il s'agit de la hauteur entre l'avion et le sol juste en dessous de sa position. Elle suit donc le relief.

Pour exprimer une hauteur au dessus de l'aérodrome, il est défini la hauteur AAL (Above Aerodrome Level).
Il s'agit de la hauteur entre l'avion et le point de référence de l'aérodrome comme s'il était en dessous de la
position de l'appareil (même s'il n'y est pas). Cette hauteur ne suit pas le relief.

source : https://aeroclub-narbonne.com/download/2017/04/BASE_ALT.pdf
IVAO TM © ELH FLA septembre 2014
je conseille de lire et relire l'ensemble de ce PDF. 

*****************************************************/

#include "PFD.h"
#include <stdint.h>

#include <TFT_eSPI.h> // Hardware-specific library

#include "Free_Fonts.h"

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include "FG_data.h"
#include "Fonctions1.h"

#include <WiFi.h> // Ce PFD est un serveur WiFi 
#include "ESPAsyncWebServer.h" 


const char* ssid = "PFD_srv"; 
const char* password = "72r4TsJ28";

AsyncWebServer server(80); // Create AsyncWebServer object on port 80

String argument_recu1;
String argument_recu2;
String argument_recu3;

TFT_eSprite SPR_E = TFT_eSprite(&TFT480);  // Declare Sprite object "SPR_11" with pointer to "TFT" object
TFT_eSprite SPR_N = TFT_eSprite(&TFT480);
TFT_eSprite SPR_O = TFT_eSprite(&TFT480);
TFT_eSprite SPR_S = TFT_eSprite(&TFT480);

TFT_eSprite SPR_trajectoire = TFT_eSprite(&TFT480);

VOYANT voyant_L; // Localizer (azimut)
VOYANT voyant_G; // Glide (hauteur)
VOYANT voyant_APP; // Approche auto (= attéro ILS)
VOYANT voyant_route;
VOYANT voyant_RD; // Auto rudder (asservissement de la gouverne de direction (lacet) et de la roulettes de nez au sol)
VOYANT voyant_ATT; // atterrissage en cours

Led Led1;
Led Led2;
Led Led3;
Led Led4;
Led Led5;


uint16_t hauteur_mini_autopilot = 100; // ou 300, ou 500, à tester...


int16_t Ax_actu, Ay_actu; 
int16_t Bx_actu, By_actu;

//position et dimensions de l'horizon artificiel
#define HA_x0 210
#define HA_y0 130
#define HA_w 120 // demi largeur
#define HA_h 100	// demi hauteur

#define x_autopilot 320

// Width and height of sprite
#define SPR_W 14
#define SPR_H 14


// =====================================================================
//mémorisation dex pixels deux lignes H et de deux lignes V
//ce qui permet d'afficher un rectangle mobile sur l'image sans l'abimer

uint16_t data_L1[480]; // pixels d'une ligne Horizontale
uint16_t data_L2[480]; // pixels d'une autre ligne Horizontale
uint16_t data_C1[320]; // pixels d'une ligne Verticale ('C' comme colonne)
uint16_t data_C2[320]; // pixels d'une autre ligne Verticale

uint16_t x_1;  // position reçu du module positionneur_XY
uint16_t x_2;  // position reçu du module positionneur_XY
uint16_t y_1;
uint16_t y_2;

uint16_t memo_x1;
uint16_t memo_y1; // position de la ligne
uint16_t memo_x2;
uint16_t memo_y2;

// =====================================================================


uint32_t memo_micros = 0;
uint32_t temps_ecoule;
uint16_t nb_secondes=0;
uint8_t nb_acqui;

String parametre; //garde en mémoire les données reçues par USB entre les passages dans la fonction "void acquisitions()"

uint8_t landing_light1=0;
uint8_t landing_light2=0;

float roulis;
float tangage;


float altitude_GPS_float; 
int32_t altitude_GPS; // accepte les valeurs négatives (par exemple si QNH mal réglé avant décollage)
int32_t hauteur_AAL; // (Above Aerodrome Level)

#define ASL 0
#define AAL 1

uint8_t mode_affi_hauteur = ASL;

int32_t gnd_elv; // feet ; // [hauteur de la surface du terrain]/mer situé sous l'avion
int32_t alti_agl; // feet ; hauteur de l'avion par rapport au terrain (pas la piste, le relief !) situé au dessous de lui
int32_t vitesse; // kts
int32_t memo_vitesse;
int16_t target_speed =180; // consigne de vitesse pour l'autopilot
int16_t dV;
int16_t acceleration;
int16_t vspeed;		// vitesse verticale

float cap; 		// en degrés d'angle; direction actuelle du nez de l'avion

int16_t hdg1  = 150; // en degrés d'angle; consigne cap = Heading (HDG) Bug // PROBLEME: imprécision de 1° -> trop imprécis !!!
int16_t memo_hdg1;
uint8_t flag_refresh_hdg=0;
uint8_t flag_traiter_SW=0;
uint8_t flag_traiter_MCDU=0;


float lat_avion;  // WGS84
float lon_avion; // WGS84

float px_par_km;
float px_par_NM;


// Les points ptAA et ptBB sont les points d'insertion en finale, situés à 15NM de chaque côté dans l'axe de la piste
// Leurs coordonnées sont calculées en fonction de celles de la piste
// ( voir "liste_bali[n].dst_pt_AB" dans le fichier FG_data.h  )

float lat_ptA;
float lon_ptA;

float lat_ptB;
float lon_ptB;

float lon_ptAA;
float lat_ptAA;

float lon_ptBB;
float lat_ptBB;

float lon_pti;
float lat_pti;

float GPS_distance_piste;  // en NM
float memo_GPS_distance_piste; // servira à savoir si on se rapproche du centre de la piste ou si on s'en éloigne après
// l'avoir dépassé (lors du décollage)

float GPS_distance_ptAA; // point situé à 10 ou 15 NM dans l'axe de la piste pour amorcer l'approche
float GPS_distance_ptBB; // point situé à 10 ou 15 NM dans l'axe de la piste, dans l'autre sens
float GPS_distance_pti;  // point quelconque

float erreur_axe=0;


#define sens_AB 0
#define sens_BA 1
uint8_t sens_app_effectif; // effectif pour l'attero (ne concerne pas le décollage)

float lat_centre_pst;
float lon_centre_pst;
float longueur_piste;

float orient_pisteAB;
float orient_pisteBA;

float GPS_azimut_piste;
float GPS_azimut_ptA;
float GPS_azimut_ptB;
float GPS_azimut_ptAA;
float GPS_azimut_ptBB;
float GPS_azimut_pti;


char extremite_pst ='X'; // le bout le plus éloigné lors de l'approche, = 'A' ou 'B' sert aussi au décollage (au roulage)
// ce paramètre est calculé en fonction de la position réelle de l'avion lors de la prise de décision

uint8_t choix_aleatoire;

int16_t asel1 = 30; // consigne altitude ('niveau de vol' en centaines de pieds) 30 -> 3000ft  (ASL)
float climb_rate=0; // taux de montée (négatif pour descendre - sert pour attérissage automatique)

float joystick1; // valeur reçue de Flightgear par la liaison USB (lue dans le properties tree de FG)
float trim_elevator;
float elevator; // valeur à envoyer à FG, qui fixera la position de la gouverne de profondeur (val <0 pour monter)
float throttle;
bool reverser1=0;
bool reverser2=0;
int8_t flaps=0; // 0..4
float speedbrake=0; // 0 = rentré, 1 = sorti
bool  gear_down=1;
float brake_left;
float brake_right;

float ailerons=0;
float rudder=0;
float rudder_manuel; // fonction directe du potentiomètre

uint8_t view_number=0;

String locks_type;  // "ALT" ou "VS"
String AP_status;  // "" ou "AP"  permet d'engager ou de désengager l'autopilot de FlightGear
bool speed_ctrl;

int16_t num_bali=0;
int16_t memo_num_bali=0; 

uint8_t  flag_SDcardOk=0;
uint32_t data_ok=0; // ce n'est pas un flag
//uint8_t gs_ok=0;
uint8_t QNH_ok=0;

uint8_t flag_1er_passage =1;
uint8_t flag_att_cnx_usb=1;
uint8_t attente_data=1;
uint8_t inc_num_pt1_autorisee=1;

int16_t loc=0; 		// localizer
int16_t memo_loc=0;

float memo_R2;
int16_t memo_y0;

uint16_t memo_x_avion=0; // pour fonction "affi_approche()"
uint16_t memo_y_avion=0;

const int bouton1 = 36;  // attention: le GPIO 36 n'a pas de R de pullup interne, il faut en câbler une (10k) au +3V3
bool bouton1_etat;
bool memo_bouton1_etat;

const int bouton2 = 39;  // attention: le GPIO 39 n'a pas de R de pullup interne, il faut en câbler une (10k) au +3V3
bool bouton2_etat;
bool memo_bouton2_etat;

String switches; // boutons connectés au 3eme ESP32 (SW), reçus par WiFi
uint16_t v_switches=0;
uint16_t memo_v_switches=0;

String switches_ND; // boutons connectés au 2eme ESP32 (ND), reçus par WiFi
uint16_t v_switches_ND=0;
uint16_t memo_v_switches_ND=0;

String bt_MCDU; // boutons connectés au 4eme ESP32 (MCDU), reçus par WiFi
uint16_t v_bt_MCDU=0;
uint16_t memo_v_bt_MCDU=0;

uint8_t options_route=0;
uint8_t num_pti=1;


String potar1;
int16_t v_potar1=0;  // peut être négatif  -127..+127
float f_potar1=0;

// deux encodeurs rotatifs pas à pas
const int rot1a = 32;  // GPIO32 -> câbler une R au +3V3
const int rot1b = 33;  // GPIO33 -> câbler une R au +3V3

const int rot2a = 35;  // GPIO35 -> câbler une R au +3V3
const int rot2b = 34;  // GPIO34 -> câbler une R au +3V3


//const int led1 = 25; // GPIO15


#define TEMPO 5 // tempo anti-rebond pour l'acquisition des encodeurs rotatifs
volatile uint32_t timer1 = 0;
volatile uint32_t timer2 = 0;

uint16_t compteur1;
int16_t tempo_message; // peut être négatif

uint8_t heures=0;
uint8_t minutes=0;
uint8_t secondes=0;

float v_test1=-1.0;

String WARNING;
String memo_WARNING;


void RAZ_variables()
{
	roulis=0;
	tangage=0;
	altitude_GPS=0;
	gnd_elv=0;
	vitesse=0;
	vspeed=0;		
	cap=0; 		
	memo_hdg1=0;

	loc=0;
	memo_loc=0;
}



/** ***********************************************************************************
		IMAGE.bmp					
***************************************************************************************/

/**
Rappel et décryptage de la fonction Color_To_565 : (elle se trouve dans le fichier LCDWIKI_KBV.cpp)

//Pass 8-bit (each) R,G,B, get back 16-bit packed color

uint16_t Color_To_565(uint8_t r, uint8_t g, uint8_t b)
{
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
}

0xF8 = 11111000
0xFC = 11111100

(r & 0xF8) -> 5 bit de gauche de r (on ignore donc les 3 bits de poids faible)
(g & 0xFC) -> 6 bit de gauche de g (on ignore donc les 2 bits de poids faible)
(b & 0xF8) -> 5 bit de gauche de b (on ignore donc les 3 bits de poids faible)

rrrrr---
gggggg--
bbbbb---

après les décallages on obtient les 16 bits suivants:

rrrrr---========
     gggggg--===
        ===bbbbb

soit après le ou :

rrrrrggggggbbbbb       

calcul de la Fonction inverse : 
RGB565_to_888
**/


void RGB565_to_888(uint16_t color565, uint8_t *R, uint8_t *G, uint8_t *B)
{
	*R=(color565 & 0xFFFFF800) >> 8;
	*G=(color565 & 0x7E0) >> 3;
	*B=(color565 & 0x1F) << 3 ;
}



/** -----------------------------------------------------------------------------------
		CAPTURE D'ECRAN vers SDcard				
/** ----------------------------------------------------------------------------------- */

void write_TFT_on_SDcard() // enregistre le fichier .bmp
{

	//TFT480.setTextColor(VERT, NOIR);
	//TFT480.drawString("CP", 450, 300);
	
	if (flag_SDcardOk==0) {return;}

	String s1;
	uint16_t ys=200;
	TFT480.setFreeFont(FF1);
	TFT480.setTextColor(JAUNE, NOIR);
	
	uint16_t x, y;
	uint16_t color565;
	uint16_t bmp_color;
	uint8_t R, G, B;
	
	if( ! SD.exists("/bmp/capture2.bmp"))
	{
		TFT480.fillRect(0, 0, 480, 320, NOIR); // efface
		TFT480.setTextColor(ROUGE, NOIR);
		TFT480.drawString("NO /bmp/capture2.bmp !", 100, ys);
		delay(300);
		TFT480.fillRect(100, ys, 220, 20, NOIR); // efface
		return;
	}


	File File1 = SD.open("/bmp/capture2.bmp", FILE_WRITE); // ouverture du fichier binaire (vierge) en écriture
	if (File1)
	{
/*
Les images en couleurs réelles BMP888 utilisent 24 bits par pixel: 
Il faut 3 octets pour coder chaque pixel, en respectant l'ordre de l'alternance bleu, vert et rouge.
*/
		uint16_t bmp_offset = 138;
		File1.seek(bmp_offset);

		
		TFT480.setTextColor(VERT, NOIR);;	
		
		for (y=320; y>0; y--)
		{
			for (x=0; x<480; x++)
			{
				color565=TFT480.readPixel(x, y);

				RGB565_to_888(color565, &R, &G, &B);
				
				File1.write(B); //G
				File1.write(G); //R
				File1.write(R); //B
			}

			s1=(String) (y/10);
			TFT480.fillRect(450, 300, 20, 20, NOIR);
			TFT480.drawString(s1, 450, 300);// affi compte à rebour
		} 

		File1.close();  // referme le fichier
		TFT480.fillRect(450, 300, 20, 20, NOIR); // efface le compte à rebour
	}
}

/** ----------------------------------------------------------------------------------- */



void Draw_arc_elliptique(uint16_t x0, uint16_t y0, int16_t dx, int16_t dy, float alpha1, float alpha2, uint16_t couleur)
// alpha1 et alpha2 en radians
{
/*
REMARQUES :	
-cette fonction permet également de dessiner un arc de cercle (si dx=dy), voire le cercle complet
- dx et dy sont du type int (et pas uint) et peuvent êtres négafifs, ou nuls.
-alpha1 et alpha2 sont les angles (en radians) des caps des extrémités de l'arc 
*/ 
	uint16_t n;
	float i;
	float x,y;

	i=alpha1;
	while(i<alpha2)
	{
		x=x0+dx*cos(i);
		y=y0+dy*cos(i+M_PI/2.0);
		TFT480.drawPixel(x,y, couleur);
		i+=0.01;  // radians
	}
}


void affi_rayon1(uint16_t x0, uint16_t y0, uint16_t rayon, double angle, float pourcent, uint16_t couleur_i, bool gras)
{
// trace une portion de rayon de cercle depuis 100%...à pourcent du rayon du cercle
// angle en radians - sens trigo
	float x1, x2;
	float y1, y2;

	x1=x0+rayon* cos(angle);
	y1=y0-rayon* sin(angle);

	x2=x0+pourcent*rayon* cos(angle);
	y2=y0-pourcent*rayon* sin(angle);

	TFT480.drawLine(x1, y1, x2, y2, couleur_i);
	if (gras)
	{
		TFT480.drawLine(x1, y1-1, x2, y2-1, couleur_i);
		TFT480.drawLine(x1, y1+1, x2, y2+1, couleur_i);
	}
}



void affi_rayon2(uint16_t x0, uint16_t y0, float r1, float r2, float angle_i, uint16_t couleur_i, bool gras)
{
// trace une portion de rayon de cercle entre les distances r1 et r2 du centre
// angle_i en degrés décimaux - sens trigo

	float angle =angle_i/57.3;  // (57.3 ~ 180/pi)
	int16_t x1, x2;
	int16_t y1, y2;

	x1=x0+int16_t(r1* cos(angle));
	y1=y0-int16_t(r1* sin(angle));

	x2=x0+int16_t(r2* cos(angle));
	y2=y0-int16_t(r2* sin(angle));

	if ((x1>0) && (x2>0) && (y1>0) && (y2>0) && (x1<480) && (x2<480) && (y1<320) && (y2<320) )
	{
		TFT480.drawLine(x1, y1, x2, y2, couleur_i);
		
		if (gras)
		{
			TFT480.drawLine(x1, y1-1, x2, y2-1, couleur_i);
			TFT480.drawLine(x1, y1+1, x2, y2+1, couleur_i);
		}
	}
	//TFT480.fillCircle(x2, y2, 2, ROUGE);
}




void affi_tiret_H(uint16_t x0, uint16_t y0, uint16_t r, double angle_i, uint16_t couleur_i)
{
// trace un tiret perpendiculaire à un rayon de cercle de rayon r
// angle_i en degrés décimaux

	float angle =angle_i/57.3;  // (57.3 ~ 180/pi)
	float x1, x2;
	float y1, y2;

	x1=x0+(r)* cos(angle-1);
	y1=y0-(r)* sin(angle-1);		

	x2=x0+(r)* cos(angle+1);
	y2=y0-(r)* sin(angle+1);

	TFT480.drawLine(x1, y1,  x2, y2, couleur_i);	
}



void affi_pointe(uint16_t x0, uint16_t y0, uint16_t r, double angle_i, float taille, uint16_t couleur_i)
{
// trace une pointe de flèche sur un cercle de rayon r
// angle_i en degrés décimaux - sens trigo

	float angle =angle_i/57.3;  // (57.3 ~ 180/pi)
	int16_t x1, x2, x3;
	int16_t y1, y2, y3;

	x1=x0+r* cos(angle); // pointe
	y1=y0-r* sin(angle); // pointe

	x2=x0+(r-7)* cos(angle-taille); // base A
	y2=y0-(r-7)* sin(angle-taille); // base A		

	x3=x0+(r-7)* cos(angle+taille); // base B
	y3=y0-(r-7)* sin(angle+taille); // base B

	TFT480.fillTriangle(x1, y1,  x2, y2,  x3, y3,	couleur_i);	
}


void affi_rectangle_incline(uint16_t x0, uint16_t y0, uint16_t r, double angle_i, uint16_t couleur_i)
{
//rectangle inscrit dans le cerce de rayon r
// angle_i en degrés décimaux - sens trigo

	float angle =angle_i/57.3;  // (57.3 ~ 180/pi)
	int16_t x1, x2, x3, x4;
	int16_t y1, y2, y3, y4;
	float d_alpha=0.08; // détermine la largeur du rectangle

// point 1
	x1=x0+r*cos(angle-d_alpha); 
	y1=y0+r*sin(angle-d_alpha);	
// point 2
	x2=x0+r*cos(angle+d_alpha); 
	y2=y0+r*sin(angle+d_alpha);	
// point 3
	x3=x0+r*cos(M_PI + angle-d_alpha); 
	y3=y0+r*sin(M_PI + angle-d_alpha);
// point 4
	x4=x0+r*cos(M_PI + angle+d_alpha); 
	y4=y0+r*sin(M_PI + angle+d_alpha);

	TFT480.drawLine(x1, y1,  x2, y2, couleur_i);
	TFT480.drawLine(x2, y2,  x3, y3, couleur_i);
	TFT480.drawLine(x3, y3,  x4, y4, couleur_i);
	TFT480.drawLine(x4, y4,  x1, y1, couleur_i);
		
}




float degTOrad(float angle)
{
	return (angle * M_PI / 180.0);
}



void init_affi_HA()
{
	TFT480.fillRect(HA_x0-HA_w, HA_y0-HA_h-1, 2*HA_w, HA_h+1, HA_CIEL);
	TFT480.fillRect(HA_x0-HA_w, HA_y0-HA_h +  HA_h, 2*HA_w, HA_h, HA_SOL);

}




void dessine_avion() // sous forme d'équerres horizontales noires entourées de blanc
{
// aile gauche	
	TFT480.fillRect(HA_x0-102, HA_y0-3, 60, 10, BLANC);	//H contour en blanc
	TFT480.fillRect(HA_x0-42, HA_y0-3, 10, 19, BLANC);	//V
	
	TFT480.fillRect(HA_x0-100, HA_y0-1, 60, 5, NOIR);	//H
	TFT480.fillRect(HA_x0-40, HA_y0-1, 5, 15, NOIR);		//V


// aile droite	
	TFT480.fillRect(HA_x0+28, HA_y0-3, 64, 10, BLANC);	//H contour en blanc
	TFT480.fillRect(HA_x0+28, HA_y0-3, 10, 19, BLANC);	//V
	
	TFT480.fillRect(HA_x0+30, HA_y0-1, 60, 5, NOIR);		//H
	TFT480.fillRect(HA_x0+30, HA_y0-1, 5, 15, NOIR);		//V

//carré blanc au centre

	TFT480.fillRect(HA_x0-4, HA_y0-3, 8, 2, BLANC);
	TFT480.fillRect(HA_x0-4, HA_y0-3, 2, 8, BLANC);

	TFT480.fillRect(HA_x0-4, HA_y0+3, 10, 2, BLANC);
	TFT480.fillRect(HA_x0+4, HA_y0-3, 2, 8, BLANC);

	//affi_dst_NAV();

}


void affiche_chrono()
{
	uint16_t x0=240;
	uint16_t y0=0;
	TFT480.setFreeFont(FM9);
	TFT480.setTextColor(JAUNE);
	String s1;
	////if(heures<10){s1+="0";}
	////s1+=String(heures);
	////s1+=":";
	if(minutes<10){s1+="0";}
	s1+=String(minutes);
	s1+=":";
	if(secondes<10){s1+="0";}
	s1+=String(secondes);
	TFT480.fillRect(x0, y0, 55, 15, BLEU); //efface
	TFT480.drawString(s1, x0, y0); 
	
}


void inc_chrono()
{
	secondes++;
	if (secondes>59)
	{
		secondes=0;
		minutes++;
		if(minutes>59)
		{
			minutes=0;
			heures++;
			if (heures>23)
			heures=0;
		}
	}
}


void RAZ_chrono()
{
	heures=0;
	minutes=0;
	secondes=0;
}



void lign_sep(uint16_t Ax, uint16_t Ay, uint16_t Bx, uint16_t By)
{
// actualise la ligne de séparation ciel-sol

	TFT480.drawLine(Ax, Ay-1, Bx, By-1, HA_CIEL);
	TFT480.drawLine(Ax, Ay, Bx, By, BLANC);
	TFT480.drawLine(Ax, Ay+1, Bx, By+1, HA_SOL);
}



void arrondissement_coins()
{

// fillTriangle(int32_t x1,int32_t y1, int32_t x2,int32_t y2, int32_t x3,int32_t y3, uint32_t color);

//HG
	TFT480.fillTriangle(
		HA_x0-HA_w, HA_y0-HA_h-1,
		HA_x0-HA_w, HA_y0-HA_h+20,
		HA_x0-HA_w+20, HA_y0-HA_h-1,
	NOIR);


//----------------------------------------------
//HD
	TFT480.fillTriangle(
		HA_x0+HA_w, HA_y0-HA_h-1,
		HA_x0+HA_w, HA_y0-HA_h+20,
		HA_x0+HA_w-20, HA_y0-HA_h-1,
	NOIR);

	

//----------------------------------------------
//BG
	TFT480.fillTriangle(
		HA_x0-HA_w, HA_y0+HA_h+1,
		HA_x0-HA_w, HA_y0+HA_h-20,
		HA_x0-HA_w+20, HA_y0+HA_h+1,
	NOIR);

//----------------------------------------------
//BD
	TFT480.fillTriangle(
		HA_x0+HA_w, HA_y0+HA_h+1,
		HA_x0+HA_w, HA_y0+HA_h-20,
		HA_x0+HA_w-20, HA_y0+HA_h+1,
	NOIR);

}

void affi_distance_piste()
{
	String s1;
	uint16_t x0=190;
	uint16_t y0=255; 
	float nav_nm;
// rappel: 1 mile marin (NM nautical mile) = 1852m
	//ils_nm = (float)ils_dst / 1852.0;
	//if (ils_nm >99) {ils_nm=0;}
	
	TFT480.drawRect(x0-47, y0-15, 190, 35, GRIS_FONCE); //encadrement

	TFT480.setTextFont(1);
	TFT480.setTextColor(BLANC, NOIR);
	TFT480.drawString("distance", x0, y0-12);

	TFT480.setFreeFont(FM9);
	TFT480.setTextColor(JAUNE, NOIR);
	TFT480.drawString("RWY", x0-45, y0-12);

		
	TFT480.setTextColor(BLANC, NOIR);
	int nb_decimales;
	if(GPS_distance_piste>99) {nb_decimales =0;} else {nb_decimales =1;}
	s1 = String(GPS_distance_piste, nb_decimales);
	if (data_ok == 0) {s1=" --";}
	TFT480.fillRect(x0, y0, 52, 18, NOIR); // efface
	TFT480.setFreeFont(FM9);
	TFT480.drawString(s1, x0, y0);
	TFT480.drawRoundRect(x0, y0-2, 50, 18, 5, GRIS_FONCE); // encadrement de la valeur affichée

//affi_float_test(GPS_distance_piste_new, 100, 3, VERT, NOIR);
	
	TFT480.setTextColor(JAUNE, NOIR);
	TFT480.drawString("NM", x0+55, y0);
	
//affi_direction_piste // direction de la piste vue de l'avion
	TFT480.setTextFont(1);
	TFT480.setTextColor(BLANC, NOIR);
	TFT480.drawString("direction", x0+80, y0-12);

	TFT480.setTextColor(BLANC, NOIR);
	s1 = String(GPS_azimut_piste, 0); // 0  -> 0 décimales
	if (data_ok == 0) {s1=" --";}
	TFT480.fillRect(x0+90, y0, 52, 18, NOIR); // efface
	TFT480.setFreeFont(FM9);
	TFT480.drawString(s1, x0+90, y0);
	TFT480.drawRoundRect(x0+90, y0-2, 40, 18, 5, GRIS_FONCE); // encadrement de la valeur affichée

	TFT480.drawCircle(x0+135, y0, 2, JAUNE); // caractère 'degré'
}



void affi_HA()  // Horizon Artificiel
{
	String s1;
	
	////String s1=(String) roulis;
	////TFT480.drawString(s1, 400, 20);


// pivot
	int16_t x0=0;
	int16_t y0=0;

//points d'intersection avec le bord du carré
	int16_t Ax, Ay; // sur le bord gauche
	int16_t Bx, By; // sur le bord droit
// Le dessin consistera à tracer des segments colorés entre les points A et B	

	
// roulis -> [-90..+90]

// normalisation de la valeur R2 -> toujours >0
	float R2 = -1*roulis;
	if (R2<0) {R2+=360;} // ce qui est un angle identique, de valeur positive (sens trigo)

// le pivot reste centré horizontalement mais se déplace verticalement en fonction du tangage	
	y0 += 2*tangage;

//calcul & memorisation de ces deux facteurs, ce qui évitera 2 calculs de tangente à chaque passage dan la fonction
	float tgt_moins = tan(degTOrad(90-R2));
	float tgt_plus = tan(degTOrad(90+R2));
	
//-----------------------------------------------------------------------------
// CALCUL COTE DROIT (point B)

// calcul du point B d'intersection 
	Bx=HA_w;
	By=y0 + HA_w*tan(degTOrad(R2));

//test si le point d'intersection se trouve plus haut que le haut du carré :
	
	if(By>HA_h)
	{
		By=HA_h;
		Bx = x0 + (HA_h-y0)*tgt_moins;
	}
	
	if(By< -HA_h)
	{
		By= -HA_h;
		Bx = x0 + (HA_h+y0)*tgt_plus;
	}
//-----------------------------------------------------------------------------
// CALCUL COTE GAUCHE (point A)

	Ax=-HA_w;
	Ay=y0 - HA_w*tan(degTOrad(R2));

	if(Ay> HA_h)
	{
		Ay= HA_h;
		Ax = x0 + (HA_h-y0)*tgt_moins;
	}

	if(Ay< -HA_h)
	{
		Ay= -HA_h;
		Ax = x0 + (HA_h+y0)*tgt_plus;
	}
	
	
//-----------------------------------------------------------------------------
// positionnement de l'ensemble sur l'écran	

	Ax += HA_x0;
	Ay += HA_y0;

	Bx += HA_x0;
	By += HA_y0;

// pour éviter un tracé hors cadre au premier passage :
	if (flag_1er_passage == 1)
	{
		Ax_actu = Ax;
		Ay_actu = Ay;
				
		Bx_actu = Bx;
		By_actu = By;
		flag_1er_passage=0;
	}


//-----------------------------------------------------------------------------
// ligne "verticale" d'inclinaison (tangage)

	affi_rayon2(HA_x0, HA_y0, 85, -memo_y0, 90-memo_R2, HA_CIEL, false); // efface partie supérieure
	affi_rayon2(HA_x0, HA_y0, 85, -y0, 90-R2, BLANC, false);	// retrace ligne partie supérieure
	
	affi_rayon2(HA_x0, HA_y0, -85,-memo_y0, 90-memo_R2, HA_SOL, false);  // efface partie inférieure
	affi_rayon2(HA_x0, HA_y0, -85,-y0, 90-R2, VERT, false);	// retrace ligne partie inférieure

	affi_pointe(HA_x0, HA_y0, 85, 90-memo_R2, 0.1, HA_CIEL); // efface
	affi_pointe(HA_x0, HA_y0, 85, 90-R2, 0.1, BLANC); // retrace


//-----------------------------------------------------------------------------
// graduation fixe
	TFT480.setFreeFont(FF1);
	TFT480.setTextColor(BLANC, GRIS_AF);
	TFT480.drawString("30", HA_x0-70, HA_y0-98);
	TFT480.drawString("60", HA_x0-120, HA_y0-55);
	
	TFT480.drawString("30", HA_x0+60, HA_y0-98);
	TFT480.drawString("60", HA_x0+100, HA_y0-55);

//-----------------------------------------------------------------------------
// animation de la ligne de séparation horizontale

	while ((Bx_actu != Bx) ||  (By_actu != By) || (Ax_actu != Ax) ||  (Ay_actu != Ay))
	{
	// déplacements successifs de 1 pixel de chaque extrémités de la ligne	

		//TFT480.drawLine(Bx, By, x2, y2, BLANC);

		if (Bx_actu < Bx) { Bx_actu++; lign_sep(Ax_actu, Ay_actu, Bx_actu, By_actu); }
		if (Bx_actu > Bx) { Bx_actu--; lign_sep(Ax_actu, Ay_actu, Bx_actu, By_actu); }
				
		if (Ax_actu < Ax) { Ax_actu++; lign_sep(Ax_actu, Ay_actu, Bx_actu, By_actu); }
		if (Ax_actu > Ax) { Ax_actu--; lign_sep(Ax_actu, Ay_actu, Bx_actu, By_actu); }
		
		if (By_actu < By) { By_actu++; lign_sep(Ax_actu, Ay_actu, Bx_actu, By_actu); }
		if (By_actu > By) { By_actu--; lign_sep(Ax_actu, Ay_actu, Bx_actu, By_actu); }
		
		if (Ay_actu < Ay) { Ay_actu++; lign_sep(Ax_actu, Ay_actu, Bx_actu, By_actu); }
		if (Ay_actu > Ay) { Ay_actu--; lign_sep(Ax_actu, Ay_actu, Bx_actu, By_actu); }
	}
	

// graduation roulis qui se déplace angulairement avec la ligne de tangage
	for (int8_t n=0; n<4; n++)
	{
		Draw_arc_elliptique(HA_x0, HA_y0, 20*n, 20*n, degTOrad(80-memo_R2+n), degTOrad(100-memo_R2-n), HA_CIEL); // efface bas
		Draw_arc_elliptique(HA_x0, HA_y0, 20*n, 20*n, degTOrad(80-R2+n), degTOrad(100-R2-n), BLANC); // trace bas

		Draw_arc_elliptique(HA_x0, HA_y0, 20*n, 20*n, degTOrad(260-memo_R2+n), degTOrad(280-memo_R2-n), HA_SOL); // efface haut
		Draw_arc_elliptique(HA_x0, HA_y0, 20*n, 20*n, degTOrad(260-R2+n), degTOrad(280-R2-n), BLANC); // trace haut
	}
	
	memo_R2 = R2;
	memo_y0 = y0;
	
//-----------------------------------------------------------------------------
	arrondissement_coins();

	if (read_bit(flags, bit_autoland) == 0)
	{
		affi_distance_piste();
	}
}



void affi_acceleration()
{
// POUR TEST **********
////String s2= (String) acceleration;
////TFT480.fillRect(100, 50, 200, 20, TFT_BLACK);
////TFT480.setFreeFont(FF5);
////TFT480.setTextColor(BLANC, NOIR);
////TFT480.drawString("Acceleration=", 100, 50);
////TFT480.drawString(s2, 250, 50);
// ******************** 
	
//barres verticales colorées juste à droite de la vitesse indiquant sa variation
	uint16_t x0=60;
	uint16_t Y_zero=162;
	
	int16_t dy=0;
	
//"fleche" haute
	TFT480.fillRect(x0, 40, 8, Y_zero, GRIS_TRES_FONCE); // efface haut
	if (acceleration > 1)
	{
		dy= acceleration;
		
		TFT480.fillRect(x0, Y_zero-dy, 8, dy, VERT); // fleche 
	}


//"fleche" basse
	TFT480.fillRect(x0, Y_zero, 8, 150, GRIS_TRES_FONCE); // efface bas
	if (acceleration < -1)
	{
		dy= -acceleration;
		
		TFT480.fillRect(x0, Y_zero, 8, dy, JAUNE); // fleche
	}
	
	TFT480.fillRect(x0, Y_zero, 10, 2, BLANC); // tiret horizontal blanc

	TFT480.fillRect(x0, 310, 8, 20, NOIR);
	
}


void bride(int16_t *valeur)
{
	int16_t y_min =40;
	int16_t y_max =310;	
	if (*valeur<y_min) {*valeur=y_min;}
	if (*valeur>y_max) {*valeur=y_max;}
}


void affi_switches() // en haut à droite
{
	TFT480.setTextFont(1);
	TFT480.setTextColor(GRIS);
	
	TFT480.fillRect(430, 0, 25, 10, NOIR); // efface le nombre précédemment affiché 
	TFT480.drawString(switches, 430, 0);

	TFT480.fillRect(430, 10, 25, 10, NOIR); // efface le nombre précédemment affiché 
	TFT480.drawString(switches_ND, 430, 10);
}



void affi_elevator() 
{
	bargraph_V_float(elevator, 340, 130, JAUNE); 
}



void affi_rudder() 
{
	TFT480.setTextFont(1);
	TFT480.fillRect(430, 20, 25, 8, NOIR); // efface le nombre précédemment affiché 
	TFT480.setTextColor(ORANGE);
	TFT480.drawString(potar1, 430, 20); // nombre orange en haut à droite

	float v1 = rudder;
	bargraph_H_float(v1, 210, 235, JAUNE); // barre horizontale sous l'horizon artificiel
}


void affi_flags() // nombre jaune en haut à droite
{
	TFT480.fillRect(430, 30, 25, 8, NOIR); // efface le nombre précédemment affiché
	TFT480.setTextFont(1);
	TFT480.setTextColor(JAUNE);
	String s1 = String(flags);
	TFT480.drawString(s1, 430, 30);
	
}


void affi_etats_bt_MCDU() // nombre vert en haut à droite
{
	TFT480.fillRect(430, 40, 25, 8, NOIR); // efface le nombre précédemment affiché
	TFT480.setTextFont(1);
	TFT480.setTextColor(VERT);
	String s1 = String(v_bt_MCDU);
	TFT480.drawString(s1, 430, 38);
}


void affi_extremite() // en haut à droite
{
	TFT480.fillRect(430, 50, 25, 6, NOIR); // efface le nombre précédemment affiché
	TFT480.setTextFont(1);
	TFT480.setTextColor(BLEU_CLAIR);
	String s1 = String(extremite_pst); // 'A'  ou 'B' (la plus éloignée de l'avion)
	TFT480.drawString(s1, 430, 45);
}


void affi_sens_APP() // en haut à droite
{
	TFT480.fillRect(445, 50, 35, 6, NOIR); // efface le nombre précédemment affiché
	TFT480.setTextFont(1);
	TFT480.setTextColor(VERT);
	String s1;
	if (sens_app_effectif == sens_AB) {s1 = "A->B";} else {s1 = "B->A";}
	TFT480.drawString(s1, 445, 48);
}


void affi_vitesse()
{
	uint16_t x1;
	String s1;
	
	int16_t y_min =40;
	int16_t y_max =300;	

	TFT480.setTextColor(BLANC); // Background is not defined so it is transparent
	
//---------------------------------------------------------------------------------------
//bande verticale multicolore

	#define vitesse_sol 40
	int16_t vitesse_mini1 =90;
	int16_t vitesse_mini2 =130;
	int16_t vitesse_maxi1 =200;
	int16_t vitesse_maxi2 =280;


//calcul de la position des limites	entre les différentes couleurs verticales

	int16_t d1, d2, d3, d4, d5;

	d1=(int16_t)(100 + 3.2*((vitesse - vitesse_sol)));		
	d2=(int16_t)(100 + 3.2*((vitesse - vitesse_mini1)));	
	d3=(int16_t)(100 + 3.2*((vitesse - vitesse_mini2)));	
	d4=(int16_t)(100 + 3.2*((vitesse - vitesse_maxi1)));	
	d5=(int16_t)(100 + 3.2*((vitesse - vitesse_maxi2)));	

	bride(&d1);
	bride(&d2);
	bride(&d3);
	bride(&d4);
	bride(&d5);
	
	int16_t h1, h2, h3, h4, h5;
	
	h1 = y_max-(int16_t)d1;
	h2 = d1-d2;	
	h3 = d2-d3;	
	h4 = d3-d4;	
	h5 = d4-d5;
	
	TFT480.fillRect(50, 40, 6, (int16_t)d5,  ORANGE);
	TFT480.fillRect(50, d5, 6, h5, JAUNE); 
	TFT480.fillRect(50, d4, 6, h4, VERT); 
	TFT480.fillRect(50, d3, 6, h3, ORANGE);
	TFT480.fillRect(50, d2, 6, h2, ROUGE);
	TFT480.fillRect(50, d1, 6, 300-(int16_t)d1, GRIS);

	TFT480.fillRect(50, 300, 6, 20, NOIR);

//---------------------------------------------------------------------------------------
//échelle verticale graduée glissante

	uint16_t y0;
	
	int16_t vit1;
	float d6;
	
	TFT480.setFreeFont(FF1);

	y0=3.2*(vitesse%10);

	TFT480.fillRect(0, y_min, 50, y_max-30, GRIS_AF); // bande verticale à gauche
	for(int n=0; n<10; n++)
	{
		d6 =2+y0+32.0*n; // 24 pixels verticalement entre chaque trait -> 10*24 = 240px (hauteur de l'affiur)
		{
			if ( (d6>y_min) && (d6<y_max-10) && (vit1>=0) && (vit1<1000) ) 
			{
				TFT480.fillRect(45, (int16_t)d6, 10, 2, BLANC); // petits tirets horizontaux
			}

			vit1 = vitesse -10*(n-5);
			vit1 /= 10;
			vit1 *= 10;
			s1=(String) vit1;

			if ( (d6>y_min) && (d6<y_max-10) && (vit1>=0) && (vit1<1000) ) 
			{
				TFT480.setTextColor(BLANC, GRIS_AF);
				//TFT480.drawString("    ", 9, d6);
				x1=0; 
				if(vit1<100){x1=7;} // pour affichage centré
				if(vit1<10){x1=14;}
				if (vit1>=10)
				{
					TFT480.drawString(s1, x1, (uint16_t)d6-5); // Graduation (tous les 20kts)
				}
			}
		}
	}
	TFT480.fillRect(0, 38, 68, 2, NOIR); // efface ;  BLEU pour test
	
//---------------------------------------------------------------------------------------
// affichage de la valeur principale

	uint16_t VP_y0 = 150;

	TFT480.setTextColor(BLANC, NOIR);
	TFT480.setFreeFont(FF18);
	s1=(String)vitesse;
	
	TFT480.fillRect(3, VP_y0, 42, 26, NOIR); //efface le nombre précédemment affiché (pour le cas où on passe de 3 à 2 chiffres)
	
	if ((vitesse>=0) && (vitesse <1000))
	{
		x1=3; 
		if(vitesse<100){x1=10;} // pour affichage centré
		if(vitesse<10){x1=20;}
		TFT480.drawString(s1, x1, VP_y0+3); // affi le nombre
	} // affi en gros à mi-hauteur de l'écran
	else
	{ TFT480.fillRect(3, VP_y0, 42, 26, GRIS); }

	TFT480.drawRoundRect(1, VP_y0-1, 45, 28, 5, BLANC); // encadrement de la valeur centrale affichée

	TFT480.fillTriangle(45, VP_y0+7, 45, VP_y0+17, 55, VP_y0+12, NOIR); // petit triangle (curseur) noir
}



void affi_asel(int32_t asel_i)
{
// consigne ALTITUDE de l'autopilot (en rose en haut à droite)
	uint16_t x1 =360;
	TFT480.setFreeFont(FF5);

	if(asel_i >=0) 
	{
		// ( chiffres en roses en haut à droite)
		String s2 =(String)(asel_i);     
		TFT480.setTextColor(ROSE, NOIR);
		
		TFT480.fillRect(x1, 0, 77, 20, NOIR); // efface
		if(asel_i<10000){x1+=7;} 
		if(asel_i<1000){x1+=7;} // pour affichage centré
		if(asel_i<100){x1+=7;}
		if(asel_i<10){x1+=7;} 
		
		TFT480.drawString(s2, x1, 5);	
	}
}


void affi_alti_agl()
{
// consigne ALTITUDE de l'autopilot (couleur du texte = HA_SOL, en bas à droite)
	uint16_t x1 =360;
	TFT480.setFreeFont(FF5);

	if(alti_agl >=0) 
	{
		// ( chiffres en roses en haut à droite)
		String s2 =(String)(alti_agl);     
		TFT480.setTextColor(HA_SOL, NOIR);
		
		TFT480.fillRect(x1, 300, 77, 20, NOIR); // efface
		if(alti_agl<10000){x1+=7;} 
		if(alti_agl<1000){x1+=7;} // pour affichage centré
		if(alti_agl<100){x1+=7;}
		if(alti_agl<10){x1+=7;} 
		
		TFT480.drawString(s2, x1, 303);

		TFT480.setTextFont(1);
		TFT480.setTextColor(BLANC, NOIR);
		TFT480.drawString("hauteur/gnd:", 290, 307);
		TFT480.drawRoundRect(287, 302, 136, 17, 3, GRIS_FONCE); // encadrement de la valeur affichée
	}
}


void affi_target_speed()
{
// consigne de vitesse de l'autopilot
// ( chiffres en rose en haut à gauche	)

	String s2 =(String)(target_speed);     
	TFT480.setTextColor(ROSE, NOIR);
	TFT480.setFreeFont(FF5);
	uint8_t x1=7;
	TFT480.fillRect(x1, 20, 60, 15, NOIR); // efface
	TFT480.drawString(s2, x1, 20);	
}




void affi_vt_verticale()
{
// affichage analogique sur la droite de l'écran

	uint16_t x0=435;
	uint16_t y0=165;

	float y1;

	uint16_t x1;
	String s1;
	
	TFT480.fillRect(x0, y0-90, 45, 180, GRIS_AF); // barre grise
	TFT480.fillRect(x0, y0, 25, 2, BLEU_CLAIR);  // centre

// ------------------------
// graduations sur un arc vertical
	TFT480.setFreeFont(FF1);

	TFT480.setTextColor(BLANC, NOIR);
	TFT480.drawString("ft/mn", x0-8, y0+125);

	TFT480.setTextColor(BLANC, GRIS_AF);

	float angle;
	for(uint8_t n=0; n<7; n++ )
	{
		angle =135+ n*15;  // 1 tiret tous les 15 degrés
		affi_rayon1(HA_x0+340, y0, 110, degTOrad(angle), 0.9, BLANC, false); // tirets de graduation
	}

	TFT480.drawString("3", x0+9, y0-90);
	TFT480.drawString("2", x0-3, y0-65);
	TFT480.drawString("1", x0-8, y0-35);
	TFT480.drawString("0", x0-3, y0-5 + 0);
	TFT480.drawString("1", x0-8, y0+25);
	TFT480.drawString("2", x0-3, y0+50);
	TFT480.drawString("3", x0+9, y0+75);
	
// ------------------------
// aiguille à droite de l'écran
	float angle2;
	
	TFT480.setFreeFont(FF1);
	s1=(String) (vspeed*60);
	
	angle2 = 180.0 - vspeed *0.92;

	TFT480.fillRect(x0-10, y0-110, 55, 15, GRIS_TRES_FONCE); // efface haut
	TFT480.fillRect(x0-10, y0+105, 55, 15, GRIS_TRES_FONCE); // efface bas

	if ((vspeed > -50) && (vspeed < 50))
	{
		affi_rayon1(HA_x0+330, y0, 100, degTOrad(angle2), 0.7, JAUNE, true);
		
		TFT480.setTextColor(JAUNE, NOIR);
	}
	else if (vspeed > 50)
	{
		affi_rayon1(HA_x0+330, y0, 110, degTOrad(132), 0.7, JAUNE, true);
		TFT480.setTextColor(JAUNE, NOIR);
		
		TFT480.drawString(s1, x0-10, y0-110);
	}
	else if (vspeed < -50)
	{
		affi_rayon1(HA_x0+330, y0, 110, degTOrad(228), 0.7, JAUNE, true);
		TFT480.setTextColor(JAUNE, NOIR);
		
		TFT480.drawString(s1, x0-10, y0+105);
	}


// affichage digital de la valeur
	

/*	
// = vitesse ascensionnelle, sous forme de barres verticales vertes, à droite, près de l'echelle d'altitude	
	uint16_t x0=405;
	uint16_t y0=40;
	
	int16_t dy=0;
	
//fleche haute
	TFT480.fillRect(x0, 0, 10, 140, GRIS_FONCE); // efface haut
	if (vspeed > 1)
	{
		dy= vspeed;
		
		TFT480.fillRect(x0, y0+100-dy, 10, dy, VERT); // fleche 
	}
	

//fleche basse
	TFT480.fillRect(x0, y0+150, 10, 135, GRIS_FONCE); // efface bas
	if (vspeed < -1)
	{
		dy= -vspeed;
		
		TFT480.fillRect(x0, y0+150, 10, dy, VERT); // fleche
	}
*/
	
}




void affi_cap()
{
// cercle tournant de CAP gradué en bas au centre de l'écran
// Les lettres 'N' 'S' 'E' 'O' pour Nord Sud Est Ouset sont initialisées sous forme de sprites dans la fonction setup()

	uint16_t x02 = 200;
	uint16_t y02 = 350;
	float angle; // en radians
	//float cap_RD; // en radians (le cap fourni par FG étant en degrés d'angle)
	uint16_t x_spr;
	uint16_t y_spr;

	uint16_t x_hdg;
	uint16_t y_hdg;

	uint8_t R =70;
	uint8_t R2 =R-6;
/**
360° =2 pi rad
1° = 2 pi/360 rad = pi/180 rad
**/

	TFT480.fillCircle(x02,y02, R, GRIS_AF);

	for(uint8_t n=0; n<24; n++ )
	{
		angle = (int16_t)cap+15 + n*15;  // 1 tiret tous les 15 degrés
		affi_rayon1(x02, y02, (R-5), degTOrad(angle), 0.9, BLANC, false); // tirets de graduation
	}
	x_hdg =  x02 + R2*cos(degTOrad(hdg1-90-cap));
	y_hdg =  y02 + R2*sin(degTOrad(hdg1-90-cap));
	
	TFT480.drawLine(x02, y02, x_hdg, y_hdg, VERT);
	TFT480.drawCircle(x_hdg, y_hdg, 5, VERT); // rond vert sur le cercle = consigne de cap	de l'autopilot

	x_spr = x02+R2 * cos(degTOrad(angle));
	y_spr = y02-R2 * sin(degTOrad(angle));
	TFT480.setPivot(x_spr, y_spr);
	SPR_E.pushRotated(-cap+90, TFT_BLACK); // Plot rotated Sprite, black = transparent

	x_spr = x02+R2* cos(degTOrad(angle+90));
	y_spr = y02-R2 * sin(degTOrad(angle+90));
	TFT480.setPivot(x_spr, y_spr);
	SPR_N.pushRotated(-cap, TFT_BLACK); 

	x_spr = x02+R2 * cos(degTOrad(angle+180));
	y_spr = y02-R2 * sin(degTOrad(angle+180));
	TFT480.setPivot(x_spr, y_spr);
	SPR_O.pushRotated(-cap-90, TFT_BLACK); 

	x_spr = x02+R2 * cos(degTOrad(angle-90));
	y_spr = y02-R2 * sin(degTOrad(angle-90));
	TFT480.setPivot(x_spr, y_spr);
	SPR_S.pushRotated(-cap, TFT_BLACK); 
	

// petite "maison" dans le cercle (valeur du cap)

	#define a 170   // x général
	#define b a+30
	#define c b+30
	#define d 288	// y général
	#define e d+10
	#define f e+20

	TFT480.drawLine(a, f, c, f, BLANC); // sol
	TFT480.drawLine(a, f, a, e, BLANC); // mur de gauche
	TFT480.drawLine(c, f, c, e, BLANC); // mur de droite
	TFT480.drawLine(a, e, b, d, BLANC); // toit pente gauche
	TFT480.drawLine(c, e, b, d, BLANC); // toit pente droite

// affi la valeur	
	String s1;
	uint16_t x0 = a+1;
	uint16_t y0 = e;
	
	uint16_t x1= x0;
	if(cap<100){x1+=5;} // pour affichage centré
	if(cap<10){x1+=5;}

	s1=String (cap, 1);

	TFT480.fillRect(x0, y0, 57, 20, NOIR); // efface le nombre précédemment affiché
	TFT480.setTextColor(BLANC, NOIR);
	TFT480.setFreeFont(FM9);
	TFT480.drawString(s1, x1, y0);
}



void affi_hauteur_RWY()
{
/**
Pour exprimer une hauteur au dessus de l'aérodrome, on défini la hauteur AAL (Above Aerodrome Level). Il
s'agit de la hauteur entre l'avion et le point de référence de l'aérodrome comme s'il était en dessous de la
position de l'appareil (même s'il n'y est pas). Cette hauteur ne suit pas le relief.
On la calculera ici en retranchant [l'altitude de l'aéroport sélectionné] à [l'altitude GPS].

En conséquense, il faut impérativement penser à sélectionner dans le module SD le bon aérodrome, celui d'où l'on décolle,
puis en cas de voyage, celui où l'on va se poser (ce qui renseignera son altitude) sinon l'affichage sera faux.

par exemple si l'on choisit "Montpellier" en étant à Clermont-Ferrand, l'erreur sera de 1089 ft

Les altitudes des aérodromes sont enregistées dans le fichier FG_data.h
*/
	
	String s1;
	uint16_t x0 =365;
//---------------------------------------------------------------------------------------
//échelle verticale graduée glissante

	uint16_t x1;
	uint16_t y0;
	uint16_t hauteur;
	int16_t alt1;
	float d5;

	if (mode_affi_hauteur == AAL) {hauteur = hauteur_AAL;}
	if (mode_affi_hauteur == ASL) {hauteur = altitude_GPS;}
	
	TFT480.setFreeFont(FF1);


	y0=3.2*(hauteur_AAL%10);

	TFT480.fillRect(x0, 20, 60, 280, GRIS_AF); //efface bande verticale à droite 

	
	for(int n=0; n<9; n++)
	{
		d5 =0+y0+32.0*n; // pixels verticalement entre chaque trait -> 10*24 = 240px (hauteur de l'affi)
		{
			if (d5>=20)  // marge en haut
			{
				TFT480.fillRect(x0, (int16_t)d5+5, 5, 2, BLANC); // petits tirets horizontaux

				alt1 = hauteur -10*(n-5);
				alt1 /= 10;
				alt1 *= 10;
				s1=(String) alt1;

				if(alt1>=0)
				{
					TFT480.setTextColor(BLANC, GRIS_AF);
					//TFT480.drawString("    ", 9, d5);
					x1=x0;
					if(alt1<10000){x1+=7;} // pour affichage centré
					if(alt1<1000){x1+=7;} 
					if(alt1<100){x1+=7;} 
					if(alt1<10){x1+=7;}
					TFT480.drawString(s1, x1, (uint16_t)d5); // Graduation (tous les 20kts)
				}
			}
		}
	}

//---------------------------------------------------------------------------------------
// affichage de la valeur principale
	
	uint16_t x2;
	uint16_t y0b = 155;
	TFT480.fillRect(x0-20, y0b, 80, 25, NOIR); // efface le nombre précédemment affiché
	TFT480.setTextColor(BLANC, NOIR);
	TFT480.setFreeFont(FF18);
	
	if ((1) && (hauteur < 60000))
	{
		s1=(String) hauteur;
	}
	else {s1="----";}
	x2=x0-20;
	if(hauteur<10000){x2+=10;} // pour affichage centré
	if(hauteur<1000){x2+=10;}
	if(hauteur<100){x2+=10;} 
	if(hauteur<10){x2+=10;}
	
	if(hauteur<0)
	{
		TFT480.setTextColor(ROUGE);
		x2=x0-20; // si valeur négative affichée avec signe "-"
	} 
	
	TFT480.drawString(s1, x2, y0b);
	uint16_t couleur1=GRIS;
	if (mode_affi_hauteur == ASL) {couleur1=BLEU;}
	if (mode_affi_hauteur == AAL) {couleur1=VERT;}
	
	TFT480.drawRoundRect(x0-20, y0b-3, 75, 28, 5, couleur1); // encadrement de la valeur centrale affichée
}







void affi_distance_ptAA()
{
	String s1;
	uint16_t x0=260;
	uint16_t y0=280;

	TFT480.setTextFont(1);
	TFT480.setTextColor(BLANC, NOIR);
	TFT480.drawString("to ptA", x0, y0);

	int nb_decimales =1;
	if (GPS_distance_ptAA>=100) {nb_decimales=0;} 
	s1 = String(GPS_distance_ptAA, nb_decimales);
	
	TFT480.fillRect(x0, y0+10, 45, 12, NOIR); // efface
	TFT480.setFreeFont(FM9);
	TFT480.setTextColor(VERT, NOIR);
	TFT480.drawString(s1, x0, y0+10);

	TFT480.drawRoundRect(x0-2, y0+8, 52, 18, 5, GRIS_FONCE); 

	TFT480.setTextColor(JAUNE, NOIR);
	TFT480.drawString("NM", x0+55, y0+10);
}


void affi_distance_ptBB()
{
	String s1;
	uint16_t x0=260;
	uint16_t y0=280;

	TFT480.setTextFont(1);
	TFT480.setTextColor(BLANC, NOIR);
	TFT480.drawString("to ptB", x0, y0);

	int nb_decimales =1;
	if (GPS_distance_ptBB>=100) {nb_decimales=0;}
	s1 = String(GPS_distance_ptBB, nb_decimales);
	
	TFT480.fillRect(x0, y0+10, 45, 12, NOIR); // efface
	TFT480.setFreeFont(FM9);
	TFT480.setTextColor(VERT, NOIR);
	TFT480.drawString(s1, x0, y0+10);

	TFT480.drawRoundRect(x0-2, y0+8, 52, 18, 5, GRIS_FONCE); 

	TFT480.setTextColor(JAUNE, NOIR);
	TFT480.drawString("NM", x0+55, y0+10);
}	


void affi_distance_pti()
{
	String s1;
	uint16_t x0=260;
	uint16_t y0=280;

	TFT480.setTextFont(1);
	TFT480.setTextColor(BLANC, NOIR);

	s1 = "to Pt ";
	s1 += String(num_pti);
		
	TFT480.drawString(s1, x0, y0);

	int nb_decimales =1;
	if (GPS_distance_pti<100) {nb_decimales=0;}
	s1 = String(GPS_distance_pti, nb_decimales);

	TFT480.fillRect(x0, y0+10, 45, 12, NOIR); // efface
	TFT480.setFreeFont(FM9);
	TFT480.setTextColor(VERT, NOIR);
	TFT480.drawString(s1, x0, y0+10);

	TFT480.drawRoundRect(x0-2, y0+8, 52, 18, 5, GRIS_FONCE); 

	TFT480.setTextColor(JAUNE, NOIR);
	TFT480.drawString("NM", x0+55, y0+10);
}


void affi_f_potar1() 
{
	uint16_t x0=145;
	uint16_t y0=160;
	TFT480.fillRect(x0, y0, 130, 30, GRIS_TRES_FONCE);
	TFT480.drawRect(x0, y0, 130, 30, ROUGE);
	TFT480.setFreeFont(FMB9);
	TFT480.setTextColor(BLANC);
	String s1="potar="+String(f_potar1);
	TFT480.drawString(s1, x0+5, y0+6);

}


void affi_hauteur_SOL(int16_t H) // de l'avion / sol en dessous de lui ; dans la partie basse du PFD (dans le 'marron')
{
	WARNING = String(H) + " ft/gnd";
	tempo_message=5;
	//affi_message(WARNING, BLANC, GRIS_TRES_FONCE);
}


void efface_hauteur_SOL()
{
	uint16_t x0=145;
	uint16_t y0=188;
	TFT480.fillRect(x0, y0, 130, 30, HA_SOL);
}

void efface_sprite_trajectoire()
{
	SPR_trajectoire.fillSprite(TFT_BLACK);
	SPR_trajectoire.drawString("pente 5%", 170, 1 );
}
	

void efface_cadre_bas(uint16_t couleur)
{
	TFT480.fillRect(70, 232, 292, 84, NOIR);
	TFT480.drawRect(70, 232, 292, 84, GRIS_FONCE);
	efface_sprite_trajectoire();
}


void efface_message(uint16_t back_couleur)
{
	int x=145;
	int y=210;
	int dx=145;
	TFT480.fillRect(x, y, dx, 18, back_couleur); //efface  le précédent
}	


void affi_message(String s, uint16_t txt_couleur, uint16_t back_couleur)
{
	int x=145;
	int y=210;
	if(WARNING != memo_WARNING)
	{
		memo_WARNING = WARNING; // pour ne pas répéter le même message
		
		TFT480.setFreeFont(FMB9);
		TFT480.setTextColor(txt_couleur, back_couleur);
		
		TFT480.drawString(s, x, y);
	}
}



void affi_Airport()
{
	uint16_t n;
	float v1;
	String s1;


	TFT480.fillRect(255, 280, 108, 20, NOIR); // efface - BLEU pour test
	TFT480.setTextFont(1);

	TFT480.setTextColor(BLEU_CLAIR, NOIR);
	s1= liste_bali[num_bali].ID_OACI;
	TFT480.drawString(s1, 255, 280);

	s1= (String)liste_bali[num_bali].altitude;
	s1 +=" ft";
	TFT480.setTextColor(VIOLET2, NOIR);
	TFT480.drawString(s1, 300, 280);
	
	TFT480.fillRect(270, 300, 60, 30, NOIR); // efface - GRIS pour test
	s1= liste_bali[num_bali].nom;
	TFT480.setTextColor(BLEU_CLAIR, NOIR);
	TFT480.drawString(s1, 255, 290);
}


void affi_mode_affi_hauteur()
{
	if (mode_affi_hauteur == AAL)
	{
		TFT480.setFreeFont(FF1);
		TFT480.setTextColor(VERT, GRIS_AF); // Autolanding en cours, ok
		TFT480.drawString("AAL", 305, 0);
	}
	if (mode_affi_hauteur == ASL)
	{
		TFT480.setFreeFont(FF1);
		TFT480.setTextColor(BLEU_CLAIR, GRIS_AF); // Autolanding en cours, ok
		TFT480.drawString("ASL", 305, 0);
	}
}

void calculs_piste() // lors du choix de l'Airport
{
	lat_ptA = liste_bali[num_bali].lat_A;
	lon_ptA = liste_bali[num_bali].lon_A;
	
	lat_ptB = liste_bali[num_bali].lat_B;
	lon_ptB = liste_bali[num_bali].lon_B;

//affi_float_test(lat_ptB, 120, 4, VERT, NOIR);
//affi_float_test(lon_ptB, 120, 5, JAUNE, NOIR);	

	longueur_piste = 1000.0* distance_AB(lat_ptA, lon_ptA, lat_ptB, lon_ptB); // en m
	
	orient_pisteAB = azimut_AB(lat_ptA, lon_ptA, lat_ptB, lon_ptB);

//orient_pisteAB = 94.43; // fixé pour TEST (piste Béziers-Vias)
//affi_float_test(orient_pisteAB, 120, 2, JAUNE, GRIS_TRES_FONCE); // pour TEST
	
	orient_pisteBA = orient_pisteAB + 180.0;
	if (orient_pisteBA > 360.0) {orient_pisteBA -= 360.0;}
	
	lat_centre_pst=(lat_ptA +lat_ptB)/2.0;
	lon_centre_pst=(lon_ptA +lon_ptB)/2.0;
}

// ===== CALCULS ===============================================================================================================


void calcul_ptAA_ptBB(float dst) // situés à dst NM de la piste, dans l'axe. (dst = 10NM en principe, sauf cas particuliers)
{

	calculs_piste();
	
	float d_lat = lat_ptA - lat_ptB;
	float d_lon = lon_ptA - lon_ptB;
	
	lat_ptAA = lat_centre_pst + (1852 * dst /longueur_piste) * d_lat;
	lon_ptAA = lon_centre_pst + (1852 * dst /longueur_piste) * d_lon; 

//affi_float_test(lat_ptAA, 120, 2, VERT, NOIR);
//affi_float_test(lon_ptAA, 120, 3, JAUNE, NOIR);

	lat_ptBB = lat_centre_pst - (1852 * dst /longueur_piste) * d_lat;
	lon_ptBB = lon_centre_pst - (1852 * dst /longueur_piste) * d_lon;

//affi_float_test(lat_ptBB, 120, 4, VERT, NOIR);
//affi_float_test(lon_ptBB, 120, 5, JAUNE, NOIR);
}








void calculs_GPS() // temps réel
{
// calculs de la position de l'avion / piste (distance et direction)

// DISTANCE (variable globale)
// voir la fonction "distance_AB()" dans le fichier "Fonctions1.h"
	GPS_distance_piste = distance_AB(lat_avion, lon_avion, lat_centre_pst, lon_centre_pst)  / 1.852; // du centre de la piste,  en NM
	GPS_distance_ptAA = distance_AB(lat_avion, lon_avion, lat_ptAA, lon_ptAA)  / 1.852;
	GPS_distance_ptBB = distance_AB(lat_avion, lon_avion, lat_ptBB, lon_ptBB)  / 1.852;
	GPS_distance_pti = distance_AB(lat_avion, lon_avion, lat_pti, lon_pti)  / 1.852;
	

// DIRECTION (variable globale)	
	GPS_azimut_piste = azimut_AB(lat_avion, lon_avion, lat_centre_pst, lon_centre_pst);// latitudes et longitudes en degrés décimaux

	GPS_azimut_ptA = azimut_AB(lat_avion, lon_avion, lat_ptA, lon_ptA);
	GPS_azimut_ptB = azimut_AB(lat_avion, lon_avion, lat_ptB, lon_ptB);
	
	GPS_azimut_ptAA = azimut_AB(lat_avion, lon_avion, lat_ptAA, lon_ptAA);
	GPS_azimut_ptBB = azimut_AB(lat_avion, lon_avion, lat_ptBB, lon_ptBB);
	GPS_azimut_pti = azimut_AB(lat_avion, lon_avion, lat_pti, lon_pti);
}


void calcul_pti(float azimut_i, float distance_i, float *latitude, float *longitude)
{
/*
calcul des coordonnées GPS d'un point quelquonque PROCHE de la piste (en vue de faire des "hyppodromes" biens maitrisés) 

données:
-azimut et distance du point concerné par rapport au, et vu du, centre de la piste (paramètres: azimut_i et distance_i)
-coordonnées GPS des points extrémités de la piste (lus dans le fichier FG_data.h)
-coordonnées GPS du centre, et l'orientation de la piste (voir fonction "void calculs_GPS()" )
on va alors ajouter la valeur de l'azimut_i à l'orientation de la piste pour faire le calcul
*/

// calcul de l'orientation (relevé) du point (azimut par rapport au nord)
	float orientation_point = -1.0 * orient_pisteAB + azimut_i; //azimut_i étant l'angle entre la piste et la direction du point

// 1 minute d'angle sur un méridien => 1 NM de latitude
// 60mn d'angle (1deg) => 60 NM
// 1 NM => 1/60 de degré -> 0.0166 degrés
	
	*latitude = lat_centre_pst + (distance_i /60.0 * sin(raddeg * (orientation_point - 90.0)));
	
// pour la longitude, il faut tenir compte que la longueur d'un parallèle dépend de la latitude 
// (max à l'équateur, nulle au pôle) suivant une loi en cos.
	*longitude= lon_centre_pst + (distance_i /(60.0 * cos(raddeg * lat_centre_pst)) * cos(raddeg * (orientation_point - 90.0)));

}

void find_END_RWY_dst() //le pt le plus ELOIGNE en face de nous, en bout de piste (= A ou B )
{
	
// calcul basé sur les distances
// en vue de guider (en lacet) l'avion au roulage lors de l'atterrissage
// on visera le point le plus éloigné
// attention: lors d'un touch and go, si l'avion a dépassé le centre de la piste lors de la remise des gaz, le sens sera FAUX !
 
	float lat_A=liste_bali[num_bali].lat_A;
	float lon_A=liste_bali[num_bali].lon_A;
	float lat_B=liste_bali[num_bali].lat_B;
	float lon_B=liste_bali[num_bali].lon_B;
	
	float dst_A = distance_AB(lat_avion, lon_avion, lat_A, lon_A);
	float dst_B = distance_AB(lat_avion, lon_avion, lat_B, lon_B);
	
	if((dst_A) > (dst_B)) {extremite_pst='A';} else {extremite_pst='B';}
}


void find_sens_approche() // en fonction de la position réelle de l'avion
{
//détermination du sens de l'approche pour l'autoland (en vol)

	////float delta_1 = orient_pisteBA - GPS_azimut_piste;
	////if (delta_1<0) {delta_1+=360.0;}
	////if (delta_1>360) {delta_1-=360.0;}
	
	////if ((delta_1 >90.0) && (delta_1 <270.0)) {sens_app_effectif = sens_AB;} else {sens_app_effectif = sens_BA;}
	find_END_RWY_dst();

	if(extremite_pst=='A') {sens_app_effectif = sens_BA;}
	if(extremite_pst=='B') {sens_app_effectif = sens_AB;}
}






void find_END_RWY_angl() //le pt le plus ELOIGNE en face de nous, en bout de piste (= A ou B )
{
// calcul par les angles
// en vue de guider (en lacet) l'avion au roulage lors du décollage
// on visera le point le plus éloigné

	
	float delta = cap - orient_pisteAB;
	
	if (delta < -180) {delta += 360;}
	if (delta >  180) {delta -= 360;}

	if(abs(delta) > 90) {extremite_pst='A';} else {extremite_pst='B';}
	
//affi_string_test((String)extremite_pst, 130, 4, BLANC, NOIR);
}

// =============================================================================================================================




void nav_to_centre_piste()
{
	voyant_APP.affiche(BLANC, BLEU);
	hdg1 = round(GPS_azimut_piste);
	if(GPS_distance_piste < 2) // on désengage tout, il faut un appui sur touche pour décider de la suite du vol
	{
		raz_bit(&flags, bit_nav_to_piste);// ce qui signe la fin des appel de cette fonction
		raz_bit(&flags, bit_nav_to_ptAA);
		raz_bit(&flags, bit_nav_to_ptBB);
		raz_bit(&flags, bit_route);
		raz_bit(&flags, bit_autoland);
		raz_bit(&flags, bit_atterrissage);
		//raz_bit(&flags, bit_au_sol);
		raz_bit(&flags, bit_decollage);

		/*
		for (int n=0; n<4; n++)
		{
			TFT480.setFreeFont(FF6);
			//affi_message("verticale RWY", 130, 200, 200, BLEU_CLAIR, HA_SOL, 1); // ici
		}
		*/

// rien de plus, on repasse en auto-pilotage manuel
	
		set_bit(&flags, bit_FG_AP);
	} 
}

void poste_warning(String texte_i)
{
	if(WARNING != texte_i)
	{
		WARNING = texte_i;
		tempo_message=5;
	}
}

void nav_to_ptAA() // on passera en boucle dans cette fonction
{
	poste_warning("nav to AA");
// point situé à 12NM dans l'axe de la piste (d'un côté)

	voyant_APP.affiche(BLANC, VIOLET1);

// CAP	
	hdg1 = round(GPS_azimut_ptAA);

	if ((GPS_distance_ptAA < 80) && (asel1 > 100)) {asel1 = 100;}
	if ((GPS_distance_ptAA < 40) && (asel1 > 60)) {asel1 = 60;}

	uint16_t asel_mini = liste_bali[num_bali].niveau_de_vol_mini;
	if ((GPS_distance_ptAA < 30) && (asel1 < asel_mini)) {asel1 ++;}
	// force à garder une hauteur minimale de sécurité le cas échéant (relief...)

	uint16_t asel_mini2 = (gnd_elv + 1600) /100;
	if(asel1 < asel_mini2) {asel1 = asel_mini2;} // remonte si trop bas / sol

	affi_distance_ptAA();

	if(GPS_distance_ptAA < 3.0)
	{
		raz_bit(&flags, bit_nav_to_piste);
		raz_bit(&flags, bit_nav_to_ptAA); // ce qui signe la fin des appel de cette fonction
		raz_bit(&flags, bit_nav_to_ptBB);
		raz_bit(&flags, bit_route);
		//efface_cadre_bas(NOIR);

		if (asel1 > 30) {asel1 = 30;}

		TFT480.setFreeFont(FF6);

		poste_warning("Finale");
			
		efface_cadre_bas(NOIR);
		find_END_RWY_dst(); // la plus éloignée (= 'A' ou = 'B')
		set_bit(&flags, bit_autoland); // on passe en finale
		//set_bit(&flags, bit_rudder_attero);
	}

// vitesse

	if ( (GPS_distance_piste < 30.0) && (target_speed>160) ) {target_speed =160;}
}



void nav_to_ptBB() // on passera en boucle dans cette fonction
{
	poste_warning("nav to BB");

// point situé à 12NM dans l'axe de la piste (de l'autre côté)

	voyant_APP.affiche(BLANC, VIOLET2);

// CAP	
	hdg1 = round(GPS_azimut_ptBB);

	if ((GPS_distance_ptBB < 80) && (asel1 > 100)) {asel1 = 100;}
	if ((GPS_distance_ptBB < 40) && (asel1 > 60)) {asel1 = 60;}

	uint16_t asel_mini = liste_bali[num_bali].niveau_de_vol_mini;
	if ((GPS_distance_ptBB < 30) && (asel1 < asel_mini)) {asel1++;} 
	// force à garder une hauteur minimale de sécurité le cas échéant (relief...)

	uint16_t asel_mini2 = (gnd_elv + 1600) /100;
	if(asel1 < asel_mini2) {asel1 = asel_mini2;} // remonte si trop bas / sol

	affi_distance_ptBB();

	if(GPS_distance_ptBB < 3.0)
	{
		raz_bit(&flags, bit_nav_to_piste);
		raz_bit(&flags, bit_nav_to_ptAA);
		raz_bit(&flags, bit_nav_to_ptBB); // ce qui signe la fin des appel de cette fonction
		raz_bit(&flags, bit_nav_to_pti);
		raz_bit(&flags, bit_circling);
		raz_bit(&flags, bit_route);
		
		if (asel1 > 30) {asel1 = 30;}

		//TFT480.setFreeFont(FF6);
		////affi_message("proche ptB", BLEU_CLAIR, HA_SOL, 1); // ici

		poste_warning("Finale");
		
		efface_cadre_bas(NOIR);
		find_END_RWY_dst(); // la plus éloignée (= 'A' ou = 'B')
		
		set_bit(&flags, bit_autoland); // on passe en finale
		//set_bit(&flags, bit_rudder_attero);
	}
// vitesse

	if ( (GPS_distance_piste < 30.0) && (target_speed>160) ) {target_speed =160;}
}


void nav_to_pti()
{
	String s1;
// point quelconque
	voyant_APP.affiche(BLANC, VERT_FONCE);

// CAP	
	hdg1 = round(GPS_azimut_pti);


	affi_distance_pti();

	if(GPS_distance_pti > 1.5) {inc_num_pt1_autorisee=1;} 


	if(GPS_distance_pti < 1.0)
	{
		TFT480.setFreeFont(FF6);
		//s1 ="PT "; 
		s1 = "to PT " + String(num_pti);
		poste_warning(s1);


		if (inc_num_pt1_autorisee==1)
		{
			num_pti ++; // pour naviguer vers le point suivant
			if (num_pti >10)
			{
				num_pti =1;
			}

			asel1 = 30; //à priori.  niveau de vol (en ft/100)

			if (num_pti==1){asel1 = 3;  flaps=3; }  // 300ft  -> 100m
			if (num_pti==2){asel1 = 15; flaps=2;  } // 1500ft -> 500m
			if (num_pti==3){asel1 = 30; flaps=0;  } // 3000ft -> 1000m
			if (num_pti==4){asel1 = 30; flaps=0;  }
			if (num_pti==5){asel1 = 30; flaps=0;  }
			if (num_pti==6){asel1 = 30; flaps=0;  }
			if (num_pti==7){asel1 = 30; flaps=0;  }
			if (num_pti==8){asel1 = 30; flaps=0;  }
			if (num_pti==9){asel1 = 20; flaps=2;  }
			if (num_pti==10){asel1 =10; flaps=3;  }
		
			inc_num_pt1_autorisee =0; // pour éviter d'incrémenter plusieurs fois lorsqu'on est proche du point
		} 
	}
}



void tour_de_piste()
{

	poste_warning("tour de piste");

	
// cheminement entre points dont la position est définie par un vecteur partant du centre de la piste (angle & distance)	
// num_pti est incrémenté dans la fonction 'nav_to_pti()' lorsque le point en cours est atteint


	float dst;
	float alpha;
	uint8_t n2=0;

	if (extremite_pst == 'A') {n2 = num_pti;}
	if (extremite_pst == 'B') {n2 = 11-num_pti;} //même trajectoire parcourue en sens inverse

	if (n2 ==1) {alpha= 0; dst = 1.0;}
	if (n2 ==2) {alpha= 0; dst = 4.0;}
	if (n2 ==3) {alpha= 16; dst = 5.1;}
	if (n2 ==4) {alpha= 46; dst = 4.0;}
	if (n2 ==5) {alpha= 72.3; dst = 3.1;}
	if (n2 ==6) {alpha= 109; dst = 3.1;}
	if (n2 ==7) {alpha= 135; dst = 4.0;}
	if (n2 ==8) {alpha= 163; dst = 5.1;}
	if (n2 ==9) {alpha= 180; dst = 4.0;}
	if (n2 ==10){alpha= 180; dst = 1.0;}

	dst *= 1.5; // taille de la figure


	if (read_bit(flags, bit_sens_circling) == 1) {alpha = 360-alpha;} // trajectoire miroir

	calcul_pti(alpha, dst, &lat_pti, &lon_pti);
	
// variante :	
	//calcul_pti(30.0*num_pti, 10.0, &lat_pti, &lon_pti); // points disposés en cercle, à 10NM
	
}





void calcul_erreur_position() // pour savoir si l'avion se trouve exactement dans l'axe de la piste
{
	float x, y;
	float x1, x2;
	float y1, y2;
	float p;
	float s;
	//float erreur;

	x=lon_avion;
	y=lat_avion;

	x1 = liste_bali[num_bali].lon_A;	x2 = liste_bali[num_bali].lon_B;
	y1 = liste_bali[num_bali].lat_A;	y2 = liste_bali[num_bali].lat_B;

	p = (y2-y1) / (x2-x1);  // pente de la droite A-B
	
	s= y1+ p * (x-x1);

	erreur_axe = y-s;
	
	//affi_float_test(erreur_axe, 110, 2, BLANC, BLEU); // pour test
	
}



void desengage_autoland()
{
	poste_warning("autoland OFF");

	
	raz_bit(&flags, bit_autoland);
	efface_cadre_bas(NOIR);
	//init_affi_HA();
	
	voyant_L.affiche(BLANC, GRIS_FONCE);
	voyant_G.affiche(BLANC, GRIS_FONCE);
/*
	target_speed =180;
	locks_type = "ALT";
	asel1 = 30; // consigne altitude 30 -> 3000ft
	climb_rate=0; // taux de montée (négatif pour descendre - sert pour attérissage automatique)
	hdg1 = cap;
	RAZ_chrono();
*/
}


void affiche_etats_flags() // certains "voyants" en haut à gauche
{
	if (read_bit(flags, bit_rudder_decol) == 1)	{ voyant_RD.affiche(NOIR, VERT);}
	else if (read_bit(flags, bit_rudder_attero) == 1){ voyant_RD.affiche(NOIR, JAUNE); }
	else { voyant_RD.affiche( BLANC, GRIS_TRES_FONCE); }

	if (read_bit(flags, bit_nav_to_piste) == 1) {voyant_route.affiche(BLANC, BLEU);}
	else if (read_bit(flags, bit_nav_to_ptAA) == 1) {voyant_route.affiche(NOIR, JAUNE);}
	else if (read_bit(flags, bit_nav_to_ptBB) == 1) {voyant_route.affiche(NOIR, JAUNE);}
	
	else {voyant_route.affiche(BLANC, GRIS_TRES_FONCE);}

	if (read_bit(flags, bit_atterrissage)==1){voyant_ATT.affiche(NOIR, VERT);}
	else {voyant_ATT.affiche(BLANC, GRIS_TRES_FONCE);}
}



void affi_ligne1_V(uint16_t x)
{
/**	DOC: (source : "TFT_eSPI.h")
// The next functions can be used as a pair to copy screen blocks (or horizontal/vertical lines) to another location

// Read a block of pixels to a data buffer, buffer is 16 bit and the size must be at least w * h
void readRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data);
  
// Write a block of pixels to the screen which have been read by readRect()
void pushRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data);
**/

	TFT480.pushRect(memo_x1, 0, 1, 320, data_C1); // efface la ligne en replaçant l'image
	memo_x1=x;
	
	TFT480.readRect(x, 0, 1, 320, data_C1); // memorisation de la ligne avant de tracer dessus
	//TFT480.drawFastVLine(x, 0, 320, ROUGE);
	TFT480.drawFastVLine(x, y_1, y_2-y_1, JAUNE);
}



void affi_ligne2_V(uint16_t x)
{
	TFT480.pushRect(memo_x2, 0, 1, 320, data_C2); // efface la ligne en replaçant l'image
	memo_x2=x;
	
	TFT480.readRect(x, 0, 1, 320, data_C2); // memorisation de la ligne avant de tracer dessus
	//TFT480.drawFastVLine(x, 0, 320, ROUGE);
	TFT480.drawFastVLine(x, y_1, y_2-y_1, JAUNE);
}



void affi_ligne1_H(uint16_t y)
{
	TFT480.pushRect(0, memo_y1, 480, 1, data_L1); // efface la ligne en replaçant l'image
	memo_y1=y;
	
	TFT480.readRect(0, y, 480, 1, data_L1); // memorisation de la ligne avant de tracer dessus
	//TFT480.drawFastHLine(0, y, 480, ROUGE);
	TFT480.drawFastHLine(x_1, y, x_2-x_1, JAUNE);
}


void affi_ligne2_H(uint16_t y)
{
	TFT480.pushRect(0, memo_y2, 480, 1, data_L2); // efface la ligne en replaçant l'image
	memo_y2=y;
	
	TFT480.readRect(0, y, 480, 1, data_L2); // memorisation de la ligne avant de tracer dessus
	//TFT480.drawFastHLine(0, y, 480, ROUGE);
	TFT480.drawFastHLine(x_1, y, x_2-x_1, JAUNE);
}


// =============================================================================================================================


void prepare_decollage()
{
	init_affi_HA();
	
	raz_bit(&flags, bit_atterrissage);
	raz_bit(&flags, bit_FG_AP);
	raz_bit(&flags, bit_autoland);
	raz_bit(&flags, bit_route);
	
	//set_bit(&flags, bit_nav_to_piste); // par défaut
	
	raz_bit(&flags, bit_nav_to_pti);
	raz_bit(&flags, bit_nav_to_ptAA);
	raz_bit(&flags, bit_nav_to_ptBB);
	raz_bit(&flags, bit_roulage);
		
	locks_type = "ALT";
	asel1 = 60;
	target_speed = 200;

	find_END_RWY_angl();
	
	set_bit(&flags, bit_au_sol);
	set_bit(&flags, bit_rudder_decol);
	
	hdg1 = cap;
	RAZ_chrono();
	
	set_bit(&flags, bit_decollage);
	
	flaps = 3; // sortie des volets
	
	poste_warning("flaps 15");

	//affi_message(WARNING, BLANC, GRIS_TRES_FONCE);
	
	landing_light1=1;
	landing_light2=1;
	//WARNING= "decollage";
	

}



void auto_rudder_deco(float correction) 
{
	//WARNING="auto rudder";
// losrqu'on est bien positionné sur la piste, on doit voir l'extrémité de la piste, en face, au loin
// dans la même direction que l'orientation physique de la piste	

	float d_alpha;
	float lat_i, lon_i;

	affi_extremite(); // l'extrémité concernée est déterminée par la fonction "find_END_RWY_angl()"

	if (extremite_pst == 'A')
	{
		lat_i=liste_bali[num_bali].lat_A;
		lon_i=liste_bali[num_bali].lon_A;
	}
	
	if (extremite_pst == 'B')
	{
		lat_i=liste_bali[num_bali].lat_B;
		lon_i=liste_bali[num_bali].lon_B;
	}
	
	float az1 = azimut_AB(lat_avion, lon_avion, lat_i, lon_i); // direction dans laquelle on voit le bout de la piste au loin...

	d_alpha = az1 - cap; 

// le débattement doit augmenter fortement lorsque la roue avant ne touche plus le sol
// et que seule la dérive a une action (aérodynamique) (vers 110 kts)

	float facteur1 = 0.03;  // 0.1->zig-zag ; agit sur l'orientation de l'avion
	float facteur2 = -0.04 * vitesse; //-0.05 // agit sur la position de l'avion / axe de la piste

	if (vitesse < 110)  { facteur2 = -0.05;} // <110 ; -0.05// sinon se met à zig-zaguer grave tant que la roue avant touche le sol 

	//if (is_in(vitesse, 80, 175)) {facteur1 = 15;} // on augmente fortement l'amplitude de cette correction
	 
	rudder = (d_alpha * facteur1) + (correction * facteur2);
	borne_in (&rudder, -0.2, 0.2);
	
	if (vitesse < 20)  { rudder =0;}
	if (vitesse > 175) { rudder =0;}  // à voir !!!
}




void decollage()
// on passera en boucle dans cette fonction ; voir aussi la fonction "prepare_decollage()"
{
	asel1 = liste_bali[num_bali].niveau_de_vol_mini;

	speedbrake =0;
	reverser1 = 0;
	reverser2 = 0;
	throttle = -0.95;

	brake_left =0;
	brake_right =0;
	landing_light1=1;
	landing_light2=1;
	
	trim_elevator = -0.3; // bonne valeur pour décoller
	//if (hauteur_AAL >10){trim_elevator=-0.25;} // pour ne pas grimper aux arbres
	//if (hauteur_AAL >15){trim_elevator=-0.2;}


// -------------------- AZIMUT -----------------------------------------------------------------

	float op1;
	float delta_AZM;
	
	find_sens_approche();
	affi_sens_APP(); // en haut à droite

	if (sens_app_effectif == sens_AB)
	{
		op1 = orient_pisteAB;
		delta_AZM =  op1 -GPS_azimut_ptB;
	}
	if (sens_app_effectif == sens_BA)
	{
		op1 = orient_pisteBA;
		delta_AZM =  op1 -GPS_azimut_ptA;
	}
	
	auto_rudder_deco(delta_AZM);
	
	ailerons= -roulis/5.0; // 1.2 //asservissement des ailerons en fonction de l'angle de roulis (-> ailes à plat)

// ----------------------------------------------------------------------------------------------	

	if (vitesse > 140){trim_elevator=-0.25;} // pour ne pas grimper aux arbres
	if (vitesse > 150){trim_elevator=-0.2;}
	
	affi_elevator();

	if (vitesse > 120)
	{
		poste_warning("  V1  ");
	}	


	if (is_in(hauteur_AAL, 5, 20))
	{
		poste_warning("flaps 15");
		flaps = 3;
	}


	if (is_in(hauteur_AAL, 20, 40))
	{
		poste_warning("gear UP");
		gear_down =0;
	}


	if (is_in(hauteur_AAL, 40, 50))
	{
		poste_warning("flaps 10");
		flaps = 2;
	}	


	if (is_in(hauteur_AAL, 50, 70))
	{
		poste_warning("flaps 5");
		flaps = 1;
	}

	
	if (is_in(hauteur_AAL, 70, 80))
	{
		poste_warning("flaps 0");
		flaps = 0;
	}	


	if (hauteur_AAL >= 80) 
	{
		set_bit(&flags, bit_FG_AP);  // engage Autopilot de FlightGear
		poste_warning("AP");
		speed_ctrl=true;

		raz_bit(&flags, bit_rudder_decol);
		//raz_bit(&flags, bit_au_sol);
		rudder=0;
		gear_down = 0;
		landing_light1=0;
		landing_light2=0;
		raz_bit(&flags, bit_decollage); // fin des appels de cette fonction
	}

	
}



// =============================================================================================================================

void auto_rudder_attero(float correction) // on passera en boucle dans cette fonction
{
// losrqu'on est bien orienté sur la piste, on doit voir l'extrémité de la piste, en face, au loin
// dans la même direction que l'orientation physique de la piste	

	float d_alpha;
	float lat_i, lon_i;

	affi_extremite(); // déterninée en une seule fois lors de la fin de la phase d'autoland
	// voir dans la fonction "void auto_landing()"
	// ne plus la re-déterminer par la suite parce qu'une fois dépassé le centre de la piste, le résultat serait faux !

	if (extremite_pst == 'A')
	{
		lat_i=liste_bali[num_bali].lat_A;
		lon_i=liste_bali[num_bali].lon_A;
	}
	
	if (extremite_pst == 'B')
	{
		lat_i=liste_bali[num_bali].lat_B;
		lon_i=liste_bali[num_bali].lon_B;
	}
	
	float az1 = azimut_AB(lat_avion, lon_avion, lat_i, lon_i); // direction dans laquelle on voit le bout de la piste au loin...

	d_alpha = az1 - cap; // orientation de l'avion vers le bout de la piste

	borne_in (&d_alpha, -3.0, 3.0);  //  -3  3

	////if (is_in(vitesse, 100,  140)) {rudder = d_alpha / 20.0;}
	////else if (is_in(vitesse, 80,  100)) {rudder = d_alpha / 25.0;} //30
	////else if (is_in(vitesse, 50,  80)) {rudder = d_alpha / 30.0;}  //40
	////else if (is_in(vitesse, 20,  50)) {rudder = d_alpha / 50.0;}  //80

	float facteur1 = -0.26 * vitesse +65.2; // agit sur l'orientation de l'avion
	float facteur2 = -0.5; // agit sur la position de l'avion / axe de la piste

	if (vitesse < 80)  { facteur2 = 0;} // sinon se met à zig-zaguer grave lorsque la roue avant touche le sol 

// le débattement doit être important lorsque la roue avant ne touche pas le sol
// et que seule la dérive a une action (aérodynamique) (> 80 kts)

	if (is_in(vitesse, 80, 175)) {facteur1 = 15;} // on augmente fortement l'amplitude de cette correction
	 
	rudder = (d_alpha / facteur1) + (correction * facteur2); 

	if (vitesse > 175) { rudder = 0;}
	if (vitesse < 10)  { rudder = 0;}
	
//raz_bit(&flags, bit_rudder_attero); // afin de pouvoir manoeuvrer sur les taxiways
	
// OK, garde l'axe de la piste, heu... lorsque la roue avant touche le sol...
// lorsque seul le train principal touche et la vitesse est faible et donc la gouverne de direction peu efficace... pas top !
// on pourrait jouer en différentiel sur les freins gauche-droite, mais ça complique pas mal l'affaire !
// toutefois si on freine rapidement (dans la seconde qui suit le toucher initial) la roue avant touche à son tour, et c'est OK

}


void affi_localizer(float valeur_i)
{
//ILS (maintenant GPS) dans le plan horizontal; affiche l'erreur de position par rapport à l'axe de la piste


//affi_float_test(valeur_i, 110, 3, JAUNE, NOIR); // pour test

	uint16_t y1 = HA_y0-HA_h-14;

	uint16_t couleur1 = ROSE;
	
	loc = HA_x0 + valeur_i; // sachant que HA_x0 par définition est le centre de l'horizon artificiel (et pas le bord de droite)
	
	if ( loc < (HA_x0-HA_w+5)) {loc = HA_x0-HA_w+5; couleur1 = GRIS;}
	if ( loc > (HA_x0+HA_w-5)) {loc= HA_x0+HA_w-5; couleur1 = GRIS;}


	TFT480.fillRect(HA_x0-HA_w, y1, 2*HA_w, 9, GRIS_TRES_FONCE);
	TFT480.drawLine(HA_x0, y1-5, HA_x0, y1+5, BLANC);

	affi_indexV(loc, y1, 1, couleur1); // petit triangle rose en haut, se déplaçant horizontalement

	memo_loc=loc;
}

void affi_index_lateral(uint16_t position_i)
{
// petits triangles roses de chaque côtés du PFD
// (à mi-hauteur du PFD si =0)

	uint16_t x1 = 75;
	uint16_t x2 = 332;

	uint16_t position_V = HA_y0 - position_i;

	TFT480.fillRect(x1, 30, 9, 2*HA_h, GRIS_TRES_FONCE); // efface
	TFT480.fillRect(x2, 30, 9, 2*HA_h, GRIS_TRES_FONCE); // efface

	TFT480.drawRect(x1, HA_y0, 12, 5, BLANC);
	TFT480.drawRect(x2, HA_y0, 12, 5, BLANC);

	uint16_t couleur1 = ROSE;
	if ( position_V < (HA_y0-HA_h+5)) {position_V = HA_y0-HA_h+5; couleur1 = GRIS;}
	if ( position_V > (HA_y0+HA_h-5)) {position_V = HA_y0+HA_h-5; couleur1 = GRIS;}

	affi_indexH(x1, position_V, 1, couleur1); 
	affi_indexH(x2+8, position_V, -1, couleur1); 		
}



void auto_landing()  // approche et finale
{
// on passera en boucle dans cette fonction	
/**
 voir: https://en.wikipedia.org/wiki/Autoland

 Approche automatique
 CAPTURE l'avion et le POSE !

 LES CONSEILS QUI SUIVENT ne concernent que l'utilisation du simulateur de vol FlightGear connecté aux ESP32
 et le choix du Citation X comme avion.
 c'est à dire qu'ils ne doivent en aucun cas servir pour le pilotage d'un avion réel.
 
 
 -vitesse conseillée : 140kts, 160kts max
 -distance conseillée : entre 20 et 10 nautiques 
 -avec une trajectoire qui recoupe l'axe de la piste, < 90°
 (si capture à moins de 10 NM, la trajectoire sera difficilement corrigée -> remise des gaz ou crash au choix !)
 -hauteur conseillée : 3000ft à 10NM (= niveau 30)

 -volets sortis 2 puis 3
 à priori pas d'AF si vitesse correcte

 -sortir le train !
 -allumer feux d'atterrissage

 notes: l'autopilot se désengage automatiquement (par FlightGear) sous 100ft de hauteur
 (réglable, voir la variable 'hauteur_mini_autopilot' au début de ce programme)

ce qui suit est actuellement devenu automatique dans les nouvelles versions :
 
 (Donc garder le manche en main pour l'arrondi et le touché final, en surveillant
 - la hauteur
 - la vitesse
 - le pitch
 - position des volets
 - éventuellement petit coup d'AF (aérofreins -> CTRL + B au clavier)
 - si piste très courte, inverseurs de poussée (au sol) + gaz (touche 'suppr')
 - toutefois si approche visiblement trop courte ou trop longue, pas d'attéro kamikaze ! -> remise des gaz !!
 - si système visuel "papi" présent, le respecter !!

 TOUTEFOIS :
 - on peut obtenir un posé 100% auto en anticipant un cabrage de l'avion pour faire l'arrondi dans les règles de l'art
 avec posé du train principal en premier, puis ensuite la roulette de nez. Pour cela :
 - en fin de finale, à une hauteur de 100 feet, dès le désengagement de l'autopilot de FlightGear:
 - ailes à plat !
 -gaz au mini et sortir les AF. La vitesse diminue, et l'avion se cabre un peu
 pour ne pas plonger... et il finit par poser le train principal.
 - dès que ce touché est fait, freiner légèrement -> la roue avant va alors à son tour toucher la piste, ce qui permet
 à l'auto-rudder de guider la trajectoire suivant l'axe de la piste.
 ( Tant que la roue avant ne touche pas, l'auto-rudder, qui n'agit alors qu'aérodynamiquement sur la gouverne de direction,
 n'est pas assezefficace).
 
 Le tout suivi d'un freinage, on pose avec arrêt complet sur 850m (sans faire craquer la structure...).
 Avec les inverseurs de poussée, on doit pouvoir faire bien mieux encore. Quant au porte-avion, il est normalement
 équipé d'un câble de retenue qu'on accroche avec une perche (sur un jet militaire, sans doute pas avec notre Cessna Citation X)
 
**/

	String s1;
	float alti1;
	float GPS_distance_seuil_piste;

	TFT480.setFreeFont(FSS9);
	
	TFT480.setTextColor(GRIS_FONCE, NOIR);
	//TFT480.fillRect(180, 0, 20, 16, JAUNE); // JAUNE pour test. efface 1/2 le bandeau d'information en haut

	voyant_L.affiche(BLANC, GRIS_TRES_FONCE);

	voyant_G.affiche(BLANC, GRIS_TRES_FONCE);
	
	voyant_APP.affiche(BLANC, GRIS_TRES_FONCE);

	uint8_t AZ =0; // flag azimut OK
	uint8_t GL =0; // flag glide OK

	//voyant_descente_GPS.affiche(BLANC, GRIS_TRES_FONCE);

	
//--------------------- (si autoland engagé, sinon on ne fait rien de plus)--------------

	//TFT480.fillRect(HA_x0-40, HA_y0+80, 87, 15, HA_SOL); // efface "APP"
	
	if (read_bit(flags, bit_autoland) == 1)
	{
		calculs_GPS();
		
		if (GPS_distance_piste > 25.0)
		{
			return; // rien de plus
		}
		else
		{
			//WARNING = "autoland ON";
//affi_float_test(liste_bali[num_bali].orient_pisteAB,110, 2, BLANC, BLEU); // pour test
//affi_float_test(GPS_azimut_piste,110, 3, BLANC, GRIS_FONCE); // pour test

			voyant_APP.affiche(NOIR, JAUNE);

			
// -------------------- AZIMUT -----------------------------------------------------------------

			float op1;
			find_sens_approche();
			affi_sens_APP(); // en haut à droite

			if (sens_app_effectif == sens_AB) {op1 = orient_pisteAB;}
			if (sens_app_effectif == sens_BA) {op1 = orient_pisteBA;}
			//s1 = String(op1,1);
			//TFT480.drawString(s1, 0, 250);
			
			
			float delta_AZM =  op1 -GPS_azimut_piste;

//affi_float_test(delta_AZM,110, 2, VERT, GRIS_FONCE); // pour test
		
			delta_AZM *= 20.0;
			borne_in(&delta_AZM, -35.0, 35.0);
			
			//if((delta_AZM >-10.0)&&(delta_AZM <10.0))
			if (is_in(delta_AZM, -10.0, 10.0)==1)
			{
				voyant_L.affiche(NOIR, VERT);
				AZ=1;
			}

			affi_localizer(delta_AZM * -2.5);
			
			hdg1 = round(op1 - delta_AZM);
			
			
//affi_float_test(hdg1,110, 5, VERT, GRIS_FONCE); // pour test

// -------------------- VITESSE -----------------------------------------------------------------

			if ( (GPS_distance_piste < 20.0) && (target_speed>180) ) {target_speed =180;}
			if ( (GPS_distance_piste < 10.0) && (target_speed>170) ) {target_speed =170;}
			if ( (GPS_distance_piste <  5.0) && (target_speed>160) ) {target_speed =160;}
			if ( (GPS_distance_piste <  2.0) && (target_speed>140) ) {target_speed =140;}
			//if ( (GPS_distance_piste <  1.0) && (target_speed>130) ) {target_speed =130;}

			if ((vitesse - target_speed) > 4  ) {speedbrake = 1.0;} else {speedbrake = 0;}
			
// -------------------- HAUTEUR -----------------------------------------------------------------

			//voyant_descente_GPS.affiche(NOIR, JAUNE);
			float  longueur_piste_NM = longueur_piste / 1852.0;
			GPS_distance_seuil_piste = GPS_distance_piste - (longueur_piste_NM/2.0); // + 0.1; 
// Rappel : "GPS_distance_piste" est la distance au point CENTRAL de la piste

			TFT480.setFreeFont(FM12);
			TFT480.setTextColor(BLANC);
			s1=String(GPS_distance_seuil_piste, 1);
			s1+= " NM";
			TFT480.fillRect(150, 300, 100, 25, GRIS_AF); // efface
			TFT480.drawString(s1, 150, 300); 

			
//affi_float_test(GPS_distance_seuil_piste, 110, 2, BLANC, BLEU); // pour test
			
			float alti_correcte = liste_bali[num_bali].altitude + 300.0 * GPS_distance_seuil_piste;
			if (alti_correcte > 3000) {alti_correcte = 3000;}			

			affi_asel(alti_correcte);
			
// soit 3000ft pour 10 nautiques -> pente 5%
//sachant que la ref de position est située au milieu de la longueur de la piste
			
//affi_float_test(alti_correcte, 110, 2, BLANC, BLEU); // pour test

			float erreur_altitude =  altitude_GPS - alti_correcte;
//affi_float_test(erreur_altitude, 110, 3, BLANC, BLEU); // pour test

			if((erreur_altitude > -20)&& (erreur_altitude < 20))
			{
				//voyant_G.affiche(NOIR, VERT);			
			}
			
			affi_index_lateral( - erreur_altitude / 3.0); // affiche les triangles roses latéraux
			
/**Rappels :
1 NM (nautical mile ou 'nautique') = 1852 m
1 feet = 0,3048 m
1 NM = 1852/0.3048 = 6076.12 feet
1 noeud (nd) = 1NM/h = 1852/3600 =  0.51444 m/s

début de descente (5%) vers la piste : 3000ft à 10NM, vitesse 150 nd (par exemple)
v=150*0.51444 = 77.17m/s
temps pour parcourir la distance : v=d/t
t=d/v = 10*1852m / 77.17 = 240 s (soit 4 minutes)

taux de descente = 3000ft/240s = 12.5 fps
**/			
			if ((GPS_distance_piste < 10) && (hauteur_AAL > 1500)) //  && (hauteur_AAL <= 6000)
			{
// initialisation de l'approche auto (palier puis descente)
				//voyant_descente_GPS.affiche(NOIR, VERT);
				
				locks_type = "VS"; // bascule le pilote auto de FG en mode vertical speed
				
				//climb_rate = -5.0;
				
			}

			//if ((GPS_distance_piste < 30) && (hauteur_AAL < (liste_bali[num_bali].niveau_de_vol_mini * 100) ) )
			if ((GPS_distance_piste < 10) && (hauteur_AAL < 8000) )
			{
// descente				
// correction du taux de descente (climb_rate) pour respecter la pente à 3° (=5%)
				voyant_G.affiche(NOIR, VERT);
				GL=1;
				
				//if (erreur_altitude > 4)  {climb_rate -= 5; }
				//if (erreur_altitude < -4) {climb_rate += 5; }

				climb_rate = erreur_altitude * -1.5;
				
				if (climb_rate > +30) {climb_rate = +30;}
				if (climb_rate < -50) {climb_rate = -50;}
				
//affi_float_test( erreur_altitude, 110, 4, NOIR, JAUNE); // pour test						
			}

// -------------------- FLAPS -----------------------------------------------------------------		

			if (is_in(GPS_distance_piste, 6, 8))
			{
				poste_warning("flaps 15");
				
				flaps = 2;
				landing_light1=1;
				landing_light2=1;
			}


			if (is_in(GPS_distance_piste, 5, 6))
			{
				poste_warning("gear down");
				gear_down = 1;
			}


			if (is_in(GPS_distance_piste, 2, 5))
			{
				flaps = 3; // participe grandement au freinage (l'asservissement précis des gaz maintiendra la bonne vitesse)
				poste_warning("flaps 20");
			}
			


			if (GPS_distance_piste <= 2)
			{
				poste_warning("flaps 35");
				flaps = 4;
			}


// -------------------- FINALE -----------------------------------------------------------------

			if (hauteur_AAL < hauteur_mini_autopilot) // signe la fin des appels de cette fonction
			{

				raz_bit(&flags, bit_rudder_decol);
				raz_bit(&flags, bit_nav_to_pti);
				raz_bit(&flags, bit_nav_to_ptAA);
				raz_bit(&flags, bit_nav_to_ptBB);
				raz_bit(&flags, bit_route);
				
				throttle = 1.0; // gaz au minimum
				//target_speed = 120;
				
				if (read_bit(flags, bit_att_short) == 1) {target_speed = 100;} else {target_speed = 120;}
				//trim_elevator = 0.0;				

				ailerons=0;
				raz_bit(&flags, bit_FG_AP);
				desengage_autoland(); // donc on ne repassera plus dans la fonction (ici)

				brake_left =0;
				brake_right =0;
				
				find_sens_approche(); //NE DOIT plus etre calculé ultérieurement! (lorsqu'on a dépassé le centre de la piste) !
				
				//set_bit(&flags, bit_rudder_attero); // gouverne de lacet en mode automatique
				set_bit(&flags, bit_atterrissage);  // on passera dorénavant en boucle dans la fonction "atterrissage()"

				affiche_etats_flags();
				//WARNING= "end autoland";
			}

			if( (AZ==1) && (GL==1) ) {voyant_APP.affiche(NOIR, VERT);}

/**			
alti1 = 3.0*GPS_distance_piste + gnd_elv/100.0 -1;
if (alti1 < asel1) //empêche de (re)monter lors de la capture ILS, reste en palier le cas échéant
{
asel1 = alti1; 
}
**/
		}
		
// =============================================================================================================================
	}		
}



void atterrissage()
// on passera en boucle dans cette fonction
{
	// premier terme = léger cabré pour l'arrondi
	// deuxième terme diminue ce cabré tant que la hauteur est grande
	// troisième terme = asservissement de l'angle de tangage de façon à stabiliser l'ensemble
	//trim_elevator = -0.5+ (float)hauteur_AAL/1500.0 + (tangage / 20.0);
	//trim_elevator = -0.3 + (tangage / 20.0);

	calculs_piste();
	calculs_GPS();

	float H0 = hauteur_AAL;
	if (H0<0) {H0 = 0;}

	locks_type = "VS"; // pilote auto de FG en mode vertical speed

	// -------------------- TANGAGE -----------------------------------------------------------------

	//if (read_bit(flags, bit_att_short) == 1) {climb_rate = -5.0;} else {climb_rate = -4.0;}
	climb_rate = -5.0;  // -5
	
	trim_elevator = -0.45 + (H0/200.0) + (tangage/20.0); //  H0/300.0

//RAPPEL: throttle 1.0 -> ralentit;  0 -> mi-gaz  ; -1.0 -> plein gaz

	if (read_bit(flags, bit_att_short) == 1)
	{
		throttle = 1.0; // gaz au mini (Rappel: le mini = +1.0, le max = -1.0)
		speedbrake = 1.0;
	}
	else
	{
		throttle = 0.7; // gaz presque au mini (Rappel: le mini = +1.0, le max = -1.0)
		speedbrake = 0.6;
	}
	

	ailerons= -roulis/10.0; // 1.2 //asservissement des ailerons en fonction de l'angle de roulis (-> ailes à plat)
	
	if(H0 < hauteur_mini_autopilot)
	{
		
		raz_bit(&flags, bit_FG_AP);

		// -------------------- AZIMUT -----------------------------------------------------------------


		float op1;
		float delta_AZM;
		
		find_sens_approche();
		affi_sens_APP(); // en haut à droite

		if (sens_app_effectif == sens_AB)
		{
			op1 = orient_pisteAB;
			delta_AZM =  op1 -GPS_azimut_ptB;
		}
		if (sens_app_effectif == sens_BA)
		{
			op1 = orient_pisteBA;
			delta_AZM =  op1 -GPS_azimut_ptA;
		}
		
		auto_rudder_attero(delta_AZM);

		// -------------------- VITESSE ----------------------------------------------------------------

		//if(is_in(vitesse, 120, 100) == 1)	{ brake_left =0.6;};


		
		if(vitesse < 100)
		{
			//brake_left =0.5;
			throttle = 1.0; // gaz au mini
			if (read_bit(flags, bit_att_short) == 1)
			{
				brake_left =1.0;// freine à fond
				reverser1 = 1;
				reverser2 = 1;
				throttle = -0.95;; // gaz au max pour freiner énergiquement avec les reverses				
			}

			
		} // pose le train avant ce qui permet le guidage au sol en lacet

		if(vitesse < 80)
		{
			speedbrake = 1.0;
			brake_left =0.8;// freine
		}
	
		if ((vitesse < 40) && (read_bit(flags, bit_att_short) == 1) )
		{
			throttle = 1.0; // gaz au mini
			brake_left =1.0; // freine très fortement
		}	

		if(vitesse < 30)
		{
			reverser1 = 0;
			reverser2 = 0;
			throttle = 1.0; // gaz au mini
		}
	}

	brake_right = brake_left;
}



void roulage()
// sur taxiways
// on passera en boucle dans cette fonction
{
	reverser1 = 0;
	reverser2 = 0;
	throttle = 0.80; // 1.0 -> ralentit; 0 -> mi-gaz  ; -1.0 -> plein gaz

	WARNING = "Roulage";
	tempo_message=5;
	//affi_message(WARNING, BLANC, GRIS_TRES_FONCE);
	brake_left =0;
	brake_right =0;
	
	//WARNING="volets 0";
	flaps = 0;
	
	if (vitesse < 15) {throttle -= 0.01;}
	if (vitesse > 15) {throttle += 0.01;}

	borne_in(&throttle, 0.5, 1.0);

	calcul_erreur_position();
}

// =============================================================================================================================









void trace_arc_gradu()
{
//arc gradué en haut au centre, indiquant la valeur de l'inclinaison
	
	float angle;
	//Draw_arc_elliptique(HA_x0, 120, 120, 80, 0.6, 2.6, BLANC);


	for(uint8_t n=0; n<9; n++ )
	{
		angle =30+ n*15;  // 1 tiret tous les 15 degrés
		float pourcent = 0.9;
		if (((n+2)%2) == 0) {pourcent = 0.8;}
		
		affi_rayon1(HA_x0, HA_y0+10, 110, degTOrad(angle), pourcent, BLANC, false); // tirets de graduation
	}
}



void rotation1()
{ 
// consigne de cap
// acquisition de l'encodeur pas à pas (1)
	if ( millis() - TEMPO  >= timer1 )
	{
		timer1 = millis();
		bool etat = digitalRead(rot1b);
		if(etat == 0) { hdg1+=1;} else { hdg1-=1;}
		if (hdg1<0){hdg1=359;}
		
		if (hdg1>359){hdg1=0;}		
	}
}



void rotation2()
{
// consigne d'altitude
// acquisition de l'encodeur pas à pas (2)
	if ( millis() - TEMPO  >= timer2 )
	{
		timer2 = millis();
		bool etat = digitalRead(rot2b);
		if(etat == 0) {	asel1+=1; }	else { asel1-=1; }
		if (asel1<1){asel1=1;} // 100 pieds -> 30m
		if (asel1>600){asel1=600;}
	}
}



void init_SDcard()
{
	String s1;
	
	TFT480.fillRect(0, 0, 480, 320, NOIR); // efface
	TFT480.setTextColor(BLANC, NOIR);
	TFT480.setFreeFont(FF1);

	uint16_t y=0;
	
	TFT480.drawString("PRIMARY FLIGHT DISPLAY", 0, y);
	y+=20;

	s1="version " + version;
	TFT480.drawString(s1, 0, y);
	
	y+=40;
	TFT480.setTextColor(VERT, NOIR);
	TFT480.drawString("Init SDcard", 0, y);
	y+=20;
	
	
 	if(!SD.begin())
	{
		TFT480.drawString("Card Mount Failed", 0, y);
		delay (2000);
		TFT480.fillRect(0, 0, 480, 320, NOIR); // efface
		return;
	}
  

    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE)
    {
        TFT480.drawString("No SDcard", 0, y);
        delay (2000);
		TFT480.fillRect(0, 0, 480, 320, NOIR); // efface
        return;
    }

    flag_SDcardOk=1;

    TFT480.drawString("SDcard Type: ", 0, y);
	if(cardType == CARD_SD)    {TFT480.drawString("SDSC", 150, y);}
    else if(cardType == CARD_SDHC) {TFT480.drawString("SDHC", 150, y);}

	y+=20;
	
	uint32_t cardSize = SD.cardSize() / (1024 * 1024);
	s1=(String)cardSize + " GB";
	TFT480.drawString("SDcard size: ", 0, y);
	TFT480.drawString(s1, 150, y);

   // listDir(SD, "/", 0);
    
    //Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    //Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));

	delay (1000);
	TFT480.fillRect(0, 0, 480, 320, NOIR); // efface
	
}






void init_sprites()
{
	// sprites représentant les lettres 'N' 'S' 'E' 'O' qui seront affichées  sur un cercle, inclinées donc.

	SPR_E.setFreeFont(FF1);
	SPR_E.setTextColor(JAUNE);
	SPR_E.createSprite(SPR_W, SPR_H);
	SPR_E.setPivot(SPR_W/2, SPR_H/2);  // Set pivot relative to top left corner of Sprite
	SPR_E.fillSprite(GRIS_TRES_FONCE);
	SPR_E.drawString("E", 2, 1 );

	SPR_N.setFreeFont(FF1);
	SPR_N.setTextColor(JAUNE);
	SPR_N.createSprite(SPR_W, SPR_H);
	SPR_N.setPivot(SPR_W/2, SPR_H/2);
	SPR_N.fillSprite(GRIS_TRES_FONCE);
	SPR_N.drawString("N", 2, 1 );

	SPR_O.setFreeFont(FF1);
	SPR_O.setTextColor(JAUNE);
	SPR_O.createSprite(SPR_W, SPR_H);
	SPR_O.setPivot(SPR_W/2, SPR_H/2);
	SPR_O.fillSprite(GRIS_TRES_FONCE);
	SPR_O.drawString("W", 2, 1 );

	SPR_S.setFreeFont(FF1);
	SPR_S.setTextColor(JAUNE);
	SPR_S.createSprite(SPR_W, SPR_H);
	SPR_S.setPivot(SPR_W/2, SPR_H/2);
	SPR_S.fillSprite(GRIS_TRES_FONCE);
	SPR_S.drawString("S", 2, 1 );

	SPR_trajectoire.setFreeFont(FF1);
	SPR_trajectoire.setTextColor(JAUNE);
	SPR_trajectoire.createSprite(292, 88);
	efface_sprite_trajectoire();
}



void init_Leds() // pour l'affichage des données, voir la fonction "affi_data_piste()"
{
	uint16_t x0 = 464;
	uint16_t y0 = 0;
	uint16_t xi=x0;
	uint16_t yi=y0;
	

	Led1.init(xi,yi, 10, 10);
	Led1.set_couleur(ROUGE);
	Led1.allume();

	yi+=10;

	Led2.init(xi,yi, 10, 10);
	Led2.set_couleur(JAUNE);
	Led2.allume();

	yi+=10;
	
	Led3.init(xi,yi, 10, 10);
	Led3.set_couleur(VERT);
	Led3.allume();

	yi+=10;

	Led4.init(xi,yi, 10, 10);
	Led4.set_couleur(BLEU);
	Led4.allume();

	yi+=10;

	Led5.init(xi,yi, 10, 10);
	Led5.set_couleur(VIOLET1);
	Led5.allume();

	delay(100);

}



void int16_to_array(int16_t valeur_i)
{
// prépare la chaine de caract à zéro terminal pour l'envoi
// Remarque : 2^16 -1 = 65535  -> 5 caractères)

	String s1= (String) valeur_i;
	uint8_t len1 = s1.length();
	for(int n=0; n<len1; n++)
	{
		var_array16[n]=s1[n];
	}
	var_array16[len1]=0;  // zéro terminal  -> chaine C
}



void int32_to_array(int32_t valeur_i)
{
// prépare la chaine de caract à zéro terminal pour l'envoi
// Remarque : 2^32 -1 =  4294967295 -> 10 caractères

	String s1= (String) valeur_i;
	uint8_t len1 = s1.length();
	for(int n=0; n<len1; n++)
	{
		var_array32[n]=s1[n];
	}
	var_array32[len1]=0;  // zéro terminal  -> chaine C 
}


void string_to_array(String str_i)
{
// prépare la chaine de caract à zéro terminal pour l'envoi

	uint8_t len1 = str_i.length();
	for(int n=0; n<len1; n++)
	{
		var_array32[n]=str_i[n];
	}
	var_array32[len1]=0;  // zéro terminal  -> chaine C 
}


void annule_tout()
{
	WARNING = "  RAZ  ";
	raz_bit(&flags, bit_decollage);	
	raz_bit(&flags, bit_atterrissage);
	
	raz_bit(&flags, bit_autoland);
	raz_bit(&flags, bit_route);
	raz_bit(&flags, bit_nav_to_piste);
	raz_bit(&flags, bit_nav_to_pti);
	raz_bit(&flags, bit_circling);
	raz_bit(&flags, bit_nav_to_ptAA);
	raz_bit(&flags, bit_nav_to_ptBB);
	raz_bit(&flags, bit_rudder_decol);
	raz_bit(&flags, bit_roulage);
	raz_bit(&flags, bit_att_short);
	raz_bit(&flags, bit_rudder_attero);
	
	reverser1 = 0;
	reverser2 = 0;
	speedbrake=0;
	landing_light1=0;
	landing_light2=0;
	
	//gear_down = 0;

	set_bit(&flags, bit_FG_AP);
	locks_type = "ALT";
	speed_ctrl=true;
	trim_elevator=0;
	
	throttle = 0; // mi-gaz
	
}



void setup()
{
    Serial.begin(38400); // 19200

	locks_type ="ALT";
	raz_bit(&flags, bit_FG_AP); // pas d'engagement de l'autopilot de FlightGear à ce stade
	WiFi.persistent(false);
	WiFi.softAP(ssid, password);  // Crée un réseau WiFi en mode privé (indépendant de celui de la box internet...)
	IPAddress IP = WiFi.softAPIP();


	server.on("/switch", HTTP_GET, [](AsyncWebServerRequest *request) // lecture des boutons de l'ESP du module SW
	{
// attention: ce code est appelé par une interruption WiFi qui intervient hors timing. Donc pas d'affichage ici !!		

		argument_recu1 = request->arg("sw1"); // réception de l'argument n°1 de la requête
		switches=argument_recu1; // réception des boutons du module SW
		v_switches=switches.toInt();
		flag_traiter_SW=1; //positionne ce drapeau afin que le traitement se fasse dans le timming général, pas ici !

		int16_to_array(0);

		argument_recu2 = request->arg("pot1"); // réception de l'argument n°2 de la requête
		potar1=argument_recu2; // = "0".."255"
		v_potar1 = -128 + potar1.toInt(); // centre autour de 0
		f_potar1 = v_potar1 / 10.0; // -> f_potar1= [-12.7 ... +12.7 ]
		
		float valeur1 = v_potar1 * v_potar1; // au carré pour avoir une bonne précision aux faibles débattements,
		//et une bonne réponse à fond (taxi au sol)
		
		//if (abs(v_potar1) > 100) {raz_bit(&flags, bit_rudder_attero);} // afin de pouvoir manoeuvrer sur les taxiways}

		
		if (v_potar1<0) {valeur1 =  -valeur1;} // because un carré est toujours positif, or on veut conserver le signe  
		rudder_manuel = valeur1 / 20000.0; // 10000.0 détermine la sensibilité de la gouverne de direction (lacet)


//cet array because la fonction "request->send_P()" n'accèpte pas directement le string
//rappel :
//void send_P(int code, const String& contentType, const uint8_t * content, size_t len, AwsTemplateProcessor callback=nullptr);
//void send_P(int code, const String& contentType, PGM_P content, AwsTemplateProcessor callback=nullptr);

		request->send_P(200, "text/plain", var_array16); // envoie  comme réponse au client
	});


	server.on("/hdg", HTTP_GET, [](AsyncWebServerRequest *request) // consigne de cap
	{
// attention: ce code est appelé par une interruption WiFi qui intervient hors timing. Donc pas d'affichage ici !!		

		argument_recu1 = request->arg("a1"); // reception de l'argument n°1 de la requête
		num_bali=argument_recu1.toInt();

		argument_recu2 = request->arg("swND");
		flag_traiter_SW=1; //positionne ce drapeau afin que le traitement se fasse dans le timming général, pas ici !
		
		v_switches_ND = argument_recu2.toInt();
		switches_ND = String(v_switches_ND);
		

		int16_to_array(hdg1);

//cet array because la fonction "request->send_P()" n'accèpte pas directement le string
//rappel :
//void send_P(int code, const String& contentType, const uint8_t * content, size_t len, AwsTemplateProcessor callback=nullptr);
//void send_P(int code, const String& contentType, PGM_P content, AwsTemplateProcessor callback=nullptr);

		request->send_P(200, "text/plain", var_array16); // envoie hdg1 comme réponse au client
	});

// réponses aux requêtes :
// VOIR la fonction "void interroge_WiFi()" dans le code du ND (l'affichage de la carte...)
// pour la réception des données qui suivent
	server.on("/cap", HTTP_GET, [](AsyncWebServerRequest *request)
	{
		int16_to_array(cap); // prépare la chaine de caract à zéro terminal pour l'envoi
		request->send_P(200, "text/plain", var_array16); // envoie réponse au client
	});


	server.on("/latitude", HTTP_GET, [](AsyncWebServerRequest *request) // latitude de l'avion
	{
		int32_t lati1 = (int32_t) (lat_avion * 10000.0);

		int32_to_array(lati1);
		request->send_P(200, "text/plain", var_array32); // envoie réponse au client
	});


	server.on("/longitude", HTTP_GET, [](AsyncWebServerRequest *request) // longitude de l'avion
	{
		int32_t longi1 = (int32_t) (lon_avion * 10000.0);

		int32_to_array(longi1);
		request->send_P(200, "text/plain", var_array32); // envoie réponse au client
	});


	server.on("/hauteur", HTTP_GET, [](AsyncWebServerRequest *request) // hauteur de l'avion / sol
	{
		int32_t haut1 = (int32_t) (hauteur_AAL * 10.0);

		int32_to_array(haut1);
		request->send_P(200, "text/plain", var_array32); // envoie réponse au client
	});


	server.on("/flags", HTTP_GET, [](AsyncWebServerRequest *request) // paramètres divers PFD -> ND & MCDU
	{
		argument_recu3 = request->arg("btMCDU"); // valeur reçue du module MCDU en tant qu'argument
		
		v_bt_MCDU = argument_recu3.toInt();
		flag_traiter_MCDU=1;

		int32_to_array(flags); // valeur à envoyer
		request->send_P(200, "text/plain", var_array32); // envoie réponse au client
	});

/*
	server.on("/msg", HTTP_GET, [](AsyncWebServerRequest *request) // paramètres divers PFD -> ND & MCDU
	{
		string_to_array(//WARNING); // valeur à envoyer
		request->send_P(200, "text/plain", var_array32); // envoie réponse au client
		//WARNING="null";
	});
*/

	server.on("/num_bali", HTTP_GET, [](AsyncWebServerRequest *request) // PFD -> MCDU
	{
		int32_t num1 = (int32_t) num_bali;
		int32_to_array(num1);
		request->send_P(200, "text/plain", var_array32); // envoie réponse au client
	});


	server.on("/Pos_XY", HTTP_GET, [](AsyncWebServerRequest *request) //pour recevoir les requêtes émises par le module positionneur
	{
		argument_recu1 = request->arg("X1"); // réception de l'argument n°1 de la requête
		x_1=argument_recu1.toInt();

		argument_recu2 = request->arg("Y1"); // réception de l'argument n°2 de la requête
		y_1=argument_recu2.toInt();

		argument_recu2 = request->arg("X2"); // réception de l'argument n°3 de la requête
		x_2=argument_recu2.toInt();

		argument_recu2 = request->arg("Y2"); // réception de l'argument n°4 de la requête
		y_2=argument_recu2.toInt();		
		
		int32_t num1 = 0;
		int32_to_array(num1);
		request->send_P(200, "text/plain", var_array32); // envoie réponse (0) au client
	});	
	
	
    server.begin();


	pinMode(bouton1, INPUT);
	pinMode(bouton2, INPUT);

	//pinMode(led1, OUTPUT);

	pinMode(rot1a, INPUT_PULLUP);
	pinMode(rot1b, INPUT_PULLUP);
	pinMode(rot2a, INPUT_PULLUP);
	pinMode(rot2b, INPUT_PULLUP);
	
	attachInterrupt(rot1a, rotation1, RISING);
	attachInterrupt(rot2a, rotation2, RISING);

	TFT480.init();
	TFT480.setRotation(3); // 0..3 à voir, suivant disposition
	TFT480.fillScreen(TFT_BLACK);

	init_SDcard();
	
	init_sprites();

	delay(100);


	TFT480.setTextColor(NOIR, BLANC);

	
//	TFT480.fillRect(0, 0, 479, 30, NOIR);
	TFT480.setTextColor(BLANC, NOIR);
	TFT480.setFreeFont(FF19);

	Ay_actu=120;
	By_actu=120;

	altitude_GPS =0;
	vitesse =0;
	roulis =0;
	tangage =0;
	cap=0;
	vspeed=0; // vitesse verticale

	//vor_frq=123500;

	//vor_dst=1852*102; // 102km
	//vor_actual_deg=45;
	//vor_actual=45.0 * 100.0;
//	affichages();

	bouton1_etat = digitalRead(bouton1);
	memo_bouton1_etat = bouton1_etat;

	bouton2_etat = digitalRead(bouton2);
	memo_bouton2_etat = bouton2_etat;
	if (bouton2_etat==0)
	{
		mode_affi_hauteur = AAL;
		affi_mode_affi_hauteur();
	}
	if (bouton2_etat==1)
	{
		mode_affi_hauteur = ASL;
		affi_mode_affi_hauteur();
	}

	init_FG_bali();
	
	//init_affi_autopilot();
	//affi_indicateurs();


	voyant_L.init(0,0,30,20);
	voyant_L.caract1 ='L';
	voyant_L.caract2 =' ';
		
	voyant_G.init(35,0,30,20);
	voyant_G.caract1 ='G';
	voyant_G.caract2 =' ';

	voyant_APP.init(70,0,30,20);
	voyant_APP.caract1 ='A';
	voyant_APP.caract2 ='P';

	voyant_route.init(105,0,30,20);
	voyant_route.caract1 ='G';
	voyant_route.caract2 ='T';

	voyant_RD.init(145,0,30,20);
	voyant_RD.caract1 ='R';
	voyant_RD.caract2 ='D';

	voyant_ATT.init(177,0,15,20);
	voyant_ATT.caract1 ='A';

	init_Leds(); // et les affiche en bordure de l'écran en haut à droite

	annule_tout();

	init_affi_HA(); // affiche juste l'horizon artificiel (ciel + terre) sans les graduations ni les échelles
	
	delay(100);
	
	//find_END_RWY_angl();// pour le 1er décollage, on ne repassera plus ici
	//prepare_decollage();
	//TFT480.fillRect(0, 0, 479, 319, BLEU);  // pour test parties libres

	////WARNING = "RESTART"; // pour reseter le MCDU
	// ^ ne marche pas ICI parce que lorsque le PFD est resété les liaisons wifi sont cassées
	// et donc les clients ne reçoivent plus les messages
	// toutefois on notera que lorsque l'on reprogramme le PFD, son serveur WIFI reste en fonction... et les liaisons subsistent

//efface_cadre_bas(NOIR); affi_approche();  while(1);  // pour test
	
	affi_distance_piste(); // cadre en bas
	affi_Airport();  // sous le cadre en bas

	String s1 = "PFD v";
	s1+= version;
	poste_warning(s1);

}



uint8_t p1;

int32_t number = 0;


String s1;
String s2;


void acquisitions()
{
// cette fonction reçoit les données de Flightgear par la liaison USB, voir le fichier "hardware4.xml" pour le protocole

// Remarque : les données des autres modules (ND et SW) sont reçues par WiFi,
// voir les sous-fonctions ("server.on(...)") dans la fonction setup()

	char buf[50];
	int32_t valeur;
	TFT480.setFreeFont(FM9);
	TFT480.setTextColor(VERT, NOIR);
	
	if(Serial.available() > 14) // 14
	{
		//poste_warning("CONNECTED");
		if(flag_att_cnx_usb==1)
		{
			flag_att_cnx_usb=0;
			poste_warning("usb connected");
		}
		
		parametre="";
		s1="";
		char octet_in;
		
		while(octet_in != '=')
		{
		  octet_in = Serial.read();
		  if(octet_in != '=')  {parametre += octet_in; }
		}		
		while(octet_in != '\n')
		{
		  octet_in = Serial.read();
		  if(octet_in != '\n') {s1 += octet_in;  }
		}


		if(parametre == "joystick1" )
		{
			s1.toCharArray(buf, 50);
			valeur = atol(buf);
			joystick1 = (float)valeur / 1000.0;
			data_ok |= 1; // positionne bit0
		}
		

		if(parametre == "alti" )
		{
/*			
 ALTITUDE GPS (et pas 'altimetre', volontairement, voir le fichier hardware4.xml)
 voir la ligne '<node>/instrumentation/gps/indicated-altitude-ft</node>'
 marre des QFE, QNH, AGL, ASFC, AMSL, STD...
 j'ai joué avec tout ça, mais finalement je décide d'utiliser le GPS,
 ce qui est contraire aux recommandations aéronautiques,
 sans doute parce que le jour où le GPS mondial viendrait à tomber en panne, c'est 275643 avions qui iraient se poser
 dans  le Triangle des Bermudes.
 mais ici on est dans FlightGear, on ne risque rien !
 Et puis on a Galiléo et le système EGNOS développé à l'origine par le cnes puis sur la planète B612 à Toulouse (ESSP)...
*/		 
			s1.toCharArray(buf, 50);

			valeur = atol(buf);
			altitude_GPS_float = (float)valeur / 1000.0;
			altitude_GPS = valeur/1000; // integer
			hauteur_AAL = altitude_GPS - liste_bali[num_bali].altitude;
			
			data_ok |= 1<<1; // positionne bit1
		}

		if(parametre == "gnd_elv" ) // hauteur de la surface du terrain situé sous l'avion
		{
			s1.toCharArray(buf, 50);
			gnd_elv = atol(buf);
			if (gnd_elv <0) {gnd_elv =0;}  
			data_ok |= 1<<2; // positionne bit2
		}


		if(parametre == "alti_agl" ) // hauteur de l'avion par rapport au terrain en dessous
		// (= altitude-agl-ft c.a.d "altitude above graound level")
		{
			s1.toCharArray(buf, 50);
			alti_agl = atol(buf);
			if (alti_agl <0) {alti_agl =0;}  
			data_ok |= 1<<3; // positionne bit3
		}

		
		
		if(parametre == "speed" )
		{
			s1.toCharArray(buf, 50);
			vitesse = atol(buf);
			data_ok |= 1<<4; // positionne bit4
		}

		if(parametre == "pitch" )
		{
			//char buf[50];
			s1.toCharArray(buf, 50);
			tangage = atol(buf);
			data_ok |= 1<<5; // positionne bit5
		}

		if(parametre == "roll" )
		{
			s1.toCharArray(buf, 50);
			roulis = atol(buf);
			data_ok |= 1<<6; // positionne bit6
		}



		if(parametre == "heading" ) // /orientation/heading-deg = cap actuel de l'avion ; ne pas confondre avec HDG bug !
		{
			s1.toCharArray(buf, 50);
			valeur = atol(buf);
			cap= (float) valeur / 100.0;
			data_ok |= 1<<7; // positionne bit7
		}
	

		if(parametre == "vspeed" )
		{
			s1.toCharArray(buf, 50);
			vspeed = atol(buf);
			data_ok |= 1<<8; // positionne bit8
		}


		if(parametre == "latitude" )
		{
			s1.toCharArray(buf, 50);
			valeur = atol(buf);
			lat_avion = (float)valeur / 100000.0;
			data_ok |= 1<<9;  // positionne bit9
		}	


		if(parametre == "longitude" )
		{
			s1.toCharArray(buf, 50);
			valeur = atol(buf);
			lon_avion = (float)valeur / 100000.0;
			data_ok |= 1<<10;   // positionne bit10
		}

		nb_acqui=11; // erreur non permise !!! ne pas oublier qu le compte commence à 0, et pas à 1
	}

	delay(3); // 3 important sinon ne recevra pas la totalité des données (qui se fait en plusieurs passes)

	
////// pour test	
////TFT480.drawString("data= ", 90, 50);
////s2= (String) data_ok;
////TFT480.fillRect(140,50, 50, 15, TFT_BLACK);
////TFT480.drawString(s2, 150, 50);
	
}



void data_out()
{
// à destination de FlightGear par la liaison série USB
// voir le fichier "hardware4.xml" pour le protocole et les paramètres

	Serial.print(hdg1);  // consigne de Cap -> autopilot
	Serial.print(',');
	 
	Serial.print(asel1); // consigne d'altitude -> autopilot
	Serial.print(',');


	uint16_t v3 = landing_light1;
	Serial.print(v3); 
	Serial.print(',');

	uint16_t v4 = landing_light2;
	Serial.print(v4); 
	Serial.print(',');

	uint16_t v5 = target_speed;
	Serial.print(v5); // écrit la consigne de vitesse (target_speed)
	Serial.print(',');

	uint16_t v6 = hauteur_mini_autopilot;
	Serial.print(v6); // écrit la hauteur minimum d'engagement du pilote auto de Flightgear (par défaut = 300)
	Serial.print(',');

	float v7 = ailerons; 
	Serial.print(v7); 
	Serial.print(',');

	float v8 = rudder; // position de la gouverne de direction (lacet)
	Serial.print(v8); 
	Serial.print(',');

	uint8_t v9 = view_number;
	Serial.print(v9); 
	Serial.print(',');

	float v10 = climb_rate; // vertical speed en fps (feet per second)
// Note : correspond au paramètre <node>/autopilot/settings/target-climb-rate-fps</node>
	Serial.print(v10); 
	Serial.print(',');

	Serial.print(locks_type); // "ALT" ou "VS"
	Serial.print(',');
/* Note : correspond au paramètre suivant dans le fichier 'harware4.xml' :
	<chunk>
		<name>locks</name>
		<node>/autopilot/locks/altitude</node>
		<type>string</type>
	</chunk>
*/

	if (read_bit(flags, bit_FG_AP) == 1) {AP_status = "AP";} else {AP_status = "";}
	Serial.print(AP_status); // "" ou "AP"
	Serial.print(',');

	Serial.print(speed_ctrl); // boolean : false ou true
	Serial.print(',');

// j'ai commenté la partie "Elevator" dans le fichier joystick_0.xml
// (usr/share/games/flightgear/Input/Joysticks/Local/joystick_0.xml)
// donc les actions de la fonction tangage sur la gouverne de profondeur passent dorénavant par ici
// ce qui implique que l'avion n'est pas pilotable si l'ESP32 du PFD n'est pas en fonction
// l'avantage est qu'il devient possible d'agir sur le tangage lors des phases de décollage et d'attérrissage auto. 


/*
	if ((read_bit(flags, bit_FG_AP) == 1) || (read_bit(flags, bit_rudder_decol) == 1))
	{
		elevator = trim_elevator; // uniquement auto
	}
	else
	{
		elevator = -1.0 * (joystick1/2.0); // on garde la main
	}
*/
	elevator = trim_elevator; // uniquement auto

	
	Serial.print(elevator);
	Serial.print(',');

	Serial.print(throttle);
	Serial.print(',');

	Serial.print(reverser1);
	Serial.print(',');

	Serial.print(reverser2);
	Serial.print(',');

	Serial.print(flaps);
	Serial.print(',');

	Serial.print(speedbrake);
	Serial.print(',');

	Serial.print(gear_down);  // boolean : false ou true
	Serial.print(',');

	Serial.print(brake_left);
	Serial.print(',');

	Serial.print(brake_right);
	Serial.print('\n');
}



void affi_nop()
{
	for(int8_t dy=-2; dy<3; dy++)
	{
		TFT480.drawLine(HA_x0-HA_w, HA_y0-HA_h +dy, HA_x0 +HA_w, HA_y0 +HA_h +dy, ROUGE);
		TFT480.drawLine(HA_x0-HA_w, HA_y0+HA_h +dy, HA_x0 +HA_w, HA_y0 -HA_h +dy, ROUGE);
	}

	//TFT480.fillRect(0, 0, 239, 30, NOIR);
	TFT480.setTextColor(BLANC, ROUGE);
	TFT480.setFreeFont(FF18);
	TFT480.drawString("No Data", HA_x0-40, HA_y0+30);

}




void init_affi_autopilot()
{

	TFT480.setFreeFont(FF1);
	TFT480.setTextColor(JAUNE, GRIS_AF);

// ALT
	
	//TFT480.drawString("ALT", x_autopilot, 260, 1);
	//TFT480.drawRoundRect(x_autopilot-4, 255, 45, 42, 5, BLANC);
}



void affi_autopilot(uint8_t complet)
{
// dans le petit cercle en bas à gauche	:
// affiche HDG (flèche jaune), piste (rectangle bleu), VOR (Nav1, ligne verte)
	
	uint16_t x0=70; // 70
	uint16_t y0=248; // 255
	
	TFT480.setFreeFont(FF1);

	TFT480.fillRect(x0, y0+2, 70, 80, NOIR); // efface
	
	TFT480.drawCircle(x0+35, y0+34, 30, BLANC);


	TFT480.setTextColor(BLANC, NOIR);
	TFT480.drawString("N", x0+30, y0-5);

	TFT480.setTextColor(BLANC, NOIR);
	TFT480.drawString("S", x0+30, y0+60);

	TFT480.setTextColor(BLANC, NOIR);
	TFT480.drawString("E", x0+60, y0+27);
	TFT480.drawString("W", x0, y0+27);

	//uint16_t x1,y1;

	
// rectangle bleu, très fin -> orientation ('radial') de l'axe de la piste
	float angle2 = orient_pisteAB;
	affi_rectangle_incline(x0+35, y0+34, 35, 90 + angle2, BLEU_CLAIR);

	if (complet ==1)
	{
		TFT480.setTextColor(JAUNE, NOIR);
		TFT480.drawString("HDG", x0, y0-18);
			
		String s1 =(String)hdg1;
		TFT480.setTextColor(BLANC, NOIR);
		TFT480.drawString(s1, x0+18, y0+35);

		// flèche jaune = règlage HDG de l'autopilot
		float angle1 = 90-hdg1;
		affi_rayon2(x0+35, y0+34, 0, 30, angle1, JAUNE, 0); // tige de la flèche
		affi_pointe(x0+35, y0+34, 30, angle1, 0.1, JAUNE); // pointe triangulaire en bout de flèche


	}
	else
	{
		
		if(sens_app_effectif == sens_AB)
		{
			affi_pointe(x0+35, y0+34, 20, 90.0 - angle2, 0.5, JAUNE);
			TFT480.drawString("A->B", 240, 250);
		}
		if(sens_app_effectif == sens_BA)
		{
			affi_pointe(x0+35, y0+34, 20, 270.0 - angle2, 0.5, JAUNE);
			TFT480.drawString("B->A", 240, 250);
		}
	}
	
}






void affi_approche() // grand rectangle en bas avec le tracé de la pente de descente (glide)
{
	
	uint16_t x1=90;
	uint16_t x2=140;
	uint16_t x3=310;
	uint16_t x4=350;
	
	uint16_t y1=240;
	uint16_t y2=310;
	
	uint16_t x_avion;
	uint16_t y_avion;

	float xF;
	float yF;
	float pente;

		
	xF = (float)x3 - GPS_distance_piste * (float)(x3-x2) / 10.0;
	yF = (float)y2 - (altitude_GPS - liste_bali[num_bali].altitude)  * (float)(y2-y1) / 3000.0;
	x_avion = (uint16_t) xF;

	pente = (float)(y2-y1)/(float)(x3-x2);
		
	TFT480.drawLine(x1, y1, x2, y1, BLANC); // trace ligne blanche (glide : trajectoire théorique)
	TFT480.drawLine(x2, y1, x3, y2, BLANC); // trace ligne blanche (glide : trajectoire théorique)
	TFT480.drawLine(x3, y2, x4, y2, BLANC); // trace ligne blanche (glide : trajectoire théorique)
	TFT480.setFreeFont(FM9);
	TFT480.setTextColor(BLANC, NOIR);
	TFT480.drawString("glide", x2+10, y2-30);
	affi_autopilot(0);

//dessine l'avion sur la ligne, avec une trainée jaune comme trace

	
	if (memo_y_avion >235) // pour ne pas effacer hors cadre
	{
		TFT480.fillCircle(memo_x_avion, memo_y_avion, 5, GRIS_TRES_FONCE);  // efface
		SPR_trajectoire.pushSprite(70, 232, TFT_BLACK); // TFT_BLACK -> défini la couleur de transparence
		
		//TFT480.drawCircle(memo_x_avion, memo_y_avion, 10, JAUNE);  // efface
		
		//TFT480.drawPixel( memo_x_avion, memo_y_avion, JAUNE); // trace la trajectoire réelle en jaune
	}
	
	if((x_avion > x1)&&(x_avion<=x2))
	{
		//y_avion = y1;
		TFT480.drawLine(x1, y1, x2, y1, BLANC); // trace ligne blanche (trajectoire théorique)
		//TFT480.fillRect(x_avion+1, y_avion, 8, 4, BLEU); // dessine avion
		if (y_avion >235)
		{
			TFT480.fillCircle(x_avion, y_avion, 5, BLEU);
			SPR_trajectoire.drawPixel( x_avion-70, y_avion-232, JAUNE);
			
		}
	}
	
	if((x_avion > x2)&&(x_avion<=x3))
	{
		y_avion = (uint16_t) yF;
		TFT480.drawLine(x2, y1, x3, y2, BLANC);
		if (y_avion >235)
		{
			TFT480.fillCircle(x_avion, y_avion, 5, BLEU);
			SPR_trajectoire.drawPixel( x_avion-70, y_avion-232, JAUNE);
			
		}
	}

	if((x_avion > x3)&&(x_avion<=x4))
	{
		y_avion = y2;
		TFT480.drawLine(x3, y2, x4, y2, BLANC);
		if (y_avion >235)
		{
			TFT480.fillCircle(x_avion, y_avion, 5, BLEU);
			SPR_trajectoire.drawPixel( x_avion-70, y_avion-232, JAUNE);
		}
	}

	memo_x_avion = x_avion;
	memo_y_avion = y_avion;

	//SPR_trajectoire.pushSprite(70, 232, TFT_BLACK); // TFT_BLACK -> défini la couleur de transparence

}



void affichages()
{
	if (roulis < -45)	{roulis = -45;}
	if (roulis > 45)	{roulis = 45;}

	if (tangage < -30)	{tangage = -30;}
	if (tangage > 30)	{tangage = 30;}
	
	affi_HA();
	dessine_avion();
	
	affi_elevator();
	affi_vitesse();
	affi_hauteur_RWY();
	affi_vt_verticale();
	affi_acceleration();
	if(locks_type != "VS") {affi_asel(asel1*100);} // attention -> résultat sur 32 bits !!!
	affi_target_speed();
	affi_flags();
	affi_rudder();

	if (read_bit(flags, bit_autoland)== 1) 
	{
		affi_approche();
	}
	else
	{
		trace_arc_gradu();
		affi_cap();
		affi_autopilot(1); // dans le petit cercle en bas à gauche
		//affi_indicateurs();
		affi_switches();
		
		//affi_radial_VOR();
		//affi_Airport();
	}
	
// //affi_message(String s, uint16_t txt_couleur, uint16_t back_couleur)

	
	//bargraph_float_test(GPS_azimut_piste/360.0);
	//bargraph_float_test(GPS_distance_piste);
}



void traitement_boutons() 
{

// ======================================================================================
//		BOUTONS DU module SW (le boitier qui ne comprend que des boutons, sans écran)
//---------------------------------------------------------------------------------------

// les valeurs sont tranmises par WiFi
// voir leur réception ici dans la fonction "setup()"

// cette fonction traite également les boutons des autres modules (ND, MCDU), voir plus bas
	
	flag_traiter_SW =0;
	
	if(v_switches != memo_v_switches)
	{
		memo_v_switches = v_switches;
		if ((v_switches & 1)==1)
		{
			if (read_bit(flags, bit_autoland) == 0)
			{
				if (hdg1>30) {hdg1 -=30;} else {hdg1+=330;} // la barre à tribord 30° !
				RAZ_chrono();
			}
		}
		
		if ((v_switches & 2)==2)
		{
			if (read_bit(flags, bit_autoland) == 0)
			{
				if (hdg1<330) {hdg1 +=30;} else {hdg1-=330 ;}  // la barre à babord 30° !
				RAZ_chrono();			
			}
		}

		

// altitude
		if ((v_switches & 4)==4)
		{
			if (is_in(asel1, 46, 190)) {asel1 +=10;} // consigne d'altitude +1000ft
			else if (is_in(asel1, 20, 45)) {asel1 +=5;} 
			else if (is_in(asel1, 0, 19)){asel1 += 1;}
			if (asel1>200) {asel1=200;}
		}

		if ((v_switches & 8)==8)
		{
			if (is_in(asel1, 46, 190)){asel1 -= 10;}  // consigne d'altitude -1000ft
			else if (is_in(asel1, 21, 45)){asel1 -= 5;} 
			else if (is_in(asel1, 1, 20)) {asel1 -= 1;}
			if (asel1<0) {asel1=0;}
		}

		
// vitesse
		if ((v_switches & 16)==16)
		{
			if (target_speed<180) {target_speed +=5;} // consigne de vitesse 
			else if ((target_speed>=180) && (target_speed<600)) {target_speed +=20;}
		}
		
		if ((v_switches & 32)==32)
		{
			if ((target_speed>180) && (target_speed<600)) {target_speed -=20;}
			else if ((target_speed<=180) && (target_speed>90)) {target_speed -=5;}
		}


		if ((v_switches & 64)==64) // bouton "RAZ"
		{
			annule_tout();
			poste_warning("   RAZ   ");

		}

		
		if ((v_switches & 128)==128) // bouton HDG SYNC	
		{
			hdg1 = cap;
			RAZ_chrono();
			poste_warning("   SYNC   ");
		}
	}


/**	
// ===============================================================
//         INVERSEUR à levier du module ND
//----------------------------------------------------------------

	if(v_switches_ND != memo_v_switches_ND)
	{
		memo_v_switches_ND = v_switches_ND;
		if (read_bit(v_switches_ND ,1)==1)
		{
			TFT480.fillCircle(465, 310, 5, BLEU_CLAIR);
			landing_light1=1;
			landing_light2=1;
			
		}
		else
		{
			TFT480.fillCircle(465, 310, 5, NOIR);
			landing_light1=0;
			landing_light2=0;
		}
	}
**/

// ===============================================================
//			BOUTONS du module MCDU (le module avec écran 3.5")
//----------------------------------------------------------------

	if(v_bt_MCDU != memo_v_bt_MCDU)
	{
		memo_v_bt_MCDU = v_bt_MCDU;
		affi_etats_bt_MCDU();


// >>>>>>>>>>>>>>>>>  ROUGE <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

		if(read_bit(v_bt_MCDU, bit_bt1) == 1)  
		{
			Led1.allume(); 
			
			if (hauteur_AAL < 50)
			{
				if (GPS_distance_piste <2)
				{
					//inv_bit(&flags, bit_rudder_decol);// toggle manuel de ce drapeau; permet d'effectuer un décollage
	// en mode manuel ou auto (concernant la gouverne de direction)
	
					////if(read_bit(flags, bit_rudder_decol) ==1)
					////{
						////find_END_RWY_angl();
					////}

					inv_bit(&flags, bit_decollage);
					affiche_etats_flags();
					
					if(read_bit(flags, bit_decollage) ==1)
					{
						prepare_decollage();
						poste_warning("decollage");
					}
					if(read_bit(flags, bit_decollage) ==0)
					{
						annule_tout();
						poste_warning("  RAZ  ");
					}
					
				}
				else
				{
					poste_warning("BAD RWY");
				}
			}  
		}
		else {Led1.eteint();}


// >>>>>>>>>>>>>>>>>  JAUNE   <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

		if(read_bit(v_bt_MCDU, bit_bt2) == 1)  
		{
			Led2.allume();
			
			inv_bit(&flags, bit_FG_AP);  // -> toggle Autopilot de FlightGear
			if (hauteur_AAL >300) // feet
			{
				if (read_bit(flags, bit_FG_AP)== 1)
				{
					poste_warning("engage AP");
					speed_ctrl = true;
				}
			}	

			if (read_bit(flags, bit_FG_AP)== 0)
			{
				poste_warning("  RAZ  ");
				annule_tout();
			}	
			
		}
		else {Led2.eteint();}

		
// >>>>>>>>>>>>>>>>>   VERT  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

		if(read_bit(v_bt_MCDU, bit_bt3) == 1)  // -> en ROUTE
		{
			Led3.allume(); 
			
			inv_bit(&flags, bit_route); // change d'état...
			// et teste le résultat
			if (read_bit(flags, bit_route)==1)// && (GPS_distance_piste > 1.0) )
			{
				// on commence à descendre le cas échéant
				//if(GPS_distance_ptAA < 20.0) { if (asel1 >60) {asel1 =60;} }
				//if(GPS_distance_ptAA < 15.0) { if (asel1 >30) {asel1 =30;} }
				poste_warning("en ROUTE");
				
				desengage_autoland();
				raz_bit(&flags, bit_roulage);
			}
			
/*			
			if (GPS_distance_piste < 1.0)
			{
				raz_bit(&flags, bit_route); 
				//WARNING ="d<1NM !"; // à destination des autres modules (par WiFi, voir server.on("/msg"... dans le setup)
				TFT480.setFreeFont(FF6);
				//affi_message(WARNING, BLANC, GRIS_TRES_FONCE); // ici
			
			}
*/			
		}
		else {Led3.eteint();}

		
// >>>>>>>>>>>>>>>>>  BLEU  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

		if(read_bit(v_bt_MCDU, bit_bt4) == 1)  // fait tourner la selection RWY -> ptA -> ptB ...
		{

			Led4.allume();
			
			raz_bit(&flags, bit_route); // désenclenche le suivi de route pour éviter de zigzaguer pendant le choix
// il faudra le réarmer manuellement avec le bouton précédent ('vert')
			poste_warning("en ROUTE");
			
			options_route ++; // 0 = RWW ; 1=ptA ; 2=ptB ; 3=crc1 ; 4=crc2
			
			if ((options_route == 1) && (liste_bali[num_bali].sens_app_interdit == 'A')) {options_route++;}
			else if ((options_route == 2) && (liste_bali[num_bali].sens_app_interdit == 'B')) {options_route++;}
			
			if (options_route>4) {options_route=0;}
				
			if(options_route == 0) //(RWW)
			{
				////WARNING += String(liste_bali[num_bali].ID_OACI);

				if (GPS_distance_ptAA > 100) {asel1 = 100;}  // ou 370 en croisière (37 000 ft soit 11 000m) à voir...
				if (is_in(GPS_distance_ptAA, 15, 100)) {asel1 = 60;} // 6000 ft, local
				
				set_bit(&flags, bit_nav_to_piste);
				////WARNING="to RWY";
				////WARNING += "- ";
				
				raz_bit(&flags, bit_nav_to_ptAA);
				raz_bit(&flags, bit_nav_to_ptBB);
				raz_bit(&flags, bit_nav_to_pti);
			}

			if(options_route == 1) // ptA
			{
				if (GPS_distance_ptAA > 100) {asel1 = 100;} // ou 370 en croisière (37 000 ft soit 11 000m) à voir...
				if (is_in(GPS_distance_ptAA, 15, 100)) {asel1 = 60;} // 6000 ft, local

				set_bit(&flags, bit_nav_to_ptAA);
				
				////WARNING="to AA";
				////WARNING += " - ";

				raz_bit(&flags, bit_nav_to_piste);
				raz_bit(&flags, bit_nav_to_ptBB);
				raz_bit(&flags, bit_nav_to_pti);
				
			}

			if(options_route == 2) //ptB
			{
				if (GPS_distance_ptBB > 100) {asel1 = 100;}  // ou 370 en croisière ()37 000 ft soit 11 000m) à voir...
				if (is_in(GPS_distance_ptBB, 15, 100)) {asel1 = 60;} // 6000 ft, local

				set_bit(&flags, bit_nav_to_ptBB);
				
				////WARNING="to BB";
				////WARNING += " - ";
				
				raz_bit(&flags, bit_nav_to_piste);
				raz_bit(&flags, bit_nav_to_ptAA);
				raz_bit(&flags, bit_nav_to_pti);
								
			}
 
			if(options_route == 3) // crc1 ( hypodromes dans un sens)
			{
				set_bit(&flags, bit_circling);
				raz_bit(&flags, bit_sens_circling);
				set_bit(&flags, bit_nav_to_pti);
				
				////WARNING="crc1";
				////WARNING += " - ";
				
				raz_bit(&flags, bit_nav_to_piste);
				raz_bit(&flags, bit_nav_to_ptAA);
				raz_bit(&flags, bit_nav_to_ptBB);
				
			}

			if(options_route == 4) // crc2 ( hypodromes dans l'autre sens)
			{
				set_bit(&flags, bit_circling);
				set_bit(&flags, bit_sens_circling);
				set_bit(&flags, bit_nav_to_pti);
				
				////WARNING="crc2";
				////WARNING += " - ";
				
				raz_bit(&flags, bit_nav_to_piste);
				raz_bit(&flags, bit_nav_to_ptAA);
				raz_bit(&flags, bit_nav_to_ptBB);
			}
			

			////WARNING += String(liste_bali[num_bali].ID_OACI);
			
	
		}
		else {Led4.eteint();}


// >>>>>>>>>>>>>>>>>   VIOLET  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

		if(read_bit(v_bt_MCDU, bit_bt5) == 1) 
		{
			if(hauteur_AAL<20)
			{
				inv_bit(&flags, bit_roulage);
				if (read_bit(flags, bit_roulage) == 0)
				{
					throttle = 1.0; // gaz à zéro.
					poste_warning("stop");
					
					brake_left=1;
					brake_right=1;
				} 
				else
				{
					annule_tout();
					set_bit(&flags, bit_roulage);
					poste_warning("roulage");
					
					brake_left=0;
					brake_right=0;
				} 
			}
			else
			{
				raz_bit(&flags, bit_roulage); // pas question d'engager le roulage auto lorsque l'avion est en vol !!!
				inv_bit(&flags, bit_att_short); // en vol permet de choisir le type d'atterrisage, long ou court.
			} 

			Led5.allume();
		}
		else {Led5.eteint();}
	}	
}





void toutes_les_5s()
{
	
}



void toutes_les_1s()
{
	nb_secondes++; // ne concerne pas le chrono de la ligne suivante
	
	affiche_chrono();
	inc_chrono();

	//affi_int_test(alti_agl, 100, 2, VERT, NOIR);
	// if((alti_agl < 100) && ( read_bit(flags, bit_decollage) ==0) ) {affi_hauteur_SOL(alti_agl);} 


	if(nb_secondes>5)
	{
		nb_secondes=0;
		toutes_les_5s();
	}

	dV =10*(vitesse - memo_vitesse);
	memo_vitesse = vitesse;

	if (vitesse > 150) // désengage auto_rudder en fin de décollage
	{
		raz_bit(&flags, bit_rudder_decol);// en vol la gouverne de lacet se commande au pédalier (dans ce programme, avec un pot
		//raz_bit(&flags, bit_rudder_attero);
	} 

	if (read_bit(flags, bit_au_sol) == 1)
	{
 // désengage l'autoland ILS après attéro pour éviter de re-décoller avec cet autoland engagé

		if (read_bit(flags, bit_autoland) == 1) {desengage_autoland();}
		//asel1 =30; // penser à re-règler cette valeur manuellement avant de ré-engager l'autopilot !
	}

	affiche_etats_flags();
	affi_alti_agl();

	calculs_GPS();
	//affi_dst_GPS();

	if (read_bit(flags, bit_route) == 1)
	{
		if (read_bit(flags, bit_nav_to_piste) == 1) { nav_to_centre_piste();}
		if (read_bit(flags, bit_nav_to_ptAA) == 1)  { nav_to_ptAA(); }
		if (read_bit(flags, bit_nav_to_ptBB) == 1)  { nav_to_ptBB(); }
		if (read_bit(flags, bit_circling) == 1)  {tour_de_piste();} // recalcul de la position du point pti
		if (read_bit(flags, bit_nav_to_pti) == 1)  { nav_to_pti(); }
	}
	
/*
	if ((memo_GPS_distance_piste - GPS_distance_piste) > 0 )
	{
		raz_bit(&flags, bit_eloignement);
		//TFT480.fillCircle(300, 5, 5, VERT);
		
	}
	else
	{
		set_bit(&flags, bit_eloignement);
		
		//TFT480.fillCircle(300, 5, 5, ROUGE);
	}
*/
	

	memo_GPS_distance_piste = GPS_distance_piste;

	if (GPS_distance_piste < 2)
	{
		raz_bit(&flags, bit_nav_to_piste); // désengage la navigation auto vers le centre de la piste
	}


	if(
	(read_bit(flags, bit_autoland) == 0 )
	&& (read_bit(flags, bit_circling) == 0 )
	&& (read_bit(flags, bit_decollage) == 0 )
	&& (read_bit(flags, bit_nav_to_ptAA) == 0 )
	&& (read_bit(flags, bit_nav_to_ptBB) == 0 )
	&& (read_bit(flags, bit_nav_to_pti) == 0 )
	&& (read_bit(flags, bit_route) == 0 )
	&& (alti_agl < 100 )
	)
	{
		//WARNING = "gear down";
		gear_down =1;
	}

/*
	if(alti_agl >= 100 )
	{
		if (gear_down == 1)	{poste_warning("gear UP");}
		gear_down =0;
	}
*/

	auto_landing(); // ne sera effectif que si le flag autoland !=0
	//toutefois la fonction est toujours appelée afin d'actualiser les affichages


	//affi_distance_ptBB(); // pour test

	//TFT480.fillRect(0, 0, 480, 320, BLEU); // pour test

	if (tempo_message==5)
	{
		efface_message(HA_SOL);
		affi_message(WARNING, BLANC, GRIS_TRES_FONCE);
	}
	
	if (tempo_message>0)
	{
		tempo_message--;
		if (tempo_message==0) {efface_message(HA_SOL);}
	}

 }






/** ================================================================== 
 variable à gérer obligatoirement */

//le nombre de ports GPIO libres étant atteint, on va utiiser un switch unique pour deux fonctions :
uint8_t fonction_bt1 = 1; // 0=saisie écran ; 1=changement de vue




/**====================================================================================================== 
										LOOP
====================================================================================================== */

uint16_t t=0; // temps -> rebouclera si dépassement    
void loop()
{
	//if (SerialBT.available()) {	Serial.write(SerialBT.read()); }

//le bouton connecté à CET ESP32-ci est partagé entre deux fonctions, voir ci-dessus "variables à gérer obligatoirement"
	bouton1_etat = digitalRead(bouton1);

	if (bouton1_etat != memo_bouton1_etat) // vue
	{
		memo_bouton1_etat = bouton1_etat;
		if (bouton1_etat==0)
		{
			TFT480.fillCircle(450, 310, 5, VERT);
			

			if (fonction_bt1 == 0) {write_TFT_on_SDcard(); }
			if (fonction_bt1 == 1)
			{
				if (view_number == 0) {view_number = 1;} else {view_number =0;}
			}
		}
		if (bouton1_etat==1)
		{
			TFT480.fillCircle(450, 310, 5, NOIR);
			if (fonction_bt1==1)
			{
				;
			}
		}
	}

	bouton2_etat = digitalRead(bouton2);  // mode affichage de la hauteur / altitude
	
	if (bouton2_etat != memo_bouton2_etat)
	{
		memo_bouton2_etat = bouton2_etat;
		if (bouton2_etat==0)
		{
			mode_affi_hauteur = AAL;
			affi_mode_affi_hauteur();
		}
		if (bouton2_etat==1)
		{
			mode_affi_hauteur = ASL;
			affi_mode_affi_hauteur();
		}
	}

	if ((flag_traiter_SW ==1) || (flag_traiter_MCDU ==1))
	{
// le positionnement de ce drapeau se fait dans l'interruption WiFi "server.on("/switch"..."
// définie dans le setup()
// la commande, en amont, provient de l'ESP n°3 qui gère les switches		
		flag_traiter_SW =0;
		flag_traiter_MCDU =0;
		traitement_boutons();
	}

		
	temps_ecoule = micros() - memo_micros;
	if (temps_ecoule  >= 1E6)  // (>=) et pas strictement égal (==) parce qu'alors on rate l'instant en question
	{
		memo_micros = micros();
		toutes_les_1s();
	}

	if (dV > acceleration) {acceleration++;}
	if (dV < acceleration) {acceleration--;}

//---------------------------------------------------------------------------------------

/*
	compteur1+=1;
	if (compteur1 >= 100)  // tous les 100 passages dans la boucle 
	{
		compteur1=0;
	}		
*/

	acquisitions();

// affi_float_test(joystick1, 100, 2, VERT, NOIR);  // ok


	if (num_bali != memo_num_bali)
	{
		memo_num_bali = num_bali;
		calculs_piste();
		calcul_ptAA_ptBB(liste_bali[num_bali].dst_pt_AB);
		affi_Airport();
	}


	if (vitesse < 30) {set_bit(&flags, bit_au_sol);}
	if (alti_agl > 100) {raz_bit(&flags, bit_au_sol);}


	if(vitesse < 20)
	{
		raz_bit(&flags, bit_rudder_attero); // ce qui permet de reprendre le guidage manuel au sol en fin d'attéro
	}


	if (read_bit(flags, bit_atterrissage)==1)	{atterrissage();}
	
	if (read_bit(flags, bit_roulage)==1)	{roulage();}

/*
	if ((read_bit(flags, bit_rudder_decol) ==1 ))
	{
		calculs_GPS();
		auto_rudder_deco();
	}
*/

		
/** ----------------------------------------------
 pour tester si data_ok à bien la valeur attendue, c.a.d si toutes les acquisitions sont effectuées
 ce qui est le cas lorsque le dialogue avec le serveur de protocole FG est correctement programmé
 sachant que ce ne sont pas les pièges qui manquent !
 Les 6 lignes de code qui suivent peuvent faire gagner une journée...
 Il suffit d'analyser la valeur affichée en binaire pour voir les bits qui restent à 0 -> problèmes (à priori de libellés)
 Il doit y avoir une correspondance stricte entre les noms de variables ici et celles émises par le fichier hardware4.xml
*/

	////TFT480.setFreeFont(FM9);
	////TFT480.setTextColor(VERT, NOIR);
	////s2= (String) data_ok;
	////TFT480.fillRect(150, 220, 80, 12, TFT_BLACK);
	////TFT480.drawString(s2, 150, 220);

/** ---------------------------------------------- **/

				
	if (data_ok == ((1<<nb_acqui)-1)  ) //  si toutes les acquisitions faites
	{
		TFT480.fillCircle(435, 310, 5, VERT);
		if (attente_data == 1)
		{
			attente_data=0;
			//TFT480.fillScreen(TFT_BLACK);
			//init_affi_HA();
			//init_affi_autopilot();
		}

		affichages();
		calculs_GPS();
		data_ok=0;

		data_out();  // ** à commenter le cas échéant pour libérer le port série lors de la mise au point **
	}
	else
	{
		
		TFT480.fillCircle(435, 310, 5, ROUGE);
		if(attente_data==1)
		{
			affi_nop();
			RAZ_variables();
			affichages();
			delay(100);
		}
		
	}
	
	//if ((vitesse > 120) && (altitude_GPS > 200)) {raz_bit(&flags, bit_au_sol);}
	
	if ((vitesse < 50) && (hauteur_AAL < 20))
	{
		raz_bit(&flags, bit_FG_AP); // désengage (ici) l'autopilot de FG
		affi_extremite();
	}

	////v_test1 += 0.01;
	////if (v_test1 > 1.0) {v_test1 = -1.0;}
	////bargraph_float_test(v_test1);

	if (read_bit(flags, bit_decollage)==1)
	{
		decollage();
	}



// Les 4 lignes suivantes affichent un rectangle jaune déplaçable et redimensionnable depuis le module "PositionneurXY"
// lors de la mise au point, pour faciliter le positionnement des éléments graphiques
// les commenter par la suite 

/*
	affi_ligne1_V(x_1);
	affi_ligne2_V(x_2);
	affi_ligne1_H(y_1);
	affi_ligne2_H(y_2);
*/
	//affi_f_potar1();

//---------------------------------------------------------------------------------------

}


/** ***************************************************************************************
	CLASS VOYANT  // affiche un nombre ou un petit texte dans un rectangle
	ainsi que (en plus petit) deux valeurs supplémentaires, par ex: les valeurs mini et maxi					
********************************************************************************************/

// Constructeur
VOYANT::VOYANT()
{

}



void VOYANT::init(uint16_t xi, uint16_t yi, uint16_t dxi, uint16_t dyi)
{
	x0 = xi;
	y0 = yi;
	dx = dxi;
	dy = dyi;

	//TFT480.setTextColor(BLANC, NOIR);
}



void VOYANT::affiche(uint16_t couleur_texte_i, uint16_t couleur_fond_i)
{
	TFT480.fillRoundRect(x0, y0, dx, dy, 3, couleur_fond_i);

	
	if (caract2 !=' ')
	{
		TFT480.setTextColor(couleur_texte_i);
		TFT480.setFreeFont(FM9);
		TFT480.drawChar(x0+3, y0+14, caract1, couleur_texte_i, couleur_fond_i, 1);
			
		TFT480.setTextColor(couleur_texte_i);
		TFT480.setFreeFont(FM9);
		TFT480.drawChar(x0+16, y0+14, caract2, couleur_texte_i, couleur_fond_i, 1);
	}
	else if (caract1 !=' ')
	{
		TFT480.setTextColor(couleur_texte_i);
		TFT480.setFreeFont(FM9);
		TFT480.drawChar(x0+8, y0+14, caract1, couleur_texte_i, couleur_fond_i, 1);
	}
	
}



/** ***************************************************************************************
	CLASS Led  // affiche un petit rectangle coloré
********************************************************************************************/


// Constructeur
Led::Led()
{

}


void Led::init(uint16_t xi, uint16_t yi, uint16_t dxi, uint16_t dyi)
{
	x0 = xi;
	y0 = yi;
	dx = dxi;
	dy = dyi;
}



void Led::allume()
{
	TFT480.fillRect(x0, y0, dx, dy, couleur);
}


void Led::eteint()
{
	TFT480.fillRect(x0, y0, dx, dy, GRIS_TRES_FONCE);
	//TFT480.drawRect(x0, y0, dx, dy, couleur); // bordure
}


void Led::set_couleur(uint16_t coul_i)
{
	couleur = coul_i;
}




/**
-----------------------------------------------------------------------------------------------------------

  ci-dessous contenu du fichier (pour Linux): "/usr/share/games/protocol/hardware4.xml"  qui va bien pour CE programme
  (et CETTE version)

  IMPORTANT : Lorsque je fais évoluer le programme principal (ci-dessus) je modifie également (le cas échéant)
  le fichier hardware4.xml (ci-dessous)
  je ne numérote pas le fichier ci-dessous.
  (en le recopiant au bon endroit, ici ce n'est qu'un commentaire non fonctionnel en tant que tel).
  en effet les variables transmises par FG doivent correspondre EXACTEMENT à ce qu'attend l'ESP32,
  voir la fonction "void acquisitions()"


  FG doit être lancé avec les paramètres suivants :

	--generic=serial,in,2,/dev/ttyUSB0,9600,hardware4
	--generic=serial,out,2,/dev/ttyUSB0,9600,hardware4

IMPORTANT :
	-Il faut ajouter ces DEUX lignes dans un lanceur (tel que "FGo!") pour obtenir un fonctionnement bidirectionnel
	-"ttyUSB0" peut être autre chose suivant la configuration de votre Linux et de l'ordinateur.

Remarques :
  - le nom du fichier "hardware4.xml" peut être autre chose pourvu que le paramètre de lancement corresponde exactement au nom.
  - le fichier en question n'existe pas par défaut dans le répertoire "/usr/share/games/protocol/", il faut le créer.


Astuce : pour gagner du temps au décollage, on peut lancer FG avec les options en ligne de commande suivantes (en plus de celles
vues plus haut):

	--prop:/autopilot/locks/heading=HDG
	--prop:/autopilot/locks/altitude=ALT
	--prop:/autopilot/locks/speed-ctrl=true
	--prop:/autopilot/settings/target-speed-kt=160

ce qui configure l'autopitot correctement en adéquation avec notre PFD
les deux premières lignes en particulier permettent de piloter l'avion depuis le PFD en tournant les encoreurs rotatifs du cap
et de l'altitude
elles permetttent également de contrôler l'avion en mode autopilot et landing auto ILS (guidage glide et localizer)	  

-----------------------------------------------------------------------------------------------------------


<?xml version="1.0"?>


<PropertyList>

<generic>
    <output>
        <binary_mode>false</binary_mode>
        <line_separator>\n</line_separator>
        <var_separator>\n</var_separator>
        <preamble></preamble>
        <postamble></postamble>

        <chunk>
            <name>Joystick_tangage</name>
            <node>/devices/status/joysticks/joystick/axis[1]</node>
            <type>integer</type>
            <factor>1000</factor>
            <format>joystick1=%i</format>
        </chunk>

        <chunk>
            <name>Altitude</name>
            <node>/instrumentation/gps/indicated-altitude-ft</node>
            <type>integer</type>

            <factor>1000</factor>
            <format>alti=%i</format>
        </chunk>

        <chunk>
			<name>altitude_agl</name>
			<type>integer</type>
			<node>/position/altitude-agl-ft</node>
			<format>alti_agl=%i</format>
		</chunk>

		<chunk>
			<name>ground_elevation</name>
			<type>integer</type>
			<node>/position/ground-elev-ft</node>
			<format>gnd_elv=%i</format>
		</chunk>
        

        <chunk>
            <name>Speed</name>
            <node>/velocities/airspeed-kt</node>
            <type>integer</type>
            <format>speed=%i</format>
        </chunk>

        <chunk>
            <name>Tangage</name>
            <node>/orientation/pitch-deg</node>
            <type>integer</type>
            <format>pitch=%i</format>
        </chunk>

        <chunk>
            <name>Roulis</name>
            <node>/orientation/roll-deg</node>
            <type>integer</type>
            <format>roll=%i</format>
        </chunk>

        <chunk>
            <name>Cap</name>
            <node>/orientation/heading-deg</node>
            <type>integer</type>
            <factor>100</factor>
            <format>heading=%i</format>
        </chunk>

        <chunk>
            <name>Vertical_speed</name>
            <node>/velocities/vertical-speed-fps</node>
            <type>integer</type>
            <format>vspeed=%i</format>
        </chunk>

        <chunk>
            <name>latitude</name>
            <node>/position/latitude-deg</node>
            <type>integer</type>
            <factor>100000</factor>
            <format>latitude=%i</format>
        </chunk>

        <chunk>
            <name>longitude</name>
            <node>/position/longitude-deg</node>
            <type>integer</type>
            <factor>100000</factor>
            <format>longitude=%i</format>
        </chunk>
        
		<chunk>
			<name>Nav1_actual</name>
			<type>integer</type>
			<node>/instrumentation/nav/radials/actual-deg</node>
			<factor>100</factor>
			<format>vor_actual=%i</format>
		</chunk>
		

    </output>


    <input>
		<line_separator>\n</line_separator>
        <var_separator>,</var_separator>

       <chunk>
         <name>heading_bug</name>
         <node>/autopilot/settings/heading-bug-deg</node>
         <type>integer</type>
         <relative>false</relative>
     </chunk>

     <chunk>
         <name>asel</name>
         <node>/autopilot/settings/asel</node>
         <type>integer</type>
         <relative>false</relative>
     </chunk>

     
	<chunk>
		<name>landing-light1</name>
		<node>/controls/lighting/landing-light</node>
		<type>integer</type>
		<relative>false</relative>
	</chunk>

	<chunk>
		<name>landing-light2</name>
		<node>/controls/lighting/landing-light[1]</node>
		<type>integer</type>
		<relative>false</relative>
	</chunk>


	<chunk>
		<name>target_speed</name>
		<node>/autopilot/settings/target-speed-kt</node>
		<type>integer</type>
		<relative>false</relative>
	</chunk>


	<chunk>
		<name>minimums</name>
		<node>/autopilot/settings/minimums</node>
		<type>integer</type>
		<relative>false</relative>
	</chunk>


	<chunk>
		<name>ailerons</name>
		<node>/controls/flight/aileron</node>
		<type>float</type>
		<relative>false</relative>
	</chunk>
		

	<chunk>
		<name>rudder</name>
		<node>/controls/flight/rudder</node>
		<type>float</type>
		<relative>false</relative>
	</chunk>


	<chunk>
		<name>view_number</name>
		<node>/sim/current-view/view-number</node>
		<type>integer</type>
		<relative>false</relative>
	</chunk>


	<chunk>
		<name>climb_rate</name>
		<node>/autopilot/settings/target-climb-rate-fps</node>
		<type>float</type>
		<relative>false</relative>
	</chunk>	

	
	<chunk>
		<name>locks</name>
		<node>/autopilot/locks/altitude</node>
		<type>string</type>
	</chunk>


	<chunk>
		<name>AP_status</name>
		<node>/autopilot/locks/AP-status</node>
		<type>string</type>
	</chunk>


	<chunk>
		<name>Speed_ctrl</name>
		<node>/autopilot/locks/speed-ctrl</node>
		<type>bool</type>
	</chunk>	


	<chunk>
		<name>elevator</name>
		<node>/controls/flight/elevator</node>
		<type>float</type>
		<relative>false</relative>
	</chunk>


	<chunk>
		<name>throttle</name>
		<node>/controls/engines/throttle-all</node>
		<type>float</type>
		<relative>false</relative>
	</chunk>
	

	<chunk>
		<name>reverser1</name>
		<node>/controls/engines/engine/reverser</node>
		<type>bool</type>
	</chunk>
	

	<chunk>
		<name>reverser2</name>
		<node>/controls/engines/engine[1]/reverser</node>
		<type>bool</type>
	</chunk>


	<chunk>
		<name>flaps</name>
		<node>/controls/flight/flaps-select</node>
		<type>integer</type>
		<relative>false</relative>
	</chunk>


	<chunk>
		<name>speedbrake</name>
		<node>/controls/flight/speedbrake</node>
		<type>float</type>
		<relative>false</relative>
	</chunk>	


	<chunk>
		<name>gear_down</name>
		<node>/controls/gear/gear-down</node>
		<type>bool</type>
		<relative>false</relative>
	</chunk>


	<chunk>
		<name>brake_left</name>
		<node>/controls/gear/brake-left</node>
		<type>float</type>
		<relative>false</relative>
	</chunk>
	

	<chunk>
		<name>brake_right</name>
		<node>/controls/gear/brake-right</node>
		<type>float</type>
		<relative>false</relative>
	</chunk>

  
   </input>



</generic>

</PropertyList>



>



**/
