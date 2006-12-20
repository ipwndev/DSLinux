#ifndef __QIODEVICE_H
#define __QIODEVICE_H

#define IO_ReadOnly             0x0001          // readable device
#define IO_WriteOnly            0x0002          // writeable device
#define IO_ReadWrite            0x0003          // read+write device
#define IO_Append               0x0004          // append
#define IO_Truncate             0x0008          // truncate device
#define IO_Translate            0x0010          // translate CR+LF
#define IO_ModeMask             0x00ff

class QIODevice
{
 public:
  bool isOpen() const { return false; }
  virtual bool open(int mode) { return true; }
  virtual void close() { }
  virtual int writeBlock( const char * data, unsigned int len) { return 0; }
  virtual int readBlock(char * data, uint maxlen) { return 0; }
  virtual bool atEnd() const { return false; }
  virtual int getch() const { return 1; }
};
#endif
