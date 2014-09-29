#ifndef FROMEVENT_HPP
#define FROMEVENT_HPP

#include "CallbackWrapper.hpp"
#include "FlagStatus.hpp"
#include "IOLogWrapper.hpp"
#include "ParamsUnion.hpp"
#include "Vector.hpp"
#include "bridge.h"

namespace org_pqrs_Karabiner {
  class FromEvent {
  public:
    class Type {
    public:
      enum Value {
        NONE,
        KEY,
        CONSUMER_KEY, // Mute, VolumeIncrement, VolumeDecrement, etcetc.
        POINTING_BUTTON,
      };
    };

    FromEvent(void)                       : isPressing_(false), type_(Type::NONE)                          {}
    explicit FromEvent(KeyCode v)         : isPressing_(false), type_(Type::KEY),             key_(v)      {}
    explicit FromEvent(ConsumerKeyCode v) : isPressing_(false), type_(Type::CONSUMER_KEY),    consumer_(v) {}
    explicit FromEvent(PointingButton v)  : isPressing_(false), type_(Type::POINTING_BUTTON), button_(v)   {}

    explicit FromEvent(const Params_Base& paramsBase) : isPressing_(false) {
      type_ = Type::NONE;

      {
        auto p = paramsBase.get_Params_KeyboardEventCallBack();
        if (p) {
          type_ = Type::KEY;
          key_ = p->key;
          return;
        }
      }
      {
        auto p = paramsBase.get_Params_KeyboardSpecialEventCallback();
        if (p) {
          type_ = Type::CONSUMER_KEY;
          consumer_ = p->key;
          return;
        }
      }
      {
        auto p = paramsBase.get_Params_RelativePointerEventCallback();
        if (p) {
          type_ = Type::POINTING_BUTTON;
          button_ = p->ex_button;
          return;
        }
      }
    }

    FromEvent(AddDataType datatype, AddValue v) : isPressing_(false) {
      switch (datatype) {
        case BRIDGE_DATATYPE_KEYCODE:         type_ = Type::KEY;             key_      = KeyCode(v);         break;
        case BRIDGE_DATATYPE_CONSUMERKEYCODE: type_ = Type::CONSUMER_KEY;    consumer_ = ConsumerKeyCode(v); break;
        case BRIDGE_DATATYPE_POINTINGBUTTON:  type_ = Type::POINTING_BUTTON; button_   = PointingButton(v);  break;
        default:
          IOLOG_ERROR("Unknown datatype: %u\n", static_cast<unsigned int>(datatype));
          type_ = Type::NONE;
          break;
      }
    }

    Type::Value getType(void) const { return type_; }

    // Return whether pressing state is changed.
    bool changePressingState(const Params_Base& paramsBase,
                             const FlagStatus& currentFlags,
                             const Vector_ModifierFlag& fromFlags);

    bool changePressingState(const ParamsUnion& paramsUnion,
                             const FlagStatus& currentFlags,
                             const Vector_ModifierFlag& fromFlags)
    {
      return changePressingState(paramsUnion.get_Params_Base(), currentFlags, fromFlags);
    }

    bool isPressing(void) const { return isPressing_; }
    void unsetPressingState(void) { isPressing_ = false; }

    // Primitive functions:
    // These functions do not treat Flags.
    // Use changePressingState in general.
    bool isTargetDownEvent(const Params_Base& paramsBase) const;
    bool isTargetUpEvent(const Params_Base& paramsBase) const;

    bool isTargetDownEvent(const ParamsUnion& paramsUnion) const
    {
      return isTargetDownEvent(paramsUnion.get_Params_Base());
    }
    bool isTargetUpEvent(const ParamsUnion& paramsUnion) const
    {
      return isTargetUpEvent(paramsUnion.get_Params_Base());
    }

    // Get ModifierFlag from KeyCode.
    ModifierFlag getModifierFlag(void) const {
      if (type_ != Type::KEY) return ModifierFlag::ZERO;
      return key_.getModifierFlag();
    }
    PointingButton getPointingButton(void) const {
      if (type_ != Type::POINTING_BUTTON) return PointingButton::NONE;
      return button_;
    }

  private:
    bool isTargetEvent(bool& isDown, const Params_Base& paramsBase) const;

    bool isPressing_;

    // Do not store Flags in FromEvent because SimultaneousKeyPresses uses multiple FromEvents.

    Type::Value type_;
    KeyCode key_;
    ConsumerKeyCode consumer_;
    PointingButton button_;
  };

  DECLARE_VECTOR(FromEvent);
}

#endif
