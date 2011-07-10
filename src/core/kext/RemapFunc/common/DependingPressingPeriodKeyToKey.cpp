#include "Config.hpp"
#include "EventWatcher.hpp"
#include "DependingPressingPeriodKeyToKey.hpp"

namespace org_pqrs_KeyRemap4MacBook {
  namespace RemapFunc {
    TimerWrapper DependingPressingPeriodKeyToKey::fire_timer_;
    DependingPressingPeriodKeyToKey* DependingPressingPeriodKeyToKey::target_ = NULL;

    DependingPressingPeriodKeyToKey::PeriodMS::PeriodMS(void) : mode_(Mode::NONE)
    {}

    void
    DependingPressingPeriodKeyToKey::PeriodMS::set(PeriodMS::Mode::Value newval)
    {
      mode_ = newval;
    }

    unsigned int
    DependingPressingPeriodKeyToKey::PeriodMS::get(PeriodMS::Type::Value type)
    {
      switch (mode_) {
        case Mode::HOLDING_KEY_TO_KEY:
          switch (type) {
            case Type::SHORT_PERIOD:             return Config::get_holdingkeytokey_wait();
            case Type::LONG_LONG_PERIOD:         return 0;
            case Type::PRESSING_TARGET_KEY_ONLY: return 0;
          }

        case Mode::KEY_OVERLAID_MODIFIER:
          switch (type) {
            case Type::SHORT_PERIOD:             return Config::get_keyoverlaidmodifier_initial_modifier_wait();
            case Type::LONG_LONG_PERIOD:         return 0;
            case Type::PRESSING_TARGET_KEY_ONLY: return Config::get_keyoverlaidmodifier_timeout();
          }

        case Mode::KEY_OVERLAID_MODIFIER_WITH_REPEAT:
          switch (type) {
            case Type::SHORT_PERIOD:             return Config::get_keyoverlaidmodifier_initial_modifier_wait();
            case Type::LONG_LONG_PERIOD:         return Config::get_keyoverlaidmodifier_initial_wait();
            case Type::PRESSING_TARGET_KEY_ONLY: return Config::get_keyoverlaidmodifier_timeout();
          }

        case Mode::NONE:
          IOLOG_ERROR("Invalid DependingPressingPeriodKeyToKey::PeriodMS::get\n");
          return 0;
      }

      return 0;
    }

    bool
    DependingPressingPeriodKeyToKey::PeriodMS::enabled(PeriodMS::Type::Value type)
    {
      switch (mode_) {
        case Mode::HOLDING_KEY_TO_KEY:
          switch (type) {
            case Type::SHORT_PERIOD:             return true;
            case Type::LONG_LONG_PERIOD:         return false;
            case Type::PRESSING_TARGET_KEY_ONLY: return false;
          }

        case Mode::KEY_OVERLAID_MODIFIER:
          switch (type) {
            case Type::SHORT_PERIOD:             return true;
            case Type::LONG_LONG_PERIOD:         return false;
            case Type::PRESSING_TARGET_KEY_ONLY: return true;
          }

        case Mode::KEY_OVERLAID_MODIFIER_WITH_REPEAT:
          switch (type) {
            case Type::SHORT_PERIOD:             return true;
            case Type::LONG_LONG_PERIOD:         return true;
            case Type::PRESSING_TARGET_KEY_ONLY: return true;
          }

        case Mode::NONE:
          IOLOG_ERROR("Invalid DependingPressingPeriodKeyToKey::PeriodMS::enabled\n");
          return false;
      }

      return false;
    }

    // ======================================================================
    void
    DependingPressingPeriodKeyToKey::static_initialize(IOWorkLoop& workloop)
    {
      fire_timer_.initialize(&workloop, NULL, DependingPressingPeriodKeyToKey::fire_timer_callback);
    }

    void
    DependingPressingPeriodKeyToKey::static_terminate(void)
    {
      fire_timer_.terminate();
    }

    DependingPressingPeriodKeyToKey::DependingPressingPeriodKeyToKey(void) :
      active_(false), periodtype_(PeriodType::NONE)
    {}

    DependingPressingPeriodKeyToKey::~DependingPressingPeriodKeyToKey(void)
    {
      if (target_ == this) {
        fire_timer_.cancelTimeout();
        target_ = NULL;
      }
    }

    void
    DependingPressingPeriodKeyToKey::add(KeyToKeyType::Value type, unsigned int datatype, unsigned int newval)
    {
      if (type == KeyToKeyType::END_) return;
      keytokey_[type].add(datatype, newval);
    }

    bool
    DependingPressingPeriodKeyToKey::remap(RemapParams& remapParams)
    {
      Flags flags = FlagStatus::makeFlags();

      bool result = keytokey_[KeyToKeyType::FROM].remap(remapParams);
      if (! result) {
        if (remapParams.params.ex_iskeydown) {
          // another key is pressed.
          dokeydown();
        }
        return false;
      }

      if (remapParams.params.ex_iskeydown) {
        target_ = this;
        active_ = true;
        periodtype_ = PeriodType::NONE;

        savedflags_ = flags;

        fire_timer_.setTimeoutMS(periodMS_.get(PeriodMS::Type::SHORT_PERIOD));

      } else {
        dokeydown();
        dokeyup();
      }
      return true;
    }

    void
    DependingPressingPeriodKeyToKey::dokeydown(void)
    {
      if (! active_) return;
      active_ = false;

      fire_timer_.cancelTimeout();

      switch (periodtype_) {
        case PeriodType::NONE:
        {
          periodtype_ = PeriodType::SHORT_PERIOD;

          FlagStatus::ScopedTemporaryFlagsChanger stfc(savedflags_);
          keytokey_[KeyToKeyType::SHORT_PERIOD].call_remap_with_VK_PSEUDO_KEY(EventType::DOWN);

          break;
        }

        case PeriodType::SHORT_PERIOD:
        case PeriodType::LONG_PERIOD:
        case PeriodType::LONG_LONG_PERIOD:
        case PeriodType::END_:
          // do nothing
          break;
      }
    }

    void
    DependingPressingPeriodKeyToKey::dokeyup(void)
    {
      switch (periodtype_) {
        case PeriodType::SHORT_PERIOD:
        {
          periodtype_ = PeriodType::NONE;
          keytokey_[KeyToKeyType::SHORT_PERIOD].call_remap_with_VK_PSEUDO_KEY(EventType::UP);
          break;
        }

        case PeriodType::LONG_PERIOD:
        {
          periodtype_ = PeriodType::NONE;
          keytokey_[KeyToKeyType::LONG_PERIOD].call_remap_with_VK_PSEUDO_KEY(EventType::UP);

          // ----------------------------------------
          // handle PRESSING_TARGET_KEY_ONLY
          if (periodMS_.enabled(PeriodMS::Type::PRESSING_TARGET_KEY_ONLY)) {
            if (! isAnyEventHappen_ &&
                ic_.getmillisec() < periodMS_.get(PeriodMS::Type::PRESSING_TARGET_KEY_ONLY)) {
              FlagStatus::ScopedTemporaryFlagsChanger stfc(savedflags_);
              keytokey_[KeyToKeyType::PRESSING_TARGET_KEY_ONLY].call_remap_with_VK_PSEUDO_KEY(EventType::DOWN);
              keytokey_[KeyToKeyType::PRESSING_TARGET_KEY_ONLY].call_remap_with_VK_PSEUDO_KEY(EventType::UP);
            }
          }

          break;
        }

        case PeriodType::LONG_LONG_PERIOD:
        {
          periodtype_ = PeriodType::NONE;
          keytokey_[KeyToKeyType::LONG_LONG_PERIOD].call_remap_with_VK_PSEUDO_KEY(EventType::UP);
          break;
        }

        case PeriodType::NONE:
        case PeriodType::END_:
          // do nothing
          break;
      }

      EventWatcher::unset(isAnyEventHappen_);
    }

    void
    DependingPressingPeriodKeyToKey::fire_timer_callback(OSObject* owner, IOTimerEventSource* sender)
    {
      if (! target_) return;

      switch (target_->periodtype_) {
        case PeriodType::NONE:
        {
          target_->periodtype_ = PeriodType::LONG_PERIOD;

          FlagStatus::ScopedTemporaryFlagsChanger stfc(target_->savedflags_);
          (target_->keytokey_[KeyToKeyType::LONG_PERIOD]).call_remap_with_VK_PSEUDO_KEY(EventType::DOWN);

          EventWatcher::set(target_->isAnyEventHappen_);
          (target_->ic_).begin();

          if (target_->periodMS_.enabled(PeriodMS::Type::LONG_LONG_PERIOD)) {
            fire_timer_.setTimeoutMS(target_->periodMS_.get(PeriodMS::Type::LONG_LONG_PERIOD));
          }

          break;
        }

        case PeriodType::LONG_PERIOD:
        {
          target_->periodtype_ = PeriodType::LONG_LONG_PERIOD;

          FlagStatus::ScopedTemporaryFlagsChanger stfc(target_->savedflags_);
          (target_->keytokey_[KeyToKeyType::LONG_LONG_PERIOD]).call_remap_with_VK_PSEUDO_KEY(EventType::DOWN);

          break;
        }

        case PeriodType::SHORT_PERIOD:
        case PeriodType::LONG_LONG_PERIOD:
        case PeriodType::END_:
          // do nothing
          break;
      }
    }
  }
}
