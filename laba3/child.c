#define _GNU_SOURCE
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>

bool sig_change = 1;
bool sig_got = 0;

int num_00 = 0;
int num_01 = 0;
int num_10 = 0;
int num_11 = 0;

struct check_stats{
    int num1;
    int num2;
};

struct check_stats stats;

void alarm_sig_handler(){
    if(stats.num1 == 0 && stats.num2 == 0) num_00 = 1;
    else if(stats.num1 == 1 && stats.num2 == 0) num_01 = 1;
    else if(stats.num1 == 0 && stats.num2 == 1) num_10 = 1;
    else if(stats.num1 == 1 && stats.num2 == 1) num_11 = 1;
    alarm(1 + rand() % 5);
}

void set_stats(struct check_stats *st){
    static int counter;
    switch (counter){
        case 0:{
            (*st).num1 = 0;
            (*st).num2 = 0;
            counter++;
            break;
        }
        case 1:{
            (*st).num1 = 1;
            (*st).num2 = 0;
            counter++;
            break;
        }
        case 2:{
            (*st).num1 = 0;
            (*st).num2 = 1;
            counter++;
            break;
        }
        case 3:{
            (*st).num1 = 1;
            (*st).num2 = 1;
            counter++;
            break;
        }
        default:{
            counter = 0;
            break;
        }
    }
}

void user_sig_handler(int signal){
    if(signal == SIGUSR1){
        sig_change = 0;
        sig_got = 1;
    }
    else if(signal == SIGUSR2){
        sig_change = 1;
        sig_got = 1;
    }
}



int main(){
    int loop_counter = 0;
    srand(time(NULL));
    signal(SIGUSR1, user_sig_handler);
    signal(SIGUSR2, user_sig_handler);
    signal(SIGALRM, alarm_sig_handler);

    alarm(1 + rand() % 5);

    while(true){
        sleep(1);
        set_stats(&stats);
        if(loop_counter >= 5 && sig_change){
            sig_got = 0;
            alarm(0);
            union sigval sig_inf;
            sig_inf.sival_int = getpid();
            while(!sig_got){
                sigqueue(getppid(), SIGUSR1, sig_inf);
                sleep(10);
            }
            alarm(1 + rand() % 5);
            if(!sig_change){
                loop_counter = 0;
                sig_change = 1;
                continue;
            }
            fprintf(stdout, "PPID - %d; PID - %d; ", getppid(), getpid());
            fprintf(stdout, "00 - %d; 01 - %d; 10 - %d; 11 - %d\n", num_00, num_01, num_10, num_11);
            sigqueue(getppid(), SIGUSR2, sig_inf);
            loop_counter = 0;
        }
        loop_counter++;
    }
    exit(0);
}