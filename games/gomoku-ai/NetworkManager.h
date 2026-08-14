#pragma once

#include <QByteArray>
#include <QHostAddress>
#include <QObject>

class QUdpSocket;
class QTcpServer;
class QTcpSocket;
class QTimer;

// 局域网联机管理：UDP 广播发现同密码对手 -> 确定性角色协商 -> TCP 传输
//
// 协议（'\n' 结尾的 ASCII 行）：
//   发现:  GOMOKU1|HELLO|<sha256(密码)hex>|<nonce>    (UDP 广播, 每 1s)
//   握手:  HELLO <sha256(密码)hex>                    (TCP 双向)
//   对局:  MOVE <row> <col> / RESTART / QUIT / ERR
//
// 角色协商：nonce 较小者 = HOST（执黑先手，TCP 服务端）；
//           nonce 较大者 = CLIENT（执白后手，TCP 客户端）。
// 双方用同一组 (自己的 nonce, 对方 nonce) 独立计算，结果必然一致。
class NetworkManager : public QObject
{
    Q_OBJECT

public:
    static constexpr quint16 kUdpPort = 45231;
    static constexpr quint16 kTcpPort = 45232;

    explicit NetworkManager(QObject* parent = nullptr);
    ~NetworkManager() override;

    void start(const QString& password);  // 开始发现+配对
    void stop();                          // 断开（若已连接先通知对端 QUIT）

    bool isConnected() const { return m_connected; }
    bool isHost() const { return m_isHost; }
    QHostAddress peerAddress() const { return m_peerIp; }

    void sendMove(int row, int col);
    void sendRestart();

signals:
    void statusChanged(const QString& text);
    void connected(bool isHost);      // 配对成功；isHost=true 执黑先手
    void moveReceived(int row, int col);
    void restartReceived();
    void disconnected();

private slots:
    void onUdpReadyRead();
    void onTcpReadyRead();
    void onTcpConnected();
    void onTcpDisconnected();
    void onTcpError(QAbstractSocket::SocketError error);
    void onNewConnection();
    void onRetryConnect();

private:
    void broadcastHello();
    void tryPair(const QHostAddress& peerIp, const QByteArray& peerNonce);
    void startTcpServer();
    void connectToPeer();
    void handleLine(const QByteArray& line);
    void sendLine(const QByteArray& line);
    void checkHandshake();
    void cleanup();

    QUdpSocket* m_udp = nullptr;
    QTcpServer* m_tcpServer = nullptr;
    QTcpSocket* m_tcp = nullptr;
    QTimer* m_broadcastTimer = nullptr;
    QTimer* m_pairTimeout = nullptr;
    QTimer* m_retryTimer = nullptr;
    int m_retryCount = 0;

    QByteArray m_passwordHash;
    QByteArray m_nonce;
    QHostAddress m_peerIp;
    bool m_isHost = false;
    bool m_connecting = false;
    bool m_connected = false;
    bool m_peerHelloOk = false;
    QByteArray m_tcpBuffer;
};
