#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define BUFFER_SIZE 4096

__inline int MyErr( char *str_err ) __attribute__((always_inline));

int MyErr( char *str_err )
{
  perror(str_err);
  return errno;
}

int MyWrite( int fd, void *buffer, size_t buf_size )
{
  size_t bytes_written = 0;

  while (bytes_written < buf_size)
  {
    int write_ret = write(fd, buffer, buf_size - bytes_written);

    if (write_ret < 0)
      return MyErr("Error with writing to stdout");

    bytes_written += write_ret;
    buffer += write_ret;
  }

  return 0;
}

int PrintFile( int fd )
{
  int bytes_read = 0;

  do
  {
    char buffer[BUFFER_SIZE];

    bytes_read = read(fd, buffer, BUFFER_SIZE);
    if (bytes_read < 0)
      return MyErr("Error with reading file");

    if (MyWrite(STDOUT_FILENO, buffer, bytes_read))
      return 0;

  } while (bytes_read != 0);

  return 1;
}


int main( int argc, char * argv[] )
{
  if (argc == 1 || argv[1][0] == '-')
  {
    if (!PrintFile(STDIN_FILENO))
      return 1;
    return 0;
  }

  for (int i = 1; i < argc; ++i)
  {
    int fd = open(argv[i], O_RDONLY);
    if (fd < 0)
    {
      MyErr(argv[i]);
      continue;
    }

    if (!PrintFile(fd))
      return 1;

    if (close(fd) < 0)
      return MyErr(argv[i]);
  }

  return 0;
}
