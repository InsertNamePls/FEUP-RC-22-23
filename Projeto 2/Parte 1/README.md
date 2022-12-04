# Instructions for **FTP download app** execution

## Project Structure

- bin/: Compiled binaries.
- src/: Source code for the implementation of the FTP protocol.
- include/: Header files.
- main.c: Main file.
- Makefile: Makefile to build the project and run the application.

## Instructions to Run the Project

1. Compile the application using the provided Makefile.

2. Test the protocol by running the download application replacing the fields in bold below:
<br> &nbsp;&nbsp;&nbsp;$ make
<br> &nbsp;&nbsp;&nbsp;$ ./bin/download ftp://[**\<user>**:**\<password>**@]**\<host>**/**\<url-path>**	