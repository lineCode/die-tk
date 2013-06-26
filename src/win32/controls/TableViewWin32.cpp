#define _WIN32_IE 0x0300
#include "TableViewWin32.h"
#include "../components/ImageListWin32.h"
#include "../../controls/base/ItemProperties.h"
#include "../../log.h"

#ifndef MAX_TABLEVIEW_ITEM_CHARS
#define MAX_TABLEVIEW_ITEM_CHARS 261
#endif

namespace tk {

// callbacks here
    
TableViewImpl::TableViewImpl(Window & parent, ControlParams const & params):
//	NativeControlImpl(parent,params,WC_LISTVIEW,LVS_REPORT | LVS_EX_SUBITEMIMAGES),
	NativeControlImpl(parent,params,WC_LISTVIEW,LVS_REPORT),
    colCount(0),
    rowCount(0)
{
    ListView_SetExtendedListViewStyle(hWnd,LVS_EX_SUBITEMIMAGES);
}

TableViewImpl::~TableViewImpl()
{
    // remove callbacks
}


int alignToWin32(ColumnProperties::Alignment alignment)
{
    switch(alignment) {
        case ColumnProperties::left:
            return LVCFMT_LEFT;
        case ColumnProperties::right:
            return LVCFMT_RIGHT;
        case ColumnProperties::center:
            return LVCFMT_CENTER;
    }

    // defaults to left if invalid
    return LVCFMT_LEFT;
}

ColumnProperties::Alignment win32ToAlign(int fmt)
{
    switch(fmt) {
        case LVCFMT_LEFT:
            return ColumnProperties::left;
        case LVCFMT_RIGHT:
            return ColumnProperties::right;
        case LVCFMT_CENTER:
            return ColumnProperties::center;
    }

    // defaults to left if invalid
    return ColumnProperties::left;
}

LVCOLUMN propertiesToColumn(int colIndex, ColumnProperties const & columnProp)
{
    LVCOLUMN lvc;
    lvc.iSubItem = colIndex;
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.pszText = const_cast<wchar_t *>(columnProp.header.wstr.c_str()); // *
    lvc.cx = columnProp.width;
    lvc.fmt = alignToWin32(columnProp.alignment);
    if( columnProp.imageIndex != ImageList::noIndex ) {
        lvc.mask |= LVCF_IMAGE;
        lvc.iImage = columnProp.imageIndex;
    }

    return lvc; // [*] LVCOLUMN pszText has the same scope as columnProp text
}

LVITEM propertiesToItem(int c, int r, ItemProperties const & itemProp)
{
    LVITEM item;
    item.iItem = r;
    item.iSubItem = c;
    item.mask = LVIF_TEXT | LVIF_IMAGE;
    item.pszText = const_cast<wchar_t *>(itemProp.text.wstr.c_str()); // *
    item.iImage = itemProp.imageIndex;
    return item; // [*] LVITEM pszText has the same scope as itemProp text
}

// == //

int TableViewImpl::rows() const 
{ 
    return rowCount; 
}

int TableViewImpl::columns() const
{
    return colCount;
}

void TableViewImpl::addColumn(ColumnProperties columnProp)
{
    LVCOLUMN lvc = propertiesToColumn(colCount,columnProp);
    if( ListView_InsertColumn(hWnd,colCount,&lvc) == -1 ) {
        log::error("ListView_InsertColumn returned -1 for hWnd ",hWnd);
    } else {
        ++colCount;
    }
}

void TableViewImpl::setColumn(int c, ColumnProperties columnProp)
{
    LVCOLUMN lvc = propertiesToColumn(c,columnProp);
    if( ListView_SetColumn(hWnd,c,&lvc) == FALSE ) {
        log::error("ListView_SetColumn returned FALSE for hWnd ",hWnd);        
    }
}

ColumnProperties TableViewImpl::column(int c) const
{
    ColumnProperties result;
    result.header.wstr.resize(MAX_TABLEVIEW_ITEM_CHARS);
    LVCOLUMN lvc;
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.pszText = &result.header.wstr[0];
    lvc.cchTextMax = MAX_TABLEVIEW_ITEM_CHARS;
    if( ListView_GetColumn(hWnd,c,&lvc) == FALSE ) {
        log::error("ListView_GetColumn returned FALSE for hWnd ",hWnd);
    } else {
        result
            .setAlignment(win32ToAlign(lvc.fmt))
            .setImageIndex(lvc.iImage)
            .setWidth(lvc.cx);
    }
    
    return result;
}

void TableViewImpl::deleteColumn(int c)
{
    if( ListView_DeleteColumn(hWnd,c) == FALSE ) {
        log::error("ListView_DeleteColumn returned FALSE for hWnd ",hWnd);        
    }
}

bool TableViewImpl::addItem(int r, ItemProperties itemProp)
{
    LVITEM item = propertiesToItem(0,r,itemProp);
    if( ListView_InsertItem(hWnd,&item) == -1 ) {
        log::error("ListView_InsertItem returned -1 for hWnd ",hWnd);
        return false;
    }
    
    return true;
}

void TableViewImpl::setRows(int r)
{
    if( r > rowCount ) {
        ListView_SetItemCount(hWnd,r);
        ItemProperties emptyText;
        int rowsAdded = 0;
        for( int i = rowCount; i < r; ++i ) {
            rowsAdded += addItem(i+1,emptyText);
        }
        rowCount += rowsAdded;
    } else
    if( r < rowCount ) {
        int rowsDeleted = 0;
        for( int i = r; i < rowCount; ++i ) {
            if( ListView_DeleteItem(hWnd,i) == FALSE ) {
                log::error("ListView_DeleteItem returned FALSE for hWnd ",hWnd);        
            } else {
                ++rowsDeleted;
            }
        }
        rowCount -= rowsDeleted;
    }
}

void TableViewImpl::deleteRow(int r)
{
    if( ListView_DeleteItem(hWnd,r) == FALSE ) {
        log::error("ListView_DeleteItem returned FALSE for hWnd ",hWnd);        
    } else {
        --rowCount;
    }
}

void TableViewImpl::setItem(int c, int r, ItemProperties itemProp)
{
    LVITEM item = propertiesToItem(c,r,itemProp);
    if( ListView_SetItem(hWnd,&item) == FALSE ) {
        log::error("ListView_SetItem returned FALSE for hWnd ",hWnd);
    }
    // ListView_SetItemText(hWnd,r,c,const_cast<wchar_t *>(itemProp.text.wstr.c_str()));
}

ItemProperties TableViewImpl::item(int c, int r) const
{
    ItemProperties result;
    result.text.wstr.resize(MAX_TABLEVIEW_ITEM_CHARS);
    LVITEM item;
    item.iItem = r;
    item.iSubItem = c;
    item.mask = LVIF_TEXT | LVIF_IMAGE;
    item.pszText = &result.text.wstr[0];
    item.cchTextMax = MAX_TABLEVIEW_ITEM_CHARS;
    if( ListView_GetItem(hWnd,&item) == FALSE ) {
        log::error("ListView_GetItem returned FALSE for hWnd ",hWnd);
    } else {
        result.imageIndex = item.iImage;
    }
    
    return result;
}

int TableViewImpl::newRow(ItemProperties itemProp)
{
    int r = rowCount;
    rowCount += addItem(r+1,itemProp);
    return r;
}

void TableViewImpl::setImageList(ImageList & imageList)
{
    ListView_SetImageList(hWnd, imageList.impl->getHiml(), LVSIL_SMALL);
    imageListImpl = imageList.impl;
}

void TableViewImpl::removeImageList()
{
    ListView_SetImageList(hWnd, NULL, LVSIL_SMALL);
    imageListImpl.reset();
}

optional<ImageList> TableViewImpl::getImageList()
{
    if( ! imageListImpl ) return optional<ImageList>();
            
    return ImageList(imageListImpl);
}

}