#include "ClientNetConnection.h"
#include <string>

using namespace asio::ip;

void socketstream_do_close(SocketStream* ss) {
	if (!ss->is_closed()) {
		ss->get_socket().close();
		ss->is_closed() = true;
	}
}

void socketstream_read_header_handler(SocketStream* ss,
		const asio::error_code& error) {
	if (!error && ss->last_message().decode_header()) {
//		printf("reading header of message\n");
//		if (ss->last_message().body_length > 0){
		asio::async_read(
				ss->get_socket(),
				asio::buffer(ss->last_message().body_start(),
						ss->last_message().body_length),
				boost::bind(socketstream_read_body_handler, ss,
						asio::placeholders::error));
//		} else {
//			asio::async_read(
//					ss->get_socket(),
//					asio::buffer(ss->last_message().data, NetPacket::HEADER_LEN),
//					boost::bind(&socketstream_read_header_handler, ss,
//							asio::placeholders::error));
//		}
	} else {
//         socketstream_do_close(ss);
	}
}

void socketstream_read_body_handler(SocketStream* ss,
		const asio::error_code& error) {
	if (!error) {
		ss->last_message().decode_header();
		ss->get_rmutex().lock();
		ss->rmessages().push_back(
				boost::shared_ptr<NetPacket>(
						new NetPacket(ss->last_message())));
		ss->get_rmutex().unlock();

		asio::async_read(
				ss->get_socket(),
				asio::buffer(ss->last_message().data, NetPacket::HEADER_LEN),
				boost::bind(&socketstream_read_header_handler, ss,
						asio::placeholders::error));

	} else {
//         socketstream_do_close(ss);
	}
}

void socketstream_write_handler(SocketStream* ss, const asio::error_code& error,
		bool pop) {
	if (!error) {
		ss->get_socket().set_option(asio::ip::tcp::no_delay());
		ss->get_wmutex().lock();
		if (pop)
			ss->wmessages().pop_front();
		if (!ss->wmessages().empty()) {
			asio::async_write(
					ss->get_socket(),
					asio::buffer(ss->wmessages().front()->data,
							ss->wmessages().front()->length()),
					boost::bind(socketstream_write_handler, ss,
							asio::placeholders::error, true));
		}
		ss->get_wmutex().unlock();
	} else {
//         socketstream_do_close(ss);
	}
}

SocketStream::SocketStream(asio::io_service & io_service) :
		io_service(io_service), socket(io_service) {
	closed = false;
}

SocketStream::~SocketStream() {
	socketstream_do_close(this);
}

bool SocketStream::get_next_packet(NetPacket& packet, packet_t type) {
	if (closed)
		return false;

	rmutex.lock();
	PacketQueue::iterator it = reading_msgs.begin();
	for (; it != reading_msgs.end(); it++) {
		NetPacket* p = it->get();
		if (p->packet_type == type) {
			packet = *p;
			reading_msgs.erase(it);
			rmutex.unlock();
			return true;
		}
	}
	rmutex.unlock();

	return false;
}

void SocketStream::send_packet(const NetPacket & packet) {
	if (closed) {
		printf("Pools closed due to AIDS\n!");
		return;
	}

	wmutex.lock();
	bool write_in_progress = !writing_msgs.empty();
	writing_msgs.push_back(boost::shared_ptr<NetPacket>(new NetPacket(packet)));
	if (!write_in_progress) {
		io_service.post(
				boost::bind(socketstream_write_handler, this,
						asio::error_code(), false));
	}
	wmutex.unlock();
}
