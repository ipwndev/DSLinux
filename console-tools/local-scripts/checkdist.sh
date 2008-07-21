#! /bin/bash

PACKAGE=$(basename $(pwd))

VERSION=$(grep AM_INIT_AUTOMAKE configure.in | 
    sed "s/AM_INIT_AUTOMAKE(${PACKAGE},//; s/)//; s/ //g")

DISTDIR=${PACKAGE}-${VERSION}

echo "VERSION = \`$VERSION'"
diff <( find . \
    -not -path "*/CVS**" \
    -not -path "./builddir*" \
    -not -path "./debian*" \
    -not -path "./${DISTDIR}*" \
    -not -path "./working*" \
    -not -path "./attic*" \
    -not -path "./*/.deps*" \
    -not -name "Makefile" \
    -not -name "Make.rules" \
    -not -name ".cvsignore" \
    -not -path "./libtool" \
    -not -path "./*/backup*" \
    | sort
) <( cd ${DISTDIR} && find | sort)
