#ifndef KEYOVERLAIDMODIFIER_HPP
#define KEYOVERLAIDMODIFIER_HPP

#include "RemapFuncClasses.hpp"
#include "DependingPressingPeriodKeyToKey.hpp"

namespace org_pqrs_KeyRemap4MacBook {
  namespace RemapFunc {
    // for SandS like behavior remappings (remap_space2shift, remap_enter2optionL_commandSpace, ...)
    class KeyOverlaidModifier {
    public:
      KeyOverlaidModifier(void);
      ~KeyOverlaidModifier(void);

      bool remap(RemapParams& remapParams);

      // ----------------------------------------
      // [0]   => fromKey_
      // [1]   => toKey_
      // [2]   => toKeys_fire_[0]
      // [3]   => toKeys_fire_[1]
      // [4]   => toKeys_fire_[2]
      // [5]   => ...
      void add(unsigned int datatype, unsigned int newval);

      // utility
      void add(KeyCode newval) { add(BRIDGE_DATATYPE_KEYCODE, newval.get()); }
      void add(Flags newval)   { add(BRIDGE_DATATYPE_FLAGS,   newval.get()); }
      void add(Option newval)  { add(BRIDGE_DATATYPE_OPTION,  newval.get()); }

    private:
      size_t index_;

      DependingPressingPeriodKeyToKey dppkeytokey_;
    };
  }
}

#endif
