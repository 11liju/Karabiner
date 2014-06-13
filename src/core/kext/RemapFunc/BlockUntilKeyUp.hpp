#ifndef BLOCKUNTILKEYUP_HPP
#define BLOCKUNTILKEYUP_HPP

#include "FromEvent.hpp"
#include "ParamsUnion.hpp"
#include "Types.hpp"

namespace org_pqrs_Karabiner {
  namespace RemapFunc {
    class BlockUntilKeyUp {
    public:
      void add(AddDataType datatype, AddValue newval);
      const FromEvent& getFromEvent(void) const { return fromEvent_; }

    private:
      FromEvent fromEvent_;
    };
  }
}

#endif
