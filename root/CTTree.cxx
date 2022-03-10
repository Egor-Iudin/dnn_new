/**
 * @author Andrey
 * @version 1.1
 * @date 17.08.2010
 *
 * Interface to use ROOT TTree in pure C application. Only few fuctions are implemented at the moment.
 */

#include "string.h"

#include "TTree.h"
#include "TObjArray.h"

#include "CTTree.h"

void *CTTree_TTree(const char *name, const char *title)
/**
 * Creates a ROOT TTree with given name and title. Returns pointer to the object converted to (void *) which is used
 * to identify the tree within all the interface functions.
 */
{
	return (void *)new TTree(name, title);
}

void CTTree_Branch(void *tree, const char *name, void *address, const char *leaflist)
/**
 * Declares a branch and associate it with the given address.
 */
{
	((TTree *)tree)->Branch(name, address, leaflist);
}

void CTTree_Delete(void *tree)
/**
 * Deletes the tree.
 */
{
	((TTree *)tree)->Delete();
}

void CTTree_Fill(void *tree)
/**
 * Fills branches of the given tree with values of associated variables.
 */
{
	((TTree *)tree)->Fill();
}

void *CTTree_GetBranchAddress(void *tree, const char *bname)
/**
 * Returns address associated with the branch.
 */
{
	return (void *)((TTree *)tree)->GetBranch(bname)->GetAddress();
}

long CTTree_GetEntries(void *tree)
/**
 * Returns number of entries in the tree.
 */
{
	return ((TTree *)tree)->GetEntries();
}

void CTTree_GetEntry(void *tree, long entry)
/**
 * Reads the specified entry from the tree to associated variables.
 */
{
	((TTree *)tree)->GetEntry(entry);
}

void CTTree_GetListOfBranchNames(void *tree, char **bnames)
/**
 * This function has no analogy among members of TTree. It stores names of all branches of the given tree in array
 * 'bnames'. User must allocate memory before calling this function.
 */
{
	TObjArray branches = *((TTree *)tree)->GetListOfBranches();

	for (short i = 0; i < ((TTree *)tree)->GetNbranches(); i++)
		strcpy(bnames[i], branches[i]->GetName());
}

short CTTree_GetNbranches(void *tree)
/**
 * Returns number of branches in the tree.
 */
{
	return (short)((TTree *)tree)->GetNbranches();
}

void CTTree_SetBranchAddress(void *tree, const char *bname, void *address)
/**
 * Associates given branch with the address.
 */
{
	((TTree *)tree)->SetBranchAddress(bname, address);
}

void CTTree_SetBranchStatus(void *tree, const char *bname, int status)
/**
 * Set status (i.e. active/inactive) of the branch specified with its name.
 */
{
	((TTree *)tree)->SetBranchStatus(bname, status);
}

void CTTree_Write(void *tree)
/**
 * Writes tree to current file.
 */
{
	((TTree *)tree)->Write();
}
