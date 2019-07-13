#include "serials.h"

class SerialsData : public QSharedData
{
public:

};

Serials::Serials() : data(new SerialsData)
{

}

Serials::Serials(const Serials &rhs) : data(rhs.data)
{

}

Serials &Serials::operator=(const Serials &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

Serials::~Serials()
{

}
