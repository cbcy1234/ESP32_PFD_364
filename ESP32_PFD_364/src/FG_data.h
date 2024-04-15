#ifndef FG_DATA_H
#define FG_DATA_H

//
// ==================================
	String version_datas="36.4";
// ==================================



/*
Liste des balises VOR et ILS

RAMARQUE : Ce fichier est commun aux codes sources FG_panel et FG_radio, il ne doit figurer qu'une fois sur le disque
la seconde occurence nécessaire pour la compilation peut se faire par un lien symbolique (sous Linux),
ou par placement judicieux dans un dossier partagé par le système Arduino (comme c'est le cas par exemple pour les lib graphiques).

Quoi qu'il en soit, la même version doit être compilée dans les deux programmes,
afin de rendre les affichages cohérents entre les deux côtés, ainsi que leur utilisation.

Tout ceci dans le but d'alléger les données échangées par WiFi entre les deux montages, et donc le temps de traitement.
Seul le numéro de l'aérodrome choisi sera transmis.
*/

/*


todo:
-Calais
-Cherbourg
-Gibraltar
-Istres-Le-Tube
-La Rochelle
-Lannion
-Londres
-Lyon
-Madrid
-Nantes
-Naple
-Palma
-Perpignan-Rivesaltes
-Pise
-Reykjavik
-Rouen
-Salon-de-Provence
-St Nazaire
-St-Malo
-Tanger
-Venise
*/

/**
Lien : https://fr.wikipedia.org/wiki/Liste_des_codes_OACI_des_a%C3%A9roports/L
**/

//ATTENTION : pour toute modif de ce fichier, il faut recompiler le source ET reprogrammer le ND ainsi que le PFD afin
// qu'ils exploitent bien les même données.
// Le ND envoie le n° de balise en cours au PFD (par WiFi) qui en déduit la fréquence radio 
// de la balise (par lecture du code de ce fichier inclus) et l'envoie à Flightgear par l'USB
// donc la transmission ND->PFD c'est juste un nombre, qui permet ensuite au PFD de connaitre toutes les caractéristiques de l'aérodrome.
// ce nombre est transmis sous la forme d'un argument lors d'une requête html.
// Voir la requête "void httpGetHDG()" du ND, et la fonction "setup()" du PFD

/*
struct FGBAL
{
	char ID_OACI[4+1];
	char nom[18+1];
	float orient_piste;  // orientation de la piste
	uint16_t altitude;   // (feet)
	float latitude;
	float longitude;
	uint16_t longueur; //en m
};
*/


struct FGBAL
{
	char ID_OACI[4+1];
	char nom[18+1];
	uint16_t altitude;   // (en feet)
 // extrémités de la piste (A et B):	
	float lat_A;
	float lon_A;
	float lat_B;
	float lon_B;
	char  sens_app_interdit;  // 'A', 'B' ou 'X' bout par lequel on doit aborder la piste en finale. X =indifférent, aléatoire
	uint16_t niveau_de_vol_mini;
	uint8_t dst_pt_AB; // distance du point AA (resp. BB) dont les coordonnées seront calculées lors du choix de l'aérodrome
	
	//float orient_piste;  // orientation de la piste // NON ! SERA CALCULEE
	//uint16_t longueur; //en m SERA CALCULEE
};


//------------------------------------------------------------------------------------------------------------------
// un truc pour la route : les data sont à retrouver dans le fichier apt.dat de FlightGear
// ce fichier fait 2,4 millions de lignes, toutefois il s'ouvre en 2 secondes avec Geany
// (il n'en va pas de même avec un traitement de texte !!!)
// chercher (par exemple) "LFMT"


uint16_t nb_elements=0;
struct FGBAL liste_bali[101]; // si > 100 aérodromes, augmenter la taille de cette variable sinon crash du programme


char n_bal_array[4]; // 3 char + zero terminal - pour envoi par WiFi

void init_FG_bali()
{
	uint16_t n =0;
	

//KSFO San Francisco Intl
// 10L  37.62872250 -122.39342127   28R  37.61351918 -122.35716907
// 10R  37.62527727 -122.39074787   28L  37.61169441 -122.35837447
// 01R  37.60620279 -122.38114713   19L  37.62781186 -122.36681630
// 01L  37.60897310 -122.38223043   19R  37.62719258 -122.37015433

	strcpy(liste_bali[n].ID_OACI, "KSFO");
	strcpy(liste_bali[n].nom, "San Francisco"); 
	liste_bali[n].altitude = 0;
	liste_bali[n].lat_A = 37.62872250;
	liste_bali[n].lon_A = -122.39342127;
	liste_bali[n].lat_B = 37.61351918;
	liste_bali[n].lon_B = -122.35716907;
	liste_bali[n].sens_app_interdit ='X';
	liste_bali[n].niveau_de_vol_mini = 40; // il y a des sommets à 100m dans les parages...
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;

// 14 1 0 LEBL Barcelona - El Prat
// 02   41.28775900  002.08483610   20   41.30938568  002.09470900
// 07R  41.28231220  002.07434999   25L  41.29222053  002.10328100
// 07L  41.29324514  002.06727496   25R  41.30572604  002.10372899
	strcpy(liste_bali[n].ID_OACI, "LEBL");
	strcpy(liste_bali[n].nom, "BARCELONA"); 
	liste_bali[n].altitude = 13;
	liste_bali[n].lat_A = 41.28231220; // 07R , celle // au rivage au bord de l'eau
	liste_bali[n].lon_A = 2.07434999;
	liste_bali[n].lat_B = 41.29222053;
	liste_bali[n].lon_B = 2.10328100;
	liste_bali[n].sens_app_interdit ='X';
	liste_bali[n].niveau_de_vol_mini = 30;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;

	
// LFRB Brest
// 07R  48.44338700 -004.43837200   25L  48.45236700 -004.39867000

	strcpy(liste_bali[n].ID_OACI, "LFRB");
	strcpy(liste_bali[n].nom, "BREST"); 
	liste_bali[n].altitude = 325;
	liste_bali[n].lat_A = 48.44338700; 
	liste_bali[n].lon_A = -4.43837200;
	liste_bali[n].lat_B = 48.45236700;
	liste_bali[n].lon_B = -4.39867000;
	liste_bali[n].sens_app_interdit ='X';
	liste_bali[n].niveau_de_vol_mini = 30;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;




// LESO San_Sebastian
// 04   43.35035700 -001.79739800   22   43.36268300 -001.78382400
	strcpy(liste_bali[n].ID_OACI, "LESO");
	strcpy(liste_bali[n].nom, "SAN SEBASTIAN"); 
	liste_bali[n].altitude = 15;
	liste_bali[n].lat_A = 43.35035700;
	liste_bali[n].lon_A = -1.79739800;
	liste_bali[n].lat_B = 43.36268300;
	liste_bali[n].lon_B = -1.78382400;
	liste_bali[n].sens_app_interdit ='X';
	liste_bali[n].niveau_de_vol_mini = 40;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;


// LFBD Bordeaux Merignac
// 05   44.81921870 -000.72896149    23   44.83874314 -000.70095469
// 11   44.83162304 -000.72924702    29   44.82546392 -000.69985752
	strcpy(liste_bali[n].ID_OACI, "LFBD");
	strcpy(liste_bali[n].nom, "BORDEAUX MERIGNAC");
	liste_bali[n].altitude = 166;
	liste_bali[n].lat_A = 44.81921870;
	liste_bali[n].lon_A = -0.72896149;
	liste_bali[n].lat_B = 44.83874314;
	liste_bali[n].lon_B = -0.70095469;
	liste_bali[n].sens_app_interdit ='B';
	liste_bali[n].niveau_de_vol_mini = 30;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;

	
// LFBO Toulouse Blagnac
// 14R  43.64411400  001.34593100    32L  43.61898100  001.37209700
// 14L  43.63736400  001.35762200    32R  43.61564400  001.38022500
	strcpy(liste_bali[n].ID_OACI, "LFBO");
	strcpy(liste_bali[n].nom, "TOULOUSE BLAGNAC"); 
	liste_bali[n].altitude = 499; // feet
	liste_bali[n].lat_A = 43.64411400;
	liste_bali[n].lon_A = 1.34593100;
	liste_bali[n].lat_B = 43.61898100 ;
	liste_bali[n].lon_B = 1.37209700;
	liste_bali[n].sens_app_interdit ='B';
	liste_bali[n].niveau_de_vol_mini = 30;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;

	
// LFBT Tarbes Ossun Lourdes
//   43.16598600 -000.01275000  20    43.19130000  000.00006900
// remarque : la piste est à cheval sur le Méridien d'Origine...
	strcpy(liste_bali[n].ID_OACI, "LFBT");
	strcpy(liste_bali[n].nom, "TARBES");  
	liste_bali[n].altitude = 1186; // feet
	liste_bali[n].lat_A = 43.16598600;
	liste_bali[n].lon_A = -0.01275000;
	liste_bali[n].lat_B = 43.19130000;
	liste_bali[n].lon_B = 0.00006900;
	liste_bali[n].sens_app_interdit ='A';
	liste_bali[n].niveau_de_vol_mini = 60;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;

	
// LFCK Mazamet
// 14   43.56266100  002.28210300  32   43.54983900  002.29626100
// attention: sommet (Pic de Nore) alt 1200m (3900 feet !) à 10NM au SE, donc dans le circuit d'approche
// donc décollages et approches par le nord si possible
 	strcpy(liste_bali[n].ID_OACI, "LFCK");
	strcpy(liste_bali[n].nom, "CASTRES MAZAMET");
	liste_bali[n].altitude = 741;
	liste_bali[n].lat_A = 43.56266100;
	liste_bali[n].lon_A = 2.28210300;
	liste_bali[n].lat_B = 43.54983900;
	liste_bali[n].lon_B = 2.29626100;
	liste_bali[n].sens_app_interdit ='B';
	liste_bali[n].niveau_de_vol_mini = 40;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;

	
// LFKB Bastia Poretta
// 16   42.56333900  009.47916800   34  42.54167700  009.48825000
	strcpy(liste_bali[n].ID_OACI, "LFKB");
	strcpy(liste_bali[n].nom, "BASTIA PORETTA");
	liste_bali[n].altitude = 26;
	liste_bali[n].lat_A = 42.56333900;
	liste_bali[n].lon_A = 9.47916800;
	liste_bali[n].lat_B = 42.54167700;
	liste_bali[n].lon_B = 9.48825000;
	liste_bali[n].sens_app_interdit ='X';
	liste_bali[n].niveau_de_vol_mini = 50;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;

	
// LFKC St Catherine
// 18   42.54117000  008.79282700  36   42.52035400  008.79302300
	strcpy(liste_bali[n].ID_OACI, "LFKC");
	strcpy(liste_bali[n].nom, "CALVI"); //  St.Catherine
	liste_bali[n].altitude = 209;
	liste_bali[n].lat_A = 42.54117000;
	liste_bali[n].lon_A = 8.79282700;
	liste_bali[n].lat_B = 42.52035400;
	liste_bali[n].lon_B = 8.79302300;
	liste_bali[n].sens_app_interdit ='B';
	liste_bali[n].niveau_de_vol_mini = 40;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;

	
// LFKF Figari Sud Corse
// 05   41.49452700  009.08600400  23   41.50960100  009.10791600
	strcpy(liste_bali[n].ID_OACI, "LFKF");
	strcpy(liste_bali[n].nom, "FIGARI SUD CORSE");
	liste_bali[n].altitude = 87;
	liste_bali[n].lat_A = 41.49452700;
	liste_bali[n].lon_A = 9.08600400;
	liste_bali[n].lat_B = 41.50960100;
	liste_bali[n].lon_B = 9.10791600;
	liste_bali[n].sens_app_interdit ='B';
	liste_bali[n].niveau_de_vol_mini = 40;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;


// LFKJ Napoleon Bonaparte
// 02   41.91186937  008.79496744   20   41.93149762  008.80723663
// 10   41.92444700  008.79174100   28   41.92372900  008.80440100
	strcpy(liste_bali[n].ID_OACI, "LFKJ");
	strcpy(liste_bali[n].nom, "AJACCIO"); // "NAPOLEON BONAPARTE"
	liste_bali[n].altitude = 17;
	liste_bali[n].lat_A = 41.91186937;
	liste_bali[n].lon_A = 8.79496744;
	liste_bali[n].lat_B = 41.93149762;
	liste_bali[n].lon_B = 8.80723663;
	liste_bali[n].sens_app_interdit ='B';
	liste_bali[n].niveau_de_vol_mini = 40;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;

	
// LFLB Aix-Les-Bains
// 18   45.64504575  005.87971068  36   45.63012950  005.88093496
// 18L  45.63803914  005.88121204  36R  45.63172344  005.88173284
	strcpy(liste_bali[n].ID_OACI, "LFLB");
	strcpy(liste_bali[n].nom, "CHAMBERY - Aix-L-B"); // Aix-Les-Bains
	liste_bali[n].altitude = 778;
	liste_bali[n].lat_A = 45.64504575;
	liste_bali[n].lon_A = 5.87971068;
	liste_bali[n].lat_B = 45.63012950;
	liste_bali[n].lon_B = 5.88093496;
	liste_bali[n].sens_app_interdit ='B'; // par moi ! (pas en réel bien sûr)
	liste_bali[n].niveau_de_vol_mini = 50;
	liste_bali[n].dst_pt_AB = 15; // NM sachant qu'une distance < 10NM avec un niveau de départ > 30 conduit à une descente musclée ! >> 5%
// dans le cas de cet aérodrome, c'est ça ou slalomer au dessus du lac entre les montagnes...
//(dans le brouillard, c'est encore mieux !)
	n++;
	

// LFLC Clermont-Ferrand_Auvergne
// 08   45.78482100  003.15074800  26   45.78860300  003.18918600

	strcpy(liste_bali[n].ID_OACI, "LFLC");
	strcpy(liste_bali[n].nom, "CLERMONT FERRAND"); 
	liste_bali[n].altitude = 1089;
	liste_bali[n].lat_A = 45.78482100;
	liste_bali[n].lon_A = 3.15074800;
	liste_bali[n].lat_B = 45.78860300;
	liste_bali[n].lon_B = 3.18918600;
	liste_bali[n].sens_app_interdit ='A';
	liste_bali[n].niveau_de_vol_mini = 50;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;
	

// LFLP Annecy Haute-Sovoie Mont Blanc Meythet
// 04   45.92345900  006.09231400  22   45.93480300  006.10529200
// 4C   45.92552100  006.09699400  22C  45.93149800  006.10380500
	strcpy(liste_bali[n].ID_OACI, "LFLP");
	strcpy(liste_bali[n].nom, "ANNECY"); // "ANNECY MEYTHET" Altitude 1521m 
	liste_bali[n].altitude = 1521;
	liste_bali[n].lat_A = 45.92345900;
	liste_bali[n].lon_A = 6.09231400;
	liste_bali[n].lat_B = 45.93480300;
	liste_bali[n].lon_B = 6.10529200;
	liste_bali[n].sens_app_interdit ='X';
	liste_bali[n].niveau_de_vol_mini = 50;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;


// LFLL Lyon Saint Exupery
// 18R  45.74659384  005.08596244   36L  45.71068847  005.09033087
	strcpy(liste_bali[n].ID_OACI, "LFLL");
	strcpy(liste_bali[n].nom, "Lyon Saint Exupery");
	liste_bali[n].altitude = 820; // ft
	liste_bali[n].lat_A = 45.74659384;
	liste_bali[n].lon_A = 5.08596244;
	liste_bali[n].lat_B = 45.71068847;
	liste_bali[n].lon_B = 5.09033087;
	liste_bali[n].sens_app_interdit ='X';
	liste_bali[n].niveau_de_vol_mini = 40;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;

	
// LFLY Lyon Bron
// 16   45.73501400  004.94093300  34   45.71932800  004.94761400
	strcpy(liste_bali[n].ID_OACI, "LFLY");
	strcpy(liste_bali[n].nom, "Lyon BRON");
	liste_bali[n].altitude = 659; // ft
	liste_bali[n].lat_A = 45.73501400;
	liste_bali[n].lon_A = 4.94093300;
	liste_bali[n].lat_B = 45.71932800;
	liste_bali[n].lon_B = 4.94761400;
	liste_bali[n].sens_app_interdit ='X';
	liste_bali[n].niveau_de_vol_mini = 50;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;

// LFMI Istres Le Tube
// 15   43.53777100  004.91330200    0.00 1207.92 3  1 0 1 33   43.50778900  004.93462600    0.00  114.00 3  0 0 1
// 15R  43.52330300  004.91811600    0.00    0.00 0  0 0 0 33L  43.51610700  004.92323400    0.00    0.00 1  0 0 0
	strcpy(liste_bali[n].ID_OACI, "LFMI");
	strcpy(liste_bali[n].nom, "Istres Le Tube");
	liste_bali[n].altitude = 81;
	liste_bali[n].lat_A = 43.53777100;
	liste_bali[n].lon_A = 4.91330200;
	liste_bali[n].lat_B = 43.50778900;
	liste_bali[n].lon_B = 4.93462600;
	liste_bali[n].sens_app_interdit ='X';
	liste_bali[n].niveau_de_vol_mini = 30;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;
	

// LFML Marseille Provence
// 13L  43.44912530  005.19729576   31R  43.42727470  005.22845736
// 13R  43.44130300  005.20302200   31L  43.42586400  005.22435300
	strcpy(liste_bali[n].ID_OACI, "LFML");	
	strcpy(liste_bali[n].nom, "MARSEILLE PROVENCE"); 
	liste_bali[n].altitude = 23; // au centre de la piste, qui est fort en pente !
	liste_bali[n].lat_A = 43.44912530;
	liste_bali[n].lon_A = 5.19729576 ;
	liste_bali[n].lat_B = 43.42727470;
	liste_bali[n].lon_B = 5.22845736;
	liste_bali[n].sens_app_interdit ='B';
	liste_bali[n].niveau_de_vol_mini = 40;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;
	

// LFMN Nice/Cote d'Azur
// 04R  43.64658511  007.20258040    22L  43.66554901  007.22843657
// 04L  43.65171156  007.20395740    22R  43.66807997  007.22647422
	strcpy(liste_bali[n].ID_OACI, "LFMN");
	strcpy(liste_bali[n].nom, "NICE COTE D'AZUR"); 
	liste_bali[n].altitude = 10;
	liste_bali[n].lat_A = 43.64658511;
	liste_bali[n].lon_A = 7.20258040;
	liste_bali[n].lat_B = 43.66554901;
	liste_bali[n].lon_B = 007.22843657;
	liste_bali[n].sens_app_interdit ='B';
	liste_bali[n].niveau_de_vol_mini = 30; // accès côté ouest, (relief côté est)
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;

	
//13L  43.58614900  003.95574600   31R  43.57281900  003.98222800
	strcpy(liste_bali[n].ID_OACI, "LFMT");
	strcpy(liste_bali[n].nom, "MONTPELLIER"); 
	liste_bali[n].altitude = 17;  // feet
	liste_bali[n].lat_A = 43.58614900;
	liste_bali[n].lon_A = 3.95574600;
	liste_bali[n].lat_B = 43.57281900;
	liste_bali[n].lon_B = 3.98222800;
	liste_bali[n].sens_app_interdit ='X';
	liste_bali[n].niveau_de_vol_mini = 30;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;


// LFMU Beziers Vias
// 10   43.32402100  003.34238700
// 28   43.32282300  003.36475900
	strcpy(liste_bali[n].ID_OACI, "LFMU");
	strcpy(liste_bali[n].nom, "BEZIERS VIAS"); 
	liste_bali[n].altitude = 52;

/*
// les valeurs ci-dessous sont celles du fichier d'origine de Flightgear (apt.dat)
	liste_bali[n].lat_A = 43.32402100; // 43.32402100
	liste_bali[n].lon_A = 3.34238700; // 3.34238700
						  
	liste_bali[n].lat_B = 43.32282300; // 43.32282300
	liste_bali[n].lon_B = 3.36475900; // 3.36475900
*/

// les valeurs ci-dessous sont celles mesurées avec l'UFO posé au sol en bout de piste
	liste_bali[n].lat_A = 43.32402254; // 43.32402100
	liste_bali[n].lon_A = 3.342375755; // 3.34238700
						  
	liste_bali[n].lat_B = 43.32282329; // 43.32282300
	liste_bali[n].lon_B = 3.364738854; // 3.36475900

  
	liste_bali[n].sens_app_interdit ='X';
	liste_bali[n].niveau_de_vol_mini = 30;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;


// LFTW Garons Navy
// 18   43.76929100  004.41555600  100.89    0.00 3  1 0 1
// 36   43.74559700  004.41722000  100.89    0.00 3  0 0 1
	strcpy(liste_bali[n].ID_OACI, "LFTW");
	strcpy(liste_bali[n].nom, "NIMES"); 
	liste_bali[n].altitude = 309;
	liste_bali[n].lat_A = 43.76929100;
	liste_bali[n].lon_A = 4.41555600;
	liste_bali[n].lat_B = 43.74559700;
	liste_bali[n].lon_B = 4.41722000;
	liste_bali[n].sens_app_interdit ='X';
	liste_bali[n].niveau_de_vol_mini = 30;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;

	
// LFMV Caumont
// 16   43.91080605  004.90321905    34   43.90662331  004.90428974 -> longueur 400m !!!
// 17   43.91556441  004.89936384    35   43.89895841  004.90384384  
// 17R  43.91292125  004.89919522    35L  43.90673757  004.90086420
	strcpy(liste_bali[n].ID_OACI, "LFMV");
	strcpy(liste_bali[n].nom, "AVIGNON Caumont"); 
	liste_bali[n].altitude = 124;
	liste_bali[n].lat_A = 43.91556441;
	liste_bali[n].lon_A = 4.89936384;
	liste_bali[n].lat_B = 43.89895841;
	liste_bali[n].lon_B = 4.90384384;
	liste_bali[n].sens_app_interdit ='X';
	liste_bali[n].niveau_de_vol_mini = 30;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;
	

// LFPG Paris Charles De Gaulle
// 08L  48.99566392  002.55215505    26R  48.99875734  002.61060285
// 09L  49.02473614  002.52489020    27R  49.02667786  002.56169447
// 08R  48.99292932  002.56581580    26L  48.99486268  002.60243818
// 09R  49.02064475  002.51305527    27L  49.02366525  002.57030334
	strcpy(liste_bali[n].ID_OACI, "LFPG");
	strcpy(liste_bali[n].nom, "PARIS Ch-De-Gaulle"); 
	liste_bali[n].altitude = 392;
	liste_bali[n].lat_A = 48.99566392;
	liste_bali[n].lon_A = 2.55215505;
	liste_bali[n].lat_B = 48.99875734;
	liste_bali[n].lon_B = 2.61060285;
	liste_bali[n].sens_app_interdit ='X';
	liste_bali[n].niveau_de_vol_mini = 30;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;

	
// LSGG Geneva
// 05   46.22579381  006.09093933   23   46.25039752  006.12706146
// 05L  46.23481646  006.09953181   23R  46.24001038  006.10715805

	strcpy(liste_bali[n].ID_OACI, "LSGG");
	strcpy(liste_bali[n].nom, "GENEVE");
	liste_bali[n].altitude = 1374;
	liste_bali[n].lat_A = 46.22579381;
	liste_bali[n].lon_A = 6.09093933;
	liste_bali[n].lat_B = 46.25039752;
	liste_bali[n].lon_B = 6.12706146;
	liste_bali[n].sens_app_interdit ='X';
	liste_bali[n].niveau_de_vol_mini = 145;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;


// LFNP Nizas
// 11   43.50372800  003.41673200    29   43.50577800  003.40853400

	strcpy(liste_bali[n].ID_OACI, "LFNP");
	strcpy(liste_bali[n].nom, "Nizas");
	liste_bali[n].altitude = 308; // = 94m
	liste_bali[n].lat_A = 43.50372800;
	liste_bali[n].lon_A = 3.41673200;
	liste_bali[n].lat_B = 43.50577800;
	liste_bali[n].lon_B = 3.40853400;
	liste_bali[n].sens_app_interdit ='X';
	liste_bali[n].niveau_de_vol_mini = 15;
	liste_bali[n].dst_pt_AB = 15; // NM
	n++;	

/*
	strcpy(liste_bali[n].ID_OACI, "TEST");
	strcpy(liste_bali[n].nom, "T1"); 	
	liste_bali[n].altitude = 0;
	liste_bali[n].lat_A = 0;
	liste_bali[n].lon_A = 0;
	liste_bali[n].lat_B = 0;
	liste_bali[n].lon_B = 0;
	liste_bali[n].sens_app_interdit ='X';
	n++;	
*/

	nb_elements =n;

}


#endif
