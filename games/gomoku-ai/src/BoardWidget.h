#pragma once

#include <QPixmap>
#include <QWidget>

#include "Chess.h"

// 棋盘绘制组件：底图 + 黑白棋子，鼠标点击换算格点
class BoardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BoardWidget(QWidget* parent = nullptr);

    void setChess(Chess* chess) { m_chess = chess; }
    void repaintBoard() { update(); }

signals:
    void cellClicked(int row, int col);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    Chess* m_chess = nullptr;
    QPixmap m_boardBg;
    QPixmap m_blackPiece;
    QPixmap m_whitePiece;
};
