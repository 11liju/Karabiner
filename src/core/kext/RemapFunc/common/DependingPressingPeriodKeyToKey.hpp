#ifndef DEPENDINGPRESSINGPERIODKEYTOKEY_HPP
#define DEPENDINGPRESSINGPERIODKEYTOKEY_HPP

#include "RemapFuncClasses.hpp"
#include "FromKeyChecker.hpp"
#include "KeyToKey.hpp"
#include "TimerWrapper.hpp"

namespace org_pqrs_KeyRemap4MacBook {
  namespace RemapFunc {
    // This class is an implementation of HoldingKeyToKey and KeyOverlaidModifier.
    //
    // DependingPressingPeriodKeyToKey modifies key event depending the pressing period.
    //
    class DependingPressingPeriodKeyToKey {
    public:
      static void static_initialize(IOWorkLoop& workloop);
      static void static_terminate(void);

      DependingPressingPeriodKeyToKey(void);
      ~DependingPressingPeriodKeyToKey(void);

      bool remap(RemapParams& remapParams);

      // ----------------------------------------
      // [0]   => fromKey_
      // [1]   => toKeys_normal_[0]
      // [2]   => toKeys_normal_[1]
      // [3]   => ...
      // [n]   => KeyCode::VK_NONE
      // [n+1] => toKeys_holding_[0]
      // [n+2] => toKeys_holding_[1]
      // [n+3] => ...
      void add(unsigned int datatype, unsigned int newval);

    private:
      enum KeyDownType {
        KEYDOWNTYPE_NONE,
        KEYDOWNTYPE_NORMAL,
        KEYDOWNTYPE_HOLDING,
      };

      void dokeydown(void);
      void dokeyup(void);
      static void fireholding_timer_callback(OSObject* owner, IOTimerEventSource* sender);

      static TimerWrapper fireholding_timer_;
      static DependingPressingPeriodKeyToKey* target_;

      size_t index_;
      bool index_is_holding_;
      Flags savedflags_;

      bool active_;
      KeyDownType keydowntype_;

      KeyToKey keytokey_drop_;
      KeyToKey keytokey_normal_;
      KeyToKey keytokey_holding_;
    };
  }
}

#endif
