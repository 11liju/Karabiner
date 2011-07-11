#include "HoldingKeyToKey.hpp"

namespace org_pqrs_KeyRemap4MacBook {
  namespace RemapFunc {
    HoldingKeyToKey::HoldingKeyToKey(void) : index_(0), index_is_holding_(false), fromKeyFlag_(ModifierFlag::NONE)
    {
      dppkeytokey_.setPeriodMS(DependingPressingPeriodKeyToKey::PeriodMS::Mode::HOLDING_KEY_TO_KEY);
    }

    HoldingKeyToKey::~HoldingKeyToKey(void)
    {}

    void
    HoldingKeyToKey::add(unsigned int datatype, unsigned int newval)
    {
      switch (datatype) {
        case BRIDGE_DATATYPE_KEYCODE:
        {
          switch (index_) {
            case 0:
              dppkeytokey_.add(DependingPressingPeriodKeyToKey::KeyToKeyType::FROM,         datatype, newval);
              dppkeytokey_.add(DependingPressingPeriodKeyToKey::KeyToKeyType::SHORT_PERIOD, KeyCode::VK_PSEUDO_KEY);
              dppkeytokey_.add(DependingPressingPeriodKeyToKey::KeyToKeyType::LONG_PERIOD,  KeyCode::VK_PSEUDO_KEY);
              fromKeyFlag_ = KeyCode(newval).getModifierFlag();
              break;

            case 1:
              // pass-through (== no break)
              dppkeytokey_.add(DependingPressingPeriodKeyToKey::KeyToKeyType::FROM, KeyCode::VK_NONE);
            default:
              if (KeyCode::VK_NONE == newval) {
                index_is_holding_ = true;
              } else {
                if (index_is_holding_) {
                  dppkeytokey_.add(DependingPressingPeriodKeyToKey::KeyToKeyType::LONG_PERIOD,  KeyCode(newval));
                } else {
                  dppkeytokey_.add(DependingPressingPeriodKeyToKey::KeyToKeyType::SHORT_PERIOD, KeyCode(newval));
                }
              }
              break;
          }
          ++index_;

          break;
        }

        case BRIDGE_DATATYPE_FLAGS:
        case BRIDGE_DATATYPE_OPTION:
        {
          switch (index_) {
            case 0:
              IOLOG_ERROR("Invalid HoldingKeyToKey::add\n");
              break;

            case 1:
              dppkeytokey_.add(DependingPressingPeriodKeyToKey::KeyToKeyType::FROM,         datatype, newval);

              switch (datatype) {
                case BRIDGE_DATATYPE_FLAGS:
                {
                  Flags flags(newval);
                  if (fromKeyFlag_ != ModifierFlag::NONE) {
                    flags.remove(fromKeyFlag_);
                  }
                  dppkeytokey_.add(DependingPressingPeriodKeyToKey::KeyToKeyType::SHORT_PERIOD, flags);
                  dppkeytokey_.add(DependingPressingPeriodKeyToKey::KeyToKeyType::LONG_PERIOD,  flags);
                  break;
                }
                case BRIDGE_DATATYPE_OPTION:
                {
                  dppkeytokey_.add(DependingPressingPeriodKeyToKey::KeyToKeyType::SHORT_PERIOD, datatype, newval);
                  dppkeytokey_.add(DependingPressingPeriodKeyToKey::KeyToKeyType::LONG_PERIOD,  datatype, newval);
                  break;
                }
              }
              break;

            default:
              if (index_is_holding_) {
                dppkeytokey_.add(DependingPressingPeriodKeyToKey::KeyToKeyType::LONG_PERIOD, datatype, newval);
              } else {
                dppkeytokey_.add(DependingPressingPeriodKeyToKey::KeyToKeyType::SHORT_PERIOD, datatype, newval);
              }
              break;
          }
          break;
        }

        default:
          IOLOG_ERROR("HoldingKeyToKey::add invalid datatype:%d\n", datatype);
          break;
      }
    }

    bool
    HoldingKeyToKey::remap(RemapParams& remapParams)
    {
      return dppkeytokey_.remap(remapParams);
    }
  }
}
