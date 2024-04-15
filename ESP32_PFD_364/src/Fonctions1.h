//
// ==================================
	String version1="36.4";
// ==================================


#ifndef FONCTIONS1_H
#define FONCTIONS1_H

#define _pi 3.141592653
float raddeg =_pi/180.0;


// COULEURS RGB565
// Outil de sélection -> http://www.barth-dev.de/online/rgb565-color-picker/

#define NOIR   		0x0000
#define MARRON   	0x9240
#define ROUGE  		0xF800
#define ROSE		0xFBDD
#define ORANGE 		0xFBC0
#define JAUNE   	0xFFE0
#define JAUNE_PALE  0xF7F4
#define VERT   		0x07E0
#define VERT_FONCE  0x02E2
#define PAMPA		0xBED6
#define OLIVE		0x05A3
#define CYAN    	0x07FF
#define BLEU_CLAIR  0x455F
#define AZUR  		0x1BF9
#define BLEU   		0x001F
#define BLEU_FONCE	0x0005
#define MAGENTA 	0xF81F
#define VIOLET1		0x781A
#define VIOLET2		0xECBE
#define GRIS_TRES_CLAIR 0xDEFB
#define GRIS_CLAIR  0xA534
#define GRIS   		0x8410
#define GRIS_FONCE  0x5ACB
#define GRIS_TRES_FONCE  0x2124

#define GRIS_AF 	0x51C5		// 0x3985
#define BLANC   	0xFFFF

// AVIONIQUE : Horizon Artificiel (HA):

#define HA_CIEL		0x33FE
#define HA_SOL		0xAA81 //0xDB60


// Width and height of sprite
#define SPR_W 25
#define SPR_H 16

char var_array16[6]; //  5 char + zero terminal - pour envoi par WiFi (because 2^16 -1 = 65535 -> 5 caractères)
char var_array32[10];// 10 char + zero terminal - pour envoi par WiFi (because 2^32 -1 = 4294967295 -> 10 caractères)



uint16_t flags=0; // 16 valeurs booléennes dont les bits sont précisés ci-dessous
// attention : pas de notations du type 1<<3 (dans les #define) qui ne sont pas, dans ce cas, précalculées par le compilateur
// mais recopiées telle quelle dans le code
// ce qui entrainerait des erreurs (à l'exécution) par la suite!
// à la rigueur, des notations binaires...


// valeurs des bits pour chaque attribution :

# define bit_au_sol 1 // bit0
# define bit_decollage 2
# define bit_FG_AP 4
# define bit_autoland 8 // procedure d'approche
# define bit_atterrissage 16  // finale et touché des roues
# define bit_rudder_decol 32
# define bit_rudder_attero 64
# define bit_route 128
# define bit_nav_to_piste 256
# define bit_nav_to_pti 512
# define bit_nav_to_ptAA 1024
# define bit_nav_to_ptBB 2048
# define bit_att_short 4096 // atterrissage court (avec reverse, etc..) pour pistes très courtes
# define bit_roulage 8192
# define bit_circling 16384 // tour de piste
# define bit_sens_circling 32768 // sens du tour de piste

// pour les boutons

# define bit_bt1 1
# define bit_bt2 2
# define bit_bt3 4
# define bit_bt4 8
# define bit_bt5 16 


/* ******************************************************************************** */


void set_bit(uint16_t *mot_i, uint16_t val_bit) // val_bit étant la valeur du bit (le poids) c.a.d =1 ou 2 ou 4 ou 8...
{
	*mot_i |= val_bit;
}


void raz_bit(uint16_t *mot_i, uint16_t val_bit) // val_bit étant la valeur du bit (le poids) c.a.d =1 ou 2 ou 4 ou 8...
{
	*mot_i &= ~val_bit;
}


void inv_bit(uint16_t *mot_i, uint16_t val_bit) // val_bit étant la valeur du bit (le poids) c.a.d =1 ou 2 ou 4 ou 8...
{
	*mot_i ^= val_bit;
}


uint8_t read_bit(uint16_t mot_i, uint16_t val_bit) // val_bit étant la valeur du bit (le poids) c.a.d =1 ou 2 ou 4 ou 8...
{
	if ((mot_i & val_bit) == 0 ) {return 0;} else {return 1;}
}



uint8_t is_in(float valeur_i, float min, float max)
{
	if ((valeur_i >= min) && (valeur_i <= max)) {return 1;} else {return 0;}
}


void borne_in(float *valeur_i, float min, float max)
{
	if (*valeur_i < min) {*valeur_i = min;} 
	if (*valeur_i > max) {*valeur_i = max;}
}




float distance_AB(float lat_A, float lon_A, float lat_B, float lon_B)
{
	float R = 6373.0; // en km : rayon de la Terre

//conversions en radians
	lat_A *= raddeg;	lat_B *= raddeg;
	lon_A *= raddeg;	lon_B *= raddeg;

	float dlat = lat_A - lat_B;
	float dlon = lon_A - lon_B;
			
	float a = pow(sin(dlat / 2.0),2) + cos(lat_A) * cos(lat_B) * pow(sin(dlon / 2.0),2);
	float c = 2.0 * atan2(sqrt(a), sqrt(1 - a));
		
	float d= R * c;
	return d;  // en km
}


/*
 calcul angle entre deux points WGS84
 voir https://fr.wikipedia.org/wiki/Trigonom%C3%A9trie_sph%C3%A9rique
 latitudes et longitudes en degrés décimaux (ex: 43.47684)
 nonbre négatif si longitude Ouest
*/
float azimut_AB(float lat_A, float lon_A, float lat_B, float lon_B)  //retour float en degrés
{
//conversions en radians
	lat_A *= raddeg;	lat_B *= raddeg;
	lon_A *= raddeg;	lon_B *= raddeg;
	
	float delta_lon = lon_B - lon_A;

	float y = sin(delta_lon) * cos(lat_B);
	float x = cos(lat_A)*sin(lat_B) - sin(lat_A)*cos(lat_B)*cos(delta_lon);
	float resultat = atan2(y, x)/raddeg;
	if (resultat<0) {resultat += 360.0;}
	return resultat; //retour en degrés
}



float arc_cos_sin(float cosAlpha, float sinAlpha)
{
//cettte fonction retourne l'angle en fontion du couple [cos, sin] ce qui lève l'ambiguite sur 4 quadrants
	if ( sinAlpha >= 0 ) {return acos(cosAlpha);} else {return -acos(cosAlpha);}
}



void bargraph_H_float(float v, uint16_t x0, uint16_t y0, uint16_t couleur) // [-1.0 .. +1.0]
{
// horizontal	
	TFT480.fillRect(x0-100, y0, 200, 3, GRIS_TRES_FONCE);

	borne_in(&v, -1, 1);
	
	int16_t dx = (int16_t) (100 * v);
	
	if(dx <0){TFT480.fillRect(x0+dx, y0, -dx, 3, couleur);}
	if(dx >0){TFT480.fillRect(x0, y0, dx, 3, couleur);}
	
}


void bargraph_V_float(float v, uint16_t x0, uint16_t y0, uint16_t couleur) // [-1.0 .. +1.0]
{
// vertical	
	TFT480.fillRect(x0, y0-100, 3, 200, GRIS_TRES_FONCE);

	borne_in(&v, -1, 1);
	
	int16_t dy = (int16_t) (100 * v);

	if(dy <0){TFT480.fillRect(x0, y0+dy, 3, -dy, couleur);}
	if(dy >0){TFT480.fillRect(x0, y0, 3, dy, couleur);}
	
}


void affi_indexH(uint16_t x, uint16_t y, int8_t sens, uint16_t couleur) 
{
// petite pointe de flèche horizontale
// sens = +1 ou -1 pour orienter la pointe vers la droite ou vers la gauche

	TFT480.fillTriangle(x, y-4,  x, y+4,  x+8*sens, y, couleur);
}


void affi_indexV(uint16_t x, uint16_t y, int8_t sens, uint16_t couleur) 
{
// petite pointe de flèche verticale
// sens = +1 ou -1 pour orienter la pointe vers le haut ou vers le bas

	TFT480.fillTriangle(x-4, y,  x+4, y,  x, y+8*sens, couleur);
}

void affi_int_en_binaire(uint8_t v, uint16_t x0,uint16_t y0)
{
	TFT480.fillRect(x0-72, y0, 80, 12, GRIS_FONCE); // efface le nombre précédemment affiché
	char s1;
	for(int i=0; i<8; i++)
	{
		if (( ((v >> i) & 1) != 0 ) == true) {s1='1';} else {s1='0';}
		TFT480.drawChar(s1, x0-10*i, y0); // affiche de droite à gauche les bits 0..7
	}
}



void affi_int_test(uint32_t v, uint16_t x0, uint8_t num, uint16_t txt_couleur, uint16_t back_couleur) // num = numéro de l'emplacement (en hauteur)
{

	TFT480.setFreeFont(FM9);
	TFT480.setTextColor(txt_couleur);
	String s1=String(v);
	TFT480.fillRect(x0, 10+18*num, 200, 18, back_couleur); //efface
	TFT480.drawString(s1, x0, 10+18*num); 
}


void affi_float_test(float v, uint16_t x0, uint8_t num, uint16_t txt_couleur, uint16_t back_couleur) // num = numéro de l'emplacement (en hauteur)
{

	TFT480.setFreeFont(FM9);
	TFT480.setTextColor(txt_couleur);
	String s1=String(v, 6);
	TFT480.fillRect(x0, 10+18*num, 200, 18, back_couleur); //efface
	TFT480.drawString(s1, x0, 10+18*num); 
}


void affi_string_test(String s, uint16_t x0, uint8_t num, uint16_t txt_couleur, uint16_t back_couleur) // num = numéro de l'emplacement (en hauteur)
{

	TFT480.setFreeFont(FM9);
	TFT480.setTextColor(txt_couleur);
	TFT480.fillRect(x0, 10+18*num, 200, 18, back_couleur); //efface
	TFT480.drawString(s, x0, 10+18*num); 
}







#endif
