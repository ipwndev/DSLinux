/*
 single path classifier
*/

#ifndef _SC_H_

#define _SC_H_

#define	MAXSCLASSES	100

typedef struct sclassifier *sClassifier;	/* single feature-vector classifier */
typedef int sClassIndex;	/* single feature-vector class index */
typedef struct sclassdope *sClassDope;	/* single feature-vector class dope */

struct sclassdope
{
    char *name;
    sClassIndex number;
    Vector average;
    Matrix sumcov;
    int nexamples;
};

struct sclassifier
{
    int nfeatures;
    int nclasses;
    sClassDope *classdope;

    Vector cnst;		/* constant term of discrimination function */
    Vector *w;			/* array of coefficient weights */
    Matrix invavgcov;		/* inverse covariance matrix */
};

sClassifier sNewClassifier();
sClassifier sRead();		/* FILE *f */
void sWrite();			/* FILE *f; sClassifier sc; */
void sFreeClassifier();		/* sc */
void sAddExample();		/* sc, char *classname; Vector y */
void sRemoveExample();		/* sc, classname, y */
void sDoneAdding();		/* sc */
sClassDope sClassify();		/* sc, y */
sClassDope sClassifyAD();	/* sc, y, float *ap; float *dp */
sClassDope sClassNameLookup();	/* sc, classname */
float MahalanobisDistance();	/* Vector v, u; Matrix sigma */
void FixClassifier();
void sDumpClassifier();
void sDistances();

#endif
