#include "drag.h"


QMetaObject * KDNDWidget::metaObj = 0;


void KDNDWidget::initMetaObject()
{
}

const char * KDNDWidget::className() const
{
  return "KDNDWidget";
}

