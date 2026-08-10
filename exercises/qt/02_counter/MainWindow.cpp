#include "MainWindow.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("Qt 练习"));
    resize(420, 200);

    m_valueLabel = new QLabel(QStringLiteral("当前值: 0"), this);

    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setRange(0, 100);
    m_slider->setValue(0);

    m_spinBox = new QSpinBox(this);
    m_spinBox->setRange(0, 100);
    m_spinBox->setValue(0);

    m_countButton = new QPushButton(QStringLiteral("计数 %1").arg(m_count), this);

    // 滑块与数字框双向同步
    connect(m_slider, &QSlider::valueChanged, m_spinBox, &QSpinBox::setValue);
    connect(m_spinBox, &QSpinBox::valueChanged, m_slider, &QSlider::setValue);

    // 数值变化时更新标签
    connect(m_slider, &QSlider::valueChanged, this, [this](int v) {
        m_valueLabel->setText(QStringLiteral("当前值: %1").arg(v));
    });

    // 按钮计数
    connect(m_countButton, &QPushButton::clicked, this, &MainWindow::onCountClicked);

    auto *valueRow = new QHBoxLayout;
    valueRow->addWidget(m_valueLabel);
    valueRow->addStretch();

    auto *sliderRow = new QHBoxLayout;
    sliderRow->addWidget(m_slider);
    sliderRow->addWidget(m_spinBox);

    auto *central = new QWidget(this);
    auto *layout  = new QVBoxLayout(central);
    layout->addLayout(valueRow);
    layout->addLayout(sliderRow);
    layout->addWidget(m_countButton);

    setCentralWidget(central);
}

void MainWindow::onCountClicked()
{
    ++m_count;
    m_countButton->setText(QStringLiteral("计数 %1").arg(m_count));
}