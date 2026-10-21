#include <graphx.h>
#include <keypadc.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fileioc.h>

uint16_t palette[17];
int TAILLE_CARREAU = 24;

struct RGB
{
    unsigned char R,G,B;
};

struct SAVE
{
    int hightscore = 0;
    RGB ColorDamier1 = {0,0,0};//uint16_t ColorDamier1 = gfx_RGBTo1555(0, 0, 0);
    RGB ColorDamier2 = {15,15,15};//uint16_t ColorDamier2 = gfx_RGBTo1555(15, 15, 15);
    RGB ColorSnake = {25,51,0};//uint16_t ColorSnake = gfx_RGBTo1555(25, 51, 0);
    RGB ColorApple = {25,51,0};//uint16_t ColorApple = gfx_RGBTo1555(25, 51, 0);
    bool AutoRun = false;
    bool Taille10p10 = true;
    bool Taille16p16 = false;
    bool Taille24p24 = false;
    bool DifficulteS = false;
    bool DifficulteM = true;
    bool DifficulteH = false;
    bool ThemeNOKIA = true;
    bool ThemeGOOGLE = false;
    bool ThemePERSO = false;
};

struct _ALLchoice_
{
    bool AutoRun = false;
    bool Taille10p10 = true;
    bool Taille16p16 = false;
    bool Taille24p24 = false;
    bool DifficulteS = false;
    bool DifficulteM = true;
    bool DifficulteH = false;
    bool ThemeNOKIA = true;
    bool ThemeGOOGLE = false;
    bool ThemePERSO = false;
};

struct POSITION
{
    char x;
    char y;
};

struct Snake
{
    char direction;// 0 = droite, 1 = bas, 2 = gauche, 3 = haut
    POSITION body[24*24];
    int longueur;
    POSITION pomme;
};

void SAUVEGARDER(SAVE& save)
{
    uint8_t file = ti_Open("SNAKEVAR", "w");
    if(!file){return;}
    ti_Write(&save,sizeof(SAVE),1,file);
    ti_Close(file);
}

void CHARGER(SAVE& save)
{
    uint8_t file = ti_Open("SNAKEVAR", "r");
    if(!file){SAUVEGARDER(save);}
    else if(file)
    {
        SAVE temp;
        if(ti_Read(&temp, sizeof(SAVE), 1, file) == 1){save = temp;}
        ti_Close(file);
    }
    
}

void SYNC_SAVE_ALLCHOICE(bool SAVE_TO_ALLCHOICE, SAVE& save, _ALLchoice_& ALLchoice, int highscore = 0, RGB d1 = {0,0,0}, RGB d2 = {15,15,15}, RGB sn = {25,51,0}, RGB ap = {25,51,0})
{
    if(SAVE_TO_ALLCHOICE)
    {
        ALLchoice.AutoRun = save.AutoRun;
        ALLchoice.Taille10p10 = save.Taille10p10;
        ALLchoice.Taille16p16 = save.Taille16p16;
        ALLchoice.Taille24p24 = save.Taille24p24;
        ALLchoice.DifficulteS = save.DifficulteS;
        ALLchoice.DifficulteM = save.DifficulteM;
        ALLchoice.DifficulteH = save.DifficulteH;
        ALLchoice.ThemeNOKIA = save.ThemeNOKIA;
        ALLchoice.ThemeGOOGLE = save.ThemeGOOGLE;
        ALLchoice.ThemePERSO = save.ThemePERSO;
    }
    else
    {
        save.AutoRun = ALLchoice.AutoRun;
        save.Taille10p10 = ALLchoice.Taille10p10;
        save.Taille16p16 = ALLchoice.Taille16p16;
        save.Taille24p24 = ALLchoice.Taille24p24;
        save.DifficulteS = ALLchoice.DifficulteS;
        save.DifficulteM = ALLchoice.DifficulteM;
        save.DifficulteH = ALLchoice.DifficulteH;
        save.ThemeNOKIA = ALLchoice.ThemeNOKIA;
        save.ThemeGOOGLE = ALLchoice.ThemeGOOGLE;
        save.ThemePERSO = ALLchoice.ThemePERSO;
        save.ColorDamier1 = d1;
        save.ColorDamier2 = d2;
        save.ColorSnake = sn;
        save.ColorApple = ap;
        save.hightscore = highscore;
    }
}

void VERIF_CHAR(char truc[4], char trucEnPlus,char& ntruc)
{
    if(ntruc == 3){return;}
    truc[ntruc] = trucEnPlus;
    truc[ntruc+1] = '\0';
    ntruc++;
    if(ntruc == 3 && atoi(truc) > 255){truc[0] = '2';truc[1] = '5';truc[2] = '5';truc[3] = '\0';}
}

void MODIF_PALETTE_PERSO(int paletteCount, RGB color)//damier1 = 0 , damier2 = 1 , snake = 2 , apple = 3
{
    palette[paletteCount+11] = gfx_RGBTo1555(color.R, color.G, color.B);
    gfx_SetPalette(palette, sizeof(palette), 0);
}

void DRAWTEXTZONE(int x,int y,int ecart,int w,int h, const char* text,int PColor_1,int PColor_2,int PColorText, bool isSelected_Button = false, int PColorSelect = 3)
{
    gfx_SetColor(PColor_1);
    gfx_FillRectangle(x, y, w, h);
    gfx_SetColor(PColor_2);
    gfx_FillRectangle(x+ecart, y+ecart, w-2*ecart, h-2*ecart);
    if(isSelected_Button)
    {
        gfx_SetColor(PColorSelect);
        gfx_Rectangle(x-1, y-1, w+2, h+2);
    }
    gfx_SetTextFGColor(PColorText);
    gfx_PrintStringXY(text, x + (((w)-gfx_GetStringWidth(text))/2), y + (((h)-8)/2));
}

void DRAWBUTTONWT(int x,int y,int w,int h, const char* text,int PColor,int PColorText,bool isSelected_Button = false, int PColorSelect = 3)
{
    gfx_SetColor(PColor);
    gfx_FillRectangle(x, y, w, h);
    if(isSelected_Button)
    {
        gfx_SetColor(PColorSelect);
        gfx_Rectangle(x, y, w, h);
    }
    gfx_SetTextFGColor(PColorText);
    gfx_PrintStringXY(text, x + ((w-gfx_GetStringWidth(text))/2), y + ((h-8)/2));
}
void DRAWBUTTONWTWC(int x,int y,int w,int h, const char* text,int PColor,int PColorText,bool choice,bool isSelected_Button = false, int PColorSelect = 3)//choice false = off ; true = on
{
    gfx_SetColor(PColor);
    gfx_FillRectangle(x, y, w, h);
    if(isSelected_Button)
    {
        gfx_SetColor(PColorSelect);
        gfx_Rectangle(x, y, w, h);
    }
    gfx_SetTextFGColor(PColorText);
    gfx_PrintStringXY(text, x + ((w-gfx_GetStringWidth(text))/2), y + ((h-8)/2));
    if(choice == false){gfx_SetTextFGColor(15);gfx_PrintStringXY("off", x + w -26, y + ((h-8)/2));}
    if(choice == true){gfx_SetTextFGColor(16);gfx_PrintStringXY("on", x + w -20, y + ((h-8)/2));}
}

void DRAWDAMIER(int x,int y,int w,int h,int size_carreau, int PColor1,int PColor2)
{
    for(int a = 0; a < w; a += size_carreau)
    {
        for(int b = 0; b < h; b += size_carreau)
        {
            if(((a/size_carreau)+(b/size_carreau)) % 2 == 0)
            {
                gfx_SetColor(PColor1);
            }
            else
            {
                gfx_SetColor(PColor2);
            }
            gfx_FillRectangle(x + a, y + b, size_carreau, size_carreau);
        }
    }
}

void DRAWSERPENT(const Snake& serpent,int w,int h,int size_carreau,int PColor_serpent, int PColor_pomme)
{
    for(int i = 0; i < serpent.longueur; i++)
    {
        gfx_SetColor(PColor_serpent);
        gfx_FillRectangle(serpent.body[i].x*size_carreau, serpent.body[i].y*size_carreau, size_carreau, size_carreau);
    }
    gfx_SetColor(PColor_pomme); // Couleur de la pomme
    gfx_FillRectangle(serpent.pomme.x*size_carreau, serpent.pomme.y*size_carreau, size_carreau, size_carreau);
}
bool ISIN(const Snake& s, int x, int y)
{
    for(int i = 0; i < s.longueur; i++)
    {
        if(s.body[i].x == x && s.body[i].y == y)
        {
            return true;
        }
    }
    return false;
}
void REPLACEPOMME(Snake& serpent, int w, int h, int size_carreau)
{
    while(ISIN(serpent, serpent.pomme.x, serpent.pomme.y))
    {
        serpent.pomme.x = rand() % (w/size_carreau);
        serpent.pomme.y = rand() % (h/size_carreau);
    }
}
void ADDTOSERPENT(Snake& serpent, bool isAdd = true)
{
    if ((serpent.longueur < TAILLE_CARREAU*TAILLE_CARREAU) && isAdd){serpent.longueur++;}
    for (int i = serpent.longueur - 1; i > 0; i--)
    {
        serpent.body[i] = serpent.body[i - 1];
    }
    if(serpent.direction == 0){serpent.body[0].x = serpent.body[0].x + 1;}
    else if(serpent.direction == 1){serpent.body[0].y = serpent.body[0].y + 1;}
    else if(serpent.direction == 2){serpent.body[0].x = serpent.body[0].x - 1;}
    else if(serpent.direction == 3){serpent.body[0].y = serpent.body[0].y - 1;}
}
void MODIFSERPENT(Snake& serpent, int w, int h, int size_carreau, unsigned int &State, int &score, int &nb_apple)
{
    if(serpent.direction == 0)
    {
        if(serpent.body[0].x == (w/size_carreau)-1){State = 3;return;}
        if(ISIN(serpent, serpent.body[0].x+1, serpent.body[0].y)){State = 3;return;}
        else if(serpent.body[0].x == serpent.pomme.x && serpent.body[0].y == serpent.pomme.y)
        {
            ADDTOSERPENT(serpent);
            REPLACEPOMME(serpent, w, h, size_carreau);
            score += 10;
            nb_apple++;
        }
        else{ADDTOSERPENT(serpent, false);}
    }
    else if(serpent.direction == 1)
    {
        if(serpent.body[0].y == (h/size_carreau)-1){State = 3;return;}
        if(ISIN(serpent, serpent.body[0].x, serpent.body[0].y+1)){State = 3;return;}
        else if(serpent.body[0].x == serpent.pomme.x && serpent.body[0].y == serpent.pomme.y)
        {
            ADDTOSERPENT(serpent);
            REPLACEPOMME(serpent, w, h, size_carreau);
            score += 10;
            nb_apple++;
        }
        else{ADDTOSERPENT(serpent, false);}
    }
    else if(serpent.direction == 2)
    {
        if(serpent.body[0].x == 0){State = 3;return;}
        if(ISIN(serpent, serpent.body[0].x-1, serpent.body[0].y)){State = 3;return;}
        else if(serpent.body[0].x == serpent.pomme.x && serpent.body[0].y == serpent.pomme.y)
        {
            ADDTOSERPENT(serpent);
            REPLACEPOMME(serpent, w, h, size_carreau);
            score += 10;
            nb_apple++;
        }
        else{ADDTOSERPENT(serpent, false);}
    }
    else if(serpent.direction == 3)
    {
        if(serpent.body[0].y == 0){State = 3;return;}
        if(ISIN(serpent, serpent.body[0].x, serpent.body[0].y-1)){State = 3;return;}
        else if(serpent.body[0].x == serpent.pomme.x && serpent.body[0].y == serpent.pomme.y)
        {
            ADDTOSERPENT(serpent);
            REPLACEPOMME(serpent, w, h, size_carreau);
            score += 10;
            nb_apple++;
        }
        else{ADDTOSERPENT(serpent, false);}
    }
}

int main()
{
    SAVE save;
    _ALLchoice_ ALLchoice;
    CHARGER(save);
    ALLchoice.AutoRun = save.AutoRun;
    ALLchoice.Taille10p10 = save.Taille10p10;
    ALLchoice.Taille16p16 = save.Taille16p16;
    ALLchoice.Taille24p24 = save.Taille24p24;
    ALLchoice.DifficulteS = save.DifficulteS;
    ALLchoice.DifficulteM = save.DifficulteM;
    ALLchoice.DifficulteH = save.DifficulteH;
    ALLchoice.ThemeNOKIA = save.ThemeNOKIA;
    ALLchoice.ThemeGOOGLE = save.ThemeGOOGLE;
    ALLchoice.ThemePERSO = save.ThemePERSO;

    //comment lire
    palette[0] = gfx_RGBTo1555(0, 0, 0);
    palette[1] = gfx_RGBTo1555(15, 15, 15);
    palette[2] = gfx_RGBTo1555(50, 50, 50);
    palette[3] = gfx_RGBTo1555(12, 200, 12);
    palette[4] = gfx_RGBTo1555(255, 255, 255);
    palette[5] = gfx_RGBTo1555(154, 205, 50);// <-- fond jeu type nokia
    palette[6] = gfx_RGBTo1555(25, 51, 0);// <-- serpent+pomme jeu type nokia
    palette[7] = gfx_RGBTo1555(255, 0, 0);// <-- pomme jeu type google
    palette[8] = gfx_RGBTo1555(76,153,0);//palette[8] = gfx_RGBTo1555(128, 255, 0);// <-- fond1damier jeu type google
    palette[9] = gfx_RGBTo1555(102,204,0);//palette[9] = gfx_RGBTo1555(153, 255, 51);// <-- fond2damier jeu type google
    palette[10] = gfx_RGBTo1555(0, 0, 255);// <-- serpent jeu type google
    palette[11] = gfx_RGBTo1555(0, 0, 0);
    palette[12] = gfx_RGBTo1555(15, 15, 15);
    palette[13] = gfx_RGBTo1555(25, 51, 0);
    palette[14] = gfx_RGBTo1555(25, 51, 0);
    palette[15] = gfx_RGBTo1555(200, 0, 0);//off
    palette[16] = gfx_RGBTo1555(0, 0, 200);//on

    gfx_Begin();

    gfx_SetPalette(palette, 34, 0);

    gfx_SetDrawBuffer();

    unsigned int State = 0; // 0 = Menu, 1 = Parametre, 2 = Jeu, 3 = Mort, credit = 4, THEMES = 5, TOUCHES = 6, DIFFICULTE = 7, JEU-PAUSE = 8, taille = 9,personalisation couleur = 10,personalisation couleur damier1 = 11,personalisation couleur damier2 = 12,personalisation couleur snake = 13, personalisation couleur apple = 14, menu infos = 15, VERSION = 16, options avancees = 17
    int positionMenu = 0;
    int positionMenuPersonnalisation = 0;
    bool CLICK_POSSIBLE_MENU = false;
    int r_theme,g_theme,b_theme;
    int theme = 0;// 0 = theme nokia, 1 = theme google, 2 = perso
    RGB theme_perso_damier1 = save.ColorDamier1,theme_perso_damier2 = save.ColorDamier2,theme_perso_snake = save.ColorSnake,theme_perso_apple = save.ColorApple;
    char char_perso1[4]="0";char char_perso2[4]="0";char char_perso3[4]="0";
    char char_perso1n=0;char char_perso2n=0;char char_perso3n=0;
    int difficulte = 2;// 1 = facile, 2 = moyen, 4 = difficile
    int score = 0;
    int highscore = save.hightscore;
    int nb_apple = 0;
    char SCORE_TEXT[8];
    char HIGHSCORE_TEXT[8];
    char NB_APPLE_TEXT[8];
    clock_t dernier_deplacement = clock();
    clock_t maintenant = clock();
    char BUFFER_SERPENT = 0;// 0 = rien, 1 = droite, 2 = bas, 3 = gauche, 4 = haut
    Snake serpent{0, {0, 0}, 1, {5, 5}};
    MODIF_PALETTE_PERSO(0,theme_perso_damier1);MODIF_PALETTE_PERSO(1,theme_perso_damier2);MODIF_PALETTE_PERSO(2,theme_perso_snake);MODIF_PALETTE_PERSO(3,theme_perso_apple);
    if(ALLchoice.DifficulteS){difficulte = 1;}else if(ALLchoice.DifficulteM){difficulte = 2;}else if(ALLchoice.DifficulteH){difficulte = 4;}else{difficulte = 2;}
    if(ALLchoice.Taille10p10){TAILLE_CARREAU = 24;serpent = Snake{0, {{0, 0}}, 1, {5, 5}};}else if(ALLchoice.Taille16p16){TAILLE_CARREAU = 15;serpent = Snake{0, {{0, 0}}, 1, {10, 10}};}else if(ALLchoice.Taille24p24){TAILLE_CARREAU = 10;serpent = Snake{0, {{0, 0}}, 1, {12, 12}};}else{TAILLE_CARREAU = 24;serpent = Snake{0, {{0, 0}}, 1, {5, 5}};}
    if(ALLchoice.AutoRun){State = 2;dernier_deplacement = clock();}
    srand(clock());
    while(1)
    {
        kb_Scan();
        if(kb_IsDown(kb_KeyClear) || kb_IsDown(kb_KeyMode))
        {
            
            if(kb_IsDown(kb_KeyMode)){break;}
            if(CLICK_POSSIBLE_MENU)
            {
                CLICK_POSSIBLE_MENU = false;
                if(kb_IsDown(kb_KeyClear) && State == 0){break;}
                else if(kb_IsDown(kb_KeyClear) && State == 1){State = 0;positionMenu = 0;}
                else if(kb_IsDown(kb_KeyClear) && State == 2){State = 8;positionMenu = 0;}
                else if(kb_IsDown(kb_KeyClear) && State == 3){State = 0;positionMenu = 0;}
                else if(kb_IsDown(kb_KeyClear) && State == 4){State = 0;positionMenu = 0;}
                else if(kb_IsDown(kb_KeyClear) && State == 5){State = 1;positionMenu = 0;}
                else if(kb_IsDown(kb_KeyClear) && State == 6){State = 1;positionMenu = 0;}
                else if(kb_IsDown(kb_KeyClear) && State == 7){State = 1;positionMenu = 0;}
                else if(kb_IsDown(kb_KeyClear) && State == 8){State = 0;positionMenu = 0;}
                else if(kb_IsDown(kb_KeyClear) && State == 9){State = 1;positionMenu = 0;}
                else if(kb_IsDown(kb_KeyClear) && State == 15){State = 1;positionMenu = 0;}
                else if(kb_IsDown(kb_KeyClear) && State == 17){State = 1;positionMenu = 0;}
            }
            
        }
        if(kb_IsDown(kb_KeyUp) && positionMenu > 0 && CLICK_POSSIBLE_MENU){positionMenu--;CLICK_POSSIBLE_MENU=false;}//||kb_IsDown(kb_Key8)
        if((kb_IsDown(kb_KeyDown)) && positionMenu < 15 && CLICK_POSSIBLE_MENU){positionMenu++;CLICK_POSSIBLE_MENU=false;}//||kb_IsDown(kb_Key2)
        if(State == 0){if(positionMenu > 2){positionMenu = 2;};if(positionMenu < 0){positionMenu = 0;}}
        if(State == 0 && kb_IsDown(kb_KeyEnter) && CLICK_POSSIBLE_MENU)
        {
            CLICK_POSSIBLE_MENU = false;
            if(positionMenu == 0){State = 2;serpent = Snake{0, {{0, 0}}, 1, {5, 5}};score = 0;nb_apple = 0;dernier_deplacement = clock();highscore = save.hightscore;}
            else if(positionMenu == 1){State = 1;}
            else if(positionMenu == 2){State = 4;}
            positionMenu = 0;
        }
        if(State == 1){if(positionMenu > 5){positionMenu = 5;};if(positionMenu < 0){positionMenu = 0;}}
        if(State == 1 && kb_IsDown(kb_KeyEnter) && CLICK_POSSIBLE_MENU)
        {
            CLICK_POSSIBLE_MENU = false;
            if(positionMenu == 0){State = 15;}
            else if(positionMenu == 1){State = 5;}
            else if(positionMenu == 2){State = 7;}
            else if(positionMenu == 3){State = 9;}
            else if(positionMenu == 4){State = 17;}
            else if(positionMenu == 5){State = 0;}
            positionMenu = 0;
        }
        if(State == 2){if(positionMenu > 0){positionMenu = 0;};if(positionMenu < 0){positionMenu = 0;}}
        if(State == 4){if(positionMenu > 0){positionMenu = 0;};if(positionMenu < 0){positionMenu = 0;}}
        if(State == 4 && kb_IsDown(kb_KeyEnter) && CLICK_POSSIBLE_MENU)
        {
            CLICK_POSSIBLE_MENU = false;
            if(positionMenu == 0){State = 0;}
            positionMenu = 0;
        }
        if(State == 6){if(positionMenu > 0){positionMenu = 0;};if(positionMenu < 0){positionMenu = 0;}}
        if(State == 6 && kb_IsDown(kb_KeyEnter) && CLICK_POSSIBLE_MENU)
        {
            CLICK_POSSIBLE_MENU = false;
            if(positionMenu == 0){State = 15;}
            positionMenu = 0;
        }
        if(State == 7){if(positionMenu > 2){positionMenu = 2;};if(positionMenu < 0){positionMenu = 0;}}
        if(State == 7 && kb_IsDown(kb_KeyEnter) && CLICK_POSSIBLE_MENU)
        {
            CLICK_POSSIBLE_MENU = false;
            if(positionMenu == 0){difficulte = 1;State = 7;ALLchoice.DifficulteS = true;ALLchoice.DifficulteM = false;ALLchoice.DifficulteH = false;}
            else if(positionMenu == 1){difficulte = 2;State = 7;ALLchoice.DifficulteS = false;ALLchoice.DifficulteM = true;ALLchoice.DifficulteH = false;}
            else if(positionMenu == 2){difficulte = 4;State = 7;ALLchoice.DifficulteS = false;ALLchoice.DifficulteM = false;ALLchoice.DifficulteH = true;}
        }
        if(State == 5){if(positionMenu > 2){positionMenu = 2;};if(positionMenu < 0){positionMenu = 0;}}
        if(State == 5 && kb_IsDown(kb_KeyEnter) && CLICK_POSSIBLE_MENU)
        {
            CLICK_POSSIBLE_MENU = false;
            if(positionMenu == 0){theme = 0;State = 5;ALLchoice.ThemeNOKIA = true;ALLchoice.ThemeGOOGLE = false;ALLchoice.ThemePERSO = false;}
            else if(positionMenu == 1){theme = 1;State = 5;ALLchoice.ThemeNOKIA = false;ALLchoice.ThemeGOOGLE = true;ALLchoice.ThemePERSO = false;}
            else if(positionMenu == 2){State = 10;ALLchoice.ThemeNOKIA = false;ALLchoice.ThemeGOOGLE = false;ALLchoice.ThemePERSO = true;}
            if(positionMenu != 0 && positionMenu != 1){positionMenu = 0;}
        }
        if(State == 9){if(positionMenu > 2){positionMenu = 2;};if(positionMenu < 0){positionMenu = 0;}}
        if(State == 9 && kb_IsDown(kb_KeyEnter) && CLICK_POSSIBLE_MENU)
        {
            CLICK_POSSIBLE_MENU = false;
            if(positionMenu == 0){serpent = Snake{0, {{0, 0}}, 1, {5, 5}};State = 9;TAILLE_CARREAU = 24;ALLchoice.Taille10p10 = true;ALLchoice.Taille16p16 = false;ALLchoice.Taille24p24 = false;}
            else if(positionMenu == 1){serpent = Snake{0, {{0, 0}}, 1, {10, 10}};State = 9;TAILLE_CARREAU = 15;ALLchoice.Taille10p10 = false;ALLchoice.Taille16p16 = true;ALLchoice.Taille24p24 = false;}
            else if(positionMenu == 2){serpent = Snake{0, {{0, 0}}, 1, {12, 12}};State = 9;TAILLE_CARREAU = 10;ALLchoice.Taille10p10 = false;ALLchoice.Taille16p16 = false;ALLchoice.Taille24p24 = true;}
        }
        if(State == 8){if(positionMenu > 1){positionMenu = 1;};if(positionMenu < 0){positionMenu = 0;}}
        if(State == 8 && kb_IsDown(kb_KeyEnter) && CLICK_POSSIBLE_MENU)
        {       
            CLICK_POSSIBLE_MENU = false;
            if(positionMenu == 0){State = 2;}
            else if(positionMenu == 1){State = 0;positionMenu = 0;save.hightscore = highscore;SAUVEGARDER(save);}
            positionMenu = 0;
        }
        if(State == 3){if(positionMenu > 0){positionMenu = 0;};if(positionMenu < 0){positionMenu = 0;}}
        if(State == 3 && kb_IsDown(kb_KeyEnter) && CLICK_POSSIBLE_MENU)
        {
            CLICK_POSSIBLE_MENU = false;
            if(positionMenu == 0){State = 0;positionMenu = 0;save.hightscore = highscore;SAUVEGARDER(save);}
            positionMenu = 0;
        }
        if(State == 10){if(positionMenu > 4){positionMenu = 4;};if(positionMenu < 0){positionMenu = 0;}}
        if(State == 10 && kb_IsDown(kb_KeyEnter) && CLICK_POSSIBLE_MENU)
        {
            CLICK_POSSIBLE_MENU = false;
            if(positionMenu == 0){State = 11;positionMenu = 0;}//snprintf(char_perso1, sizeof(char_perso1), "%d", (unsigned char)theme_perso_damier1.R);snprintf(char_perso2, sizeof(char_perso2), "%d", (unsigned char)theme_perso_damier1.G);snprintf(char_perso3, sizeof(char_perso3), "%d", (unsigned char)theme_perso_damier1.B);}
            if(positionMenu == 1){State = 12;positionMenu = 0;}//snprintf(char_perso1, sizeof(char_perso1), "%d", (unsigned char)theme_perso_damier2.R);snprintf(char_perso2, sizeof(char_perso2), "%d", (unsigned char)theme_perso_damier2.G);snprintf(char_perso3, sizeof(char_perso3), "%d", (unsigned char)theme_perso_damier2.B);}
            if(positionMenu == 2){State = 13;positionMenu = 0;}//snprintf(char_perso1, sizeof(char_perso1), "%d", (unsigned char)theme_perso_snake.R);snprintf(char_perso2, sizeof(char_perso2), "%d", (unsigned char)theme_perso_snake.G);snprintf(char_perso3, sizeof(char_perso3), "%d", (unsigned char)theme_perso_snake.B);}
            if(positionMenu == 3){State = 14;positionMenu = 0;}//snprintf(char_perso1, sizeof(char_perso1), "%d", (unsigned char)theme_perso_apple.R);snprintf(char_perso2, sizeof(char_perso2), "%d", (unsigned char)theme_perso_apple.G);snprintf(char_perso3, sizeof(char_perso3), "%d", (unsigned char)theme_perso_apple.B);}
            if(positionMenu == 4){State = 5;positionMenu = 0;theme = 2;}
            positionMenu = 0;
        }
        if(State == 11 || State == 12 || State == 13 || State == 14){if(positionMenu > 1){positionMenu = 1;};if(positionMenu < 0){positionMenu = 0;}}
        if((State == 11 || State == 12 || State == 13 || State == 14) && kb_IsDown(kb_KeyEnter) && CLICK_POSSIBLE_MENU && positionMenu > 0)
        {
            CLICK_POSSIBLE_MENU = false;
            if(positionMenu == 1)
            {
                if(State == 11)
                {
                    theme_perso_damier1.R = (char)atoi(char_perso1);
                    theme_perso_damier1.G = (char)atoi(char_perso2);
                    theme_perso_damier1.B = (char)atoi(char_perso3);
                    MODIF_PALETTE_PERSO(0,theme_perso_damier1);
                }
                if(State == 12)
                {
                    theme_perso_damier2.R = (char)atoi(char_perso1);
                    theme_perso_damier2.G = (char)atoi(char_perso2);
                    theme_perso_damier2.B = (char)atoi(char_perso3);
                    MODIF_PALETTE_PERSO(1,theme_perso_damier2);
                }
                if(State == 13)
                {
                    theme_perso_snake.R = (char)atoi(char_perso1);
                    theme_perso_snake.G = (char)atoi(char_perso2);
                    theme_perso_snake.B = (char)atoi(char_perso3);
                    MODIF_PALETTE_PERSO(2,theme_perso_snake);
                }
                if(State == 14)
                {
                    theme_perso_apple.R = (char)atoi(char_perso1);
                    theme_perso_apple.G = (char)atoi(char_perso2);
                    theme_perso_apple.B = (char)atoi(char_perso3);
                    MODIF_PALETTE_PERSO(3,theme_perso_apple);
                }
            }
            char_perso1[0]='\0';
            char_perso2[0]='\0';
            char_perso3[0]='\0';
            char_perso1n=0;
            char_perso2n=0;
            char_perso3n=0;
            State = 10;
            positionMenu = 0;
        }
        if(State == 15){if(positionMenu > 1){positionMenu = 1;};if(positionMenu < 0){positionMenu = 0;}}
        if(State == 15 && kb_IsDown(kb_KeyEnter) && CLICK_POSSIBLE_MENU)
        {
            CLICK_POSSIBLE_MENU = false;
            if(positionMenu == 0){State = 6;positionMenu = 0;}
            if(positionMenu == 1){State = 16;positionMenu = 0;}
            positionMenu = 0;
        }
        if(State == 16){if(positionMenu > 0){positionMenu = 0;};if(positionMenu < 0){positionMenu = 0;}}
        if(State == 16 && kb_IsDown(kb_KeyEnter) && CLICK_POSSIBLE_MENU)
        {
            CLICK_POSSIBLE_MENU = false;
            if(positionMenu == 0){State = 15;}
            positionMenu = 0;
        }
        if(State == 17){if(positionMenu > 1){positionMenu = 1;};if(positionMenu < 0){positionMenu = 0;}}
        if(State == 17 && kb_IsDown(kb_KeyEnter) && CLICK_POSSIBLE_MENU)
        {
            CLICK_POSSIBLE_MENU = false;
            if(positionMenu == 0){State = 17;ALLchoice.AutoRun = !ALLchoice.AutoRun;}
            if(positionMenu == 1)
            {
                State = 17;
                SAVE temp;SAUVEGARDER(temp);CHARGER(save);
                SYNC_SAVE_ALLCHOICE(true,save,ALLchoice);
                if(ALLchoice.DifficulteS){difficulte = 1;}else if(ALLchoice.DifficulteM){difficulte = 2;}else if(ALLchoice.DifficulteH){difficulte = 4;}else{difficulte = 2;}
                if(ALLchoice.Taille10p10){TAILLE_CARREAU = 24;serpent = Snake{0, {{0, 0}}, 1, {5, 5}};}else if(ALLchoice.Taille16p16){TAILLE_CARREAU = 15;serpent = Snake{0, {{0, 0}}, 1, {10, 10}};}else if(ALLchoice.Taille24p24){TAILLE_CARREAU = 10;serpent = Snake{0, {{0, 0}}, 1, {12, 12}};}else{TAILLE_CARREAU = 24;serpent = Snake{0, {{0, 0}}, 1, {5, 5}};}
            }
        }

        gfx_FillScreen(0);

        if(State == 0)
        {
            gfx_SetTextScale(5, 5);
            gfx_SetTextFGColor(3);
            gfx_PrintStringXY("SNAKE", ((320-gfx_GetStringWidth("SNAKE"))/2), 50);//gfx_PrintStringXY("SNAKE", 62, 50);
            gfx_SetTextScale(1,1);
            if(positionMenu == 0){DRAWBUTTONWT(64, 125, 192, 25, "Jouer", 2, 4, true);}else{DRAWBUTTONWT(64, 125, 192, 25, "Jouer", 2, 4, false);}
            if(positionMenu == 1){DRAWBUTTONWT(64, 160, 192, 25, "Parametre", 2, 4, true);}else{DRAWBUTTONWT(64, 160, 192, 25, "Parametre", 2, 4, false);}
            if(positionMenu == 2){DRAWBUTTONWT(64, 195, 192, 25, "Credit", 2, 4, true);}else{DRAWBUTTONWT(64, 195, 192, 25, "Credit", 2, 4, false);}

            //gfx_SetColor(gfx_RGBTo1555(50, 50, 50));
            //gfx_SetColor(2);
            //gfx_FillRectangle(64, 125, 192, 25);
            //gfx_FillRectangle(64, 160, 192, 25);
            //gfx_FillRectangle(64, 195, 192, 25);

            //gfx_SetColor(gfx_RGBTo1555(12, 200, 12));
            //gfx_SetColor(3);
            //gfx_Rectangle(64, 125 + (positionMenu * 35), 192, 25);
            
        }
        else if(State == 1)
        {
            if(positionMenu == 0){DRAWBUTTONWT(64, 25, 192, 25, "INFOS", 2, 4, true);}else{DRAWBUTTONWT(64, 25, 192, 25, "INFOS", 2, 4, false);}
            if(positionMenu == 1){DRAWBUTTONWT(64, 60, 192, 25, "THEMES", 2, 4, true);}else{DRAWBUTTONWT(64, 60, 192, 25, "THEMES", 2, 4, false);}
            if(positionMenu == 2){DRAWBUTTONWT(64, 95, 192, 25, "DIFFICULTE", 2, 4, true);}else{DRAWBUTTONWT(64, 95, 192, 25, "DIFFICULTE", 2, 4, false);}
            if(positionMenu == 3){DRAWBUTTONWT(64, 130, 192, 25, "TAILLE", 2, 4, true);}else{DRAWBUTTONWT(64, 130, 192, 25, "TAILLE", 2, 4, false);}
            if(positionMenu == 4){DRAWBUTTONWT(64, 165, 192, 25, "OPTIONS AVANCEES", 2, 4, true);}else{DRAWBUTTONWT(64, 165, 192, 25, "OPTIONS AVANCEES", 2, 4, false);}
            if(positionMenu == 5){DRAWBUTTONWT(64, 200, 192, 25, "RETOUR", 2, 4, true);}else{DRAWBUTTONWT(64, 200, 192, 25, "RETOUR", 2, 4, false);}
        }
        else if(State == 2)
        {
            if(kb_IsDown(kb_KeyRight) && serpent.direction != 2){serpent.direction = 0;}
            if(kb_IsDown(kb_KeyDown) && serpent.direction != 3){serpent.direction = 1;}
            if(kb_IsDown(kb_KeyLeft) && serpent.direction != 0){serpent.direction = 2;}
            if(kb_IsDown(kb_KeyUp) && serpent.direction != 1){serpent.direction = 3;}
            if(kb_IsDown(kb_Key6) && serpent.direction != 2){serpent.direction = 0;}
            if(kb_IsDown(kb_Key2) && serpent.direction != 3){serpent.direction = 1;}
            if(kb_IsDown(kb_Key4) && serpent.direction != 0){serpent.direction = 2;}
            if(kb_IsDown(kb_Key8) && serpent.direction != 1){serpent.direction = 3;}
            gfx_SetTextFGColor(4);

            if(score > highscore){highscore = score;}

            snprintf(SCORE_TEXT, sizeof(SCORE_TEXT), "%d", score);
            snprintf(HIGHSCORE_TEXT, sizeof(HIGHSCORE_TEXT), "%d", highscore);
            snprintf(NB_APPLE_TEXT, sizeof(NB_APPLE_TEXT), "%d", nb_apple);

            gfx_PrintStringXY("Score", 250, 10);//gfx_PrintStringXY("SNAKE", 62, 50);
            gfx_PrintStringXY(SCORE_TEXT, 250, 20);//gfx_PrintStringXY("SNAKE", 62, 50);

            gfx_PrintStringXY("PB", 250, 40);//gfx_PrintStringXY("SNAKE", 62, 50);
            gfx_PrintStringXY(HIGHSCORE_TEXT, 250, 50);//gfx_PrintStringXY("SNAKE", 62, 50);

            gfx_PrintStringXY("Pommes", 250, 70);//gfx_PrintStringXY("SNAKE", 62, 50);
            gfx_PrintStringXY(NB_APPLE_TEXT, 250, 80);//gfx_PrintStringXY("SNAKE", 62, 50);

            if(ALLchoice.ThemeNOKIA)//theme == 0)
            {
                DRAWDAMIER(0, 0, 240, 240, TAILLE_CARREAU, 5, 5);
                DRAWSERPENT(serpent,240,240,TAILLE_CARREAU, 6, 6);
            }
            else if(ALLchoice.ThemeGOOGLE)//theme == 1)
            {
                DRAWDAMIER(0, 0, 240, 240, TAILLE_CARREAU, 8, 9);
                DRAWSERPENT(serpent,240,240,TAILLE_CARREAU, 10, 7);
            }
            else if(ALLchoice.ThemePERSO)//theme == 2)
            {
                DRAWDAMIER(0, 0, 240, 240, TAILLE_CARREAU, 11, 12);
                DRAWSERPENT(serpent,240,240,TAILLE_CARREAU, 13, 14);
            }

            clock_t maintenant = clock();
            if((maintenant - dernier_deplacement) >= (10000 / difficulte))
            {
                dernier_deplacement = maintenant;
                if(ALLchoice.ThemeNOKIA)//theme == 0)
                {
                    DRAWDAMIER(0, 0, 240, 240, TAILLE_CARREAU, 5, 5);
                    MODIFSERPENT(serpent, 240, 240, TAILLE_CARREAU, State, score, nb_apple);
                    DRAWSERPENT(serpent,240,240,TAILLE_CARREAU, 6, 6);
                }
                else if(ALLchoice.ThemeGOOGLE)//theme == 1)
                {
                    DRAWDAMIER(0, 0, 240, 240, TAILLE_CARREAU, 8, 9);
                    MODIFSERPENT(serpent, 240, 240, TAILLE_CARREAU, State, score, nb_apple);
                    DRAWSERPENT(serpent,240,240,TAILLE_CARREAU, 10, 7);
                }
                else if(ALLchoice.ThemePERSO)//theme == 2)
                {
                    DRAWDAMIER(0, 0, 240, 240, TAILLE_CARREAU, 11, 12);
                    MODIFSERPENT(serpent, 240, 240, TAILLE_CARREAU, State, score, nb_apple);
                    DRAWSERPENT(serpent,240,240,TAILLE_CARREAU, 13, 14);
                }
            }
            
        }
        else if(State == 3)
        {
            gfx_SetTextScale(4, 4);
            gfx_SetTextFGColor(7);
            gfx_PrintStringXY("GAME OVER", ((320-gfx_GetStringWidth("GAME OVER"))/2), 50);//gfx_PrintStringXY("SNAKE", 62, 50);
            gfx_SetTextScale(1,1);
            if(positionMenu == 0){DRAWBUTTONWT(64, 205, 192, 25, "MENU", 2, 4, true, 7);}else{DRAWBUTTONWT(64, 205, 192, 25, "MENU", 2, 4, false);}
        }
        else if(State == 4)
        {
            gfx_SetTextFGColor(3);
            gfx_PrintStringXY("Createur : YoCodeShi", ((320-gfx_GetStringWidth("Createur : YoCodeShi"))/2), 10);//gfx_PrintStringXY("SNAKE", 62, 50);
            gfx_PrintStringXY("Github : https://github.com/veltix-code", ((320-gfx_GetStringWidth("Github : https://github.com/veltix-code"))/2), 20);//gfx_PrintStringXY("SNAKE", 62, 50);
            gfx_PrintStringXY("Je vous remercie d'avoir joue a mon jeu!", ((320-gfx_GetStringWidth("Je vous remercie d'avoir joue a mon jeu!"))/2), 30);//gfx_PrintStringXY("SNAKE", 62, 50);
            if(positionMenu == 0){DRAWBUTTONWT(64, 205, 192, 25, "RETOUR", 2, 4, true);}else{DRAWBUTTONWT(64, 205, 192, 25, "RETOUR", 2, 4, false);}

        }
        else if(State == 5)
        {
            if(positionMenu == 0){DRAWBUTTONWTWC(64, 25, 192, 25, "NOKIA", 2, 4,ALLchoice.ThemeNOKIA, true);}else{DRAWBUTTONWTWC(64, 25, 192, 25, "NOKIA", 2, 4,ALLchoice.ThemeNOKIA, false);}
            if(positionMenu == 1){DRAWBUTTONWTWC(64, 60, 192, 25, "GOOGLE", 2, 4,ALLchoice.ThemeGOOGLE, true);}else{DRAWBUTTONWTWC(64, 60, 192, 25, "GOOGLE", 2, 4,ALLchoice.ThemeGOOGLE, false);}
            if(positionMenu == 2){DRAWBUTTONWTWC(64, 95, 192, 25, "PERSONNALISE", 2, 4,ALLchoice.ThemePERSO, true);}else{DRAWBUTTONWTWC(64, 95, 192, 25, "PERSONNALISE", 2, 4,ALLchoice.ThemePERSO, false);}
            /*            if(positionMenu == 0){DRAWBUTTONWT(64, 25, 192, 25, "NOKIA", 2, 4, true);}else{DRAWBUTTONWT(64, 25, 192, 25, "NOKIA", 2, 4, false);}
            if(positionMenu == 1){DRAWBUTTONWT(64, 60, 192, 25, "GOOGLE", 2, 4, true);}else{DRAWBUTTONWT(64, 60, 192, 25, "GOOGLE", 2, 4, false);}
            if(positionMenu == 2){DRAWBUTTONWT(64, 95, 192, 25, "PERSONNALISE", 2, 4, true);}else{DRAWBUTTONWT(64, 95, 192, 25, "PERSONNALISE", 2, 4, false);}
*/
        }
        else if(State == 6)
        {
            gfx_SetTextFGColor(3);
            gfx_PrintStringXY("Les touches sont :", 10, 10);//gfx_PrintStringXY("SNAKE", 62, 50);
            gfx_PrintStringXY("JEU : HAUT, BAS, GAUCHE, DROITE, 8, 2, 4, 6", 10, 20);
            gfx_PrintStringXY("MENU : HAUT, BAS", 10, 30);
            gfx_PrintStringXY("GLOBAL :", 10, 50);
            gfx_PrintStringXY("MODE = QUITTER", 10, 60);
            gfx_PrintStringXY("ANNUL = RETOUR", 10, 70);
            gfx_PrintStringXY("ENTER = VALIDER", 10, 80);
            if(positionMenu == 0){DRAWBUTTONWT(64, 205, 192, 25, "RETOUR", 2, 4, true);}else{DRAWBUTTONWT(64, 205, 192, 25, "RETOUR", 2, 4, false);}
        }
        else if(State == 7)
        {
            if(positionMenu == 0){DRAWBUTTONWTWC(64, 25, 192, 25, "EASY", 2, 4,ALLchoice.DifficulteS, true);}else{DRAWBUTTONWTWC(64, 25, 192, 25, "EASY", 2, 4,ALLchoice.DifficulteS, false);}
            if(positionMenu == 1){DRAWBUTTONWTWC(64, 60, 192, 25, "NORMAL", 2, 4,ALLchoice.DifficulteM, true);}else{DRAWBUTTONWTWC(64, 60, 192, 25, "NORMAL", 2, 4,ALLchoice.DifficulteM, false);}
            if(positionMenu == 2){DRAWBUTTONWTWC(64, 95, 192, 25, "HARD", 2, 4,ALLchoice.DifficulteH, true);}else{DRAWBUTTONWTWC(64, 95, 192, 25, "HARD", 2, 4,ALLchoice.DifficulteH, false);}
        }
        else if(State == 8)
        {
            if(positionMenu == 0){DRAWBUTTONWT(64, 25, 192, 25, "CONTINUER", 2, 4, true);}else{DRAWBUTTONWT(64, 25, 192, 25, "CONTINUER", 2, 4, false);}
            if(positionMenu == 1){DRAWBUTTONWT(64, 60, 192, 25, "MENU", 2, 4, true);}else{DRAWBUTTONWT(64, 60, 192, 25, "MENU", 2, 4, false);}
        }
        else if(State == 9)
        {
            if(positionMenu == 0){DRAWBUTTONWTWC(64, 25, 192, 25, "10x10", 2, 4,ALLchoice.Taille10p10, true);}else{DRAWBUTTONWTWC(64, 25, 192, 25, "10x10", 2, 4,ALLchoice.Taille10p10, false);}
            if(positionMenu == 1){DRAWBUTTONWTWC(64, 60, 192, 25, "16x16", 2, 4,ALLchoice.Taille16p16, true);}else{DRAWBUTTONWTWC(64, 60, 192, 25, "16x16", 2, 4,ALLchoice.Taille16p16, false);}
            if(positionMenu == 2){DRAWBUTTONWTWC(64, 95, 192, 25, "24x24", 2, 4,ALLchoice.Taille24p24, true);}else{DRAWBUTTONWTWC(64, 95, 192, 25, "24x24", 2, 4,ALLchoice.Taille24p24, false);}
        }
        else if(State == 10)
        {
            if(positionMenu == 0){DRAWBUTTONWT(64, 25, 192, 25, "DAMIER 1", 2, 4, true);}else{DRAWBUTTONWT(64, 25, 192, 25, "DAMIER 1", 2, 4, false);}
            if(positionMenu == 1){DRAWBUTTONWT(64, 60, 192, 25, "DAMIER 2", 2, 4, true);}else{DRAWBUTTONWT(64, 60, 192, 25, "DAMIER 2", 2, 4, false);}
            if(positionMenu == 2){DRAWBUTTONWT(64, 95, 192, 25, "SNAKE", 2, 4, true);}else{DRAWBUTTONWT(64, 95, 192, 25, "SNAKE", 2, 4, false);}
            if(positionMenu == 3){DRAWBUTTONWT(64, 130, 192, 25, "APPLE", 2, 4, true);}else{DRAWBUTTONWT(64, 130, 192, 25, "APPLE", 2, 4, false);}
            if(positionMenu == 4){DRAWBUTTONWT(64, 165, 192, 25, "RETOUR", 2, 4, true);}else{DRAWBUTTONWT(64, 165, 192, 25, "RETOUR", 2, 4, false);}
        }
        else if(State == 11 || State == 12 || State == 13 || State == 14)
        {
            if(positionMenuPersonnalisation < 0){positionMenuPersonnalisation = 0;}if(positionMenuPersonnalisation > 2){positionMenuPersonnalisation = 2;}
            if(kb_IsDown(kb_KeyRight) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation > 2){}else{positionMenuPersonnalisation++;}CLICK_POSSIBLE_MENU = false;}
            if(kb_IsDown(kb_KeyLeft) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation < 1){}else{positionMenuPersonnalisation--;}CLICK_POSSIBLE_MENU = false;}
            if(kb_IsDown(kb_KeyDel) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation == 0){char_perso1[0] = '\0';char_perso1n = 0;}else if(positionMenuPersonnalisation == 1){char_perso2[0] = '\0';char_perso2n = 0;}else if(positionMenuPersonnalisation == 2){char_perso3[0] = '\0';char_perso3n = 0;}CLICK_POSSIBLE_MENU = false;}
            if(kb_IsDown(kb_Key0) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation == 0){VERIF_CHAR(char_perso1,'0',char_perso1n);}else if(positionMenuPersonnalisation == 1){VERIF_CHAR(char_perso2,'0',char_perso2n);}else if(positionMenuPersonnalisation == 2){VERIF_CHAR(char_perso3,'0',char_perso3n);}CLICK_POSSIBLE_MENU = false;}
            if(kb_IsDown(kb_Key1) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation == 0){VERIF_CHAR(char_perso1,'1',char_perso1n);}else if(positionMenuPersonnalisation == 1){VERIF_CHAR(char_perso2,'1',char_perso2n);}else if(positionMenuPersonnalisation == 2){VERIF_CHAR(char_perso3,'1',char_perso3n);}CLICK_POSSIBLE_MENU = false;}
            if(kb_IsDown(kb_Key2) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation == 0){VERIF_CHAR(char_perso1,'2',char_perso1n);}else if(positionMenuPersonnalisation == 1){VERIF_CHAR(char_perso2,'2',char_perso2n);}else if(positionMenuPersonnalisation == 2){VERIF_CHAR(char_perso3,'2',char_perso3n);}CLICK_POSSIBLE_MENU = false;}
            if(kb_IsDown(kb_Key3) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation == 0){VERIF_CHAR(char_perso1,'3',char_perso1n);}else if(positionMenuPersonnalisation == 1){VERIF_CHAR(char_perso2,'3',char_perso2n);}else if(positionMenuPersonnalisation == 2){VERIF_CHAR(char_perso3,'3',char_perso3n);}CLICK_POSSIBLE_MENU = false;}
            if(kb_IsDown(kb_Key4) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation == 0){VERIF_CHAR(char_perso1,'4',char_perso1n);}else if(positionMenuPersonnalisation == 1){VERIF_CHAR(char_perso2,'4',char_perso2n);}else if(positionMenuPersonnalisation == 2){VERIF_CHAR(char_perso3,'4',char_perso3n);}CLICK_POSSIBLE_MENU = false;}
            if(kb_IsDown(kb_Key5) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation == 0){VERIF_CHAR(char_perso1,'5',char_perso1n);}else if(positionMenuPersonnalisation == 1){VERIF_CHAR(char_perso2,'5',char_perso2n);}else if(positionMenuPersonnalisation == 2){VERIF_CHAR(char_perso3,'5',char_perso3n);}CLICK_POSSIBLE_MENU = false;}
            if(kb_IsDown(kb_Key6) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation == 0){VERIF_CHAR(char_perso1,'6',char_perso1n);}else if(positionMenuPersonnalisation == 1){VERIF_CHAR(char_perso2,'6',char_perso2n);}else if(positionMenuPersonnalisation == 2){VERIF_CHAR(char_perso3,'6',char_perso3n);}CLICK_POSSIBLE_MENU = false;}
            if(kb_IsDown(kb_Key7) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation == 0){VERIF_CHAR(char_perso1,'7',char_perso1n);}else if(positionMenuPersonnalisation == 1){VERIF_CHAR(char_perso2,'7',char_perso2n);}else if(positionMenuPersonnalisation == 2){VERIF_CHAR(char_perso3,'7',char_perso3n);}CLICK_POSSIBLE_MENU = false;}
            if(kb_IsDown(kb_Key8) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation == 0){VERIF_CHAR(char_perso1,'8',char_perso1n);}else if(positionMenuPersonnalisation == 1){VERIF_CHAR(char_perso2,'8',char_perso2n);}else if(positionMenuPersonnalisation == 2){VERIF_CHAR(char_perso3,'8',char_perso3n);}CLICK_POSSIBLE_MENU = false;}
            if(kb_IsDown(kb_Key9) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation == 0){VERIF_CHAR(char_perso1,'9',char_perso1n);}else if(positionMenuPersonnalisation == 1){VERIF_CHAR(char_perso2,'9',char_perso2n);}else if(positionMenuPersonnalisation == 2){VERIF_CHAR(char_perso3,'9',char_perso3n);}CLICK_POSSIBLE_MENU = false;}

            if(positionMenuPersonnalisation == 0 && positionMenu == 0){DRAWTEXTZONE(32, 100,5, 64, 25, char_perso1, 2, 4,0, true,3);}else{DRAWTEXTZONE(32, 100,5, 64, 25, char_perso1, 2, 4,0, false);}
            if(positionMenuPersonnalisation == 1 && positionMenu == 0){DRAWTEXTZONE(128, 100,5, 64, 25, char_perso2, 2, 4,0, true,3);}else{DRAWTEXTZONE(128, 100,5, 64, 25, char_perso2, 2, 4,0, false);}
            if(positionMenuPersonnalisation == 2 && positionMenu == 0){DRAWTEXTZONE(224, 100,5, 64, 25, char_perso3, 2, 4,0, true,3);}else{DRAWTEXTZONE(224, 100,5, 64, 25, char_perso3, 2, 4,0, false);}
            if(positionMenu == 1){DRAWBUTTONWT(64, 205, 192, 25, "ENREGISTRER", 2, 4, true);}else{DRAWBUTTONWT(64, 205, 192, 25, "ENREGISTRER", 2, 4, false);}
        }
        else if(State == 15)
        {
            if(positionMenu == 0){DRAWBUTTONWT(64, 25, 192, 25, "TOUCHES", 2, 4, true);}else{DRAWBUTTONWT(64, 25, 192, 25, "TOUCHES", 2, 4, false);}
            if(positionMenu == 1){DRAWBUTTONWT(64, 60, 192, 25, "VERSION", 2, 4, true);}else{DRAWBUTTONWT(64, 60, 192, 25, "VERSION", 2, 4, false);}
        }
        else if(State == 16)
        {
            gfx_SetTextFGColor(3);
            gfx_PrintStringXY("VERSION = 0.3", 10, 10);
            if(positionMenu == 0){DRAWBUTTONWT(64, 205, 192, 25, "RETOUR", 2, 4, true);}else{DRAWBUTTONWT(64, 205, 192, 25, "RETOUR", 2, 4, false);}
        }
        else if(State == 17)
        {
            if(positionMenu == 0){DRAWBUTTONWTWC(64, 25, 192, 25, "AUTORUN", 2, 4,ALLchoice.AutoRun, true);}else{DRAWBUTTONWTWC(64, 25, 192, 25, "AUTORUN", 2, 4,ALLchoice.AutoRun, false);}
            if(positionMenu == 1){DRAWBUTTONWT(64, 60, 192, 25, "REINITIALISER SAUVEGARDE", 2, 4, true);}else{DRAWBUTTONWT(64, 60, 192, 25, "REINITIALISER SAUVEGARDE", 2, 4, false);}
        }
        else
        {
            gfx_SetTextFGColor(3);
            gfx_PrintStringXY("ERREUR", ((320-gfx_GetStringWidth("ERREUR"))/2), 50);//gfx_PrintStringXY("SNAKE", 62, 50);
        }
        if (!kb_IsDown(kb_KeyUp) && !kb_IsDown(kb_KeyDown) && !kb_IsDown(kb_KeyLeft) && !kb_IsDown(kb_KeyRight) && !kb_IsDown(kb_KeyEnter) && !kb_IsDown(kb_KeyClear) && !kb_IsDown(kb_Key0) && !kb_IsDown(kb_Key1) && !kb_IsDown(kb_Key2) && !kb_IsDown(kb_Key3) && !kb_IsDown(kb_Key4) && !kb_IsDown(kb_Key5) && !kb_IsDown(kb_Key6) && !kb_IsDown(kb_Key7) && !kb_IsDown(kb_Key8) && !kb_IsDown(kb_Key9) && !kb_IsDown(kb_KeyDel))
        {
            CLICK_POSSIBLE_MENU = true;
        }
        gfx_SwapDraw();
        msleep(30);
    }
    save.AutoRun = ALLchoice.AutoRun;save.Taille10p10 = ALLchoice.Taille10p10;save.Taille16p16 = ALLchoice.Taille16p16;save.Taille24p24 = ALLchoice.Taille24p24;save.DifficulteS = ALLchoice.DifficulteS;save.DifficulteM = ALLchoice.DifficulteM;save.DifficulteH = ALLchoice.DifficulteH;save.ThemeNOKIA = ALLchoice.ThemeNOKIA;save.ThemeGOOGLE = ALLchoice.ThemeGOOGLE;save.ThemePERSO = ALLchoice.ThemePERSO;save.ColorDamier1 = theme_perso_damier1;save.ColorDamier2 = theme_perso_damier2;save.ColorSnake = theme_perso_snake;save.ColorApple = theme_perso_apple;save.hightscore = highscore;SAUVEGARDER(save);
    gfx_End();
    return 0;
}
/*

snprintf(chaine, sizeof(chaine), "%d", (unsigned char)theme_perso_damier1.R);
char nombre = (char)atoi(chaine);

estetique a garder pour plus tard

bouton :
largeur 192
hauteur 25
espacement 10

couleur fond 

    palette[0] = gfx_RGBTo1555(0, 0, 0); <-- noir
    palette[1] = gfx_RGBTo1555(15, 15, 15); <-- gris fonce
    palette[2] = gfx_RGBTo1555(50, 50, 50); <-- gris claire bouton
    palette[3] = gfx_RGBTo1555(12, 200, 12); <-- joli vert
    palette[4] = gfx_RGBTo1555(255, 255, 255); <-- blanc



if(kb_IsDown(kb_Key0) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation == 0){ADD_NUMBER_TO_CHAR4(char_perso1,'0');}else if(positionMenuPersonnalisation == 1){ADD_NUMBER_TO_CHAR4(char_perso2,'0');}else if(positionMenuPersonnalisation == 2){ADD_NUMBER_TO_CHAR4(char_perso1,'0');}CLICK_POSSIBLE_MENU = false;}
if(kb_IsDown(kb_Key1) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation == 0){ADD_NUMBER_TO_CHAR4(char_perso1,'1');}else if(positionMenuPersonnalisation == 1){ADD_NUMBER_TO_CHAR4(char_perso2,'1');}else if(positionMenuPersonnalisation == 2){ADD_NUMBER_TO_CHAR4(char_perso1,'1');}CLICK_POSSIBLE_MENU = false;}
if(kb_IsDown(kb_Key2) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation == 0){ADD_NUMBER_TO_CHAR4(char_perso1,'2');}else if(positionMenuPersonnalisation == 1){ADD_NUMBER_TO_CHAR4(char_perso2,'2');}else if(positionMenuPersonnalisation == 2){ADD_NUMBER_TO_CHAR4(char_perso1,'2');}CLICK_POSSIBLE_MENU = false;}
if(kb_IsDown(kb_Key3) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation == 0){ADD_NUMBER_TO_CHAR4(char_perso1,'3');}else if(positionMenuPersonnalisation == 1){ADD_NUMBER_TO_CHAR4(char_perso2,'3');}else if(positionMenuPersonnalisation == 2){ADD_NUMBER_TO_CHAR4(char_perso1,'3');}CLICK_POSSIBLE_MENU = false;}
if(kb_IsDown(kb_Key4) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation == 0){ADD_NUMBER_TO_CHAR4(char_perso1,'4');}else if(positionMenuPersonnalisation == 1){ADD_NUMBER_TO_CHAR4(char_perso2,'4');}else if(positionMenuPersonnalisation == 2){ADD_NUMBER_TO_CHAR4(char_perso1,'4');}CLICK_POSSIBLE_MENU = false;}
if(kb_IsDown(kb_Key5) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation == 0){ADD_NUMBER_TO_CHAR4(char_perso1,'5');}else if(positionMenuPersonnalisation == 1){ADD_NUMBER_TO_CHAR4(char_perso2,'5');}else if(positionMenuPersonnalisation == 2){ADD_NUMBER_TO_CHAR4(char_perso1,'5');}CLICK_POSSIBLE_MENU = false;}
if(kb_IsDown(kb_Key6) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation == 0){ADD_NUMBER_TO_CHAR4(char_perso1,'6');}else if(positionMenuPersonnalisation == 1){ADD_NUMBER_TO_CHAR4(char_perso2,'6');}else if(positionMenuPersonnalisation == 2){ADD_NUMBER_TO_CHAR4(char_perso1,'6');}CLICK_POSSIBLE_MENU = false;}
if(kb_IsDown(kb_Key7) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation == 0){ADD_NUMBER_TO_CHAR4(char_perso1,'7');}else if(positionMenuPersonnalisation == 1){ADD_NUMBER_TO_CHAR4(char_perso2,'7');}else if(positionMenuPersonnalisation == 2){ADD_NUMBER_TO_CHAR4(char_perso1,'7');}CLICK_POSSIBLE_MENU = false;}
if(kb_IsDown(kb_Key8) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation == 0){ADD_NUMBER_TO_CHAR4(char_perso1,'8');}else if(positionMenuPersonnalisation == 1){ADD_NUMBER_TO_CHAR4(char_perso2,'8');}else if(positionMenuPersonnalisation == 2){ADD_NUMBER_TO_CHAR4(char_perso1,'8');}CLICK_POSSIBLE_MENU = false;}
if(kb_IsDown(kb_Key9) && CLICK_POSSIBLE_MENU && positionMenu == 0){if(positionMenuPersonnalisation == 0){ADD_NUMBER_TO_CHAR4(char_perso1,'9');}else if(positionMenuPersonnalisation == 1){ADD_NUMBER_TO_CHAR4(char_perso2,'9');}else if(positionMenuPersonnalisation == 2){ADD_NUMBER_TO_CHAR4(char_perso1,'9');}CLICK_POSSIBLE_MENU = false;}
     
void ADD_NUMBER_TO_CHAR4(char truc[4], char trucEnPlus)
{
    truc[0] = truc[1];truc[1] = truc[2];truc[2] = trucEnPlus;
}



*/