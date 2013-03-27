#include "../../components/ImageList.h"
#include "ImageListWin32.h"

namespace tk {
    
ImageList::ImageList(WDims dims, int capacity):
    impl(std::make_shared<ImageListImpl>(dims,capacity))
{
}

ImageList::ImageList(std::shared_ptr<ImageListImpl> impl):
    impl(impl)
{
}

ImageList::~ImageList()
{}

ImageList::Index ImageList::add(ImageRef ih)
{
    return impl->add(ih);
}

void ImageList::replace(ImageRef ih, ImageList::Index index)
{
    impl->replace(ih,index);
}

void ImageList::remove(ImageList::Index index)
{
    impl->remove(index);
}

WDims ImageList::dims() const
{
    return impl->dims();
}

int ImageList::count() const
{
    return impl->count();
}

void ImageList::clear()
{
    impl->clear();
}

}
