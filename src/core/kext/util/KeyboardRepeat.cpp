#include "CommonData.hpp"
#include "EventOutputQueue.hpp"
#include "KeyboardRepeat.hpp"
#include "ListHookedKeyboard.hpp"
#include "remap.hpp"

namespace org_pqrs_KeyRemap4MacBook {
  List* KeyboardRepeat::queue_ = NULL;
  TimerWrapper KeyboardRepeat::fire_timer_;
  int KeyboardRepeat::id_ = 0;

  void
  KeyboardRepeat::initialize(IOWorkLoop& workloop)
  {
    queue_ = new List();
    fire_timer_.initialize(&workloop, NULL, KeyboardRepeat::fire_timer_callback);
  }

  void
  KeyboardRepeat::terminate(void)
  {
    fire_timer_.terminate();

    if (queue_) {
      delete queue_;
    }
  }

  void
  KeyboardRepeat::cancel(void)
  {
    fire_timer_.cancelTimeout();
    if (queue_) {
      queue_->clear();
    }
  }

  void
  KeyboardRepeat::primitive_add(EventType eventType,
                                Flags flags,
                                KeyCode key,
                                KeyboardType keyboardType,
                                Item::Type type)
  {
    if (! queue_) return;
    if (key == KeyCode::VK_NONE) return;

    // ------------------------------------------------------------
    Params_KeyboardEventCallBack::auto_ptr ptr(Params_KeyboardEventCallBack::alloc(eventType,
                                                                                   flags,
                                                                                   key,
                                                                                   keyboardType,
                                                                                   true));
    if (! ptr) return;
    Params_KeyboardEventCallBack& params = *ptr;
    queue_->push_back(new Item(params, type));
  }

  void
  KeyboardRepeat::primitive_add(EventType eventType,
                                Flags flags,
                                ConsumerKeyCode key)
  {
    if (! queue_) return;
    if (key == ConsumerKeyCode::VK_NONE) return;

    // ------------------------------------------------------------
    Params_KeyboardSpecialEventCallback::auto_ptr ptr(Params_KeyboardSpecialEventCallback::alloc(eventType,
                                                                                                 flags,
                                                                                                 key,
                                                                                                 true));
    if (! ptr) return;
    Params_KeyboardSpecialEventCallback& params = *ptr;
    queue_->push_back(new Item(params, Item::TYPE_NORMAL));
  }

  void
  KeyboardRepeat::primitive_add_downup(Flags flags,
                                       KeyCode key,
                                       KeyboardType keyboardType)
  {
    primitive_add(EventType::DOWN, flags, key, keyboardType, Item::TYPE_DOWNUP);
  }

  int
  KeyboardRepeat::primitive_start(int wait)
  {
    ++id_;
    if (id_ > MAX_KEYBOARDREPEATID) id_ = 0;

    fire_timer_.setTimeoutMS(wait);

    return id_;
  }

  void
  KeyboardRepeat::set(EventType eventType,
                      Flags flags,
                      KeyCode key,
                      KeyboardType keyboardType,
                      int wait)
  {
    if (! queue_) return;

    if (key == KeyCode::VK_NONE) return;

    if (eventType == EventType::MODIFY) {
      goto cancel;

    } else if (eventType == EventType::UP) {
      // The repetition of plural keys is controlled by manual operation.
      // So, we ignore it.
      if (queue_->size() != 1) return;

      // We stop key repeat only when the repeating key is up.
      KeyboardRepeat::Item* p = static_cast<KeyboardRepeat::Item*>(queue_->front());
      if (p && (p->params).type == ParamsUnion::KEYBOARD) {
        Params_KeyboardEventCallBack* params = (p->params).params.params_KeyboardEventCallBack;
        if (params && key == params->key) {
          goto cancel;
        }
      }

    } else if (eventType == EventType::DOWN) {
      cancel();

      primitive_add(eventType, flags, key, keyboardType, Item::TYPE_NORMAL);
      primitive_start(wait);

      IOLOG_DEVEL("KeyboardRepeat::set key:%d flags:0x%x\n", key.get(), flags.get());

    } else {
      goto cancel;
    }

    return;

  cancel:
    cancel();
  }

  void
  KeyboardRepeat::set(EventType eventType,
                      Flags flags,
                      ConsumerKeyCode key)
  {
    if (! queue_) return;

    if (key == ConsumerKeyCode::VK_NONE) return;

    if (eventType == EventType::UP) {
      goto cancel;

    } else if (eventType == EventType::DOWN) {
      if (! key.isRepeatable()) {
        goto cancel;
      }

      cancel();

      primitive_add(eventType, flags, key);
      primitive_start(Config::get_repeat_consumer_initial_wait());

      IOLOG_DEVEL("KeyboardRepeat::set consumer key:%d flags:0x%x\n", key.get(), flags.get());

    } else {
      goto cancel;
    }

    return;

  cancel:
    cancel();
  }

  void
  KeyboardRepeat::fire_timer_callback(OSObject* owner, IOTimerEventSource* sender)
  {
    if (! queue_) return;

    IOLOG_DEVEL("KeyboardRepeat::fire queue_->size = %d\n", static_cast<int>(queue_->size()));

    // ----------------------------------------
    bool isconsumer = false;

    for (KeyboardRepeat::Item* p = static_cast<KeyboardRepeat::Item*>(queue_->front()); p; p = static_cast<KeyboardRepeat::Item*>(p->getnext())) {
      switch ((p->params).type) {
        case ParamsUnion::KEYBOARD:
        {
          Params_KeyboardEventCallBack* params = (p->params).params.params_KeyboardEventCallBack;
          if (params) {
            switch (p->type) {
              case Item::TYPE_NORMAL:
              {
                Params_KeyboardEventCallBack::auto_ptr ptr(Params_KeyboardEventCallBack::alloc(params->eventType,
                                                                                               params->flags,
                                                                                               params->key,
                                                                                               params->keyboardType,
                                                                                               queue_->size() == 1 ? true : false));
                if (ptr) {
                  EventOutputQueue::FireKey::fire(*ptr);
                }
                break;
              }

              case Item::TYPE_DOWNUP:
              {
                EventOutputQueue::FireKey::fire_downup(params->flags,
                                                       params->key,
                                                       params->keyboardType);
                break;
              }
            }
          }
          break;
        }

        case ParamsUnion::KEYBOARD_SPECIAL:
        {
          Params_KeyboardSpecialEventCallback* params = (p->params).params.params_KeyboardSpecialEventCallback;
          if (params) {
            Params_KeyboardSpecialEventCallback::auto_ptr ptr(Params_KeyboardSpecialEventCallback::alloc(params->eventType,
                                                                                                         params->flags,
                                                                                                         params->key,
                                                                                                         queue_->size() == 1 ? true : false));
            if (ptr) {
              EventOutputQueue::FireConsumer::fire(*ptr);
            }
          }
          isconsumer = true;
          break;
        }

        case ParamsUnion::UPDATE_FLAGS:
        case ParamsUnion::RELATIVE_POINTER:
        case ParamsUnion::SCROLL_POINTER:
          // do nothing
          break;
      }
    }

    if (isconsumer) {
      fire_timer_.setTimeoutMS(Config::get_repeat_consumer_wait());
    } else {
      fire_timer_.setTimeoutMS(Config::get_repeat_wait());
    }
  }
}
