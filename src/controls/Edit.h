#ifndef EDIT_CONTROL_H_jfffffffff9329jdsfn
#define EDIT_CONTROL_H_jfffffffff9329jdsfn

#include "../Control.h"

namespace tk {

class Edit: public Control {
public:
    Edit() = default;
    Edit(Window & parent, ControlParams const & params = ControlParams());
    Edit clone() const;
    
	void setReadOnly(bool readOnly);
    void setTextAlign(HTextAlign alignment);
    
    bool readOnly() const;
    
    using Control::onKey;
    using Control::onKeypress;
};

}


#endif
