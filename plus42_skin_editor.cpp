//********************************************************************************
//  plus42 skin editor - main file
//  License: Creative Commons CC0 1.0 Universal
//  Written by: Derell Licht
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
static TCHAR ref_file[MAX_PATH_LEN+1] = _T("") ;

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
   
   printf("target file: %s\n", target_file);
   printf("reference file: %s\n", ref_file);

   return 0;
}