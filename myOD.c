#include <errno.h>
#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// create global structure
struct OD
   {
     bool ASCII;
     bool decimal;
     bool octal;
     bool hex;
     char *radix; 
     int bytes;
   };

void octal_dump(FILE *fp, struct OD flags)
{
  // create union to hold file data
  union DATA_OVERLAY {
    char buffer[16];
    unsigned short int shortData[8];
  } data;

  int i, len1=0, len2, fileLen;
  char c;

  // check and set initial offset into file
  if(flags.bytes>0){
    // retrieve file size
    fseek(fp,0,SEEK_END);
    fileLen=ftell(fp);
    fseek(fp,0,SEEK_SET);
    // check if offset is valid, set or notify
    if(flags.bytes<fileLen) {
      fseek(fp,flags.bytes,SEEK_SET);
      len1+=flags.bytes;
    }
    else
      fprintf(stdout,"\nOffset too large, defaulting to start of file\n\n");
  }
  
  while(1)
  {
    // read data into buffer
    for(i=0;i<16;i++) {
      c = fgetc(fp);
      if(c!=EOF)
        data.buffer[i]=c;
      else
        data.buffer[i]='\0';
    }

    len2=strlen(data.buffer)-1;

    // print the correct offset
    if(strcmp(flags.radix,"o")==0)
      fprintf(stdout,"%08o  ", len1);
    if(strcmp(flags.radix,"d")==0)
      fprintf(stdout,"%08d  ", len1);
    if(strcmp(flags.radix,"x")==0)
      fprintf(stdout,"%08x  ", len1);

    // print data
    if(flags.ASCII) {
      // check and print characters or periods
      fprintf(stdout,"\n c        ");
      for(i=0;i<(strlen(data.buffer)-1);i++) {
        if((i>0)&&(i%2==0))
          fprintf(stdout,"   ");
        else if((i>0)&&(i%2!=0))
          fprintf(stdout,"  ");
	if(isprint(data.buffer[i]))
          fprintf(stdout,"%c",data.buffer[i]);
        else
          fprintf(stdout,".");
      }
    }
    if(flags.decimal){
      fprintf(stdout,"\n d        ");
      for(i=0;i<=(strlen(data.buffer)/2-1);i++)  
        fprintf(stdout,"%06u ", data.shortData[i]);
    }
    if(flags.hex){
      fprintf(stdout,"\n x        ");
      for(i=0;i<=(strlen(data.buffer)/2-1);i++)
        fprintf(stdout,"%06x ", data.shortData[i]);
    }
    if(flags.octal){
      fprintf(stdout,"\n o        ");
      for(i=0;i<=(strlen(data.buffer)/2-1);i++)
        fprintf(stdout,"%06o ", data.shortData[i]);
    }

    // new line, keep track of offset
    fprintf(stdout,"\n");
    len1+=len2;
    
    // check if at the end of file
    if(strlen(data.buffer)<16){
      // print final offset
      if(strcmp(flags.radix,"o")==0)
        fprintf(stdout,"%08o  ", len1);
      else if(strcmp(flags.radix,"d")==0)
        fprintf(stdout,"%08d  ", len1);
      else if(strcmp(flags.radix,"x")==0)
        fprintf(stdout,"%08x  ", len1);
      fprintf(stdout,"\n\n\n");
      return;
    }
  }
}

void standardIN(struct OD flags)
{
  char buffer[256];
  FILE * pFile;
  pFile = tmpfile();
  
  // read in from standard in and store in temp file
  do {
    if (!fgets(buffer,256,stdin))
      break;
    fputs (buffer,pFile);
  } while (strlen(buffer)>1);

  rewind(pFile);

  fprintf(stdout,"\n");

  octal_dump(pFile,flags);
  fclose(pFile);

  return;
}
 
int main(int argc, char * argv[],char * envp[])
{
  // set defaults
  struct OD myOD={true, false, false, false, "o", 0};

  int flag;
  long double result;

  // check and process flags
  while((flag=getopt(argc,argv,"cdoxA:j:"))!=-1)
  {
     fprintf(stdout,"Processing cmd line arg #%d with flag %c.\n",
                    optind-1,flag); 
     switch(flag)
     {
       case 'c':
            myOD.ASCII=false;
            break;
       case 'd':
            myOD.decimal=true;
            break;
       case 'o':
            myOD.octal=true;
            break;
       case 'x':
            myOD.hex=true;
            break;
       case 'A':
            myOD.radix=strdup(optarg);
            // check if valid -A flag, else default to Octal
	    if((strcmp(myOD.radix,"o")!=0)&&
               (strcmp(myOD.radix,"d")!=0)&&
               (strcmp(myOD.radix,"x")!=0)){
              fprintf(stdout,"Invalid -A argument, defaulting to Octal\n");
              myOD.radix="o";
            }
            break;
       case 'j':
            myOD.bytes=atoi(optarg);
            break;
       default:
            perror("Ooops! Bad flag or missing argument");
            fprintf(stderr,"flag: %c  opterr: %d optopt %c\n",
                    flag, opterr, optopt);
       }
  }

     // print out flags
     fprintf(stdout,"\nThe flag -c is set to: %s\n", myOD.ASCII ? "true" : "false");
     fprintf(stdout,"The flag -d is set to: %s\n", myOD.decimal ? "true" : "false");
     fprintf(stdout,"The flag -o is set to: %s\n", myOD.octal ? "true" : "false");
     fprintf(stdout,"The flag -x is set to: %s\n", myOD.hex ? "true" : "false");
     fprintf(stdout,"The flag -A is set to: %s\n", myOD.radix);
     fprintf(stdout,"The flag -j is set to: %d\n\n\n", myOD.bytes);

  //Print out the remaining values
  for(int i=optind; i<argc; i++)
    fprintf(stdout,"%d. %s\n", i, argv[i]); 

  if(optind==argc) {
    fprintf(stdout,"Processing stdin\n");
    standardIN(myOD);
  }
  else
    while(argv[optind]) //Process each of the files
    {
      FILE *infile;
      fprintf(stdout,"Processing file: %s\n\n",argv[optind]);
      infile=fopen(argv[optind],"r");
      if(infile) {
        octal_dump(infile,myOD);
        fclose(infile);
      }
      else {
        fprintf(stderr,"Error code: %d ",errno);
        perror("Oops ");
      }
      optind++;
    }

  return 0;
}
