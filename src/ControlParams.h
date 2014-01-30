#ifndef CONTROL_PARAMS_H_gfu43983tgfff45kh03hjg312
#define CONTROL_PARAMS_H_gfu43983tgfff45kh03hjg312

#include "WindowObjects.h"
#include <NativeString.h>
#include "util/optional.h"

namespace tk {

class ControlParams {
public:
	die::NativeString text_;
	Point start_;
	WDims dims_;
    bool visible_;
	Scrollbar scrollbar_;
	bool autosize_;
    optional<RGBColor> backgroundColor_;
    Cursor cursor_;

	ControlParams():
        visible_(true),
        scrollbar_(Scrollbar::none),
        autosize_(false),
        cursor_(Cursor::defaultCursor)
    {
    }

	ControlParams & text(die::NativeString const & value) { text_ = value; return *this; }
	ControlParams & start(Point value) { start_ = value; return *this; }
	ControlParams & start(int x, int y) { start_ = Point(x,y); return *this; }
	ControlParams & dims(WDims value) { dims_ = value; return *this; }
	ControlParams & dims(int width, int height) { dims_ = WDims(width,height); return *this; }
	ControlParams & visible(bool value = true) { visible_ = value; return *this; }
	ControlParams & scrollbar(Scrollbar value) { scrollbar_ = value; return *this; }
	ControlParams & autosize(bool value = true) { autosize_ = value; return *this; }
	ControlParams & backgroundColor(RGBColor value) { backgroundColor_ = value; return *this; }
	ControlParams & cursor(Cursor value) { cursor_ = value; return *this; }
};

}

#endif
