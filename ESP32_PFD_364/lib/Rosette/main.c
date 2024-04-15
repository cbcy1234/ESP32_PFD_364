#include <graphics.h>
#include <stdlib.h>
#include <stdio.h>
#include"design.h"
#include"fonctions.h"

int lang =0;
int im1, im2;

int main(int argc, char *argv[])

{   int retour_menu;

    initwindow(getmaxwidth(),getmaxheight(), "ROSETTE:Le Monde des Abeilles", 0,0);
    lang= accueil();
    //readimagefile("images/49320_capture.jpg",0,0,getmaxwidth(),getmaxheight());
    readimagefile("images/49321_capture.jpg",0,0,getmaxwidth(),getmaxheight());
    load();

    menu_pos :
    retour_menu = 0;
    menu(lang, &im1, &im2);
      cleardevice();
      setbkcolor(LIGHTGRAY);
      setcolor(YELLOW);

      rectangle(300,0,getmaxwidth()-300,getmaxheight());
      readimagefile("images/exemple.jpg",300,0,getmaxwidth()-300,getmaxheight());
      rectangle(300,0,getmaxwidth()-300,getmaxheight());
      readimagefile("images/images-9.jpeg",0,0,300,getmaxheight());
      readimagefile("images/images-14.jpeg",getmaxwidth()-300,0,getmaxwidth(),getmaxheight());
      load();
      setbkcolor(8);
      cleardevice();

      board(lang,im1,im2);

       retour_menu = jouer(lang);
       if (retour_menu == 1) goto menu_pos;
       getch();
       return 0;
}

