#include "WindowImplX11.h"
#include "../log.h"
#include "ResourceManager.h"
#include "../CallbackUtils.h"
#include "ConvertersX11.h"
#include <X11/Xatom.h>
#include <memory>

namespace tk {

template<typename T> using WindowCallbackMap = CallbackMap<WindowImpl *, T>;

WindowCallbackMap<AllowOperation> cbClose;
WindowCallbackMap<ProcessResize> cbResize;
WindowCallbackMap<HandleEvent> cbUserEvent;

Atom WM_DELETE_WINDOW = XInternAtom(resourceManager.dpy, "WM_DELETE_WINDOW", False);

WindowImpl::WindowImpl(WindowParams const & params)
{
	int x,y;
	WDims dims;
	if( params.isDefaultPos() ) {
		x = y = 0;
	} else {
		x = params.start_.x;
		y = params.start_.y;
	}
	if( params.isDefaultDims() ) {
		dims.width = dims.height = 200; // TODO find a correct default
	} else {
		dims = params.dims_;
	}

	int borderColor = BlackPixel(resourceManager.dpy, DefaultScreen(resourceManager.dpy));
	int backgroundColor = borderColor;
	int borderWidth = 0;
	windowId = XCreateSimpleWindow(
			resourceManager.dpy,
			DefaultRootWindow(resourceManager.dpy),  // parent :-)
			x, y, dims.width, dims.height,
			borderWidth, borderColor, backgroundColor);

	XSetWMProtocols(resourceManager.dpy, windowId, &WM_DELETE_WINDOW, 1);

	XSelectInput(resourceManager.dpy, windowId,
			KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
			EnterWindowMask | LeaveWindowMask | PointerMotionMask |
			ExposureMask | StructureNotifyMask | PropertyChangeMask
			);

	setText(params.text_);
	XSetIconName(resourceManager.dpy, windowId, params.text_.str.c_str());

	if( params.initialState & ws_visible ) {
		show();
	}
	if( params.initialState & ws_maximized ) {
		maximize(true);
	}
	if( params.initialState & ws_minimized ) {
		XIconifyWindow(resourceManager.dpy, windowId, DefaultScreen(resourceManager.dpy));
	}

	windowCanvas = CanvasX11(windowId);
}

WindowImpl::~WindowImpl()
{
    removeFromCb(this,cbClose);
    removeFromCb(this,cbResize);
    removeFromCb(this,cbUserEvent);

	XDestroyWindow(resourceManager.dpy, windowId);
}

int WindowImpl::state() const
{
	int result = 0;
	if( visible() ) result |= ws_visible;

	Atom actual_type;
	int actual_format;
	unsigned long num_items, bytes_after;
	Atom * atoms = NULL;
	XGetWindowProperty(resourceManager.dpy, windowId, XInternAtom(resourceManager.dpy, "_NET_WM_STATE", False),
			0, 1024, False, XA_ATOM, &actual_type, &actual_format, &num_items, &bytes_after, (unsigned char**)&atoms);
    for( unsigned long i=0; i<num_items; ++i ) {
        if( atoms[i] == XInternAtom(resourceManager.dpy, "_NET_WM_STATE_HIDDEN", False) ) {
        	result |= ws_minimized;
        } else
		if( atoms[i] == XInternAtom(resourceManager.dpy, "_NET_WM_STATE_MAXIMIZED_HORZ", False) ) {
			result |= ws_maximized;
		}
    }
	XFree(atoms);

	return result;
}

void WindowImpl::setBorders(bool value)
{
	XSetWindowAttributes attr;
	attr.override_redirect = value ? False : True;
	XChangeWindowAttributes(resourceManager.dpy, windowId, CWOverrideRedirect, &attr);
}

void WindowImpl::registerControl(std::shared_ptr<NativeControlImpl> control)
{
	controls[control->windowId] = control;
	// do I need to register on resourceManager too?
}

void WindowImpl::unregisterControl(std::shared_ptr<NativeControlImpl> control)
{
    if( controls.erase(control->windowId) > 0 ) {
    	// do I need to unregister on resourceManager too?
        XDestroyWindow(resourceManager.dpy, control->windowId);
    }
}

die::NativeString WindowImpl::getText() const
{
	char * windowName;
	XFetchName(resourceManager.dpy, windowId, &windowName);
	if( windowName == nullptr ) return {};

	auto windowNamePtr = std::unique_ptr<void,int (*)(void*)>(static_cast<void *>(windowName),&XFree);
	die::NativeString result;
	result.str.assign(windowName);
	return result;
}

void WindowImpl::setText(die::NativeString const & text)
{
	XStoreName(resourceManager.dpy, windowId, text.str.c_str());
}

// callbacks & messages

AllowOperation WindowImpl::onClose(AllowOperation callback)
{
    return setCallback(this,cbClose,callback);
}

ProcessResize WindowImpl::onResize(ProcessResize callback)
{
    return setCallback(this,cbResize,callback);
}

HandleEvent WindowImpl::onUserEvent(HandleEvent callback)
{
    return setCallback(this,cbUserEvent,callback);
}

void WindowImpl::processMessage(XEvent & e)
{
	//log::info("window ",e.xany.window," got event ",xEventToStr(e.type));

	switch(e.type) {
	case ClientMessage: {
		auto atom = Atom(e.xclient.data.l[0]);
		if( atom == WM_DELETE_WINDOW ) {
            auto canClose = findExec(this,cbClose);
            if( canClose && *canClose == false ) return;

			hide();
		} else {
			executeCallback(this,cbUserEvent,UserEvent{atom,&e.xclient});
		}
	} break;

	case ConfigureNotify: {
		auto & data = e.xconfigure;
		WDims newDims(data.width,data.height);
		if( newDims != cachedDims ) {
			auto resNewDims = findExec(this,cbResize,newDims);
			if( resNewDims && *resNewDims != newDims ) {
				cachedDims = WDims();   // invalidate cache to make the event to trigger again. because WMs
				setDims(*resNewDims);
				//XResizeWindow(data.display,data.window,resNewDims->width,resNewDims->height);
			} else {
				cachedDims = newDims;
			}
		}
	} break;

	default:
		NativeControlImpl::processMessage(e);

	}
}

void WindowImpl::maximize(bool yes)
{
	XEvent xev{};
	xev.type = ClientMessage;
	xev.xclient.window = windowId;
	xev.xclient.message_type = XInternAtom(resourceManager.dpy, "_NET_WM_STATE",
			False);
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = yes ? 1: 0;
	xev.xclient.data.l[1] = XInternAtom(resourceManager.dpy,
			"_NET_WM_STATE_MAXIMIZED_HORZ", False);
	xev.xclient.data.l[2] = XInternAtom(resourceManager.dpy,
			"_NET_WM_STATE_MAXIMIZED_VERT", False);
	XSendEvent(resourceManager.dpy, DefaultRootWindow(resourceManager.dpy),
				False, SubstructureNotifyMask, &xev);
}

Canvas & WindowImpl::canvas()
{
	return windowCanvas;
}

}