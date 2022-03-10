/**
 * @author Andrey
 * @version 1.2
 * @date 17.08.2010
 *
 * Interface to use ROOT TFile in pure C application. Note that only few functions are implemented now. Descriptions
 * of the functions see in cxx-file.
 */

#ifndef CTFILE_H
#define CTFILE_H

#ifdef __cplusplus
extern "C"
{
#endif
	void *CTFile_TFile(const char *fname, const char *option);
	void *CTFile_Get(void *file, const char *name);
	void *CTFile_GetDirectory(void *file, const char *path);
	short CTFile_IsZombie(void *file);
	void *CTFile_mkdir(void *file, const char *name);
	void CTFile_cd(void *file, const char *path);
	void CTFile_Write(void *file);
	void CTFile_Close(void *file);
#ifdef __cplusplus
}
#endif

#endif
