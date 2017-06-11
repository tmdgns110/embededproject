/*
*****************************************************************************
                  Embedded System S/W Design, Spring 2017
    Team	:	뿌셔뿌셔 미세먼지
    Team No.:	12조	
    Stu No	:	2012722002
				2012722041
*****************************************************************************
*/

/*
*****************************************************************************
                                    HEADER
    Description : Include the required header file.
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
    Description :	Define the constants. 
					These correspond to the task stack size, priority, 
					direction, length, and density.    
*****************************************************************************
*/
#define TASK_STK_SIZE			512
#define Prior					20
#define N_MSG					100

#define LEFT					0
#define RIGHT					1
#define UP						2
#define BOTTOM					3

#define LENGTH					11
#define DENSITY					13

/*
*****************************************************************************
                          VARIABLES & STRUCTURES
    Description :	Declare required variables and structures, 
					task stacks, task events, and so on.
*****************************************************************************
*/
//The part of stack.
OS_STK		testStk[TASK_STK_SIZE];
OS_STK		windStk[TASK_STK_SIZE];
OS_STK		AirPollutionStk[TASK_STK_SIZE];
OS_STK		UptStk[TASK_STK_SIZE];
OS_STK		DispStk[TASK_STK_SIZE];
OS_STK		CalStk[TASK_STK_SIZE];
OS_STK		LogStk[TASK_STK_SIZE];
OS_STK		AlertStk[TASK_STK_SIZE];
OS_STK		InitStk[TASK_STK_SIZE];
OS_STK		InitVarStk[TASK_STK_SIZE];

//The part of event. There are two types of events: 
//semaphores and message queues. 
OS_EVENT	*sem;
OS_EVENT	*msg_q;

//The part of variable. A calculation value of a message queue and 
//each atmospheric substance, a variable for 
//storing a value in a manual mode.
void		*msg_array[N_MSG];
int			calc = 0;
int			sml = 0;
int			mid = 0;
int			lar = 0;
int			weight;
int			dens;
int			VALUE;

//The part of layout.
//Specify the bounds for each div for the display layout.
INT8U		div1[4] = { 1,	39,   4,  21};
INT8U		div2[4] = {41,	79,   4,  21};
INT8U		div3[4] = {32,	39,  18,  21};
INT8U		div4[4] = { 1,	33,  18,  21};
INT8U		div5[4] = { 3,	 3,   4,  17};
INT8U		div6[4] = {25,	39,  14,  17};

//Structure to store atmospheric materials.
typedef struct {
	INT8U Large;
	INT8U Middle;
	INT8U Small;
} AIR_POLLUTANT;
AIR_POLLUTANT	AirPollutant[LENGTH];

/*
*****************************************************************************
                            FUNCTION PROTOTYPES
    Description :	There are two types of functions.
					Functions corressponding to task routines,
					the other specific functions by the function-call.
*****************************************************************************
*/
void TaskCreate(void);
static void TaskStartDispInit(void);
void fillZero(INT8U from, INT8U to);

//The task routine.
void testRoutine(void *data);
void generateWind(void *pdata);
void generateAirPollution(void *pdata);
void updateStructure(void *pdata);
void taskDisplay(void *pdata);
void Calc(void *pdata);
void LogTask(void*pdata);
void AlertTask(void*pdata);
void InitTask(void*pdata);
void InitAllVar(void*data);
/*
*****************************************************************************
                                    MAIN
    Description :	This function is the main function. 
					The main function consists of OS initialization, 
					task creation, and OS startup.
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
    Description :	This task is a task that creates several tasks. 
					These tasks include tasks that generate wind, 
					tasks that create atmospheric substances, 
					and tasks that update structures.
*****************************************************************************
*/
void TaskCreate(void) {
	sem = OSSemCreate(1);
	OSTaskCreate(
		generateWind,
		NULL,
		&windStk[TASK_STK_SIZE-1],
		Prior
	);
	OSTaskCreate(
		generateAirPollution,
		NULL,
		&AirPollutionStk[TASK_STK_SIZE-1],
		Prior+1
	);
	OSTaskCreate(
		updateStructure,
		NULL,
		&UptStk[TASK_STK_SIZE-1],
		Prior-1
	);
	OSTaskCreate(
		taskDisplay,
		NULL,
		&DispStk[TASK_STK_SIZE-1],
		Prior-3
	);
	OSTaskCreate(
		Calc,
		NULL,
		&CalStk[TASK_STK_SIZE - 1],
		Prior-4
	);
	OSTaskCreate(
		LogTask,
		NULL,
		&LogStk[TASK_STK_SIZE - 1],
		Prior-5
	);
	OSTaskCreate(
		AlertTask,
		NULL,
		&LogStk[TASK_STK_SIZE - 1],
		Prior-2
		);
	OSTaskCreate(
		testRoutine,
		NULL,
		&testStk[TASK_STK_SIZE - 1],
		Prior-6
	);
	OSTaskCreate(
		InitTask,
		NULL,
		&InitStk[TASK_STK_SIZE - 1],
		Prior-7
	);
	OSTaskCreate(
		InitAllVar,
		NULL,
		&InitVarStk[TASK_STK_SIZE - 1],
		Prior+8
	);
}

/*
*****************************************************************************
                     Function - TaskStartDispInit
    Description :	This function initializes the display layout and 
					updates the display according to the value 
					of the global variable.
*****************************************************************************
*/
static void TaskStartDispInit(void) {
	//Set the initial color, x-value, y-value.
	INT8U initColor = DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY;
	INT8U x,y;
	INT8U i, j;

	//Set the _weight,_s. 
	char _weight[10]={0};
	char _dens[10]={0};
	char s[40];

	//Clear display from LIGHT_GRAY
	PC_DispClrScr(DISP_BGND_LIGHT_GRAY);

	//draw the layout - div1
	PC_DispStr(div1[LEFT]+1, div1[UP]-3, "팀 명 : 뿌셔뿌셔 미세먼지", initColor);
	PC_DispStr(div1[LEFT] + 1, div1[UP] - 2, "학번(이름) : 2012722002(이도연), 2012722041(김승훈)", initColor);
	PC_DispStr(div1[LEFT] + 1, div1[UP] - 1, "작 품 명: 실시간 대기오염 물질 측정 시스템", initColor);
	PC_DispStr(div1[LEFT]+1, div1[UP], "―――――――――――――――――――", initColor);
	for(y=div1[UP]+1; y<div1[BOTTOM]; y++) {
		PC_DispStr(div1[LEFT],  y, "｜", initColor);
		PC_DispStr(div1[RIGHT], y, "｜", initColor);
	} 
	for (y = div1[UP] + 1; y<div6[UP]; y++) {
		PC_DispStr(div6[LEFT]-1,y, "||", initColor);
	}
	PC_DispStr(div1[LEFT]+1, div1[BOTTOM], "―――――――――――――――――――", initColor);

	//draw the layout - div2
	PC_DispStr(div2[LEFT]+1, div2[UP], "―――――――――――――――――――", initColor);
	for(y=div2[UP]+1; y<div2[BOTTOM]; y++) {
		PC_DispStr(div2[LEFT],  y, "｜", initColor);
		PC_DispStr(div2[RIGHT], y, "｜", initColor);
	} 
	PC_DispStr(div2[LEFT]+1, div1[BOTTOM], "―――――――――――――――――――", initColor);
	PC_GetDateTime(s);
	PC_DispStr(div2[LEFT]+18, div2[BOTTOM]-1, s, initColor);
	
	//draw the layout - div3
	for(x=div3[LEFT]+1; x<div3[RIGHT]; x++) PC_DispStr(x, div3[UP], "―", initColor);
	for(y=div3[UP]+1; y<div3[BOTTOM]; y++) {
		PC_DispStr(div3[LEFT],  y, "｜", initColor);
	}
	PC_DispStr(div3[LEFT]+2, div3[UP]+1, "R　▶", initColor);
	PC_DispStr(div3[LEFT]+2, div3[UP]+2, "10m/s", initColor);

	//draw the layout - div4
	PC_DispStr(div4[LEFT]+1, div4[UP], "―――――――――――――――――――", initColor);
	y = div1[UP] + 13;
	PC_DispStr(div1[LEFT] + 4, y, "１２３４５６７８９10", DISP_FGND_RED + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(div4[LEFT]+2, div4[UP]+1, "○○○○○○○○○○○○○○", initColor);
	PC_DispStr(div4[LEFT]+2, div4[UP]+2, "○○○○○○○○○○○○○○", initColor);

	//draw the layout - div5
	for(y=div5[UP]+1;y<=div5[BOTTOM];y++) PC_DispStr(div5[LEFT],y,"▶", initColor);

	//draw the layout - div6 : manual mode.
	if(weight||dens) {
		PC_DispStr(div6[LEFT]+1,div6[UP],"―――――――", initColor);		
		for(y=div6[UP]+1;y<=div6[BOTTOM];y++) PC_DispStr(div6[LEFT],y,"｜", initColor);
		PC_DispStr(div6[LEFT]+2,div6[UP]+1,"Manual Mode", initColor);
	}
	if(dens) {
		sprintf(_dens, "Density:%d", dens);
		PC_DispStr(div6[LEFT]+2,div6[UP]+2,_dens, initColor);
	}
	if(weight) {
		sprintf(_weight,"Wind : %d", weight);
		PC_DispStr(div6[LEFT]+2,div6[UP]+3,_weight, initColor);
	}
}


/*
*****************************************************************************
                             Function - fillZero
    Description :	This function initializes the AirPollutant structure 
					array, which is a structure that stores 
					the atmospheric substance, 
					to zero as much as the incoming parameter.
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
		OSSemPend(sem,0,&err); // binary semaphore locking
		if(weight) {
			VALUE = weight;
		}
		else {
			VALUE = rand()%3;// Range : [-3,3]
			VALUE = rand()%6? VALUE : -VALUE; // Negative by 16.67% 
		}
		OSTimeDly(1); // syncrhonize as 1 second.
		OSSemPost(sem);
	}
}
/*
*****************************************************************************
                     TASK - Generate Random Air Pollution
    Description :	This task is a task to randomly generate 
					atmospheric substances. 
					First, limit the concentration of all the atmospheric 
					substances and set the concentration of each element 
					arbitrarily according to the number of them using a 
					random function, rand().	
*****************************************************************************
*/
void generateAirPollution(void *pdata) {
	INT8U err;
	INT8U remain;
	INT8U i;
	srand(time((unsigned int *)0) + (OSTCBCur->OSTCBPrio));

	for(;;) {
		OSSemPend(sem,0,&err); // binary semaphore locking.

		remain = rand()%(DENSITY-dens)+dens; // set total density.

		if(remain) {
			AirPollutant[0].Large = rand()%remain;
			remain -= AirPollutant[0].Large; // remain remains -Large. 
		}
		if(remain) {
			AirPollutant[0].Middle = rand()%remain;
			remain -= AirPollutant[0].Middle; // remain remains -Middle.
		}
		if(remain) {
			AirPollutant[0].Small = remain; // remain remains -Small.
		}
		OSTaskResume(Prior-1);
		OSSemPost(sem);
	}
}
/*
*****************************************************************************
                      TASK - Update The Air Pollutant
    Description :	This task is a task to update air pollutants. 
					Air pollutants are updated to reflect 
					the VALUE value generated by the generateWind, 
					the 0th AirPollutant structure generated by 
					generateAirPollution.

					If VALUE is positive, the structure is copied back,
					If VALUE is negative, the structure is copied forward.
*****************************************************************************
*/
void updateStructure(void *pdata) {
	INT8U i,_VALUE;
	char msg[100];
	INT8U err;

	for(;;) {
		OSTaskSuspend(OS_PRIO_SELF);
		if(VALUE>0) {
			for(i=LENGTH-1; i>=VALUE; i--) AirPollutant[i] = AirPollutant[i-VALUE]; //forwarding copy from i to i-VALUE.
			fillZero(0,VALUE); // fill the structures be remain as zero.
		}
		else if(VALUE<0) {
			_VALUE = -VALUE;
			for(i=0; i<LENGTH-_VALUE; i++) AirPollutant[i] = AirPollutant[i+_VALUE]; //backing copy from i to i+_VALUE.
			fillZero(LENGTH-_VALUE-1,LENGTH); // fill the structures be remain as zero.
		}
		sprintf(msg,"%4u: Task %u schedule", OSTimeGet(), OSTCBCur->OSTCBPrio);
		err = OSQPostOpt(msg_q, msg,OS_POST_OPT_BROADCAST); // (13) BROADCASTING
		while (err != OS_NO_ERR) { 
			err = OSQPost(msg_q, msg);
		}
	}
}

/*
*****************************************************************************
                               TASK - taskDisplay
    Description :	This task is a task for displaying information 
					in the console window. Scheduling activation/deactivation 
					is used to prevent preemption.
*****************************************************************************
*/
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

		//According to the sign of VALUE, set character properly. 
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
		
		//display AirPollutant from 1 to LENGTH.
		for(i=1;i<LENGTH;i++) {
			temp[0] = AirPollutant[i].Large;
			temp[1] = AirPollutant[i].Middle;
			temp[2] = AirPollutant[i].Small;
			y=div1[UP]+1;

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

			//this character moves as total density, calc.
			for (j = 0; j < per; j++){
				if (calc < 25){ // < 25%
					PC_DispStr(div4[LEFT] + 2 + j * 2, div4[UP] + 1, "●", DISP_FGND_WHITE + DISP_BGND_LIGHT_GRAY);
					PC_DispStr(div4[LEFT] + 2 + j * 2, div4[UP] + 2, "●", DISP_FGND_WHITE + DISP_BGND_LIGHT_GRAY);
				}
				else if (calc < 50){ // <50%
					PC_DispStr(div4[LEFT] + 2 + j * 2, div4[UP] + 1, "●", DISP_FGND_CYAN + DISP_BGND_LIGHT_GRAY);
					PC_DispStr(div4[LEFT] + 2 + j * 2, div4[UP] + 2, "●", DISP_FGND_CYAN + DISP_BGND_LIGHT_GRAY);
				}
				else if (calc < 75){ // <75%
					PC_DispStr(div4[LEFT] + 2 + j * 2, div4[UP] + 1, "●", DISP_FGND_YELLOW + DISP_BGND_LIGHT_GRAY);
					PC_DispStr(div4[LEFT] + 2 + j * 2, div4[UP] + 2, "●", DISP_FGND_YELLOW + DISP_BGND_LIGHT_GRAY);
				}
				else { // >=75%
					PC_DispStr(div4[LEFT] + 2 + j * 2, div4[UP] + 1, "●", DISP_FGND_RED + DISP_BGND_LIGHT_GRAY);
					PC_DispStr(div4[LEFT] + 2 + j * 2, div4[UP] + 2, "●", DISP_FGND_RED + DISP_BGND_LIGHT_GRAY);
				}
			}					
		}
		OSSchedUnlock();
	}

}

/*
*****************************************************************************
                                  TASK - Calc
    Description :	This task is a task to calculate the air pollutant 
					concentration based on the antenna.
*****************************************************************************
*/
void Calc(void *data){
	void *msg;
	INT8U err;
	INT8U temp[3];

	for (;;) {
		msg = OSQPend(msg_q, 0, &err); 
		if (msg != 0) { // Antenna is located in [10].
			lar = AirPollutant[10].Large * (100/12);
			mid = AirPollutant[10].Middle *(100 / 12);
			sml = AirPollutant[10].Small * (100 / 12);
			calc = lar + mid + sml;
			OSTaskResume(Prior - 3);
		}
	}
}

/*
*****************************************************************************
                            TASK - LogTask
    Description :	This task is a task to save the current Air Pollutant 
					Index to a log file.
*****************************************************************************
*/
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

/*
*****************************************************************************
                             TASK - AlertTask
    Description :	This task is a task that alerts you based on the current 
					level of air pollution. An alarm message occurs 
					when the percentage of each contaminant exceeds 50%, 
					and the alarm sounds through the sndPlaySoundA () function.
*****************************************************************************
*/
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
                                TASK - testRoutine
    Description :	This task is a task that responds to user input events. 
					The event is triggered when the user presses 
					the direction key, 0, Ctrl + Z, ESC.
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
				weight=0;
				dens=0;
			} else if(key == 75) {
				weight--;
			} else if(key == 77) {
				weight++;
			} else if(key == 72) {
				if(dens!=(DENSITY-1)) dens++;
			} else if(key == 80) {
				if(dens) dens--;
			}
			else if (key == 26){ // preemptived by initTask
				OSTaskResume(Prior - 7);
				 dens=0;
				 weight=0;
			}
		}	OSTimeDlyHMSM(0,0,1,0);
  }
}

/*
*****************************************************************************
                               TASK - InitTask, InitAllVar
    Description :	This task is a task that initializes variables, structure.
*****************************************************************************
*/
void InitTask(void *data){
	for (;;) {
		OSTaskSuspend(OS_PRIO_SELF);
		fillZero(0, LENGTH);
	}
}

void InitAllVar(void *data) {
	calc=sml=mid=lar=weight=dens=VALUE=0;
	for(;;){
		OSTimeDly(1);
		OSTaskSuspend(OS_PRIO_SELF);
		OSTaskDel(OS_PRIO_SELF);
	}
}