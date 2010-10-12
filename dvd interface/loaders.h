#ifndef LOADER_LOADERS
#define LOADER_LOADERS

//external access for the loaders
bool DOL_Open(char *filename);
bool ELF_Open(char *filename);
bool GCM_Open(char *filename);
bool GCMDMP_Open(char *filename);

//get gcm info
int ReadGCMInfo(char *filename, unsigned long *filesize, void *BannerBuffer /* 0x1960 bytes */, void *Header /* 0x440 bytes */);

extern char CurrentGameName[992];
extern char CurrentGameCRC[7];

#endif