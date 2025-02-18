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
#include <stdlib.h>  //  atoi
#include <tchar.h>

//lint -e10  Expecting '}'
//lint -esym(818, argv)  //Pointer parameter could be declared as pointing to const

typedef unsigned int  uint ;

#define  MAX_PATH_LEN   1024

static TCHAR target_file[MAX_PATH_LEN+1] = _T("") ;
static TCHAR target_file_lo[MAX_PATH_LEN+1] = _T("") ;
static TCHAR target_file_gif[MAX_PATH_LEN+1] = _T("") ;
static TCHAR ref_file[MAX_PATH_LEN+1] = _T("") ;
static TCHAR ref_file_lo[MAX_PATH_LEN+1] = _T("") ;

static uint ref_width = 0 ;         
static uint ref_height = 0 ;         
static uint target_width = 0 ;         
static uint target_height = 0 ;         
         
//********************************************************************************
static void usage(void)
{
   puts("Usage: plus42_skin_editor <target_file_name> <ref_file_name");
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
   

   return 0;
}

