/*****************************************************************************
 * The following license applies to the code between the solid rows          *
 * of comments. The code was obtained from:                                  *
 *  - http://hadjieleftheriou.com/frequent-items/index.html                  *
 *****************************************************************************
 * Count-Min Sketches                                                        *
 *                                                                           *
 * G. Cormode 2003,2004                                                      *
 *                                                                           *
 * Initial version: 2003-12                                                  *
 *                                                                           *
 * This work is licensed under the Creative Commons                          *
 * Attribution-NonCommercial License. To view a copy of this license,        *
 * visit http://creativecommons.org/licenses/by-nc/1.0/ or send a letter     *
 * to Creative Commons, 559 Nathan Abbott Way, Stanford, California          *
 * 94305, USA.                                                               *
 *****************************************************************************/

#include "cormode_cmh.h"

#include <math.h>
#include <stdio.h>

#include "hh/hh.h"
#include "hh/sketch.h"
#include "util/cormode_prng.h"

// Initialization
CMH_type *hh_cormode_cmh_create(heavy_hitter_params_t *restrict p) {
	hh_cormode_cmh_params_t *restrict params = (hh_cormode_cmh_params_t *)p->params;

	int gran           = 1;

	const uint8_t logm = ceil(log2(params->m)/log2(2*gran));
	double delta       = (double)((params->delta*params->phi)/(2.*gran*logm));

	uint32_t width     = ceil(params->b / params->epsilon) * p->hash->c;
	uint32_t depth     = ceil(log2(1 / delta) / log2(params->b));

	int U              = xceil_log2(params->m);

	// CMH_type * CMH_Init(int width, int depth, int U, int gran)
	//CMH_type *hh = CMH_Init(w, d, xceil_log2(params->m), 1);

///////////////////////////////////////////////////////////////////////////////
  CMH_type * cmh;
  int i,j,k;
  prng_type * prng;

  if (U<=0 || U>32) return(NULL);
  // U is the log the size of the universe in bits

  if (gran>U || gran<1) return(NULL);
  // gran is the granularity to look at the universe in 
  // check that the parameters make sense...

  cmh=(CMH_type *) calloc(1,sizeof(CMH_type));

  prng=prng_Init(-12784,2);
  // initialize the generator for picking the hash functions

  if (cmh && prng)
    {
      cmh->depth=depth;
      cmh->width=width;
      cmh->count=0;
      cmh->U=U;
      cmh->gran=gran;
      cmh->levels=(int) ceil(((float) U)/((float) gran));
      for (j=0;j<cmh->levels;j++)
	if ((int64_t) 1<<(cmh->gran*j) <= cmh->depth*cmh->width)
	  cmh->freelim=j;
      //find the level up to which it is cheaper to keep exact counts
      cmh->freelim=cmh->levels-cmh->freelim;
      
      cmh->counts=(int **) calloc(sizeof(int *), 1+cmh->levels);
      cmh->hasha=(unsigned int **)calloc(sizeof(unsigned int *),1+cmh->levels);
      cmh->hashb=(unsigned int **)calloc(sizeof(unsigned int *),1+cmh->levels);
      j=1;
      for (i=cmh->levels-1;i>=0;i--)
	{
	  if (i>=cmh->freelim)
	    { // allocate space for representing things exactly at high levels
	      cmh->counts[i]=(int *) calloc(1<<(cmh->gran*j),sizeof(int));
	      j++;
	      cmh->hasha[i]=NULL;
	      cmh->hashb[i]=NULL;
	    }
	  else 
	    { // allocate space for a sketch
	      cmh->counts[i]=(int *)calloc(sizeof(int), cmh->depth*cmh->width);
	      cmh->hasha[i]=(unsigned int *)
		calloc(sizeof(unsigned int),cmh->depth);
	      cmh->hashb[i]=(unsigned int *)
		calloc(sizeof(unsigned int),cmh->depth);

	      if (cmh->hasha[i] && cmh->hashb[i])
		for (k=0;k<cmh->depth;k++)
		  { // pick the hash functions
		    cmh->hasha[i][k]=prng_int(prng) & MOD;
		    cmh->hashb[i][k]=prng_int(prng) & MOD;
		  }
	    }
	}
    }
///////////////////////////////////////////////////////////////////////////////


#ifdef SPACE
	printf("Space: %d\n", CMH_Size(hh));
#endif

	cmh->phi = params->phi;
	cmh->L1 = 0;

	return cmh;
}

// Destruction
void hh_cormode_cmh_destroy(CMH_type *restrict cmh) {
	// void CMH_Destroy(CMH_type * cmh)
	//CMH_Destroy(hh);

///////////////////////////////////////////////////////////////////////////////
  int i;
  if (!cmh) return;
  for (i=0;i<cmh->levels;i++)
    {
      if (i>=cmh->freelim)
	{
	  free(cmh->counts[i]);
	}
      else 
	{
	  free(cmh->hasha[i]);
	  free(cmh->hashb[i]);
	  free(cmh->counts[i]);
	}
    }
  free(cmh->counts);
  free(cmh->hasha);
  free(cmh->hashb);
  free(cmh);
  cmh=NULL;
///////////////////////////////////////////////////////////////////////////////
}

// Update
void hh_cormode_cmh_update(CMH_type *restrict cmh, const uint32_t idx, 
		const int64_t diff) {
	// void CMH_Update(CMH_type * cmh, unsigned int item, int diff)
	//CMH_Update(hh, idx, c);

	uint32_t item = idx;
///////////////////////////////////////////////////////////////////////////////
  int i,j,offset;

  if (!cmh) return;
  cmh->count+=diff;
  for (i=0;i<cmh->levels;i++)
    {
      offset=0;
      if (i>=cmh->freelim)
	{
	  cmh->counts[i][item]+=diff;
	  // keep exact counts at high levels in the hierarchy  
	}
      else
	for (j=0;j<cmh->depth;j++)
	  {
	    cmh->counts[i][(comode_hash31(cmh->hasha[i][j],cmh->hashb[i][j],item) 
			    % cmh->width) + offset]+=diff;
	    // this can be done more efficiently if the width is a power of two
	    offset+=cmh->width;
	  }
      item>>=cmh->gran;
    }
///////////////////////////////////////////////////////////////////////////////

	cmh->L1 += diff;
}



// Helpers
//
static int CMH_count(CMH_type * cmh, int depth, int item)
{
  // return an estimate of item at level depth

  int j;
  int offset;
  int estimate;

  if (depth>=cmh->levels) return(cmh->count);
  if (depth>=cmh->freelim)
    { // use an exact count if there is one
      return(cmh->counts[depth][item]);
    }
  // else, use the appropriate sketch to make an estimate
  offset=0;
  estimate=cmh->counts[depth][(comode_hash31(cmh->hasha[depth][0],
				      cmh->hashb[depth][0],item) 
			       % cmh->width) + offset];
  for (j=1;j<cmh->depth;j++)
    {
      offset+=cmh->width;
      estimate=min(estimate,
		   cmh->counts[depth][(comode_hash31(cmh->hasha[depth][j],
					      cmh->hashb[depth][j],item) 
				       % cmh->width) + offset]);
    }
  return(estimate);
}

static void CMH_recursive(CMH_type * cmh, int depth, int start, 
//		    int thresh, std::map<uint32_t, uint32_t>& res)
			int thresh, int * results)
{
	// for finding heavy hitters, recursively descend looking 
	// for ranges that exceed the threshold

	int i;
	int blocksize;
	int estcount;
	int itemshift;

	estcount=CMH_count(cmh,depth,start);

	if (estcount>=thresh) 
	{
		if (depth==0)
		{
//			if (res.size()<cmh->width)
			if (results[0]<cmh->width -1)
			{
//				res.insert(std::pair<uint32_t, uint32_t>(start, estcount));
				results[0]++;
				results[results[0]]=start;
			}
		}
		else
		{
			blocksize=1<<cmh->gran;
			itemshift=start<<cmh->gran;
			// assumes that gran is an exact multiple of the bit dept
			for (i=0;i<blocksize;i++)
//				CMH_recursive(cmh,depth-1,itemshift+i,thresh,res);
				CMH_recursive(cmh,depth-1,itemshift+i,thresh,results);
		}
	}
}



// Query
heavy_hitter_t *hh_cormode_cmh_query(CMH_type *restrict cmh) {
	int threshold = cmh->L1 * cmh->phi;
	int i;

	//int * CMH_FindHH(CMH_type * cmh, int thresh)
	//int *results = CMH_FindHH(hh, threshold);
///////////////////////////////////////////////////////////////////////////////
	int * results;
	results=(int *) calloc(cmh->width,sizeof(int));
	results[0]=0;

	CMH_recursive(cmh,cmh->levels,0,threshold,results);
///////////////////////////////////////////////////////////////////////////////

	heavy_hitter_t *hitters = xmalloc(sizeof(heavy_hitter_t));

	hitters->hitters = xmalloc(results[0] * sizeof(uint32_t));
	hitters->count   = results[0];
	hitters->size    = results[0];

	for (i = 0; i < results[0]; i++) {
		hitters->hitters[i] = results[i + 1];
	}

	return hitters;
}
