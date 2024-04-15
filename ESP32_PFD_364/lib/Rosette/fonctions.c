#include <graphics.h>
#include <stdlib.h>
#include <stdio.h>
#include<math.h>
#include"design.h"
#include"extras.h"
#include "fonctions.h"

void test(info_pion ip);
int undo(info_pion ip);
 extern int lang;
extern int im1;
extern int im2;

void recommencer()
{

         vider();
         setbkcolor(8);
         cleardevice();
         board(lang,im1,im2);


}
int undo(info_pion ip)
{
retirer(ip);
setbkcolor(8);
cleardevice();
board(lang,im1,im2);
afficher_pions();

}

int arrondit(float a,float reste)
{
    if(reste<0.5)
    {

    a = (int) a;
    }
    else
    {
        a= (int )a;
        a=a+1;
    }
    return a;
}
int jouer(int lang)
{ int im2,im1;
int switch1=0;
int switch2=0;
    double c= sqrt(3)/2;
         float xo,yo,u,v,k,l,n;
         float s,t;/// valeurs representant les decimaux apres k et l respectivement
         int i,j,x,y;
         yo=getmaxheight()/2-13*c*r;/// valeur en y, d'un axe qui passe par le premier hexagone
         xo=getmaxwidth()/2+r/2;/// valeur en x, d'un axe qui passe au milieu du board
         int player=1;
         int quit =1;

    while(quit)
    {
        while (!ismouseclick(WM_LBUTTONDOWN));

            getmouseclick(WM_LBUTTONDOWN,x,y);





           if(getpixel(x,y)!= 59015812)/// pour gerer les clics hors board(sur espace gris)
               {
                if(sqrt(pow(x-(getmaxwidth()/2-12*r),2)+pow(y-(getmaxheight()-6*c*r),2))<=r)///pour gerer les clics dans les lieux non-gris
                 {
                    if(player==1) /// pour alterner les joueurs
                       {
                           setlinestyle(0,1,3);
                           switch1=switch1+1;

                          player=2;
                          setcolor(YELLOW);
                        circle(getmaxwidth()-5.2*r,getmaxheight()/2,5.3*r);
                           setcolor(LIGHTGRAY);
                          circle(5.2*r,getmaxheight()/2,5.3*r);

                        }
                        else
                        {   switch2=switch2+1;
                            setlinestyle(0,1,3);
                            player=1;
                           setcolor(YELLOW);
                          circle(5.2*r,getmaxheight()/2,5.3*r);
                           setcolor(LIGHTGRAY);
                        circle(getmaxwidth()-5.2*r,getmaxheight()/2,5.3*r);
                        }
                 }
                else
                    {
                        switch1=0;
                        switch2=0;
                    }

                    if ((switch1==2)||(switch2==2))
                    {
                       if (nombre_pions1()>nombre_pions2())
                       {
                           cleardevice();
                           setbkcolor(LIGHTGRAY);
                           readimagefile("images/49320_capture.jpg",0,0,getmaxwidth(),getmaxheight());
                           if(lang==0)
                           {
                               setcolor(BLACK);
                               settextstyle(9,HORIZ_DIR,1);
                              outtextxy(getmaxwidth()/2-200,getmaxheight()/2,"FELICITATIONS JOUEUR 1");
                           }
                           else if(lang==1)
                           {
                               setcolor(RED);
                               settextstyle(9,HORIZ_DIR,1);
                              outtextxy(getmaxwidth()/2-200,getmaxheight()/2,"BON BAGAY JWE 1");
                           }
                           else if(lang==2)
                           {
                               setcolor(RED);
                               settextstyle(9,HORIZ_DIR,1);
                              outtextxy(getmaxwidth()/2-200,getmaxheight()/2,"CONGRATS PLAYER 1");
                           }
                           else if(lang==3)
                            {
                           setcolor(RED);
                            settextstyle(9,HORIZ_DIR,1);
                            outtextxy(getmaxwidth()/2-200,getmaxheight()/2,"JUGADOR 1");
                            }





                       }
                       else if(nombre_pions2()>=nombre_pions1())
                       {
                            cleardevice();
                           setbkcolor(LIGHTGRAY);
                           readimagefile("images/49320_capture.jpg",0,0,getmaxwidth(),getmaxheight());
                           if(lang==0)
                           {
                               setcolor(BLACK);
                               settextstyle(9,HORIZ_DIR,1);
                              outtextxy(getmaxwidth()/2-200,getmaxheight()/2,"FELICITATIONS JOUEUR 2");
                           }
                           else if(lang==1)
                           {
                               setcolor(RED);
                               settextstyle(9,HORIZ_DIR,1);
                              outtextxy(getmaxwidth()/2-200,getmaxheight()/2,"BON BAGAY JWE 2");
                           }
                           else if(lang==2)
                           {
                               setcolor(RED);
                               settextstyle(9,HORIZ_DIR,1);
                              outtextxy(getmaxwidth()/2-200,getmaxheight()/2,"CONGRATS PLAYER 2");
                           }
                           else if(lang==3)
                           {
                             setcolor(RED);
                            settextstyle(9,HORIZ_DIR,1);
                            outtextxy(getmaxwidth()/2-200,getmaxheight()/2,"JUGADOR 2");
                           }

                       }

                    }
                }
                u=x-xo;/// distance du lieu du click par rapport a xo
               v=y-yo;///distance du lieu du click par rapport a yo
               n=c*r;
               k=u/(r/2);/// afin de ramener a un pas proportionel a r/2 suivant x
               l=v/n;///afin de ramener a un pas proportionel a c*r
               s= k -(int) k;
               t=l-(int)l;


                if (s<0)/// sur la position du click, negatif ou pas
                {
                    s=-1*s;
                    k=-1*k;
                    i=arrondit(k,s);
                    i=-1*i;

                }
                else
                 {
                     i=arrondit(k,s);
                 }
                 j=arrondit(l,t);

                setlinestyle(0,1,3);
                 info_pion ipx;/// pour tester l'existence d'un pion dans une intersection
                 info_pion ip;
                 ipx.i=i;
                 ipx.j=j;
                 if(i%3!=0 && ((i%2==0 &&j%2!=0) || (i%2!=0 &&j%2==0) )&&(abs(i)<=20 && j<=26) && !isExist(ipx))
                 {
                    x=xo+i*r/2;
                    y=yo+j*n;
                    setlinestyle(0,1,3);
                     if(player == 1)
                        {
                         pion1(x,y);/// dessiner pion player 1 et l'enregistrer
                        setlinestyle(0,1,3);
                        setcolor(LIGHTGRAY);
                        circle(5.2*r,getmaxheight()/2,5.3*r);

                        ip.i = i;
                        ip.j = j;
                        ip.player = player;
                        inserer(ip);

                        player = 2;
                        setlinestyle(0,1,3);
                        setcolor(YELLOW);
                        circle(getmaxwidth()-5.2*r,getmaxheight()/2,5.3*r);

                     }
                     else if (player == 2)


                     {    setlinestyle(0,1,3);
                         setcolor(LIGHTGRAY);
                        circle(getmaxwidth()-5.2*r,getmaxheight()/2,5.3*r);
                        pion2(x,y);/// dessiner pion player 1 et l'enregistrer

                        ip.i = i;
                        ip.j = j;
                        ip.player = player;
                        inserer(ip);
                        player = 1;
                        setlinestyle(0,1,3);
                        setcolor(YELLOW);
                        circle(5.2*r,getmaxheight()/2,5.3*r);

                     }
                   }
                   setlinestyle(0,1,3);
                    int da = 0;
                    while(tab_pions[da].i != NULL){
                        test(tab_pions[da]);
                        da++;
                    }


                   if ((x>=50&&x<=100)&&(y>=25&&y<=70))
                 {

                    recommencer();
                    player=1;

                  }
                else if((x>=110&&x<=160)&&(y>=25&&y<=70))
                {
                  undo(ip);
                if (player==1)
                   {
                    player=2;
                    setcolor(YELLOW);
                    setlinestyle(0,1,3);
                    circle(getmaxwidth()-5.2*r,getmaxheight()/2,5.3*r);
                    setcolor(LIGHTGRAY);
                    circle(5.2*r,getmaxheight()/2,5.3*r);
                   }

                else
                 {
                     player=1;
                    setcolor(YELLOW);
                    setlinestyle(0,1,3);
                     circle(5.2*r,getmaxheight()/2,5.3*r);
                      setcolor(LIGHTGRAY);
                      circle(getmaxwidth()-5.2*r,getmaxheight()/2,5.3*r);


                 }


              }
               else if ((x>=170&&x<=220)&&(y>=25&&y<=70))
               {
                 exit(0);
               }
               else if ((x>=getmaxwidth()-90 && x<=getmaxwidth()-20)&&(y>=getmaxheight()-85 && y<=getmaxheight()-20))
               {
                 cleardevice();
                 setbkcolor(LIGHTGRAY);
                 settextstyle(9, HORIZ_DIR, 1);
                 return(1);
               }



    }
    return(0);


}



int choiximage(int x, int y)

{


    if (((x>=getmaxwidth()/2-275) && (x<=getmaxwidth()/2-25)) && ((y>=100) && (y<=350)))
        {rectangle(getmaxwidth()/2-275,100,getmaxwidth()/2-25,350);

        return 1;
        }
      else if((x>=getmaxwidth()/2+25) && (x<=getmaxwidth()/2+275) && (y>=100 && y<=350))
        {rectangle(getmaxwidth()/2+25,100,getmaxwidth()/2+275,350);

        return 2;
        }
      else if (((x>=getmaxwidth()/2+325) && (x<=getmaxwidth()/2+575))&&((y>=100)&&(y<=350)))
         {rectangle(getmaxwidth()/2+325,100,getmaxwidth()/2+575,350);
           return 3;
         }
      else if (((x>=getmaxwidth()/2-275)&&(x<=getmaxwidth()/2-25))&&((y>=400)&&(y<650)))
          {rectangle(getmaxwidth()/2-275,400,getmaxwidth()/2-25,650);
           return 4;
          }
          else return 0;
}

void test(info_pion ip)
{
    int i;
    int j;
    info_pion ip1;
    info_pion ip2;
    info_pion ip3;
    info_pion ip4;
    info_pion ip5;
    i=ip.i;
    j=ip.j;
    i=i-4;

    if(i%3==0)
        {
          ip1.i=ip.i-2;
          ip1.j = ip.j;
          ip2.i=ip.i+1;
          ip2.j = ip.j - 1;
          ip3.i = ip.i + 1;
          ip3.j = ip.j + 1;

        }
    else
        {
          ip1.i=ip.i+2;
          ip1.j = ip.j;
          ip2.i=ip.i - 1;
          ip2.j = ip.j - 1;
          ip3.i = ip.i - 1;
          ip3.j = ip.j + 1;

        }
     //printf("\npion4  %d %d\n", ip4.i, ip4.j);
     //printf("pion2 %d %d\n", ip5.i, ip5.j);
    // printf("pion3 %d %d\n", ip3.i, ip3.j);

    ip1.player = isExist(ip1);
    ip2.player = isExist(ip2);
    ip3.player = isExist(ip3);
    ip3.player = isExist(ip3);
     ip3.player = isExist(ip3);


   // printf("ip1=%d",ip1.player);
   // printf("ip2=%d",ip2.player);
    //88//////////8+9/9//9//////////

//printf("ip3=%d",ip3.player);

    if(ip1.player && ip2.player && ip3.player )
   {
     if((ip.player==1)&&(ip1.player==2)&&(ip2.player==2) &&(ip3.player==2))
         {
             retirer(ip);
            setbkcolor(8);
            cleardevice();
            board(lang,im1,im2);
             afficher_pions();
         }
         else if((ip.player==2)&&(ip1.player==1)&&(ip2.player==1) &&(ip3.player==1))
         {
            retirer(ip);
            setbkcolor(8);
            cleardevice();
           board(lang,im1,im2);
            afficher_pions();
         }
          if((ip.player==1)&&(ip1.player==1)&&(ip2.player==2) &&(ip3.player==2))
          {  if (i%3==0)
            {
                ip4.i=ip2.i-4;
                ip4.j=ip2.j;
                ip5.i=ip2.i-4;
                ip5.j=ip3.j;

            }
            else
            {
                ip4.i=ip2.i+4;
               ip4.j=ip2.j;
               ip5.i=ip2.i+4;
                ip5.j=ip3.j;
            }
              ip4.player = isExist(ip4);
              ip5.player = isExist(ip5);
             if(ip4.player && ip5.player)
             {
                 if((ip4.player==2)&&(ip5.player==2))
         {
                retirer(ip);
                retirer (ip1);
                setbkcolor(8);
                cleardevice();
                board(lang,im1,im2);
                afficher_pions();
         }
             }

          }
          else if ((ip.player==2)&&(ip1.player==2)&&(ip2.player==1) &&(ip3.player==1))
             if (i%3==0)
            {
                ip4.i=ip2.i-4;
                ip4.j=ip2.j;
                ip5.i=ip2.i-4;
                ip5.j=ip3.j;

            }
            else
            {
                ip4.i=ip2.i+4;
               ip4.j=ip2.j;
               ip5.i=ip2.i+4;
                ip5.j=ip3.j;
            }

             ip4.player = isExist(ip4);
             ip5.player = isExist(ip5);
              if(ip4.player && ip5.player)
             {
                 if((ip4.player==1)&&(ip5.player==1))
         {
                retirer(ip);
                retirer (ip1);
                setbkcolor(8);
                cleardevice();
                board(lang,im1,im2);
                afficher_pions();
         }
             }

    }

}


