#include "ImageListWin32.h"
#include "../ConvertersWin32.h"
#include "../ScopedObjects.h"
#include "../CanvasImplWin32.h"
#include "../../log.h"

namespace tk {

ImageList::Index const ImageList::noIndex = -1;

ImageListImpl::ImageListImpl(WDims dims, int capacity)
{
    himl = ImageList_Create(dims.width,dims.height,ILC_COLOR24,capacity,0);
}

ImageListImpl::~ImageListImpl()
{
    if( ! ImageList_Destroy(himl) ) {
        log::error("failure to destroy imagelist with hWnd ",himl);
    }
}

ImageList::Index ImageListImpl::add(ImageRef ih)
{
    scoped::Bitmap bmImage(ihToBitmap(ih));
    int result = ImageList_Add(himl,bmImage.get(),0);
    if( result == -1 ) {
        log::error("failure to add image into imagelist with hWnd ",himl);
    }
    return result;
}

void ImageListImpl::replace(ImageRef ih, ImageList::Index index)
{
    scoped::Bitmap bmImage(ihToBitmap(ih));
    BOOL result = ImageList_Replace(himl,index,bmImage.get(),0);
    if( result == 0 ) {
        log::error("failure to replace image ",index," into imagelist with hWnd ",himl);
    }
}

void ImageListImpl::remove(ImageList::Index index)
{
    BOOL result = ImageList_Remove(himl,index);
    if( result == 0 ) {
        log::error("failure to remove image ",index," from imagelist with hWnd ",himl);
    }
}

void ImageListImpl::clear()
{
    remove(-1);
}

void ImageListImpl::drawInto(CanvasImpl & canvas, ImageList::Index index, Point pos)
{
    if( ImageList_DrawEx(himl,index,canvas.getHDC(),pos.x,pos.y,0,0,CLR_NONE,CLR_NONE,ILD_IMAGE) == 0 ) {
        log::error("failure to draw image ",index," from imagelist with hWnd ",himl);
    }
}

WDims ImageListImpl::dims() const
{
    int cx,cy;
    BOOL result = ImageList_GetIconSize(himl,&cx,&cy);
    if( result == 0 ) {
        log::error("failure to get iconsize from imagelist with hWnd ",himl);
    }
    return WDims(cx,cy);
}

int ImageListImpl::count() const
{
    return ImageList_GetImageCount(himl);
}

HIMAGELIST ImageListImpl::getHiml() const
{
    return himl;
}

}
