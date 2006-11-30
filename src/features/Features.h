/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 1999-2006 Soeren Sonnenburg
 * Written (W) 1999-2006 Gunnar Raetsch
 * Copyright (C) 1999-2006 Fraunhofer Institute FIRST and Max-Planck-Society
 */

#ifndef _CFEATURES__H__
#define _CFEATURES__H__

#include "lib/common.h"
#include "preproc/PreProc.h"
#include <stdio.h>
#include <cstring>
using namespace std;

class CPreProc;
class CFeatures;

class FeatureException{
      private:
         char *mes;
      public:
         FeatureException(const char *_mes) {
            mes = new char[strlen(_mes)];
            strcpy(mes,_mes);
         }

         char* get_debug_string() {
            return mes;
         }
};

class CFeatures
{
public:
	/** Features can 
	 * just be DREALs, SHORT
	 * or STRINGs, FILES, or...
	 *
	 * size - cache size
	*/
	CFeatures(INT size);

	// copy constructor
	CFeatures(const CFeatures& orig);

	/** load features from file
	 * fname - filename
	 */

	CFeatures(CHAR* fname);

	virtual CFeatures* duplicate() const=0 ;

	virtual ~CFeatures();

	/** return feature type with which objects derived 
	    from CFeatures can deal
	*/
	virtual EFeatureType get_feature_type()=0;

	/** return feature class
	    like Sparse,Simple,...
	*/
	virtual EFeatureClass get_feature_class()=0;
		
	/// set preprocessor
	virtual INT add_preproc(CPreProc* p);
	
	/// delete preprocessor from list
	/// caller has to clean up returned preproc
	virtual CPreProc* del_preproc(INT num);

	/// get specified preprocessor
	CPreProc* get_preproc(INT num);
	
	/// set applied flag for preprocessor
	inline void set_preprocessed(INT num) { preprocessed[num]=true; }

	/// get whether specified preprocessor was already applied
	inline bool is_preprocessed(INT num) { return preprocessed[num]; }

	/// get the number of applied preprocs
	INT get_num_preprocessed();

	/// get number of preprocessors
	inline INT get_num_preproc() { return num_preproc; }

	/// clears all preprocs
	void clean_preprocs();

	/// get cache size
	inline INT get_cache_size() { return cache_size; };

	/// return the number of examples
	virtual INT get_num_vectors()=0 ;

	//in case there is a feature matrix allow for reshaping
	virtual bool reshape(INT num_features, INT num_vectors) { return false; }

	/** return size (in bytes) a single element (e.g. sizeof(float))
	    requires
	*/
	virtual INT get_size()=0;

	void list_feature_obj();

	virtual bool load(CHAR* fname);
	virtual bool save(CHAR* fname);

	bool check_feature_compatibility(CFeatures* f);
	
private:
	/// size of cache in MB
	INT cache_size;

	/// list of preprocessors
	CPreProc** preproc;

	/// number of preprocs in list
	INT num_preproc;

	/// i'th entry is true if features were already preprocessed with preproc i
	bool* preprocessed;
};
#endif
