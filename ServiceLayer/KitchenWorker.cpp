#include "KitchenWorker.h"

void KitchenWorker::MakeProduct(size_t cooking_time) {
    boost::this_thread::sleep_for(boost::chrono::milliseconds(cooking_time));
}