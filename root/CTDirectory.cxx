/**
 * @author Andrey
 * @version 1.0
 * @date 16.08.2010
 *
 * Interface to use ROOT TDirectory in pure C application. Only few fuctions are implemented at the moment.
 */

#include "TDirectory.h"

#include "CTDirectory.h"

void *CTDirectory_mkdir(void *dir, const char *name)
/**
 * Creates directory of given name in directory 'dir'. Returns poiner to the new directory converted to (void *).
 */
{
	return (void *)((TDirectory *)dir)->mkdir(name);
}

void CTDirectory_cd(void *dir, const char *path)
/**
 * Changes current directory to the one given by 'path' or to 'dir' if 'path' is empty. Relative path is evaluated
 * with respect to 'dir'.
 */
{
	((TDirectory *)dir)->cd(path);
}
