#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
// #include <stdio.h> 

struct termios orig_termios;


void die(const char *s){
  //PERROR  first prints string passed to it. then looks at errno var and prints
  //        a descriptive err msg. most c libs funcs that fail set errno
  perror(s);
  //exit program with status 1 (indicated failure)
  exit(1);
}

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
  //ICRNL FLAG: carriage return new line flag ctrl-m now 13 
  //IXON FLAG: suspend software control signals ctrl-s/-q
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  //OPOST FLAG: output processing 
  // e.g. by default it a `\n` new line is processed as a `\r\n` so the new line
  //      begins at start of next line 
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag &= ~(CS8);
  //ECHO FLAG: print input to terminal
  //ICANNON FLAG: read line by line not byte by byte
  //IEXTEN FLAG: disable ctrl-v 
  //ISIS FLAG: suspend ctrl-c/-z sending signal inputs
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  //Miscellaneous flags
  //  wont have any obserbale effect as they are already or turned off or dont 
  //  apply to modern term emnulators. but used to be considered to be apart of
  //  enabling raw mode.
  //BRKINT, INPCK, ISTRIP, and CS8 all come from <termios.h>.

  //TIMEOUT READ()
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main(){
  enableRawMode();
  
  
  while (1){
    char c = '\0';
    read(STDIN_FILENO, &c, 1); 
    // iscntrl() test if char is (non printable) control char.
    if(iscntrl(c)) {
      printf("%d\n", c);
    } else {
      printf("%d ('%c')\n", c, c);
    }
    if(c == 'q') break;
  }
  
  return 0;

}
