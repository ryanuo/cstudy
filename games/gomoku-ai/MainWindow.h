#pragma once

#include <QMainWindow>

#include "Chess.h"

class BoardWidget;
class NetworkManager;
class QPushButton;
class QMediaPlayer;
class QAudioOutput;
class QSoundEffect;

// 主窗口：联机对战唯一模式（黑方 HOST 先手，白方 CLIENT 后手）
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void startOnline(const QString& password); // 供 --online 参数调用

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onConnectClicked();
    void onNewGameClicked();
    void onDisconnectClicked();
    void onCellClicked(int row, int col);
    void onConnected(bool isHost);
    void onMoveReceived(int row, int col);
    void onRestartReceived();
    void onDisconnected();
    void onStatusChanged(const QString& text);

private:
    void resetBoard();
    void checkGameEnd(chess_kind_t lastKind);
    void playSfx(const QString& qrcPath);
    void setStatus(const QString& text);
    bool myTurn() const;
    chess_kind_t opponentKind() const;

    Chess* m_chess = nullptr;
    BoardWidget* m_board = nullptr;
    NetworkManager* m_network = nullptr;
    QPushButton* m_connectBtn = nullptr;
    QPushButton* m_newGameBtn = nullptr;
    QPushButton* m_disconnectBtn = nullptr;

    chess_kind_t m_myKind = CHESS_BLACK;
    bool m_connected = false;
    bool m_gameOver = false;

    QMediaPlayer* m_bgPlayer = nullptr;
    QAudioOutput* m_bgAudio = nullptr;
    QMediaPlayer* m_sfxPlayer = nullptr;
    QAudioOutput* m_sfxAudio = nullptr;
    QSoundEffect* m_startSound = nullptr;
    QSoundEffect* m_downSound = nullptr;
};
