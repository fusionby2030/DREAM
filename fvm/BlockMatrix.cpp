/**
 * Implementation of the DREAM equation system class.
 */

#include <softlib/SFile.h>
#include <iostream>
#include <petscmat.h>
#include "FVM/config.h"
#include "FVM/BlockMatrix.hpp"

using namespace DREAM::FVM;
using namespace std;

/**
 * Constructor.
 */
BlockMatrix::BlockMatrix() { }

/**
 * Destructor.
 */
BlockMatrix::~BlockMatrix() {
    this->Destroy();
}

/**
 * Construct the matrix
 */
void BlockMatrix::ConstructSystem() {
    // Determine matrix size
    PetscInt mSize = this->next_subindex;

    // Calculate number of non-zero elements in matrix
    PetscInt *nnz = new PetscInt[mSize];
    for (vector<struct _subeq>::iterator it = this->subeqs.begin(); it != this->subeqs.end(); it++) {
        for (PetscInt i = 0; i < it->n; i++)
            nnz[it->offset + i] = it->nnz;
    }

    this->Construct(mSize, mSize, 0, nnz);

    delete [] nnz;
}

/**
 * Defines a new "sub-equation" to include in the matrix. The
 * sub-equation appears by getting its own (square) matrix block
 * in the large matrix, and can be accessed as a separate matrix.
 *
 * n:    Number of elements in solution vector.
 * nnz:  Number of non-zero elements in matrix block row
 *       representing this equation.
 */
len_t BlockMatrix::CreateSubEquation(const PetscInt n, const PetscInt nnz) {
    // Define index set
    struct _subeq se;
    se.n      = n;
    se.nnz    = nnz;
    se.offset = this->next_subindex;

    this->subeqs.push_back(se);

    this->next_subindex += n;

    return (this->subeqs.size()-1);
}

/**
 * Turns this block matrix from 'M' into the new matrix
 * 'I - dt*M', where 'I' is the identity matrix.
 *
 * dt: Factor by which to rescale all elements of
 *     this block matrix.
 *
 * WARNING: This routine is relatively slow for block matrices!
 */
void BlockMatrix::IMinusDtA(const PetscScalar dt) {
    Vec v;
    VecCreateSeq(PETSC_COMM_WORLD, n, &v);
    
    const PetscInt offs = this->rowOffset;
    for (PetscInt i = 0; i < this->blockn; i++)
        VecSetValue(v, offs+i, -dt, INSERT_VALUES);

    VecAssemblyBegin(v);
    VecAssemblyEnd(v);

    // Rescale all elements
    MatDiagonalScale(this->petsc_mat, NULL, v);

    for (PetscInt i = 0; i < this->blockn; i++)
        this->SetElement(i, i, 1, ADD_VALUES);

    VecDestroy(&v);
}

/**
 * Get sub equation offset.
 */
PetscInt BlockMatrix::GetOffset(const PetscInt subeq) {
    return this->subeqs.at(subeq).offset;
}

/**
 * Sets which sub-equation to write into.
 *
 * subeq1: Index of equation to set matrix to (block row index of sub-matrix).
 * subeq2: Index of unknown to set matrix to (block row column of sub-matrix).
 */
void BlockMatrix::SelectSubEquation(const PetscInt subeq1, const PetscInt subeq2) {
    this->SetOffset(this->subeqs.at(subeq1).offset, this->subeqs.at(subeq2).offset);
    this->blockn = this->subeqs.at(subeq1).n;
}

/**
 * Sets the matrix corresponding to the specified
 * sub-equation to zero.
 *
 * subeq1: Index of equation for which matrix should be zeroed (block row index of sub-matrix).
 * subeq2: Index of unknown for which matrix should be zeroed (block row column of sub-matrix).
 */
void BlockMatrix::ZeroEquation(const PetscInt subeq) {
    IS is;
    ISCreateStride(PETSC_COMM_WORLD, this->subeqs.at(subeq).n, this->subeqs.at(subeq).offset, 1, &is);

    MatZeroRowsColumnsIS(this->petsc_mat, is, 0, nullptr, nullptr);

    ISDestroy(&is);
}

