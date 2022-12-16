#ifndef __CONVERTER_BASE_H_
#define __CONVERTER_BASE_H_

template<class Src, class Dst>
class Converter {
public:
  Converter();
  virtual ~Conveter();
  virtual Src ToSrc(const Dst&) = 0;
  virtual Dst ToDst(const Src&) = 0;
};

#endif
