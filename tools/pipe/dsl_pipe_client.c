/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifdef LINUX

#include "dsl_cpe_config.h"
#include "dsl_pipe_client.h"

#define PIPE_PREFIX "/tmp/pipe/dsl_cpe"
#define SYS_NAME_PREFIX  "/tmp"

#undef DSL_DBG_BLOCK
#define DSL_DBG_BLOCK DSL_DBG_OS

static char Result[20000];
static int sema;

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
   /* union semun is defined by including <sys/sem.h> */
#else
   /* according to X/OPEN we have to define it ourselves */
   union semun {
      int val;                  /* value for SETVAL */
      struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */
      unsigned short *array;    /* array for GETALL, SETALL */
                              /* Linux specific part: */
      struct seminfo *__buf;    /* buffer for IPC_INFO */
   };
#endif

#ifndef DSL_DEBUG_DISABLE
static void print_sem_error(int err)
{
   switch(err)
   {
      case E2BIG:
      printf( "DSL[Pipe]: The value of nsops is greater than the system-"
         "imposed maximum." DSL_PIPE_CRLF);
      break;

      case EACCES:
      printf( "DSL[Pipe]: Operation permission is denied to the calling "
         "process, see IPC." DSL_PIPE_CRLF);
      break;

      case EAGAIN:
      printf( "DSL[Pipe]: The operation would result in suspension of the "
         "calling process but (sem_flg&IPC_NOWAIT) is non-zero." DSL_PIPE_CRLF);
      break;

      case EFBIG:
      printf( "DSL[Pipe]: The value of sem_num is less than 0 or greater "
         "than or equal to the number of semaphores in the set associated "
         "with semid. " DSL_PIPE_CRLF);
      break;

      case EIDRM:
      printf( "DSL[Pipe]: The semaphore identifier semid is removed from "
         "the system. " DSL_PIPE_CRLF);
      break;

      case EINTR:
      printf( "DSL[Pipe]: The semop() function was interrupted by a signal." DSL_PIPE_CRLF);
      break;

      case EINVAL:
      printf( "DSL[Pipe]: The value of semid is not a valid semaphore "
         "identifier, or the number of individual semaphores for which the "
         "calling process requests a SEM_UNDO would exceed the system-"
         "imposed limit. " DSL_PIPE_CRLF);
      break;

      case ENOSPC:
      printf( "DSL[Pipe]: The limit on the number of individual processes "
         "requesting a SEM_UNDO would be exceeded. " DSL_PIPE_CRLF);
      break;

      case ERANGE:
      printf( "DSL[Pipe]: An operation would cause a semval to overflow "
         "the system-imposed limit, or an operation would cause a semadj "
         "value to overflow the system-imposed limit." DSL_PIPE_CRLF);
      break;

      default:
      break;
   }
}
#endif /* DSL_DEBUG_DISABLE */

/** signal handler, will abort blocking semop() call */
static void DSL_PIPE_SemAlarm(int val)
{
}

static int DSL_PIPE_SemaphoreGlobalCreate(DSL_char_t *pName)
{
   /* Semaphore ID used in further semaphore operations */
   int semid = -1;
   /* Used to generate the semaphore key in case pName is not a number */
   DSL_char_t *sempath = DSL_NULL;
   /* the name should be an integer if not the key for semaphore is generated at
   runtime*/
   DSL_int_t nsemkey;
   union semun arg;

   arg.val = 0;

   if (pName == DSL_NULL)
      pName = "";

   nsemkey = atoi(pName);

   /*incase atoi fails generate a tmp string and try to generate a key
      at runtime */
   if (nsemkey == 0)
   {
      sempath = DSL_PIPE_Malloc(strlen(SYS_NAME_PREFIX"/") + strlen(pName) + 1);
      if (sempath == DSL_NULL)
      {
         printf( "DSL: No Mem." DSL_PIPE_CRLF);
         return -1;
      }
      mkdir(SYS_NAME_PREFIX, S_IFDIR | 0777);
      chmod(SYS_NAME_PREFIX, 0040777);
      strcpy(sempath, SYS_NAME_PREFIX"/\0");
      strcat(sempath, pName);
      strcat(sempath, "\0");

      /* creat(sempath, 0666); */
      creat(sempath, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
      /* Access to all */
      /*chmod(sempath, 0777);*/

      /* generate key */
      if ( (nsemkey = ftok(sempath, 5)) < 0)
      {
         switch(errno)
         {
            case EBADF:
            printf( "DSL: filedes is bad." DSL_PIPE_CRLF);
            break;

            case ENOENT:
            printf( "DSL: A component of the path file_name does not exist, "
               "or the path is an empty string." DSL_PIPE_CRLF);
            break;

            case ENOTDIR:
            printf( "DSL: A component of the path is not a directory." DSL_PIPE_CRLF);
            break;

            case ELOOP:
            printf( "DSL: Too many symbolic links encountered while "
               "traversing the path." DSL_PIPE_CRLF);
            break;

            case EFAULT:
            printf( "DSL: Bad address." DSL_PIPE_CRLF);
            break;

            case EACCES:
            printf( "DSL: Permission denied." DSL_PIPE_CRLF);
            break;

            case ENOMEM:
            printf( "DSL: Out of memory (i.e. kernel memory)." DSL_PIPE_CRLF);
            break;

            case ENAMETOOLONG:
            printf( "DSL: File name too long." DSL_PIPE_CRLF);
            break;

            default:
            break;
         }

         return -1;
      }
      DSL_PIPE_Free(sempath);
   }

   if ( ((semid = semget(nsemkey, 1, 0666|IPC_CREAT|IPC_EXCL)) < 0) )
   {
      /* ERROR: check errno */
      if (errno == EEXIST)
      {
         /* get without create */
         if ( ((semid = semget(nsemkey, 1, 0666|IPC_EXCL)) < 0) )
         {
            printf( "DSL: create semaphore - semget(0x%X,0), errno=%d\n\r",
               nsemkey, errno );
            #ifndef DSL_DEBUG_DISABLE
            print_sem_error(errno);
            #endif
            return -1;
         }
      }
      else
      {
         #ifndef DSL_DEBUG_DISABLE
         print_sem_error(errno);
         #endif
         return -1;
      }
   }
   else
      arg.val = 1;

   if (arg.val == 1)
   {
      /* set the value of semaphore to 1 ie released or free to use */
      if (semctl(semid, 0, SETVAL, arg) < 0 )
      {
         printf( "DSL: create semaphore - semctl(0x%X,0), errno=%d\n\r",
            semid, errno );
         #ifndef DSL_DEBUG_DISABLE
         print_sem_error(errno);
         #endif
         return -1;
      }
   }

   return semid;
}

DSL_Error_t DSL_PIPE_SemaphoreGet(int sem, DSL_uint32_t nTimeout)
{
   struct sigaction sa;
   struct sembuf sb;
   struct timespec timeout;
   struct timespec *pTimeout = DSL_NULL;

   sb.sem_num = 0;
   /* specifies the operation ie to get the semaphore */
   sb.sem_op = -1;
   sb.sem_flg = SEM_UNDO;

   switch(nTimeout)
   {
      case 0xFFFFFFFF:
      /* Blocking call */
      break;

      case 0:
      /* Non Blocking */
      sb.sem_flg |= IPC_NOWAIT;
      break;

      default:
      /* Blocking call */
      /* Initialize timer expiration value */
      timeout.tv_sec        = (nTimeout/1000);
      timeout.tv_nsec       = (nTimeout%1000) * 1000 * 1000;

      pTimeout = &timeout;

      sa.sa_flags = 0;
      sa.sa_handler = DSL_PIPE_SemAlarm;
      sigaction(SIGALRM, &sa, DSL_NULL);
      alarm(timeout.tv_sec + 1);

      break;
   }

   /* Acquire semaphore */
   if (semop(sem, &sb, 1) == 0)
   {
      alarm(0);
      return DSL_SUCCESS;
   }
   else
   {
      alarm(0);
      if(errno == EINTR)
      {
         printf( "DSL: semaphore timeout." DSL_PIPE_CRLF);
         return DSL_ERR_TIMEOUT;
      }
      else
      {
         #ifndef DSL_DEBUG_DISABLE
         print_sem_error(errno);
         #endif
         printf( "DSL: get semaphore %x failed (errno=%d)" DSL_PIPE_CRLF, sem, errno);
         return DSL_ERROR;
      }
   }
}

DSL_Error_t DSL_PIPE_SemaphoreSet(int sem)
{
   struct sembuf sb;

   sb.sem_num = 0;
   /* specifies the operation ie to set the semaphore */
   sb.sem_op = 1;
   sb.sem_flg = SEM_UNDO;

   if (semop(sem, &sb, 1) == 0)
   {
      return DSL_SUCCESS;
   }
   else
   {
      #ifndef DSL_DEBUG_DISABLE
      print_sem_error(errno);
      #endif
      printf( "DSL: set semaphore %x failed (errno=%d)" DSL_PIPE_CRLF, sem, errno);
      return DSL_ERROR;
   }
}

/* 1 colon means there is a required parameter */
/* 2 colons means there is an optional parameter */
static const char GETOPT_LONG_OPTSTRING[] = "hve:";

int main(int argc, char **argv)
{
   char *pCommand = DSL_NULL;
   int i, k=0, len=0, ret;
   FILE *pipe_cmd, *pipe_ack;
   DSL_int8_t instance = 0;
   DSL_char_t nameBuf[64], *pChar;

   i = 1;
   pChar = argv[1];
   if (pChar)
   {
      if (pChar[0] == '-')
      {
         switch(pChar[1])
         {
         case 'h':
            printf(DSL_PIPE_CRLF);
            printf("Client for the pipe interface of the \"DSL CPE API\"" DSL_PIPE_CRLF);
            printf("\t%s [-hv] [pipe_no] cmd [params]" DSL_PIPE_CRLF, argv[0]);
            printf(DSL_PIPE_CRLF);
            printf("\t-h\tthis help" DSL_PIPE_CRLF);
            printf("\tpipe_no\tpipe number (default 0)" DSL_PIPE_CRLF);
            printf("\tcmd\tcommand to execute" DSL_PIPE_CRLF);
            printf("\tparams\tparameters of the command" DSL_PIPE_CRLF);
            printf(DSL_PIPE_CRLF);
            return 0;
         case 'v':
            printf("\tdsl_pipe_client v0.0.2" DSL_PIPE_CRLF);
            return 0;
         }
      }
      else if (isdigit(pChar[0]))
      {
         if (argc >= 3)
         {
            instance = atoi(argv[1]);
            if ((instance >= MAX_CLI_PIPES) || (instance < 0))
            {
               printf("\tpipe_no\toption requires parameter in range [0;%d]"
                      DSL_PIPE_CRLF, (MAX_CLI_PIPES - 1));
               return -1;
            }
            i = 2;
         }
      }

      k = i;
      for(;i<argc;i++)
      {
         len += strlen(argv[i]) + 1;
      }
   }
   else
   {
      len = 0;
   }


   if(len == 0)
   {
      pCommand = DSL_PIPE_Malloc(16);
      strcpy(pCommand, "help all");
   }
   else
   {
      pCommand = DSL_PIPE_Malloc(len + 1);

      if(pCommand == DSL_NULL)
      {
         printf( "Memory allocation failed" DSL_PIPE_CRLF);
         return -1;
      }

      pCommand[0] = 0;
      for(i=k;i<argc;i++)
      {
         strcat(pCommand, argv[i]);
         strcat(pCommand, " ");
      }
   }

   sema = DSL_PIPE_SemaphoreGlobalCreate("dsl_pipe");
   if (sema == -1)
   {
      printf( "dsl_pipe already running!" DSL_PIPE_CRLF);
      /* semaphore not created */
          DSL_PIPE_Free(pCommand);
      return -1;
   }

   if (DSL_PIPE_SemaphoreGet(sema, 100) != DSL_SUCCESS)
   {
      printf( "dsl_pipe already running!" DSL_PIPE_CRLF);
      printf( "Please delete /tmp/dsl_pipe to clean up "
         "if dsl_pipe is not running anymore!" DSL_PIPE_CRLF);
      DSL_PIPE_Free(pCommand);
          return -1;
   }

   sprintf (nameBuf, "%s%d_cmd", PIPE_PREFIX, instance);

   pipe_cmd = DSL_PIPE_FOpen(nameBuf, "w");
   if (pipe_cmd == DSL_NULL)
   {
      printf( "fopen %s failed (errno=%d)\r\n", nameBuf, errno);
      goto error;
   }

   /* execute command */
   DSL_PIPE_FPrintf(pipe_cmd, "%s\r\n", pCommand);
   DSL_PIPE_FClose(pipe_cmd);

   sprintf (nameBuf, "%s%d_ack", PIPE_PREFIX, instance);
   pipe_ack = DSL_PIPE_FOpen(nameBuf, "r");

   if (pipe_ack == DSL_NULL)
   {
      printf( "fdopen %s failed (errno=%d)" DSL_PIPE_CRLF, nameBuf, errno);
      goto error;
   }

   /* read answer */
   do
   {
      ret = DSL_PIPE_FRead(Result,1,sizeof(Result)-1,pipe_ack);
      if (ret>0)
      {
         Result[ret] = '\0';
         printf( "%s", Result);
      }
   }
   while ((ret > 0));

   DSL_PIPE_FClose(pipe_ack);

   DSL_PIPE_SemaphoreSet(sema);

   printf( DSL_PIPE_CRLF );

   DSL_PIPE_Free(pCommand);

   return 0;

error:
   DSL_PIPE_SemaphoreSet(sema);

   printf( "There were errors!" DSL_PIPE_CRLF);

   DSL_PIPE_Free(pCommand);

   return -1;
}

#endif /* LINUX */

