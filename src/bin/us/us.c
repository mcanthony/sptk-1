/*
  ---------------------------------------------------------------  
            Speech Signal Processing Toolkit (SPTK)

                      SPTK Working Group                           
                                                                   
                  Department of Computer Science                   
                  Nagoya Institute of Technology                   
                               and                                 
   Interdisciplinary Graduate School of Science and Engineering    
                  Tokyo Institute of Technology                    
                                                                   
                     Copyright (c) 1984-2007                       
                       All Rights Reserved.                        
                                                                   
  Permission is hereby granted, free of charge, to use and         
  distribute this software and its documentation without           
  restriction, including without limitation the rights to use,     
  copy, modify, merge, publish, distribute, sublicense, and/or     
  sell copies of this work, and to permit persons to whom this     
  work is furnished to do so, subject to the following conditions: 
                                                                   
    1. The source code must retain the above copyright notice,     
       this list of conditions and the following disclaimer.       
                                                                   
    2. Any modifications to the source code must be clearly        
       marked as such.                                             
                                                                   
    3. Redistributions in binary form must reproduce the above     
       copyright notice, this list of conditions and the           
       following disclaimer in the documentation and/or other      
       materials provided with the distribution.  Otherwise, one   
       must contact the SPTK working group.                        
                                                                   
  NAGOYA INSTITUTE OF TECHNOLOGY, TOKYO INSTITUTE OF TECHNOLOGY,   
  SPTK WORKING GROUP, AND THE CONTRIBUTORS TO THIS WORK DISCLAIM   
  ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL       
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   
  SHALL NAGOYA INSTITUTE OF TECHNOLOGY, TOKYO INSTITUTE OF         
  TECHNOLOGY, SPTK WORKING GROUP, NOR THE CONTRIBUTORS BE LIABLE   
  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY        
  DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,  
  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTUOUS   
  ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR          
  PERFORMANCE OF THIS SOFTWARE.                                    
                                                                   
  ---------------------------------------------------------------  
*/

/************************************************************************
*                                                                       *
*    up-sampling                                                        *
*                                                                       *
*                                       1998.3  T.Kobayashi             *
*                                                                       *
*       usage:                                                          *
*               us [ options ] infile1 > stdout                         *
*       options:                                                        *
*               -s s  :  conversion type                       [58]     *
*                         23F up-sampling by 2:3                        *
*                         23S up-sampling by 2:3                        *
*                         34  up-sampling by 3:4                        *
*                         45  up-sampling by 4:5                        *
*                         57  up-sampling by 5:7                        *
*                         58  up-sampling by 5:8                        *
*               -c c  :  filename of low pass filter coef.     [Default]*
*               -u u  :  up-sampling ratio                     [N/A]    *
*               -d d  :  down-sampling ratio                   [N/A]    *
*       infile:                                                         *
*               data sequence                                  [stdin]  *
*       notice:                                                         *
*               Default LPF coefficients File                           *
*                2:3 -> /usr/local/cmnd/lib/lpfcoef.2to3f               *
*                2:3 -> /usr/local/cmnd/lib/lpfcoef.2to3s               *
*                3:4 -> /usr/local/cmnd/lib/lpfcoef.3to4                *
*                4:5 -> /usr/local/cmnd/lib/lpfcoef.4to5                *
*                5:7 -> /usr/local/cmnd/lib/lpfcoef.5to7                *
*                5:8 -> /usr/local/cmnd/lib/lpfcoef.5to8                *
*                                                                       *
************************************************************************/

static char *rcs_id = "$Id: us.c,v 1.17 2007/09/23 17:46:34 heigazen Exp $";


/* Standard C Libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <SPTK.h>


/*  Required Functions  */
void srcnv(FILE *fp);
void firin(double in);
double firout(int os);
void firinit(void);
int freada(double *p,int bl,FILE *fp);


/* Default Value */
#define RBSIZE  512
#define DSIZE  512
#define SIZE  256
#define LINEBUFSIZE 256
#define STYPE  "58"
#define mod(x) ((x) & (RBSIZE -1))


#ifndef LIB
#define LIB "/usr/local/SPTK/lib"
#endif


#define COEF23F LIB "/lpfcoef.2to3f"
#define COEF23S LIB "/lpfcoef.2to3s"
#define COEF34 LIB "/lpfcoef.3to4"
#define COEF45 LIB "/lpfcoef.4to5"
#define COEF57 LIB "/lpfcoef.5to7"
#define COEF58 LIB "/lpfcoef.5to8"

/* Command Name */
char *cmnd, *coef=NULL, *type=STYPE;


void usage (int status)
{
   fprintf(stderr, "\n");
   fprintf(stderr, " %s - up-sampling\n\n",cmnd);
   fprintf(stderr, "  usage:\n");
   fprintf(stderr, "       %s [ options ] infile1 > stdout\n", cmnd);
   fprintf(stderr, "  options:\n");
   fprintf(stderr, "       -s s  : conversion type                           [%s]\n",STYPE);
   fprintf(stderr, "                23F up-sampling by 2:3\n");
   fprintf(stderr, "                23S up-sampling by 2:3\n");
   fprintf(stderr, "                34  up-sampling by 3:4\n");
   fprintf(stderr, "                45  up-sampling by 4:5\n");
   fprintf(stderr, "                57  up-sampling by 5:7\n");
   fprintf(stderr, "                58  up-sampling by 5:8\n");
   fprintf(stderr, "       -c c  : filename of low pass filter coefficients  [Default]\n");
   fprintf(stderr, "       -u u  : up-sampling ratio                         [N/A]\n");
   fprintf(stderr, "       -d d  : down-sampling ratio                       [N/A]\n");
   fprintf(stderr, "       -h    : print this message\n");
   fprintf(stderr, "  infile:\n");
   fprintf(stderr, "       data sequence (%s)                                [stdin]\n", FORMAT);
   fprintf(stderr, "  notice:\n");
   fprintf(stderr, "       Default LPF coefficients File\n");
   fprintf(stderr, "        2:3 -> %s\n", COEF23F);
   fprintf(stderr, "        2:3 -> %s\n", COEF23S);
   fprintf(stderr, "        3:4 -> %s\n", COEF34);
   fprintf(stderr, "        4:5 -> %s\n", COEF45);
   fprintf(stderr, "        5:7 -> %s\n", COEF57);
   fprintf(stderr, "        5:8 -> %s\n", COEF58);
#ifdef PACKAGE_VERSION
   fprintf(stderr, "\n");
   fprintf(stderr, " SPTK: version %s\n",PACKAGE_VERSION);
   fprintf(stderr, " CVS Info: %s", rcs_id);
#endif
   fprintf(stderr, "\n");
   exit(status);
}

static int start=-1, u_rate=0, d_rate=0;

int main (int argc,char *argv[])
{
   FILE *fp;
   char *s, *infile=NULL, c;
   int def_d_rate=1, def_u_rate=1;

   if ((cmnd=strrchr(argv[0], '/'))==NULL)
      cmnd = argv[0];
   else
      cmnd++;

   while (--argc) {
      if (*(s=*++argv)=='-') {
         c = *++s;
         if (*++s=='\0') {
            s = *++argv;
            --argc;
         }
         switch (c) {
         case 'c':
            coef = s;
            type = NULL;
            break;
         case 's':
            type = s;
            break;
         case 'u':
            u_rate = atoi(s);
            break;
         case 'd':
            d_rate = atoi(s);
            break;
         case 'h':
            usage (0);
         default:
            fprintf(stderr, "%s : Invalid option '%c'\n", cmnd, *(*argv+1));
            usage (1);
         }
      }
      else
         infile = s;
   }
   if (type==NULL) {
      ;
   }
   else if (strcmp(type, "23F")==0) {
      coef = COEF23F;
      def_d_rate = 2;
      def_u_rate = 3;
   }
   else if (strcmp(type, "23S")==0) {
      coef = COEF23S;
      def_d_rate = 2;
      def_u_rate = 3;
   }
   else if (strcmp(type, "34")==0) {
      coef = COEF34;
      def_d_rate = 3;
      def_u_rate = 4;
   }
   else if (strcmp(type, "45")==0) {
      coef = COEF45;
      def_d_rate = 4;
      def_u_rate = 5;
   }
   else if (strcmp(type, "57")==0) {
      coef = COEF57;
      def_d_rate = 5;
      def_u_rate = 7;
   }
   else if (strcmp(type, "58")==0) {
      coef = COEF58;
      def_d_rate = 5;
      def_u_rate = 8;
   }
   else {
      fprintf(stderr, "%s : Invalid option '%s'\n", cmnd, type);
      usage (1);
   }

   if (d_rate==0) d_rate = def_d_rate;
   if (u_rate==0) u_rate = def_u_rate;

   if (coef==NULL) {
      fprintf(stderr, "%s : LPF-coef file not specified\n", cmnd);
      return(1);
   }
   if (infile) {
      if ((fp=fopen(infile, "r"))==NULL) {
         fprintf(stderr, "%s : cannot open %s\n", cmnd, infile);
         return(1);
      }
   }
   else {
      fp = stdin;
   }
   srcnv(fp);
   fclose(fp);

   return(0);
}

void srcnv (FILE *fp)
{
   int is_cont, i, k, nread, count, nwr, delay;
   double x[SIZE], y[DSIZE];

   firinit();
   delay = start;
   for (count=is_cont=1; is_cont; ) {
      if ((nread=freadf(x, sizeof(*x), SIZE, fp))==0) {
         is_cont = 0;
         nread = (d_rate * start) / u_rate;
         for (k=0; k<nread; ++k)
            x[k]=0;
      }
      for (nwr=k=0; k<nread; ++k) {
         firin(x[k]);
         for (i=0; i<u_rate; ++i) {
            if (--count==0) {
               y[nwr++] = firout(i);
               count = d_rate;
            }
         }
      }
      if (delay) {
         if (nwr>delay) {
            nwr -= delay;
            if (fwritef(y + delay, sizeof(*y), nwr, stdout)!=nwr) {
               fprintf(stderr, "%s : write error\n", cmnd);
               exit(1);
            }
            delay = 0;
         }
         else {
            delay -= nwr;
         }
      }
      else {
         if (fwritef(y, sizeof(*y), nwr, stdout)!=nwr) {
            fprintf(stderr, "%s : write error\n", cmnd);
            exit(1);
         }
      }
   }
}

static double rb[RBSIZE], hdn[RBSIZE + 1];
static int flengdn=-1, indx=0;

void firin (double in)
{
   indx = mod(indx - 1);
   rb[indx] = in;
}

double firout (int os)
{
   double out;
   int k, l;

   out = 0;
   for (k=os,l=indx; k<=flengdn; k+=u_rate,l=mod(l + 1))
      out += rb[l] * hdn[k];

   return(out);
}

void firinit (void)
{
   FILE *fp;

   if ((fp=fopen(coef, "r"))==NULL) {
      fprintf(stderr, "%s : cannot open %s\n", cmnd, coef);
      exit(1);
   }
   flengdn = freada(hdn, RBSIZE + 1, fp);
   fclose(fp);
   if (--flengdn<0) {
      fprintf(stderr, "%s : cannot read filter coefficients\n", cmnd);
      exit(1);
   }
   if (start) {
      start = flengdn / (2 * d_rate);
   }
}

int freada (double *p,int bl,FILE *fp)
{
   int c;
   char buf[LINEBUFSIZE];

   c = 0;
   while (c<bl ) {
      if (fgets( buf, LINEBUFSIZE, fp)==NULL) break;
      p[c] = atof( buf);
      c+=1;
   }
   return c;
}
