#ifndef IMAGE_LIST_H_l4038vmsh3487gk
#define IMAGE_LIST_H_l4038vmsh3487gk

#include <memory>
#include "../WindowObjects.h"

namespace tk {

class ImageList {
public:
    typedef int Index;
    static Index const noIndex;

    virtual Index add(ImageRef ih) = 0;
    virtual void replace(ImageRef ih, Index index) = 0;
    virtual void remove(Index index) = 0;

    virtual WDims dims() const = 0;
    virtual int count() const = 0;

    virtual void clear() = 0;

    virtual ~ImageList() {};
};

}

#endif
