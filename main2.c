
#include <ncurses.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

struct worker {
    int place;
};

struct place {
    bool occupied;
};

struct boss {
    bool online;
    int pass;
};




#define workerAmount 10
#define passAmount 2
#define exitkey 113

int howMuchWorkersStand = workerAmount;
int howMuchWorkersonHoliday=0;

boss boss = {false,0};
worker workers[workerAmount];
place place ={false};


pthread_mutex_t synchro_worker = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t synchro_pass = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t synchro_boss = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t synchro_hr=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t iffreeplace = PTHREAD_COND_INITIALIZER;
pthread_cond_t ifbossOnline = PTHREAD_COND_INITIALIZER;


void draw_legend(){
  mvprintw(20,59," LEGENDA");
  mvprintw(20,59, "OK - boss dostepny");
  mvprintw(21,59,"W8 - oczekujemy bossa");
  mvprintw(22,59,"P - podanie");
  mvprintw(23,59,"W - pracownik");
  refresh();
}


void draw_boss_table(){
  int y,x;
//y = 12
  for(int x = 20; x<=50; x++){
    y=10;
    mvprintw(y,x,"_");
    mvprintw(y+6,x,"_");
  }
  for(int y=11; y<=23; y++){
    x=19;
    mvprintw(y,x,"|");
  }
  for(int y=11; y<=23; y++){
    x=51;
    mvprintw(y,x,"|");
  }

  refresh();
}

void draw_worker(int number){
  int y,x;

  for(int x_clean=0; x_clean<=79; x_clean++ ){
    for(int y_clean=2; y_clean<=6; y_clean++){
      mvprintw(y_clean,x_clean," ");
    }
  }

  for(int i=1; i<=number; i++){
    y = 2+(rand() % 5);
    x = 0+(rand() % 80);
    mvprintw(y,x,"W");

  }

  refresh();
}

/// 20<x<50, 11<y<15 table
void draw_pass(int number){
  int y,x;

  for(int x_clean=21; x_clean<=50; x_clean++ ){
    for(int y_clean=11; y_clean<=15; y_clean++){
      mvprintw(y_clean,x_clean," ");

    }
  }

  for(int i=1; i<=number; i++){
    y = 11+(rand() % 4);
    x = 21+(rand() % 29);
    mvprintw(y,x,"|P|");

  }

  refresh();
}


void clear_boss(int y, int x){
  for(int x_clean=21; x_clean<50; x_clean++ ){
    for(int y_clean=17; y_clean<=25; y_clean++){
      mvprintw(y_clean,x_clean," ");
    }
  }
  mvprintw(19,27,"Boss out of office");
  refresh();
}

void draw_boss(int y, int x){

  for(int x_clean=21; x_clean<50; x_clean++ ){
    for(int y_clean=17; y_clean<=25; y_clean++){
      mvprintw(y_clean,x_clean," ");
    }
  }

  mvprintw(y,x+2, "/\\");
  mvprintw(y+1,x+2, "\\/");
  mvprintw(y+2,x+4, "\\");
  mvprintw(y+2,x+1, "/");
  mvprintw(y+2,x+2, "||");
  mvprintw(y+3,x+2, "||");
  mvprintw(y+4,x+4, "\\");
  mvprintw(y+4,x+1, "/");
  refresh();

}

void draw_office(){
  int x, y;
  getmaxyx(stdscr, y, x);
  for(int i = 0 ; i < x; i++){

    mvprintw(0, x/2 - 8, "OFFICE");
    mvprintw(1, i, "_");
    mvprintw(8, i, "_");

  }

  mvprintw(9, x/2 - 10, "BOSS OFFICE ");
  refresh();
}

/*void fill_workers_table(){

    for(int i=0; i<workerAmount; i++){
      workers[i].place = (rand() % passAmount) + 2;
      //printf("Worker %d ma miejsce %d\n",i, workers[i].place );
    }
}
int tmp;
*/
void* workers_func(void *args){
  while(1){

    pthread_mutex_lock(&synchro_worker);
    pthread_cond_wait(&iffreeplace, &synchro_worker);
    boss.pass++;
    howMuchWorkersStand--;
    draw_worker(howMuchWorkersStand);
    pthread_mutex_lock(&synchro_boss);


    if(boss.pass>passAmount){
        place.occupied=true;
        pthread_cond_broadcast(&iffreeplace);
    }


    usleep(1000*1000);
    draw_pass(boss.pass);
    pthread_mutex_unlock(&synchro_boss);

    pthread_mutex_unlock(&synchro_worker);

  }

}



void* boss_func(void *args){
  while(1){
    pthread_mutex_lock(&synchro_worker);

    pthread_mutex_lock(&synchro_pass);
    mvprintw(0,0,"W8");
    pthread_mutex_unlock(&synchro_pass);

    pthread_cond_wait(&ifbossOnline, &synchro_worker);

    pthread_mutex_lock(&synchro_pass);
    mvprintw(0,0,"OK");
    pthread_mutex_unlock(&synchro_pass);

      draw_boss(18,32);
      howMuchWorkersonHoliday=0;
      howMuchWorkersStand= howMuchWorkersStand+boss.pass;
      usleep(1000*1000);
      clear_boss(18,32);
      place.occupied=false;
      boss.pass=0;
      pthread_mutex_unlock(&synchro_worker);

  }
}

void* HR_func(void* args){
  while(1){
    pthread_mutex_lock(&synchro_hr);
    if(place.occupied==false){
      pthread_cond_broadcast(&iffreeplace);
      pthread_mutex_unlock(&synchro_hr);

    }
    if(place.occupied==true){
      pthread_cond_broadcast(&ifbossOnline);
      pthread_mutex_unlock(&synchro_hr);
    }
  pthread_mutex_unlock(&synchro_hr);
  }
}



int main(){

srand(time(NULL));
initscr();
noecho();
curs_set(0);

draw_office();
draw_boss_table();
draw_legend();

clear_boss(18,35);

pthread_t workers_thread[workerAmount];
pthread_t boss[workerAmount];
pthread_t hr;
//pthread_t boss_offline;


  pthread_create(&hr,NULL,HR_func, NULL);




//  pthread_create(&boss_offline,NULL,boss_off,NULL);


for(int i =1; i<=workerAmount; i++){

  pthread_create(&workers_thread[i], NULL, workers_func, NULL);
    pthread_create(&boss[i], NULL, boss_func, NULL);
}








while(1){

    int is_exit = getch();

    if (is_exit == exitkey){ /// q
      endwin();
      exit(0);
    }
  }
}
