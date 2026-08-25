#include <ais/serialization/Datum.h> // For Boost serialization

#ifndef _${DataType}_h_
#define _${DataType}_h_
class ${DataType}Storage
{
public:
  ${DataType}Storage() /* Initialize Fields Here */ { }

  template <class Archive>
  void serialize(Archive& ar, unsigned int version)
  {
    /* Archive Fields Here */
  }

private:
  /* Add Fields Here */
};

typedef Datum<${DataType}Storage> ${DataType};

BOOST_CLASS_VERSION(${DataType}Storage, 1);
#endif

