//********************************************************************************
//  plus42 skin editor - main file
//  License: Creative Commons CC0 1.0 Universal
//  Written by: Derell Licht
//  
//  build: g++ -Wall -s -O3 -c -Weffc++ plus42_skin_editor.cpp -o plus42_skin_editor.exe
//  If we want to build with gdiplus (GDI+) library, then we need:
//  build: c:\tdm32\bin\g++ -Wall -s -O3 -c -Weffc++ -Wno-stringop-truncation plus42_skin_editor.cpp -o plus42_skin_editor.exe
//  run example: plus42_skin_editor LandscapeRightSmall LandscapeRight
//********************************************************************************
//  The initial version of this editor will assume that the target file
//  is a resize of some reference file.
//********************************************************************************

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>  //  atoi
#include <tchar.h>
#include <math.h>

// #include "header.h"
#define  MAX_PATH_LEN   1024

typedef unsigned int  uint ;

#define  HTAB     9
//lint -e10  Expecting '}'
//lint -esym(818, argv)  //Pointer parameter could be declared as pointing to const
//lint -esym(438, outlen)  // Last value assigned to variable not used

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
   puts("Usage: plus42_skin_editor <target_file_name> <ref_file_name>");
}

//**********************************************************************
static TCHAR *skip_spaces_and_commas(TCHAR *hd)
{
   while (*hd == ' '  ||  *hd == ',') {
      hd++ ;
   }
   return hd ;
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

//**********************************************************************
//lint -esym(714, next_field)
//lint -esym(759, next_field)
//lint -esym(765, next_field)
char *next_field(char *q)
{
   while (*q != ' '  &&  *q != HTAB  &&  *q != 0)
      q++ ; //  walk past non-spaces
   while (*q == ' '  ||  *q == HTAB)
      q++ ; //  walk past all spaces
   return q;
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
static double fracpart(double inval)
{
   unsigned mant = (unsigned) inval ;
   double frac = inval - (double) mant ;
   return frac ;
}

//********************************************************************************
static unsigned dround(double inval)
{
   double frac = fracpart(inval);
   unsigned uval = (unsigned) inval ;
   if (frac >= .50) {
      uval++ ;
   }
   return uval ;
}

//********************************************************************************
//  these two functions should use round(), rather than ceil()
//********************************************************************************
static uint scale_x(uint xnum)
{
   double xvalue = (double) xnum * x_scale ;
   // xvalue = ceil(xvalue);
   // return (uint) xvalue ;
   return dround(xvalue) ;
}

static uint scale_y(uint ynum)
{
   double yvalue = (double) ynum * y_scale ;
   // yvalue = ceil(yvalue);
   // return (uint) yvalue ;
   return dround(yvalue) ;
}

//********************************************************************************
//Display: 58,120 4 6 9EA48D 242A26
//********************************************************************************
static int scale_display(TCHAR *inpstr, FILE *outfd)
{
   static bool entry_shown = false ;
   TCHAR outstr[MAX_LINE_LEN+1] ;
   uint outlen ;
   uint xnum, ynum ;
   TCHAR *hd ;
   hd = next_field(inpstr);
   if (hd == NULL) { puts("PARSE ERROR"); return 1 ; }
   outlen = (uint) hd - (uint) inpstr ;
   _tcsncpy(outstr, inpstr, outlen);   //  copy intro data to output

   //  get x0, y0
   xnum = (uint) atoi(hd) ;
   xnum = scale_x(xnum) ;
   
   hd = _tcschr(hd, ',');
   if (hd == NULL) { puts("PARSE ERROR"); return 1 ; }
   hd = skip_spaces_and_commas(hd);
   // hd++ ;
   ynum = (uint) atoi(hd) ;
   ynum = scale_y(ynum) ;
   
   sprintf(outstr+outlen, "%u,%u ", xnum, ynum);
   outlen = _tcslen(outstr);  //  used for appending later data
   
   //  go to magnification fields
   hd = next_field(hd);
   if (hd == NULL) { puts("PARSE ERROR"); return 1 ; }
   
   xnum = (uint) atoi(hd) ;
   xnum = scale_x(xnum) ;
   
   hd = next_field(hd);
   if (hd == NULL) { puts("PARSE ERROR"); return 1 ; }
   ynum = (uint) atoi(hd) ;
   ynum = scale_y(ynum) ;
   
   hd = next_field(hd);
   sprintf(outstr+outlen, "%u %u %s", xnum, ynum, hd);
   outlen = _tcslen(outstr);  //lint !e438
   
   // printf("D2: [%s] %u, hd: [%s]\n", outstr, outlen, hd);
   // fputs(inpstr, outfd);
   if (!entry_shown) {
      printf("Display in:  [%s]\n", inpstr);
      printf("Display out: [%s]\n", outstr);
      entry_shown = true ;
   }
   fprintf(outfd, "%s\n", outstr);
   return 0 ;
}

//********************************************************************************
//Annunciator: 1 60,90,30,26 1330,94
//********************************************************************************
static int scale_annunciator(TCHAR *inpstr, FILE *outfd)
{
   static bool entry_shown = false ;
   TCHAR outstr[MAX_LINE_LEN+1] ;
   uint outlen ;
   uint xnum, ynum ;
   TCHAR *hd ;
   // puts("found Annunciator");
   hd = next_field(inpstr);
   if (hd == NULL) { puts("PARSE ERROR"); return 1 ; }
   hd = next_field(hd);
   if (hd == NULL) { puts("PARSE ERROR"); return 1 ; }
   outlen = (uint) hd - (uint) inpstr ;
   _tcsncpy(outstr, inpstr, outlen);   //  copy intro data to output
   
   //  get x0, y0
   xnum = (uint) atoi(hd) ;
   xnum = scale_x(xnum) ;
   
   hd = _tcschr(hd, ',');
   if (hd == NULL) { puts("PARSE ERROR"); return 1 ; }
   hd = skip_spaces_and_commas(hd);
   // hd++ ;
   ynum = (uint) atoi(hd) ;
   ynum = scale_y(ynum) ;
   
   sprintf(outstr+outlen, "%u,%u,", xnum, ynum);
   outlen = _tcslen(outstr);  //  used for appending later data
   hd = _tcschr(hd, ',');
   if (hd == NULL) { puts("PARSE ERROR"); return 1 ; }
   hd = skip_spaces_and_commas(hd);
   // hd++ ;
   
   //  get dx, dy
   xnum = (uint) atoi(hd) ;
   xnum = scale_x(xnum) ;
   
   hd = _tcschr(hd, ',');
   if (hd == NULL) { puts("PARSE ERROR"); return 1 ; }
   hd = skip_spaces_and_commas(hd);
   // hd++ ;
   ynum = (uint) atoi(hd) ;
   ynum = scale_y(ynum) ;
   hd = next_field(hd) ;
   sprintf(outstr+outlen, "%u,%u ", xnum, ynum);
   outlen = _tcslen(outstr);  //  used for appending later data
   
   //  get x0, y0 for active-state bitmap
   xnum = (uint) atoi(hd) ;
   xnum = scale_x(xnum) ;
   
   hd = _tcschr(hd, ',');
   if (hd == NULL) { puts("PARSE ERROR"); return 1 ; }
   hd = skip_spaces_and_commas(hd);
   // hd++ ;
   ynum = (uint) atoi(hd) ;
   ynum = scale_y(ynum) ;
   sprintf(outstr+outlen, "%u,%u", xnum, ynum);
   outlen = _tcslen(outstr);  //lint !e438
   
   if (!entry_shown) {
      printf("Annun: in:  [%s]\n", inpstr);
      printf("Annun: out: [%s]\n", outstr);
      entry_shown = true ;
   }
   fprintf(outfd, "%s\n", outstr);
   return 0 ;
}

//********************************************************************************
//Key: 2 117,450,102,106 127,478,82,58 1389,478
//********************************************************************************
static int scale_key(TCHAR *inpstr, FILE *outfd)
{
   static bool entry_shown = false ;
   TCHAR outstr[MAX_LINE_LEN+1] ;
   uint outlen ;
   uint xnum, ynum ;
   TCHAR *hd ;
   // puts("found Annunciator");
   hd = next_field(inpstr);
   if (hd == NULL) { puts("PARSE ERROR"); return 1 ; }
   hd = next_field(hd);
   if (hd == NULL) { puts("PARSE ERROR"); return 1 ; }
   outlen = (uint) hd - (uint) inpstr ;
   _tcsncpy(outstr, inpstr, outlen);   //  copy intro data to output
   
   //******************************************************************
   //  get x0, y0 (sensitive rectangle)
   xnum = (uint) atoi(hd) ;
   xnum = scale_x(xnum) ;
   
   hd = _tcschr(hd, ',');
   if (hd == NULL) { puts("PARSE ERROR"); return 1 ; }
   hd = skip_spaces_and_commas(hd);
   // hd++ ;
   ynum = (uint) atoi(hd) ;
   ynum = scale_y(ynum) ;
   
   sprintf(outstr+outlen, "%u,%u,", xnum, ynum);
   outlen = _tcslen(outstr);  //  used for appending later data
   hd = _tcschr(hd, ',');
   if (hd == NULL) { puts("PARSE ERROR"); return 1 ; }
   hd = skip_spaces_and_commas(hd);
   // hd++ ;
   
   //  get dx, dy
   xnum = (uint) atoi(hd) ;
   xnum = scale_x(xnum) ;
   
   hd = _tcschr(hd, ',');
   if (hd == NULL) { puts("PARSE ERROR"); return 1 ; }
   hd = skip_spaces_and_commas(hd);
   // hd++ ;
   ynum = (uint) atoi(hd) ;
   ynum = scale_y(ynum) ;
   hd = next_field(hd) ;
   sprintf(outstr+outlen, "%u,%u ", xnum, ynum);
   outlen = _tcslen(outstr);  //  used for appending later data
   
   //******************************************************************
   //  get x0, y0 (display rectangle)
   xnum = (uint) atoi(hd) ;
   xnum = scale_x(xnum) ;
   
   hd = _tcschr(hd, ',');
   if (hd == NULL) { puts("PARSE ERROR"); return 1 ; }
   hd = skip_spaces_and_commas(hd);
   // hd++ ;
   ynum = (uint) atoi(hd) ;
   ynum = scale_y(ynum) ;
   
   sprintf(outstr+outlen, "%u,%u,", xnum, ynum);
   outlen = _tcslen(outstr);  //  used for appending later data
   hd = _tcschr(hd, ',');
   if (hd == NULL) { puts("PARSE ERROR"); return 1 ; }
   hd = skip_spaces_and_commas(hd);
   // hd++ ;
   
   //  get dx, dy
   xnum = (uint) atoi(hd) ;
   xnum = scale_x(xnum) ;
   
   hd = _tcschr(hd, ',');
   if (hd == NULL) { puts("PARSE ERROR"); return 1 ; }
   hd = skip_spaces_and_commas(hd);
   // hd++ ;
   ynum = (uint) atoi(hd) ;
   ynum = scale_y(ynum) ;
   hd = next_field(hd) ;
   sprintf(outstr+outlen, "%u,%u ", xnum, ynum);
   outlen = _tcslen(outstr);  //  used for appending later data
   
   //  get x0, y0 for active-state bitmap
   xnum = (uint) atoi(hd) ;
   xnum = scale_x(xnum) ;
   
   hd = _tcschr(hd, ',');
   if (hd == NULL) { puts("PARSE ERROR"); return 1 ; }
   hd = skip_spaces_and_commas(hd);
   // hd++ ;
   ynum = (uint) atoi(hd) ;
   ynum = scale_y(ynum) ;
   sprintf(outstr+outlen, "%u,%u", xnum, ynum);
   outlen = _tcslen(outstr);  //lint !e438
   
   // puts("found Key");
   // fprintf(outfd, "%s\n", inpstr);
   if (!entry_shown) {
      printf("Key: in:  [%s]\n", inpstr);
      printf("Key: out: [%s]\n", outstr);
      entry_shown = true ;
   }
   fprintf(outfd, "%s\n", outstr);
   return 0 ;
}

//********************************************************************************
//AltBkgd: 1 1294,2,192,84 864,196
//********************************************************************************
static int scale_altbkgd(TCHAR *inpstr, FILE *outfd)
{
   // puts("found AltBkgd");
   // fprintf(outfd, "%s\n", inpstr);
   static bool entry_shown = false ;
   TCHAR outstr[MAX_LINE_LEN+1] ;
   uint outlen ;
   uint xnum, ynum ;
   TCHAR *hd ;
   // puts("found Annunciator");
   hd = next_field(inpstr);
   if (hd == NULL) { puts("PARSE ERROR 1"); return 1 ; }
   hd = next_field(hd);
   if (hd == NULL) { puts("PARSE ERROR 2"); return 1 ; }
   outlen = (uint) hd - (uint) inpstr ;
   _tcsncpy(outstr, inpstr, outlen);   //  copy intro data to output
   
   //******************************************************************
   //  get x0, y0 
   xnum = (uint) atoi(hd) ;
   xnum = scale_x(xnum) ;
   
   hd = _tcschr(hd, ',');
   if (hd == NULL) { puts("PARSE ERROR 3"); return 1 ; }
   hd = skip_spaces_and_commas(hd);
   // hd++ ;
   ynum = (uint) atoi(hd) ;
   ynum = scale_y(ynum) ;
   
   sprintf(outstr+outlen, "%u,%u,", xnum, ynum);
   outlen = _tcslen(outstr);  //  used for appending later data
   hd = _tcschr(hd, ',');
   if (hd == NULL) { puts("PARSE ERROR 4"); return 1 ; }
   hd = skip_spaces_and_commas(hd);
   // hd++ ;
   
   //  get dx, dy
   xnum = (uint) atoi(hd) ;
   xnum = scale_x(xnum) ;
   
   hd = _tcschr(hd, ',');
   if (hd == NULL) { puts("PARSE ERROR 5"); return 1 ; }
   hd = skip_spaces_and_commas(hd);
   // hd++ ;
   ynum = (uint) atoi(hd) ;
   ynum = scale_y(ynum) ;
   hd = next_field(hd) ;
   sprintf(outstr+outlen, "%u,%u ", xnum, ynum);
   outlen = _tcslen(outstr);  //  used for appending later data
   
   //  get x0, y0 for active-state bitmap
   xnum = (uint) atoi(hd) ;
   xnum = scale_x(xnum) ;
   
   hd = _tcschr(hd, ',');
   if (hd == NULL) { puts("PARSE ERROR 6"); return 1 ; }
   hd = skip_spaces_and_commas(hd);
   // hd++ ;
   ynum = (uint) atoi(hd) ;
   ynum = scale_y(ynum) ;
   sprintf(outstr+outlen, "%u,%u", xnum, ynum);
   outlen = _tcslen(outstr);  //lint !e438
   
   if (!entry_shown) {
      printf("altbg: in:  [%s]\n", inpstr);
      printf("altbg: out: [%s]\n", outstr);
      entry_shown = true ;
   }
   fprintf(outfd, "%s\n", outstr);
   return 0 ;
}

//********************************************************************************
//AltKey: 1 14 1298,386
//********************************************************************************
static int scale_altkey(TCHAR *inpstr, FILE *outfd)
{
   // puts("found AltKey");
   static bool entry_shown = false ;
   TCHAR outstr[MAX_LINE_LEN+1] ;
   uint outlen ;
   uint xnum, ynum ;
   TCHAR *hd ;
   // puts("found Annunciator");
   hd = next_field(inpstr);
   if (hd == NULL) { puts("PARSE ERROR 1"); return 1 ; }
   hd = next_field(hd);
   if (hd == NULL) { puts("PARSE ERROR 2"); return 1 ; }
   hd = next_field(hd);
   if (hd == NULL) { puts("PARSE ERROR 3"); return 1 ; }
   outlen = (uint) hd - (uint) inpstr ;
   _tcsncpy(outstr, inpstr, outlen);   //  copy intro data to output
   
   //  get x0, y0 for active-state bitmap
   xnum = (uint) atoi(hd) ;
   xnum = scale_x(xnum) ;
   
   hd = _tcschr(hd, ',');
   if (hd == NULL) { puts("PARSE ERROR 4"); return 1 ; }
   hd = skip_spaces_and_commas(hd);
   // hd++ ;
   ynum = (uint) atoi(hd) ;
   ynum = scale_y(ynum) ;
   sprintf(outstr+outlen, "%u,%u", xnum, ynum);
   outlen = _tcslen(outstr);  //lint !e438
   
   if (!entry_shown) {
      printf("altkey: in:  [%s]\n", inpstr);
      printf("altkey: out: [%s]\n", outstr);
      entry_shown = true ;
   }
   fprintf(outfd, "%s\n", outstr);
   return 0 ;
}

//********************************************************************************
static int scale_layout_values(TCHAR *dest_file, TCHAR *source_file)
{
   int result ;
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
   // TCHAR *hd ;
   // TCHAR outstr[MAX_LINE_LEN+1] ;
   // uint outlen = 0 ;
   // uint xnum, ynum ;
//Skin: 0,0,1280,656
   while (fgets(inpstr, MAX_LINE_LEN, infd) != NULL) {
      strip_newlines(inpstr);
      
      //*******************************************************************
      //Display: 58,120 4 6 9EA48D 242A26
      if (_tcsncmp(inpstr, "Display:", 8) == 0) {
         result = scale_display(inpstr, outfd);
         if (result != 0) {
            break ;
         }
      }

      //*******************************************************************
      //Annunciator: 1 60,90,30,26 1330,94
      else if (_tcsncmp(inpstr, "Annunciator:", 12) == 0) {
         result = scale_annunciator(inpstr, outfd);
         if (result != 0) {
            break ;
         }
      }

      //*******************************************************************
      //Key: 2 117,450,102,106 127,478,82,58 1389,478
      else if (_tcsncmp(inpstr, "Key:", 4) == 0) {
         result = scale_key(inpstr, outfd);
         if (result != 0) {
            break ;
         }
      }

      //*******************************************************************
      //AltBkgd: 1 1294,2,192,84 864,196
      else if (_tcsncmp(inpstr, "AltBkgd:", 8) == 0) {
         result = scale_altbkgd(inpstr, outfd);
         if (result != 0) {
            break ;
         }
      }

      //*******************************************************************
      //AltKey: 1 14 1298,386
      else if (_tcsncmp(inpstr, "AltKey:", 7) == 0) {
         result = scale_altkey(inpstr, outfd);
         if (result != 0) {
            break ;
         }
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
   _unlink(target_lo_backup); //lint !e534
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

