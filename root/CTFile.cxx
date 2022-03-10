/**
 * @author Andrey
 * @version 1.2
 * @date 17.08.2010
 *
 * Interface to use of ROOT TFile in pure C application. Just several functions are implemented now.
 */

#include "TFile.h"
#include "CTFile.h"

void *CTFile_TFile(const char *fname, const char *option)
/**
 * Opens a ROOT file named 'fname'. 'option' is same as in TFile, i.e. RECREATE, READ, etc. The return value is pointer
 * to TFile object (converted to type (void *). It is used to identificate the object when calling all the interface
 * functions.
 */
{
	return (void *)new TFile(fname, option);
}

void *CTFile_Get(void *file, const char *name)
/**
 * Gets a TObject from the file. Returns pointer to it, converted to (void *).
 */
{
	return (void *)((TFile *)file)->Get(name);
}

void *CTFile_GetDirectory(void *file, const char *path)
/**
 * Returns pointer to TDirectory (converted to (void *)) specified by 'path'. If 'path' is empty returns root directory
 * of the file.
 */
{
	return (void *)((TFile *)file)->GetDirectory(path);
}

short CTFile_IsZombie(void *file)
/**
 * Checks if opened file really exists or if it's a valid ROOT file.
 */
{
	return (short)((TFile *)file)->IsZombie();
}

void *CTFile_mkdir(void *file, const char *name)
/**
 * Makes directory in the current one. Arguments are pointer to TFile (converted to (void *)) and directory name.
 * Returns pointer to TDirectory converted to (void *).
 */
{
	return (void *)((TFile *)file)->mkdir(name);
}

void CTFile_cd(void *file, const char *path)
/**
 * Changes current directory to the given one (if exists). 'file' identifies opened TFile.
 */
{
	((TFile *)file)->cd(path);
}

void CTFile_Write(void *file)
/**
 * Writes all the ROOT object in memory of TFile 'file' to disk.
 */
{
	((TFile *)file)->Write();
}

void CTFile_Close(void *file)
/**
 * Closes specified file and deletes object corresponding 'file' pointer.
 */
{
	((TFile *)file)->Close();
	delete (TFile *)file;
}
