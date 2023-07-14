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


  //Timeout read()
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}

/*** init ***/

int main(){
  enableRawMode();
  
  
  while (1){
    char c = '\0';
    //CHECK errno != EAGAIN as Cygwin returns -1 when read times out
    //  with errno of EAGAIN
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
      die("read"); 
    // iscntrl() test if char is (non printable) control char.
    if(iscntrl(c)) {
      printf("%d\n", c);
    } else {
      printf("%d ('%c')\n", c, c);
    }
    if(c == CTRL_KEY('q')) break;
  }
  
  return 0;

}
