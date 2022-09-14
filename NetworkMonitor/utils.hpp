//
// Created by Ruslan on 13/02/2022.
//

#ifndef NETWORK_MONITOR_UTILS_HPP
#define NETWORK_MONITOR_UTILS_HPP

#include <vector>

using std::vector;

namespace utils {

    template <typename T>
    T VectorSum(
            const std::vector<T>& vector,
            T initialValue
    ) {
//        for_each(vector.begin(), vector.end(), [&](auto& val) {
//            initialValue += val;
//        });

        for(T val : vector) {
            initialValue += val;
        }

        return initialValue;
    }

}

#endif //NETWORK_MONITOR_UTILS_HPP
