#include "focuscontroller.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QLineEdit>
#include <QDoubleValidator>

#include <QDebug>

int FocusController::getNextId() {
    static int curId = 0;
    return ++curId;
}

void FocusController::setupWidget() {
    QVBoxLayout *widgetLayout = new QVBoxLayout(this);

    // title
    QLabel *titleLabel = new QLabel();
    titleLabel->setText("Hedrick Focuser #" + QString::number(m_focuserId));
    titleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    titleLabel->setStyleSheet("background-color: none; border: 1px inset black;");
    titleLabel->setMaximumHeight(48);
    titleLabel->setFont(QFont("Consolas", 32, QFont::Bold));
    widgetLayout->addWidget(titleLabel);

    // connect & enable
    QVBoxLayout *buttonLayout = new QVBoxLayout();

    QHBoxLayout *connectLayout = new QHBoxLayout();
    m_connectBtn = new QPushButton("Connect", this);
    m_disconnectBtn = new QPushButton("Disconnect", this);
    m_disconnectBtn->setDisabled(true);
    connectLayout->addWidget(m_connectBtn);
    connectLayout->addWidget(m_disconnectBtn);

    connect(m_connectBtn, &QPushButton::clicked, this, &FocusController::connectPwi);

    QHBoxLayout *enableLayout = new QHBoxLayout();
    m_enableBtn = new QPushButton("Enable", this);
    m_disableBtn = new QPushButton("Disable", this);
    m_enableBtn->setDisabled(true);
    m_disableBtn->setDisabled(true);
    enableLayout->addWidget(m_enableBtn);
    enableLayout->addWidget(m_disableBtn);

    buttonLayout->addLayout(connectLayout);
    buttonLayout->addLayout(enableLayout);
    widgetLayout->addLayout(buttonLayout);

    // focuser position
    QHBoxLayout *positionLayout = new QHBoxLayout();
    QLabel *positionDescriptionLabel = new QLabel("Position: ");
    positionDescriptionLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    positionDescriptionLabel->setFont(QFont("Consolas", 20));
    positionLayout->addWidget(positionDescriptionLabel, 2);

    m_positionLabel = new QLabel();
    m_positionLabel->setText("NaN");
    m_positionLabel->setFont(QFont("Consolas", 20));
    m_positionLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    positionLayout->addWidget(m_positionLabel, 2);

    QVBoxLayout *stepLayout = new QVBoxLayout();
    m_upStepBtn = new QPushButton();
    m_downStepBtn = new QPushButton();

    QPixmap arrowIcon(":images/Arrow Icon.png");
    m_upStepBtn->setIcon(QIcon(arrowIcon.transformed(QTransform().rotate(90))));
    m_downStepBtn->setIcon(QIcon(arrowIcon.transformed(QTransform().rotate(-90))));

    m_stepSizeText = new QLineEdit();
    m_stepSizeText->setValidator(new QDoubleValidator(1, 500, 1));
    m_stepSizeText->setAlignment(Qt::AlignHCenter);
    connect(m_stepSizeText, &QLineEdit::textChanged, this, &FocusController::updateStepSize);

    stepLayout->addWidget(m_upStepBtn);
    stepLayout->addWidget(m_stepSizeText);
    stepLayout->addWidget(m_downStepBtn);

    positionLayout->addLayout(stepLayout);
    widgetLayout->addLayout(positionLayout);

    // is-moving/stop
    QHBoxLayout *movingLayout = new QHBoxLayout();
    m_isMovingLabel = new QLabel("Disconnected...");
    m_isMovingLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_isMovingLabel->setFont(QFont("Consolas", 20));
    m_isMovingLabel->setStyleSheet("color: gray;");

    m_stopBtn = new QPushButton();
    m_stopBtn->setStyleSheet("background-color: red; color: white;");
    m_stopBtn->setText("STOP");
    m_stopBtn->setFont(QFont("Consolas", 24));

    movingLayout->addWidget(m_isMovingLabel);
    movingLayout->addWidget(m_stopBtn);
    widgetLayout->addLayout(movingLayout);

    // event timer
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &FocusController::updatePosition);
    connect(timer, &QTimer::timeout, this, &FocusController::updateIsMoving);
    timer->start(100);
}

void FocusController::checkPwiConnection() {
    if (m_pwi->focuserIsConnected()) {
        m_connectBtn->setDisabled(true);
        m_disconnectBtn->setDisabled(false);

        m_enableBtn->setDisabled(false);
    }
}

FocusController::FocusController(QWidget *parent, IPWI4Client *pwi) : QWidget{parent}, m_pwi(pwi) {
    m_focuserId = getNextId();
    this->setupWidget();
}

FocusController::~FocusController() {
    delete m_pwi;
}

// slots
void FocusController::connectPwi(bool isChecked) {
    m_pwi->focuserConnect();
    QTimer::singleShot(500, this, &FocusController::checkPwiConnection);
}

void FocusController::updatePosition() {
    if (m_pwi->focuserIsConnected() && m_pwi->focuserIsEnabled()) {
        m_positionLabel->setText(QString::number(m_pwi->focuserPosition()));
    }
}

void FocusController::updateStepSize(const QString &textChanged) {
    float stepSizeChange = textChanged.toFloat();
    if (stepSizeChange > 500) {
        m_upStepBtn->setDisabled(true);
        m_downStepBtn->setDisabled(true);
        m_stepSizeText->setStyleSheet("background-color: darkred;");
    } else {
        m_upStepBtn->setDisabled(false);
        m_downStepBtn->setDisabled(false);
        m_stepSize = stepSizeChange;
        m_stepSizeText->setStyleSheet("background-color: none;");
    }
}

void FocusController::updateIsMoving() {
    if (m_pwi->focuserIsConnected() && m_pwi->focuserIsEnabled()) {
        if (m_pwi->focuserIsMoving()) {
            m_isMovingLabel->setText("Moving...");
            m_isMovingLabel->setStyleSheet("color: red");
        } else {
            m_isMovingLabel->setText("Ready");
            m_isMovingLabel->setStyleSheet("color: green");
        }
    }
}
