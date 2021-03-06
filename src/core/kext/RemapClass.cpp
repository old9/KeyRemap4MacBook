#include "bridge.h"
#include "RemapClass.hpp"
#include "KeyboardRepeat.hpp"
#include "VirtualKey/VK_CONFIG.hpp"
#include "VirtualKey/VK_CHANGE_INPUTSOURCE.hpp"
#include "util/CommonData.hpp"
#include "util/EventInputQueue.hpp"
#include "strlcpy_utf8.hpp"

namespace org_pqrs_KeyRemap4MacBook {
  RemapClass::Item::Item(const uint32_t* vec, size_t length)
  {
    type_ = BRIDGE_REMAPTYPE_NONE;
    active_ = false;

    // ------------------------------------------------------------
    // check parameters.
    //
    if (! vec || length == 0) {
      IOLOG_ERROR("RemapClass::Item::Item invalid parameter.\n");
      return;
    }

    // ------------------------------------------------------------
    // initialize values.
    //
#define INITIALIZE_UNION_VALUE(POINTER, CLASS) {                   \
    POINTER = new CLASS;                                           \
    if (! POINTER) {                                               \
      IOLOG_ERROR("RemapClass::Item::Item failed to allocate.\n"); \
      return;                                                      \
    } else {                                                       \
      type_ = newtype;                                             \
      for (size_t i = 1;; i += 2) {                                \
        size_t datatype_index = i;                                 \
        size_t value_index    = i + 1;                             \
        if (value_index >= length) break;                          \
        (POINTER)->add(vec[datatype_index], vec[value_index]);     \
      }                                                            \
    }                                                              \
}

    unsigned int newtype = vec[0];

    switch (newtype) {
      // handle BRIDGE_REMAPTYPE_NONE as error. (see default)
      case BRIDGE_REMAPTYPE_KEYTOKEY:                       INITIALIZE_UNION_VALUE(p_.keyToKey,                       RemapFunc::KeyToKey);                       break;
      case BRIDGE_REMAPTYPE_KEYTOCONSUMER:                  INITIALIZE_UNION_VALUE(p_.keyToConsumer,                  RemapFunc::KeyToConsumer);                  break;
      case BRIDGE_REMAPTYPE_KEYTOPOINTINGBUTTON:            INITIALIZE_UNION_VALUE(p_.keyToPointingButton,            RemapFunc::KeyToPointingButton);            break;
      case BRIDGE_REMAPTYPE_CONSUMERTOCONSUMER:             INITIALIZE_UNION_VALUE(p_.consumerToConsumer,             RemapFunc::ConsumerToConsumer);             break;
      case BRIDGE_REMAPTYPE_CONSUMERTOKEY:                  INITIALIZE_UNION_VALUE(p_.consumerToKey,                  RemapFunc::ConsumerToKey);                  break;
      case BRIDGE_REMAPTYPE_DOUBLEPRESSMODIFIER:            INITIALIZE_UNION_VALUE(p_.doublePressModifier,            RemapFunc::DoublePressModifier);            break;
      case BRIDGE_REMAPTYPE_DROPKEYAFTERREMAP:              INITIALIZE_UNION_VALUE(p_.dropKeyAfterRemap,              RemapFunc::DropKeyAfterRemap);              break;
      case BRIDGE_REMAPTYPE_DROPPOINTINGRELATIVECURSORMOVE: INITIALIZE_UNION_VALUE(p_.dropPointingRelativeCursorMove, RemapFunc::DropPointingRelativeCursorMove); break;
      case BRIDGE_REMAPTYPE_DROPSCROLLWHEEL:                INITIALIZE_UNION_VALUE(p_.dropScrollWheel,                RemapFunc::DropScrollWheel); break;
      case BRIDGE_REMAPTYPE_FORCENUMLOCKON:                 INITIALIZE_UNION_VALUE(p_.forceNumLockOn,                 RemapFunc::ForceNumLockOn);                 break;
      case BRIDGE_REMAPTYPE_HOLDINGKEYTOKEY:                INITIALIZE_UNION_VALUE(p_.holdingKeyToKey,                RemapFunc::HoldingKeyToKey);                break;
      case BRIDGE_REMAPTYPE_IGNOREMULTIPLESAMEKEYPRESS:     INITIALIZE_UNION_VALUE(p_.ignoreMultipleSameKeyPress,     RemapFunc::IgnoreMultipleSameKeyPress);     break;
      case BRIDGE_REMAPTYPE_KEYOVERLAIDMODIFIER:            INITIALIZE_UNION_VALUE(p_.keyOverlaidModifier,            RemapFunc::KeyOverlaidModifier);            break;
      case BRIDGE_REMAPTYPE_POINTINGBUTTONTOKEY:            INITIALIZE_UNION_VALUE(p_.pointingButtonToKey,            RemapFunc::PointingButtonToKey);            break;
      case BRIDGE_REMAPTYPE_POINTINGBUTTONTOPOINTINGBUTTON: INITIALIZE_UNION_VALUE(p_.pointingButtonToPointingButton, RemapFunc::PointingButtonToPointingButton); break;
      case BRIDGE_REMAPTYPE_POINTINGRELATIVETOSCROLL:       INITIALIZE_UNION_VALUE(p_.pointingRelativeToScroll,       RemapFunc::PointingRelativeToScroll);       break;
      case BRIDGE_REMAPTYPE_SIMULTANEOUSKEYPRESSES:         INITIALIZE_UNION_VALUE(p_.simultaneousKeyPresses,         RemapFunc::SimultaneousKeyPresses);         break;
      case BRIDGE_REMAPTYPE_SETKEYBOARDTYPE:                INITIALIZE_UNION_VALUE(p_.setKeyboardType,                RemapFunc::SetKeyboardType);                break;
      case BRIDGE_REMAPTYPE_SCROLLWHEELTOSCROLLWHEEL:       INITIALIZE_UNION_VALUE(p_.scrollWheelToScrollWheel,       RemapFunc::ScrollWheelToScrollWheel);       break;
      case BRIDGE_REMAPTYPE_SCROLLWHEELTOKEY:               INITIALIZE_UNION_VALUE(p_.scrollWheelToKey,               RemapFunc::ScrollWheelToKey);               break;
      default:
        IOLOG_ERROR("RemapClass::Item::Item unknown type_ (%d)\n", type_);
        type_ = BRIDGE_REMAPTYPE_NONE;
        return;
    }

#undef INITIALIZE_UNION_VALUE
  }

  RemapClass::Item::~Item(void)
  {
#define DELETE_UNLESS_NULL(POINTER) { \
    if (POINTER) { delete POINTER; }  \
}

    switch (type_) {
      case BRIDGE_REMAPTYPE_NONE:                                                                                  break;
      case BRIDGE_REMAPTYPE_KEYTOKEY:                       DELETE_UNLESS_NULL(p_.keyToKey);                       break;
      case BRIDGE_REMAPTYPE_KEYTOCONSUMER:                  DELETE_UNLESS_NULL(p_.keyToConsumer);                  break;
      case BRIDGE_REMAPTYPE_KEYTOPOINTINGBUTTON:            DELETE_UNLESS_NULL(p_.keyToPointingButton);            break;
      case BRIDGE_REMAPTYPE_CONSUMERTOCONSUMER:             DELETE_UNLESS_NULL(p_.consumerToConsumer);             break;
      case BRIDGE_REMAPTYPE_CONSUMERTOKEY:                  DELETE_UNLESS_NULL(p_.consumerToKey);                  break;
      case BRIDGE_REMAPTYPE_DOUBLEPRESSMODIFIER:            DELETE_UNLESS_NULL(p_.doublePressModifier);            break;
      case BRIDGE_REMAPTYPE_DROPKEYAFTERREMAP:              DELETE_UNLESS_NULL(p_.dropKeyAfterRemap);              break;
      case BRIDGE_REMAPTYPE_DROPPOINTINGRELATIVECURSORMOVE: DELETE_UNLESS_NULL(p_.dropPointingRelativeCursorMove); break;
      case BRIDGE_REMAPTYPE_DROPSCROLLWHEEL:                DELETE_UNLESS_NULL(p_.dropScrollWheel);                break;
      case BRIDGE_REMAPTYPE_FORCENUMLOCKON:                 DELETE_UNLESS_NULL(p_.forceNumLockOn);                 break;
      case BRIDGE_REMAPTYPE_HOLDINGKEYTOKEY:                DELETE_UNLESS_NULL(p_.holdingKeyToKey);                break;
      case BRIDGE_REMAPTYPE_IGNOREMULTIPLESAMEKEYPRESS:     DELETE_UNLESS_NULL(p_.ignoreMultipleSameKeyPress);     break;
      case BRIDGE_REMAPTYPE_KEYOVERLAIDMODIFIER:            DELETE_UNLESS_NULL(p_.keyOverlaidModifier);            break;
      case BRIDGE_REMAPTYPE_POINTINGBUTTONTOKEY:            DELETE_UNLESS_NULL(p_.pointingButtonToKey);            break;
      case BRIDGE_REMAPTYPE_POINTINGBUTTONTOPOINTINGBUTTON: DELETE_UNLESS_NULL(p_.pointingButtonToPointingButton); break;
      case BRIDGE_REMAPTYPE_POINTINGRELATIVETOSCROLL:       DELETE_UNLESS_NULL(p_.pointingRelativeToScroll);       break;
      case BRIDGE_REMAPTYPE_SIMULTANEOUSKEYPRESSES:         DELETE_UNLESS_NULL(p_.simultaneousKeyPresses);         break;
      case BRIDGE_REMAPTYPE_SETKEYBOARDTYPE:                DELETE_UNLESS_NULL(p_.setKeyboardType);                break;
      case BRIDGE_REMAPTYPE_SCROLLWHEELTOSCROLLWHEEL:       DELETE_UNLESS_NULL(p_.scrollWheelToScrollWheel);       break;
      case BRIDGE_REMAPTYPE_SCROLLWHEELTOKEY:               DELETE_UNLESS_NULL(p_.scrollWheelToKey);               break;
      default:
        IOLOG_ERROR("RemapClass::Item::terminate unknown type_ (%d)\n", type_);
        break;
    }

#undef DELETE_UNLESS_NULL

    // ------------------------------------------------------------
    for (size_t i = 0; i < filters_.size(); ++i) {
      RemapFilter::FilterUnion* p = filters_[i];
      if (p) {
        delete p;
      }
    }
  }

  void
  RemapClass::Item::append_filter(const unsigned int* vec, size_t length)
  {
    // ------------------------------------------------------------
    // check parameters.
    //
    if (! vec || length <= 0) {
      IOLOG_ERROR("RemapClass::Item::append_filter invalid parameter.\n");
      return;
    }

    // ------------------------------------------------------------
    // append to filters_.
    //
    RemapFilter::FilterUnion* newp = new RemapFilter::FilterUnion(vec, length);
    if (! newp) {
      IOLOG_ERROR("RemapClass::Item::append_filter failed to allocate.\n");
      return;
    }

    filters_.push_back(newp);
  }

  void
  RemapClass::Item::remap(RemapParams& remapParams)
  {
    if (remapParams.params.ex_iskeydown) {
      if (isblocked()) return;
    } else {
      // We ignore filters_ if active_ is set at KeyDown.
      if (isblocked() && ! active_) return;
    }

#define CALL_UNION_FUNCTION(POINTER) {         \
    if (POINTER) {                             \
      if ((POINTER)->remap(remapParams)) {     \
        if (remapParams.params.ex_iskeydown) { \
          active_ = true;                      \
        } else {                               \
          active_ = false;                     \
        }                                      \
      }                                        \
    }                                          \
}

    switch (type_) {
      case BRIDGE_REMAPTYPE_KEYTOKEY:                   CALL_UNION_FUNCTION(p_.keyToKey);                   break;
      case BRIDGE_REMAPTYPE_KEYTOCONSUMER:              CALL_UNION_FUNCTION(p_.keyToConsumer);              break;
      case BRIDGE_REMAPTYPE_KEYTOPOINTINGBUTTON:        CALL_UNION_FUNCTION(p_.keyToPointingButton);        break;
      case BRIDGE_REMAPTYPE_DOUBLEPRESSMODIFIER:        CALL_UNION_FUNCTION(p_.doublePressModifier);        break;
      case BRIDGE_REMAPTYPE_HOLDINGKEYTOKEY:            CALL_UNION_FUNCTION(p_.holdingKeyToKey);            break;
      case BRIDGE_REMAPTYPE_IGNOREMULTIPLESAMEKEYPRESS: CALL_UNION_FUNCTION(p_.ignoreMultipleSameKeyPress); break;
      case BRIDGE_REMAPTYPE_KEYOVERLAIDMODIFIER:        CALL_UNION_FUNCTION(p_.keyOverlaidModifier);        break;
      case BRIDGE_REMAPTYPE_SIMULTANEOUSKEYPRESSES:     CALL_UNION_FUNCTION(p_.simultaneousKeyPresses);     break;
      default:
        // do nothing. (Do not call IOLOG_ERROR)
        break;
    }

#undef CALL_UNION_FUNCTION
  }

  void
  RemapClass::Item::remap(RemapConsumerParams& remapParams)
  {
    if (remapParams.params.ex_iskeydown) {
      if (isblocked()) return;
    } else {
      // We ignore filters_ if active_ is set at KeyDown.
      if (isblocked() && ! active_) return;
    }

#define CALL_UNION_FUNCTION(POINTER) {         \
    if (POINTER) {                             \
      if ((POINTER)->remap(remapParams)) {     \
        if (remapParams.params.ex_iskeydown) { \
          active_ = true;                      \
        } else {                               \
          active_ = false;                     \
        }                                      \
      }                                        \
    }                                          \
}

    switch (type_) {
      case BRIDGE_REMAPTYPE_CONSUMERTOCONSUMER:  CALL_UNION_FUNCTION(p_.consumerToConsumer);  break;
      case BRIDGE_REMAPTYPE_CONSUMERTOKEY:       CALL_UNION_FUNCTION(p_.consumerToKey);       break;
      case BRIDGE_REMAPTYPE_HOLDINGKEYTOKEY:     CALL_UNION_FUNCTION(p_.holdingKeyToKey);     break;
      case BRIDGE_REMAPTYPE_KEYOVERLAIDMODIFIER: CALL_UNION_FUNCTION(p_.keyOverlaidModifier); break;
      default:
        // do nothing. (Do not call IOLOG_ERROR)
        break;
    }

#undef CALL_UNION_FUNCTION
  }

  void
  RemapClass::Item::remap(RemapPointingParams_relative& remapParams)
  {
    if (remapParams.params.ex_isbuttondown) {
      if (isblocked()) return;
    } else {
      // We ignore filters_ if active_ is set at ButtonDown.
      if (isblocked() && ! active_) return;
    }

#define CALL_UNION_FUNCTION(POINTER) {            \
    if (POINTER) {                                \
      if ((POINTER)->remap(remapParams)) {        \
        if (remapParams.params.ex_isbuttondown) { \
          active_ = true;                         \
        } else {                                  \
          active_ = false;                        \
        }                                         \
      }                                           \
    }                                             \
}

    switch (type_) {
      case BRIDGE_REMAPTYPE_DROPPOINTINGRELATIVECURSORMOVE: CALL_UNION_FUNCTION(p_.dropPointingRelativeCursorMove); break;
      case BRIDGE_REMAPTYPE_POINTINGBUTTONTOKEY:            CALL_UNION_FUNCTION(p_.pointingButtonToKey);            break;
      case BRIDGE_REMAPTYPE_POINTINGBUTTONTOPOINTINGBUTTON: CALL_UNION_FUNCTION(p_.pointingButtonToPointingButton); break;
      case BRIDGE_REMAPTYPE_POINTINGRELATIVETOSCROLL:       CALL_UNION_FUNCTION(p_.pointingRelativeToScroll);       break;
      case BRIDGE_REMAPTYPE_HOLDINGKEYTOKEY:                CALL_UNION_FUNCTION(p_.holdingKeyToKey);                break;
      case BRIDGE_REMAPTYPE_KEYOVERLAIDMODIFIER:            CALL_UNION_FUNCTION(p_.keyOverlaidModifier);            break;
      default:
        // do nothing. (Do not call IOLOG_ERROR)
        break;
    }

#undef CALL_UNION_FUNCTION
  }

  void
  RemapClass::Item::remap(RemapPointingParams_scroll& remapParams)
  {
    if (isblocked()) return;

#define CALL_UNION_FUNCTION(POINTER) {              \
    if (POINTER) { (POINTER)->remap(remapParams); } \
}

    switch (type_) {
      case BRIDGE_REMAPTYPE_DROPSCROLLWHEEL:          CALL_UNION_FUNCTION(p_.dropScrollWheel);          break;
      case BRIDGE_REMAPTYPE_SCROLLWHEELTOSCROLLWHEEL: CALL_UNION_FUNCTION(p_.scrollWheelToScrollWheel); break;
      case BRIDGE_REMAPTYPE_SCROLLWHEELTOKEY:         CALL_UNION_FUNCTION(p_.scrollWheelToKey);         break;
      case BRIDGE_REMAPTYPE_HOLDINGKEYTOKEY:          CALL_UNION_FUNCTION(p_.holdingKeyToKey);          break;
      case BRIDGE_REMAPTYPE_KEYOVERLAIDMODIFIER:      CALL_UNION_FUNCTION(p_.keyOverlaidModifier);      break;
      default:
        // do nothing. (Do not call IOLOG_ERROR)
        break;
    }

#undef CALL_UNION_FUNCTION
  }

  bool
  RemapClass::Item::drop(const Params_KeyboardEventCallBack& params)
  {
    if (isblocked()) return false;

#define CALL_UNION_FUNCTION(POINTER) {               \
    if (POINTER) { return (POINTER)->drop(params); } \
}

    switch (type_) {
      case BRIDGE_REMAPTYPE_DROPKEYAFTERREMAP: CALL_UNION_FUNCTION(p_.dropKeyAfterRemap); break;
      default:
        // do nothing. (Do not call IOLOG_ERROR)
        break;
    }

#undef CALL_UNION_FUNCTION

    return false;
  }

  bool
  RemapClass::Item::remap_SimultaneousKeyPresses(void)
  {
    if (isblocked()) return false;

#define CALL_UNION_FUNCTION(POINTER) {          \
    if (POINTER) { return (POINTER)->remap(); } \
}

    switch (type_) {
      case BRIDGE_REMAPTYPE_SIMULTANEOUSKEYPRESSES: CALL_UNION_FUNCTION(p_.simultaneousKeyPresses); break;
      default:
        // do nothing. (Do not call IOLOG_ERROR)
        break;
    }

#undef CALL_UNION_FUNCTION

    return false;
  }

  void
  RemapClass::Item::remap_setkeyboardtype(KeyboardType& keyboardType)
  {
    if (isblocked()) return;

#define CALL_UNION_FUNCTION(POINTER) {               \
    if (POINTER) { (POINTER)->remap(keyboardType); } \
}

    switch (type_) {
      case BRIDGE_REMAPTYPE_SETKEYBOARDTYPE: CALL_UNION_FUNCTION(p_.setKeyboardType); break;
      default:
        // do nothing. (Do not call IOLOG_ERROR)
        break;
    }

#undef CALL_UNION_FUNCTION
  }

  void
  RemapClass::Item::remap_forcenumlockon(ListHookedKeyboard::Item* item)
  {
    if (isblocked()) return;

#define CALL_UNION_FUNCTION(POINTER) {       \
    if (POINTER) { (POINTER)->remap(item); } \
}

    switch (type_) {
      case BRIDGE_REMAPTYPE_FORCENUMLOCKON: CALL_UNION_FUNCTION(p_.forceNumLockOn); break;
      default:
        // do nothing. (Do not call IOLOG_ERROR)
        break;
    }

#undef CALL_UNION_FUNCTION
  }

  void
  RemapClass::Item::call_disabled_callback(void)
  {
#define CALL_UNION_FUNCTION(POINTER) {               \
    if (POINTER) { (POINTER)->disabled_callback(); } \
}

    switch (type_) {
      case BRIDGE_REMAPTYPE_KEYTOKEY:            CALL_UNION_FUNCTION(p_.keyToKey);            break;
      case BRIDGE_REMAPTYPE_POINTINGBUTTONTOKEY: CALL_UNION_FUNCTION(p_.pointingButtonToKey); break;
      default:
        // do nothing. (Do not call IOLOG_ERROR)
        break;
    }

#undef CALL_UNION_FUNCTION
  }

  bool
  RemapClass::Item::isblocked(void)
  {
    for (size_t i = 0; i < filters_.size(); ++i) {
      RemapFilter::FilterUnion* p = filters_[i];
      if (p && p->isblocked()) return true;
    }

    return false;
  }

  // ----------------------------------------------------------------------
  int RemapClass::allocation_count_ = 0;

  RemapClass::RemapClass(const uint32_t* const initialize_vector, uint32_t vector_size) :
    statusmessage_(NULL),
    enabled_(false),
    is_simultaneouskeypresses_(false)
  {
    if (! initialize_vector) {
      IOLOG_ERROR("RemapClass::RemapClass invalid parameter.\n");
      return;
    }

    // ------------------------------------------------------------
    if (allocation_count_ + vector_size > MAX_ALLOCATION_COUNT) {
      IOLOG_ERROR("RemapClass::RemapClass too many allocation_count_.\n");
      return;
    }
    allocation_count_ += vector_size;

    // ------------------------------------------------------------
    // initialize items_ from vector
    const uint32_t* p = initialize_vector;

    for (;;) {
      if (p == initialize_vector + vector_size) break;

      if (p > initialize_vector + vector_size) {
        IOLOG_ERROR("RemapClass::RemapClass vector_size mismatch. (vector_size:%d)\n", vector_size);
        return;
      }

      uint32_t size = *p++;
      if (p + size > initialize_vector + vector_size) {
        IOLOG_ERROR("RemapClass::RemapClass vector_size mismatch. (vector_size:%d)\n", vector_size);
        return;
      }

      if (size > 0) {
        unsigned int type = p[0];

        if (BRIDGE_REMAPTYPE_NONE < type && type < BRIDGE_REMAPTYPE_END) {
          Item* newp = new Item(p, size);
          if (! newp) {
            IOLOG_ERROR("RemapClass::RemapClass newp == NULL.\n");
            return;
          }
          items_.push_back(newp);

          if (type == BRIDGE_REMAPTYPE_SIMULTANEOUSKEYPRESSES) {
            is_simultaneouskeypresses_ = true;
          }

        } else if (BRIDGE_FILTERTYPE_NONE < type && type < BRIDGE_FILTERTYPE_END) {
          if (items_.size() == 0) {
            IOLOG_ERROR("RemapClass::RemapClass invalid filter (%d).\n", type);
            return;
          }
          Item* back = items_.back();
          if (back) {
            back->append_filter(p, size);
          }

        } else if (type == BRIDGE_STATUSMESSAGE) {
          if (statusmessage_) {
            delete[] statusmessage_;
          }
          statusmessage_ = new char[size];
          if (statusmessage_) {
            for (size_t i = 0; i < size - 1; ++i) {
              statusmessage_[i] = p[i + 1];
            }
            statusmessage_[size - 1] = '\0';
          }

        } else if (type == BRIDGE_VK_CONFIG) {
          if (size != 5) {
            IOLOG_ERROR("RemapClass::RemapClass invalid size for BRIDGE_VK_CONFIG. (%d)\n", size);
            return;

          } else {
            unsigned int keycode_toggle         = p[1];
            unsigned int keycode_force_on       = p[2];
            unsigned int keycode_force_off      = p[3];
            unsigned int keycode_sync_keydownup = p[4];
            VirtualKey::VK_CONFIG::add_item(this,
                                            keycode_toggle,
                                            keycode_force_on,
                                            keycode_force_off,
                                            keycode_sync_keydownup);
          }

        } else if (type == BRIDGE_VK_CHANGE_INPUTSOURCE) {
          if (size != 2) {
            IOLOG_ERROR("RemapClass::RemapClass invalid size for BRIDGE_VK_CHANGE_INPUTSOURCE. (%d)\n", size);
            return;

          } else {
            unsigned int keycode = p[1];
            VirtualKey::VK_CHANGE_INPUTSOURCE::add_item(this, keycode);
          }

        } else {
          IOLOG_ERROR("RemapClass::RemapClass unknown type:%d.\n", type);
          return;
        }

        p += size;
      }
    }
  }

  RemapClass::~RemapClass(void)
  {
    for (size_t i = 0; i < items_.size(); ++i) {
      Item* p = items_[i];
      if (p) {
        delete p;
      }
    }
    if (statusmessage_) {
      delete[] statusmessage_;
    }
  }

  void
  RemapClass::remap_setkeyboardtype(KeyboardType& keyboardType)
  {
    for (size_t i = 0; i < items_.size(); ++i) {
      Item* p = items_[i];
      if (p) {
        p->remap_setkeyboardtype(keyboardType);
      }
    }
  }

  void
  RemapClass::remap_forcenumlockon(ListHookedKeyboard::Item* item)
  {
    for (size_t i = 0; i < items_.size(); ++i) {
      Item* p = items_[i];
      if (p) {
        p->remap_forcenumlockon(item);
      }
    }
  }

  void
  RemapClass::remap_key(RemapParams& remapParams)
  {
    for (size_t i = 0; i < items_.size(); ++i) {
      Item* p = items_[i];
      if (p) {
        // DependingPressingPeriodKeyToKey watches another key status.
        // Therefore, we need to call 'p->remap(remapParams)' for all items.
        p->remap(remapParams);
      }
    }
  }

  void
  RemapClass::remap_consumer(RemapConsumerParams& remapParams)
  {
    for (size_t i = 0; i < items_.size(); ++i) {
      Item* p = items_[i];
      if (p) {
        p->remap(remapParams);
      }
    }
  }

  void
  RemapClass::remap_pointing(RemapPointingParams_relative& remapParams)
  {
    for (size_t i = 0; i < items_.size(); ++i) {
      Item* p = items_[i];
      if (p) {
        p->remap(remapParams);
      }
    }
  }

  void
  RemapClass::remap_pointing_scroll(RemapPointingParams_scroll& remapParams)
  {
    for (size_t i = 0; i < items_.size(); ++i) {
      Item* p = items_[i];
      if (p) {
        p->remap(remapParams);
      }
    }
  }

  bool
  RemapClass::remap_simultaneouskeypresses(void)
  {
    bool queue_changed = false;
    for (size_t i = 0; i < items_.size(); ++i) {
      Item* p = items_[i];
      if (p) {
        if (p->remap_SimultaneousKeyPresses()) {
          queue_changed = true;
        }
      }
    }

    return queue_changed;
  }

  bool
  RemapClass::remap_dropkeyafterremap(const Params_KeyboardEventCallBack& params)
  {
    bool dropped = false;

    if (enabled()) {
      for (size_t i = 0; i < items_.size(); ++i) {
        Item* p = items_[i];
        if (p) {
          if (p->drop(params)) {
            dropped = true;
          }
        }
      }
    }

    return dropped;
  }

  void
  RemapClass::call_disabled_callback(void)
  {
    for (size_t i = 0; i < items_.size(); ++i) {
      Item* p = items_[i];
      if (p) {
        p->call_disabled_callback();
      }
    }
  }

  const char*
  RemapClass::get_statusmessage(void)
  {
    return statusmessage_;
  }

  bool
  RemapClass::is_simultaneouskeypresses(void)
  {
    return is_simultaneouskeypresses_;
  }

  void
  RemapClass::log_allocation_count(void)
  {
    IOLOG_INFO("RemapClass::allocation_count_ %d/%d (memory usage: %d%% of %dKB)\n",
               allocation_count_,
               MAX_ALLOCATION_COUNT,
               allocation_count_ * 100 / MAX_ALLOCATION_COUNT,
               static_cast<int>(MAX_ALLOCATION_COUNT * sizeof(uint32_t) / 1024));
  }

  void
  RemapClass::reset_allocation_count(void)
  {
    allocation_count_ = 0;
  }

  // ================================================================================
  namespace RemapClassManager {
    TimerWrapper refresh_timer_;

    char statusmessage_[BRIDGE_USERCLIENT_STATUS_MESSAGE_MAXLEN];
    char lastmessage_[BRIDGE_USERCLIENT_STATUS_MESSAGE_MAXLEN];
    bool isEventInputQueueDelayEnabled_ = false;

    Vector_RemapClassPointer* remapclasses_ = NULL;
    Vector_RemapClassPointer* enabled_remapclasses_ = NULL;

    // ======================================================================
    static void
    refresh_timer_callback(OSObject* owner, IOTimerEventSource* sender)
    {
      if (! remapclasses_) {
        IOLOG_ERROR("RemapClassManager::refresh_core remapclasses_ == NULL.\n");
        return;
      }

      // ----------------------------------------
      if (enabled_remapclasses_) {
        // call disabled_callback
        for (size_t i = 0; i < enabled_remapclasses_->size(); ++i) {
          RemapClass* p = (*enabled_remapclasses_)[i];
          if (p && ! p->enabled()) {
            p->call_disabled_callback();
          }
        }

        delete enabled_remapclasses_;
      }
      enabled_remapclasses_ = new Vector_RemapClassPointer();
      if (! enabled_remapclasses_) return;

      // ----------------------------------------
      KeyboardRepeat::cancel();

      statusmessage_[0] = '\0';

      isEventInputQueueDelayEnabled_ = false;

      for (size_t i = 0; i < remapclasses_->size(); ++i) {
        RemapClass* p = (*remapclasses_)[i];
        if (! p) continue;

        if (p->enabled()) {
          enabled_remapclasses_->push_back(p);

          const char* msg = p->get_statusmessage();
          if (msg) {
            strlcat(statusmessage_, msg, sizeof(statusmessage_));
            strlcat(statusmessage_, " ", sizeof(statusmessage_));
          }

          if (p->is_simultaneouskeypresses()) {
            isEventInputQueueDelayEnabled_ = true;
          }
        }
      }

      if (strcmp(statusmessage_, lastmessage_) != 0) {
        pqrs::strlcpy_utf8::strlcpy(lastmessage_, statusmessage_, sizeof(lastmessage_));

        int index = BRIDGE_USERCLIENT_STATUS_MESSAGE_EXTRA;
        CommonData::clear_statusmessage(index);
        CommonData::append_statusmessage(index, statusmessage_);
        CommonData::send_notification_statusmessage(index);
      }
    }

    // ======================================================================

    void
    initialize(IOWorkLoop& workloop)
    {
      statusmessage_[0] = '\0';
      lastmessage_[0] = '\0';
      remapclasses_ = NULL;
      enabled_remapclasses_ = NULL;

      refresh_timer_.initialize(&workloop, NULL, refresh_timer_callback);
    }

    static void
    clear_remapclasses(void)
    {
      VirtualKey::VK_CONFIG::clear_items();

      if (enabled_remapclasses_) {
        delete enabled_remapclasses_;
        enabled_remapclasses_ = NULL;
      }

      if (remapclasses_) {
        for (size_t i = 0; i < remapclasses_->size(); ++i) {
          RemapClass* p = (*remapclasses_)[i];
          if (p) {
            delete p;
          }
        }
        delete remapclasses_;
        remapclasses_ = NULL;
      }
    }

    void
    terminate(void)
    {
      refresh_timer_.terminate();

      clear_remapclasses();
    }

    bool
    load_remapclasses_initialize_vector(const uint32_t* const remapclasses_initialize_vector, mach_vm_size_t vector_size)
    {
      // ------------------------------------------------------------
      // clean previous resources and setup new resources.
      clear_remapclasses();

      remapclasses_ = new Vector_RemapClassPointer();
      if (! remapclasses_) {
        IOLOG_ERROR("%s remapclasses_ == NULL.\n", __FUNCTION__);
        goto error;
      }

      // ------------------------------------------------------------
      // Validate vector_size

      // "vector_size" is byte of remapclasses_initialize_vector. (!= count of items.)
      // Confirming vector_size is a multiple of sizeof(uint32_t).
      if ((vector_size % sizeof(uint32_t)) != 0) {
        IOLOG_ERROR("%s (vector_size %% sizeof(uint32_t)) != 0. (%d)\n", __FUNCTION__, static_cast<int>(vector_size));
        goto error;
      }

      // change vector_size to num of uint32_t.
      vector_size /= sizeof(uint32_t);
      // Then, we can treat "remapclasses_initialize_vector + vector_size" as valid.

      if (vector_size < 2) {
        IOLOG_ERROR("%s vector_size < 2. (%d)\n", __FUNCTION__, static_cast<int>(vector_size));
        goto error;
      }
      if (vector_size > RemapClass::MAX_ALLOCATION_COUNT) {
        IOLOG_ERROR("%s too large vector_size. (%d)\n", __FUNCTION__, static_cast<int>(vector_size));
        goto error;
      }

      // ------------------------------------------------------------
      {
        const uint32_t* p = remapclasses_initialize_vector;
        uint32_t version = *p++;
        uint32_t count   = *p++;

        if (version != BRIDGE_REMAPCLASS_INITIALIZE_VECTOR_FORMAT_VERSION) {
          IOLOG_ERROR("%s version mismatch.\n", __FUNCTION__);
          goto error;
        }
        if (count > RemapClass::MAX_CONFIG_COUNT) {
          IOLOG_ERROR("%s too many count. (%d)\n", __FUNCTION__, count);
          goto error;
        }

        // ------------------------------------------------------------
        // load
        remapclasses_->reserve(count);
        RemapClass::reset_allocation_count();

        // (1) Setting NULL to all items.
        for (uint32_t i = 0; i < count; ++i) {
          remapclasses_->push_back(NULL);
        }

        // (2) Setting RemapClass* to items.
        for (uint32_t i = 0; i < count; ++i) {
          if (p >= remapclasses_initialize_vector + vector_size) {
            IOLOG_ERROR("%s vector_size mismatch.\n", __FUNCTION__);
            goto error;
          }

          uint32_t size = *p++;
          if (p + size > remapclasses_initialize_vector + vector_size) {
            IOLOG_ERROR("%s vector_size mismatch. (vector_size:%d, size:%d)\n", __FUNCTION__,
                        static_cast<int>(vector_size), size);
            goto error;
          }
          if (size == 0) {
            IOLOG_ERROR("%s size == 0.\n", __FUNCTION__);
            goto error;
          }

          uint32_t configindex = *p++;
          --size;

          RemapClass* newp = new RemapClass(p, size);
          if (! newp) {
            IOLOG_ERROR("%s newp == NULL.\n", __FUNCTION__);
            goto error;
          }
          p += size;

          if (configindex >= remapclasses_->size()) {
            IOLOG_ERROR("%s invalid configindex %d (remapclasses_->size() == %d).\n", __FUNCTION__,
                        configindex, static_cast<int>(remapclasses_->size()));
            goto error;
          }
          (*remapclasses_)[configindex] = newp;
        }

        // (3) Making sure that is not NULL for all items.
        for (uint32_t i = 0; i < remapclasses_->size(); ++i) {
          if (! (*remapclasses_)[i]) {
            IOLOG_ERROR("%s (*remapclasses_)[i] == NULL.\n", __FUNCTION__);
            goto error;
          }
        }
      }

      RemapClass::log_allocation_count();

      return true;

    error:
      clear_remapclasses();
      return false;
    }

    bool
    set_config(const int32_t* const config_vector, mach_vm_size_t config_size)
    {
      // ------------------------------------------------------------
      // check
      if (! remapclasses_) {
        IOLOG_ERROR("%s remapclasses_ == NULL.\n", __FUNCTION__);
        return false;
      }

      if (config_size != (BRIDGE_ESSENTIAL_CONFIG_INDEX__END__ + remapclasses_->size()) * sizeof(int32_t)) {
        IOLOG_ERROR("%s config_size mismatch.\n", __FUNCTION__);
        return false;
      }

      // ------------------------------------------------------------
      // essential config
      const int32_t* p = config_vector;
      Config::set_essential_config(p, BRIDGE_ESSENTIAL_CONFIG_INDEX__END__);
      // remapclasses config
      p += BRIDGE_ESSENTIAL_CONFIG_INDEX__END__;
      for (size_t i = 0; i < remapclasses_->size(); ++i) {
        RemapClass* rc = (*remapclasses_)[i];
        if (! rc) {
          IOLOG_ERROR("%s RemapClass == NULL.\n", __FUNCTION__);
        } else {
          rc->setEnabled(p[i]);
        }
      }

      refresh();

      return true;
    }

    void
    refresh(void)
    {
      // We use timer to prevent deadlock of lock_. (refresh may be called in the remap_key, remap_consumer, *).
      enum {
        DELAY = 1,
      };
      refresh_timer_.setTimeoutMS(DELAY);
    }

    // ----------------------------------------------------------------------
#define CALL_REMAPCLASS_FUNC(FUNC, PARAMS) {                       \
    if (enabled_remapclasses_) {                                   \
      for (size_t i = 0; i < enabled_remapclasses_->size(); ++i) { \
        RemapClass* p = (*enabled_remapclasses_)[i];               \
        if (p) p->FUNC(PARAMS);                                    \
      }                                                            \
    }                                                              \
}

    void
    remap_setkeyboardtype(KeyboardType& keyboardType)
    {
      CALL_REMAPCLASS_FUNC(remap_setkeyboardtype, keyboardType);
    }

    void
    remap_forcenumlockon(ListHookedKeyboard::Item* item)
    {
      CALL_REMAPCLASS_FUNC(remap_forcenumlockon, item);
    }

    void
    remap_key(RemapParams& remapParams)
    {
      CALL_REMAPCLASS_FUNC(remap_key, remapParams);
    }

    void
    remap_consumer(RemapConsumerParams& remapParams)
    {
      CALL_REMAPCLASS_FUNC(remap_consumer, remapParams);
    }

    void
    remap_pointing(RemapPointingParams_relative& remapParams)
    {
      CALL_REMAPCLASS_FUNC(remap_pointing, remapParams);
    }

    void
    remap_pointing_scroll(RemapPointingParams_scroll& remapParams)
    {
      CALL_REMAPCLASS_FUNC(remap_pointing_scroll, remapParams);
    }

#undef CALL_REMAPCLASS_FUNC

    bool
    remap_simultaneouskeypresses(void)
    {
      bool queue_changed = false;

      if (enabled_remapclasses_) {
        for (size_t i = 0; i < enabled_remapclasses_->size(); ++i) {
          RemapClass* p = (*enabled_remapclasses_)[i];
          if (p) {
            if (p->remap_simultaneouskeypresses()) {
              queue_changed = true;
            }
          }
        }
      }

      return queue_changed;
    }

    bool
    remap_dropkeyafterremap(const Params_KeyboardEventCallBack& params)
    {
      bool dropped = false;

      if (enabled_remapclasses_) {
        for (size_t i = 0; i < enabled_remapclasses_->size(); ++i) {
          RemapClass* p = (*enabled_remapclasses_)[i];
          if (p) {
            if (p->remap_dropkeyafterremap(params)) dropped = true;
          }
        }
      }

      return dropped;
    }

    bool
    isEventInputQueueDelayEnabled(void)
    {
      return isEventInputQueueDelayEnabled_;
    }

    bool
    isEnabled(size_t configindex)
    {
      if (! remapclasses_) return false;

      if (configindex >= remapclasses_->size()) {
        IOLOG_ERROR("RemapClass::isEnabled invalid configindex.\n");
        return false;
      }

      RemapClass* p = (*remapclasses_)[configindex];
      if (! p) return false;

      return p->enabled();
    }
  }
}
