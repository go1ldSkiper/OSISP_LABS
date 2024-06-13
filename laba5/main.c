#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stddef.h>
#include <pthread.h>
#include <string.h>

#define data_max (((256 + 3) / 4) * 4)
#define mes_start_max 10
#define mes_final_max 40
#define max_child 1024

typedef struct{
    int type;
    int hash;
    int size;
    char data[data_max];
}mes;

typedef struct{
    mes buf[mes_final_max];
    int curr_max_count;
    int head;
    int tail;
    int counter;
    int injected;
    int extracted;
}message_queue;

message_queue queue;
pthread_mutex_t mutex;
sem_t *free_space;
sem_t *items;

pthread_t  prods[max_child];
int prod_num;

pthread_t  cons[max_child];
int cons_num;

void init_queue(){
    queue.extracted = 0;
    queue.injected = 0;
    queue.counter = 0;
    queue.head = 0;
    queue.tail = 0;
    queue.curr_max_count = mes_start_max;
    memset(queue.buf, 0, sizeof(queue.buf));
}

void check_threads(){
    init_queue();
    int res = pthread_mutex_init(&mutex, NULL);
    if(res){
        fprintf(stdout, "mutex init\n");
        exit(1);
    }
    if((free_space = sem_open("free_space_sem1", (O_RDWR | O_CREAT | O_TRUNC), (S_IRUSR | S_IWUSR),  mes_start_max)) == SEM_FAILED
       || (items = sem_open("items_sem2", (O_RDWR | O_CREAT | O_TRUNC), (S_IRUSR | S_IWUSR), 0)) == SEM_FAILED){
        fprintf(stderr, "sem_open");
        exit(1);
    }
}

int hash(mes* msg)
{
    unsigned long hash = 5381;
    for (int i = 0; i < msg->size + 4; i++)
        hash = ((hash << 5) + hash) + i;
    return (int) hash;
}

void prod_mes(mes* msg){
    int val = rand() % 257;
    if (val == 256)
        msg->type = -1;
    else{
        msg->type = 0;
        msg->size = val;
    }
    for (int i = 0; i < val; ++i)
        msg->data[i] = (char) (rand() % 256);
    msg->hash = 0;
    msg->hash = hash(msg);
}

void consume_mes(mes* msg){
    int mes_hash = msg->hash;
    msg->hash = 0;
    int check = hash(msg);
    if(check != mes_hash){
        fprintf(stderr, "Check sum (= %d) not equal msg_hash (= %d)\n",check, mes_hash);
    }
    msg->hash = mes_hash;
}

int put_msg(mes* msg){
    if(queue.tail == queue.curr_max_count)
        queue.tail = 0;
    queue.buf[queue.tail] = *msg;
    queue.tail++;
    queue.counter++;
    return ++queue.injected;
}

int get_msg(mes* msg){
    if(queue.head == queue.curr_max_count)
        queue.head = 0;
    *msg = queue.buf[queue.head];
    queue.head++;
    queue.counter--;
    return ++queue.extracted;
}

void* prod_handler(){
    mes msg;
    int counter, temp;
    while(true){
        sleep(4);
        sem_getvalue(free_space, &temp);
        if(!temp){
            fprintf(stderr, "Queue is full, prod with id = %lu is waiting\n", pthread_self());
        }
        prod_mes(&msg);
        sem_wait(free_space);
        pthread_mutex_lock(&mutex);
        counter = put_msg(&msg);
        pthread_mutex_unlock(&mutex);
        sem_post(items);
        fprintf(stdout,"Id = %lu\nMessage = %X\nMessages injected counter = %d\n", pthread_self(), msg.hash, counter);
    }
}

void create_prod() {
    if(prod_num == max_child - 1){
        fprintf(stderr, "Max count of prods");
        exit(1);
    }
    int res = pthread_create(&prods[prod_num], NULL, prod_handler, NULL);
    if(res){
        fprintf(stderr, "Can't create a prod\n");
        return;
    }
    prod_num++;

}

void* cons_handler(){
    mes msg;
    int counter, temp;
    while(true){
        sleep(4);
        sem_getvalue(items, &temp);
        if(!temp){
            fprintf(stderr, "Queue is empty, cons with id = %lu is waiting\n", pthread_self());
        }
        sem_wait(items);
        pthread_mutex_lock(&mutex);
        counter = get_msg(&msg);
        pthread_mutex_unlock(&mutex);
        sem_post(free_space);
        consume_mes(&msg);
        fprintf(stdout,"Id = %lu\nMessage = %X\nMessages extracted counter = %d\n", pthread_self(), msg.hash, counter);
    }

}

void create_cons() {
    if(prod_num == max_child - 1){
        fprintf(stderr, "Max count of prods");
        exit(1);
    }
    int res = pthread_create(&cons[cons_num], NULL, cons_handler, NULL);
    if(res){
        fprintf(stderr, "Can't create a prod\n");
        return;
    }
    cons_num++;
}

void del_prod(){
    if(prod_num == 0){
        fprintf(stderr, "No prods to delete\n");
        return;
    }
    prod_num--;
    pthread_cancel(prods[prod_num]);
    pthread_join(prods[prod_num], NULL);
}

void del_cons(){
    if(cons_num == 0){
        fprintf(stderr, "No cons to delete\n");
        return;
    }
    cons_num--;
    pthread_cancel(cons[cons_num]);
    pthread_join(cons[cons_num], NULL);
}

void decrease_buf(){
    if(sem_trywait(free_space)) {
        if(!queue.curr_max_count)
            fprintf(stderr, "size of ring buffer can't be less than 0\n");
        else
            fprintf(stderr, "Cant decrease buff. Consume the message first\n");
        }
        queue.curr_max_count--;
}

void increase_buf(){
    pthread_mutex_lock(&mutex);
    if(queue.curr_max_count != mes_final_max){
        queue.curr_max_count++;
        sem_post(free_space);
    }
    pthread_mutex_unlock(&mutex);
}


int main(){
    check_threads();
    fprintf(stdout, "p - create producer\n");
    fprintf(stdout, "c - create consumer\n");
    fprintf(stdout, "d - delete producer\n");
    fprintf(stdout, "r - delete consumer\n");
    fprintf(stdout, "l - show processes\n");
    fprintf(stdout, "+ - increase buf\n");
    fprintf(stdout, "- - decrease buf\n");
    fprintf(stdout, "q - quit program\n");
    while(true){
        switch(getchar()){
            case 'p' : { create_prod(); break; }
            case 'c' : { create_cons(); break; }
            case 'd' : { del_prod(); break; }
            case 'r' : { del_cons(); break; }
            case '+' : { increase_buf(); break; }
            case '-' : { decrease_buf(); break; }
            case 'l' : {
                for (int i = 0; i < prod_num; i++)
                    fprintf(stdout,"Producer %d: %lu\n", i + 1, prods[i]);
                fprintf(stdout,"\n");
                for (int i = 0; i < cons_num; i++)
                    printf("Consumer %d: %lu\n", i + 1, cons[i]);
                fprintf(stdout,"\n");
                break; }
            case 'q' : {
                int res = pthread_mutex_destroy(&mutex);
                if (res){
                    fprintf(stderr, "Failed mutex destroy\n");
                    exit(1);
                }
                if (sem_unlink("free_space_sem1") || sem_unlink("items_sem2")){
                    fprintf(stderr, "sem_unlink");
                    abort();
                }
                return 0;
            }
            default : break;
        }
    }

}