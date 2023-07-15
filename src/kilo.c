/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
 

/*** defines ***/


/*
 * returns hex value of key modified by ctrl key
 */
#define CTRL_KEY(k) ((k) & 0x1f)


/*** data ***/


struct editorConfig {
  int screenrows;
  int screencols;
  struct termios orig_termios;
};

struct editorConfig E;


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
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) 
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
 
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
    die("tcgetattr");

  //atexit: comes from <stdlib.h>. Registers function to be called 
  //automatically when the program exits.
  atexit(disableRawMode);

  struct termios raw = E.orig_termios;
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

/*
 * Name: editorReadyKey
 *
 * Description:   
 * 
 */
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

/*
 * Name: getCursorPosition
 *
 * Description: returns cursors row and column postion. used to calculate the
 * terminal size manually if cannot be done any other way. 
 */
int getCursorPosition(int *rows, int *cols) {
  char buf[32];
  unsigned int i = 0;

  if(write(STDOUT_FILENO, "\x1b[6n]", 4) != 4) return -1;

  while (i < sizeof(buf) -1 ) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1){
      break;
    } 
    if (buf[i] == 'R') {
      break;
    } 
    i++;
  }
  buf[i] = '\0';

  if (buf[0] != '\x1b' || buf[1] != '[') return -1;
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;
  return 0;
}

/*
 * Name: getWindowSize
 *
 * Description: gets the number of columns and rows visible in the terminal
 * window.
 */
int getWindowSize(int *rows, int *cols) {
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12){
      return -1;
    } 
    return getCursorPosition(rows, cols); 
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}
/*** append buffer ***/

struct abuf{
  char *b;
  int len;
};

#define ABUF_INIT {NULL, 0}

void abAppend(struct abuf *ab, const char *s, int len){

}

/*** output ***/

/*
 * Name: editorDrawRows
 * 
 * Description: Draws the tilde at begining of each row.  
 */
void editorDrawRows(){
  int y;
  for (y = 0; y < E.screenrows-1; y++){
    write(STDOUT_FILENO, "~\r\n", 3);
  }
  write(STDOUT_FILENO, "~", 1);
}

/*
 * Name: editorRefreshScreen
 * 
 * Description: 
 */
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

void initEditor(){
  if (getWindowSize(&E.screenrows, &E.screencols) == -1 ) die("getWindowSize");
}

int main(){ 
  enableRawMode(); 
  initEditor();

  while (1){
    editorRefreshScreen();
    editorProcessKeypress();
  }
  
  return 0;
}
