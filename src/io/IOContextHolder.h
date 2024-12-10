#ifndef IOCONTEXTHOLDER_H
#define IOCONTEXTHOLDER_H

#include <boost/asio.hpp>
#include <thread>
#include <vector>
#include <memory>

class IOContextHolder {
public:
	using WorkGuardType = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

	IOContextHolder(boost::asio::io_context& iOContext);
	~IOContextHolder();
	void start();
	static boost::asio::io_context& getIOContext();

private:
	boost::asio::io_context& IOContext;
	std::vector<std::thread> threadVector;
	WorkGuardType workGuard;
};

#endif // IOCONTEXTHOLDER_H