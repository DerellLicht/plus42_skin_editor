//********************************************************************************
//  plus42 skin editor - main file
//  License: Creative Commons CC0 1.0 Universal
//  Written by: Derell Licht
//********************************************************************************
//  The initial version of this editor will assume that the target file
//  is a resize of some reference file.
//********************************************************************************

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

//********************************************************************************
static void usage(void)
{
   puts("Usage: plus42_skin_editor <target_file_name> <ref_file_name");
}

//********************************************************************************
#define  MAX_PATH_LEN   1024

static TCHAR target_file[MAX_PATH_LEN+1] = _T("") ;
static TCHAR target_file_lo[MAX_PATH_LEN+1] = _T("") ;
static TCHAR target_file_gif[MAX_PATH_LEN+1] = _T("") ;
static TCHAR ref_file[MAX_PATH_LEN+1] = _T("") ;
static TCHAR ref_file_lo[MAX_PATH_LEN+1] = _T("") ;

int _tmain(int argc, TCHAR** argv)
{
   int idx ;
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
   printf("target file: %s\n", target_file);
   printf("target layout file: %s\n", target_file_lo);
   printf("target gif file: %s\n", target_file_gif);

   return 0;
}