#include <iostream>
#include <string>
#include <set>
#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include "build_info.hpp"

#define RESET_COLOR "\e[m"
#define MAKE_GREEN "\e[32m"
#define MAKE_YELLOW "\e[33m"
#define MAKE_RED "\e[31m"

#define LOG_HEADER '[' << ::getpid() << "] "

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
	void stop()
	{
		echo_timer_.cancel();
	}
	void operator()(const boost::system::error_code & ec)
	{
		if (ec == boost::asio::error::operation_aborted)
		{
			std::cout << esc(MAKE_YELLOW)
				<< LOG_HEADER
				<< "Echo timer is aborted."
				<< esc(RESET_COLOR)
				<< std::endl;
			return;
		}
		std::cout << esc(counter_ % 2 == 0 ? MAKE_GREEN : MAKE_YELLOW)
			<< LOG_HEADER
			<< (++counter_)
			<< ". Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua." 			<< esc(RESET_COLOR)
			<< std::endl;
		start();
	}
};

struct session
	: boost::enable_shared_from_this<session>
{
	boost::asio::io_service & io_service_;
	boost::asio::ip::tcp::socket socket_;
	boost::array<char, 1024> data_;
	session(boost::asio::io_service & io_service)
		: io_service_(io_service)
		, socket_(io_service_)
	{
	}
	void start()
	{
		socket_.async_read_some(boost::asio::buffer(data_),
			boost::bind(&session::handle_read, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}
	void handle_read(const boost::system::error_code& error,
		size_t bytes_transferred)
	{
		if (!error)
		{
			boost::asio::async_write(socket_,
				boost::asio::buffer(data_.data(), bytes_transferred),
				boost::bind(&session::handle_write, shared_from_this(),
					boost::asio::placeholders::error));
		}
		else
		{
			std::cerr << esc(MAKE_RED)
				<< LOG_HEADER
				<< "Unable to receive data: "
				<< error.message()
				<< " [Errno: "
				<< error.value()
				<< ']'
				<< esc(RESET_COLOR)
				<< std::endl;
		}
	}
	void handle_write(const boost::system::error_code & error)
	{
		if (!error)
		{
			start();
		}
		else
		{
			std::cerr << esc(MAKE_RED)
				<< LOG_HEADER
				<< "Unable to write data: "
				<< error.message()
				<< " [Errno: "
				<< error.value()
				<< ']'
				<< esc(RESET_COLOR)
				<< std::endl;
		}
	}
};

struct server
{
	boost::asio::io_service & io_service_;
	boost::asio::ip::tcp::acceptor acceptor_;
	server(boost::asio::io_service & io_service,
				const boost::asio::ip::tcp::endpoint & endpoint)
		: io_service_(io_service)
		, acceptor_(io_service_, endpoint)
	{
		std::cout << esc(MAKE_GREEN)
			<< LOG_HEADER
			<< "Echo server is listening on port: "
			<< endpoint.port()
			<< esc(RESET_COLOR)
			<< std::endl;
		start_accept();
	}
	void start_accept()
	{
		boost::shared_ptr<session> new_session =
			boost::make_shared<session>(boost::ref(io_service_));
		acceptor_.async_accept(new_session->socket_,
			boost::bind(&server::handle_accept, this,
				boost::asio::placeholders::error,
				new_session));
	}
	void stop_accept()
	{
		acceptor_.close();
	}
	void handle_accept(const boost::system::error_code & error,
		const boost::shared_ptr<session> & session_ptr)
	{
		if (!error)
		{
			boost::asio::ip::tcp::endpoint remote_endpoint =
				session_ptr->socket_.remote_endpoint();
			boost::asio::ip::address remote_addr = remote_endpoint.address();
			std::cout << esc(MAKE_GREEN)
				<< LOG_HEADER
				<< "New client connected (" << remote_addr.to_string() << ")"
				<< esc(RESET_COLOR)
				<< std::endl;
			session_ptr->start();
		}
		start_accept();
	}
};

struct application
{
	boost::asio::io_service & io_service_;
	echo_handler echo_service_;
	boost::asio::signal_set signals_;
	server server_;
	application(boost::asio::io_service & io_service, short port)
		: io_service_(io_service)
		, echo_service_(io_service_)
		, signals_(io_service_, SIGTERM, SIGINT)
		, server_(io_service_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
	{
		echo_service_.start();
		signals_.async_wait(boost::bind(&application::operator(), this, boost::asio::placeholders::error(), _2));
	}
	void operator()(const boost::system::error_code & ec, int signal_number)
	{
		std::string signal_name;
		switch (signal_number)
		{
		case SIGINT:
			signal_name = "SIGINT";
			break;
		case SIGTERM:
			signal_name = "SIGTERM";
			break;
		default:
			signal_name = "Unknown signal";
			break;
		}
		std::cerr << esc(MAKE_RED) << LOG_HEADER << signal_name << " received. Exiting!" << esc(RESET_COLOR) << std::endl;
		io_service_.stop();
	}
};

int
main(int argc, char * argv[])
{
	namespace po = boost::program_options;
	short app_port;
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("version", "show version")
		("port,p", po::value<short>(&app_port)->default_value(9876), "server port")
	;
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	if (vm.count("help"))
	{
		std::cout << desc << std::endl;
		return 0;
	}
	if (vm.count("version"))
	{
		std::cout << "build: " BUILD_INFO_DATE << std::endl;
		std::cout << "version: " BUILD_INFO_COMMIT << std::endl;
		return 0;
	}
	boost::asio::io_service io_service;
	application app(io_service, app_port);
	io_service.run();
}