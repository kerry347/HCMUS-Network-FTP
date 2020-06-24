#include <experimental/io_context>
#include <experimental/internet>

#include <string>
#include <utility>
#include <fstream>

namespace net = std::experimental::net;

class FTPClient {
  using FTPMsg = std::pair<int, std::string>;
  using charbuf_ptr = std::shared_ptr<std::vector<char>>;

 public:
  FTPClient();
  void connect(std::string const& ip, uint16_t port);
  void close();

  bool signup(std::string const& uname, std::string const& pass);
  bool login(std::string const& uname, std::string const& pass);
  bool upload(std::string const& local_file, std::string const& remote_file);
  bool download(std::string const& remote_file, std::string const& local_file);
  bool pwd();
  bool ls(std::string const& remoteDir);
  bool cd(std::string const& remoteDir);

 private:
  FTPMsg readFTPMsg();
  void sendCmd(std::string const& cmd);
  std::string dataRecv();

  void closeDataSocket();
  bool resetDataSocket();

  net::io_context ioContext_;
  net::ip::tcp::socket msgSocket_;
  net::ip::tcp::socket dataSocket_;
};