#include <experimental/io_context>
#include <experimental/internet>

#include <string>
#include <utility>
#include <fstream>
#include <sstream>
#include <list>

namespace net = std::experimental::net;
using FTPMsg = std::pair<int, std::string>;

class FTPClient {
  using charbuf_ptr = std::shared_ptr<std::vector<char>>;

 public:
  FTPClient();
  void connect(std::string const& ip, uint16_t port);
  void close();

  bool signup(std::string const& uname, std::string const& pass);
  bool login(std::string const& uname, std::string const& pass);
  bool upload(std::string const& local_file, std::string const& remote_file);
  bool download(std::string const& remote_file, std::string const& local_file);
  bool mkdir(std::string const& dirName);
  bool pwd();
  bool ls(std::string const& remoteDir);
  bool cd(std::string const& remoteDir);
  bool rmdir(std::string const& directory_name);
  bool rm(std::string const& remote_file);

 private:
  FTPMsg recvFTPMsg();
  void sendCmd(std::string const& cmd);
  std::string recvListDir();
  void sendData(charbuf_ptr const& data);
  size_t recvData(charbuf_ptr const& bufPtr);

  void closeDataSocket();
  bool resetDataSocket();

  net::io_context ioContext_;
  net::ip::tcp::socket msgSocket_;
  net::ip::tcp::socket dataSocket_;
};