#include "ErrorHandler.h"

void fail(boost::beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}