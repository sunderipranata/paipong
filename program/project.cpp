#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <wiringPi.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
//for sonar
#define TRUE 1
#define TRIG1 4
#define ECHO1 5
#define TRIG2 26
#define ECHO2 6
#define TURBIN 27
//for servo
#define SERVO1 0
#define SERVO2 1

void *thread1(void *ptr);
void *thread2(void *ptr);
void *thread3(void *ptr);

void setup(){
	wiringPiSetup();
	// setup untuk servo 1 dan 2
	pinMode(SERVO1, OUTPUT);
	pinMode(SERVO2, OUTPUT);
	digitalWrite(SERVO1, LOW);
	digitalWrite(SERVO2, LOW);

	// setup untuk sonar 1
	pinMode(TRIG1, OUTPUT);
	pinMode(ECHO1, INPUT);
	digitalWrite(TRIG1, LOW);
	// setup untuk sonar 2
	pinMode(TRIG2, OUTPUT);
	pinMode(ECHO2, INPUT);
	digitalWrite(TRIG2, LOW);

	//setup turbin
	pinMode(TURBIN, OUTPUT);
	digitalWrite(TURBIN,LOW);

	delay(30);
}

double getUltraOne(){
	//kirim pulsa
	digitalWrite(TRIG1, HIGH);
	delayMicroseconds(20);
	digitalWrite(TRIG1, LOW);
	// tunggu echonya start
	while(digitalRead(ECHO1)==LOW);
	//Tunggu echo selesai
	double startTime=micros();
	while(digitalRead(ECHO1)==HIGH);
	double  travelTime = micros() - startTime;
	//distance
	double distance = (travelTime/1000000.0)*344/2; //m
	return distance*100; // cm
}

double getUltraTwo(){
	//kirim pulsa
	digitalWrite(TRIG2, HIGH);
	delayMicroseconds(20);
	digitalWrite(TRIG2, LOW);
	// tunggu echonya start
	while(digitalRead(ECHO2)==LOW);
	//Tunggu echo selesai
	double startTime=micros();
	while(digitalRead(ECHO2)==HIGH);
	double  travelTime = micros() - startTime;
	//distance
	double distance = (travelTime/1000000.0)*344/2; //m
	return distance*100; // cm
}

int speed1=1500;
int speed2=1000;
int akses=0;
double sonarKanan=0, sonarKiri=0;

void kiri()
{
	printf("kiri\n");
	speed1=1000;
	speed2=1000;
}
void kanan()
{
	printf("kanan\n");
	speed1=1500;
	speed2=1500;
}
void maju()
{
	printf("maju\n");
	speed1=1500;
	speed2=1000;
}
void mundur()
{
	printf("mundur\n");
	speed1=1000;
	speed2=1500;
}
int main(){
	int fd;
	if((fd=open("fifo", O_RDONLY))<0){
		perror("open fifo1");
		exit(EXIT_FAILURE);
	}
	//1 servo1,2 servo2, 3 sonar
	pthread_t thrd1,thrd2,thrd3;
	int ret;
	setup();

	ret=pthread_create(&thrd1,NULL, &thread1, NULL);
	if(ret){
		perror("pthread create thread1");
		exit(EXIT_FAILURE);
	}

	ret=pthread_create(&thrd2,NULL, &thread2, NULL);
	if(ret){
		perror("pthread create thread2");
		exit(EXIT_FAILURE);
	}
/*
	ret=pthread_create(&thrd3,NULL, &thread3, NULL);
	if(ret){
		perror("pthread create thread3");
		exit(EXIT_FAILURE);
	}
*/
	delay(1000);
	int ctr=0,turbinctr=0;
	char buf[4];
	int simbol=0;
	bool turbin= 0;
	kanan();
	while(1){
		//read(fd,buf,sizeof(int));
		//printf("Servo 1 : %d # Servo 2 : %d\n", speed1,speed2); fflush(stdout);
		//printf("Sonar 1 : %.2lf # Sonar 2 : %.2lf \n",sonarKiri,sonarKanan); fflush(stdout);
		delay(5);
		read(fd,buf,sizeof(int));
		simbol=atoi(buf);
		if(akses==0)
		{
			//read(fd,buf,sizeof(int));
			//simbol=atoi(buf);
			if(simbol == 0)
			{
				ctr++;
				if(ctr==15)
				{
					kanan();
					ctr=0;
				}
			}
			else if(simbol == 1)
			{
				kanan();
				ctr=0;
			}
			else if(simbol == 2)
			{
				kiri();
				ctr=0;
			}
			else if(simbol == 3)
			{
				maju();
				ctr=0;
			}
			else if(simbol ==4) //dekat bola
			{
				//turbinctr++;
				printf("Turbin On\n");
				digitalWrite(TURBIN, HIGH);
				maju();
				//delay(2000);
				//if(turbinctr==30)
				//digitalWrite(TURBIN, LOW);
				//printf("Turbin Off\n");
				ctr=0;
				turbin=1;
			}

			if(turbin)
			{
				turbinctr++;
				if(turbinctr>=40)
				{
					printf("Turbin Off\n");
					digitalWrite(TURBIN, LOW);
					turbin=false;
					turbinctr=0;
				}
			}

		}
	}

	pthread_join(thrd2,NULL);
	pthread_join(thrd1,NULL);
	pthread_join(thrd3,NULL);
	close(fd);
	exit(EXIT_SUCCESS);
	return 0;
}

//servo 1
void *thread1(void *ptr){
	while(1){
		//printf("Servo 1 : %d\n",speed1); fflush(stdout);
		digitalWrite(SERVO1, HIGH);
		delayMicroseconds(speed1);
		digitalWrite(SERVO1, LOW);
		delayMicroseconds(speed1);
	}
}

//servo2
void *thread2(void *ptr){
	while(1){
		//printf("Servo 2 : %d\n",speed2); fflush(stdout);
		digitalWrite(SERVO2, HIGH);
		delayMicroseconds(speed2);
		digitalWrite(SERVO2, LOW);
		delayMicroseconds(speed2);
	}
}
//sonar
void *thread3(void *ptr){
	while(1){
		sonarKiri=getUltraOne();
		sonarKanan=getUltraTwo();
		//printf("Sonar kiri : %lf # Sonar kanan : %lf\n",sonarKiri, sonarKanan); fflush(stdout);

		//bila kanan kiri dekat tembok
		if(sonarKiri < 10.0 && sonarKanan<10.0)
		{
			//printf("mundur\n");
			akses=1;
			mundur();
			delay(2000);
			//printf("kanan\n");
			kanan();
			delay(2000);
			maju();
		}
		else if(sonarKiri < 10.0)
		{
			//printf("kanan\n");
			akses=1;
			kanan();
			delay(2000);
			maju();
		}
		else if(sonarKanan<10.0)
		{
			//printf("kiri\n");
			akses=1;
			kiri();
			delay(2000);
			maju();
		}
		else{
			//printf("maju\n");
		}
		akses=0;
		delay(200);
	}
}
