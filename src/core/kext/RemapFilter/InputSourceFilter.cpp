#include <IOKit/IOLib.h>

#include "CommonData.hpp"
#include "IOLogWrapper.hpp"
#include "InputSourceFilter.hpp"
#include "bridge.h"

namespace org_pqrs_Karabiner {
  namespace RemapFilter {
    void
    InputSourceFilter::initialize(const unsigned int* vec, size_t length)
    {
      for (size_t i = 0; i < length; ++i) {
        targets_.push_back(AddValue(vec[i]));
      }
    }

    bool
    InputSourceFilter::isblocked(void)
    {
      unsigned int current = 0;
      switch (get_type()) {
        case BRIDGE_FILTERTYPE_INPUTSOURCE_NOT:
        case BRIDGE_FILTERTYPE_INPUTSOURCE_ONLY:
          current = CommonData::getcurrent_workspacedata().inputsource;
          break;

        case BRIDGE_FILTERTYPE_INPUTSOURCEDETAIL_NOT:
        case BRIDGE_FILTERTYPE_INPUTSOURCEDETAIL_ONLY:
          current = CommonData::getcurrent_workspacedata().inputsourcedetail;
          break;
      }

      switch (get_type()) {
        case BRIDGE_FILTERTYPE_INPUTSOURCE_NOT:
        case BRIDGE_FILTERTYPE_INPUTSOURCEDETAIL_NOT:
        case BRIDGE_FILTERTYPE_INPUTSOURCE_ONLY:
        case BRIDGE_FILTERTYPE_INPUTSOURCEDETAIL_ONLY:
        {
          bool isnot = (get_type() == BRIDGE_FILTERTYPE_INPUTSOURCE_NOT ||
                        get_type() == BRIDGE_FILTERTYPE_INPUTSOURCEDETAIL_NOT);

          for (size_t i = 0; i < targets_.size(); ++i) {
            if (targets_[i] == current) {
              return isnot ? true : false;
            }
          }
          return isnot ? false : true;
        }

        default:
          IOLOG_ERROR("InputSourceFilter::isblocked unknown type_(%d)\n", get_type());
          break;
      }

      return false;
    }
  }
}
