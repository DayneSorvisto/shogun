#include "lib/common.h"
#include "lib/Mathmatics.h"
#include "kernel/LinearByteKernel.h"
#include "kernel/ByteKernel.h"
#include "features/ByteFeatures.h"
#include "lib/io.h"

#include <assert.h>

CLinearByteKernel::CLinearByteKernel(LONG size)
  : CByteKernel(size),scale(1.0),normal(NULL)
{
}

CLinearByteKernel::~CLinearByteKernel() 
{
	cleanup();
}
  
bool CLinearByteKernel::init(CFeatures* l, CFeatures* r, bool do_init)
{
	CByteKernel::init(l, r, do_init); 

	if (do_init)
		init_rescale() ;

	CIO::message(M_INFO, "rescaling kernel by %g (num:%d)\n",scale, CMath::min(l->get_num_vectors(), r->get_num_vectors()));

	return true;
}

void CLinearByteKernel::init_rescale()
{
	LONGREAL sum=0;
	scale=1.0;
	for (INT i=0; (i<lhs->get_num_vectors() && i<rhs->get_num_vectors()); i++)
			sum+=compute(i, i);

	if ( sum > (pow((double) 2, (double) 8*sizeof(LONG))) )
		CIO::message(M_ERROR, "the sum %lf does not fit into integer of %d bits expect bogus results.\n", sum, 8*sizeof(LONG));
	scale=sum/CMath::min(lhs->get_num_vectors(), rhs->get_num_vectors());
}

void CLinearByteKernel::cleanup()
{
	delete_optimization();
}

bool CLinearByteKernel::load_init(FILE* src)
{
    assert(src!=NULL);
    UINT intlen=0;
    UINT endian=0;
    UINT fourcc=0;
    UINT r=0;
    UINT doublelen=0;
    double s=1;

    assert(fread(&intlen, sizeof(BYTE), 1, src)==1);
    assert(fread(&doublelen, sizeof(BYTE), 1, src)==1);
    assert(fread(&endian, (UINT) intlen, 1, src)== 1);
    assert(fread(&fourcc, (UINT) intlen, 1, src)==1);
    assert(fread(&r, (UINT) intlen, 1, src)==1);
    assert(fread(&s, (UINT) doublelen, 1, src)==1);
    CIO::message(M_INFO, "detected: intsize=%d, doublesize=%d, r=%d, scale=%g\n", intlen, doublelen, r, s);
	scale=s;
	return true;
}

bool CLinearByteKernel::save_init(FILE* dest)
{
    BYTE intlen=sizeof(UINT);
    BYTE doublelen=sizeof(double);
    UINT endian=0x12345678;
    BYTE fourcc[5]="LINK"; //id for linear kernel

    assert(fwrite(&intlen, sizeof(BYTE), 1, dest)==1);
    assert(fwrite(&doublelen, sizeof(BYTE), 1, dest)==1);
    assert(fwrite(&endian, sizeof(UINT), 1, dest)==1);
    assert(fwrite(&fourcc, sizeof(char), 4, dest)==1);
    assert(fwrite(&scale, sizeof(double), 1, dest)==1);
    CIO::message(M_INFO, "wrote: intsize=%d, doublesize=%d, scale=%g\n", intlen, doublelen, scale);

	return true;
}

void CLinearByteKernel::clear_normal()
{
	int num = lhs->get_num_vectors();

	for (int i=0; i<num; i++)
		normal[i]=0;
}

void CLinearByteKernel::add_to_normal(INT idx, REAL weight) 
{
	INT vlen;
	bool vfree;
	BYTE* vec=((CByteFeatures*) lhs)->get_feature_vector(idx, vlen, vfree);

	for (int i=0; i<vlen; i++)
		normal[i]+= weight*vec[i];

	((CByteFeatures*) lhs)->free_feature_vector(vec, idx, vfree);
}
  
REAL CLinearByteKernel::compute(INT idx_a, INT idx_b)
{
  INT alen, blen;
  bool afree, bfree;

  BYTE* avec=((CByteFeatures*) lhs)->get_feature_vector(idx_a, alen, afree);
  BYTE* bvec=((CByteFeatures*) rhs)->get_feature_vector(idx_b, blen, bfree);
  assert(alen==blen);

  double sum=0;
  for (INT i=0; i<alen; i++)
	  sum+=((LONG) avec[i])*((LONG) bvec[i]);
  REAL result=sum/scale;

  ((CByteFeatures*) lhs)->free_feature_vector(avec, idx_a, afree);
  ((CByteFeatures*) rhs)->free_feature_vector(bvec, idx_b, bfree);

  return result;
}

bool CLinearByteKernel::init_optimization(INT num_suppvec, INT* sv_idx, REAL* alphas) 
{
	CIO::message(M_DEBUG,"drin gelandet yeah\n");
	INT alen;
	bool afree;
	int i;

	int num_feat=((CByteFeatures*) lhs)->get_num_features();
	assert(num_feat);

	normal=new REAL[num_feat];
	assert(normal);

	for (i=0; i<num_feat; i++)
		normal[i]=0;

	for (int i=0; i<num_suppvec; i++)
	{
		BYTE* avec=((CByteFeatures*) lhs)->get_feature_vector(sv_idx[i], alen, afree);
		assert(avec);

		for (int j=0; j<num_feat; j++)
			normal[j]+= alphas[i] * ((double) avec[j]);

		((CByteFeatures*) lhs)->free_feature_vector(avec, 0, afree);
	}

	set_is_initialized(true);
	return true;
}

bool CLinearByteKernel::delete_optimization()
{
	delete[] normal;
	normal=NULL;

	set_is_initialized(false);

	return true;
}

REAL CLinearByteKernel::compute_optimized(INT idx_b) 
{
	INT blen;
	bool bfree;

	BYTE* bvec=((CByteFeatures*) rhs)->get_feature_vector(idx_b, blen, bfree);

	double result=0;
	{
		for (INT i=0; i<blen; i++)
			result+= normal[i] * ((double) bvec[i]);
	}
	result/=scale;

	((CByteFeatures*) rhs)->free_feature_vector(bvec, idx_b, bfree);

	return result;
}
