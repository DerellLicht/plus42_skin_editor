//********************************************************************************
//  plus42 skin editor - main file
//  License: Creative Commons CC0 1.0 Universal
//  Written by: Derell Licht
//  
//  run example: plus42_skin_editor LandscapeRightSmall LandscapeRight
//********************************************************************************
//  The initial version of this editor will assume that the target file
//  is a resize of some reference file.
//********************************************************************************

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>  //  atoi
#include <tchar.h>

// #include "header.h"
#define  MAX_PATH_LEN   1024

typedef unsigned int  uint ;

//lint -e10  Expecting '}'
//lint -esym(818, argv)  //Pointer parameter could be declared as pointing to const


static TCHAR target_file[MAX_PATH_LEN+1] = _T("") ;
static TCHAR target_file_lo[MAX_PATH_LEN+10+1] = _T("") ;
static TCHAR target_file_gif[MAX_PATH_LEN+10+1] = _T("") ;
static TCHAR ref_file[MAX_PATH_LEN+1] = _T("") ;
static TCHAR ref_file_lo[MAX_PATH_LEN+10+1] = _T("") ;
static TCHAR target_lo_backup[MAX_PATH_LEN+14+1] = _T("") ;

static uint ref_width = 0 ;         
static uint ref_height = 0 ;         
static uint target_width = 0 ;         
static uint target_height = 0 ;         
static double x_scale = 0.0 ;
static double y_scale = 0.0 ;

//********************************************************************************
static void usage(void)
{
   puts("Usage: plus42_skin_editor <target_file_name> <ref_file_name");
}

//**********************************************************************
//lint -esym(714, strip_newlines)
//lint -esym(759, strip_newlines)
//lint -esym(765, strip_newlines)
void strip_newlines(char *rstr)
{
   int slen = (int) strlen(rstr) ;
   while (1) {
      if (slen == 0)
         break;
      if (*(rstr+slen-1) == '\n'  ||  *(rstr+slen-1) == '\r') {
         slen-- ;
         *(rstr+slen) = 0 ;
      } else {
         break;
      }
   }
}

//*************************************************************
//lint -esym(714, get_system_message)
//lint -esym(759, get_system_message)
//lint -esym(765, get_system_message)
char *get_system_message(DWORD errcode)
{
   // static char msg[261] ;
   static TCHAR msg[1500] ;
   // int slen ;
   int result = (int) errcode ;
   if (result < 0) {
      result = -result ;
      errcode = (DWORD) result ;
      // wsprintfA(msg, "Win32: unknown error code %d", result) ;
      // return msg;
   }
   LPVOID lpMsgBuf;
   DWORD dresult = FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER |
      FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      errcode,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
      (LPSTR) &lpMsgBuf,
      0, 0);
   // Process any inserts in lpMsgBuf.
   // ...
   // Display the string.
   if (dresult == 0) {
      DWORD glError = GetLastError() ;
      if (glError == 317) {   //  see comment at start of function
         sprintf(msg, "FormatMessage(): no message for error code %d", result) ;
      } else {
         sprintf(msg, "FormatMessage() failed: [%u], errcode %d", (uint) GetLastError(), result) ;
      }
      
   } else
   if (lpMsgBuf == NULL) {
      sprintf(msg, "NULL buffer in response from FormatMessage() [%u]", (uint) GetLastError()) ;
   } else 
   {
      _tcsncpy(msg, (char *) lpMsgBuf, sizeof(msg)) ;
      // Free the buffer.
      LocalFree( lpMsgBuf );  //lint !e534
   }

   //  trim the newline off the message before copying it...
   strip_newlines(msg) ;

   return msg;
}

//*************************************************************
//  each subsequent call to this function overwrites
//  the previous report.
//*************************************************************
char *get_system_message(void)
{
   return get_system_message(GetLastError());
}

//********************************************************************************
static int get_skin_dimens(TCHAR *skin_file, bool isRefFile)
{
   FILE *fd = fopen(skin_file, "rt");
   if (fd == NULL) {
      printf("unable to open %s\n", skin_file);
      return 1 ;
   }
#define  MAX_LINE_LEN   80   
   TCHAR inpstr[MAX_LINE_LEN+1] ;
//Skin: 0,0,1280,656
   while (fgets(inpstr, MAX_LINE_LEN, fd) != NULL) {
      if (_tcsncmp(inpstr, "Skin:", 5) == 0) {
         TCHAR *dimens = _tcsstr(inpstr, _T("0,0,"));
         if (dimens == NULL) {
            printf("bad format: [%s]\n", inpstr);
            return 1;
         }
         
         dimens += 4 ;  //  skip past zeroes
         uint width = (uint) atoi(dimens);
         
         dimens = _tcschr(dimens, _T(','));
         if (dimens == NULL) {
            printf("bad format: [%s]\n", inpstr);
            return 1;
         }
         dimens++ ;  //  skip past comma
         uint height = (uint) atoi(dimens);
         
         //  store results in appropriate fields
         if (isRefFile) {
            ref_width = width ;
            ref_height = height ;
         }
         else {
            target_width = width ;
            target_height = height ;
         }
         return 0 ;
      }
   }
   fclose(fd);
   
   printf("Skin: field not found\n");
   return 1 ;
}

//********************************************************************************
static int scale_layout_values(TCHAR *dest_file, TCHAR *source_file)
{
   FILE *infd = fopen(source_file, "rt");
   if (infd == NULL) {
      printf("%s: cannot open for reading\n", source_file);
      return 1 ;
   }
   FILE *outfd = fopen(dest_file, "wt");
   if (outfd == NULL) {
      printf("%s: cannot open file for writing\n", dest_file);
      return 1 ;
   }
   
   TCHAR inpstr[MAX_LINE_LEN+1] ;
//Skin: 0,0,1280,656
   while (fgets(inpstr, MAX_LINE_LEN, infd) != NULL) {
      strip_newlines(inpstr);
//Display: 58,120 4 6 9EA48D 242A26
      if (_tcsncmp(inpstr, "Display:", 8) == 0) {
         puts("found Display");
         // fputs(inpstr, outfd);
         fprintf(outfd, "%s\n", inpstr);
      }
//Annunciator: 1 60,90,30,26 1330,94
      else if (_tcsncmp(inpstr, "Annunciator:", 12) == 0) {
         puts("found Annunciator");
         fprintf(outfd, "%s\n", inpstr);
      }
//Key: 2 117,450,102,106 127,478,82,58 1389,478
      else if (_tcsncmp(inpstr, "Key:", 4) == 0) {
         puts("found Key");
         fprintf(outfd, "%s\n", inpstr);
      }
//AltBkgd: 1 1294,2,192,84 864,196
      else if (_tcsncmp(inpstr, "AltBkgd:", 8) == 0) {
         puts("found AltBkgd");
         fprintf(outfd, "%s\n", inpstr);
      }
//AltKey: 1 14 1298,386
      else if (_tcsncmp(inpstr, "AltKey:", 7) == 0) {
         puts("found AltKey");
         fprintf(outfd, "%s\n", inpstr);
      }
      else {
         fprintf(outfd, "%s\n", inpstr);
      }
   }
   
   fclose(infd);
   fclose(outfd);
   return 0 ;
}

//********************************************************************************
int _tmain(int argc, TCHAR** argv)
{
   int idx, result ;
   for (idx=1; idx<argc; idx++) {
      TCHAR *p = argv[idx] ;
      if (idx == 1) {
         _tcscpy(target_file, p) ;
      }
      else if (idx == 2) {
         _tcscpy(ref_file, p) ;
      }
      else {
         usage();
         return 1 ;
      }
   }
   
   //  check args
   if (target_file[0] == 0  ||  ref_file[0] == 0) {
      usage() ;
      return 1 ;
   }
   
   sprintf(target_file_lo,  "%s.layout", target_file);
   sprintf(target_file_gif, "%s.gif",    target_file);
   sprintf(ref_file_lo,     "%s.layout", ref_file);
   
   // printf("reference file: %s\n", ref_file);
   printf("reference layout file: %s\n", ref_file_lo);
   // printf("target file: %s\n", target_file);
   printf("target layout file:    %s\n", target_file_lo);
   printf("target gif file:       %s\n", target_file_gif);
   
   //  get original dimensions from the reference file
   result = get_skin_dimens(ref_file_lo, true);
   if (result != 0) {
      return result ;
   }
   result = get_skin_dimens(target_file_lo, false);
   if (result != 0) {
      return result ;
   }
   
   printf("ref dimens:    %ux%u\n", ref_width, ref_height) ;
   printf("target dimens: %ux%u\n", target_width, target_height);
   x_scale = (double) target_width  / (double) ref_width ;
   y_scale = (double) target_height / (double) ref_height ;
   printf("scaling factors: X: %.5f, Y: %.5f\n", x_scale, y_scale) ;
   
   //  draw image of selected skin
   //  Later note: I've decided to skip the graphics rendering for now...
   //  it is *so much* more work than any of the other tasks that I need to do in this program...
   // draw_gif_file(target_file_gif);
   
   //  back up the layout file
   sprintf(target_lo_backup, _T("%s.bak"), target_file_lo);
   printf("rename [%s] to [%s]\n", target_file_lo, target_lo_backup);
   result = CopyFile(target_file_lo, target_lo_backup, TRUE);
   if (result == 0) {
      printf("rename: %s\n", get_system_message());
      return result ;
   }
   
   //  open/read/parse/scale/write the layout file
   result = scale_layout_values(target_file_lo, target_lo_backup);
   if (result != 0) {
      return result ;
   }

   return 0;
}

