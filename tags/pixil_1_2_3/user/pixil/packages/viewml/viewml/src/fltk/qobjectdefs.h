#ifndef __QOBJECTDEFS_H
#define __QOBJECTDEFS_H

#define emit

#define METHOD(a)       "0"#a
#define SLOT(a)         "1"#a
#define SIGNAL(a)       "2"#a

#define Q_NAME2(a,b)            Q_NAME2_AUX(a,b)
#define Q_NAME2_AUX(a,b)        a##b
//#define Q_DECLARE(a,t)          Q_NAME2(a,declare)(t)
#define Q_DECLARE(a,t) ;

#define  QListM_QConnection QConnectionList

class QConnectionListIt;
class QConnectionList;

#define Q_OBJECT \
public:                                                                       \
    QMetaObject *metaObject() const { return metaObj; }                       \
    const char  *className()  const;                                          \
protected:                                                                    \
    virtual void         initMetaObject();                                    \
private:                                                                      \
    static QMetaObject *metaObj;

#endif
