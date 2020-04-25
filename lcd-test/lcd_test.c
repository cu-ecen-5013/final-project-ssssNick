// Description: Tests LCD driver module
//
// Author: Nick Brubaker
//
// References://    Checking for sucessful read - LSP textbook

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{

  // Setup logging
  openlog("lcd_test.c", LOG_ODELAY, LOG_USER);

  char *writeFile = "/dev/lcd"; // device node
  char *writeStr; // String to write

  // Create file
  int fd = open(writeFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);

  // Check if device node was opened sucessfully
  if(fd == -1)
  {
    syslog(LOG_ERR, "error: couldn't open device");
    close(fd);
    return -1;
  }

  //Test writing and seeking on LCD

  writeStr = "Writing from beginning";
  write(fd, writeStr, strlen(writeStr));

  writeStr = "\nWriting some more";
  write(fd, writeStr, strlen(writeStr));

  close(fd);
  return 0;

}
