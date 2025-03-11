#pragma once
#include <iostream>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>

class KitchenWorker {
public:
    KitchenWorker() {}
    void MakeProduct(size_t cooking_time);
};