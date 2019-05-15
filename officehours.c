/*
  Name: Pranav Bhandari
*/




#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>


/*** Constants that define parameters of the simulation ***/

#define MAX_SEATS 3        /* Number of seats in the professor's office */
#define professor_LIMIT 10 /* Number of students the professor can help before he needs a break */
#define MAX_STUDENTS 1000  /* Maximum number of students in the simulation */

#define CLASSA 0
#define CLASSB 1
#define CLASSC 2
#define CLASSD 3

/*
  These mutexes are used to provide consistensy 
  semaphore mutex makes sure there are always less than
  3 people in the office hours.
  Rest of the mutexes are used for different critical regions
*/
sem_t mutex;
pthread_mutex_t class = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t class2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t class3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t class4 = PTHREAD_MUTEX_INITIALIZER;
sem_t mut;
int current_class = 2;
static int students_sincea = 0; /*counts the number of students since a student from classA entered*/
static int students_sinceb = 0; /*counts the number of students since a student from classB entered*/
int t = 0;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond3 = PTHREAD_COND_INITIALIZER;
int off = 0; 


static int students_in_office;   /* Total numbers of students currently in the office */
static int classa_inoffice;      /* Total numbers of students from class A currently in the office */
static int classb_inoffice;      /* Total numbers of students from class B in the office */
static int students_since_break = 0;


typedef struct 
{
  int arrival_time;  // time between the arrival of this student and the previous student
  int question_time; // time the student needs to spend with the professor
  int student_id;
  int class;
} student_info;

//Called at beginning of simulation.  
static int initialize(student_info *si, char *filename) 
{
  students_in_office = 0;
  classa_inoffice = 0;
  classb_inoffice = 0;
  students_since_break = 0;

  // Initializing synchronization variables
  sem_init(&mutex,0,3);
  sem_init(&mut,0,1);

  /* Read in the data file and initialize the student array */
  FILE *fp;

  if((fp=fopen(filename, "r")) == NULL) 
  {
    printf("Cannot open input file %s for reading.\n", filename);
    exit(1);
  }

  int i = 0;
  while ( (fscanf(fp, "%d%d%d\n", &(si[i].class), &(si[i].arrival_time), &(si[i].question_time))!=EOF) && 
           i < MAX_STUDENTS ) 
  {
    i++;
  }

 fclose(fp);
 return i;
}

// Code executed by professor to simulate taking a break. 
static void take_break() 
{
  printf("The professor is taking a break now.\n");
  assert( students_in_office == 0 );
  sleep(5);
  students_since_break = 0;
  students_sincea = 0;
  students_sinceb = 0;
  if(t==1)
  {
    current_class = 2;
    pthread_cond_broadcast(&cond2);
    t=0;
  }
  else 
    pthread_cond_broadcast(&cond);
}

//Code for the professor thread. 
void *professorthread(void *junk) 
{
  printf("The professor arrived and is starting his office hours\n");

  /* Loop while waiting for students to arrive. */
  while (1) 
  {
    /* 
      code here to handle the student's request.             
      the professor thread runs continuously till students since break = 10
      the thread takes a break and continues runnning
    */
    if(students_since_break == 10)
    {
      pthread_mutex_lock(&class3);
      pthread_cond_wait(&cond1, &class3);
      take_break();
      pthread_mutex_unlock(&class3);
    }
  }
  pthread_exit(NULL);
}


// Code executed by a class A student to enter the office.
void classa_enter() 
{

  /* Request permission to enter the office.  */                                                  
  if(current_class == 2)
    current_class = CLASSA;
  
  /* 
    The thread here gos ahead if the current class is the class of the thread
    otherwise it waits for a signal from the other class that they are done to continue 
  */
  pthread_mutex_lock(&class);
  while(current_class != CLASSA)
  {
    pthread_cond_wait(&cond, &class);
    current_class = CLASSA;
  }
  pthread_mutex_unlock(&class);

  sem_wait(&mutex);
  classa_inoffice += 1;
  students_sinceb += 1;
  students_in_office += 1;
  students_since_break += 1;
  if(students_since_break == 10)
    off = 1;
}

//Code executed by a class B student to enter the office.
void classb_enter() 
{
  /* Requesting permission to enter the office. */
  if(current_class == 2)
    current_class = CLASSB;

  /*
    The thread here gos ahead if the current class is the class of the thread
    otherwise it waits for a signal from the other class that they are done to continue 
  */
  pthread_mutex_lock(&class2);
  while(current_class != CLASSB)
  {
    pthread_cond_wait(&cond, &class2);
    current_class = CLASSB;
  }
  pthread_mutex_unlock(&class2);
  
  sem_wait(&mutex);
  classb_inoffice += 1;
  students_sincea += 1;
  students_in_office += 1;
  students_since_break += 1;
  if(students_since_break == 10)
    off = 1;
}

//Code executed by a student to simulate the time he spends in the office asking questions
static void ask_questions(int t) 
{
  sleep(t);
}

//Code executed by a class A student when leaving the office.
static void classa_leave() 
{

  students_in_office -= 1;
  classa_inoffice -= 1;
  sem_post(&mutex);
  /*
    checks if this is the last student
    if it is then it does various checks and broadcasts a signal accordingly
  */
  if(students_in_office == 0)
  {
    if(off!=0)
    {
      pthread_cond_broadcast(&cond1);
      off = 0;
    }
    else if(students_sinceb == 5)
    {
      pthread_cond_broadcast(&cond);
      pthread_cond_broadcast(&cond3);
    }
    else
      pthread_cond_broadcast(&cond);
    students_sinceb = 0;
  }
}
// Code executed by a class B student when leaving the office.
static void classb_leave() 
{
  students_in_office -= 1;
  classb_inoffice -= 1;
  sem_post(&mutex);
  /*
    checks if this is the last student
    if it is then it does various checks and broadcasts a signal accordingly
  */
  if(students_in_office == 0)
  { 
    if(off!=0)
    {  
      pthread_cond_broadcast(&cond1);
      off = 0;
    }
    else if(students_sincea == 5)
    {
      students_sincea = 0;
      pthread_cond_broadcast(&cond);
      pthread_cond_broadcast(&cond3);
    }
    else
      pthread_cond_broadcast(&cond);
    students_sincea = 0;
  }
}

// Main code for class A student threads.  
void* classa_student(void *si) 
{
  student_info *s_info = (student_info*)si;

  /* enter office */
  sem_wait(&mut);
  if(off != 0)
  {
    pthread_mutex_lock(&class4);
    t = 1;
    pthread_cond_wait(&cond2,&class4);
    pthread_mutex_unlock(&class4);
  }

  else if (students_sinceb == 5)
  {
    pthread_mutex_lock(&class4);
    pthread_cond_wait(&cond3,&class4);
    pthread_mutex_unlock(&class4);
  }
  
  classa_enter();
  sem_post(&mut);

  printf("Student %d from class A enters the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classb_inoffice == 0 );
  
  /* ask questions  --- do not make changes to the 3 lines below*/
  printf("Student %d from class A starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class A finishes asking questions and prepares to leave\n", s_info->student_id);
  printf("Student %d from class A leaves the office\n", s_info->student_id);
  /* leave office */
  classa_leave();  

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  pthread_exit(NULL);
}

// Main code for class B student threads.
void* classb_student(void *si) 
{
  student_info *s_info = (student_info*)si;
  
  /* enter office */
  sem_wait(&mut);
  if(off!=0)
  {
    pthread_mutex_lock(&class4);
    t =1;
    pthread_cond_wait(&cond2,&class4);
    pthread_mutex_unlock(&class4);
  }

  else if (students_sincea == 5)
  {
    pthread_mutex_lock(&class4);
    pthread_cond_wait(&cond3,&class4);
    pthread_mutex_unlock(&class4);
  }
  classb_enter();
  sem_post(&mut);
  printf("Student %d from class B enters the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  assert(classa_inoffice == 0 );

  printf("Student %d from class B starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class B finishes asking questions and prepares to leave\n", s_info->student_id);

  printf("Student %d from class B leaves the office\n", s_info->student_id);
  /* leave office */
  classb_leave();        

  

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

  pthread_exit(NULL);
}

/* 
  Main function sets up simulation and prints report
  at the end.
*/
int main(int nargs, char **args) 
{
  int i;
  int result;
  int student_type;
  int num_students;
  void *status;
  pthread_t professor_tid;
  pthread_t student_tid[MAX_STUDENTS];
  student_info s_info[MAX_STUDENTS];
  if (nargs != 2) 
  {
    printf("Usage: officehour <name of inputfile>\n");
    return EINVAL;
  }

  num_students = initialize(s_info, args[1]);
  if (num_students > MAX_STUDENTS || num_students <= 0) 
  {
    printf("Error:  Bad number of student threads. "
           "Maybe there was a problem with your input file?\n");
    return 1;
  }

  printf("Starting officehour simulation with %d students ...\n",
    num_students);

  result = pthread_create(&professor_tid, NULL, professorthread, NULL);

  if (result) 
  {
    printf("officehour:  pthread_create failed for professor: %s\n", strerror(result));
    exit(1);
  }

  for (i=0; i < num_students; i++) 
  {

    s_info[i].student_id = i;
    sleep(s_info[i].arrival_time);
                
    student_type = random() % 2;

    if (s_info[i].class == CLASSA)
    {
      result = pthread_create(&student_tid[i], NULL, classa_student, (void *)&s_info[i]);
    }
    else // student_type == CLASSB
    {
      result = pthread_create(&student_tid[i], NULL, classb_student, (void *)&s_info[i]);
    }

    if (result) 
    {
      printf("officehour: thread_fork failed for student %d: %s\n", 
            i, strerror(result));
      exit(1);
    }
  }

  /* wait for all student threads to finish */
  for (i = 0; i < num_students; i++) 
  {
    pthread_join(student_tid[i], &status);
  }

  /* tell the professor to finish. */
  pthread_cancel(professor_tid);

  printf("Office hour simulation done.\n");

  return 0;
}
