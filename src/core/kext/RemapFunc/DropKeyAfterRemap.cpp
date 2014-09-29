#include <IOKit/IOLib.h>

#include "DropKeyAfterRemap.hpp"
#include "IOLogWrapper.hpp"

namespace org_pqrs_Karabiner {
  namespace RemapFunc {
    void
    DropKeyAfterRemap::add(AddDataType datatype, AddValue newval)
    {
      switch (datatype) {
        case BRIDGE_DATATYPE_KEYCODE:
        {
          fromEvent_ = FromEvent(datatype, newval);
          break;
        }

        case BRIDGE_DATATYPE_MODIFIERFLAG:
        case BRIDGE_DATATYPE_MODIFIERFLAGS_END:
        {
          fromModifierFlags_.push_back(ModifierFlag(datatype, newval));
          break;
        }

        default:
          IOLOG_ERROR("DropKeyAfterRemap::add invalid datatype:%u\n", static_cast<unsigned int>(datatype));
          break;
      }
    }

    bool
    DropKeyAfterRemap::drop(const Params_KeyboardEventCallBack& params)
    {
      FlagStatus currentFlags(params.flags);
      if (! fromEvent_.changePressingState(params, currentFlags, fromModifierFlags_)) return false;
      return true;
    }
  }
}
