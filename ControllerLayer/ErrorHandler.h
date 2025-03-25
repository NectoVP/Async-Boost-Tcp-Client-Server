#pragma once

#include <iostream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

void fail(boost::beast::error_code ec, char const* what);