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
  //PERROR: first prints string passed to it. then looks at errno var and prints
  //        a descriptive err msg. most c libs funcs that fail set errno
  perror(s);
  //EXIT:   exit program with status 1 (indicated failure)
  exit(1);
}

/*
 * Name: disableRawMode
 * 
 * Description: Sets terminal back to normal mode. Undoes attributes changed by
 * enableRawMode.
 */
void disableRawMode(){	
  //TCSAFLUSH:  will discard any unread input before applying changes.
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
  //ATEXIT: comes from <stdlib.h>.
  //        use it to register our function to be called automatically when the
  //        program exits.
  atexit(disableRawMode);

  struct termios raw = orig_termios;
  //ASSIGNMENT EXPRESSION: flags = set flags and not (bit x,y,z)
  //ICRNL FLAG:   carriage return new line flag ctrl-m now 13 
  //IXON FLAG:    suspend software control signals ctrl-s/-q
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  //OPOST FLAG:   output processing   e.g. by default it a `\n` new line is
  //              processed as a `\r\n` so the new line begins at start. 
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag &= ~(CS8);
  //ECHO FLAG:    print input to terminal
  //ICANON FLAG:  read line by line not byte by byte
  //IEXTEN FLAG:  disable ctrl-v 
  //ISIS FLAG:    suspend ctrl-c/-z sending signal inputs
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  //Notes on miscellaneous flags
  //  wont have any obserbale effect as they are already or turned off or dont 
  //  apply to modern term emnulators. but used to be considered to be apart of
  //  enabling raw mode.
  //BRKINT, INPCK, ISTRIP, and CS8 all come from <termios.h>.

  //TIMEOUT READ()
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
