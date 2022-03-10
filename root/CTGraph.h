/**
 * @author Andrey
 * @version 1.2
 * @date 17.08.2010
 *
 * Interface to use ROOT TFile in pure C application. Note that only few functions are implemented now. Descriptions
 * of the functions see in cxx-file.
 */

#ifndef CTGRAPH_H
#define CTGRAPH_H

#ifdef __cplusplus
extern "C"
{
#endif
	void *C_TGraph(int n, const float *x, const float *y);
	void *C_Draw(void *Graph, char *chopt);
	void *C_Write(void *Graph);
	void *C_Print(void *Graph, char *chopt);
	void *C_CreateCanvas(char *name, char *tittle);
	void *C_PrintGraphs(void *canvas, char *fileName);
	void *PrintGraph(int n, const float *x, const float *y, char *filename, char *graphName);

	// short CTFile_IsZombie(void *file);
	// void *CTFile_mkdir(void *file, const char *name);
	// void CTFile_cd(void *file, const char *path);
	// void CTFile_Write(void *file);
	// void CTFile_Close(void *file);
#ifdef __cplusplus
}
#endif

#endif
