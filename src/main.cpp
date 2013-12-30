#include <iostream>
#include <string>
#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <unistd.h>

#define RESET_COLOR "\e[m"
#define MAKE_GREEN "\e[32m"
#define MAKE_YELLOW "\e[33m"

const char * esc(const char * code)
{
	return ::isatty(::fileno(stdout))
		? (code)
		: "";
}

struct echo_handler
{
	boost::asio::io_service & io_service_;
	boost::asio::deadline_timer echo_timer_;
	int counter_;
	echo_handler(boost::asio::io_service & io_service)
		: io_service_(io_service)
		, echo_timer_(io_service_)
		, counter_(0)
	{
	}
	void start()
	{
		echo_timer_.expires_from_now(boost::posix_time::seconds(1));
		echo_timer_.async_wait(boost::bind(&echo_handler::operator(), this, boost::asio::placeholders::error()));
	}
	void operator()(const boost::system::error_code & ec)
	{
		if (ec == boost::asio::error::operation_aborted)
		{
			return;
		}
		std::cout << esc(counter_ % 2 == 0 ? MAKE_GREEN : MAKE_YELLOW) << (++counter_) << ". Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua." << esc(RESET_COLOR) << std::endl;
		start();
	}
};

int
main(int argc, char * argv[])
{
	boost::asio::io_service io_service;
	echo_handler echo_service(io_service);
	echo_service.start();
	io_service.run();
}