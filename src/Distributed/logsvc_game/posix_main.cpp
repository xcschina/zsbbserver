#if !defined(_WIN32)

#include "server.hpp"
#include "request_handler.hpp"
#include "xmldoc.hpp"

#include <pthread.h>
#include <signal.h>

void read_xml_config(int & thread_pool, u_short & listen_port, std::string & db_uid, 
					 std::string & db_pwd, std::string & db_dsn)
{
	xmlreader::xmldoc doc("config.xml");

	if (!doc.load_xml_doc()) throw std::runtime_error("load_xml_doc error");

	xmlreader::xmlnode * config_node = doc.get_root_node("config");
	if (!config_node) throw std::runtime_error("get_root_node(config) error");

	xmlreader::xmlnode * logsvc_node = config_node->get_child("logsvc");
	if (!logsvc_node) throw std::runtime_error("get_child(logsvc) error");

	thread_pool = ::atoi(logsvc_node->property("thread_pool").c_str());
	listen_port = ::atoi(logsvc_node->property("listen_port").c_str());
	db_pwd = logsvc_node->property("db_password").c_str();
	db_uid = logsvc_node->property("db_username").c_str();
	db_dsn = logsvc_node->property("db_dsn").c_str();
}

int main(int argc, char* argv[])
{
	try
	{
		u_short listen_port;
		int thread_pool;
		std::string db_uid;
		std::string db_pwd;
		std::string db_dsn;

		read_xml_config(thread_pool, listen_port, db_uid, db_pwd, db_dsn);

		sigset_t new_mask;
		sigfillset(&new_mask);
		sigset_t old_mask;
		pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);

		request_handler handler;
		server s(thread_pool, &handler, listen_port);
		handler.start(db_uid.c_str(), db_pwd.c_str(), db_dsn.c_str());

		boost::thread t(boost::bind(&server::io_service_run_event_loop, &s));

		std::cout << "LogServer v1.01\nPress CTRL + C key to exit\n";

		pthread_sigmask(SIG_SETMASK, &old_mask, 0);

		sigset_t wait_mask;
		sigemptyset(&wait_mask);
		sigaddset(&wait_mask, SIGINT);
		sigaddset(&wait_mask, SIGQUIT);
		sigaddset(&wait_mask, SIGTERM);
		pthread_sigmask(SIG_BLOCK, &wait_mask, 0);
		int sig = 0;
		sigwait(&wait_mask, &sig);

		s.io_service_end_event_loop();
		handler.stop();
		t.join();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}

#endif // !defined(_WIN32)
