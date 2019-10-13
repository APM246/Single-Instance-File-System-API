#include "helperfunctions.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// read the contents of an existing file from an existing volume
int SIFS_readfile(const char *volumename, const char *pathname,
		  void **data, size_t *nbytes)
{
    // NO SUCH VOLUME 
    if (access(volumename, F_OK) != 0)
    {
        SIFS_errno	= SIFS_ENOVOL;
        return 1;
    }

    // ROOT IS NOT A FILE
    if (strlen(pathname) == 1 && *pathname == '/')
    {
        SIFS_errno = SIFS_ENOTFILE;
        return 1;
    }

    // ACCESS VOLUME HEADER INFORMATION
    int blocksize, nblocks, blockID;
    get_volume_header_info(volumename, &blocksize, &nblocks);

    // NO SUCH FILE EXISTS 
    if ((blockID = find_blockID(volumename, pathname, nblocks, blocksize)) == -1) 
    {
        SIFS_errno = SIFS_ENOENT;
        return 1;
    }

    FILE *fp = fopen(volumename, "r+");
    char bitmap[nblocks];
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    fread(bitmap, sizeof(bitmap), 1, fp);

    // NOT A FILE
    if ((bitmap[blockID] != SIFS_FILE))
    {
        SIFS_errno = SIFS_ENOTFILE;
        return 1;
    }

    // ACQUIRE FIRSTBLOCKID
    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + blockID*blocksize, SEEK_SET);
    SIFS_FILEBLOCK fileblock;
    fread(&fileblock, sizeof(SIFS_FILEBLOCK), 1, fp);
    int firstblockID = fileblock.firstblockID;
    *nbytes = fileblock.length;

    // ALLOCATE MEMORY FOR BUFFER AND COPY INTO
    char *buffer = malloc(*nbytes);
    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + firstblockID*blocksize, SEEK_SET);
    fread(buffer, *nbytes, 1, fp);

    // ASSIGN TO DATA pointer
    *data = buffer;

    fclose(fp);
    return 0;
}
