#ifndef _SVM_H___
#define _SVM_H___

#include "lib/common.h"
#include "features/Features.h"
#include "kernel/Kernel.h"
#include "kernel/KernelMachine.h"

#include <stdio.h>

class CSVM : public CKernelMachine
{
	public:
		CSVM();
		virtual ~CSVM();

		bool load(FILE* svm_file);
		bool save(FILE* svm_file);

		inline void set_C(REAL c1, REAL c2) { C1=c1; C2=c2; }
		inline void set_weight_epsilon(REAL eps) { weight_epsilon=eps; }

		inline REAL get_weight_epsilon() { return weight_epsilon; }
		inline REAL get_C1() { return C1; }
		inline REAL get_C2() { return C2; }

		inline int get_support_vector(int idx)
		{
			assert(svm_model.svs && idx<svm_model.num_svs);
			return svm_model.svs[idx];
		}

		inline REAL get_alpha(int idx)
		{
			assert(svm_model.alpha && idx<svm_model.num_svs);
			return svm_model.alpha[idx];
		}

		inline bool set_support_vector(int idx, INT val)
		{
			if (svm_model.svs && idx<svm_model.num_svs)
				svm_model.svs[idx]=val;
			else
				return false;

			return true;
		}

		inline bool set_alpha(int idx, REAL val)
		{
			if (svm_model.alpha && idx<svm_model.num_svs)
				svm_model.alpha[idx]=val;
			else
				return false;

			return true;
		}

		inline REAL get_bias()
		{
			return svm_model.b;
		}

		inline void set_bias(double bias)
		{
			svm_model.b=bias;
		}

		inline int get_num_support_vectors()
		{
			return svm_model.num_svs;
		}

		inline bool create_new_model(int num)
		{
			delete[] svm_model.alpha;
			delete[] svm_model.svs;

			svm_model.b=0;
			svm_model.num_svs=num;
			svm_model.alpha= new double[num];
			svm_model.svs= new int[num];

			return (svm_model.alpha!=NULL && svm_model.svs!=NULL);
		}

		inline void use_kerncombination(bool enable)
		{
			use_mkl=enable;
		}

		inline bool is_mkl_enabled()
		{
			return use_mkl;
		}

		REAL* test();

		CLabels* classify(CLabels* labels=NULL);
		REAL classify_example(INT num);

	protected:

		/// an SVM is defined by support vectors, their coefficients alpha
		/// and the bias b ( + CKernelMachine::get_kernel())
		struct TModel
		{
			REAL b;

			REAL* alpha;
			int* svs;

			int num_svs;
		};

		TModel svm_model;
		bool svm_loaded;

		REAL weight_epsilon;

		REAL C1;
		REAL C2;
		bool use_mkl;
};
#endif
