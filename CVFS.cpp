
////////////////////////////////////////////////////////////////////////////////
//
//  Header files inclusion
//
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <iostream>
using namespace std;

////////////////////////////////////////////////////////////////////////////////
//
//  User defined Macros
//
////////////////////////////////////////////////////////////////////////////////

// Max file size
#define MAXFILESIZE 100
// Maximum number of files that we can open
#define MAXOPENEDFILES 20

// Maximum number of files that we can create
#define MAXINODE 5
#define READ 1
#define WRITE 2
#define EXECUTE 3
#define REGULARFILE 1
#define SPECIALFILE 2
#define START 0
#define CURRENT 1
#define END 2
#define EXECUTE_SUCCESS 0

////////////////////////////////////////////////////////////////////////////////
//
//  User defined Macros for error handling
//
////////////////////////////////////////////////////////////////////////////////
#define ERR_INVALID_PARAMETER -1
#define ERR_NO_INODES -2
#define ERR_FILE_ALREADY_EXIST -3
#define ERR_FILE_NOT_EXIST -3
#define ERR_PERMISSION_DENIED -5
#define ERR_INSUFFECIENT_MEMORY -6
#define ERR_INSUFFICIENT_DATA -7

////////////////////////////////////////////////////////////////////////////////
//
//  Structure Name :    BootBlock
//  Description    :    Holds information to boot the operating system
//
////////////////////////////////////////////////////////////////////////////////
struct BootBlock
{
  char Information[100];
};

////////////////////////////////////////////////////////////////////////////////
//
//  Structure Name :    SuperBlock
//  Description    :    Holds information about the file system
//
////////////////////////////////////////////////////////////////////////////////
struct SuperBlock
{
  int TotalInodes;
  int FreeInodes;
};
////////////////////////////////////////////////////////////////////////////////
//
//  Structure Name :    Inode
//  Description    :    Holds information about the file
//
////////////////////////////////////////////////////////////////////////////////

typedef struct inode
{
  char FileName[50];
  int InodeNumber;
  int FileSize;
  int ActualFileSize;
  int FileType;
  int LinkCount;
  int ReferenceCount;
  int Permission;
  char *Buffer;

  struct inode *next;
  struct inode *parent;
  struct inode *child;
  struct inode *sibling;
} INODE, *PINODE, **PPINODE;

////////////////////////////////////////////////////////////////////////////////
//
//  Structure Name :    FileTable
//  Description    :    Holds information about the opened file
//
////////////////////////////////////////////////////////////////////////////////
typedef struct FileTable
{
  int ReadOffset;
  int WriteOffset;
  int Count;
  int Mode;
  PINODE ptrinode;
} FILETABLE, *PFILETABLE;

////////////////////////////////////////////////////////////////////////////////
//
//  Structure Name :    UAREA
//  Description    :    Holds information about the process
//
////////////////////////////////////////////////////////////////////////////////
struct UAREA
{
  char ProcessName[50];
  char CurrentDirectory[50];
  PFILETABLE UFDT[MAXOPENEDFILES];
};

////////////////////////////////////////////////////////////////////////////////
//
//  Global variables or objects used in the project
//
////////////////////////////////////////////////////////////////////////////////
BootBlock bootobj;
SuperBlock superobj;
PINODE head = NULL;
PINODE root = NULL;
PINODE cwd = NULL;
UAREA uareaobj;

////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     InitialiseUAREA
//  Description   :     It is used to intialise the contents UAREA
//  Author        :     Aditi Dipak Thorat
//  Date          :     10/08/2025
//
////////////////////////////////////////////////////////////////////////////////
void InitialiseUAREA()
{
  strcpy(uareaobj.ProcessName, "Myexe");
  strcpy(uareaobj.CurrentDirectory, "/");
  int i = 0;
  while (i < MAXOPENEDFILES)
  {
    uareaobj.UFDT[i] = NULL;
    i++;
  }
  cout << "Custom Virtual File System : UAREA initialised successfully" << endl;
}
////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     InitialiseSuperblock
//  Description   :     It is used to intialise the contents of super block
//  Author        :     Aditi Dipak Thorat
//  Date          :     10/08/2025
//
////////////////////////////////////////////////////////////////////////////////
void InitialiseSuperBlock()
{
  superobj.TotalInodes = MAXINODE;
  superobj.FreeInodes = MAXINODE;
  cout << "Custom Virtual File System : SuperBlock initialised successfully" << endl;
}
////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     CreateDILB
//  Description   :     It is used to create Linked List of Inodes
//  Author        :     Aditi Dipak Thorat
//  Date          :     10/08/2025
//
////////////////////////////////////////////////////////////////////////////////
void CreateDLIB()
{
  int i = 1;
  PINODE newn = NULL;
  PINODE temp = head;
  while (i <= MAXINODE)
  {
    newn = new INODE;
    newn->InodeNumber = i;
    newn->FileSize = 0;
    newn->ActualFileSize = 0;
    newn->LinkCount = 0;
    newn->FileType = 0;
    newn->ReferenceCount = 0;
    newn->Buffer = NULL;
    newn->next = NULL;
    newn->parent = NULL;
    newn->child = NULL;
    newn->sibling = NULL;
    if (temp == NULL)
    {
      head = newn;
      temp = head;
    }
    else
    {
      temp->next = newn;
      temp = temp->next;
    }
    i++;
  }
  cout << "Custom Virtual File System : DILIB created successfully " << endl;
}
////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     StartAuxilaryDataInitialisation
//  Description   :     It is used to intialise the Auxilary data
//  Author        :     Aditi Dipak Thorat
//  Date          :     10/08/2025
//
////////////////////////////////////////////////////////////////////////////////

void StartAuxillaryDataInitialisation()
{
  strcpy(bootobj.Information, "Boot process of operating system done");
  cout << bootobj.Information << endl;
  InitialiseSuperBlock();
  CreateDLIB();
  InitialiseUAREA();

  // Initialize root directory
  root = get_free_inode();
  if (root)
  {
    strcpy(root->FileName, "/");
    root->FileType = SPECIALFILE;
    root->FileSize = MAXFILESIZE;
    root->ActualFileSize = 0;
    root->LinkCount = 1;
    root->ReferenceCount = 1;
    root->Permission = READ + WRITE;
    root->Buffer = (char *)malloc(MAXFILESIZE);
    root->parent = NULL;
    root->child = NULL;
    root->sibling = NULL;
    cwd = root;
  }

  cout << "Custom Virtual File System : Auxilliary data initalised successfully" << endl;
}

///////////////////////////////////////////////////////////////////////////////
//
//  Function Name :   DisplayHelp
//  Description   :   Displays information about all supported commands in CVFS.
//  Input         :   None
//  Output        :   Prints command details to the console
//  Author        :   Aditi Dipak Thorat
//  Date          :   11/08/2025
//
///////////////////////////////////////////////////////////////////////////////
void DisplayHelp()
{
  cout << "--------------------------------------------------------------------------------" << endl;
  cout << "----------------------------Command Manual CVFS---------------------------------" << endl;
  cout << "--------------------------------------------------------------------------------" << endl;
  cout << "man    : It is used to display the specific manual page for the command" << endl;
  cout << "exit   : It is used to terminate the shell of CVFS" << endl;
  cout << "clear  : It is used to clear the console of CVFS" << endl;
  cout << "creat  : It is used to create new regular file" << endl;
  cout << "unlink : It is used to delete the existing file" << endl;
  cout << "stat   : It is used to display statistical information about the file" << endl;
  cout << "ls     : It is used to list out all the files from the directory" << endl;
  cout << "write  : It is used to write the data into the file" << endl;
  cout << "read   : It is used to read the data from the file" << endl;
  cout << "chmod  : It is used to change permissions of the file" << endl;
  cout << "open   : It is used to open the file using file descriptor" << endl;
  cout << "close  : It is used to close an opened file" << endl;
  cout << "fstat  : It is used to display file stats using FD" << endl;
  cout << "lseek  : It is used to change read/write offsets in file" << endl;
  cout << "mkdir  : It is used to create a directory" << endl;
  cout << "rmdir  : It is used to remove a directory" << endl;
  cout << "cd     : It is used to change current directory" << endl;
  cout << "find   : It is used to find file in a given path" << endl;
  cout << "--------------------------------------------------------------------------------" << endl;
}

////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     ManPage
//  Description   :     It is used to display the manual page of a command
//  Input         :     Name of the command to display the manual for
//  Output        :     Prints the manual details of the command to the console
//  Author        :     Aditi Dipak Thorat
//  Date          :     11/08/2025
//
////////////////////////////////////////////////////////////////////////////////
void ManPage(
    char *name // Name of command
)
{
  if (strcmp(name, "creat") == 0)
  {
    printf("Description : This command is used to create new regular file on our file system\n");

    printf("Usage : creat File_name Permissions\n");
    printf("File_name : The name of file that you want to create\n");
    printf("Permissions : \n1 : Read \n2 : Write \n3 : Read + Write\n");
  }
  else if (strcmp(name, "exit") == 0)
  {
    printf("Description : This command is used to terminate the Custom Virtual File System\n");

    printf("Usage : exit\n");
  }
  else if (strcmp(name, "unlink") == 0)
  {
    printf("Description : This command is used to delete the existing regular file on our file system\n");

    printf("Usage : unlink File_name \n");
    printf("File_name : The name of file that you want to delete\n");
  }
  else if (strcmp(name, "ls") == 0)
  {
    printf("Description : This command is used to display all file name  on our Directory \n");

    printf("Usage : ls  \n");
  }
  else if (strcmp(name, "stat") == 0)
  {
    printf("Description : This command is used to display  the  stats of the  file on our file system\n");

    printf("Usage : stat File_name \n");
    printf("File_name : The name of file that you want to see the details of \n");
  }
  else if (strcmp(name, "write") == 0)
  {
    printf("Description : This command is used to write the data into the  file \n");

    printf("Usage : write file_descriptor \n");
    printf("file_descriptor : It is the value returned by create system call \n");
  }
  else if (strcmp(name, "read") == 0)
  {
    printf("Description : This command is used to read the data from the  file \n");

    printf("Usage : read file_descriptor size \n");
    printf("File_Descriptor : It is the value returned by create system call  \n");
    printf("Size : Number of bytes you want to read\n");
  }
  else if (strcmp(name, "chmod") == 0)
  {
    printf("Description : This command is used to change the permissions of the file\n");

    printf("Usage : chmod File_name Permissions\n");
    printf("File_name : The name of file whose permissions you want to change\n");
    printf("Permissions : \n1 : Read \n2 : Write \n3 : Read + Write\n");
  }
  else if (strcmp(name, "open") == 0)
  {
    printf("Description : This command is used to open an existing file\n");

    printf("Usage : open File_name Mode\n");
    printf("File_name : The name of file that you want to open\n");
    printf("Mode : \n1 : Read \n2 : Write \n3 : Read + Write\n");
  }
  else if (strcmp(name, "close") == 0)
  {
    printf("Description : This command is used to close an opened file\n");

    printf("Usage : close file_descriptor\n");
    printf("file_descriptor : It is the value returned by open system call\n");
  }
  else if (strcmp(name, "fstat") == 0)
  {
    printf("Description : This command is used to display file stats using file descriptor\n");

    printf("Usage : fstat file_descriptor\n");
    printf("file_descriptor : It is the value returned by open system call\n");
  }
  else if (strcmp(name, "lseek") == 0)
  {
    printf("Description : This command is used to change read/write offsets in file\n");

    printf("Usage : lseek file_descriptor offset whence\n");
    printf("file_descriptor : It is the value returned by open system call\n");
    printf("offset : Number of bytes to move the offset\n");
    printf("whence : \n0 : Start \n1 : Current \n2 : End\n");
  }

  else if (strcmp(name, "mkdir") == 0)
  {
    printf("Description : This command is used to create a directory\n");

    printf("Usage : mkdir Directory_name\n");
    printf("Directory_name : The name of directory that you want to create\n");
  }
  else if (strcmp(name, "rmdir") == 0)
  {
    printf("Description : This command is used to remove a directory\n");

    printf("Usage : rmdir Directory_name\n");
    printf("Directory_name : The name of directory that you want to remove\n");
  }
  else if (strcmp(name, "cd") == 0)
  {
    printf("Description : This command is used to change current directory\n");

    printf("Usage : cd Directory_name\n");
    printf("Directory_name : The name of directory to change to\n");
  }
  else if (strcmp(name, "find") == 0)
  {
    printf("Description : This command is used to find file in a given path\n");

    printf("Usage : find File_name\n");
    printf("File_name : The name of file that you want to find\n");
  }
  else
  {
    printf("No manual entry for %s\n", name);
  }
}

////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     IsFileExists
//  Description   :     Checks whether the given file name exists in the file
//                      system
//  Input         :     Name of the file to check
//  Output        :     Returns true if the file exists, false otherwise
//  Author        :     Aditi Dipak Thorat
//  Date          :     11/08/2025
//
////////////////////////////////////////////////////////////////////////////////
bool IsFileExists(
    char *name // Name of file that we want to check
)
{
  PINODE temp = head;
  bool bFlag = false;

  while (temp != NULL)
  {
    if ((strcmp(name, temp->FileName) == 0) && (temp->FileType == REGULARFILE))
    {
      bFlag = true;
      break;
    }

    temp = temp->next;
  }

  return bFlag;
}

////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     CreateFile
//  Description   :     It is used to create a new regular file
//  Input         :     It accepts the file name and permission
//  Output        :     It returns the file descriptor
//  Author        :     Aditi Dipak Thorat
//  Date          :     11/08/2025
//
////////////////////////////////////////////////////////////////////////////////
int CreateFile(
    char *name,    // Name of file
    int permission // Permission to create file
)
{
  PINODE temp = head;
  int i = 0;

  // Filters

  // If file name is missing
  if (name == NULL)
  {
    return ERR_INVALID_PARAMETER;
  }

  // If entered permission is invalid
  if (permission < 1 || permission > 3)
  {
    return ERR_INVALID_PARAMETER;
  }

  // Check whether empty inode is there or not
  if (superobj.FreeInodes == 0)
  {
    return ERR_NO_INODES;
  }

  // Check whether file is already exist or not
  if (IsFileExists(name) == true)
  {
    return ERR_FILE_ALREADY_EXIST;
  }

  // Loop to search free Inode
  while (temp != NULL)
  {
    if (temp->FileType == 0)
    {
      break;
    }
    temp = temp->next;
  }

  // Inode not found
  if (temp == NULL)
  {
    printf("Inode not found\n");
    return ERR_NO_INODES;
  }

  for (i = 0; i < MAXINODE; i++)
  {
    if (uareaobj.UFDT[i] == NULL)
    {
      break;
    }
  }

  if (i == MAXINODE)
  {
    printf("Unable to create file as MAX OPENED FILE LIMIT REACHED\n");
    return -1;
  }

  // Allocate memory for file table
  uareaobj.UFDT[i] = (PFILETABLE)malloc(sizeof(FILETABLE));

  // Initailise elements of File table
  uareaobj.UFDT[i]->ReadOffset = 0;
  uareaobj.UFDT[i]->WriteOffset = 0;
  uareaobj.UFDT[i]->Count = 1;
  uareaobj.UFDT[i]->Mode = permission;

  // Connect file table with IIT
  uareaobj.UFDT[i]->ptrinode = temp;

  strcpy(uareaobj.UFDT[i]->ptrinode->FileName, name);
  uareaobj.UFDT[i]->ptrinode->FileSize = MAXFILESIZE;
  uareaobj.UFDT[i]->ptrinode->ActualFileSize = 0;
  uareaobj.UFDT[i]->ptrinode->FileType = REGULARFILE;
  uareaobj.UFDT[i]->ptrinode->ReferenceCount = 1;
  uareaobj.UFDT[i]->ptrinode->LinkCount = 1;
  uareaobj.UFDT[i]->ptrinode->Permission = permission;

  // Allocate memory for Buffer
  uareaobj.UFDT[i]->ptrinode->Buffer = (char *)malloc(MAXFILESIZE);

  // decrement the number of free inodes by one
  superobj.FreeInodes--;
  return i;
}

////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     UnlinkFile
//  Description   :     It is used to delete a regular file
//  Input         :     It accepts the file name
//  Output        :     It returns an integer
//  Author        :     Aditi Dipak Thorat
//  Date          :     15/08/2025
//
////////////////////////////////////////////////////////////////////////////////
int UnlinkFile(
    char *name // Name of the file
)
{
  int i = 0;
  if (name == NULL)
  {
    return ERR_INVALID_PARAMETER;
  }

  if (IsFileExists(name) == false)
  {
    return ERR_FILE_NOT_EXIST;
  }

  for (i = 0; i < MAXINODE; i++)
  {
    if (uareaobj.UFDT[i] != NULL)
    {
      if (strcmp(uareaobj.UFDT[i]->ptrinode->FileName, name) == 0)
      {
        // Deallocate the memory of buffer
        free(uareaobj.UFDT[i]->ptrinode->Buffer);

        // Reset all values of Inode
        uareaobj.UFDT[i]->ptrinode->FileSize = 0;
        uareaobj.UFDT[i]->ptrinode->ActualFileSize = 0;
        uareaobj.UFDT[i]->ptrinode->LinkCount = 0;
        uareaobj.UFDT[i]->ptrinode->Permission = 0;
        uareaobj.UFDT[i]->ptrinode->FileType = 0;
        uareaobj.UFDT[i]->ptrinode->ReferenceCount = 0;

        // deallocate memory of file table
        free(uareaobj.UFDT[i]);

        // set null to the ufdt member
        uareaobj.UFDT[i] = NULL;

        // Increment the value of free Inodes Count
        superobj.FreeInodes++;
        break;
      }
    }
  }
  return EXECUTE_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     ls_file
//  Description   :     It is used to display the information about all the
//                      files in the directory
//  Input         :     None
//  Output        :     None
//  Author        :     Aditi Dipak Thorat
//  Date          :     15/08/2025
//
////////////////////////////////////////////////////////////////////////////////

void ls_file()
{
  if (head == NULL)
  {
    printf("No files present in the system.\n");
    return;
  }

  PINODE temp = head;

  printf("--------------------------------------------------------------------------------\n");
  printf("%-15s %-8s %-10s %-12s %-15s %-5s\n",
         "File Name", "Inode", "Type", "Size(Bytes)", "Permission", "Links");
  printf("--------------------------------------------------------------------------------\n");

  while (temp != NULL)
  {
    if (temp->FileType != 0)
    {
      // File Type
      const char *type = (temp->FileType == REGULARFILE) ? "File" : "Dir";

      // Permissions
      char perm[20] = "";
      if (temp->Permission == READ)
        strcpy(perm, "Read");
      else if (temp->Permission == WRITE)
        strcpy(perm, "Write");
      else if (temp->Permission == (READ + WRITE))
        strcpy(perm, "Read+Write");

      printf("%-15s %-8d %-10s %-12d %-15s %-5d\n",
             temp->FileName,
             temp->InodeNumber,
             type,
             temp->ActualFileSize,
             perm,
             temp->LinkCount);
    }
    temp = temp->next;
  }
}
////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     stat_file
//  Description   :     It is used to display the information about the given file
//  Input         :     It accepts file_name
//  Output        :     It returns an integer
//  Author        :     Aditi Dipak Thorat
//  Date          :     15/08/2025
//
////////////////////////////////////////////////////////////////////////////////

int stat_file(
    char *name //
)
{
  if (name == NULL)
  {
    return ERR_INVALID_PARAMETER;
  }

  if (IsFileExists(name) == false)
  {
    return ERR_FILE_NOT_EXIST;
  }

  PINODE temp = head;

  while (temp != NULL)
  {
    if ((strcmp(name, temp->FileName) == 0) && (temp->FileType != 0))
    {
      printf("-------------------------------------------------------------------------------------------\n");
      printf("------------------------ Statistical Information of the File -------------------------------\n");
      printf("File Name         : %s\n", temp->FileName);
      printf("File Size on disk : %d Bytes\n", temp->FileSize);
      printf("Actual File size  : %d Bytes\n", temp->ActualFileSize);
      printf("Link Count        : %d\n", temp->LinkCount);

      printf("File Permission   : ");
      if (temp->Permission == READ)
        printf("Read\n");
      else if (temp->Permission == WRITE)
        printf("Write\n");
      else if (temp->Permission == (READ + WRITE))
        printf("Read + Write\n");

      printf("File Type         : ");
      if (temp->FileType == REGULARFILE)
        printf("Regular File\n");
      else if (temp->FileType == SPECIALFILE)
        printf("Special File\n");

      printf("-------------------------------------------------------------------------------------------\n");

      break;
    }
    temp = temp->next;
  }

  return EXECUTE_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     write_file
//  Description   :     It is used to write the content into the file
//  Input         :     File_descriptor
//                      Address of buffer with contains data
//                      Size of data that we want to write
//  Output        :     Number of bytes sucessfully written into the file
//  Author        :     Aditi Dipak Thorat
//  Date          :     15/08/2025
//
////////////////////////////////////////////////////////////////////////////////
int write_file(
    int fd,     // File Descriptor of the file
    char *data, // Data that we want to write
    int size    // Size of the data that we want to write
)
{
  if (fd < 0 || fd >= MAXOPENEDFILES)
    return ERR_INVALID_PARAMETER;

  if (uareaobj.UFDT[fd] == NULL)
    return ERR_FILE_NOT_EXIST;

  if (data == NULL || size <= 0)
    return ERR_INVALID_PARAMETER;

  if (uareaobj.UFDT[fd]->Mode != WRITE && uareaobj.UFDT[fd]->Mode != (READ + WRITE))
    return ERR_PERMISSION_DENIED;

  PINODE pinode = uareaobj.UFDT[fd]->ptrinode;

  // Calculate required total size
  int required = uareaobj.UFDT[fd]->WriteOffset + size;

  // If current buffer size is not enough, grow it
  if (required > pinode->FileSize)
  {
    char *newBuffer = (char *)realloc(pinode->Buffer, required);
    if (newBuffer == NULL)
      return ERR_INSUFFECIENT_MEMORY;

    pinode->Buffer = newBuffer;
    pinode->FileSize = required; // Update max capacity
  }

  // Perform actual write
  memcpy(pinode->Buffer + uareaobj.UFDT[fd]->WriteOffset, data, size);

  // Update offsets and file size
  uareaobj.UFDT[fd]->WriteOffset += size;
  pinode->ActualFileSize += size;

  // Null-terminate for safety (optional)
  pinode->Buffer[uareaobj.UFDT[fd]->WriteOffset] = '\0';

  return size;
}
////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     read_file
//  Description   :     It is used to read the content from the file
//  Input         :     File_descriptor
//                      Address of empty buffer which contains data
//                      Size of data that we want to read
//  Output        :     Number of bytes sucessfully read from the file
//  Author        :     Aditi Dipak Thorat
//  Date          :     15/08/2025
//
////////////////////////////////////////////////////////////////////////////////
int read_file(
    int fd,     // File Descriptor
    char *data, // Address of empty buffer
    int size    // Number of bytes that we want to read
)
{
  int bytesToRead = 0;

  // Invalid value of fd
  if (fd < 0 || fd >= MAXOPENEDFILES)
  {
    return ERR_INVALID_PARAMETER;
  }

  if (data == NULL || size <= 0)
  {
    return ERR_INVALID_PARAMETER;
  }

  // File is not opened with the given fd
  if (uareaobj.UFDT[fd] == NULL)
  {
    return ERR_FILE_NOT_EXIST;
  }

  // If there is no permission to read the data from the file
  if (uareaobj.UFDT[fd]->ptrinode->Permission < READ)
  {
    return ERR_PERMISSION_DENIED;
  }

  // If file actually has less data than requested
  int availableData = uareaobj.UFDT[fd]->ptrinode->ActualFileSize - uareaobj.UFDT[fd]->ReadOffset;
  if (availableData <= 0)
  {
    return ERR_INSUFFICIENT_DATA;
  }

  bytesToRead = (availableData < size) ? availableData : size;

  strncpy(data, uareaobj.UFDT[fd]->ptrinode->Buffer + uareaobj.UFDT[fd]->ReadOffset, bytesToRead);

  // Add null terminator so no garbage characters appear
  data[bytesToRead] = '\0';

  // Update offset
  uareaobj.UFDT[fd]->ReadOffset += bytesToRead;

  return bytesToRead;
}
////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     chmod_file
//  Description   :     It is used to change the permissions of the file
//  Input         :     File name and new permission
//  Output        :     Returns an integer
//  Author        :     Aditi Dipak Thorat
//  Date          :     23/08/2025
//
////////////////////////////////////////////////////////////////////////////////
int chmod_file(
    char *name,    // Name of the file
    int permission // New permission to set
)
{
  PINODE temp = head;

  // Check if file name is valid
  if (name == NULL)
  {
    return ERR_INVALID_PARAMETER;
  }

  // Check if permission is valid
  if (permission < 1 || permission > 3)
  {
    return ERR_INVALID_PARAMETER;
  }

  // Check if file exists
  if (IsFileExists(name) == false)
  {
    return ERR_FILE_NOT_EXIST;
  }

  // Find the file and update permission
  while (temp != NULL)
  {
    if ((strcmp(name, temp->FileName) == 0) && (temp->FileType == REGULARFILE))
    {
      temp->Permission = permission;
      break;
    }
    temp = temp->next;
  }

  return EXECUTE_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     OpenFile
//  Description   :     It is used to open an existing file
//  Input         :     File name and mode
//  Output        :     Returns the file descriptor
//  Author        :     Aditi Dipak Thorat
//  Date          :     23/08/2025
//
////////////////////////////////////////////////////////////////////////////////
int OpenFile(
    char *name, // Name of the file
    int mode    // Mode to open the file
)
{
  PINODE temp = head;
  int i = 0;

  // Check if file name is valid
  if (name == NULL)
  {
    return ERR_INVALID_PARAMETER;
  }

  // Check if mode is valid
  if (mode < 1 || mode > 3)
  {
    return ERR_INVALID_PARAMETER;
  }

  // Check if file exists
  if (IsFileExists(name) == false)
  {
    return ERR_FILE_NOT_EXIST;
  }

  // Find the file
  while (temp != NULL)
  {
    if ((strcmp(name, temp->FileName) == 0) && (temp->FileType == REGULARFILE))
    {
      break;
    }
    temp = temp->next;
  }

  // Find free file descriptor
  for (i = 0; i < MAXOPENEDFILES; i++)
  {
    if (uareaobj.UFDT[i] == NULL)
    {
      break;
    }
  }

  if (i == MAXOPENEDFILES)
  {
    printf("Unable to open file as MAX OPENED FILE LIMIT REACHED\n");
    return -1;
  }

  // Allocate memory for file table
  uareaobj.UFDT[i] = (PFILETABLE)malloc(sizeof(FILETABLE));

  // Initialize file table
  uareaobj.UFDT[i]->ReadOffset = 0;
  uareaobj.UFDT[i]->WriteOffset = 0;
  uareaobj.UFDT[i]->Count = 1;
  uareaobj.UFDT[i]->Mode = mode;
  uareaobj.UFDT[i]->ptrinode = temp;

  // Increment reference count
  temp->ReferenceCount++;

  return i;
}
////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     CloseFile
//  Description   :     It is used to close an opened file
//  Input         :     File descriptor
//  Output        :     Returns an integer
//  Author        :     Aditi Dipak Thorat
//  Date          :     23/08/2025
//
////////////////////////////////////////////////////////////////////////////////
int CloseFile(
    int fd // file descriptor
)
{
  // Validate fd
  if (fd < 0 || fd >= MAXOPENEDFILES)
    return ERR_INVALID_PARAMETER;

  // Check if file is opened
  if (uareaobj.UFDT[fd] == NULL)
    return ERR_FILE_NOT_EXIST;

  // Decrement filetable reference count
  uareaobj.UFDT[fd]->Count--;

  // If no more references, clean up
  if (uareaobj.UFDT[fd]->Count == 0)
  {
    if (uareaobj.UFDT[fd]->ptrinode != NULL)
    {
      uareaobj.UFDT[fd]->ptrinode->ReferenceCount--;
    }
    free(uareaobj.UFDT[fd]);
    uareaobj.UFDT[fd] = NULL;
  }

  return EXECUTE_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////
//
//  Function Name : fstat_file
//  Description   : Displays statistical information of a file using its file
//                  descriptor.
//  Input         : File descriptor of the file
//  Output        : Returns an integer status code
//  Author        : Aditi Dipak Thorat
//  Date          : 23/08/2025
//
///////////////////////////////////////////////////////////////////////////////

int fstat_file(
    int fd // File descriptor
)
{
  // Check if file descriptor is valid
  if (fd < 0 || fd >= MAXOPENEDFILES)
  {
    return ERR_INVALID_PARAMETER;
  }

  // Check if file is opened
  if (uareaobj.UFDT[fd] == NULL)
  {
    return ERR_FILE_NOT_EXIST;
  }

  PINODE temp = uareaobj.UFDT[fd]->ptrinode;

  printf("-------------------------------------------------------------------------------------------\n");
  printf("--------------------------Statistical Information of the File -----------------------------\n");
  printf("File Name : %s\n", temp->FileName);
  printf("File Size on disk : %d Bytes\n", temp->FileSize);
  printf("Actual File size : %d Bytes\n", temp->ActualFileSize);
  printf("Link Count : %d\n", temp->LinkCount);
  printf("File Permission : ");
  if (temp->Permission == READ)
  {
    printf("Read\n");
  }
  else if (temp->Permission == WRITE)
  {
    printf("Write\n");
  }
  else if (temp->Permission == READ + WRITE)
  {
    printf("Read + Write\n");
  }
  printf("File Type :");
  if (temp->FileType == REGULARFILE)
  {
    printf("Regular File \n");
  }
  else if (temp->FileType == SPECIALFILE)
  {
    printf("Sepcial File\n");
  }
  printf("-------------------------------------------------------------------------------------------\n");

  return EXECUTE_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////
//
//  Function Name : lseek_file
//  Description   : Changes the read/write offsets in an open file.
//  Input         : File descriptor
//                  Offset value to move
//                  Reference point for offset (START, CURRENT, END)
//  Output        : Returns an integer status code
//  Author        : Aditi Dipak Thorat
//  Date          : 24/08/2025
//
///////////////////////////////////////////////////////////////////////////////

int lseek_file(
    int fd,     // File descriptor
    int offset, // Offset to move
    int whence  // Position to move from
)
{
  // Check if file descriptor is valid
  if (fd < 0 || fd >= MAXOPENEDFILES)
  {
    return ERR_INVALID_PARAMETER;
  }

  // Check if file is opened
  if (uareaobj.UFDT[fd] == NULL)
  {
    return ERR_FILE_NOT_EXIST;
  }

  // Check if whence is valid
  if (whence < START || whence > END)
  {
    return ERR_INVALID_PARAMETER;
  }

  // Calculate new offset
  int newOffset = 0;
  if (whence == START)
  {
    newOffset = offset;
  }
  else if (whence == CURRENT)
  {
    newOffset = uareaobj.UFDT[fd]->ReadOffset + offset;
  }
  else if (whence == END)
  {
    newOffset = uareaobj.UFDT[fd]->ptrinode->ActualFileSize + offset;
  }

  // Check if new offset is valid
  if (newOffset < 0 || newOffset > uareaobj.UFDT[fd]->ptrinode->ActualFileSize)
  {
    return ERR_INVALID_PARAMETER;
  }

  // Update read and write offsets
  uareaobj.UFDT[fd]->ReadOffset = newOffset;
  uareaobj.UFDT[fd]->WriteOffset = newOffset;

  return EXECUTE_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//
//  Function Name : find_file
//  Description   : Searches for a file in the file system.
//  Input         : Name of the file to be searched
//  Output        : Returns an integer status code
//  Author        : Aditi Dipak Thorat
//  Date          : 24/08/2025
//
///////////////////////////////////////////////////////////////////////////////

int find_file(
    char *name // Name of the file
)
{
  // Check if file name is valid
  if (name == NULL)
  {
    return ERR_INVALID_PARAMETER;
  }

  // Check if file exists
  if (IsFileExists(name))
  {
    printf("File %s found in the file system\n", name);
    return EXECUTE_SUCCESS;
  }
  else
  {
    return ERR_FILE_NOT_EXIST;
  }
}

///////////////////////////////////////////////////////////////////////////////
//
//  Function Name : SaveToSecondaryMemory
//  Description   : Saves the entire file system to secondary memory (text file).
//  Input         : None
//  Output        : None
//  Author        : Aditi Dipak Thorat
//  Date          : 24/08/2025
//
///////////////////////////////////////////////////////////////////////////////

void SaveToSecondaryMemory()
{
  FILE *fp = fopen("SecondaryMemory.txt", "w");
  if (fp == NULL)
  {
    cout << "Error: Unable to open secondary memory file\n";
    return;
  }

  PINODE temp = head;
  while (temp != NULL)
  {
    if (temp->FileType != 0) // only used inodes
    {
      fprintf(fp, "INODE\n");
      fprintf(fp, "FileName: %s\n", temp->FileName);
      fprintf(fp, "FileType: %d\n", temp->FileType);
      fprintf(fp, "FileSize: %d\n", temp->FileSize);
      fprintf(fp, "ActualFileSize: %d\n", temp->ActualFileSize);
      fprintf(fp, "LinkCount: %d\n", temp->LinkCount);
      fprintf(fp, "ReferenceCount: %d\n", temp->ReferenceCount);
      fprintf(fp, "Permission: %d\n", temp->Permission);
      fprintf(fp, "InodeNumber: %d\n", temp->InodeNumber);

      if (temp->FileType == REGULARFILE && temp->Buffer != NULL)
      {
        // write file content as a single line
        fprintf(fp, "Buffer: %s\n", temp->Buffer);
      }
      else
      {
        fprintf(fp, "Buffer: \n");
      }

      fprintf(fp, "ENDINODE\n\n");
    }

    temp = temp->next;
  }

  // Save current directory of process
  fprintf(fp, "CURRENT_DIR: %s\n", uareaobj.CurrentDirectory);

  fclose(fp);
  cout << "All directories and files saved to secondary memory.\n";
}
///////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     get_free_inode
//  Description   :     Retrieves a free inode from the global inode list.
//  Input         :     None
//  Output        :     Returns a PINODE
//  Author        :     Aditi Dipak Thorat
//  Date          :     10/08/2025
//
///////////////////////////////////////////////////////////////////////////////

PINODE get_free_inode()
{
  PINODE temp = head;
  while (temp != NULL)
  {
    if (temp->FileType == 0)
      return temp;
    temp = temp->next;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     traverse_path
//  Description   :     It is used to  traverse path from a given start
//                      directorylist
//  Input         :     Path to traverse
//                      Starting directory inode
//  Output        :     Returns the inode (PINODE) corresponding to the last
//                      component of the path if it exists, otherwise NULL
//  Author        :     Aditi Dipak Thorat
//  Date          :     10/08/2025
//
////////////////////////////////////////////////////////////////////////////////

PINODE traverse_path(
    const char *path, // path to traverse
    PINODE start      // Starting directory inode
)
{
  if (path == NULL || start == NULL)
    return NULL;
  char tempPath[256];
  strncpy(tempPath, path, sizeof(tempPath) - 1);
  tempPath[sizeof(tempPath) - 1] = '\0';

  char *token = strtok(tempPath, "/");
  PINODE curr = start;
  while (token != NULL)
  {
    PINODE child = curr->child;
    while (child != NULL)
    {
      if (strcmp(child->FileName, token) == 0 && child->FileType == SPECIALFILE)
        break;
      child = child->sibling;
    }
    if (child == NULL)
      return NULL;
    curr = child;
    token = strtok(NULL, "/");
  }
  return curr;
}
////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     mkdir_file
//  Description   :     It is used to create a directory
//  Input         :     Directory name
//  Output        :     Returns an integer
//  Author        :     Aditi Dipak Thorat
//  Date          :     24/08/2025
//
////////////////////////////////////////////////////////////////////////////////

int mkdir_file(
    const char *path // Name of the directory
)
{
  if (path == NULL || strlen(path) == 0)
    return ERR_INVALID_PARAMETER;

  char tempPath[256];
  strncpy(tempPath, path, sizeof(tempPath) - 1);
  tempPath[sizeof(tempPath) - 1] = '\0';

  char *dirname = strrchr(tempPath, '/');
  PINODE parentDir = NULL;
  if (dirname != NULL)
  {
    *dirname = '\0';
    dirname++;
    parentDir = traverse_path(tempPath, root);
    if (parentDir == NULL || parentDir->FileType != SPECIALFILE)
      return ERR_FILE_NOT_EXIST;
  }
  else
  {
    dirname = tempPath;
    parentDir = cwd ? cwd : root;
  }

  PINODE temp = parentDir->child;
  while (temp != NULL)
  {
    if (strcmp(temp->FileName, dirname) == 0 && temp->FileType == SPECIALFILE)
      return ERR_FILE_ALREADY_EXIST;
    temp = temp->sibling;
  }

  if (superobj.FreeInodes == 0)
    return ERR_NO_INODES;

  PINODE newDir = get_free_inode();
  if (newDir == NULL)
    return ERR_NO_INODES;

  strcpy(newDir->FileName, dirname);
  newDir->FileType = SPECIALFILE;
  newDir->FileSize = MAXFILESIZE;
  newDir->ActualFileSize = 0;
  newDir->LinkCount = 1;
  newDir->ReferenceCount = 1;
  newDir->Permission = READ + WRITE;
  newDir->Buffer = (char *)malloc(MAXFILESIZE);

  newDir->parent = parentDir;
  newDir->child = NULL;
  newDir->sibling = parentDir->child;
  parentDir->child = newDir;

  superobj.FreeInodes--;

  return EXECUTE_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     rmdir_file
//  Description   :     It is used to remove a directory
//  Input         :     Directory name
//  Output        :     Returns an integer
//  Author        :     Aditi Dipak Thorat
//  Date          :     24/08/2025
//
////////////////////////////////////////////////////////////////////////////////
int rmdir_file(
    const char *path // Name of the directory
)
{
  if (path == NULL || strlen(path) == 0)
    return ERR_INVALID_PARAMETER;

  PINODE dir = traverse_path(path, root);
  if (dir == NULL || dir->FileType != SPECIALFILE)
    return ERR_FILE_NOT_EXIST;

  if (dir == root)
    return ERR_INVALID_PARAMETER;

  if (dir->child != NULL)
    return ERR_PERMISSION_DENIED;

  for (int i = 0; i < MAXOPENEDFILES; i++)
  {
    if (uareaobj.UFDT[i] != NULL && uareaobj.UFDT[i]->ptrinode == dir)
      return ERR_PERMISSION_DENIED;
  }

  PINODE parent = dir->parent;
  if (parent == NULL)
    return ERR_INVALID_PARAMETER;

  if (parent->child == dir)
  {
    parent->child = dir->sibling;
  }
  else
  {
    PINODE prev = parent->child;
    while (prev && prev->sibling != dir)
      prev = prev->sibling;
    if (prev)
      prev->sibling = dir->sibling;
  }

  if (dir->Buffer)
    free(dir->Buffer);

  dir->FileSize = 0;
  dir->ActualFileSize = 0;
  dir->LinkCount = 0;
  dir->Permission = 0;
  dir->FileType = 0;
  dir->ReferenceCount = 0;
  strcpy(dir->FileName, "");

  superobj.FreeInodes++;

  return EXECUTE_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     cd_file
//  Description   :     It is used to change current directory
//  Input         :     Directory name
//  Output        :     Returns an integer
//  Author        :     Aditi Dipak Thorat
//  Date          :     24/08/2025
//
////////////////////////////////////////////////////////////////////////////////

int cd_file(
    const char *path // Name of the directory
)
{
  if (path == NULL || strlen(path) == 0)
    return ERR_INVALID_PARAMETER;

  if (strcmp(path, "..") == 0)
  {
    if (cwd && cwd->parent != NULL)
      cwd = cwd->parent;
    return EXECUTE_SUCCESS;
  }
  if (strcmp(path, ".") == 0)
    return EXECUTE_SUCCESS;

  PINODE target = NULL;
  if (path[0] == '/')
    target = traverse_path(path, root);
  else
    target = traverse_path(path, cwd ? cwd : root);

  if (target == NULL || target->FileType != SPECIALFILE)
    return ERR_FILE_NOT_EXIST;

  cwd = target;
  strncpy(uareaobj.CurrentDirectory, target->FileName, sizeof(uareaobj.CurrentDirectory) - 1);
  uareaobj.CurrentDirectory[sizeof(uareaobj.CurrentDirectory) - 1] = '\0';

  return EXECUTE_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////
//
//  Entry point function of project (main)
//
////////////////////////////////////////////////////////////////////////////////
int main()
{
  char str[80] = {'\0'};
  int iCount = 0;
  int iRet = 0;
  char Command[5][80];
  char InputBuffer[MAXFILESIZE] = {'\0'};
  char *EmptyBuffer = nullptr;
  StartAuxillaryDataInitialisation();

  cout << "---------------------------------------------------------\n";
  cout << "--------- Custom Virtual File System Started Succesfully ------------\n";
  cout << "----------------------------------------------------------\n";

  while (1)
  {
    fflush(stdin);
    strcpy(str, "");
    printf("Custom Virtual File System%s > ", uareaobj.CurrentDirectory);

    fgets(str, sizeof(str), stdin);

    iCount = sscanf(str, "%s %s %s %s", Command[0], Command[1], Command[2], Command[3]);
    fflush(stdin);
    if (iCount == 1)
    {
      if (strcmp(Command[0], "exit") == 0)
      {
        SaveToSecondaryMemory();
        printf("Thank you for using Custom Virtual File System\n");
        printf("Deallocating all resources...\n");
        break;
      }
      else if (strcmp(Command[0], "help") == 0)
      {
        DisplayHelp();
      }
      else if (strcmp(Command[0], "clear") == 0)
      {
        system("cls");
      }
      else if (strcmp(Command[0], "ls") == 0)
      {
        ls_file();
      }
      else
      {
        printf("Command not found...\n");
        printf("Please refer Help option or use man command\n");
      }
    }
    else if (iCount == 2)
    {
      if (strcmp(Command[0], "man") == 0)
      {
        ManPage(Command[1]);
      }
      else if (strcmp(Command[0], "unlink") == 0)
      {
        iRet = UnlinkFile(Command[1]);
        if (iRet == EXECUTE_SUCCESS)
        {
          printf("Unlink operation is sucessfully performed\n");
        }
        else if (iRet == ERR_FILE_NOT_EXIST)
        {
          printf("Error : Unable to do unlink activity as file is not present\n");
        }
        else if (iRet == ERR_INVALID_PARAMETER)
        {
          printf("Error : Invalid parameters for the function\n");
          printf("Please check Man page for more details\n");
        }
      }
      else if (strcmp(Command[0], "stat") == 0)
      {
        iRet = stat_file(Command[1]);
        if (iRet == ERR_FILE_NOT_EXIST)
        {
          printf("Error : Unable to display statistics as file is not present\n");
        }
        else if (iRet == ERR_INVALID_PARAMETER)
        {
          printf("Error : Invalid parameters for the function\n");
          printf("Please check Man page for more details\n");
        }
      }
      else if (strcmp(Command[0], "write") == 0)
      {
        printf("Please Enter the data that you want to write into the file:\n");
        fgets(InputBuffer, MAXFILESIZE, stdin);
        iRet = write_file(atoi(Command[1]), InputBuffer, strlen(InputBuffer) - 1);
        if (iRet == ERR_INSUFFECIENT_MEMORY)
        {
          printf("Error : Insufficent memory for data block of the file \n");
        }
        else if (iRet == ERR_PERMISSION_DENIED)
        {
          printf("Error : Unable to write as no permission to write\n");
        }
        else if (iRet == ERR_INVALID_PARAMETER)
        {
          printf("Error : Invalid parameters for the function\n");
          printf("Please check Man page for more details\n");
        }
        else if (iRet == ERR_FILE_NOT_EXIST)
        {
          printf("Error : Fd is invalid\n");
        }
        else
        {
          printf("%d Bytes gets successfully written in the file \n", iRet);
          printf("Data from file is %s \n ", uareaobj.UFDT[atoi(Command[1])]->ptrinode->Buffer);
        }
      }
      else if (strcmp(Command[0], "close") == 0)
      {
        iRet = CloseFile(atoi(Command[1]));
        if (iRet == EXECUTE_SUCCESS)
        {
          printf("File closed successfully\n");
        }
        else if (iRet == ERR_FILE_NOT_EXIST)
        {
          printf("Error : Unable to close as file is not open\n");
        }
        else if (iRet == ERR_INVALID_PARAMETER)
        {
          printf("Error : Invalid parameters for the function\n");
          printf("Please check Man page for more details\n");
        }
      }
      else if (strcmp(Command[0], "fstat") == 0)
      {
        iRet = fstat_file(atoi(Command[1]));
        if (iRet == ERR_FILE_NOT_EXIST)
        {
          printf("Error : Unable to display statistics as file is not open\n");
        }
        else if (iRet == ERR_INVALID_PARAMETER)
        {
          printf("Error : Invalid parameters for the function\n");
          printf("Please check Man page for more details\n");
        }
      }
      else if (strcmp(Command[0], "mkdir") == 0)
      {
        iRet = mkdir_file(Command[1]);
        if (iRet == EXECUTE_SUCCESS)
        {
          printf("Directory created successfully\n");
        }
        else if (iRet == ERR_FILE_ALREADY_EXIST)
        {
          printf("Error : Unable to create directory as it already exists\n");
        }
        else if (iRet == ERR_NO_INODES)
        {
          printf("Error : Unable to create directory as there are no free inodes\n");
        }
        else if (iRet == ERR_INVALID_PARAMETER)
        {
          printf("Error : Invalid parameters for the function\n");
          printf("Please check Man page for more details\n");
        }
      }
      else if (strcmp(Command[0], "rmdir") == 0)
      {
        iRet = rmdir_file(Command[1]);
        if (iRet == EXECUTE_SUCCESS)
        {
          printf("Directory removed successfully\n");
        }
        else if (iRet == ERR_FILE_NOT_EXIST)
        {
          printf("Error : Unable to remove directory as it does not exist\n");
        }
        else if (iRet == ERR_INVALID_PARAMETER)
        {
          printf("Error : Invalid parameters for the function\n");
          printf("Please check Man page for more details\n");
        }
      }
      else if (strcmp(Command[0], "cd") == 0)
      {
        iRet = cd_file(Command[1]);
        if (iRet == EXECUTE_SUCCESS)
        {
          printf("Changed to directory %s\n", Command[1]);
        }
        else if (iRet == ERR_FILE_NOT_EXIST)
        {
          printf("Error : Unable to change directory as it does not exist\n");
        }
        else if (iRet == ERR_INVALID_PARAMETER)
        {
          printf("Error : Invalid parameters for the function\n");
          printf("Please check Man page for more details\n");
        }
      }
      else if (strcmp(Command[0], "find") == 0)
      {
        iRet = find_file(Command[1]);
        if (iRet == ERR_FILE_NOT_EXIST)
        {
          printf("Error : File %s not found\n", Command[1]);
        }
        else if (iRet == ERR_INVALID_PARAMETER)
        {
          printf("Error : Invalid parameters for the function\n");
          printf("Please check Man page for more details\n");
        }
      }
      else
      {
        printf("Command not found...\n");
        printf("Please refer Help option or use man command\n");
      }
    }
    else if (iCount == 3)
    {
      if (strcmp(Command[0], "creat") == 0)
      {
        iRet = CreateFile(Command[1], atoi(Command[2]));
        if (iRet == ERR_INVALID_PARAMETER)
        {
          printf("Error : Invalid parameters for the function\n");
          printf("Please check Man page for more details\n");
        }
        else if (iRet == ERR_NO_INODES)
        {
          printf("Error : Unable to create file as there is no Inodes\n");
        }
        else if (iRet == ERR_FILE_ALREADY_EXIST)
        {
          printf("Error : Unable to create file as file is already existing\n");
        }
        else
        {
          printf("File is succesfully created with FD : %d\n", iRet);
        }
      }
      else if (strcmp(Command[0], "read") == 0)
      {
        EmptyBuffer = (char *)malloc(atoi(Command[2]) + 1);
        iRet = read_file(atoi(Command[1]), EmptyBuffer, atoi(Command[2]));
        if (iRet == ERR_INSUFFICIENT_DATA)
        {
          printf("Error : Insufficent memory in data block of the file \n");
        }
        else if (iRet == ERR_PERMISSION_DENIED)
        {
          printf("Error : Unable to read as no read permission \n");
        }
        else if (iRet == ERR_INVALID_PARAMETER)
        {
          printf("Error : Invalid parameters for the function\n");
          printf("Please check Man page for more details\n");
        }
        else if (iRet == ERR_FILE_NOT_EXIST)
        {
          printf("Error : Fd is invalid\n");
        }
        else
        {
          printf("Read operation is successfull \n");
          printf("Data from file is : %s \n", EmptyBuffer);
          free(EmptyBuffer);
        }
      }
      else if (strcmp(Command[0], "chmod") == 0)
      {
        iRet = chmod_file(Command[1], atoi(Command[2]));
        if (iRet == EXECUTE_SUCCESS)
        {
          printf("File permissions changed successfully\n");
        }
        else if (iRet == ERR_FILE_NOT_EXIST)
        {
          printf("Error : Unable to change permissions as file does not exist\n");
        }
        else if (iRet == ERR_INVALID_PARAMETER)
        {
          printf("Error : Invalid parameters for the function\n");
          printf("Please check Man page for more details\n");
        }
      }
      else if (strcmp(Command[0], "open") == 0)
      {
        iRet = OpenFile(Command[1], atoi(Command[2]));
        if (iRet == ERR_INVALID_PARAMETER)
        {
          printf("Error : Invalid parameters for the function\n");
          printf("Please check Man page for more details\n");
        }
        else if (iRet == ERR_FILE_NOT_EXIST)
        {
          printf("Error : Unable to open file as it does not exist\n");
        }
        else
        {
          printf("File opened successfully with FD : %d\n", iRet);
        }
      }
      else
      {
        printf("Command not found...\n");
        printf("Please refer Help option or use man command\n");
      }
    }
    else if (iCount == 4)
    {
      if (strcmp(Command[0], "lseek") == 0)
      {
        iRet = lseek_file(atoi(Command[1]), atoi(Command[2]), atoi(Command[3]));
        if (iRet == EXECUTE_SUCCESS)
        {
          printf("Offset changed successfully\n");
        }
        else if (iRet == ERR_FILE_NOT_EXIST)
        {
          printf("Error : Unable to change offset as file is not open\n");
        }
        else if (iRet == ERR_INVALID_PARAMETER)
        {
          printf("Error : Invalid parameters for the function\n");
          printf("Please check Man page for more details\n");
        }
      }
      else
      {
        printf("Command not found...\n");
        printf("Please refer Help option or use man command\n");
      }
    }
    else
    {
      printf("Command not found...\n");
      printf("Please refer Help option or use man command\n");
    }
  }

  // Cleanup before exit
  PINODE temp = head;
  while (temp != NULL)
  {
    if (temp->Buffer != NULL)
    {
      free(temp->Buffer);
    }
    PINODE next = temp->next;
    delete temp;
    temp = next;
  }
  for (int i = 0; i < MAXOPENEDFILES; i++)
  {
    if (uareaobj.UFDT[i] != NULL)
    {
      free(uareaobj.UFDT[i]);
    }
  }

  return 0;
}