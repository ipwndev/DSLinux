#include <Flek/FVector2.H>
#include <Flek/FVector3.H>
#include <Flek/FVector4.H>

#include <Flek/FMatrix3x3.H>
#include <Flek/FMatrix4x4.H>

void
FVector3::copy_from(FVector4 const &v)
{
    elem[0] = v[0];
    elem[1] = v[1];
    elem[2] = v[2];
}

void
FVector3::copy_from(FVector2 const &v)
{
    elem[0] = v[0];
    elem[1] = v[1];
    elem[0] = 0;
}

void
FVector2::copy_from(FVector4 const &v)
{
    elem[0] = v[0];
    elem[1] = v[1];
}

void
FVector2::copy_from(FVector3 const &v)
{
    elem[0] = v[0];
    elem[1] = v[1];
}

void
FVector4::copy_from(FVector3 const &v)
{
    elem[0] = v[0];
    elem[1] = v[1];
    elem[2] = v[2];
}

void
FVector4::copy_from(FVector2 const &v)
{
    elem[0] = v[0];
    elem[1] = v[1];
}

/*
  The following functions are defined outside the class so that they use the
  friend versions of member functions instead of the member functions themselves
*/

FMatrix4x4
operator *(const FMatrix4x4 & mat1, const FMatrix4x4 & mat2)
{
    FMatrix4x4 prod, trans;

    // Find the transpose of the 2nd matrix
    trans = transpose(mat2);

    // The columns of mat2 are now the rows of trans
    // Multiply appropriate rows and columns to get the product
    prod.row[0].set(mat1.row[0] * trans.row[0],
		    mat1.row[0] * trans.row[1],
		    mat1.row[0] * trans.row[2], mat1.row[0] * trans.row[3]);
    prod.row[1].set(mat1.row[1] * trans.row[0],
		    mat1.row[1] * trans.row[1],
		    mat1.row[1] * trans.row[2], mat1.row[1] * trans.row[3]);
    prod.row[2].set(mat1.row[2] * trans.row[0],
		    mat1.row[2] * trans.row[1],
		    mat1.row[2] * trans.row[2], mat1.row[2] * trans.row[3]);
    prod.row[3].set(mat1.row[3] * trans.row[0],
		    mat1.row[3] * trans.row[1],
		    mat1.row[3] * trans.row[2], mat1.row[3] * trans.row[3]);
    return prod;
}

FVector4
operator *(const FVector4 & vec, const FMatrix4x4 & mat)
{
    return (transpose(mat) * vec);
}

/*
  The following functions are defined outside the class so that they use the
  friend versions of member functions instead of the member functions themselves
*/

FMatrix3x3
operator *(const FMatrix3x3 & mat1, const FMatrix3x3 & mat2)
{
    FMatrix3x3 prod, trans;

    // Find the transpose of the 2nd matrix
    trans = transpose(mat2);

    // The columns of mat2 are now the rows of trans
    // Multiply appropriate rows and columns to get the product
    prod.row[0].set(mat1.row[0] * trans.row[0],
		    mat1.row[0] * trans.row[1], mat1.row[0] * trans.row[2]);
    prod.row[1].set(mat1.row[1] * trans.row[0],
		    mat1.row[1] * trans.row[1], mat1.row[1] * trans.row[2]);
    prod.row[2].set(mat1.row[2] * trans.row[0],
		    mat1.row[2] * trans.row[1], mat1.row[2] * trans.row[2]);
    return prod;
}

FVector3
operator *(const FVector3 & vec, const FMatrix3x3 & mat)
{
    return (transpose(mat) * vec);
}
