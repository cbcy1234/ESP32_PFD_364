#include <graphics.h>
#include <stdlib.h>
#include <stdio.h>
#include  <math.h>
#define r 30
#define p 10
#include "fonctions.h"
double c= sqrt(3)/2;
int quit=1;
int i;
extern int im1, im2, lang;
void pion1(int x,int y);
void pion2(int x,int y);
void hexagone(double x,double y) /// Pour faire une case du board
{
    setcolor(BLACK);
    setlinestyle(0,1,3);
    line(x,y, x+r, y);
    line(x+r,y,x+3*r/2, y+c*r);
    line(x+3*r/2, y+c*r,x+r, y+2*c*r);
    line(x+r, y+2*c*r,x,y+2*c*r);
    line(x,y+2*c*r,x-r/2,y+c*r);
    line(x-r/2,y+c*r,x,y);
    setfillstyle(SOLID_FILL , YELLOW);
    floodfill(x+6,y+6,BLACK);
}


void board(int langue, int image1,int image2) /// Pour faire le board complet
{
    double x= getmaxwidth()/2;
    double y=getmaxheight()/2-13*c*r;
    int i,j;
    for (i=0;i<7;i++)
    {
        for (j=0;j<13-i;j++)
        {
            hexagone(x+i*3*r/2,y+j*2*c*r);
            if(i>0)
            hexagone(x-i*3*r/2,y+j*2*c*r);
        }
        y=y+c*r;


    }
      setcolor(YELLOW);
      circle(5.2*r,getmaxheight()/2,5.3*r);
    setcolor(BLACK);
   circle(5.2*r,getmaxheight()/2,5*r);
    setfillstyle( SOLID_FILL , WHITE);
    floodfill(6.2*r,getmaxheight()/2,BLACK);
     pion1(5.2*r,getmaxheight()/2+6*r);
    setcolor(BLACK);
   circle(getmaxwidth()-5.2*r,getmaxheight()/2,5*r);
    setfillstyle( SOLID_FILL , WHITE);
    floodfill(getmaxwidth()-6.2*r,getmaxheight()/2,BLACK);
    pion2(getmaxwidth()-5.2*r,getmaxheight()/2+6*r);

     if (langue==0)
           {setcolor(BLACK);
             settextstyle(9,HORIZ_DIR,5);
            outtextxy(25,getmaxheight()/2-200,"JOUEUR 1");
            setcolor(WHITE);
           settextstyle(9,HORIZ_DIR,5);
           outtextxy(getmaxwidth()-312,getmaxheight()/2-200,"JOUEUR 2");
           }
        else
            if(langue==1)
             {setcolor(BLACK);
             settextstyle(9,HORIZ_DIR,5);
            outtextxy(25,getmaxheight()/2-200,"JWE 1");
            setcolor(WHITE);
     settextstyle(9,HORIZ_DIR,5);
     outtextxy(getmaxwidth()-312,getmaxheight()/2-200,"JWE 2");
             }
       else if(langue==2)
            {setcolor(BLACK);
             settextstyle(9,HORIZ_DIR,5);
            outtextxy(25,getmaxheight()/2-200,"PLAYER 1");

                 setcolor(WHITE);
     settextstyle(9,HORIZ_DIR,5);
     outtextxy(getmaxwidth()-312,getmaxheight()/2-200,"PLAYER 2");
            }
      else if(langue==3)
            {setcolor(BLACK);
             settextstyle(9,HORIZ_DIR,5);
            outtextxy(25,getmaxheight()/2-200,"JUGADOR 1");
                 setcolor(WHITE);
     settextstyle(9,HORIZ_DIR,5);
     outtextxy(getmaxwidth()-312,getmaxheight()/2-200,"JUGADOR 2");
            }




switch(image1) /// Pour faire le choix d'image des joueurs
{
case 1:
    {
        readimagefile("images/images-14.jpeg",5.2*r-100,(getmaxheight()/2)-80,5.2*r+100,(getmaxheight()/2)+80 );
        break;
    }
case 2:
    {
        readimagefile("images/images-15.jpeg",5.2*r-100,(getmaxheight()/2)-80,5.2*r+100,(getmaxheight()/2)+80 );
        break;
    }
case 3:
    {
       readimagefile("images/images-20.jpeg",5.2*r-100,(getmaxheight()/2)-80,5.2*r+100,(getmaxheight()/2)+80 );
       break;
    }
case 4:
    {
       readimagefile("images/images-25.jpeg",5.2*r-100,(getmaxheight()/2)-80,5.2*r+100,(getmaxheight()/2)+80 );
       break;
    }

}

switch(image2)
{
case 1:
    {
         readimagefile("images/images-14.jpeg",getmaxwidth()-5.2*r-100,(getmaxheight()/2)-80,getmaxwidth()-5.2*r+100,(getmaxheight()/2)+80 );
        break;
    }
case 2:
    {
         readimagefile("images/images-15.jpeg",getmaxwidth()-5.2*r-100,(getmaxheight()/2)-80,getmaxwidth()-5.2*r+100,(getmaxheight()/2)+80 );
        break;
    }
case 3:
    {
        readimagefile("images/images-20.jpeg",getmaxwidth()-5.2*r-100,(getmaxheight()/2)-80,getmaxwidth()-5.2*r+100,(getmaxheight()/2)+80 );
       break;
    }
case 4:
    {
         readimagefile("images/images-25.jpeg",getmaxwidth()-5.2*r-100,(getmaxheight()/2)-80,getmaxwidth()-5.2*r+100,(getmaxheight()/2)+80 );
       break;
    }
}


    setlinestyle(0,1,3);
    setcolor(YELLOW);
    circle(getmaxwidth()/2-12*r,getmaxheight()-6*c*r,0.8*r);///skip sur board
    setfillstyle( SOLID_FILL , WHITE);
    floodfill(getmaxwidth()/2-12*r+5,getmaxheight()-6*c*r+5,YELLOW);


setlinestyle(0,1,3);
rectangle(50,25,100,70);
rectangle(110,25,160,70);
rectangle(170,25,220,70);
readimagefile("images/images-28.jpeg",50,25,100,70 );
readimagefile("images/images-19.jpeg",110,25,160,70 );
readimagefile("images/images-33.jpeg",170,25,220,70 );
readimagefile("images/images-26.jpeg",getmaxwidth()/2-12*r-15,getmaxheight()-6*c*r-15,getmaxwidth()/2-12*r+15,getmaxheight()-6*c*r+15 );

readimagefile("images/fleche_retour.gif",getmaxwidth()-90,getmaxheight()-85,getmaxwidth()-20,getmaxheight()-20);
}



void pion1(int x,int y) /// Pour creer les pions noirs
{
    setcolor(7);
    setlinestyle(0,1,1);
   circle(x,y,p);
  setfillstyle(SOLID_FILL , BLACK);
   floodfill(x,y,LIGHTGRAY);
    setcolor(BLACK);
 circle(x,y,p);

}

void pion2(int x,int y) /// Pour creer les pions blancs
{
   setcolor(7);
   setlinestyle(0,1,1);
   circle(x,y,p);
   setfillstyle(SOLID_FILL ,WHITE);
   floodfill(x,y,LIGHTGRAY);
    setcolor(WHITE);
    circle(x,y,p);
}
  void load()
 {//cleardevice();
// setbkcolor(LIGHTGRAY);
     //readimagefile("images/exemple.jpg",300,0,getmaxwidth()-300,getmaxheight());
     setlinestyle(0,1,3);
    setcolor(BLUE);
     line(getmaxwidth()/2-101,getmaxheight()-81,getmaxwidth()/2+101,getmaxheight()-81);
     line(getmaxwidth()/2+101,getmaxheight()-81,getmaxwidth()/2+101,getmaxheight()-59);
     line(getmaxwidth()/2+101,getmaxheight()-59,getmaxwidth()/2-101,getmaxheight()-59);
     line(getmaxwidth()/2-101,getmaxheight()-59,getmaxwidth()/2-101,getmaxheight()-81);
     for (i=0;i<190;i++)
     {   delay(50);
         setcolor(WHITE);
         line(getmaxwidth()/2-100+i,getmaxheight()-80,getmaxwidth()/2-100+i,getmaxheight()-60);
         line(getmaxwidth()/2-100,getmaxheight()-80,getmaxwidth()/2-100+i,getmaxheight()-60);


     }
     for (i=190;i<200;i++) /// Ralentissement du load
     {
         setcolor(WHITE);
         line(getmaxwidth()/2-100+i,getmaxheight()-80,getmaxwidth()/2-100+i,getmaxheight()-60);
         line(getmaxwidth()/2-100,getmaxheight()-80,getmaxwidth()/2-100+i,getmaxheight()-60);
         delay(1000);

     }

          setcolor(YELLOW);
         line(getmaxwidth()/2+100,getmaxheight()-80,getmaxwidth()/2+100,getmaxheight()-60);

 }
void menu(int langue, int *im1, int *im2)
{
  int i, midx, y, x;
    setbkcolor(LIGHTGRAY);
    cleardevice();

    //load();
    cleardevice();




    line(20, 150, 35, 120);
    line(35, 120, 335, 120);
    line(335, 120, 345, 150);
    line(345, 150, 335, 180);
    line(335, 180, 35, 180);
    line(35, 180, 20, 150);
    line(335, 180, 345, 210);
    line(345, 210, 335, 240);
    line(335, 240, 35, 240);
    line(35, 240, 20, 210);
    line(20, 210, 35, 180);
    line(335, 240, 345, 270);
    line(345, 270, 335, 300);
    line(335, 300, 35, 300);
    line(35, 300, 20, 270);
    line(20, 270, 35, 240);
    line(335, 300, 345, 330);
    line(345, 330, 335, 360);
    line(335, 360, 35, 360);
    line(35, 360, 20, 330);
    line(20, 330, 35, 300);
  setfillstyle(SOLID_FILL , WHITE);
   floodfill(35,220,YELLOW);
    setfillstyle(SOLID_FILL , WHITE);
   floodfill(35,350,YELLOW);
    setfillstyle(SOLID_FILL , WHITE);
   floodfill(35,170,YELLOW);
    setfillstyle(SOLID_FILL , WHITE);
   floodfill(35,250,YELLOW);
   circle(45, 150, 5);
     circle(45, 210, 5);
     circle(45, 270, 5);
     circle(45, 330, 5);
   setbkcolor(WHITE);
    setcolor(BLACK);
   readimagefile("images/fleche_retour.gif",getmaxwidth()-90,getmaxheight()-85,getmaxwidth()-20,getmaxheight()-20);

    if (langue==0)
    {
        outtextxy(55, 140, "NOUVEAU JEU");
       outtextxy(55, 200, "COMMENT JOUER");
       outtextxy(55, 260, "PARAMETRES");
      outtextxy(55, 320, "QUITTER");
      setbkcolor(LIGHTGRAY);
      outtextxy(30, 30, "Bienvenue sur Rosette");
    }
    else
        if(langue==1)

        {

           outtextxy(55, 140, "NOUVO PATI");
            outtextxy(55, 200, "KIJANW JWE");
             outtextxy(55, 260, "PARAMET");
             outtextxy(55, 320, "KITE");
             setbkcolor(LIGHTGRAY);
             outtextxy(30, 30, "Byenvini sou Rosette");
        }
    else if(langue==2)
    {

        outtextxy(55, 140, "NEW GAME");
       outtextxy(55, 200, "HOW TO PLAY");
       outtextxy(55, 260, "SETTINGS");
        outtextxy(55, 320, "QUIT");
        setbkcolor(LIGHTGRAY);
        outtextxy(30, 30, "Welcome to Rosette");
    }
    else if(langue==3)
    {

        outtextxy(55, 140, "NUEVO JUEGO");
       outtextxy(55, 200, "COMO JUGAR");
       outtextxy(55, 260, "CONFIGURACION");
         outtextxy(55, 320, "DEJAR");
         setbkcolor(LIGHTGRAY);
         outtextxy(30, 30, "Bienvenido al Rosette");
    }

setbkcolor(LIGHTGRAY);

int step_im=1;
int ima1= 0;
int ima2 = 0;
int next=0;
  while(next==0)

{
    while(! ismouseclick(WM_LBUTTONDOWN));

        getmouseclick(WM_LBUTTONDOWN, x, y);


       if(((x>=35) && (x<=335)) && (y>=120) && (y<=180)) ///Nouveau Jeu
       {
           setcolor(YELLOW);
           rectangle(getmaxwidth()/2-300,50,getmaxwidth()/2+600,700);
           setfillstyle(SOLID_FILL,WHITE);
           floodfill(getmaxwidth()/2+90,90,YELLOW);
           setcolor(LIGHTGRAY);
           rectangle(getmaxwidth()/2-275,100,getmaxwidth()/2-25,350);
           rectangle(getmaxwidth()/2+25,100,getmaxwidth()/2+275,350);
           rectangle(getmaxwidth()/2+325,100,getmaxwidth()/2+575,350);
           rectangle(getmaxwidth()/2-275,400,getmaxwidth()/2-25,650);
           rectangle(getmaxwidth()/2+25,400,getmaxwidth()/2+275,650);
           rectangle(getmaxwidth()/2+325,400,getmaxwidth()/2+575,650);
          readimagefile("images/images-14.jpeg",getmaxwidth()/2-250,125,getmaxwidth()/2-50,325 );
          readimagefile("images/images-15.jpeg",getmaxwidth()/2+50,125,getmaxwidth()/2+250,325 );
          readimagefile("images/images-20.jpeg",getmaxwidth()/2+350,125,getmaxwidth()/2+550,325 );
          readimagefile("images/images-25.jpeg",getmaxwidth()/2-250,425,getmaxwidth()/2-50,625 );
          readimagefile("images/locked.jpeg",getmaxwidth()/2+50,425,getmaxwidth()/2+250,625);
          readimagefile("images/arton2296.jpg",getmaxwidth()/2+350,425,getmaxwidth()/2+550,625);

          setcolor(RED);
            setbkcolor(WHITE);
          settextstyle(9,HORIZ_DIR,1);
          outtextxy(getmaxwidth()/2-263,110,"Vico Allen FOREST");

          setcolor(RED);
            setbkcolor(WHITE);
          settextstyle(9,HORIZ_DIR,1);
          outtextxy(getmaxwidth()/2+26,110,"Barbara G. THEODORE");

         setcolor(RED);
            setbkcolor(WHITE);
          settextstyle(9,HORIZ_DIR,1);
          outtextxy(getmaxwidth()/2+363,110,"Rolex JOSEPH");

          setcolor(RED);
            setbkcolor(WHITE);
          settextstyle(9,HORIZ_DIR,1);
          outtextxy(getmaxwidth()/2-263,405,"Edgard ETIENNE");


         /*setcolor(RED);
         setbkcolor(LIGHTGRAY);
          settextstyle(9,HORIZ_DIR,5);*/

          if (langue==0)
           {setcolor(BLACK);
            setbkcolor(LIGHTGRAY);
          settextstyle(9,HORIZ_DIR,1);
          outtextxy(getmaxwidth()/2+350,425,"VEROUILLE");

          setcolor(BLACK);
        setbkcolor(LIGHTGRAY);
          settextstyle(9,HORIZ_DIR,1);
          outtextxy(getmaxwidth()/2+50,425,"VEROUILLE");

        setcolor(BLACK);
          settextstyle(9,HORIZ_DIR,5);
            outtextxy(55,420,"JOUEUR 1");
            setcolor(WHITE);
            settextstyle(9,HORIZ_DIR,5);
            outtextxy(55,520,"JOUEUR 2");
           }
        else
            if(langue==1)
             {setcolor(BLACK);
            setbkcolor(LIGHTGRAY);
          settextstyle(9,HORIZ_DIR,1);
          outtextxy(getmaxwidth()/2+350,425,"BLOKE");
          setcolor(BLACK);
         setbkcolor(LIGHTGRAY);
          settextstyle(9,HORIZ_DIR,1);
          outtextxy(getmaxwidth()/2+50,425,"BLOKE");

        setcolor(BLACK);
          settextstyle(9,HORIZ_DIR,5);
            outtextxy(55,420,"JWE 1");
            setcolor(WHITE);
            settextstyle(9,HORIZ_DIR,5);
         outtextxy(55,520,"JWE 2");
             }
       else if(langue==2)
            {setcolor(BLACK);
            setbkcolor(LIGHTGRAY);
          settextstyle(9,HORIZ_DIR,1);
          outtextxy(getmaxwidth()/2+350,425,"LOCKED");
          setcolor(BLACK);
         setbkcolor(LIGHTGRAY);
          settextstyle(9,HORIZ_DIR,1);
          outtextxy(getmaxwidth()/2+50,425,"LOCKED");

        setcolor(BLACK);
          settextstyle(9,HORIZ_DIR,5);
         outtextxy(55,420,"PLAYER 1");
         setcolor(WHITE);
            settextstyle(9,HORIZ_DIR,5);
        outtextxy(55,520,"PLAYER 2");
            }
      else if(langue==3)
            {setcolor(BLACK);
            setbkcolor(LIGHTGRAY);
          settextstyle(9,HORIZ_DIR,1);
          outtextxy(getmaxwidth()/2+350,425,"loka");
          setcolor(BLACK);
         setbkcolor(LIGHTGRAY);
          settextstyle(9,HORIZ_DIR,1);
          outtextxy(getmaxwidth()/2+50,425,"loka");

       setcolor(BLACK);
          settextstyle(9,HORIZ_DIR,5);
        outtextxy(55,420,"JUGADO 1");
         setcolor(WHITE);
            settextstyle(9,HORIZ_DIR,5);
             outtextxy(55,520,"JUGADO 2");
             }
             while(ima2 == 0)

{
    while(! ismouseclick(WM_LBUTTONDOWN));

        getmouseclick(WM_LBUTTONDOWN, x, y);
        if (step_im==1)
          {   setcolor(BLACK);
              ima1 = choiximage(x,y);
              setcolor(BLACK);
              rectangle(45, 410, 345, 460);
              if (ima1 != 0)
              step_im = 2;
          }

          else if (step_im == 2)
          {   setcolor(YELLOW);
              ima2 = choiximage(x,y);
              setcolor(YELLOW);
              rectangle(45,500,345,570);
              *im1= ima1;
              *im2= ima2;

          }
           printf("%d %d \n", ima1,ima2);
           next=1;

}

          }

          if(((x>=35) && (x<=335)) && (y>=180) && (y<=240)) /// Comment jouer
           {
               system("start Comment_jouer.pdf");
           }
         if (((x>=35) && (x<=335)) && (y>=300) && (y<=360)) /// Quitter
         {

             exit(0);
         }
        delay(1500);
        }


}




 int accueil()
 {
    int midx, midy,i,x, y;
    int LANGUE=8;

    setbkcolor(WHITE);
    cleardevice();


    midx = getmaxx() / 2;
    midy = getmaxy() / 2;

    settextstyle(10, HORIZ_DIR, 10);
    for(i=0;i<midy-50;i++)
       {
           setcolor(RED);
           outtextxy(midx-200, i, "V");
            //delay(10);
       }
    for(i=0;i<midy-50;i++)
       {
           setcolor(YELLOW);
           outtextxy(midx-100, i, "I");
           // delay(10);
       }
    for(i=0;i<midy-50;i++)
       {
           setcolor(GREEN);
           outtextxy(midx, i, "B");
          //delay(10);
       }
    for(i=0;i<midy-50;i++)
       {
           setcolor(BLUE);
           outtextxy(midx+100, i, "R");
          // delay(10);
       }
    for(i=0;i<midy-50;i++)
       {
           setcolor(LIGHTGRAY);
           outtextxy(midx+200, i, "O");
          //delay(10);
       }
    setcolor(LIGHTCYAN);
    settextstyle(1, HORIZ_DIR, 1);
    outtextxy(midx-50, midy+50,"PRODUCTION");
    delay(5000);

    setcolor(BLACK);
    outtextxy(getmaxx()/2-170,getmaxy()-50,"FDS~Tous droits reserves~Juin 2k16");
    delay(5000);
     setbkcolor(WHITE);
     cleardevice();

    setlinestyle(0,1,3);
    setcolor(YELLOW);
    rectangle(midx-350,0,midx+350,getmaxheight());
    setfillstyle(SOLID_FILL,WHITE);
    floodfill(midx,50,YELLOW);
    settextstyle(10, HORIZ_DIR, 10);
    outtextxy(midx-250, 20, "ROSETTE");

readimagefile("images/images-2.jpg",midx-320,20,midx-260,80 );
readimagefile("images/images-27.jpeg",getmaxwidth()/2-300,getmaxheight()/2,getmaxwidth()/2+300,getmaxheight()/2+300 );

    settextstyle(9, HORIZ_DIR, 1);
    setcolor(RED);
    for(i=0;i<midx-35;i++)
    {
      outtextxy(i, midy-60, "Francais");
    }

    for(i=getmaxx();i>midx-30;i--)
    {
         outtextxy(i, midy-20, "Kreyol");
    }

    for(i=0;i<midx-35;i++)
    {
        outtextxy(i, midy+20, "English");
    }

    for(i=getmaxx();i>midx-35;i--)
    {
        outtextxy(i, midy+60, "Espanol");
    }
    setlinestyle(0,1,3);
    setcolor(YELLOW);
    rectangle(midx-350,0,midx+350,getmaxheight());

    setfillstyle(SOLID_FILL,LIGHTGRAY);
    floodfill(midx-500,50,YELLOW);
    setfillstyle(SOLID_FILL,LIGHTGRAY);
    floodfill(midx+500,50,YELLOW);


int quit = 1;
do
{
    quit = 0;
    while(! ismouseclick(WM_LBUTTONDOWN)); /// Choix de Langue

    {
        getmouseclick(WM_LBUTTONDOWN, x, y);
        if(((x>=midx-40) && (x<=midx+40)) && (y>=midy-60) && (y<=midy-50))
           LANGUE = 0;
           else
                  if(((x>=midx-35) && (x<=midx+35)) && (y>=midy+20) && (y<=midy+30))
                 LANGUE = 2;
                  else
                      if(((x>=midx-35) && (x<=midx+35)) && (y>=midy+60) && (y<=midy+70))
                       LANGUE = 3;
                    else
                       if(((x>=midx-30) && (x<=midx+30)) && (y>=midy-20) && (y<=midy-10))
                        {
                            LANGUE = 1;
                        }
                        else

                            quit =1;

    printf("%d\n",LANGUE);
   }
}while (quit);
return LANGUE;
 }



