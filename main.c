/* Backrooms Tale - PSP homebrew starter prototype
 * Build with pspsdk: make
 * Controls: D-pad move/turn, X select/continue, START return to menu, TRIANGLE skip intro.
 */
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

PSP_MODULE_INFO("BackroomsTale", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
#define W 480
#define H 272
#define PI 3.14159265f
static unsigned int *fb;
static int scene=0, menu=0, introTick=0;
static float px=2.5f, py=2.5f, pa=0.0f;
static unsigned int rgb(int r,int g,int b){return 0xff000000u|((b&255)<<16)|((g&255)<<8)|(r&255);}
static void put(int x,int y,unsigned int c){if(x>=0&&x<W&&y>=0&&y<H)fb[y*W+x]=c;}
static void rect(int x,int y,int w,int h,unsigned int c){int xx,yy;for(yy=y;yy<y+h;yy++)for(xx=x;xx<x+w;xx++)put(xx,yy,c);}
static void line(int x0,int y0,int x1,int y1,unsigned int c){int dx=abs(x1-x0),sx=x0<x1?1:-1,dy=-abs(y1-y0),sy=y0<y1?1:-1,err=dx+dy,e2;for(;;){put(x0,y0,c);if(x0==x1&&y0==y1)break;e2=2*err;if(e2>=dy){err+=dy;x0+=sx;}if(e2<=dx){err+=dx;y0+=sy;}}}
static void text(int x,int y,const char*s,unsigned int c){pspDebugScreenSetXY(x/8,y/8);pspDebugScreenSetTextColor(c);pspDebugScreenPrintf("%s",s);}
/* map: 1 = wall, 0 = corridor */
static const char *map[]={"111111111111","100000000001","101101001001","100001000001","101001011101","100000000001","111111111111"};
static int wall(float x,float y){int ix=(int)x,iy=(int)y;if(iy<0||iy>=7||ix<0||ix>=12)return 1;return map[iy][ix]=='1';}
static void render_world(void){int x,y; for(y=0;y<H;y++)for(x=0;x<W;x++){if(y<H/2){float k=(float)y/(H/2);put(x,y,rgb(25+(int)(k*12),23+(int)(k*10),9));}else{float k=(float)(y-H/2)/(H/2);put(x,y,rgb(55+(int)(k*35),48+(int)(k*29),20+(int)(k*10)));}}
 for(x=0;x<W;x++){float ra=pa+atanf(((float)x-W/2)/(W*.68f));float dist=0;int hit=0;float hx=0,hy=0;while(dist<12&&!hit){dist+=.035f;hx=px+cosf(ra)*dist;hy=py+sinf(ra)*dist;hit=wall(hx,hy);}if(dist<.05f)dist=.05f;int hh=(int)(H/(dist*.95f));int top=H/2-hh/2,bot=H/2+hh/2;float shade=1.0f/(1.0f+dist*.13f);for(y=top;y<bot;y++){int ty=(int)((float)(y-top)/((float)(bot-top))*128);int tx=(int)(hx*32+hy*17)&31;int pattern=((tx/4+ty/8)%2);int base=pattern?181:161;int r=(int)(base*shade),g=(int)((base-17)*shade),b=(int)((base-88)*shade);if(tx<2||ty%32<2){r-=18;g-=18;b-=8;}put(x,y,rgb(r,g,b));}if(top>=0&&top<H)put(x,top,rgb(95,82,35));}
 /* fluorescent ceiling panels */
 for(int i=0;i<4;i++){int cx=240+(int)((i-1.5f)*150/(i+1));int yy=22+i*18;rect(cx-24,yy,48,4,rgb(210,204,147));rect(cx-20,yy+1,40,2,rgb(255,244,190));}
 /* red smiley graffiti on nearby wall */
 if(fabsf(pa)<1.0f){int sx=370,sy=110;for(int a=0;a<360;a+=4){float q=a*PI/180;put(sx+(int)(25*cosf(q)),sy+(int)(25*sinf(q)),rgb(160,15,18));}rect(sx-12,sy-9,4,7,rgb(20,15,10));rect(sx+8,sy-9,4,7,rgb(20,15,10));for(int i=0;i<15;i++){int xx=sx-14+i*2;int yy=sy+4+(int)(8*sinf(i*PI/14));put(xx,yy,rgb(170,15,20));}}
 /* crosshair */ line(236,136,244,136,rgb(230,220,175));line(240,132,240,140,rgb(230,220,175));
}
static void menu_screen(void){rect(0,0,W,H,rgb(18,15,8));for(int i=0;i<12;i++){int x=i*43;rect(x,0,39,H,rgb(44+i%3*3,38+i%3*2,16));for(int y=0;y<H;y+=18)line(x,y,x+39,y,rgb(61,52,22));}rect(0,0,W,58,rgb(12,11,6));text(20,12,"BACKROOMS TALE",rgb(235,220,80));text(22,34,"ISTORIYA STIVA  /  PSP DEMO",rgb(190,177,87));const char *opts[]={"> START","  CONTINUE","  SETTINGS","  EXIT"};for(int i=0;i<4;i++)text(35,112+i*30,opts[i],i==menu?rgb(255,255,220):rgb(150,143,111));text(20,244,"D-PAD: MOVE MENU   X: SELECT",rgb(190,180,135));}
static void intro_screen(void){rect(0,0,W,H,rgb(5,5,4));if(introTick<100){rect(0,0,W,H,rgb(35,35,31));for(int x=0;x<W;x+=48)rect(x,65,35,120,rgb(55,52,43));rect(205,90,38,70,rgb(26,62,72));rect(210,75,28,25,rgb(78,49,32));text(18,18,"SCENE 1: THE STORE",rgb(230,230,210));text(18,224,"Steve wandered into a store...",rgb(230,230,210));}else if(introTick<200){rect(0,0,W,H,rgb(10,9,7));rect(190,45,100,190,rgb(45,41,25));rect(203,70,55,70,rgb(25,61,70));rect(214,48,34,25,rgb(75,48,30));text(18,18,"SCENE 2: THE WALL",rgb(230,230,210));text(18,224,"He touched the wall...",rgb(230,230,210));}else{for(int y=0;y<H;y++)for(int x=0;x<W;x++)put(x,y,rgb(90+y/6,77+y/8,29+y/12));text(18,18,"BACKROOMS TALE",rgb(255,230,80));text(18,224,"...and the world disappeared.",rgb(240,230,190));}text(365,252,"TRIANGLE: SKIP",rgb(180,170,120));}
int main(void){sceCtrlSetSamplingCycle(0);sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);sceDisplaySetMode(0,W,H);sceDebugScreenInit();SceCtrlData pad;unsigned int old=0;for(;;){sceDisplayWaitVblankStart();fb=(unsigned int*)sceDisplayGetFrameBuf(&W,&H,0,1);if(!fb)continue;sceCtrlReadBufferPositive(&pad,1);unsigned int press=pad.Buttons&~old;old=pad.Buttons;if(scene==0){intro_screen();introTick++;if(introTick>300||(press&PSP_CTRL_TRIANGLE)){scene=1;introTick=0;}}else if(scene==1){menu_screen();if(press&PSP_CTRL_UP)menu=(menu+3)%4;if(press&PSP_CTRL_DOWN)menu=(menu+1)%4;if(press&PSP_CTRL_CROSS){if(menu==0||menu==1)scene=2;else if(menu==3){sceKernelExitGame();}}}else{render_world();text(8,8,"BACKROOMS TALE - LEVEL 0",rgb(240,230,170));text(8,252,"D-PAD MOVE / LEFT-RIGHT TURN   START MENU",rgb(225,215,160));if(press&PSP_CTRL_START)scene=1;if(pad.Buttons&PSP_CTRL_LEFT)pa-=.045f;if(pad.Buttons&PSP_CTRL_RIGHT)pa+=.045f;float nx=px,ny=py;if(pad.Buttons&PSP_CTRL_UP){nx+=cosf(pa)*.055f;ny+=sinf(pa)*.055f;}if(pad.Buttons&PSP_CTRL_DOWN){nx-=cosf(pa)*.045f;ny-=sinf(pa)*.045f;}if(!wall(nx,py))px=nx;if(!wall(px,ny))py=ny;}
 }
 return 0;}
