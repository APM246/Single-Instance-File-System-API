#include "sifs-internal.h"

extern void get_volume_header_info(const char *volumename, int *blocksize, int *nblocks);
extern int change_bitmap(const char *volumename, char SIFS_BIT, int *blockID, int nblocks);
extern int get_number_of_slashes(const char* pathname);
extern int find_parent_blockID(const char *volumename, const char *pathname, int nblocks, int blocksize);
extern int find_blockID(const char *volumename, const char *pathname, int nblocks, int blocksize);
