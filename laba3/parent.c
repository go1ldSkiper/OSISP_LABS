#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#define _GNU_SOURCE
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

struct proc_stats{
    pid_t pid;
    char name[40];
    bool is_acceptable;
};

struct proc_stats *child_proc = NULL;
int proc_num = 0;

void sig_handler(int sig, siginfo_t *inf, void *ptr){
    if(sig == SIGUSR1){
            kill((*inf).si_value.sival_int, SIGUSR2);
    } else if(sig == SIGUSR2) {
        fprintf(stdout, "Child process %d printed info successfully\n",(*inf).si_value.sival_int);
    }else if(sig == SIGALRM){
        for(int i = 0; i < proc_num; i++){
            kill(child_proc[i].pid, SIGUSR2);
        }
    }
}

void handler_settings(){
    struct sigaction act;
    sigset_t set;
    memset(&act, 0 , sizeof(act));
    act.sa_sigaction = sig_handler;
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    sigaddset(&set, SIGALRM);
    act.sa_mask = set;
    sigaction(SIGUSR1, &act, 0);
    sigaction(SIGUSR2, &act, 0);
    sigaction(SIGALRM, &act, 0);
}

void create_process(){
    pid_t pid = fork();
    proc_num++;
    child_proc = realloc(child_proc, proc_num * sizeof(struct proc_stats));
    sprintf(child_proc[proc_num - 1].name, "C_%02d", proc_num - 1);
    child_proc[proc_num - 1].pid = pid;
    child_proc[proc_num - 1].is_acceptable = true;
    if(pid < 0){
        fprintf(stderr, "Can't create new process\n");
        exit(1);
    }else if(pid == 0){
        execl("./child", "child", NULL);
    }else{
        fprintf(stdout, "PID %d(%s) - created\n", pid, child_proc[proc_num - 1].name);
        fprintf(stdout, "Processes number = %d\n", proc_num);
    }
}

void delete_last_proc(){
    fprintf(stdout, "Process %d killed\n", child_proc[proc_num - 1].pid);
    kill(child_proc[proc_num - 1].pid, SIGTERM);
    proc_num--;
    child_proc = realloc(child_proc, proc_num * sizeof(struct proc_stats));
    fprintf(stdout, "Processes number = %d\n", proc_num);
}

void delete_all_processes(){
    while(proc_num){
        delete_last_proc();
    }
}

void option_l(){
    fprintf(stdout, "parent - %d\n", getpid());
    fprintf(stdout, "child :\n");

    for(int i = 0; i < proc_num; i++){
        fprintf(stdout, "%s - \t%d\t", child_proc[i].name, child_proc[i].pid);
        if(!child_proc[i].is_acceptable)
            fprintf(stdout, "stopped\n");
        else
            fprintf(stdout, "acceptable\n");
    }
}

void option_k(){
    if(proc_num){
        delete_all_processes();
    }
}

void option_s(){
    for(int i = 0; i < proc_num; i++){
        kill(child_proc[i].pid, SIGUSR1);
        child_proc[i].is_acceptable = false;
    }
}

void option_g(){
    for(int i = 0; i < proc_num; i++){
        kill(child_proc[i].pid, SIGUSR2);
        child_proc[i].is_acceptable = true;
    }
}

int main(){
    int i = 0, j = 0, step = 0;
    handler_settings();
    char choice[20];
    char exact_child[20];
    int exact_child_num;
    while(true){
        choice[0] = '\0';
        fgets(choice, sizeof(choice), stdin);
        if(strstr(choice, "<") && strstr(choice, ">") && step == 0){
            for( i = 0, j = 0; choice[i] != '>'; i++){
                if(i && choice[i - 1] == '<'){
                    exact_child[j] = choice[i];
                    j++;
                }
            }
            exact_child[j] = '\0';
            exact_child_num = atoi(exact_child);
            if(exact_child_num >= proc_num){
                fprintf(stderr, "There are only %d processes\n", proc_num);
                continue;
            }
            else if(choice[0] == 's'){
                kill(child_proc[exact_child_num].pid, SIGUSR1);
                child_proc[exact_child_num].is_acceptable = false;
                fprintf(stdout, "%s process is stopped\n", child_proc[exact_child_num].name);
            }
            else if(choice[0] == 'g'){
                kill(child_proc[exact_child_num].pid, SIGUSR2);
                child_proc[exact_child_num].is_acceptable = true;
                fprintf(stdout, "%s process is running\n", child_proc[exact_child_num].name);
            }
            else if(choice[0] == 'p'){
                option_s();
                kill(child_proc[exact_child_num].pid, SIGUSR2);
                child_proc[exact_child_num].is_acceptable = true;
                fprintf(stdout, "%s process is running\n", child_proc[exact_child_num].name);
                step = 5;
            }
        }
        else switch(choice[0]){
            case '+':{
                create_process();
                break;
            }
            case '-':{
                delete_last_proc();
                break;
            }
            case 'q':{
                if(proc_num){
                    delete_all_processes();
                    exit(1);
                }else exit(1);
            }
            case 'l':{
                option_l();
                break;
            }
            case 'k':{
                option_k();
                break;
            }
            case 's':{
                option_s();
                break;
            }
            case 'g':{
                option_g();
                step = 0;
                break;
            }
        }
        if(step > 1){
            sleep(1);
            step--;
        }
        else if(step == 1){
            sleep(1);
            option_g();
            step--;
        }
        
    }
    return 0;
}