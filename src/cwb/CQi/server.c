/*
 *  IMS Open Corpus Workbench (CWB)
 *  Copyright (C) 1993-2006 by IMS, University of Stuttgart
 *  Copyright (C) 2007-     by the respective contributers (see file AUTHORS)
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2, or (at your option) any later
 *  version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 *  Public License for more details (in the file "COPYING", or available via
 *  WWW at http://www.gnu.org/copyleft/gpl.html).
 */
#ifdef __MINGW__
#include <winsock2.h>
#define socklen_t int
#endif
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/time.h>

#ifndef __MINGW__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif


#include "../cl/cl.h"           /* this is the local cl.h header */

#include "../cqp/options.h"
#include "../cqp/corpmanag.h"
#include "../cqp/parse_actions.h"

#include "server.h"
#include "auth.h"
#include "log.h"
#include "cqi.h"

/** the att hash is initially sized to 2 to the power of 14 */
#define ATTHASHSIZE 16384

void Rprintf(const char *, ...);

/** Error strings are limited to this many bytes. @see cqi_error_string */
#define GENERAL_ERROR_SIZE 1024

#ifndef MSG_WAITALL
/* Linux doesn't define the MSG_WAITALL flag (ditto MinGW), but under normal conditions
   it _does_ wait for the entire amount of data requested to arrive; so we
   just set MSG_WAITALL to 0 (nothing) in this case */
#define MSG_WAITALL 0
#endif

/*
 *
 * Global variables for CQPSERVER
 *
 */

int sockfd;                       /**< Connection in:  file-descriptor integer */
int connfd;                       /**< Connection out: file-descriptor integer */
FILE *conn_out;                   /**< Connection out: stream for buffered output (don't forget to flush()) */
struct sockaddr_in my_addr, client_addr;
struct hostent *remote_host;
char *remote_address;


int cqi_errno = CQI_STATUS_OK;    /**< CQi last error */

/** String describing the last CQi error. This can be queried by the client. */
char cqi_error_string[GENERAL_ERROR_SIZE] = "No error.";



/*
 *
 *  Error messages
 *
 */

/**
 * Prints an error message to cqpserver's STDERR and then exits.
 *
 * This function reports an error in SENDING data to the outgoing connection.
 *
 * @param function  String containing the name of the function where the error was raised.
 */
void
cqi_send_error(char *function)
{
#ifndef R_PACKAGE
  cqiserver_log(Error, "ERROR CQi data send failure in function\n\t%s() <server.c>", function);
  exit(1);
#else
  Rf_error("ERROR CQi data send failure in function\n\t%s() <server.c>", function);
#endif
}

/**
 * Prints an error message to cqpserver's STDERR and then exits.
 *
 * This function reports an error in READING data from the incoming connection.
 *
 * @param function  String containing the name of the function where the error was raised.
 */
void
cqi_recv_error(char *function)
{
#ifndef R_PACKAGE
  cqiserver_log(Error, "ERROR CQi data recv failure in function\n\t%s() <server.c>\n", function);
  exit(1);
#else
  Rf_error("ERROR CQi data recv failure in function\n\t%s() <server.c>\n", function);
#endif
}

/**
 * Prints an error message to cqpserver's STDERR and then exits.
 *
 * This function reports a (miscellanrous) internal error in a cqpserver function.
 *
 * @param function  String containing the name of the function where the error was raised.
 * @param cause     String containing a description of what caused the error.
 */
void
cqi_internal_error(char *function, char *cause)
{
#ifndef R_PACKAGE
  cqiserver_log(Error, "ERROR Internal error in function\n\t%s() <server.c>\n\t''%s''", function, cause);
  exit(1);
#else
  Rf_error("ERROR Internal error in function\n\t%s() <server.c>\n\t''%s''", function, cause);
#endif
}


/**
 * General error reporting function.
 *
 * Note that unlike other CQi error functions, this function sends an error
 * message to the outgoing connection, rather than printing to the server's
 * STDERR. Also note that the program doesn't exit!
 *
 * The error message is placed into the global variable cqi_error_string,
 * whence it can be accessed by the client if the CQI_CTRL_LAST_GENERAL_ERROR
 * is sent to the server.
 *
 * @param errstring  String containing the error message.
 */
void
cqi_general_error(char *errstring)
{
  if (strlen(errstring) >= GENERAL_ERROR_SIZE)
    cqi_internal_error("cqi_general_error", "Error message too long.");
  strcpy(cqi_error_string, errstring);
  cqi_command(CQI_ERROR_GENERAL_ERROR);
}

/*
 *
 *  Connection
 *
 */

/**
 * Wait for, and then process, an attempt by a client to initiate a connection
 * to cqpserver via TCP/IP.
 *
 * Note that this function may or may not fork the cqpserver process.
 *
 * If forking happens, then the child handles the connection,
 * whereas the parent carries on waiting for further connections.
 *
 * On Windows, forking never happens (since Windows doesn't support it).
 *
 * On *nix, forking happens UNLESS the global private_server is true.
 * (Actually, if private_server is true, then forking still happens,
 * but the parent process immeidately exits.)
 *
 * @param port  The integer identifier of the port to listen on.
 * @return      A > 0 value ( the socket ID of the incoming connection)
 *              if all is OK; otherwise -1.
 */
int
accept_connection(int port)
{
  const int on = 1;
  socklen_t sin_size = sizeof(struct sockaddr_in);
#ifndef __MINGW__
  pid_t child_pid;
#endif

#ifndef __MINGW__
  if (SIG_ERR == signal(SIGCHLD, SIG_IGN)) {
#ifndef R_PACKAGE
    perror("ERROR Can't ignore SIGCHLD");
    exit(1);
#else
    Rf_error("ERROR Can't ignore SIGCHLD");
#endif
  }
#endif

  if (port <= 0)
    port = CQI_PORT;

  cqiserver_debug_msg("Opening socket and binding to port %d", port);

#ifdef __MINGW__
  /* startup the use of the Winsock DLL */
  WORD wVersionRequested;
  WSADATA wsaData;
  int err;
  wVersionRequested = MAKEWORD(2, 2); /* 2.2 is the higher version */
  err = WSAStartup(wVersionRequested, &wsaData);

  if (err != 0) {
    char buffer[50];
    snprintf(buffer, 50, "ERROR WSAStartup failed with error: %d\n",err);
    perror(buffer);
    return -1;
  }
#endif

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

#ifndef __MINGW__
  if (sockfd < 0) {
#else
  if (sockfd == INVALID_SOCKET) {
#endif
    perror("ERROR Can't create socket");
    return -1;
  }
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&on, sizeof(int)) < 0)
    perror("WARNING Can't set address reuse option"); /* can be ignored... */

  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(port);
  if (localhost)
    my_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); /* loopback device */
  else
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);      /* all network devices on local machine */
  memset(&(my_addr.sin_zero), '\0', 8);
  if (0 != bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr))) {
    perror("ERROR Can't bind socket to port");
    return -1;
  }

  cqiserver_log(Info, "Waiting for client on port #%d.\n", port);
  if (0 != listen(sockfd, 5)) {
    perror("ERROR listen() failed");
    return -1;
  }

#ifndef __MINGW__
  /* if called with '-q', fork() and quit before waiting for connections */
  if (server_quit) {
    pid_t pid = fork();
    if (pid != 0) {
      /* parent returns to caller */
      close(sockfd);
#ifndef R_PACKAGE
      exit(cqiserver_log(Info, "[child is running in background now, parent server quits]") || cqp_error_status);
#else
      Rf_error("[child is running in background now, parent server quits]");
#endif
    }
  }
#else
  /* no forking in Windows! */
#endif

  while (42) {
    /* when run as a private server, we'll only wait for up to 10 seconds */
    if (private_server) {
      struct timeval tv;
      fd_set read_fd;

      tv.tv_sec = 10;
      tv.tv_usec = 0;
      FD_ZERO(&read_fd);        /* select() on sockfd */
      FD_SET(sockfd, &read_fd);

      if (0 >= select(sockfd+1, &read_fd, NULL, NULL, &tv) || !FD_ISSET(sockfd, &read_fd))
#ifndef R_PACKAGE
        exit(cqiserver_log(Error, "Port #%d timed out in private server mode. Aborting.", port) || cqp_error_status);
#else
      Rf_error("Port #%d timed out in private server mode. Aborting.", port);
#endif      
      
    }

    connfd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
    if (connfd < 0) {
      perror("ERROR Can't establish connection");
      return -1;
    }

    cqiserver_debug_msg("Connection established. Looking up client's name.");
    remote_address = inet_ntoa(client_addr.sin_addr);
    remote_host = gethostbyaddr((void *)&(client_addr.sin_addr), 4, AF_INET);
    cqiserver_log(Info, "Connection established with %s (%s)", remote_address, remote_host ? remote_host->h_name : "name unknown");

#ifndef __MINGW__
    /* spawn a server to handle the request */
    child_pid = fork();
    if (child_pid < 0) {
      perror("ERROR can't fork() server");
      return -1;
    }

    if (0 == child_pid)
      break;                    /* the child exits the listen() loop */

    /* this is the listening 'parent', which exits immediately in private_server mode */
    cqiserver_log(Info, "Spawned child CQPserver, pid = %d.", (int)child_pid);
    close(connfd);              /* this is the child's connection */

    if (private_server) {
      close(sockfd);
#ifndef R_PACKAGE
      exit(cqiserver_log(Info, "Accepting no more connections (private server).") || cqp_error_status);
#else
      Rf_error("Accepting no more connections (private server).");
#endif
      /* SIGCHLD should be reaped by calling process */
    }
#else
    /* no forking in Windows!
     * So all we can do here is break the loop, assuming we are a child.
     *
     * Note this means: the Windows version of cqpserver accepts only one client!
     * (see here for some discussion:
     *      http://www.gamedev.net/community/forums/topic.asp?topic_id=360290
     *      http://stackoverflow.com/questions/985281/what-is-the-closest-thing-windows-has-to-fork
     *      )
     * We print the "private server" message unconditionally.
     */
    cqiserver_log(Info, "Accepting no more connections (private server).\n");
    break;
#endif
  }/* endwhile true */

  /* this is the child serving the new CQi connection */
  cqiserver_debug_msg("** new CQPserver created, initiating CQi session");
  close(sockfd);

  /* check if remote host is in validation list */
  if (!check_host(client_addr.sin_addr)) {
#ifndef R_PACKAGE
    cqiserver_log(Info, "WARNING %s not in list, connection refused!\n", remote_address);
    cqiserver_log(Info, "Exit. (pid = %d)\n", (int)getpid());
    close(connfd);
    exit(1);
#else 
    close(connfd);
    Rprintf("WARNING %s not in list, connection refused!\n", remote_address);
    Rf_error("Exit. (pid = %d)\n", (int)getpid());
#endif
  }

#ifndef __MINGW__
  /* create a buffered stream to the outgoing connection */
  if (!(conn_out = fdopen(connfd, "w"))) {
    perror("ERROR Can't switch CQi connection to buffered output");
    close(connfd);
    return -1;
  }
#endif
  cqiserver_debug_msg("creating attribute hash (size = %d)", ATTHASHSIZE);
  make_attribute_hash(ATTHASHSIZE);

  return connfd;
}




/*
 *
 *  Server -> Client communication
 *
 */

/* communication primitives (no auto-flush; use cqi_flush()) */

/**
 * Flushes the stream from the CQP server to the client program,
 * emptying its buffer.
 *
 * Note that under windows, buffered output is not possible, so
 * this function does nothing.
 *
 * @return  Boolean: true if everything OK, otherwise false.
 */
int
cqi_flush(void)
{
#ifdef __MINGW__
  return 1;
#else
  cqiserver_snoop("FLUSH");
  if (EOF == fflush(conn_out)) {
    perror("ERROR cqi_flush()");
    return 0;
  }
  return 1;
#endif
}

/**
 * Sends a BYTE to the client.
 *
 * This function should be called via one of the cqi_data_* functions
 * and not on its own.
 *
 * This is the fundamental "sending" function, and the only one that calls the underlying
 * OS-specific file stream/socket functions.
 *
 * @param n        The byte to send. NOTE that as the parameter is an int, numbers bigger than
 *                 0xff can be passed. BUT all content except the lowest-order 8-bits are discarded
 *                 (0xff is used as a mask with bitwise-and).
 * @param nosnoop  Boolean: if true, snoop functionality is overridden (to allow for non-repetition
 *                 of messages when called from a function that has already printed a message)
 */
int
cqi_send_byte(int n, int nosnoop)
{
#ifdef __MINGW__
  const char prep = (const char) 0xff & n;
  /* prep = (unsigned char) 0xff & n; */
#endif

  if (!nosnoop)
    cqiserver_snoop("SEND BYTE   %02X        [= %d]", n, n);

  /* note that the actual sending is wrapped in an "if" whose content differs between OSes */
  if (
#ifndef __MINGW__
      EOF == putc(0xff & n, conn_out)
#else
      1 != send(connfd, &prep, 1, MSG_WAITALL)
#endif
      )  {
    perror("ERROR cqi_send_byte()");
    return 0;
  }
  return 1;
}

/**
 * Sends a WORD to the client.
 *
 * A word consists of two bytes in network order. Since CQi commands are
 * two bytes, this function can be used to send commands.
 *
 * This function should be called via one of the cqi_data_* functions
 * and not on its own.
 */
int
cqi_send_word(int n)
{
  cqiserver_snoop("SEND WORD   %04X      [= %d]", n, n);
  if (
      /* exploit the fact that cqi_send_byte() only uses the lowest 8 bytes of its argument */
      !(cqi_send_byte(n>>8, 1)) ||
      !(cqi_send_byte(n, 1))
      ) {
    perror("ERROR cqi_send_word()");
    return 0;
  }
  return 1;
}

/**
 * Sends an INT to the client.
 *
 * An int consists of four bytes in network order.
 *
 * This function should be called via one of the cqi_data_* functions
 * and not on its own.
 *
 * @return  Boolean: true if everything OK, otherwise false.
 */
int
cqi_send_int(int n)
{
  cqiserver_snoop("SEND INT    %08X  [= %d]", n, n);
  if (
      /* exploit the fact that cqi_send_byte() only uses the lowest 8 bytes of its argument */
      !(cqi_send_byte(n>>24, 1)) ||
      !(cqi_send_byte(n>>16, 1)) ||
      !(cqi_send_byte(n>>8, 1)) ||
      !(cqi_send_byte(n, 1))
      ) {
    perror("ERROR cqi_send_int()");
    return 0;
  }
  return 1;
}


/**
 * Sends a STRING to the client.
 *
 * CQi strings are NOT null-terminated -- so while the argument to this
 * function needs to be a null-terminated string, the string that actually
 * gets sent across the network will not be.
 *
 * This function should be called via one of the cqi_data_* functions
 * and not on its own.
 *
 * @return  Boolean: true if everything OK, otherwise false.
 */
int
cqi_send_string(const char *str)
{
  int len;

  if (!str) {
    if (!cqi_send_word(0)) {
      perror("ERROR cqi_send_string()");
      return 0;
    }
    return 1;
  }

  len = strlen(str);
  if (!cqi_send_word(len)) {
     perror("ERROR cqi_send_string()");
     return 0;
  }
  cqiserver_snoop("SEND CHAR[] '%s'", str);

  /* we can afford to mangle len and str because we're at the end of the function */
  while (--len >= 0)
    if (!cqi_send_byte(*str++, 1)) {
      perror("ERROR cqi_send_string()");
      return 0;
    }

  return 1;
}


/**
 * Sends a BYTE[] (byte list) to the client.
 *
 * This function should be called via one of the cqi_data_* functions
 * and not on its own.
 *
 * @param list        pointer to a block of bytes to send.
 * @param l           the number of bytes to send.
 * @param as_boolean  if true, send 1 for any true byte, 0 for 0.
 *
 * @return  Boolean: true if everything OK, otherwise false.
 */
int
cqi_send_byte_list(cqi_byte *list, int l, int as_boolean)
{
  if (!cqi_send_int(l)) {
    perror("ERROR cqi_send_byte_list()");
    return 0;
  }
  while (--l >= 0) {
    if (!cqi_send_byte(as_boolean ? (*list ? CQI_CONST_TRUE : CQI_CONST_FALSE) : *list, 0)) {
      perror("ERROR cqi_send_byte_list()");
      return 0;
    }
    list++;
  }
  return 1;
}

/**
 * Sends an INT[] (integer list) to the client.
 *
 * This function should be called via one of the cqi_data_* functions
 * and not on its own.
 *
 * @param list  pointer to a block of integers to send.
 * @param l     the number of integers to send.
 *
 * @return  Boolean: true if everything OK, otherwise false.
 */
int
cqi_send_int_list(int *list, int l)
{
  if (!cqi_send_int(l)) {
    perror("ERROR cqi_send_int_list()");
    return 0;
  }
  while (--l >= 0) {
    if (!cqi_send_int(*list++)) {
      perror("ERROR cqi_send_int_list()");
      return 0;
    }
  }
  return 1;
}


/**
 * Sends a STRING[] (string list) to the client.
 *
 * This function should be called via one of the cqi_data_* functions
 * and not on its own.
 *
 * @param list  pointer to a block of pointers-to-strings; the strings will be sent.
 * @param l     the number of strings to send.
 */
int
cqi_send_string_list(char **list, int l)
{
  if (!cqi_send_int(l)) {
    perror("ERROR cqi_send_string_list()");
    return 0;
  }
  while (--l >= 0) {
    if (!cqi_send_string(*list++)) {
      perror("ERROR cqi_send_string_list()");
      return 0;
    }
  }
  return 1;
}



/**
 * Sends a general CQi command, without any arguments.
 */
void
cqi_command(int command)
{
  if (!cqi_send_word(command) || !cqi_flush())
    cqi_send_error("cqi_command");
}

/**
 * Sends a byte of data to the client.
 */
void
cqi_data_byte(int n)
{
  if (!cqi_send_word(CQI_DATA_BYTE) || !cqi_send_byte(n ? CQI_CONST_TRUE : CQI_CONST_FALSE, 0) || !cqi_flush())
    cqi_send_error("cqi_data_byte");
}

/**
 * Sends a boolean to the client.
 */
void
cqi_data_bool(int n)
{
  if (!cqi_send_word(CQI_DATA_BOOL) || !cqi_send_byte(n ? CQI_CONST_TRUE : CQI_CONST_FALSE, 0) || !cqi_flush())
    cqi_send_error("cqi_data_bool");
}

/**
 * Sends an integer to the client.
 */
void
cqi_data_int(int n)
{
  if (!cqi_send_word(CQI_DATA_INT) || !cqi_send_int(n) || !cqi_flush())
    cqi_send_error("cqi_data_int");
}

/**
 * Sends a string to the client.
 */
void
cqi_data_string(const char *str)
{
  if (!cqi_send_word(CQI_DATA_STRING) || !cqi_send_string(str) || !cqi_flush())
    cqi_send_error("cqi_data_string");
}

/**
 * Sends a byte list to the client.
 *
 * @param list  pointer to a block of bytes to send.
 * @param l     the number of bytes to send.
 */
void
cqi_data_byte_list(cqi_byte *list, int l)
{
  if (!cqi_send_word(CQI_DATA_BYTE_LIST) || !cqi_send_byte_list(list, l, 0) || !cqi_flush())
    cqi_send_error("cqi_data_byte_list");
}

/**
 * Sends a list of booleans to the client.
 *
 * @param list  pointer to a block of booleans (each occupying one byte) to send.
 * @param l     the number of bytes to send.
 */
void
cqi_data_bool_list(cqi_byte *list, int l)
{
  if (!cqi_send_word(CQI_DATA_BOOL_LIST) || !cqi_send_byte_list(list, l, 1) || !cqi_flush())
    cqi_send_error("cqi_data_bool_list");
}

/**
 * Sends a list of integers to the client.
 *
 * @param list  pointer to a block of integers to send.
 * @param l     the number of integers to send.
 */
void
cqi_data_int_list(int *list, int l)
{
  if (!cqi_send_word(CQI_DATA_INT_LIST) || !cqi_send_int_list(list, l) || !cqi_flush())
    cqi_send_error("cqi_data_int_list");
}

/**
 * Sends a list of strings to the client.
 *
 * @param list  pointer to a block of pointers-to-strings; the strings will be sent.
 * @param l     the number of strings to send.
 */
void
cqi_data_string_list(char **list, int l)
{
  if (!cqi_send_word(CQI_DATA_STRING_LIST) || !cqi_send_string_list(list, l) || !cqi_flush())
    cqi_send_error("cqi_data_string_list");
}

/**
 * Sends a sequence of two integers to the client.
 *
 * @param n1  The first integer sent
 * @param n2  The second integer sent
 */
void
cqi_data_int_int(int n1, int n2)
{
  if (
      !cqi_send_word(CQI_DATA_INT_INT) ||
      !cqi_send_int(n1) ||
      !cqi_send_int(n2) ||
      !cqi_flush()
      )
    cqi_send_error("cqi_data_int_int");
}

/**
 * Sends a sequence of four integers to the client.
 *
 * @param n1  The first integer sent
 * @param n2  The second integer sent
 * @param n3  The third integer sent
 * @param n4  The fourth integer sent
 */
void
cqi_data_int_int_int_int(int n1, int n2, int n3, int n4)
{
  if (
      !cqi_send_word(CQI_DATA_INT_INT_INT_INT) ||
      !cqi_send_int(n1) ||
      !cqi_send_int(n2) ||
      !cqi_send_int(n3) ||
      !cqi_send_int(n4) ||
      !cqi_flush()
      )
      cqi_send_error("cqi_data_int_int_int_int");
}


/*
 *
 *  Client -> Server communication
 *
 */

int
cqi_recv_bytes(cqi_byte *buf, int bytes)
{
  if (bytes <= 0)
    return 1;
  cqiserver_snoop("RECV BYTE[%d]", bytes);
  if (bytes != recv(connfd, buf, bytes, MSG_WAITALL)) {
    perror("ERROR cqi_recv_bytes()");
    return 0;
  }
  return 1;
}

int
cqi_recv_byte(void)
{
  cqi_byte b;
  if (1 != recv(connfd, &b, 1, MSG_WAITALL)) {
    perror("ERROR cqi_recv_byte()");
    return EOF;
  }
  cqiserver_snoop("RECV BYTE 0x%02X", b);
  return b;
}

int
cqi_read_byte(void)
{
  int b = cqi_recv_byte();
  if (b == EOF)
    cqi_recv_error("cqi_read_byte");
  return b;
}

int
cqi_read_bool(void)
{
  int b = cqi_recv_byte() ? CQI_CONST_TRUE : CQI_CONST_FALSE;
  if (b == EOF)
    cqi_recv_error("cqi_read_bool");
  return b;
}

int
cqi_read_word(void)
{
  int n = cqi_read_byte();
  n = (n << 8) | cqi_read_byte();
  cqiserver_snoop("READ WORD   %04X      [= %d]", n, n);
  return n;
}

int
cqi_read_int(void)
{
  int n = cqi_read_byte();
  int minus_bits = ((int)-1) ^ 0xFFFFFFFF; /* extra minus bits if int is > 32 bit*/

  n = (n << 8) | cqi_read_byte();
  n = (n << 8) | cqi_read_byte();
  n = (n << 8) | cqi_read_byte();
  if (n & 0x80000000)           /* negative 32bit quantity */
    n |= minus_bits;            /* expand to full size of int type */
  cqiserver_snoop("READ INT    %08X  [= %d]", n, n);
  return n;
}

char *
cqi_read_string(void)
{
  char *s;
  int len = cqi_read_word();

  s = (char *) cl_malloc(len + 1);
  if (!cqi_recv_bytes((cqi_byte *)s, len))
    cqi_recv_error("cqi_read_string");
  s[len] = '\0';
  cqiserver_snoop("READ CHAR[] '%s'", s);
  return s;
}

int
cqi_read_command(void)
{
  int command;
  cqiserver_debug_msg("waiting for command");
  command = cqi_read_byte();
  while (command == CQI_PAD)
    command = cqi_read_byte();
  command = (command << 8) | cqi_read_byte();
  return command;
}

int
cqi_read_byte_list(cqi_byte **list)
{
  int i, len;

  len = cqi_read_int();
  if (len <= 0) {
    *list = NULL;
    return 0;
  }

  *list = (cqi_byte *) cl_malloc(len);
  for (i=0; i<len; i++)
    (*list)[i] = cqi_read_byte();
  cqiserver_snoop("READ BYTE[%d]", len);
  return len;
}

int
cqi_read_bool_list(cqi_byte **list)
{
  int i, len;

  len = cqi_read_int();
  if (len <= 0) {
    *list = NULL;
    return 0;
  }

  *list = (cqi_byte *) cl_malloc(len);
  for (i=0; i<len; i++)
    (*list)[i] = cqi_read_byte() ? CQI_CONST_TRUE : CQI_CONST_FALSE;
  cqiserver_snoop("READ BOOL[%d]", len);
  return len;
}

int
cqi_read_int_list(int **list)
{
  int i, len;

  len = cqi_read_int();
  if (len <= 0) {
    *list = NULL;
    return 0;
  }

  *list = (int *) cl_malloc(len * sizeof(int));
  for (i=0; i<len; i++)
    (*list)[i] = cqi_read_int();
  cqiserver_snoop("READ INT[%d]", len);
  return len;
}

int
cqi_read_string_list(char ***list)
{
  int i, len;

  len = cqi_read_int();
  if (len <= 0) {
    *list = NULL;
    return 0;
  }

  *list = (char **) cl_malloc(len * sizeof(char *));
  for (i=0; i<len; i++)
    (*list)[i] = cqi_read_string();
  cqiserver_snoop("READ STRING[%d]\n", len);
  return len;
}





/*
 *
 *  Naming conventions & specifier split
 *
 */

char cqi_id_uc_first[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_";
char cqi_id_uc[]       = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_-0123456789";
char cqi_id_lc_first[] = "abcdefghijklmnopqrstuvwxyz_";
char cqi_id_lc[]       = "abcdefghijklmnopqrstuvwxyz_-0123456789";
char cqi_id_all[]      = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-0123456789";



/**
 * Makes sure that an identifier complies with the specified convention regarding its initial and subsequent characters.
 *
 * If the identifier  does not conform, cqi_errno is set and the function returns false.
 *
 * @param name       The identifier to check.
 * @param init_cap   If true, the initial char must be uppercase or underscore.
 *                   If false, it can be lowercase or underscore.
 * @param rest_cap   If true, subsequent chars must be uppercase or underscore.
 *                   If false, they must be lowercase or underscore or hyphen.
 * @param rest_any   If true, rest_cap is overridden: any valid identifier char is accepted.
 *                   If false, the value given as rest_cap is enforced.
 *
 *
 * @return    Boolean: is the identifier in accordance with convention?
 */
int
check_identifier_convention(char *name, int init_cap, int rest_cap, int rest_any)
{
  char *valid_init = init_cap ? cqi_id_uc_first : cqi_id_lc_first;
  char *valid_rest = rest_any ? cqi_id_all      : (rest_cap ? cqi_id_uc : cqi_id_lc);

  if (
      !strchr(valid_init, *name)
      ||
      strspn(name+1, valid_rest) != strlen(name+1)
      ) {
    cqi_errno = CQI_ERROR_SYNTAX_ERROR;
    return 0;
  }
  cqi_errno = CQI_STATUS_OK;
  return 1;
}

// now implemented as macro in server.h:

//int
//check_corpus_name(char *name)
//{
//  if (
//      !strchr(cqi_id_uc_first, *name)
//      ||
//      strspn(name+1, cqi_id_uc) != strlen(name+1)
//      ) {
//    cqi_errno = CQI_ERROR_SYNTAX_ERROR;
//    return 0;
//  }
//  cqi_errno = CQI_STATUS_OK;
//  return 1;
//}
//int
//check_attribute_name(char *name)
//{
//  if (
//      !strchr(cqi_id_lc_first, *name)
//      ||
//      strspn(name+1, cqi_id_lc) != strlen(name+1)
//      ) {
//    cqi_errno = CQI_ERROR_SYNTAX_ERROR;
//    return 0;
//  }
//  cqi_errno = CQI_STATUS_OK;
//  return 1;
//}


//int
//check_subcorpus_name(char *name)
//{
//  if (
//      !strchr(cqi_id_uc_first, *name)
//      ||
//      strspn(name+1, cqi_id_all) != strlen(name+1)
//      ) {
//    cqi_errno = CQI_ERROR_SYNTAX_ERROR;
//    return 0;
//  }
//  cqi_errno = CQI_STATUS_OK;
//  return 1;
//}

/** new strings are allocated for the output arguments */
int
split_attribute_spec(char *spec, char **corpus, char **attribute)
{
  char *split = strchr(spec, '.');

  if (!split) {
    cqi_errno = CQI_ERROR_SYNTAX_ERROR;
    return 0;
  }
  *corpus = cl_strdup_to(spec, split);
  *attribute = cl_strdup(split+1);
  if (!check_corpus_name(*corpus) || !check_attribute_name(*attribute)) {
    cl_free(*corpus);
    cl_free(*attribute);
    return 0;                   /* cqi_errno set by check_* function */
  }
  cqi_errno = CQI_STATUS_OK;
  return 1;
}

int
split_subcorpus_spec(char *spec, char **corpus, char **subcorpus)
{
  char *split = strchr(spec, ':');

  if (!split) {
    *corpus = cl_strdup(spec);
    *subcorpus = NULL;
  }
  else {
    *corpus    = cl_strdup_to(spec, split);
    *subcorpus = cl_strdup(split+1);
  }
  if (
      !check_corpus_name(*corpus)
      ||
      (NULL != *subcorpus && !check_subcorpus_name(*subcorpus))
      ) {
    cl_free(*corpus);
    cl_free(*subcorpus);
    return 0;                 /* cqi_errno set by check_* function */
  }
  cqi_errno = CQI_STATUS_OK;
  return 1;
}

char *
combine_subcorpus_spec(char *corpus, char *subcorpus)
{
  char *spec;
  if (!subcorpus)
    return cl_strdup(corpus);
  spec = (char *) cl_malloc(strlen(corpus) + strlen(subcorpus) + 2);
  snprintf(spec, strlen(corpus) + strlen(subcorpus) + 2, "%s:%s", corpus, subcorpus);
  return spec;
}

/*
 *
 *  Hash Utilities (lookup of attributes)
 *
 */

/** The AttBucket object is a holder for an attribute and its key string. */
typedef struct att_bucket {
  char *string;                 /**< the key for this attribute bucket. */
  Attribute *attribute;
  int type;                     /**< type of the attribute in this bucket: ATT_NONE, ATT_POS, ATT_STRUC, ... */
} AttBucket;

/** Underlying structure for the AttHashTable object.  @see AttHashTable */
struct att_hashtable {
  AttBucket *space;             /**< the actual array of attribute buckets. */
  int code;
  int size;
};

/** An AttHashTable object contains space for a hash table of attribute-pointers. */
typedef struct att_hashtable *AttHashTable;

/** The CQi server's global attribute hash */
AttHashTable AttHash = NULL;

/**
 * This function has to be called once to initialise the global attribute hash.
 *
 * @see AttHash
 */
void
make_attribute_hash(int size)
{
  int bytes;
  AttHash = (AttHashTable)cl_malloc(sizeof(struct att_hashtable));
  AttHash->size = cl_find_prime(size);
  bytes = sizeof(AttBucket) * AttHash->size;
  AttHash->space = (AttBucket *)cl_malloc(bytes);
  memset(AttHash->space, 0, bytes);
  AttHash->code = 0;
}

/**
 * Frees the global AttHash object and the space that it points to.
 *
 * @see AttHash
 */
void
free_attribute_hash(void)
{
  if (AttHash)
    cl_free(AttHash->space);
  cl_free(AttHash);
}

/**
 * Finds an AttBucket within the global AttHash that matches the argument string.
 */
static AttBucket *
att_hash_lookup(char *str)
{
  AttBucket *p, *end;
  int i = 0;
  int offset;

  if (!AttHash)
    cqi_internal_error("att_hash_lookup", "AttHash not initialised.");

  /* points to the end of the space */
  end = AttHash->space + AttHash->size;
  /* the hash value of the string */
  offset = cl_hash_string(str) % AttHash->size;
  /* the primary pointer into the space */
  p = AttHash->space + offset;

  for (i = (int)AttHash->size/5; i>0; p++, i--) {
    if (p >= end)
      p = AttHash->space;
    if (!p->string) {
      /* init new bucket */
      p->string = cl_strdup(str);
      p->attribute = NULL;
      p->type = ATT_NONE;
      break;
    }
    else if (cl_streq(p->string, str))
      break;
  }

  if (i == 0)
    cqi_internal_error("att_hash_lookup", "Too many collisions.");
  return p;
}

Attribute *
cqi_lookup_attribute(char *name, int type)
{
  AttBucket *p = att_hash_lookup(name);
  if (p->attribute == NULL) {
    /* try to open the attribute */
    char *corpus_name, *attribute_name;
    CorpusList *cl;
    Attribute *attribute;

    cqiserver_debug_msg("AttHash: attribute '%s' not found, trying to open ...", name);

    if (!split_attribute_spec(name, &corpus_name, &attribute_name))
      return NULL;
    cl = findcorpus(corpus_name, SYSTEM, 0);
    if (!cl || !access_corpus(cl)) {
      cqi_errno = CQI_CQP_ERROR_NO_SUCH_CORPUS;
      return NULL;
    }
    if (!(attribute = cl_new_attribute(cl->corpus, attribute_name, type))) {
      cqi_errno = CQI_CL_ERROR_NO_SUCH_ATTRIBUTE;
      return NULL;
    }
    p->attribute = attribute;
    p->type = type;
    cqi_errno = CQI_STATUS_OK;
    return p->attribute;
  }
  if (p->type != type) {
    cqiserver_debug_msg("AttHash: attribute '%s' found, wrong attribute type.", name);
    cqi_errno = CQI_CL_ERROR_WRONG_ATTRIBUTE_TYPE;
    return NULL;
  }
  cqiserver_debug_msg("AttHash: attribute '%s' found in hash.", name);
  cqi_errno = CQI_STATUS_OK;
  return p->attribute;
}

/**
 * Drops the named attribute from memory.
 *
 * The attribute name is looked up in the global hash,
 * and then cl_delete_attribute() is called on the result.
 *
 * @see         cl_delete_attribute
 * @param name  Name of the attribute to be deleted
 * @return      Boolean: true for all OK, otherwise false
 */
int
cqi_drop_attribute(char *name)
{
  AttBucket *p = att_hash_lookup(name);
  if (p->attribute && cl_delete_attribute(p->attribute)) {
    p->attribute = NULL;
    p->type = ATT_NONE;
    return 1;
  }
  return 0;
}


/*
 *
 *  CQP internal function wrappers
 *
 */

/**
 * Gets a pointer to the corpus with the given name.
 */
CorpusList *
cqi_find_corpus(char *name)
{
  CorpusList *cl;
  char *corpus, *subcorpus;

  if (check_corpus_name(name))
    cl = findcorpus(name, SYSTEM, 0);
  else {
    if (split_subcorpus_spec(name, &corpus, &subcorpus)) {
      cl_free(corpus);
      cl_free(subcorpus);
      cl = findcorpus(name, SUB, 0);
    }
    else
      cl = NULL;                        /* cqi_errno set by split_subcorpus_spec */
  }

  if (!cl || !access_corpus(cl)) {
    cqi_errno = CQI_CQP_ERROR_NO_SUCH_CORPUS;
    return NULL;
  }
  cqi_errno = CQI_STATUS_OK;
  return cl;
}

/**
 * Activates the named corpus.
 */
int
cqi_activate_corpus(char *name)
{
  CorpusList *cl;
  cqiserver_debug_msg("cqi_activate_corpus('%s');\n", name);
  if (!(cl = cqi_find_corpus(name)))
    return 0;
  set_current_corpus(cl, 0);
  return 1;
}
