/* +---------------------------------------------------------------------------+
   |          The Mobile Robot Programming Toolkit (MRPT) C++ library          |
   |                                                                           |
   |                   http://mrpt.sourceforge.net/                            |
   |                                                                           |
   |   Copyright (C) 2005-2010  University of Malaga                           |
   |                                                                           |
   |    This software was written by the Machine Perception and Intelligent    |
   |      Robotics Lab, University of Malaga (Spain).                          |
   |    Contact: Jose-Luis Blanco  <jlblanco@ctima.uma.es>                     |
   |                                                                           |
   |  This file is part of the MRPT project.                                   |
   |                                                                           |
   |     MRPT is free software: you can redistribute it and/or modify          |
   |     it under the terms of the GNU General Public License as published by  |
   |     the Free Software Foundation, either version 3 of the License, or     |
   |     (at your option) any later version.                                   |
   |                                                                           |
   |   MRPT is distributed in the hope that it will be useful,                 |
   |     but WITHOUT ANY WARRANTY; without even the implied warranty of        |
   |     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         |
   |     GNU General Public License for more details.                          |
   |                                                                           |
   |     You should have received a copy of the GNU General Public License     |
   |     along with MRPT.  If not, see <http://www.gnu.org/licenses/>.         |
   |                                                                           |
   +---------------------------------------------------------------------------+ */
#ifndef CSparseMatrix_H
#define CSparseMatrix_H

#include <mrpt/utils/utils_defs.h>
#include <mrpt/math/CSparseMatrixTemplate.h>

#include <mrpt/math/math_frwds.h>

extern "C"{
#include <mrpt/otherlibs/CSparse/cs.h>
}

namespace mrpt
{
	namespace math
	{
		/** A sparse matrix capable of efficient math operations (a wrapper around the CSparse library)
		  *  The type of cells is fixed to "double".
		  *
		  * \note This class was initially adapted from "robotvision", by Hauke Strasdat, Steven Lovegrove and Andrew J. Davison. See http://www.openslam.org/robotvision.html
		  * \note CSparse is maintained by Timothy Davis: http://people.sc.fsu.edu/~jburkardt/c_src/csparse/csparse.html
		  *
		  * \sa mrpt::math::CMatrixFixedNumeric, mrpt::math::CMatrixTemplateNumeric, etc.
		  */
		class BASE_IMPEXP CSparseMatrix
		{
		private:
			cs sparse_matrix;

			/** Initialization from a dense matrix of any kind existing in MRPT. */
			template <class MATRIX>
			void construct_from_mrpt_mat(const MATRIX & C)
			{
				std::vector<int> row_list, col_list;  // Use "int" for convenience so we can do a memcpy below...
				std::vector<double> content_list;
				const int nCol = C.getColCount();
				const int nRow = C.getRowCount();
				for (int c=0; c<nCol; ++c)
				{
					col_list.push_back(row_list.size());
					for (int r=0; r<nRow; ++r)
						if (C.get_unsafe(r,c)!=0)
						{
							row_list.push_back(r);
							content_list.push_back(C(r,c));
						}
				}
				col_list.push_back(row_list.size());

				sparse_matrix.m = nRow;
				sparse_matrix.n = nCol;
				sparse_matrix.nzmax = content_list.size();
				sparse_matrix.i = (int*)malloc(sizeof(int)*row_list.size());
				sparse_matrix.p = (int*)malloc(sizeof(int)*col_list.size());
				sparse_matrix.x = (double*)malloc(sizeof(double)*content_list.size());

				::memcpy(sparse_matrix.i, &row_list[0], sizeof(row_list[0])*row_list.size() );
				::memcpy(sparse_matrix.p, &col_list[0], sizeof(col_list[0])*col_list.size() );
				::memcpy(sparse_matrix.x, &content_list[0], sizeof(content_list[0])*content_list.size() );

				sparse_matrix.nz = -1;
			}

			/** Initialization from a triplet "cs", which is first compressed */
			void construct_from_triplet(const cs & triplet);

			/** free buffers (deallocate the memory of the i,p,x buffers) */
			void internal_free_mem();


		public:

			/** @name Constructors, destructor & copy operations
			    @{  */

			/** Create an initially empty sparse matrix */
			CSparseMatrix(const size_t nRows, const size_t nCols); 

			/** Best way to initialize a sparse matrix from a list of non NULL elements */
			template <typename T>
			CSparseMatrix(const CSparseMatrixTemplate<T> & data)
			{
				ASSERTMSG_(!data.empty(), "Input data must contain at least one non-zero element.")
				sparse_matrix.i = NULL; // This is to know they shouldn't be tried to free()
				sparse_matrix.p = NULL;
				sparse_matrix.x = NULL;

				// 1) Create triplet matrix
				CSparseMatrix  triplet(data.getRowCount(),data.getColCount());
				// 2) Put data in:
				for (typename CSparseMatrixTemplate<T>::const_iterator it=data.begin();it!=data.end();++it)
					triplet.insert_entry(it->first.first,it->first.second, it->second);

				// 3) Compress:
				construct_from_triplet(triplet.sparse_matrix);
			}


			// We can't do a simple "template <class ANYMATRIX>" since it would be tried to match against "cs*"...

			/** Constructor from a dense matrix of any kind existing in MRPT. */
			template <typename T,size_t N,size_t M> inline CSparseMatrix(const CMatrixFixedNumeric<T,N,M> &MAT) { construct_from_mrpt_mat(MAT); }

			/** Constructor from a dense matrix of any kind existing in MRPT. */
			template <typename T>  inline CSparseMatrix(const CMatrixTemplateNumeric<T> &MAT) { construct_from_mrpt_mat(MAT); }

			/** Copy constructor */
			CSparseMatrix(const CSparseMatrix & other);

			/** Copy constructor from an existing "cs" CSparse data structure */
			CSparseMatrix(const cs  * const sm);

			/** Destructor */
			virtual ~CSparseMatrix();

			/** Copy the data from an existing "cs" CSparse data structure */
			void  copy(const cs  * const sm);

			/** Fast copy the data from an existing "cs" CSparse data structure, copying the pointers and leaving NULLs in the source structure. */
			void  copy_fast(cs  * const sm);

			/** Copy operator from another existing object */
			void operator = (const CSparseMatrix & other);

			/** @}  */

			/** @name Math operations (the interesting stuff...)
				@{ */

			CSparseMatrix operator + (const CSparseMatrix & other) const;
			CSparseMatrix operator * (const CSparseMatrix & other) const;

			std::vector<double> operator * (const std::vector<double> & other) const;

			void operator += (const CSparseMatrix & other);
			void operator *= (const CSparseMatrix & other);
			CSparseMatrix transpose() const;

			/** @}  */


			/** @ Access the matrix, get, set elements, etc.
			    @{ */

			/** Insert a <b>new</b> non-zero entry in the matrix. */
			void insert_entry(const size_t row, const size_t col, const double val );

			/** Return a dense representation of the sparse matrix */
			void get_dense(CMatrixDouble &outMat) const;

			// Very basic, standard methods that MRPT methods expect for any matrix:
			inline size_t getRowCount() const { return sparse_matrix.m; }
			inline size_t getColCount() const { return sparse_matrix.n; }


			/** @} */

		}; // end class CSparseMatrix

	}
}
#endif
