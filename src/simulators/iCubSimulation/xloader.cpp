/**
 * \file xloader.cpp
 * \brief This loads a mesh from a 3D file and loads it onto the simulator..
 * \date 2007
 * \note Release under GNU GPL v2.0
 **/

#include "xloader.h"
#if _MSC_VER
#pragma warning(disable:4996 4244 4305)
#endif

const float ModelScale = 1.01;
//---------------------------------------------------------------
//---------------------------------------------------------------
dxTriMeshX *dLoadMeshFromX(const char* FileName)
{
	dxTriMeshX *tmpTriMesh = new dxTriMeshX;
	char word[256];
	char*  symbol;
	int  indexCount;
	char *p;
	int  i, j=0;
	int ret;
	FILE *in;
	double dblval;
	int intval;

	if ((in = fopen(FileName, "r")) == NULL) {
		printf ("Can't open the file '%s'\n", FileName);
		return 0;
		}
	else {
		printf("Loading mesh data from '%s' ", FileName);
    	
		while((symbol = fgets(word, 256, in)) != (char*) NULL) { // Read till the end of file
	  		if (strcmp(word, "template") == 0) {
				ret=fscanf(in, "%s", word);
				ret=fscanf(in, "%s", word);
	 		}else 
	  		if (strncmp(word, "Mesh ", 5) == 0){								// All you need is Mesh !!
				if (strcmp(word, "{") != 0 )	// If the mesh has a name
				//fscanf(in, "%s", word);			// then skip it.
				ret=fscanf(in, "%d", &(tmpTriMesh->VertexCount));	// Get vertex count
				tmpTriMesh->Vertices = (float *)malloc(tmpTriMesh->VertexCount * 3 * sizeof(float));
				ret=fscanf(in, "%s", word);
				printf("...");
        		if (fgets(word, 256, in)==0)
                    return 0;

				for (i = 0; i < tmpTriMesh->VertexCount; i++) {
					//fscanf(in, "%s", word);
            		if (fgets(word, 256, in)==0)
                        return 0;

            		//printf("Read(%d): '%s'\n", i, word);
					p = strtok(word, ",;");		
					while (p != NULL) {
						//printf("p = '%s'\n", p);
                		ret = sscanf(p, "%lf", &dblval);
                		if(ret > 0) { // only process if double was read
				  			tmpTriMesh->Vertices[j] = dblval * ModelScale;
                  			j++;
                		}
						p = strtok(NULL, ",;");
						//printf("j = %d\n", j);
					}
				}
				printf("...");
				ret=fscanf(in, "%d", &indexCount);	// Get index count
				tmpTriMesh->IndexCount = indexCount * 3;
				tmpTriMesh->Indices = (int *)malloc(tmpTriMesh->IndexCount * sizeof(int)); 
	
        		if (fgets(word, 256, in)==0)
                    return 0;
	
				for (i = 0; i < indexCount; i++) {
            		if (fgets(word, 256, in)==0)
                        return 0;

           		    p = strtok(word, ",;");
            		ret = sscanf(p, "%d", &intval);
            		if(intval != 3) {
            	  		printf("Only triangular polygons supported! Convert your model!\n");
            	 		return 0;
            		}
            		for(j = 0; j < 3; j++) {//hardcoded 3
            	    	p = strtok(NULL, ",;");
            	    	ret = sscanf(p, "%d", &intval);
            	    	tmpTriMesh->Indices[i*3 + j] = intval;
            		}	
				}
			    printf("... OK!\n");
	  	    }
	    }
    }
  	fclose(in);

	if ((in = fopen(FileName, "r")) == NULL) {
		printf ("Can't open the file '%s'\n", FileName);
		return 0;
		}
	else {
		printf("Loading texture coordinate from '%s' ", FileName);
    	j=0;
		while((symbol = fgets(word, 256, in)) != (char*) NULL) {
			if (strncmp(word, "MeshTextureCoords", 17) == 0){	
				ret=fscanf(in, "%d", &(tmpTriMesh->MeshCoordCount));
        		if (fgets(word, 256, in)==NULL)
                    return 0; // consume newline

				tmpTriMesh->MeshCoord = (float *)malloc(tmpTriMesh->MeshCoordCount*2 * sizeof(float));
				printf("...");
				for (i = 0; i < tmpTriMesh->MeshCoordCount; i++) {	
					if (fgets(word, 256, in)==0)
                        return 0;

					p = strtok(word, ",;");
					while (p != NULL) {
						ret = sscanf(p, "%lf", &dblval);
						if(ret > 0) {
							tmpTriMesh->MeshCoord[j] = dblval ;//* ModelScale;		
							j++;	
						}
						p = strtok(NULL, ",;");
					}
				}	
				printf("...");				
			}	
			
		}printf("... OK!\n");
	}
	
  	fclose(in);
  	return tmpTriMesh;
}
void dTriMeshXDestroy(dTriMeshX TriMesh)
{
	free (TriMesh->Vertices);
	free (TriMesh->Indices);
    free (TriMesh->MeshCoord);
	free (TriMesh);
}
