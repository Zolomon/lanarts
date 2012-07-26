#include "ServerNetConnection.h"

typedef boost::shared_ptr<SocketStream> stream_ptr;

void ServerNetConnection::finalize_connections() {
	accepting_connections = false;
}

void ServerNetConnection::accept_handler(SocketStream* ss) {
	if (!error) {
//		if (!accepting_connections) {
//			if (ss)
//				delete ss;
//			return;
//		}
		if (ss) {
			streamlock.lock();
			streams.push_back(stream_ptr(ss));
			streamlock.unlock();

			printf("connection accepted\n");
			asio::async_read(
					ss->get_socket(),
					asio::buffer(ss->last_message().data,
							NetPacket::HEADER_LEN),
					boost::bind(socketstream_read_header_handler, ss,
							asio::placeholders::error));
			sendlock.lock();
			std::vector<boost::shared_ptr<NetPacket> > tosend_copy =
					tosend_to_new;
			sendlock.unlock();
			for (int i = 0; i < tosend_copy.size(); i++) {
				ss->send_packet(*tosend_copy[i].get());
			}
		}
		ss = new SocketStream(io_service);
//		peer_id = ss->get_peer_id();
		acceptor.async_accept(
				ss->get_socket(),
				boost::bind(&ServerNetConnection::accept_handler, this, ss,
						asio::placeholders::error));
	}
}

static void wrapped_run(asio::io_service* ios) {
	try {
		ios->run();
	} catch (const std::exception& e) {
		printf("type=%s\n", typeid(e).name());
		printf("%s\n", e.what());
	}
	printf("io_service::run completed!\n");
}

ServerNetConnection::ServerNetConnection(int port) :
		io_service(), endpoint(asio::ip::tcp::v4(), port), acceptor(io_service,
				endpoint) {

	io_service.post(
			boost::bind(&ServerNetConnection::accept_handler, this,
					(SocketStream*)NULL, asio::error_code()));
//	accept_handler(NULL, asio::error_code());
	execution_thread = boost::shared_ptr<asio::thread>(
			new asio::thread(boost::bind(&wrapped_run, &io_service)));

	accepting_connections = true;
}

ServerNetConnection::~ServerNetConnection() {
}

void ServerNetConnection::assign_peerid(SocketStream* stream, int peerid) {
//	NetPacket packet(get_peer_id());
//	packet.packet_type = NetPacket::PACKET_ASSIGN_PEERID;
//	packet.add_int(peerid);
//	stream->send_packet(packet);
}
bool ServerNetConnection::get_next_packet(NetPacket& packet, packet_t type) {
	bool found = false;

	streamlock.lock();
	std::vector<stream_ptr> s = streams;
	streamlock.unlock();

	for (int i = 0; i < s.size() && !found; i++) {
		SocketStream* ss = s[i].get();
		boost::mutex& m = ss->get_rmutex();
		//We try to determine the status without a lock, should never be 0 when non-empty
		m.lock();
		PacketQueue::iterator it = ss->rmessages().begin();
		for (; it != ss->rmessages().end(); it++) {
			NetPacket* p = it->get();
			if (p->packet_type == type) {
				packet = *p;
				ss->rmessages().erase(it);
				found = true;
				break;
			}
		}
		m.unlock();
	}
	return found;
}

void ServerNetConnection::broadcast_packet(const NetPacket & packet,
		bool send_to_new) {
	streamlock.lock();
	for (int i = 0; i < streams.size(); i++) {
		streams[i]->send_packet(packet);
	}
	streamlock.unlock();
	if (send_to_new) {
		sendlock.lock();
		tosend_to_new.push_back(
				boost::shared_ptr<NetPacket>(new NetPacket(packet)));
		sendlock.unlock();
	}
}

void ServerNetConnection::join() {
	execution_thread->join();
}

