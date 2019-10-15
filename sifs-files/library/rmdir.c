#include "helperfunctions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// remove an existing directory from an existing volume
int SIFS_rmdir(const char *volumename, const char *pathname)
{
    // NO SUCH VOLUME 
    if (access(volumename, F_OK) != 0)
    {
        SIFS_errno	= SIFS_ENOVOL;
        return 1;
    }

    // THROW ERROR IF USER TRIES TO DELETE ROOT DIRECTORY
    if (strlen(pathname) == 1 && *pathname == '/')
    {
        SIFS_errno = SIFS_EINVAL;
        return 1;
    }

    FILE *fp = fopen(volumename, "r+");
    int nblocks, blocksize;
    get_volume_header_info(volumename, &blocksize, &nblocks);
    int block_ID = find_blockID(volumename, pathname, nblocks, blocksize); 
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    char bitmap[nblocks];
    fread(bitmap, sizeof(bitmap), 1, fp);
    SIFS_BIT type = bitmap[block_ID];

    // THROW ERROR IF PATHNAME IS NOT A DIRECTORY
    if (type != SIFS_DIR)
    {
        if (block_ID == -1) SIFS_errno = SIFS_ENOENT;
        else SIFS_errno = SIFS_ENOTDIR; // The block is a file or data block
        return 1;
    }

    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + block_ID*blocksize, SEEK_SET);
    SIFS_DIRBLOCK dirblock;
    fread(&dirblock, sizeof(dirblock), 1, fp);

    // THROW ERROR IF DIRECTORY IS NOT EMPTY 
    if (dirblock.nentries != 0)
    {
        SIFS_errno = SIFS_ENOTEMPTY;
        return 1;
    }

    // change bitmap - REMOVE FIRST TWO LINES (?)
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    fread(bitmap, sizeof(bitmap), 1, fp);
    bitmap[block_ID] = SIFS_UNUSED;
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    fwrite(bitmap, sizeof(bitmap), 1, fp);
    
    // clear directory block from volume 
    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + block_ID*blocksize, SEEK_SET);
    memset(&dirblock, 0, sizeof(dirblock)); 
    fwrite(&dirblock, sizeof(dirblock), 1, fp);

    // update 3 fields of parent directory 
    SIFS_DIRBLOCK parentblock;
    int parent_blockID = find_parent_blockID(volumename, pathname, nblocks, blocksize); 
    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + parent_blockID*blocksize, SEEK_SET);
    fread(&parentblock, sizeof(parentblock), 1, fp);
    int nentries = parentblock.nentries;
    for (int i = 0; i < SIFS_MAX_ENTRIES; i++)   //change this to nentries of parent block
    {
        if (parentblock.entries[i].blockID == block_ID)
        {
            // shuffle entries in entries array 1 spot down
            while (i < nentries - 1)
            { 
                parentblock.entries[i].blockID = parentblock.entries[i+1].blockID;
                parentblock.entries[i].fileindex = parentblock.entries[i+1].fileindex;
                i++;
            }

            break;
        }
    }
    parentblock.entries[nentries - 1].blockID = 0;   //update last spot that was shuffled down
    parentblock.entries[nentries - 1].fileindex = 0;
    parentblock.nentries--;
    parentblock.modtime = time(NULL);
    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + parent_blockID*blocksize, SEEK_SET);
    fwrite(&parentblock, sizeof(parentblock), 1, fp);

    fclose(fp);
    return 0;
}
