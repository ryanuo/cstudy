#include "BoardWidget.h"

#include <QMouseEvent>
#include <QPainter>

BoardWidget::BoardWidget(QWidget* parent)
    : QWidget(parent)
{
    setFixedSize(600, 600);

    // 资源来自 resources.qrc（原 EasyX 的 loadimage 改为 Qt 资源）
    m_boardBg = QPixmap(QStringLiteral(":/res/board.jpg"));
    m_blackPiece = QPixmap(QStringLiteral(":/res/black.png"));
    m_whitePiece = QPixmap(QStringLiteral(":/res/white.png"));
}

void BoardWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.drawPixmap(rect(), m_boardBg);

    if (!m_chess)
    {
        return;
    }

    const int gradeSize = m_chess->getGradeSize();
    const int marginX = m_chess->getMarginX();
    const int marginY = m_chess->getMarginY();
    const float chessSize = m_chess->getChessSize();

    for (int row = 0; row < gradeSize; row++)
    {
        for (int col = 0; col < gradeSize; col++)
        {
            const int kind = m_chess->getChessData(row, col);
            if (kind == 0)
            {
                continue;
            }

            // 与原版一致：棋子左上角 = 交点 - 半格
            const int x = marginX + chessSize * col - 0.5f * chessSize;
            const int y = marginY + chessSize * row - 0.5f * chessSize;
            const QPixmap& piece = (kind == CHESS_BLACK) ? m_blackPiece : m_whitePiece;
            painter.drawPixmap(x, y, static_cast<int>(chessSize), static_cast<int>(chessSize), piece);
        }
    }
}

void BoardWidget::mousePressEvent(QMouseEvent* event)
{
    if (!m_chess || event->button() != Qt::LeftButton)
    {
        return;
    }

    ChessPos pos;
    if (m_chess->clickBoard(event->pos().x(), event->pos().y(), &pos))
    {
        emit cellClicked(pos.row, pos.col);
    }
}
