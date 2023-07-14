/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
 

/*** defines ***/

/*
 * returns hex value of key modified by ctrl key
 */
#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/

struct termios orig_termios;

/*** terminal ***/

/*
 * Name: die
 * 
 * Parameters:  char *s:  string to print to assist error description
 *  
 * Description: prints passed string followed by descriptive error message 
 * read from variable errno, then exits program with non-zero status. 
 */
void die(const char *s){
  perror(s);
  exit(1);
}

/*
 * Name: disableRawMode
 * 
 * Description: Sets terminal back to normal mode. Undoes attributes changed by
 * enableRawMode.
 */
void disableRawMode(){
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) 
    die("tcsetattr");
}

/*
 * Name:  enableRawMode
 *
 * Description: configures the terminal to be in raw mode. removing as much 
 * processing done by the terminal as possible. Additonally registers
 * disableRawMode to be exucted on program exit. 
 */
void enableRawMode() {
 
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    die("tcgetattr");

  //atexit: comes from <stdlib.h>. Registers function to be called 
  //automatically when the program exits.
  atexit(disableRawMode);

  struct termios raw = orig_termios;
  /*
   * Note: the flags that were disabled and what they did.
   * 
   * ICRNL: ctrl-m and enter modified to not clash with CR. 
   * IXON:  software control signals ctrl-s/-q.
   * OPOST: ouput processing. e.g., \n results in \r\n.
   * ECHO:  echo input to terminal.    
   * ICANON:read input line by line. 
   * IEXTEN:ctrl-v.
   * ISIG:  signal inputs from application. ctrl-c/-z.
   *  
   * Misc flags:  BRKINT, INPCK, ISTRIP, and CS8.
   */

  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag &= ~(CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
 
  raw.c_cc[VMIN] = 0; //min number of input bytes before read() returns.
  raw.c_cc[VTIME] = 1;//max time read() waits before returning.

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}

char editorReadKey(){
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1){
    if (nread == -1 && errno != EAGAIN){
      die("read"); 
    } 
  }
  return c;
}

/*** output ***/

void editorDrawRows(){
  int y;
  for (y = 0; y < 24; y++){
    write(STDOUT_FILENO, "~\r\n", 3);
  }
}

void editorRefreshScreen(){
  /*
   * write() and STDOUT_FILENO come from <unistd.h>
   * we write 4 bytes out to terminal. \1xb (escape char) followed by [ char.
   * 2J is the J command (erase) with argument value 2 (whole screen).
   */
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  editorDrawRows();

  write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** input ***/

void editorProcessKeypress(){
  char c = editorReadKey();

  switch (c) {
    case CTRL_KEY('q'):
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
      break;
  }
}

/*** init ***/

int main(){ 
  enableRawMode(); 
  
  while (1){
    editorRefreshScreen();
    editorProcessKeypress();
  }
  
  return 0;
}
