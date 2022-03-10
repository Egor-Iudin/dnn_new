/**
 * @author Andrey
 * @version 1.1
 * @date 17.08.2010
 *
 * Interface to use ROOT TTree in pure C application. Only few fuctions are implemented at the moment. Description
 * of the functions see in cxx-file.
 */

#ifndef CTTREE_H
#define CTTREE_H

#ifdef __cplusplus
extern "C"
{
#endif
	void *CTTree_TTree(const char *name, const char *title);
	void CTTree_Branch(void *tree, const char *name, void *address, const char *leaflist);
	void CTTree_Delete(void *tree);
	void CTTree_Fill(void *tree);
	void *CTTree_GetBranchAddress(void *tree, const char *bname);
	long CTTree_GetEntries(void *tree);
	void CTTree_GetEntry(void *tree, long entry);
	void CTTree_GetListOfBranchNames(void *tree, char **bnames);
	short CTTree_GetNbranches(void *tree);
	void CTTree_SetBranchAddress(void *tree, const char *bname, void *address);
	void CTTree_SetBranchStatus(void *tree, const char *bname, int status);
	void CTTree_Write(void *tree);
#ifdef __cplusplus
}
#endif

#endif