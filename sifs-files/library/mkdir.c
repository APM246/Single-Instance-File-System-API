#include "sifs-internal.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// make a new directory within an existing volume
int SIFS_mkdir(const char *volumename, const char *dirname)
{
    if (access(volumename, F_OK) != 0)
    {
        SIFS_errno	= SIFS_ENOVOL;
        return 1;
    }

    SIFS_DIRBLOCK new_dir; 
    strcpy(new_dir.name, dirname);
    new_dir.modtime = time(NULL);
    new_dir.nentries = 0;

    // CONVERT INTO FUNCTION AND ADD TO INTERNAL-SIFS.H. THIS FUNCTION WILL BE CALLED
    // EVERYTIME A NEW BLOCK IS USED (DIR BLOCK, FILE BLOCK, FILES CONTENTS)
    FILE *fp = fopen(volumename, "w");
    //fseek(fp, , SEEK_SET);
    

        
    //printf("\n%s\n", new_dir.name);

    return 0;
}
