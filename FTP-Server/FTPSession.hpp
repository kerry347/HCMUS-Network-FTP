#pragma once
#include <experimental/net>

#include <filesystem>
#include <fstream>
#include <deque>
#include <map>
#include <memory>
#include <set>

#include "FTPMsgs.hpp"
#include "FTPUser.hpp"
#include "UserDatabase.hpp"

namespace fs = std::filesystem;
namespace net = std::experimental::net;

class FTPSession;
using session_ptr = std::shared_ptr<FTPSession>;

class FTPSession : public std::enable_shared_from_this<FTPSession> {
  using charbuf_ptr = std::shared_ptr<std::vector<char>>;

 public:
  FTPSession(net::io_context& context, net::ip::tcp::socket& socket,
             UserDatabase& userDb, std::set<session_ptr>& logged_users,
             std::function<void(std::string)> const& completionHandler);
  virtual ~FTPSession();
  std::string getUserName() const;
  void start();

 private:
  struct IoFile {
    IoFile(fs::path const& path, std::ios::openmode mode)
        : fileStream_(path, mode), streamBuf_(1 << 20) {
      fileStream_.rdbuf()->pubsetbuf(
          streamBuf_.data(), static_cast<std::streamsize>(streamBuf_.size()));
    }
    virtual ~IoFile() {
      fileStream_.flush();
      fileStream_.close();
    }
    std::fstream fileStream_;
    std::vector<char> streamBuf_;
  };
  using ioFile_ptr = std::shared_ptr<IoFile>;

  FTPMsgs handleFTPCmdUADD(std::string const& para);
  FTPMsgs handleFTPCmdUSER(std::string const& para);
  FTPMsgs handleFTPCmdPASS(std::string const& para);
  FTPMsgs handleFTPCmdACCT(std::string const& para);
  FTPMsgs handleFTPCmdCWD(std::string const& para);
  FTPMsgs handleFTPCmdCDUP(std::string const& para);
  FTPMsgs handleFTPCmdREIN(std::string const& para);
  FTPMsgs handleFTPCmdQUIT(std::string const& para);

  // Transfer paraeter commands
  FTPMsgs handleFTPCmdPORT(std::string const& para);
  FTPMsgs handleFTPCmdPASV(std::string const& para);
  FTPMsgs handleFTPCmdTYPE(std::string const& para);
  FTPMsgs handleFTPCmdSTRU(std::string const& para);
  FTPMsgs handleFTPCmdMODE(std::string const& para);

  // Ftp service commands
  FTPMsgs handleFTPCmdRETR(std::string const& para);
  FTPMsgs handleFTPCmdSTOR(std::string const& para);
  FTPMsgs handleFTPCmdSTOU(std::string const& para);
  FTPMsgs handleFTPCmdAPPE(std::string const& para);
  FTPMsgs handleFTPCmdALLO(std::string const& para);
  FTPMsgs handleFTPCmdREST(std::string const& para);
  FTPMsgs handleFTPCmdRNFR(std::string const& para);
  FTPMsgs handleFTPCmdRNTO(std::string const& para);
  FTPMsgs handleFTPCmdABOR(std::string const& para);
  FTPMsgs handleFTPCmdDELE(std::string const& para);
  FTPMsgs handleFTPCmdRMD(std::string const& para);
  FTPMsgs handleFTPCmdMKD(std::string const& para);
  FTPMsgs handleFTPCmdPWD(std::string const& para);
  FTPMsgs handleFTPCmdLIST(std::string const& para);
  FTPMsgs handleFTPCmdNLST(std::string const& para);
  FTPMsgs handleFTPCmdSITE(std::string const& para);
  FTPMsgs handleFTPCmdSYST(std::string const& para);
  FTPMsgs handleFTPCmdSTAT(std::string const& para);
  FTPMsgs handleFTPCmdHELP(std::string const& para);
  FTPMsgs handleFTPCmdNOOP(std::string const& para);

  void sendFile(ioFile_ptr const& file);
  void readFileDataAndSend(net::ip::tcp::socket& dataSocket,
                           ioFile_ptr const& file);
  void addDataToBufferAndSend(
      net::ip::tcp::socket& dataSocket, charbuf_ptr const& data,
      std::function<void(void)> fetchMore = []() { return; });
  void writeDataToSocket(net::ip::tcp::socket& dataSocket,
                         std::function<void(void)> fetchMore);

  void receiveFile(ioFile_ptr const& file);
  void receiveDataFromSocketAndWriteToFile(net::ip::tcp::socket& dataSocket,
                                           ioFile_ptr const& file);
  void writeDataToFile(
      charbuf_ptr const& data, ioFile_ptr const& file,
      std::function<void(void)> fetchMore = []() { return; });

  fs::path FTP2LocalPath(fs::path const& ftpPath) const;
  std::string Local_to_FTP_Path(fs::path const& ftp_Path) const;
  FTPMsgs checkPathRenamable(fs::path const& ftpPath) const;
  void sendDirListing(std::map<fs::path, fs::file_status> const& dirContent);
  void sendNameList(std::map<fs::path, fs::file_status> const& dirContent);

  void sendFTPMsg(FTPMsgs const& msg);
  void startSendingMsgs();
  void readFTPCmd();
  void handleFTPCmd(std::string const& cmd);

  std::function<void(std::string)> const completionHandler_;

  UserDatabase& userDb_;
  std::shared_ptr<FTPUser> loggedUser_;
  std::string lastCmd_;
  std::string username_;
  std::string renameSrcPath_;
  fs::path ftpWorkingDir_;

  net::io_context& context_;
  net::ip::tcp::socket cmdSocket_;
  net::strand<net::io_context::executor_type> msgWriteStrand_;
  std::string cmdInputStr_;
  std::deque<std::string> msgOutputQueue_;

  bool dataTypeBinary_;
  std::set<session_ptr>& logged_users_;
  net::ip::tcp::acceptor dataAcceptor_;
  std::deque<charbuf_ptr> dataBuffer_;
  net::strand<net::io_context::executor_type> fileRWStrand_;
  net::strand<net::io_context::executor_type> dataBufStrand_;
};
