#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
// #include <stdio.h> 

struct termios orig_termios;

void disableRawMode(){	
  //TCSAFLUSH will discard any unread input before applying changes.
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
 
  tcgetattr(STDIN_FILENO, &orig_termios);  
  //atext comes from <stdlib.h>.
  //use it to register our function to be called automatically when the
  //program exits.
  atexit(disableRawMode);

  struct termios raw = orig_termios;
  //FLAGSET = FLAGS AND NOT (X Y Z)
  //ECHO FLAG: print input to terminal
  //ICANNON FLAG: read line by line not byte by byte
  raw.c_lflag &= ~(ECHO | ICANON);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main(){
  enableRawMode();
  
  char c;
  while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q'){
    // iscntrl() test if char is (non printable) control char.
    if(iscntrl(c)) {
      printf("%d\n", c);
    } else {
      printf("%d ('%c')\n", c, c);
    }      
  }
  
  return 0;

}
