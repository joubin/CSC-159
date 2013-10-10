// proc.c, 159
// processes are here

#include "sys_calls.h"
#include "spede.h"   // for IO_DELAY() needed here below
#include "externs.h" // for cur_pid needed here below


void IdleProc()
{
  int i;
  while(1)
  {
    cons_printf("0 ");
        for(i=0;i<1666000;i++) IO_DELAY(); // delay for about 1 sec
      }
  }


// void SimpleProc(){
//     int pid;
//     pid = GetPid();
//     while(1)
//     {
//         int sleep_secs = (pid %5)+1; 
//         cons_printf("%d ", pid);
//         Sleep(sleep_secs);
//     }
// }

void Init() // handles key events, move the handling code out of Kernel()
{

    int product_num = 0; // set product_num to zero
    int common_sid = SemInit(1); //get "common_sid" thru SemInit() call

    while (true){
      cons_printf("%d ", GetPid());
        for(i=0;i<1666000;i++) IO_DELAY(); // delay for about 1 sec
          if (cons_kbhit()){
      char key = cons_getchar(); // get the pressed key
      int child_pid;
      switch(key) // see if it's one of the following for service
      {
       case 'p':
      child_pid = Spawn(Producer);
       break;
       case 'c':
       child_pid = Spawn(Consumer);
       break;
       case 's': ShowStatusISR(); break;
         case 'b': breakpoint(); break; // this stops when run in GDB mode
         case 'q': exit(0);
      } // switch(key)
   } // if(cons_kbhit())        

 }
}

void Producer()
{
 int sleep_secs = (pid %5)+1; // determine its sleep seconds (1-5) according to its PID 
 while(1){
  SemWait(common_sid);
    for(i=0;i<1666000;i++) IO_DELAY(); // delay for about 1 sec
      product_num += 1;
    cons_printf("Product count is now %d\n", product_num);
    SemPost(common_sid);
    Sleep(sleep_secs);
  }
}


void Consumer()
{
 int sleep_secs = (pid %5)+1; // determine its sleep seconds (1-5) according to its PID 
 while(1){
  Sleep(sleep_secs);
  SemWait(common_sid);
  if (product_num == 0)
  {
    cons_printf( "Consumer %d wants to consume but no product...\n", GetPid());
  }else{
    cons_printf("Consumer %d is consuming...\n", GetPid());
    product_num -= 1;
      for(i=0;i<1666000;i++) IO_DELAY(); // delay for about 1 sec
        cons_printf("Product count is now %d\n", product_num);

    }
    SemPost(common_sid);
  }
}


