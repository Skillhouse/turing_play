#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <getopt.h>
#include <linux/limits.h>



/* asr hacked in some options */

/* Flag set by ‘--verbose’. */
static int verbose_flag;
static char outdir[PATH_MAX] = ".";
static char basename[1024] = "frame";
static char target[PATH_MAX] = "";
static double seed = clock();
static double **A,**B,**TMP,CA=3,CB=20;
static int iterations = 2000, maxI=512, maxJ=512;



void dump_field (char const *base, int iter)
{

  int blen = 1024;
  char fname[blen];





  snprintf(fname,blen,"%s-%d.pgm",base,iter);

  FILE* zout=fopen(fname,"wb");




  fprintf (zout,"P5 %d %d 255 ",maxJ,maxI);
  
  for (int i=0;i<maxI;i++) 
    for (int j=0;j<maxJ;j++)
      fprintf(zout,"%c",(unsigned char)A[i][j]);       
  
  fclose(zout);
  
}



main(int argc, char **argv)
{ 
  /**** Get CA and CB, two factors that affect outcome of program. Can get
	from command line as below or just put in as a variable as in this version. Size of
	field is maxI X maxJ, can go much bigger ****/ 

  /*  char str[20];
      printf("CB->");
      fgets(str, 256, stdin);
      CB = atoi(str);
      printf("CB is %f, enter CA->",CB);
      fgets(str, 256, stdin);
      CA = atoi(str);
      printf("CA=%.0f and CB=%.0f",CA,CB);
      getchar();  */


  int c;
  int i,j,n;
  

  while (1)
    {
      static struct option long_options[] =
	{
	  /* These options set a flag. */
	  {"verbose", no_argument,       &verbose_flag, 1},
	  {"brief",   no_argument,       &verbose_flag, 0},
	  /* These options don’t set a flag.
	     We distinguish them by their indices. */
	  {"ca",     required_argument,       0, 'a'},
	  {"cb",     required_argument,       0, 'b'},
	  {"iterations",     required_argument,       0, 'i'},
	  {"outputdir",      required_argument,       0, 'o'},
	  {"basename" ,      required_argument,       0, 'n'},
	  {"seed",           required_argument,       0, 's'},
	  
	  {0, 0, 0, 0}
	};
      /* getopt_long stores the option index here. */
      int option_index = 0;
	  
      c = getopt_long (argc, argv, "a:b:i: ",
		       long_options, &option_index);
	  
      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c)
        {
        case 0:
          /* If this option set a flag, do nothing else now. */
          if (long_options[option_index].flag != 0)
            break;
          printf ("option %s", long_options[option_index].name);
          if (optarg)
            printf (" with arg %s", optarg);
          printf ("\n");
          break;

        case 'a':
	  if (verbose_flag) printf ("option -ca with value '%s'\n", optarg);
	  CA=strtol(optarg,0,10);
          break;

        case 'b':
	  if (verbose_flag) printf ("option -cb with value '%s'\n", optarg);
	  CA=strtol(optarg,0,10);
          break;

        case 'i':
	  if (verbose_flag) printf ("option -iterations with value '%s'\n", optarg);
	  iterations=strtol(optarg,0,10);
          break;

        case 's':
	  if (verbose_flag) printf ("option -seed with value '%s'\n", optarg);
	  seed=strtol(optarg,0,10);
          break;

        case '?':
          /* getopt_long already printed an error message. */
          break;

        default:
          abort ();
        }
    }


  snprintf(target,PATH_MAX,"%s/%s",outdir,basename);

  if (verbose_flag) {
    
    printf (" CA: '%lf'\n", CA);
    printf (" CB: '%lf'\n", CB);
    printf (" seed: '%lf'\n", seed);
    printf (" Iterations: '%d'\n", iterations);
    printf (" Output dir: '%s'\n", outdir);
    printf (" target: '%s'\n", target);
    
  }






  /**** Initialize A and B randomly ****/

  srand(seed);
  A=(double **) calloc (maxI,sizeof(double*));
  B=(double **) calloc (maxI,sizeof(double*));
  TMP=(double **) calloc (maxI,sizeof(double*));

  for (i=0;i<maxI;i++)
    {
      A[i]= (double *) calloc (maxJ,sizeof(double));
      B[i]= (double *) calloc (maxJ,sizeof(double));
      TMP[i]= (double *) calloc (maxJ,sizeof(double));
      if (!B[i] || !A[i] || !TMP[i]) 
	{ 
	  printf ("Memory problems!\n");
	  exit(1);
	} 

      for (j=0; j<maxJ; j++) 
	{ 
	  A[i][j]= 12*(double) rand()/RAND_MAX;
	  B[i][j]= 12*(double) rand()/RAND_MAX;
	} 
    }
		
  FILE* fich=fopen("start-new-11.pgm","wb");
  if (!fich) 
    { 
      printf ("File problems!\n");
      exit(1);
    }


  dump_field(target,0);


  /**** Use Turing's Model ****/ 

  for (n=0;n<iterations;n++) 
    { 
      if (n % 100 == 0) printf("%d\n",iterations-n);
      for (i=0;i<maxI;i++) 
	{ 
	  int iplus1 = i+1, iminus1 = i-1;
	  if (i == 0) iminus1 = maxI-1;
	  if (i == maxI-1) iplus1 = 0;
	  for (j=0;j<maxJ;j++) 
	    { 
	      int jplus1 = j+1, jminus1 = j-1;
	      double Aold = A[i][j];
	      double DiA, ReA, DiB, ReB;
	      if ( j == 0 ) jminus1 = (maxJ-1);
	      if ( j == maxJ-1 ) jplus1 = 0;

	      /*** COMPONENT A ****/ 

	      //              DiA = CA * ( A[iplus1][j] + A[iminus1][j] + A[i][jplus1]  + A[i][jminus1]- 4*A[i][j]);
	      //				ReA = A[i][j] * B[i][j] - A[i][j] - 128.0;
	      DiA = CA * ( A[iplus1][j]-2*A[i][j]+A[iminus1][j]+A[i][jplus1]-2*A[i][j]+A[i][jminus1]);
	      ReA = A[i][j]*B[i][j] - A[i][j] - 12.0;
	      A[i][j] = A[i][j] + 0.01*(ReA + DiA);
	      if (A[i][j]<0) A[i][j] = 0;

	      /*** COMPONENT B ****/ 

	      //              DiB = CB * ( B[iplus1][j] + B[i][jminus1] + B[iminus1][j] + B[i][jplus1] - 4*B[i][j]);
	      //				ReB = 128.0 - Aold*B[i][j];
	      DiB =CB * ( B[iplus1][j]-2*B[i][j]+B[iminus1][j]+B[i][jplus1]-2*B[i][j]+B[i][jminus1]);
	      ReB = 16.0-Aold*B[i][j];
	      B[i][j]=B[i][j]+0.01*(ReB + DiB);
	      if (B[i][j]<0) B[i][j] = 0;
	    }
	}
    }

  /**** Scale the results to 0-255 ******/ 
	
  { 
    double high=0, low=255;
    for (i=0;i<maxI;i++) 
      for (j=0;j<maxJ;j++) 
	{ 
	  if (A[i][j]>high) high=A[i][j];
	  else if (A[i][j]<low) low=A[i][j];
	}
                 
    for (i=0;i<maxI;i++) 
      for (j=0;j<maxJ;j++) 
	A[i][j]=(A[i][j]-low)*255.0/(high-low);
  } 

  /**** Output to a PGM format file *****/ 

  dump_field(target,iterations);


}


