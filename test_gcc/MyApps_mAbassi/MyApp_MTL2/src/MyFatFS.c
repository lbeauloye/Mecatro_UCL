/* ------------------------------------------------------------------------------------------------ */

#include "mAbassi.h"
#include "SysCall.h"								/* System call layer definitions				*/
#include "Platform.h"								/* Everything about the target platform is here	*/
#include "HWinfo.h"									/* Everything about the target board is here	*/
#include "MediaIF.h"								/* Media interface with SD/MMC, QSPI etc		*/

#include "ff.h"										/* FatFS definitions							*/
#include "diskio.h"									/* Media I/F: needed to access disk_ioctl()		*/

#include "arm_pl330.h"
#include "dw_sdmmc.h"
#include "dw_uart.h"

/* ------------------------------------------------------------------------------------------------ */
/* App definitions																					*/

#define PRT_ERROR(x) puts(x)

/* ------------------------------------------------------------------------------------------------ */
/* Apps variables																					*/

static DIR   g_Dir;
static char  g_DirNowPath[512]  __attribute__ ((aligned (OX_CACHE_LSIZE)));

static struct {										/* See SysCall_FatFS_....c for an explanation 	*/
	FATFS   FileSys;								/* about the padding & alignment				*/
	uint8_t Pad[OX_CACHE_LSIZE];
} gg_FileSys __attribute__ ((aligned (OX_CACHE_LSIZE)));

static char g_Drive[3];								/* Mounted drive								*/

#define g_FileSys (gg_FileSys.FileSys)

typedef struct {
	char *Name;
	int (* FctPtr)(int argc, char *argv[]);
} Cmd_t;
													/* Align on cache lines if cached transfers		*/
static char g_Buffer[16384] __attribute__ ((aligned (OX_CACHE_LSIZE)));	/* Buffer used for all I/O	*/

static char  CmdLine[256];                         /* Command line typed by the user                */
static char *Arg_V[10];                            /* Individual tokens from the command line       */

/* ------------------------------------------------------------------------------------------------ */
/* Apps functions																					*/

void CleanRead(char *Str);

int cmd_cat(int argc, char *argv[]);
int cmd_cd(int argc, char *argv[]);
int cmd_chmod(int argc, char *argv[]);
int cmd_cp(int argc, char *argv[]);
int cmd_du(int argc, char *argv[]);
int cmd_fmt(int argc, char *argv[]);
int cmd_help(int argc, char *argv[]);
int cmd_ls(int argc, char *argv[]);
int cmd_mkdir(int argc, char *argv[]);
int cmd_mnt(int argc, char *argv[]);
int cmd_mv(int argc, char *argv[]);
int cmd_perf(int argc, char *argv[]);
int cmd_pwd(int argc, char *argv[]);
int cmd_rm(int argc, char *argv[]);
int cmd_rmdir(int argc, char *argv[]);
int cmd_umnt(int argc, char *argv[]);
Cmd_t CommandLst[] = {
	{ "cat",	&cmd_cat	},
	{ "cd",		&cmd_cd		},
	{ "chmod",	&cmd_chmod	},
	{ "cp",		&cmd_cp		},
	{ "du",		&cmd_du		},
	{ "fmt",	&cmd_fmt	},
	{ "help",   &cmd_help	},
	{ "ls",		&cmd_ls		},
	{ "mkdir",	&cmd_mkdir	},
	{ "mnt",	&cmd_mnt	},
	{ "mv",		&cmd_mv		},
	{ "perf",	&cmd_perf	},
	{ "pwd",	&cmd_pwd	},
	{ "rm",		&cmd_rm		},
	{ "rmdir",	&cmd_rmdir	},
	{ "umnt",	&cmd_umnt	},
	{ "?",      &cmd_help	}
};

/* ------------------------------------------------------------------------------------------------ */

void DoInitFatFS(void)
{
    int ii;
    
    g_DirNowPath[0] = '\0';

	puts("\n\nTiny Shell for FatFS\n");
	puts("Available commands\n\n");

	cmd_help(100, 0);

	puts("\nThe EOF character for 'cat' is CTRL-D");
	puts("The device must first be mounted using the command \"mnt\"\n");

	puts("\nAvailable drives / devices:");
	for (ii=0 ; ii<10 ; ii++) {
		if (MEDIAinfo(ii) != (char *)NULL) {
			printf("    Drive %d is %s\n", ii, MEDIAinfo(ii));
		}
	}
	putchar('\n');
}

/* ------------------------------------------------------------------------------------------------ */

void DoProcessFatFS(void)
{
    
    int  Arg_C;										/* Number of tokens on the command line			*/
    int  ii;										/* General purpose								*/
    int  IsStr1;									/* Toggle to decode quoted ' strings			*/
    int  IsStr2;									/* Toggle to decode quoted " strings			*/
    int  jj;										/* General purpose								*/
    char PrevC;										/* Previous character during lexical analysis	*/
    
		if (FR_OK == f_getcwd(&g_DirNowPath[0], sizeof(g_DirNowPath))) {
			printf("%s > ", g_DirNowPath);			/* Prompt										*/
		}
		else {
			printf("> ");
		}

		gets(&CmdLine[0]);							/* Get the command from the user				*/

		CleanRead(&CmdLine[0]);						/* Replace tab by space and handle backspaces	*/

		ii     = 0;
		IsStr1 = 0;
		IsStr2 = 0;
		jj     = 0;
		PrevC  = ' ';
		while (CmdLine[ii] != '\0') {				/* Multi-space elimination						*/
			if (CmdLine[ii] == '\'') {				/* Toggle beginning / end of string				*/
				IsStr1 = !IsStr1;
			}
			if (CmdLine[ii] == '\"') {				/* Toggle beginning / end of string				*/
				IsStr2 = !IsStr2;
			}

			while ((PrevC == ' ')
			&&     (CmdLine[ii] == ' ')
			&&     (IsStr1 == 0)					/* Strings are left untouched					*/
			&&     (IsStr2 == 0)) {
				ii++;
			}
			PrevC = CmdLine[ii];
			CmdLine[jj++] = CmdLine[ii++];
		}
		CmdLine[jj] = '\0';

		Arg_C  = 0;									/* Split the command line into tokens			*/
		ii     = 0;
		IsStr1 = 0;
		IsStr2 = 0;
		PrevC  = '\0';
		while (CmdLine[ii] != '\0') {
			if (CmdLine[ii] == '\'') {				/* Properly handle strings enclosed with '		*/
				if (IsStr1 == 0) {
					Arg_V[Arg_C++] = &CmdLine[++ii];
				}
				else {
					CmdLine[ii++] = '\0';
				}
				IsStr1 = !IsStr1;
			}
			else if (CmdLine[ii] == '\"') {			/* Properly handle strings enclosed with "		*/
				if (IsStr2 == 0) {
					Arg_V[Arg_C++] = &CmdLine[++ii];
				}
				else {
					CmdLine[ii++] = '\0';
				}
				IsStr2 = !IsStr2;
			}
			if ((IsStr1 == 0)						/* Not in as string and is a white space		*/
			&&  (IsStr2 == 0)
			&&  (CmdLine[ii] == ' ')) {
				CmdLine[ii] = '\0';					/* This is the end of the token					*/
			}
					
			if ((PrevC == '\0')						/* Start of a new token							*/
			&&  (IsStr1 == 0)
			&&  (IsStr2 == 0)) {
				Arg_V[Arg_C++] = &CmdLine[ii];
			}

			PrevC = CmdLine[ii];
			ii++;
		}

		if (IsStr1 != 0) {							/* Check for incomplete strings					*/
			puts("ERROR: Missing end quote \'");
			Arg_C = 0;
		}
		if (IsStr2 != 0) {
			puts("ERROR: Missing end quote \"");
			Arg_C = 0;
		}

		if (Arg_C != 0) {							/* It is not an empty line						*/
			for (ii=0 ; ii<(sizeof(CommandLst)/sizeof(CommandLst[0])); ii++) {
				if (0 == strcmp(CommandLst[ii].Name, Arg_V[0])) {
					printf("\n");
					CommandLst[ii].FctPtr(Arg_C, Arg_V);
					break;
				}
			}
			if (ii == (sizeof(CommandLst)/sizeof(CommandLst[0]))) {
				printf("ERROR: unknown command \"%s\"\n", Arg_V[0]); 
			}
		}
}

/* ------------------------------------------------------------------------------------------------ */

void CleanRead(char *Str)
{
int  ii;											/* Read index									*/
int  jj;											/* Write index									*/

	ii = 0;
	jj = 0;
	while(Str[ii] != '\0') {						/* Process the whole string						*/
		if (Str[ii] == '\b') {						/* This is a backspace							*/
			if (ii != 0) {							/* Not the first char in the string, then go	*/
				jj--;								/* back 1 character in the destination			*/
			}
			ii++;									/* Skip the back space							*/
		}
		else {										/* Is not a back space							*/
			if (Str[ii] == '\t') {					/* Replace TABS by white space					*/
				Str[ii] = ' ';
			}
			Str[jj++] = Str[ii++];					/* Copy the src to the destination				*/
		}
	}
	Str[jj] = '\0';									/* Terminate the string							*/

	return;
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_cat(int argc, char *argv[])
{
char   *Fname;										/* Name of the file to cat						*/
FIL     FileDsc;									/* File descriptor								*/
int     ii;											/* General purpose								*/
int     IsEOF;										/* If eon of file is reached					*/
int     IsWrite;									/* If writing to the file						*/
UINT    Nrd;										/* Number of bytes read							*/
UINT    Nwrt;										/* Number of bytes written						*/
int     RetVal;										/* Return value									*/

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("cat   : Redirect a file to stdout or redirect stdin to a file");
		return(0);
	}

	if (argc < 2) {									/* Needs 1 or 2 options on the command line		*/
		RetVal = 1;
	}

	IsWrite = 0;
	if (RetVal == 0) {
		if (argv[1][0] == '>') {					/* Is a write to a file							*/
			IsWrite = 1;
			if (argv[1][1] != '\0') {				/* If no space between '>' & file name			*/
				Fname = &argv[1][1];				/* The file name starts at the 2nd character	*/
				if (argc != 2) {					/* Can only have 2 tokens then					*/
					RetVal = 1;
				}
			}
			else {									/* Space between '>' & file name				*/
				Fname = argv[2];					/* The file name is the 3rd token				*/
				if (argc != 3) {					/* Can only have 3 tokens then					*/
					RetVal = 1;
				}
			}
		}
		else if (argv[1][0] == '<'){				/* In case using > for write					*/
			if (argv[1][1] != '\0') {				/* If no space between '>' & file name			*/
				Fname = &argv[1][1];				/* The file name starts at the 2nd character	*/
				if (argc != 2) {					/* Can only have 2 tokens then					*/
					RetVal = 1;
				}
			}
			else {									/* Space between '>' & file name				*/
				Fname = argv[2];					/* The file name is the 3rd token				*/
				if (argc != 3) {					/* Can only have 3 tokens then					*/
					RetVal = 1;
				}
			}
		}
		else {
			Fname = &argv[1][0];
			if (argc != 2) {
				RetVal = 1;
			}
		}
	}


	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: cat  source_file           Output the file contents on the screen");
		puts("       cat <target_file           Output the file contents on the screen");
		puts("       cat >target_file           Write from terminal to the file");
		puts("                                  Terminate with EOF<CR>");
		RetVal = 1;
	}

	if (RetVal == 0) {
		if (IsWrite == 0) {							/* Reading a file								*/
			if (FR_OK != f_open(&FileDsc, Fname, FA_READ)) {
				printf("ERROR: cannot open file %s\n", Fname);
				RetVal = 1;
			}
			else {
				do {								/* Dump the file contents to stdio until EOF	*/
					if (FR_OK != f_read(&FileDsc, &g_Buffer[0], sizeof(g_Buffer), &Nrd)) {
						PRT_ERROR("ERROR: problems reading the file");
						RetVal = 1;
						Nrd    = 0;
					}
					for (ii=0 ; ii<Nrd ; ii++) {
						putchar(g_Buffer[ii]);
					}
				} while (Nrd > 0);
				f_close(&FileDsc);
			}
		}
		else {										/* Writing to a file							*/
			if (FR_OK == f_stat(Fname, NULL)) {		/* If the file to write exist, delete it first	*/
				if (FR_OK != f_unlink(Fname)) {
					printf("ERROR: cannot overwrite the file %s\n", Fname);
					RetVal = 1;
				}
			}
			if (RetVal == 0) {
				if  (FR_OK != f_open(&FileDsc, Fname, FA_CREATE_ALWAYS|FA_WRITE)) {
					printf("ERROR: cannot open file %s\n", Fname);
					RetVal = 1;
				}
				else {
					IsEOF = 0;
					Nwrt  = 0;
					do {							/* Write to it until EOF from terminal			*/
						ii = getchar();
						if ((ii == EOF)
						||  (ii == 4)) {			/* 4 is CTRL-D									*/
							IsEOF = 1;
						}
						else {
							g_Buffer[Nwrt++] = ii;
						}
						if ((Nwrt >= sizeof(g_Buffer))	/* Buffer full								*/
						|| (IsEOF != 0)) {			/* CTRL-D typed									*/
							if (FR_OK != f_write(&FileDsc, &g_Buffer[0], Nwrt, &Nrd)) {
								PRT_ERROR("ERROR: problems writing the file");
								RetVal = 1;
							}
							if (Nwrt != Nrd) {
								RetVal = 1;
							}
							Nwrt = 0;
						}
					} while (IsEOF == 0);
					f_close(&FileDsc);
				}
			}
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_cd(int argc, char *argv[])
{
int   RetVal;										/* Return value									*/


	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("cd    : Change directory");
		return(0);
	}

	if (argc != 2) {								/* Need a directory name or ..					*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: cd dir_name                Change directory to dir_name or up with dir_name=..");
		RetVal = 1;
	}

	if (RetVal == 0) {
		if (FR_OK != f_chdir(argv[1])) {			/* Use FatFS directly							*/
			PRT_ERROR("ERROR: cannot change directory");
			RetVal = 1;
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_chmod(int argc, char *argv[])
{
BYTE ReadOnly;										/* Setting read-only or read-write				*/
int  RetVal;										/* Return value									*/

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("chmod : Change a file / directory access modes");
		return(0);
	}

	if (argc != 3) {								/* Need a mode + directory name or a file name	*/
		RetVal = 1;
	}

	ReadOnly = (BYTE)0;								/* Assume is chmod +w							*/
	if (0 == strcmp(argv[1], "-w")) {
		ReadOnly = AM_RDO;
	}
	else if (0 != strcmp(argv[1], "+w")) {
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: chmod -w file_name        Change a file / directory to read-only");
		puts("       chmod +w file_name        Change a file / directory to read-write");
		RetVal = 1;
	}

	if (RetVal == 0) {
		if (FR_OK != f_chmod(argv[2], ReadOnly, AM_RDO)) {
			PRT_ERROR("ERROR: cannot chmod the file");
			RetVal = 1;
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_cp(int argc, char *argv[])
{
FIL   FileDscDst;									/* Destination file descriptor					*/
FIL   FileDscSrc;									/* Source file descriptor						*/
UINT  Nrd;											/* Number of bytes read							*/
UINT  Nwrt;											/* Number of bytes written						*/
int   RetVal;										/* Return value									*/

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("cp    : Copy a file");
		return(0);
	}

	if (argc != 3) {								/* Need a source and destination file			*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: cp src_file dst_file       Copy the file src_file to dst_file");
		RetVal = 1;
	}

	if (RetVal == 0) {
		if (FR_OK != f_open(&FileDscSrc, argv[1], FA_READ)) {
			PRT_ERROR("ERROR: cannot open src file");
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		if (FR_OK == f_stat(argv[2], NULL)) {
			if (FR_OK != f_unlink(argv[2])) {
				PRT_ERROR("ERROR: cannot overwrite dst file");
				RetVal = 1;
				f_close(&FileDscSrc);
			}
		}
	}

	if (RetVal == 0) {
		if  (FR_OK != f_open(&FileDscDst, argv[2], FA_CREATE_ALWAYS|FA_WRITE)) {
			PRT_ERROR("ERROR: cannot open dst file");
			RetVal = 1;
			f_close(&FileDscSrc);
		}
	}

	if (RetVal == 0) {

		do {
			if (FR_OK != f_read(&FileDscSrc, &g_Buffer[0], sizeof(g_Buffer), &Nrd)) {
				PRT_ERROR("ERROR: problems reading the file");
				RetVal = 1;
				Nrd    = 0;
				Nwrt   = 0;
			}
			if (FR_OK != f_write(&FileDscDst, &g_Buffer[0], Nrd, &Nwrt)) {
				PRT_ERROR("ERROR: problems writing the file");
				RetVal = 1;
				Nrd    = 0;
			}
			if (Nrd != Nwrt) {
				PRT_ERROR("ERROR: problems writing the file");
				Nrd = 0;
			}
		} while (Nrd > 0);
		f_close(&FileDscDst);
		f_close(&FileDscSrc);
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_du(int argc, char *argv[])
{
DWORD    Bsize;										/* SD card block size							*/
DWORD    Capacity;									/* SD card number of sectors					*/
uint64_t Free;										/* Free space on the disk						*/
FATFS   *Fsys;										/* Volume to dump the stats						*/
int      ii;										/* General purpose								*/
DWORD    Ncluster;									/* Number of clusters on the disk				*/
char     Path[16];									/* Path of the disk								*/
int      RetVal;									/* Return value									*/
uint64_t Size;										/* Size of the disk								*/
WORD     Ssize;										/* SD card number of bytes per sector			*/


	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("du    : Show disk usage");
		return(0);
	}

	if (argc != 1) {								/* Does not need an argument					*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: du                         Show the disk usage");
		RetVal = 1;
	}

	if (RetVal == 0) {
		ii = 0;
		while((g_DirNowPath[ii] != '\0')
		&&    (g_DirNowPath[ii] != '/')) {
			Path[ii] = g_DirNowPath[ii];
			ii++;
		}
		Path[++ii] = '\0';

		if (FR_OK != f_getfree(Path, &Ncluster, &Fsys)) {
			PRT_ERROR("ERROR: problems retrieving the information");
			RetVal = 1;
		}

		if (RetVal == 0) {
			Free  = (uint64_t)Ncluster;
			Size  = (uint64_t)Fsys->n_fatent-2;		/* Number of FAT entries						*/
			Free *= Fsys->csize;					/* Time # sectors per clusters					*/
			Size *= Fsys->csize;
		  #if ((_MAX_SS) == 512)
			Free *= 512;							/* Time # bytes per sectors						*/
			Size *= 512;							/* Time # bytes per sectors						*/
		  #else
			Free *= Fsys->ssize;					/* Time # bytes per sectors						*/
			Size *= Fsys->ssize;					/* Time # bytes per sectors						*/
		  #endif
			printf("Disk size:%12llu bytes\n", Size);
			printf("Disk free:%12llu bytes\n", Free);
			printf("Disk used:%12llu bytes\n", Size-Free);

			ii = g_Drive[0]-'0';
			disk_ioctl(ii, GET_SECTOR_COUNT, &Capacity);
			disk_ioctl(ii, GET_SECTOR_SIZE,  &Ssize);

		  #if (((OS_PLATFORM) & 0x00FFFFFF) == 0x000007020)
			if (0 == strncmp(MEDIAinfo(ii), "SDMMC", 5)) {
				puts("\nOn ZYNQ, the two next fields are invalid\n");
			}
		  #endif
			printf("Dev size :%12llu bytes\n", ((uint64_t)Capacity) * ((uint64_t)Ssize));
			disk_ioctl(ii, GET_BLOCK_SIZE,  &Bsize);
			printf("Blk size :%12lu bytes\n", 512*Bsize);
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_fmt(int argc, char *argv[])
{
char Drive[3];
int  RetVal;										/* Return value									*/
static char Buf[4096];

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("fmt   : Format a drive");
		return(0);
	}

	if (argc != 2) {								/* Need a volume name to format					*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: fmt device                 Format the device");
		RetVal = 1;
	}

	if (RetVal == 0) {
	  #if ((VERBOSE) > 0) 
		puts("Formatting started");
	  #endif										/* Let f_mkfs decide the au size & others		*/
		Drive[0] = argv[1][0];
		Drive[1] = ':';
		Drive[2] = '\0';
		if (FR_OK != f_mkfs(Drive, FM_ANY, 0, &Buf[0], sizeof(Buf))) {
			PRT_ERROR("ERROR: format of the drive failed");
			RetVal = 1;
		}
	}

  #if ((VERBOSE) > 0) 
	if (RetVal == 0) {
		puts("Done");
	}
  #endif

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_help(int argc, char *argv[])
{
int ii;												/* General purpose								*/
int RetVal;											/* Return value									*/


	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("help  : Help");
		return(0);
	}

	if (argc == 1) {								/* Print the help for all commands				*/
		puts("usage: help                       Show help for all commands");
		puts("       help cmd ...               Show help for specified commands");
		putchar('\n');
	}
	if ((argc == 100)
	||  (argc ==   1)) {

		for (ii=0 ; ii<(sizeof(CommandLst)/sizeof(CommandLst[0])) ; ii++) {
			CommandLst[ii].FctPtr(-1, NULL);
		}
		return(0);
	}

	if (argc == 2) {								/* Print the help for the specified command		*/
		for (ii=0 ; ii<(sizeof(CommandLst)/sizeof(CommandLst[0])) ; ii++) {
			if (0 == strcmp(argv[1], CommandLst[ii].Name)) {
				(void)CommandLst[ii].FctPtr(-1, NULL);
				return(0);
			}
		}
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: help                       Show help for all commands");
		puts("       help cmd ...               Show help for specified commands");
		RetVal = 1;
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_ls(int argc, char *argv[])
{
FILINFO FileInfo;
int RetVal;											/* Return value									*/

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("ls    : List the current directory contents");
		return(0);
	}

	if (argc != 1) {								/* No argument accepted							*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: ls                         List the directory contents");
		RetVal = 1;
	}

	if (RetVal == 0) {								/* Refresh the current directory path			*/
		if (FR_OK != f_getcwd(&g_DirNowPath[0], sizeof(g_DirNowPath))) {
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		f_opendir(&g_Dir, &g_DirNowPath[0]);		/* Open the directory							*/

		while((FR_OK == f_readdir(&g_Dir, &FileInfo))	/* Scan all directory entries				*/
		&&    (FileInfo.fname[0] != '\0')) {
			putchar((FileInfo.fattrib & AM_DIR) ? 'd' : ' ');
			putchar('r');
			putchar((FileInfo.fattrib & AM_RDO) ? '-' : 'w');
			putchar('x');
			printf(" (%04d.%02d.%02d %02d:%02d:%02d) ", (((int)FileInfo.fdate>>9) &0x7F)+1980,
			                                            (((int)FileInfo.fdate>>5) &0x0F),
			                                            (((int)FileInfo.fdate)    &0x1F),
			                                            (((int)FileInfo.ftime>>11)&0x1F),
			                                            (((int)FileInfo.ftime>> 5)&0x3F),
			                                            (((int)FileInfo.ftime)    &0x1F)*2);
			printf(" %10lu ", (unsigned long)FileInfo.fsize);
		  #if _USE_LFN
			if (FileInfo.fname[0] != '\0') {
				puts(FileInfo.fname);
			}
			else {
				puts(&FileInfo.fname[0]);
			}
		  #else
			puts(&FileInfo.fname[0]);
		  #endif
		}
	}
	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_mkdir(int argc, char *argv[])
{
int RetVal;											/* Return value									*/


	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("mkdir : Make a new directory");
		return(0);
	}

	if (argc != 2) {								/* Need the name of the directory to create		*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: mkdir dir_name             Make a new directory with the name dir_name");
		RetVal = 1;
	}

	if (RetVal == 0) {
		if (FR_OK != f_mkdir(argv[1])) {			/* Direct use of FatFS function					*/
			PRT_ERROR("ERROR: cannot create directory");
			RetVal = 1;
		}
	}
	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_mnt(int argc, char *argv[])
{
char DirName[16];
int RetVal;											/* Return value									*/
int VolNmb;

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("mnt   : Mount a drive");
		return(0);
	}

	if (argc != 2) {								/* Need he volume and the mount point			*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: mnt device                 Mount a file system device number");
		RetVal = 1;
	}

	if (RetVal == 0) {
		VolNmb = strtol(argv[1], NULL, 10);
		if ((VolNmb < 0)
		||  (VolNmb >= _VOLUMES)) {
		  #if ((_VOLUMES) == 1)
			PRT_ERROR("ERROR: volume number must be 0 (only one file system available)");
		  #else
			printf("ERROR: volume number must be between 0 and %d\n", (_VOLUMES)-1);
		  #endif
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		g_Drive[0] = argv[1][0];
		g_Drive[1] = ':';
		g_Drive[2] = '\0';
		if (FR_OK != f_mount(&g_FileSys, g_Drive, 1)) {
			printf("ERROR: cannot mount volume %s\n", argv[1]);
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		strcpy(DirName, g_Drive);
		strcat(DirName, "/");
		if (FR_OK != f_opendir(&g_Dir, DirName)) {
			printf("ERROR: cannot opendir on volume %s\n", argv[1]);
			RetVal = 1;
		}
	}

  #if ((_VOLUMES) != 1)
	if (RetVal == 0) {
		if (FR_OK != f_chdrive(g_Drive)) {
			printf("ERROR: cannot chdrive to %s\n", g_Drive);
			RetVal = 1;
		}
	}
  #endif

  #if ((VERBOSE) > 0)
	if (RetVal == 0) {
		puts("The volume is now mounted");
	}
  #endif

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_mv(int argc, char *argv[])
{
int  DoCP;											/* If crossing directory, need to copy & delete	*/
int  RetVal;										/* Return value									*/


	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("mv    : Move / rename a file");
		return(0);
	}

	if (argc != 3) {								/* Need a source and destination name			*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: mv file_name new_name      Rename file_name to new_name");
		RetVal = 1;
	}

	if (RetVal == 0) {
	DoCP = (NULL != strchr(argv[1], '/'))
	     | (NULL != strchr(argv[2], '/'));
		if (DoCP == 0) {
			if (FR_OK != f_rename(argv[1], argv[2])) {
				RetVal = 1;
			}
		}
		else {
			RetVal = cmd_cp(argc, argv);
			if (RetVal == 0) {
				if (FR_OK != f_unlink(argv[1])) {
					RetVal = 1;
				}
			}
		}
		if (RetVal != 0) {
			PRT_ERROR("ERROR: renaming the file");
			RetVal = 1;
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_pwd(int argc, char *argv[])
{
int RetVal;											/* Return value									*/


	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("pwd   : Show current directory");
		return(0);
	}

	if (argc != 1) {								/* Does not accept arguments					*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: pwd                        Print the current working directory");
		RetVal = 1;
	}

	if (RetVal == 0) {
		if (FR_OK != f_getcwd(&g_DirNowPath[0], sizeof(g_DirNowPath))) {
			PRT_ERROR("ERROR: cannot get directory information");
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		printf("Current directory: %s\n", &g_DirNowPath[0]);
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_perf(int argc, char *argv[])
{
UINT   BlkSize;
char  *Buffer;
char  *Cptr;
FIL    FileDsc;										/* File descriptor								*/
UINT   Left;
UINT   Nrd;
UINT   Nwrt;
int    RetVal;										/* Return value									*/
UINT   Size;
int    StartTime;
double Time;
static const char Fname[] = "__PERF__";
FILINFO FileInfo;


	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("perf  : Throughput measurements");
		return(0);
	}

	if (argc != 3) {								/* Need the size of the transfers				*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: perf                       Measure the read and write transfer rates");
		puts("       perf Nbytes BlkSize        Nbytes : file size to write then read");
		puts("                                  BlkSize: block size to use");
		RetVal = 1;
	}

	if (RetVal == 0) {
		Size    = (UINT)strtoul(argv[1], &Cptr, 10);
		if ((*Cptr=='k')
		||  (*Cptr=='K')) {
			Size *= 1024;
		}
		if ((*Cptr=='m')
		||  (*Cptr=='M')) {
			Size *= 1024*1024;
		}
		BlkSize = (UINT)strtoul(argv[2], &Cptr, 10);
		if ((*Cptr=='k')
		||  (*Cptr=='K')) {
			BlkSize *= 1024;
		}
		if ((*Cptr=='m')
		||  (*Cptr=='M')) {
			BlkSize *= 1024*1024;
		}
		Buffer  = &g_Buffer[0];
		if (BlkSize > sizeof(g_Buffer)) {
			Buffer = malloc(BlkSize);
			if (Buffer == (char *)NULL) {
				puts("ERROR: cannot allocate memory");
				RetVal = 1;
			}

		}
		memset(&Buffer[0], 0x55, BlkSize);
		TSKsleep(2);								/* Do this to make sure the test always same	*/
		if ((RetVal == 0)
		&& (FR_OK == f_stat(Fname, NULL))) {		/* If the file to write exist, delete it first	*/
			if (FR_OK != f_unlink(Fname)) {
				printf("ERROR: cannot overwrite the file %s\n", Fname);
				RetVal = 1;
			}
		}
		if (RetVal == 0) {
			printf("%d bytes file using R/W block size of %d bytes\n", Size, BlkSize);
			if  (FR_OK != f_open(&FileDsc, Fname, FA_CREATE_ALWAYS|FA_WRITE)) {
				printf("ERROR: cannot open file %s\n", Fname);
				RetVal = 1;
			}
			else {
				puts("Starting test");
				Left      = Size;
				StartTime = G_OStimCnt;
				do {
					Nwrt = Left;
					if (Nwrt > BlkSize) {
						Nwrt = BlkSize;
					}
					Left -= Nwrt;
					if (FR_OK != f_write(&FileDsc, &Buffer[0], Nwrt, &Nrd)) {
						PRT_ERROR("ERROR: problems writing the file");
						RetVal = 1;
						Left   = 0U;
					}
					else {
						if (Nrd != Nwrt) {
							PRT_ERROR("ERROR: problems writing the file");
							RetVal = 1;
							Left   = 0U;;
						}
					}
				} while (Left != 0U);
				StartTime = G_OStimCnt
				          - StartTime;

				f_close(&FileDsc);

				Time = ((double)StartTime)/(1000000.0/OX_TIMER_US);
				if (RetVal == 0) {
					printf("[%7.3lfs] Write rate %9.3lf kB/s\n",
					       Time, ((double)Size/1000.0)/Time);
				}
			}
			if ((RetVal == 0)
			&&  (FR_OK != f_open(&FileDsc, Fname, FA_READ))) {
				printf("ERROR: cannot open file %s\n", Fname);
				RetVal = 1;
			}
			else {
				Left      = Size;
				StartTime = G_OStimCnt;
				do {
					Nrd = Left;
					if (Nrd > BlkSize) {
						Nrd = BlkSize;
					}
					Left -= Nrd;
					if (FR_OK != f_read(&FileDsc, &Buffer[0], BlkSize, &Nrd)) {
						PRT_ERROR("ERROR: problems reading the file");
						RetVal = 1;
						Left   = 0U;
					}
				} while (Left != 0U);
				StartTime = G_OStimCnt
				          - StartTime;

				f_close(&FileDsc);

				Time = ((double)StartTime)/(1000000.0/OX_TIMER_US);
				if (RetVal == 0) {
					printf("[%7.3lfs] Read rate  %9.3lf kB/s\n",
					       Time, ((double)Size/1000.0)/Time);
				}
				f_close(&FileDsc);
			}
		}
		if ((BlkSize > sizeof(g_Buffer))
		&&  (Buffer != (char *)NULL)) {
			free(Buffer);
		}
	}

	if (RetVal == 0) {
		memset(&FileInfo, 0, sizeof(FileInfo));		/* Must zero otherwise garbage pointer			*/
		if (FR_OK != f_stat(Fname, &FileInfo)) {	/* doc: Pointer to the blank FILINFO structure	*/
			PRT_ERROR("ERROR: cannot stat the file");
			RetVal = 1;			
		}
	}

	if (RetVal == 0) {
		if (FR_OK != f_unlink(Fname)) {
			PRT_ERROR("ERROR: cannot remove the file");
			RetVal = 1;
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_rm(int argc, char *argv[])
{
FILINFO FileInfo;
int RetVal;											/* Return value									*/

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("rm    : Remove / delete a file");
		return(0);
	}

	if (argc != 2) {								/* Need the file name to delete					*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: rm file_name               Delete the file file_name");
		RetVal = 1;
	}

	if (RetVal == 0) {
		memset(&FileInfo, 0, sizeof(FileInfo));		/* Must zero otherwise garbage pointer			*/
		if (FR_OK != f_stat(argv[1], &FileInfo)) {	/* doc: Pointer to the blank FILINFO structure"	*/
			PRT_ERROR("ERROR: cannot stat the file");
			RetVal = 1;			
		}
	}

	if (RetVal == 0) {
		if (FileInfo.fattrib & AM_DIR) {
			PRT_ERROR("ERROR: this is a directory, use rmdir");
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		if (FileInfo.fattrib & AM_RDO) {
			PRT_ERROR("ERROR: the file is read-only");
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		if (FR_OK != f_unlink(argv[1])) {
			PRT_ERROR("ERROR: cannot remove the file");
			RetVal = 1;
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_rmdir(int argc, char *argv[])
{
FILINFO FileInfo;
int RetVal;											/* Return value									*/


	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("rmdir : Remove / delete a directory");
		return(0);
	}

	if (argc != 2) {								/* Need the directory name to delete	   		*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: rmdir dir_name             Delete the directory dirname");
		RetVal = 1;
	}

	if (RetVal == 0) {
		memset(&FileInfo, 0, sizeof(FileInfo));		/* Must zero otherwise garbage pointer			*/
		if (FR_OK != f_stat(argv[1], &FileInfo)) {	/* doc: Pointer to the blank FILINFO structure"	*/
			PRT_ERROR("ERROR: cannot stat the file");
			RetVal = 1;			
		}
	}

	if (RetVal == 0) {
		if (!(FileInfo.fattrib & AM_DIR)) {
			PRT_ERROR("ERROR: this is not a directory, use rm");
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		if (FileInfo.fattrib & AM_RDO) {
			PRT_ERROR("ERROR: the directory is read-only");
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		if (FR_OK != f_unlink(argv[1])) {
			PRT_ERROR("ERROR: cannot remove the directory");
			RetVal = 1;
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_umnt(int argc, char *argv[])
{
char Drive[3];
int  RetVal;										/* Return value									*/
int  VolNmb;

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("umnt  : Unmount a drive");
		return(0);
	}

	if (argc != 2) {								/* Need he volume and the mount point			*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: umnt device                Unmount a file system device number");
		RetVal = 1;
	}

	VolNmb = strtol(argv[1], NULL, 10);
	if ((VolNmb < 0)
	||  (VolNmb >= _VOLUMES)) {
	  #if ((_VOLUMES) == 1)
		PRT_ERROR("ERROR: volume number must be 0 (only one file system available)");
	  #else
		printf("ERROR: volume number must be between 0 and %d\n", (_VOLUMES)-1);
	  #endif
		RetVal = 1;
	}

	if (RetVal == 0) {
		Drive[0] = argv[1][0];
		Drive[1] = ':';
		Drive[2] = '\0';
		if (FR_OK != f_mount(NULL, Drive, 0)) {
			printf("ERROR: cannot unmount volume %s\n", argv[1]);
			RetVal = 1;
		}
	}

  #if ((VERBOSE) > 0)
	if (RetVal == 0) {
		puts("The volume is now unmounted");
	}
  #endif

	return(RetVal);
}

/* Needed for the interrupt call-back of GPIOs when target platform rerquites it					*/
/* ------------------------------------------------------------------------------------------------ */

void GPIOintHndl(int IOPin)
{
	return;
}

/* EOF */
