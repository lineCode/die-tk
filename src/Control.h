#ifndef CONTROL_H_gfh430hm4bg73bg93gllhr52bzq
#define CONTROL_H_gfh430hm4bg73bg93gllhr52bzq

#include <memory>
#include "WindowObjects.h"
#include "Callbacks.h"
#include "ControlParams.h"

namespace tk {

class Window;
class WindowRef;
class Canvas;
class NativeControlImpl;

// base class for all widgets, including windows

class Control {
    friend class Window;
    friend class ControlRef;
public:
    Control() = default;
    Control(Control const &) = delete;
    Control & operator=(Control const &) = delete;
    Control(Control &&) = default;
    Control & operator=(Control &&) = default;
    virtual ~Control() = 0;
    
    explicit operator bool() const;
    
	int x() const;
	int y() const;
	Point pos() const;
	int width() const;
	int height() const;
	WDims dims() const;
	Rect rect() const;

	Control & setPos(Point pos);
	Control & setDims(WDims dims);
	Control & setRect(Rect rect);

	die::NativeString getText() const;
	Control & setText(die::NativeString const & text);
    
	void bringToFront();
	void sendToBack();
    
	bool enabled() const;
	void enable();
	void disable();
    void setEnabled(bool enabled);

	bool visible() const;
	void show();
	void hide();
    void setVisible(bool visible);    

	Canvas & canvas();
	void repaint();

	void setCursor(Cursor cursor);
	void setBackground(RGBColor const & color);

	void clear(RGBColor const & color = RGBColor());

	Point screenToClient(Point const & point) const;
    
    WindowRef getParent() const;
    
	HandleMouseEvent onMouse(HandleMouseEvent callback);     
protected:
    std::shared_ptr<NativeControlImpl> impl;

    // most used callbacks
	ProcessKeyEvent onKey(ProcessKeyEvent callback);       // edit, memo
	ProcessKeypress onKeypress(ProcessKeypress callback);  // edit, memo
    // seldom used callbacks
	HandlePaint onPaint(HandlePaint callback);       // window and paintbox
};

}

#endif

