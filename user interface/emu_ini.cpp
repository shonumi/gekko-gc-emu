// emu_ini.cpp
// (c) 2005,2006 Gekko Team

#include "..\emu.h"

//

#include <fstream>
using namespace std;

EmuRomInfo RomInfo;
bool DisableINIPatches = 0;

int findINIEntry(char *gamecode, EmuRomInfo *rominfo, bool SkipPatch)
{
	char	str[2048], filename[1024];
	u32		patch_addr, patch_data;
	u32		x;
	char	*y;
	u32		SkipGetLine;

	fstream inifile;
	int FoundEntry = 0;

	// if(SkipPatch) return 0;

	sprintf(filename, "%sGameList.ini", ProgramDirectory);

	inifile.open(filename);
	if(inifile.fail()==1)
	{
		printf("ini not found, ignoring..\n");
		return FoundEntry;
	}

  // There is a pb with ID6 match... The checksum of the banner seems to change over time !
  // So I will accept ID4 check only in the ini file
  printf("Looking for : %s ", gamecode);
	while(!inifile.eof())
	{
		inifile.getline(str,2048);
		if(*(u32*)&str[1] == *(u32*)&gamecode[0])
		{
      // This is a potential ID4 match
			FoundEntry = 1;  // This is the official ID6 one !
//			if(*(u16*)&str[5] == *(u16*)&gamecode[4])
			{
				if(!SkipPatch)
					printf("Found ID4 match ! Loading game %s\n", CurrentGameName);
        else
					printf("Found ID4 match !\n");

				//get the title, comments, and the first patch
				for(x = 0; x < 3; x++)
				{
					if(inifile.eof())
						break;

				  inifile.getline(str,1023);  // Shortened for rominfo size
					if(strncmp(str,"title", 5) == 0)
					{
						//find the = and any space after it
						y = strchr(str, 0x3D);
						if(y)
						{
							y++;
							while(*y == 0x20)
								y++;

							//get our data
							sscanf(y,"%[^\n]",rominfo->title_alt);
						}
					}
					else if(strncmp(str,"comments", 8) == 0)
					{
						//find the = and any space after it
						y = strchr(str, 0x3D);
						if(y)
						{
							y++;
							while(*y == 0x20)
								y++;

							//get our data
							sscanf(y,"%[^\n]",rominfo->comments);
						}
					}
					else
						break;
				}

				//skip the first getline read. I could get line at the end
				//but then I get an error about EOF if the last entry does not
				//have a blank line after it. We already have the first
				//patch from above, either due to not having title and comment so
				//we exited early or we got both and read the next line anyways
				SkipGetLine = 1;
				while(!inifile.eof())
				{
					if(!SkipGetLine)
						inifile.getline(str,2048);

					SkipGetLine = 0;

					if(strlen(str)<3) break;
					if(strncmp(str, "patch", 5) == 0)
					{
						//find the = and any space after it
						y = strchr(str, 0x3D);
						if(y)
						{
							y++;
							while(*y == 0x20)
								y++;

							//get our data
							sscanf(y,"0x%08X",&patch_addr);

							//find the , then remove any space after it
							y = strchr(y, 0x2C);
							if(y)
							{
								y++;
								while(*y == 0x20)
									y++;

								//get our data
								sscanf(y,"0x%08X",&patch_data);

                                if (!SkipPatch)  // Skip patch is a real skip 
                                {
								Memory_Write32(patch_addr,patch_data);
								printf("Writing patch %08X at %08X\n", patch_data, patch_addr);
							}
						}
					}
				}
				}

				//stop processing the ini
				break;
			}
		}
	}

  if (!FoundEntry)
	  printf("Entry ID4 not found for %s\n", gamecode);

	inifile.close();
	return FoundEntry;
}
