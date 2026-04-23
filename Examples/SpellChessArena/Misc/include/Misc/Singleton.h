#pragma once

namespace Misc {

    template <class Self>
    class Singleton {
        public:
        static Self & getInstance() {
            static Self inst;
            return inst;
        };
    };

};