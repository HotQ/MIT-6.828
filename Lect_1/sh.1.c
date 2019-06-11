#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#include "logging.h"

// Simplifed xv6 shell.

#define MAXARGS 10
#define OUT
#define IN
#define INOUT

#define _E(chr) (chr == 0 ? '$' : chr)
// All commands have at least a type. Have looked at the type, the code
// typically casts the *cmd to some specific cmd type.
struct cmd {
  int type;          //  ' ' (exec), | (pipe), '<' or '>' for redirection
};

struct execcmd {
  int type;              // ' '
  char *argv[MAXARGS];   // arguments to the command to be exec-ed
};

struct redircmd {
  int type;          // < or > 
  struct cmd *cmd;   // the command to be run (e.g., an execcmd)
  char *file;        // the input/output file
  int mode;          // the mode to open the file with
  int fd;            // the file descriptor number to use for the file
};

struct pipecmd {
  int type;          // |
  struct cmd *left;  // left side of pipe
  struct cmd *right; // right side of pipe
};

int fork1(void);  // Fork but exits on failure.
struct cmd *parsecmd(char*);

char shellpath[128];
static char *bufbeg;

// Execute cmd.  Never returns.
void
runcmd(struct cmd *cmd)
{
  int p[2], r;
  struct execcmd *ecmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;
  debug("┌runcmd() start with \e[1;7m %c \e[m type!", cmd->type);
  if(cmd == 0)
    exit(0);
  
  switch(cmd->type){
  default:
    error("unknown runcmd");
    exit(-1);

  case ' ':
    ecmd = (struct execcmd*)cmd;
    info("│   '%s'",ecmd->argv[0]);
    info("│   sizeof(execcmd) = %lu/%lu/%lu!",sizeof(*cmd),sizeof(*ecmd),sizeof(struct execcmd));
    if(ecmd->argv[0] == 0)
      exit(0);
    int err = execvp(ecmd->argv[0], ecmd->argv);
    error("│   '%s' went wrong,errno = %d: %s", ecmd->argv[0],errno,strerror(errno) );
    break;

  case '>':
  case '<':
    rcmd = (struct redircmd*)cmd;
    info("rcmd->fd %d,mod = %0x",rcmd->fd,rcmd->mode);
    close(rcmd->fd);
    int fd = open(rcmd->file,rcmd->mode, S_IRWXU);
    if(fd<0)
      fatal("fd = %d, error:%d %s",fd,errno,strerror(errno));
    runcmd(rcmd->cmd);

    break;

  case '|':
    pcmd = (struct pipecmd*)cmd;
    pipe(p);
    if(fork1()==0)
    {
      info("      \e[1mpipe right==>\e[m");
      close(0);
      dup(p[0]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->right);
    }
    if(fork1()==0)
    {
      info("      \e[1m<==pipe left\e[m");
      close(1);
      dup(p[1]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->left);
    }
    close(p[0]);
    close(p[1]);
    wait(&r);
    wait(&r);
  
    break;
  }    
  debug("└runcmd() stop with \e[1;7m %c \e[m type!", cmd->type);
  exit(0);
}

int
getcmd(char *buf, char *hostbuf, int nbuf)
{
  gethostname(hostbuf,100);
  size_t _offset = strlen(hostbuf);
  getcwd(hostbuf + _offset, 101 - _offset);
  if (isatty(fileno(stdin)))
    fprintf(stdout, "\e[4m%s$\e[m ",hostbuf);
  memset(buf, 0, nbuf);
  fgets(buf, nbuf, stdin);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

int
main(void)
{
  extern int log_output_level;
  log_output_level = -1;

  static char buf[100],
              hostbuf[100];
  int fd, r;
  
  //save the path of the shell
  getcwd(shellpath, sizeof(shellpath));
  bufbeg = buf;
  // Read and run input commands.
  while(getcmd(buf, hostbuf, sizeof(buf)) >= 0){
    if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
      // Clumsy but will have to do for now.
      // Chdir has no effect on the parent if run in the child.
      buf[strlen(buf)-1] = 0;  // chop \n
      if(chdir(buf+3) < 0)
        error("cannot cd %s", buf+3);
      continue;
    }
    notset("buf = '%s'",buf);
    if(fork1() == 0)
      runcmd(parsecmd(buf));
    wait(&r);
  }
  exit(0);
}

int
fork1(void)
{
  int pid;
  
  pid = fork();
  if(pid == -1)
    perror("fork");
  return pid;
}

struct cmd*
execcmd(void)
{
  struct execcmd *cmd;
  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = ' ';
  return (struct cmd*)cmd;
}

struct cmd*
redircmd(struct cmd *subcmd, char *file, int type)
{
  struct redircmd *cmd;
  debug("     ┌redirccmd() start! '%c'",type);
  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = type;
  cmd->cmd = subcmd;
  cmd->file = file;
  cmd->mode = (type == '<') ?  O_RDONLY : O_WRONLY|O_CREAT|O_TRUNC;
  cmd->fd = (type == '<') ? 0 : 1;
  debug("     └redirccmd() stop!");
  return (struct cmd*)cmd;
}

struct cmd*
pipecmd(struct cmd *left, struct cmd *right)
{
  struct pipecmd *cmd;
  debug("   pipecmd()");
  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = '|';
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

// Parsing

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>";

int
gettoken(IN char **ps, IN char *es, OUT char **q, OUT char **eq)
{
  char *s;
  int ret;
  debug("     ┌gettoken() [%d]'%c'",*ps-bufbeg, _E(**ps));
  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  if(q)
    *q = s;
  ret = *s;
  switch(*s){
  case 0:
    break;
  case '|':
  case '<':
    s++;
    break;
  case '>':
    s++;
    break;
  default:
    ret = 'a';
    while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
      s++;
    break;
  }
  if(eq)
    *eq = s;
  
  debug("     │here");

  while(s < es && strchr(whitespace, *s)){
    s++;
  }
  debug("     │here [%d]'%c'",*ps-bufbeg, _E(**ps));

  *ps = s;
  debug("     └ret '%c'",_E(ret));
  return ret;
}

int
peek(INOUT char **ps, IN char *es, IN char *toks)
{
  char *s;
  
  s = *ps;
  debug("\e[1;37m     ┌hi peek\e[m([\e[1;7m %s \e[m%ld]\e[1;7m %c \e[m\t[%ld]\e[1;7m %c \e[m toks!", toks,*ps-bufbeg, _E(**ps), es-bufbeg, _E(*es));

  while(s < es && strchr(whitespace, *s)){
    debug("     │   matched  [%ld]\e[1;7m %c \e[m!", s-bufbeg, _E(*s));
    s++;
  }
  *ps = s;
  
  int ret =  *s && strchr(toks, *s); // not EOF and 
  debug("\e[1;37m     └pointer\e[m moved to [%ld]\e[1;7m %c \e[m, ret = %d", *ps-bufbeg, _E(**ps), ret);

  return ret;
}

struct cmd *parseline(char**, char*);
struct cmd *parsepipe(char**, char*);
struct cmd *parseexec(char**, char*);

// make a copy of the characters in the input buffer, starting from s through es.
// null-terminate the copy to make it a string.
char 
*mkcopy(char *s, char *es)
{
  int n = es - s;
  char *c = malloc(n+1);
  assert(c);
  strncpy(c, s, n);
  c[n] = 0;
  return c;
}

struct cmd*
parsecmd(char *s)
{
  char *es;
  struct cmd *cmd;

  es = s + strlen(s);
  debug("\e[1;34m┌─────parsecmd()\e[m start  [%ld]\e[1;7m %c \e[m [%ld]\e[1;7m %c \e[m!",s-bufbeg, _E(*s), es-bufbeg,_E(*es) );
  cmd = parseline(&s, es);
  peek(&s, es, "");
  
  debug("\e[1;34m└─────parsecmd()\e[m stop with [%ld]\e[1;7m %c \e[m\t[%ld]\e[1;7m %c \e[m!", s-bufbeg, _E(*s), es-bufbeg, _E(*es) );

  if(s != es){
    error("leftovers: %s", s);
    exit(-1);
  }
  return cmd;
}

struct cmd*
parseline(char **ps, char *es)
{
  struct cmd *cmd;
  debug("\e[1;33m ┌───parseline()\e[m start with [%ld]\e[1;7m %c \e[m!\t[%ld]\e[1;7m %c \e[m", *ps-bufbeg, _E(**ps), es-bufbeg, _E(*es));
  cmd = parsepipe(ps, es);
  debug("\e[1;33m └───parseline()\e[m stop with [%ld]\e[1;7m %c \e[m!\t[%ld]", *ps-bufbeg, _E(**ps), es-bufbeg, _E(*es));
  return cmd;
}

struct cmd*
parsepipe(char **ps, char *es)
{
  struct cmd *cmd;
  debug("\e[1;31m    ┌─parsepipe()\e[m start with [%ld]\e[1;7m %c \e[m\t[%ld]\e[1;7m %c \e[m!", *ps-bufbeg, _E(**ps), es-bufbeg, _E(*es));
  cmd = parseexec(ps, es);
  if(peek(ps, es, "|")){
    gettoken(ps, es, 0, 0);
    debug("┌parsepipe() sub in");
    cmd = pipecmd(cmd, parsepipe(ps, es));
    debug("└parsepipe() sub out");
  }
  debug("\e[1;31m    └─parsepipe()\e[m stop with [%ld]\e[1;7m %c \e[m\t[%ld]\e[1;7m %c \e[m!", *ps-bufbeg, _E(**ps), es-bufbeg, _E(*es));
  return cmd;
}

struct cmd*
parseredirs(struct cmd *cmd, char **ps, char *es)
{
  int tok;
  char *q, *eq;
  debug("\e[1;36m    ┌parseredirs()\e[m start with [%ld]\e[1;7m %c \e[m\t[%ld]\e[1;7m %c \e[m!", *ps-bufbeg, _E(**ps), es-bufbeg, _E(*es) );
  while(peek(ps, es, "<>")){
    tok = gettoken(ps, es, 0, 0);
    debug("      tok = '%c'",_E(tok));
    if(gettoken(ps, es, &q, &eq) != 'a') {
      error("missing file for redirection");
      exit(-1);
    }
    switch(tok){
    case '<':
      cmd = redircmd(cmd, mkcopy(q, eq), '<');
      break;
    case '>':
      cmd = redircmd(cmd, mkcopy(q, eq), '>');
      break;
    }
  }
  debug("\e[1;36m    └parseredirs()\e[m stop with [%ld]\e[1;7m %c \e[m\t[%ld]\e[1;7m %c \e[m!", *ps-bufbeg, _E(**ps), es-bufbeg, _E(*es) );
  return cmd;
}

struct cmd*
parseexec(char **ps, char *es)
{
  char *q, *eq;
  int tok, argc;
  struct execcmd *cmd;
  struct cmd *ret;
  debug("\e[1;35m   ┌parseexec()\e[m start with [%ld]\e[1;7m %c \e[m\t[%ld]\e[1;7m %c \e[m!", *ps-bufbeg, _E(**ps), es-bufbeg, _E(*es));
  ret = execcmd();
  cmd = (struct execcmd*)ret;

  argc = 0;
  ret = parseredirs(ret, ps, es);
  while(!peek(ps, es, "|")){
    if((tok=gettoken(ps, es, &q, &eq)) == 0)
      break;
    if(tok != 'a') {
      error("syntax error");
      exit(-1);
    }
    cmd->argv[argc] = mkcopy(q, eq);
    argc++;
    if(argc >= MAXARGS) {
      error("too many args");
      exit(-1);
    }
    ret = parseredirs(ret, ps, es);
  }
  debug("\e[1;35m   └parseexec()\e[m stop with [%ld]\e[1;7m %c \e[m\t[%ld]\e[1;7m %c \e[m!", *ps-bufbeg, _E(**ps), es-bufbeg, _E(*es));
  cmd->argv[argc] = 0;
  return ret;
}
