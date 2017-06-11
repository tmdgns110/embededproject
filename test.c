/*
*****************************************************************************
                  Embedded System S/W Design, Spring 2017
    Team : ""
    Title :  ""
    Stu No :  2012722002
              2012722041

          Copyright (c) 2017, "" Copyright Holder All Rights Reserved.
*****************************************************************************
*/

/*
*****************************************************************************
                                    HEADER
    Description :
*****************************************************************************
*/
#define _CRT_SECURE_NO_WARNINGS
#include "includes.h"
#include <time.h>
#include"Windows.h"
#include"MMSystem.h"
#include "stdlib.h"  // _MAX_PATH, free()
#include "direct.h"  // _getcwd()

#pragma comment(lib,"Winmm.lib")


/*
*****************************************************************************
                                  CONSTANTS
    Description :
    
*****************************************************************************
*/
#define TASK_STK_SIZE 512
#define testPrior     10
#define N_MSG					100

#define LEFT          0
#define RIGHT         1
#define UP            2
#define BOTTOM        3

#define LENGTH				11
#define DENSITY			13

#define BLACK					DISP_FGND_BLACK
#define YELLOW				DISP_FGND_YELLOW
/*
*****************************************************************************
                          VARIABLES & STRUCTURES
    Description :
*****************************************************************************
*/
OS_STK  testStk[TASK_STK_SIZE];
OS_STK	testStk2[TASK_STK_SIZE];
OS_STK	windStk[TASK_STK_SIZE];
OS_STK	AirPollutionStk[TASK_STK_SIZE];
OS_STK	UptStk[TASK_STK_SIZE];
OS_STK	DispStk[TASK_STK_SIZE];
OS_STK CalStk[TASK_STK_SIZE];
OS_STK LogStk[TASK_STK_SIZE];
OS_STK AlertStk[TASK_STK_SIZE];

OS_EVENT *sem;
OS_EVENT *msg_q; 
void *msg_array[N_MSG];
int calc = 0;
int sml = 0;
int mid = 0;
int lar = 0;

INT8U   div1[4] = { 1,  39,   4,  21};
INT8U   div2[4] = {41,  79,   4,  21};
INT8U   div3[4] = {32,  39,  18,  21};
INT8U   div4[4] = { 1,  33,  18,  21};
INT8U   div5[4] = { 3,   3,   4,  17};


typedef struct {
	// INT8U color;
	// INT8U posY;
	INT8U Large;
	INT8U Middle;
	INT8U Small;
	// INT8U state;
} AIR_POLLUTANT;

AIR_POLLUTANT AirPollutant[LENGTH];
int						VALUE;
/*
*****************************************************************************
                            FUNCTION PROTOTYPES
    Description :
*****************************************************************************
*/
void TaskCreate(void);
void testRoutine(void *data);
void testRoutine2(void *data);

static void TaskStartDispInit(void);
void generateWind(void *pdata);
void generateAirPollution(void *pdata);
void updateStructure(void *pdata);
void taskDisplay(void *pdata);
void Calc(void *pdata);
void LogTask(void*pdata);
void AlertTask(void*pdata);

void fillZero(INT8U from, INT8U to);
/*
*****************************************************************************
                                    MAIN
    Description :
*****************************************************************************
*/

int main (void)
{
	OSInit();
	msg_q = OSQCreate(msg_array, (INT16U)N_MSG);
	if (msg_q == 0)
	{
		printf("creating msg_q is failed\n");
		return -1;
	}
	TaskStartDispInit();
	TaskCreate();
	OSStart();
	return 0;
}



/*
*****************************************************************************
                                  Function - 1
    Description :
*****************************************************************************
*/
void TaskCreate(void) {

	/*
		1.화면 초기화
		2.바람 방향/세기를 랜덤으로 생성하는 태스크
		3.초마다 랜덤으로 대기 값을 생성하는 태스크
		4.구조체를 업데이트하는 태스크
		5.선형식 업데이틑 하는 태스크(값계산)
		6.구조체 정보를 화면에 업데이트 하는 태스크
		7. 사용자로부터 값을 입력받는 태스크
		8.
	*/

	sem = OSSemCreate(1);
	OSTaskCreate(
		generateWind,
		NULL,
		&windStk[TASK_STK_SIZE-1],
		testPrior
	);
	OSTaskCreate(
		generateAirPollution,
		NULL,
		&AirPollutionStk[TASK_STK_SIZE-1],
		testPrior+1
	);
	OSTaskCreate(
		updateStructure,
		NULL,
		&UptStk[TASK_STK_SIZE-1],
		testPrior-1
	);
	OSTaskCreate(
		taskDisplay,
		NULL,
		&DispStk[TASK_STK_SIZE-1],
		testPrior-3
	);
	OSTaskCreate(
		Calc,
		NULL,
		&CalStk[TASK_STK_SIZE - 1],
		testPrior -4
	);
	OSTaskCreate(
		LogTask,
		NULL,
		&LogStk[TASK_STK_SIZE - 1],
		testPrior - 5
	);
	OSTaskCreate(
		AlertTask,
		NULL,
		&LogStk[TASK_STK_SIZE - 1],
		testPrior - 2
		);

//	OSTaskCreate(
//		testRoutine,
	//	(void *)0,
		//&testStk[TASK_STK_SIZE - 1],
		//testPrior+4
	//);
	//
	// OSTaskCreate(
	// 	testRoutine2,
	// 	(void *)0,
	// 	&testStk2[TASK_STK_SIZE - 1],
	// 	testPrior-1
	// );
}


/*
*****************************************************************************
                                  Function - 1
    Description :
*****************************************************************************
*/
static void TaskStartDispInit(void) {
	INT8U initColor = DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY;
	INT8U x,y;
	INT8U i, j;

	PC_DispClrScr(DISP_BGND_LIGHT_GRAY);

	//draw the layout - div1
	//for(x=div1[LEFT]+1; x<div1[RIGHT]; x++) 
	PC_DispStr(div1[LEFT]+1, div1[UP], "―――――――――――――――――――", initColor);
	for(y=div1[UP]+1; y<div1[BOTTOM]; y++) {
		PC_DispStr(div1[LEFT],  y, "｜", initColor);
		PC_DispStr(div1[RIGHT], y, "｜", initColor);
	} 
	//for(x=div1[LEFT]+1; x<div1[RIGHT]; x++) PC_DispStr(x, div1[BOTTOM], "￣", initColor);
	PC_DispStr(div1[LEFT]+1, div1[BOTTOM], "―――――――――――――――――――", initColor);

	//draw the layout - div2
	//for(x=div2[LEFT]+1; x<div2[RIGHT]; x++) 
	PC_DispStr(div2[LEFT]+1, div2[UP], "―――――――――――――――――――", initColor);
	for(y=div2[UP]+1; y<div2[BOTTOM]; y++) {
		PC_DispStr(div2[LEFT],  y, "｜", initColor);
		PC_DispStr(div2[RIGHT], y, "｜", initColor);
	} 
	//for(x=div2[LEFT]+1; x<div2[RIGHT]; x++) PC_DispStr(x, div2[BOTTOM], "￣", initColor);
	PC_DispStr(div2[LEFT]+1, div1[BOTTOM], "―――――――――――――――――――", initColor);

	//draw the layout - div3
	for(x=div3[LEFT]+1; x<div3[RIGHT]; x++) PC_DispStr(x, div3[UP], "―", initColor);
	for(y=div3[UP]+1; y<div3[BOTTOM]; y++) {
		PC_DispStr(div3[LEFT],  y, "｜", initColor);
	}
	PC_DispStr(div3[LEFT]+2, div3[UP]+1, "R　▶", initColor);
	PC_DispStr(div3[LEFT]+2, div3[UP]+2, "10m/s", initColor);

	//draw the layout - div4
	//for(x=div4[LEFT]+1; x<div4[RIGHT]; x++) PC_DispStr(x, div4[UP], "―", initColor);


	PC_DispStr(div4[LEFT]+1, div4[UP], "―――――――――――――――――――", initColor);
	
	//for(y=div4[UP]+1; y<div4[BOTTOM]; y++) {
	//	for(x=div4[LEFT]+2; x<div4[RIGHT]-1; x++) PC_DispStr(x, y, "○", initColor);
	//}
	y = div1[UP] + 13;

	PC_DispStr(div1[LEFT] + 4, y, "１２３４５６７８９10", DISP_FGND_RED + DISP_BGND_LIGHT_GRAY);


	PC_DispStr(div4[LEFT]+2, div4[UP]+1, "○○○○○○○○○○○○○○", initColor);
	PC_DispStr(div4[LEFT]+2, div4[UP]+2, "○○○○○○○○○○○○○○", initColor);

	//draw the layout - div5
	for(y=div5[UP]+1;y<=div5[BOTTOM];y++) PC_DispStr(div5[LEFT],y,"▶", initColor);
}


/*
*****************************************************************************
                                  Function - 1
    Description :
*****************************************************************************
*/
void fillZero(INT8U from, INT8U to) {
	INT8U i;
	for(i=from; i<to; i++) {
		AirPollutant[i].Large=0;
		AirPollutant[i].Middle=0;
		AirPollutant[i].Small=0;
	}
}

/*
*****************************************************************************
                        TASK - Generate Random Wind
    Description :	This task is a task to automatically generate wind
					using rand() function.
					
					If the value is negative, it is the east wind.
					If the value is positive, it is the west wind.
*****************************************************************************
*/
void generateWind(void *pdata) {
	INT8U err;

	srand(time((unsigned int *)0) + (OSTCBCur->OSTCBPrio));

	for(;;) {
		OSSemPend(sem,0,&err);
		// VALUE = rand()%LENGTH;
		// VALUE = rand()%2? VALUE : -VALUE;
		VALUE = rand()%3;
		VALUE = rand()%6? VALUE : -VALUE;
		//VALUE =1 ;
//		printf("[Generate Random Wind]\n\tvalue is %d\n", VALUE);
		OSTimeDly(2);
		OSSemPost(sem);
		// OSTaskSuspend(OS_PRIO_SELF);
	}
}

/*
*****************************************************************************
                      TASK - Generate Random Air Pollution
    Description :
*****************************************************************************
*/
void generateAirPollution(void *pdata) {
	INT8U err;
	INT8U remain;
	INT8U i;
	srand(time((unsigned int *)0) + (OSTCBCur->OSTCBPrio));

	for(;;) {
		OSSemPend(sem,0,&err);
		remain = rand()%DENSITY;
		if(remain) {
			AirPollutant[0].Large = rand()%remain;
			remain -= AirPollutant[0].Large;
		}
		if(remain) {
			AirPollutant[0].Middle = rand()%remain;
			remain -= AirPollutant[0].Middle;
		}
		if(remain) {
			AirPollutant[0].Small = remain;
		}

		// AirPollutant[0].color = 0;
		// AirPollutant[0].posY = ;
		// AirPollutant[0].state = 0;

//		printf("[GenerateAirPollution]\n\t%4u, L:%d M:%d S:%d\n", OSTimeGet(), AirPollutant[0].Large, AirPollutant[0].Middle, AirPollutant[0].Small);
		OSTaskResume(testPrior-1);
		OSSemPost(sem);
	}
}
/*
*****************************************************************************
                      TASK - Update The Air Pollutant
    Description :
*****************************************************************************
*/
void updateStructure(void *pdata) {
	INT8U i,_VALUE;
	char msg[100];
	INT8U err;

	for(;;) {
		OSTaskSuspend(OS_PRIO_SELF);
//		printf("[UpdateTheAirPollution] VALUE is : %d\n", VALUE);

		if(VALUE>0) {
			for(i=LENGTH-1; i>=VALUE; i--) AirPollutant[i] = AirPollutant[i-VALUE];
			fillZero(0,VALUE);
		}
		else if(VALUE<0) {
			_VALUE = -VALUE;
			for(i=0; i<LENGTH-_VALUE; i++) AirPollutant[i] = AirPollutant[i+_VALUE];
			fillZero(LENGTH-_VALUE-1,LENGTH);
		}

		sprintf(msg,"%4u: Task %u schedule", OSTimeGet(),
			OSTCBCur->OSTCBPrio);
		err = OSQPostOpt(msg_q, msg,OS_POST_OPT_BROADCAST); // (13)
		while (err != OS_NO_ERR)
		{
			err = OSQPost(msg_q, msg);
		}
//		 for(i=0;i<LENGTH;i++) printf("%d\t", AirPollutant[i].Large); printf("\n");
//		 for(i=0;i<LENGTH;i++) printf("%d\t", AirPollutant[i].Middle); printf("\n");
//		 for(i=0;i<LENGTH;i++) printf("%d\t", AirPollutant[i].Small); printf("\n");
//		 for(i=0;i<LENGTH;i++) printf("%d\t", i); printf("\n");
		 //OSTaskResume(testPrior-2);
	}
}

void taskDisplay(void *pdata) {
	INT8U i,j,y;
	INT8U temp[3];
	char s[80];
	int per = 0;
	for(;;) {
		OSTaskSuspend(OS_PRIO_SELF);
		

		OSSchedLock();
		PC_DispClrScr(DISP_BGND_LIGHT_GRAY);
		TaskStartDispInit();
		if (VALUE == 0) {
			for (y = div5[UP] + 1; y <= div5[BOTTOM]; y++) PC_DispStr(div5[LEFT], y, "▲", DISP_FGND_BLUE+ DISP_BGND_LIGHT_GRAY);
			PC_DispStr(div3[LEFT] + 2, div3[UP] + 1, "N　▲", DISP_FGND_BLUE + DISP_BGND_LIGHT_GRAY);
			sprintf(s, "%d m/s", VALUE);
			PC_DispStr(div3[LEFT] + 2, div3[UP] + 2, s, DISP_FGND_BLUE + DISP_BGND_LIGHT_GRAY);
		}
		else if (VALUE > 0) {	
			for (y = div5[UP] + 1; y <= div5[BOTTOM]; y++) PC_DispStr(div5[LEFT], y, "▶", DISP_FGND_GREEN + DISP_BGND_LIGHT_GRAY); 
			PC_DispStr(div3[LEFT] + 2, div3[UP] + 1, "R　▶", DISP_FGND_GREEN + DISP_BGND_LIGHT_GRAY);
			sprintf(s, "%d m/s", VALUE);
			PC_DispStr(div3[LEFT] + 2, div3[UP] + 2, s, DISP_FGND_GREEN + DISP_BGND_LIGHT_GRAY);
		}
		else {
			for (y = div5[UP] + 1; y <= div5[BOTTOM]; y++) PC_DispStr(div5[LEFT], y, "◀", DISP_FGND_RED + DISP_BGND_LIGHT_GRAY);
			PC_DispStr(div3[LEFT] + 2, div3[UP] + 1, "L　◀", DISP_FGND_RED + DISP_BGND_LIGHT_GRAY);
			sprintf(s, "%d m/s", VALUE);
			PC_DispStr(div3[LEFT] + 2, div3[UP] + 2, s, DISP_FGND_RED + DISP_BGND_LIGHT_GRAY);
		}
		for(i=1;i<LENGTH;i++) {


			temp[0] = AirPollutant[i].Large;
			temp[1] = AirPollutant[i].Middle;
			temp[2] = AirPollutant[i].Small;
			y=div1[UP]+1;
			//PC_DispStr(div1[LEFT]+2,y,"■", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY );
//			printf("%dm, temp[0] : %d\t, temp[1]:%d\t, temp[2]:%d\n", i,temp[0],temp[1],temp[2]);
//			for(j=0;j>30000000;j++);				

			for(j=temp[0];j>0;j--,y++) {
				PC_DispStr(div1[LEFT]+2+i*2,y,"■", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY); 
			}
			for(j=temp[1];j>0;j--,y++) {
				PC_DispStr(div1[LEFT]+2+i*2,y,"■", DISP_FGND_YELLOW + DISP_BGND_LIGHT_GRAY);
			}
			for(j=temp[2];j>0;j--,y++) {
				PC_DispStr(div1[LEFT]+2+i*2,y,"■", DISP_FGND_RED + DISP_BGND_LIGHT_GRAY); 
			}
			
			sprintf(s, "통합지수 : %d percent \n\t\t\t\t\t\t황사 농도 : %d percent \n\t\t\t\t\t\t미세먼지 농도 : %d percent \n\t\t\t\t\t\t초미세먼지 농도 : %d percent", calc, lar, mid, sml);
			PC_DispStr(div2[LEFT] + 7, div2[UP] + 5, s, DISP_FGND_RED + DISP_BGND_LIGHT_GRAY);
			per = (calc * 14) / 100;
			for (j = 0; j < per; j++){
				if (calc < 25){
					PC_DispStr(div4[LEFT] + 2 + j * 2, div4[UP] + 1, "●", DISP_FGND_WHITE + DISP_BGND_LIGHT_GRAY);
					PC_DispStr(div4[LEFT] + 2 + j * 2, div4[UP] + 2, "●", DISP_FGND_WHITE + DISP_BGND_LIGHT_GRAY);
				}
				else if (calc < 50){
					PC_DispStr(div4[LEFT] + 2 + j * 2, div4[UP] + 1, "●", DISP_FGND_CYAN + DISP_BGND_LIGHT_GRAY);
					PC_DispStr(div4[LEFT] + 2 + j * 2, div4[UP] + 2, "●", DISP_FGND_CYAN + DISP_BGND_LIGHT_GRAY);
				}
				else if (calc < 75){
					PC_DispStr(div4[LEFT] + 2 + j * 2, div4[UP] + 1, "●", DISP_FGND_YELLOW + DISP_BGND_LIGHT_GRAY);
					PC_DispStr(div4[LEFT] + 2 + j * 2, div4[UP] + 2, "●", DISP_FGND_YELLOW + DISP_BGND_LIGHT_GRAY);
				}
				else {
					PC_DispStr(div4[LEFT] + 2 + j * 2, div4[UP] + 1, "●", DISP_FGND_RED + DISP_BGND_LIGHT_GRAY);
					PC_DispStr(div4[LEFT] + 2 + j * 2, div4[UP] + 2, "●", DISP_FGND_RED + DISP_BGND_LIGHT_GRAY);
				}
			}
					
			

		
		}
		OSSchedUnlock();
	}

}

void Calc(void *data){

	void *msg;
	INT8U err;
	INT8U temp[3];

	for (;;) {
		msg = OSQPend(msg_q, 0, &err); 
		if (msg != 0)
		{
			lar = AirPollutant[10].Large * (100/12);
			mid = AirPollutant[10].Middle *(100 / 12);
			sml = AirPollutant[10].Small * (100 / 12);
			calc = lar + mid + sml;
			OSTaskResume(testPrior - 3);
		}
	}
}

void LogTask(void *pdata) 
{
	FILE *log;
	void *msg;
	INT8U err;
	INT8U temp[3];
	char s[80];
	log = fopen("log.txt", "w"); // (7)
	for (;;) {
		msg = OSQPend(msg_q, 0, &err); // (8)
		if (msg != 0)
		{
			OSSchedLock();
			lar = AirPollutant[10].Large * (100 / 12);
			mid = AirPollutant[10].Middle *(100 / 12);
			sml = AirPollutant[10].Small * (100 / 12);
			calc = lar + mid + sml;
			sprintf(s,"%s\n통합지수 : %d percent \n황사 농도 : %d percent \n미세먼지 농도 : %d percent \n초미세먼지 농도 : %d percent\n", msg, calc, lar, mid, sml);
			if (calc > 0){
				fprintf(log, "%s", s); // (9)
				fflush(log);
			}
			OSSchedUnlock();
		}
	}
}

void AlertTask(void *pdata)
{

	void *msg;
	INT8U err;
	INT8U temp[3];
	char s[80];
	char s1[80];
	char currentPath[_MAX_PATH];
	for (;;) {
		msg = OSQPend(msg_q, 0, &err); // (8)
		if (msg != 0)
		{
			lar = AirPollutant[10].Large * (100 / 12);
			mid = AirPollutant[10].Middle *(100 / 12);
			sml = AirPollutant[10].Small * (100 / 12);

	
			_getcwd(currentPath, _MAX_PATH);
			sprintf(s1, "%s\\55.wav", currentPath);
			if (lar > 50){
				sprintf(s, "황사 주의보 발령");
				PC_DispStr(div2[LEFT] + 7, div2[UP] + 12, s, DISP_FGND_RED + DISP_BGND_LIGHT_GRAY);
				sndPlaySoundA(s1, SND_ASYNC | SND_NODEFAULT);

			}
			else if (lar > 70){
				sprintf(s, "황사 경보 발령");
				PC_DispStr(div2[LEFT] + 7, div2[UP] + 12, s, DISP_FGND_RED + DISP_BGND_LIGHT_GRAY);
				sndPlaySoundA(s1, SND_ASYNC | SND_NODEFAULT);
			}
			else if (mid > 50){
				sprintf(s, "미세먼지 주의보 발령");
				PC_DispStr(div2[LEFT] + 7, div2[UP] + 12, s, DISP_FGND_RED + DISP_BGND_LIGHT_GRAY);
				sndPlaySoundA(s1, SND_ASYNC | SND_NODEFAULT);
			}
			else if (mid > 70){
				sprintf(s, "미세먼지 경보 발령");
				PC_DispStr(div2[LEFT] + 7, div2[UP] + 12, s, DISP_FGND_RED + DISP_BGND_LIGHT_GRAY);
				sndPlaySoundA(s1, SND_ASYNC | SND_NODEFAULT);
			}
			else if (sml > 50){
				sprintf(s, "초미세먼지 주의보 발령");
				PC_DispStr(div2[LEFT] + 7, div2[UP] + 12, s, DISP_FGND_RED + DISP_BGND_LIGHT_GRAY);
				sndPlaySoundA(s1, SND_ASYNC | SND_NODEFAULT);
			}
			else if (sml > 70){
				sprintf(s, "초미세먼지 경보 발령");
				PC_DispStr(div2[LEFT] + 7, div2[UP] + 12, s, DISP_FGND_RED + DISP_BGND_LIGHT_GRAY);
				sndPlaySoundA(s1, SND_ASYNC | SND_NODEFAULT);
			
			}
	

		}
	}
}



/*
*****************************************************************************
                                  TASK - 2
    Description :
*****************************************************************************
*/

void testRoutine(void *data) {
	INT16S key;
	TaskStartDispInit();

  while(1) {
		if (PC_GetKey(&key)) {
      if (key == 0x1B) {
        exit(0);
      } else if(key == 0x30) {
        PC_DispStr(0,0,"inserted 0", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
      } else if(key == 0x31) {
        PC_DispStr(0,0,"inserted 1", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
      } else if(key == 0x32) {
        PC_DispStr(0,0,"inserted 2", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
      } else if(key == 0x33) {
        PC_DispStr(0,0,"inserted 3", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
      } else if(key == 0x34) {
        PC_DispStr(0,0,"inserted 4", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
      } else if(key == 0x35) {
        PC_DispStr(0,0,"inserted 5", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
      }
    }
//	OSTimeDly(1);
  }
}



/*
*****************************************************************************
                                  TASK - 2
    Description :
*****************************************************************************
*/

void testRoutine2(void *data) {
	INT8U posX,posY;
	srand(time(NULL));

  while(1) {
	  posX=rand()%10 + div1[LEFT] + 1;
	  posY=rand()%10 + div1[UP] + 1;
	  PC_DispStr(posX,posY, "Q", DISP_FGND_YELLOW + DISP_BGND_BLACK);

	  OSTimeDly(1);
  }
}

/*
*****************************************************************************
                                  HEADER
    Description :
*****************************************************************************
*/
