#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#include <windows.h>

#include "MenuWin32.h"
#include <memory>
#include <unordered_map>
#include "../../log.h"
#include "../ConvertersWin32.h"
#include "../WindowImplWin32.h"

namespace tk {
    
unsigned const menuEndPos = unsigned(-2);
std::shared_ptr<WindowImpl> nullWindow;

std::unordered_map<HMENU,std::weak_ptr<WindowImpl>> attachedMenus;

UINT lastId = 0;
std::unordered_map<UINT,HandleOperation> clickMap;

void detachExistingMenu(WindowImpl::Components & components)
{
    for( auto it = components.begin(); it != components.end(); ++it ) {
        auto menuPtr = std::dynamic_pointer_cast<MenuImpl>(*it);
        if( menuPtr ) {
            components.erase(it);
            return;
        }
    }
}

std::shared_ptr<WindowImpl> findWindow(HMENU hMenu)
{
    auto it = attachedMenus.find(hMenu);
    if( it == attachedMenus.end() ) return nullWindow;
    
    auto window = it->second.lock();
    if( ! window ) {
        // defensive code: attached window is expired. remove logical attachment and bail out
        attachedMenus.erase(it);
        return nullWindow;
    }
    
    return window;
}

void removeCallbacks(HMENU hMenu);

void removeCallbacks(HMENU hMenu, unsigned pos)
{
    MENUITEMINFOW mii;
    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_ID | MIIM_SUBMENU;
    if( GetMenuItemInfoW(hMenu,pos,TRUE,&mii) == 0 ) {
        log::error("GetMenuItemInfoW returned zero for hMenu ",hMenu);
    } else {
        clickMap.erase(mii.wID);

        if( mii.hSubMenu != NULL ) {
            removeCallbacks(mii.hSubMenu);
        }
    }    
}

void removeCallbacks(HMENU hMenu)
{
    auto count = GetMenuItemCount(hMenu);
    for( int pos = 0; pos < count; ++pos ) {
        removeCallbacks(hMenu,pos);
    }
}

void addNewItem(HMENU hMenu, MenuItemProperties const & properties)
{
    ++lastId;
    // add the item on the menu 
    if( properties.image.type == it_none ) {
        if( AppendMenuW(hMenu,MF_STRING,lastId,properties.text.wstr.c_str()) == 0 ) {
            log::error("AppendMenuW returned zero for hMenu ",hMenu);
        }
    } else {
        scoped::Bitmap bitmap = ihToBitmap(properties.image);
        if( AppendMenuW(hMenu,MF_BITMAP,lastId,reinterpret_cast<LPCWSTR>(bitmap.get())) == 0 ) {
            log::error("AppendMenuW returned zero for hMenu ",hMenu);
        } else {
            bitmap.release(); // avoids bitmap destruction
        }
    }    
}

unsigned getCount(HMENU subMenu)
{
    int result = GetMenuItemCount(subMenu);
    if( result == -1 ) {
        log::error("GetMenuItemCount returned -1 for subMenu ",subMenu);
        return 0;
    }
    
    return unsigned(result);
}

// MenuImpl

MenuImpl::MenuImpl():
    hMenu(CreateMenu())
{
}

MenuImpl::~MenuImpl()
{
    detach();
    removeCallbacks(hMenu);
    DestroyMenu(hMenu);
    // TODO investigate if we need to manually destroy submenus and bitmaps
}

MenuItem MenuImpl::root()
{
    return MenuItem(*this);
}

optional<LRESULT> MenuImpl::processNotification(UINT message, UINT notification, UINT id, LPARAM lParam)
{
    auto it = clickMap.find(id);
    if( it != clickMap.end() ) {
        it->second();
    }
    return optional<LRESULT>();
}

void MenuImpl::attach(std::shared_ptr<WindowImpl> window)
{
    if( SetMenu(window->hWnd,hMenu) == 0 ) {
        log::error("SetMenu returned zero for hWnd ",window->hWnd," with hMenu ",hMenu);
        return;
    }
    
    detachExistingMenu(window->components);
    
    window->components.insert(std::static_pointer_cast<MenuImpl>(shared_from_this()));
    
    attachedMenus[hMenu] = std::weak_ptr<WindowImpl>(window);
}

void MenuImpl::detach()
{
    auto window = findAndUnregister();
    if( window ) {
        // remove from the window registered components as well
        window->components.erase(std::static_pointer_cast<MenuImpl>(shared_from_this()));    
    }
}

WindowRef MenuImpl::getAttachedWindow()
{
    return WindowRef(findWindow(hMenu));
}

void MenuImpl::unregister()
{
    findAndUnregister();
}

std::shared_ptr<WindowImpl> MenuImpl::findAndUnregister()
{
    auto window = findWindow(hMenu);
    if( window ) {
        if( SetMenu(window->hWnd,0) == 0 ) {
            log::error("SetMenu returned zero for hWnd ",window->hWnd," while removing a menu");
        }
    }

    attachedMenus.erase(hMenu);
    
    return window;
}

// MenuItemImpl

MenuItemImpl::MenuItemImpl(HMENU hMenu, unsigned pos):
    hMenu(hMenu),
    pos(pos)
{
}

MenuItemProperties MenuItemImpl::getProperties() const
{
    MenuItemProperties result;
    
    MENUITEMINFOW mii;
    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_STATE | MIIM_TYPE;
    mii.dwTypeData = 0;
    if( GetMenuItemInfoW(hMenu,pos,TRUE,&mii) == 0 ) {
        log::error("GetMenuItemInfoW returned zero for hMenu ",hMenu);
    } else {
        result.checked = (mii.fState & MFS_CHECKED) == MFS_CHECKED;
        result.enabled = (mii.fState & MFS_DISABLED) == 0;
        if( mii.fType == MFT_STRING ) {
            ++mii.cch;
            result.text.wstr.resize(mii.cch);
            mii.dwTypeData = &result.text.wstr[0];
            mii.fMask = MIIM_STRING;
            GetMenuItemInfoW(hMenu,pos,TRUE,&mii);
        } else
        if( mii.fType == MFT_BITMAP ) {
            NativeBitmap bitmap = extractBitmap(mii.hbmpItem);
            result.imageBuffer.swap(bitmap.imageBuffer);
            result.image = ImageRef::native(&bitmap.info,result.imageBuffer.data());
        }
    }
    
    return result;
}

HMENU MenuItemImpl::parentHMenu() const
{
    MENUINFO mi;
    mi.cbSize = sizeof(MENUINFO);
    mi.fMask = MIM_MENUDATA;   
    if( GetMenuInfo(hMenu,&mi) == 0 ) {
        log::error("GetMenuInfo returned zero for hMenu ",hMenu);
        return 0;
    }
    return reinterpret_cast<HMENU>(mi.dwMenuData);
    // TODO try GetParent() as well
}

void MenuItemImpl::updateIfTopLevel()
{
    if( parentHMenu() == 0 ) {
        auto window = findWindow(hMenu);
        if( ! window ) return;

        DrawMenuBar(window->hWnd);
    }
}

void MenuItemImpl::setText(die::NativeString const & text)
{
    MENUITEMINFOW mii;
    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_STRING;
    mii.dwTypeData = const_cast<wchar_t *>(text.wstr.c_str());
    if( SetMenuItemInfoW(hMenu,pos,TRUE,&mii) == 0 ) {
        log::error("SetMenuItemInfoW returned zero for hMenu ",hMenu," and pos ",pos," while changing text");
    }    
    updateIfTopLevel();
}

void MenuItemImpl::setImage(ImageRef ih)
{
    scoped::Bitmap bitmap = ihToBitmap(ih);
    MENUITEMINFOW mii;
    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_BITMAP;
    mii.hbmpItem = bitmap.get();
    if( SetMenuItemInfoW(hMenu,pos,TRUE,&mii) == 0 ) {
        log::error("SetMenuItemInfoW returned zero for hMenu ",hMenu," and pos ",pos," while changing bitmap");
    } else {
        bitmap.release(); // to avoid bitmap destruction
    }
    updateIfTopLevel();    
}

UINT MenuItemImpl::getItemId() const
{
    UINT result = GetMenuItemID(hMenu,pos);
    if( result == UINT(-1) ) {
        log::error("GetMenuItemID returned -1 for hMenu ",hMenu," and pos ",pos);
    }
    
    return result;
}

HandleOperation MenuItemImpl::onClick(HandleOperation callback)
{
    auto id = getItemId();
    auto it = clickMap.find(id);
    if( it == clickMap.end() ) {
        if( callback != nullptr ) {
            clickMap.insert(it,std::make_pair(id,callback));
        }
        return nullptr;
    }
    
    auto result = it->second;
    if( callback != nullptr ) {
        it->second = callback;
    } else {
        clickMap.erase(it);
    }
    return result;
}

MenuItemIterator MenuItemImpl::begin()
{
    HMENU subMenu = GetSubMenu(hMenu,pos);
    return MenuItemIterator(new MenuItemIteratorImpl(subMenu,subMenu == NULL ? menuEndPos : 0));
}

MenuItemIterator MenuItemImpl::end()
{
    HMENU subMenu = GetSubMenu(hMenu,pos);
    return MenuItemIterator(new MenuItemIteratorImpl(subMenu,menuEndPos));
}

bool MenuItemImpl::empty() const
{
    return count() == 0;
}

unsigned MenuItemImpl::total() const
{
    return getCount(hMenu);
}

unsigned MenuItemImpl::count() const
{
    HMENU subMenu = GetSubMenu(hMenu,pos);
    if( subMenu == NULL ) return 0;
    
    return getCount(subMenu);
}
    
MenuItemIterator MenuItemImpl::addItem(MenuItemProperties const & properties)
{
    HMENU subMenu = GetSubMenu(hMenu,pos);
    int nItems;
    // if submenu is still not created
    if( subMenu == NULL ) {
        // creates a submenu with dwMenuData = hMenu (the parent)
        subMenu = CreatePopupMenu();
        MENUINFO mi;
        mi.cbSize = sizeof(MENUINFO);
        mi.fMask = MIM_MENUDATA;
        mi.dwMenuData = reinterpret_cast<ULONG_PTR>(hMenu);
        if( SetMenuInfo(subMenu,&mi) == 0 ) {
            log::error("SetMenuInfo returned zero for hMenu ",subMenu);
            return MenuItemIterator();
        }
        // sets the submenu handle on the own item
        MENUITEMINFOW mii;
        mii.cbSize = sizeof(MENUITEMINFOW);
        mii.fMask = MIIM_SUBMENU;
        mii.hSubMenu = subMenu;
        if( SetMenuItemInfoW(hMenu,pos,TRUE,&mii) == 0 ) {
            log::error("SetMenuItemInfoW returned zero for hMenu ",hMenu," and pos ",pos," while adding submenu ",subMenu);
            return MenuItemIterator();
        }
        nItems = 0;
    } else {
        nItems = GetMenuItemCount(subMenu);
    }

    addNewItem(subMenu,properties);
    
    return MenuItemIterator(new MenuItemIteratorImpl(subMenu,nItems));
}

void MenuItemImpl::eraseItem(MenuItemIterator it)
{
    it.iteratorImpl->deleteItem();
}

void MenuItemImpl::advance() const
{
    ++pos;
    if( pos >= total() ) {
        pos = menuEndPos;
    }
}

void MenuItemImpl::deleteItem()
{
    removeCallbacks(hMenu,pos);
    
    if( DeleteMenu(hMenu,pos,MF_BYPOSITION) == 0 ) {
        log::error("DeleteMenu returned zero for hMenu ",hMenu);
    }
    
    updateIfTopLevel();
}

bool MenuItemImpl::operator==(MenuItemImpl const & item) const
{
    return hMenu == item.hMenu && pos == item.pos;
}

// MenuItemImplRoot

MenuItemImplRoot::MenuItemImplRoot(HMENU hMenu):
    MenuItemImpl(hMenu,menuEndPos)
{
}
    
MenuItemProperties MenuItemImplRoot::getProperties() const
{
    return MenuItemProperties();
}

void MenuItemImplRoot::setText(die::NativeString const & text)
{
}

void MenuItemImplRoot::setImage(ImageRef ih)
{
}

HandleOperation MenuItemImplRoot::onClick(HandleOperation callback)
{
    return nullptr;
}

MenuItemIterator MenuItemImplRoot::begin()
{
    return MenuItemIterator(new MenuItemIteratorImpl(hMenu,0));
}

MenuItemIterator MenuItemImplRoot::end()
{
    return MenuItemIterator(new MenuItemIteratorImpl(hMenu,menuEndPos));
}

unsigned MenuItemImplRoot::count() const
{
    return getCount(hMenu);
}

MenuItemIterator MenuItemImplRoot::addItem(MenuItemProperties const & properties)
{
    int nItems = count();
    
    addNewItem(hMenu,properties);
    
    updateTopLevel();    
    
    return MenuItemIterator(new MenuItemIteratorImpl(hMenu,nItems));
}

void MenuItemImplRoot::updateTopLevel()
{
    auto window = findWindow(hMenu);
    if( ! window ) return;

    DrawMenuBar(window->hWnd);
}

// MenuItemIteratorImpl

MenuItemIteratorImpl::MenuItemIteratorImpl(HMENU hMenu, unsigned pos):
    item(std::make_shared<MenuItemImpl>(hMenu,pos))
{
}

MenuItem & MenuItemIteratorImpl::currentItem()
{
    return item;
}

MenuItem const & MenuItemIteratorImpl::currentItem() const
{
    return item;
}

void MenuItemIteratorImpl::advance() const
{
    item.itemImpl->advance();
}

void MenuItemIteratorImpl::deleteItem()
{
    item.itemImpl->deleteItem();
}

bool MenuItemIteratorImpl::operator==(MenuItemIteratorImpl const & it) const
{
    return *item.itemImpl == *it.item.itemImpl;
}

}
