#ifndef _GLPRINT_H_
#define _GLPRINT_H_

#include <windows.h>

class GLPrint
{
public:
  GLPrint() {};
  ~GLPrint()  {};

  void SetDeviceContext(HDC hdc);
  void Print(double x, double y, const char *format, ...);

private:
  GLuint m_base; //< display list id base for font
};

#endif // _GLPRINT_H_