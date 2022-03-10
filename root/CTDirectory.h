/**
 * @author Andrey
 * @version 1.0
 * @date 16.08.2010
 *
 * Interface to use ROOT TDirectory in pure C application. Only few fuctions are implemented at the moment. Description
 * of the functions see in cxx-file.
 */

#ifndef CTDIRECTORY_H
#define CTDIRECTORY_H

#ifdef __cplusplus
extern "C"
{
#endif
	void *CTDirectory_mkdir(void *dir, const char *name);
	void CTDirectory_cd(void *dir, const char *path);
#ifdef __cplusplus
}
#endif

#endif
