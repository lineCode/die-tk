#include "../WindowImplWin32.h"

namespace {

inline tk::CONTROL_IMPL & ci(std::shared_ptr<tk::NativeControlImpl> & impl)
{
    return static_cast<tk::CONTROL_IMPL &>(*impl);
}

inline tk::CONTROL_IMPL const & ci(std::shared_ptr<tk::NativeControlImpl> const & impl)
{
    return static_cast<tk::CONTROL_IMPL const &>(*impl);
}

template<typename C>
void createAndRegister(std::shared_ptr<tk::NativeControlImpl> & impl, tk::Window & parent, tk::ControlParams const & params)
{
    impl = std::static_pointer_cast<tk::NativeControlImpl>(std::make_shared<C>(parent,params));
    parent.getImpl().registerControl(impl);
}

}

#define IMPL ci(impl)

#define CONSTRUCTOR_IMPL(CLASS) \
CLASS::CLASS(Window & parent, ControlParams const & params) \
{ \
    createAndRegister<CONTROL_IMPL>(impl,parent,params); \
}
