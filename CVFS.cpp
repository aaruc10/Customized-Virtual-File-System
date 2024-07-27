#include<stdio.h>//1
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<iostream>
//<include<io.h>

//macros
#define MAXINODE 50//8

#define READ 1 
#define WRITE 2

#define MAXFILESIZE 1024//file chi size define keli

#define REGULAR 1
#define SPECIAL 2

#define START 0
#define CURRENT 1
#define END 2

typedef struct superblock//22
{
    int TotalInodes;
    int FreeInode;
}SUPERBLOCK,*PSUPERBLOCK;

typedef struct inode//28
{
    char FileName[50];
    int InodeNumber;
    int FileSize;
    int FileActualSize;
    int FileType;
    char *Buffer;//buffer pointer pointing to some address where character is stored
    int LinkCount;
    int ReferenceCount;
    int permission;//1 2 3 
    struct inode*next;//next pointer used to store address of next inode 
}INODE,*PINODE,**PPINODE;

typedef struct filetable//42
{
    int readoffset;
    int writeoffset;
    int count;
    int mode;//1 2 3 
    PINODE ptrinode;
}FILETABLE,*PFILETABLE;

typedef struct ufdt//51
{
    PFILETABLE ptrfiletable;
}UFDT;

UFDT UFDTArr[50];
SUPERBLOCK SUPERBLOCKobj;
PINODE head=NULL;

void man(char*name)//60
{
    if(name==NULL)return;

    if(strcmp(name,"create")==0)
    {
        printf("Description :Used to create new regular file\n");
        printf("Usage;create File_Name Permission\n");
    }
    else if(strcmp(name,"read")==0)
    {
        printf("Description: Used to read data from regular file\n");
        printf("Usage: read File_Name No_Of_Bytes_To_Read\n");
    }
    else if(strcmp(name,"write")==0)
    {
        printf("Used to write into regulat file\n");
        printf("Usage:write File_NAme\nAfter this enter the data that we want to write\n");
    }
    else if(strcmp(name,"ls")==0)
    {
        printf("Description: Used to list all information of  files\n ");
        printf("Usage:ls\n");
    }
    else if(strcmp(name,"stat")==0)
    {
        printf("Description:Used to display information about file");
        printf("Usage:stat File_Name\n");
    }
    else if(strcmp(name,"truncate")==0)
    {
        printf("Description:Used to remove data from file\n");
        printf("Usage:truncate File_Name\n");
    }
    else if(strcmp(name,"open")==0)
    {
        printf("Description:Used to open existing file\n");
        printf("Usage:open File_Name mode\n");
    }
    else if(strcmp(name,"close")==0)
    {
        printf("Description:Used to close opened file\n");
        printf("Usage:close File_Name \n");
    }
    else if(strcmp(name,"rm")==0)
    {
        printf("Description:Used to delete the file\n");
        printf("Usage:rm File_Name\n");
    }
    else
    {
        printf("ERROR:No manual entry available\n ");
    }
}
void DisplayHelp()//130
{
    printf("ls:To list out all files\n");
    printf("clear:To clear console\n");
    printf("create:To create new files\n");
    printf("open:To open the files\n");
    printf("close:To close the files\n");
    printf("read:To read content from files\n");
    printf("write:To write contents into files\n");
    printf("exit:To terminate file system\n");
    printf("stat:To display information of file using name\n");
    printf("truncate:To remove all data from  files\n");
    printf("rm:To Delete the files\n");
}

int GetFDFromName(char*name)//146
{
    int i=0;

    while(i<50)
    {
        if(UFDTArr[i].ptrfiletable!=NULL)
           if(strcmp((UFDTArr[i].ptrfiletable->ptrinode->FileName),name)==0)
             break;
        
        i++;
    }
    if(i==50)
      return -1;
    else
      return i;   
}
PINODE Get_Inode(char*name)//162
{
    PINODE temp=head;
    int i=0;

    if(name==NULL)
     return NULL;

    while(temp!=NULL)
    {
        if(strcmp(name,temp->FileName)==0)
          break;
        temp=temp->next;
    }
    return temp;
}
void CreateDILB()//179
{
    int i=1;
    PINODE newn=NULL;
    PINODE temp=head;
   
    while(i<=MAXINODE)
    {
        newn=(PINODE)malloc(sizeof(INODE));
        
        newn->LinkCount=0;
        newn->ReferenceCount=0;
        newn->FileType=0;
        newn->FileSize=0;

        newn->Buffer=NULL;
        newn->next=NULL;

        newn->InodeNumber=i;

        if(temp==NULL)
        {
            head=newn;
            temp=head;
        }
        else
        {
            temp->next=newn;
            temp=temp->next;
        }
        i++;
    }
    printf("DILB Created Sucessfully\n");

}
void InitialiseSuperBlock()//214
{
    int i=0;
    while(i<MAXINODE)
    {
        UFDTArr[i].ptrfiletable=NULL;
        i++;
    }
    SUPERBLOCKobj.TotalInodes=MAXINODE;
    SUPERBLOCKobj.FreeInode=MAXINODE;

}
int CreateFile(char*name,int permission)//227
{
    int i=0;
    PINODE temp=head;

    if((name==NULL)||(permission==0)||(permission>3))
      return -1;

    if(SUPERBLOCKobj.FreeInode==0)
      return -2;

    (SUPERBLOCKobj.FreeInode)--;

    if(Get_Inode(name)!=NULL)
      return -3;

    while(temp!=NULL)
    {
        if(temp->FileType==0)
           break;
        temp=temp->next;
    }

    while(i<50)
    {
        if(UFDTArr[i].ptrfiletable==NULL)
          break;
        i++;
    }

    UFDTArr[i].ptrfiletable=(PFILETABLE)malloc(sizeof(FILETABLE));

    UFDTArr[i].ptrfiletable->count=1;
    UFDTArr[i].ptrfiletable->mode=permission;
    UFDTArr[i].ptrfiletable->readoffset=0;
    UFDTArr[i].ptrfiletable->writeoffset=0;

    UFDTArr[i].ptrfiletable->ptrinode=temp;

    strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName,name);
    UFDTArr[i].ptrfiletable->ptrinode->FileType=REGULAR;
    UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount=1;
    UFDTArr[i].ptrfiletable->ptrinode->LinkCount=1;
    UFDTArr[i].ptrfiletable->ptrinode->FileSize=MAXFILESIZE;
    UFDTArr[i].ptrfiletable->ptrinode->FileActualSize=0;
    UFDTArr[i].ptrfiletable->ptrinode->permission=permission;
    UFDTArr[i].ptrfiletable->ptrinode->Buffer=(char*)malloc(MAXFILESIZE);

    return i;
}
int rm_File(char*name)//278
{
    int fd=0;

    fd=GetFDFromName(name);
    if(fd==-1)
      return -1;

    (UFDTArr[fd].ptrfiletable->ptrinode->LinkCount==0);
    {
        UFDTArr[fd].ptrfiletable->ptrinode->FileType=0;
        //free(UFDTArr[i].ptrfiletable->Buffer)
        free(UFDTArr[fd].ptrfiletable);
    }
    UFDTArr[fd].ptrfiletable=NULL;
    (SUPERBLOCKobj.FreeInode)++;
}
int ReadFile(int fd,char*arr,int isize)//300
{
    int read_size=0;

    if(UFDTArr[fd].ptrfiletable==NULL)
      return -1;

    if(UFDTArr[fd].ptrfiletable->mode!=READ && UFDTArr[fd].ptrfiletable->mode!=READ+WRITE)
      return -2;

    if(UFDTArr[fd].ptrfiletable->ptrinode->permission!=READ && UFDTArr[fd].ptrfiletable->ptrinode->permission!=READ+WRITE)
      return -2;

    if(UFDTArr[fd].ptrfiletable->readoffset==UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
      return -3;

    if(UFDTArr[fd].ptrfiletable->ptrinode->FileType!=REGULAR)
      return -4;

    read_size=(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)-(UFDTArr[fd].ptrfiletable->readoffset);
      if(read_size<isize)
      {
        strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->readoffset),read_size);
        (UFDTArr[fd].ptrfiletable->readoffset)=(UFDTArr[fd].ptrfiletable->readoffset)+read_size;
      }
      else{
        strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->readoffset),isize);

        (UFDTArr[fd].ptrfiletable->readoffset)=(UFDTArr[fd].ptrfiletable->readoffset)+isize;
      }
      return isize;
}
int WriteFile(int fd,char*arr,int isize)//331
{
    if(((UFDTArr[fd].ptrfiletable->mode)!=WRITE)&&((UFDTArr[fd].ptrfiletable->mode)!=READ+WRITE))
      return -1;
    if(((UFDTArr[fd].ptrfiletable->ptrinode->permission)!=WRITE)&&((UFDTArr[fd].ptrfiletable->ptrinode->permission)!=READ+WRITE))
      return -1;
    if((UFDTArr[fd].ptrfiletable->writeoffset)==MAXFILESIZE)
      return -2;
    if((UFDTArr[fd].ptrfiletable->ptrinode->FileType)!=REGULAR)
      return -3;
    
    strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->writeoffset),arr,isize);

    (UFDTArr[fd].ptrfiletable->writeoffset)=(UFDTArr[fd].ptrfiletable->writeoffset)+isize;

    (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)=(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+isize;

    return isize;

}
int OpenFile(char *name,int mode)//350
{
    int i=0;
    PINODE temp=NULL;

    if(name==NULL||mode<=0)
      return -1;

    temp=Get_Inode(name);
    if(temp==NULL)
      return -2;

    if(temp->permission<mode)
      return -3;

    while(i<50)
    {
        if(UFDTArr[i].ptrfiletable==NULL)
          break;
        i++;
    }
    UFDTArr[i].ptrfiletable=(PFILETABLE)malloc(sizeof(FILETABLE));
    if(UFDTArr[i].ptrfiletable==NULL)
      return -1;
    UFDTArr[i].ptrfiletable->count=1;
    UFDTArr[i].ptrfiletable->mode=mode;
    if(mode==READ+WRITE)
    {
        UFDTArr[i].ptrfiletable->readoffset=0;
        UFDTArr[i].ptrfiletable->writeoffset=0;
    }
    else if(mode==READ)
    {
        UFDTArr[i].ptrfiletable->readoffset=0;
    }
    else if(mode=WRITE)
    {
        UFDTArr[i].ptrfiletable->writeoffset=0;
    }
    UFDTArr[i].ptrfiletable->ptrinode=temp;
    (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)++;

    return i;

}
void CloseFileByName(int fd)//402
{

    UFDTArr[fd].ptrfiletable->readoffset=0;
    UFDTArr[fd].ptrfiletable->writeoffset=0;
    (UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount)--;

    
}
int CloseFileByName(char *name)//416
{
  int i=0;
  i=GetFDFromName(name);
  if(i==-1)
    return -1;

    UFDTArr[i].ptrfiletable->readoffset=0;
    UFDTArr[i].ptrfiletable->writeoffset=0;
    (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;

    return 0;
}
void ls_file()//485
{
    int i=0;
    PINODE temp=head;

    if(SUPERBLOCKobj.FreeInode==MAXINODE)
    {
        printf("ERROR:There are no files\n");
        return ;
    }
    printf("\nFile Name\tInode Number\t File size\tLink count\n");
    printf("...............................................................\n");

    while(temp!=NULL)
    {
        if(temp->FileType !=0)
        {
            printf("%s\t\t%d\t\t%d\t\t%d\n",temp->FileName,temp->InodeNumber,temp->FileActualSize,temp->LinkCount);
        }
        temp=temp->next;
    }
    printf("...............................................................\n");
}

int stat_file(char*name)//539
{
    PINODE temp=head;
    int i=0;

    if(name==NULL)
      return -1;

    while(temp!=NULL)
    {
        if(strcmp(name,temp->FileName)==0)
          break;
        temp=temp->next;
    }

    if(temp==NULL)
      return -2;

    printf("\n..........Statistical Information about file.............\n");
    printf("File name:%s\n",temp->FileName);
    printf("Inode Number:%d\n",temp->InodeNumber);
    printf("File Size:%d\n",temp->FileSize);
    printf("Actual File size:%d\n",temp->FileActualSize);
    printf("Link Count:%d\n",temp->LinkCount);
    printf("Reference Count:%d\n",temp->ReferenceCount);

    if(temp->permission==1)
      printf("File Permission:Read Only\n");
    if(temp->permission==2)
      printf("File Permission:Write Only\n");
    if(temp->permission==3)
      printf("File Permission:Read & Write\n");
    printf("........................................................\n\n");

    return 0;
}
int truncate_File(char*name)//574
{
    int fd=GetFDFromName(name);
    if(fd==-1)
      return -1;

     memset(UFDTArr[fd].ptrfiletable->ptrinode->Buffer,0,1024);
     UFDTArr[fd].ptrfiletable->readoffset=0;
     UFDTArr[fd].ptrfiletable->writeoffset=0;
     UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize=0;
}

int main()//586
{
    char *ptr=NULL;//
    int ret=0;
    int fd=0;
    int count=0;
    char command[4][80],str[80],arr[1024];

    InitialiseSuperBlock();//
    CreateDILB();//

    while(1)
    {
        fflush(stdin);//
        strcpy(str,"");

        printf("\n Aarti's VFS:>");

        fgets(str,80,stdin);//scanf("%[^'\n']s",str);

        count=sscanf(str,"%s %s %s %s",command[0],command[1],command[2],command[3]);

        if(count==1)
        {
            if(strcmp(command[0],"ls")==0)
            {
                ls_file();//
                
            }
            else if(strcmp(command[0],"clear")==0)
            {
                system("cls");
                continue;
            }
            else if (strcmp(command[0],"help")==0)
            {
              DisplayHelp();//
              continue;
            }
            else if(strcmp(command[0],"exit")==0)
            {
                printf("Terminating the Marvellous Virtual File System\n");
                break;
            }
            else
            {
                printf("\nERROR: Command not found!!!\n");
                continue;
            }
        }
        else if(count==2)
        {
            if(strcmp(command[0],"stat")==0)
            {
                ret=stat_file(command[1]);//
                if(ret==-1)
                   printf("ERROR:Incoorect parameters\n");
                if(ret==-2)
                   printf("ERROR:There is no such file\n");
                continue;
            }
            
            else if(strcmp(command[0],"close")==0)
            {
               ret=CloseFileByName(command[1]);//
               if(ret==-1)
                 printf("ERROR:There is no such file\n");
                else(printf("File Closed Sucessfully"));
               continue;   
            }
            else if(strcmp(command[0],"rm")==0)
            {
                ret=rm_File(command[1]);//
                if(ret==-1)
                  printf("ERROR:There is no such file\n");
                continue;    
            }
            else if(strcmp(command[0],"man")==0)
            {
                man(command[1]);
            }
            else if(strcmp(command[0],"write")==0)
            {
                fd=GetFDFromName(command[1]);//
                if(fd==-1)
                {
                    printf("error:Incorrect parameter\n");
                    continue;
                }
                printf("Enter the data:\n");
                scanf("%[^\n]",arr);

                ret=strlen(arr);
                if(ret==0)
                {
                    printf("ERROR:Incorrect parameter\n");
                    continue;
                }
                ret=WriteFile(fd,arr,ret);//
                if(ret==-1)
                  printf("ERROR:Permission denied\n");
                if(ret==-2)
                  printf("ERROR:There is no sufficient memory to write\n");
                if(ret==-3)
                  printf("ERROR:It is not regular file\n");
            }
            else if(strcmp(command[0],"truncate")==0)
            {
                ret=truncate_File(command[1]);//
                if(ret==-1)
                   printf("ERROR:Incorrect parameter\n");
            }
            else
            {
                printf("\nERROR:Commnad not found!!!\n");
                continue;
            }

        }
        else if(count==3)
        {
            if(strcmp(command[0],"create")==0)
            {
                ret=CreateFile(command[1],atoi(command[2]));//
                if(ret>=0)
                   printf("File is sucessfully created with the file descriptor:%d\n",ret);
                if(ret==-1)
                   printf("ERROR:Incorect parameter\n");
                if(ret==-3)
                   printf("ERROR:There is no inodes\n");
                if(ret==-4)
                   printf("ERROR:Memory allocation failure\n");
                continue;
                
            }
            else if(strcmp(command[0],"open")==0)
            {
                ret=OpenFile(command[1],atoi(command[2]));//
                if(ret>=0)
                   printf("File is sucessfully opened ");
                if(ret==-1)
                   printf("ERROR:Incorrect parameter\n");
                if(ret==-2)
                   printf("ERROR:File not present\n");
                if(ret==-3)
                   printf("ERROR:Premission denies\n");
                continue;
            }
            else if(strcmp(command[0],"read")==0)
            {
                fd=GetFDFromName(command[1]);//
                if(fd==-1)
                {
                    printf("ERROR:Incorrect Parameters\n");
                    continue;
                }
                ptr=(char*)malloc(sizeof(atoi(command[2]))+1);
                if(ptr==NULL)
                {
                    printf("ERROR:Memory allocation failure\n");
                }
                ret=ReadFile(fd,ptr,atoi(command[2]));//
                if(ret==-1)
                   printf("ERROR:File not existing\n");
                if(ret==-2)
                   printf("ERROR:Permission denied\n");
                if(ret==-3)
                   printf("ERROR:Reaches at the end of file");
                if(ret==-4)
                   printf("ERROR:It is not regular file\n");
                if(ret==0)
                   printf("ERROR:File empty\n");
                if(ret>0)
                {
                    write(2,ptr,ret);
                }
                continue;
            }
            else
            {
                printf("\nERROR:Command not found!!!\n");
                continue;
            }
        }
        
    }
    return 0;
}
